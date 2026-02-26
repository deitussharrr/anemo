#include "semantic.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct VarSym {
    char *name;
    TypeKind type;
    int mutable_flag;
    int depth;
} VarSym;

typedef struct FnSym {
    char *name;
    TypeKind ret;
    ParamArray params;
    int line;
    int col;
} FnSym;

typedef struct Checker {
    const char *file;
    Program *program;

    FnSym *fns;
    size_t fn_len;
    size_t fn_cap;

    VarSym *vars;
    size_t var_len;
    size_t var_cap;
    int depth;

    FnSym *current_fn;
    int saw_offer;
    int loop_depth;
} Checker;

static int max_call_args(void) {
#ifdef _WIN32
    return 4;
#else
    return 6;
#endif
}

static void fn_push(Checker *c, FnSym fn) {
    if (c->fn_len == c->fn_cap) {
        size_t next = c->fn_cap == 0 ? 16 : c->fn_cap * 2;
        c->fns = xrealloc(c->fns, next * sizeof(FnSym));
        c->fn_cap = next;
    }
    c->fns[c->fn_len++] = fn;
}

static void var_push(Checker *c, VarSym var) {
    if (c->var_len == c->var_cap) {
        size_t next = c->var_cap == 0 ? 32 : c->var_cap * 2;
        c->vars = xrealloc(c->vars, next * sizeof(VarSym));
        c->var_cap = next;
    }
    c->vars[c->var_len++] = var;
}

static FnSym *find_fn(Checker *c, const char *name) {
    for (size_t i = 0; i < c->fn_len; i++) {
        if (strcmp(c->fns[i].name, name) == 0) {
            return &c->fns[i];
        }
    }
    return NULL;
}

static VarSym *find_var(Checker *c, const char *name) {
    for (size_t i = c->var_len; i > 0; i--) {
        VarSym *v = &c->vars[i - 1];
        if (strcmp(v->name, name) == 0) {
            return v;
        }
    }
    return NULL;
}

static void begin_scope(Checker *c) {
    c->depth++;
}

static void end_scope(Checker *c) {
    while (c->var_len > 0 && c->vars[c->var_len - 1].depth == c->depth) {
        c->var_len--;
    }
    c->depth--;
}

static void define_var(Checker *c, const char *name, TypeKind type, int mutable_flag, int line, int col) {
    for (size_t i = c->var_len; i > 0; i--) {
        VarSym *v = &c->vars[i - 1];
        if (v->depth != c->depth) {
            break;
        }
        if (strcmp(v->name, name) == 0) {
            fatal_at(c->file, line, col, "'%s' already declared in this scope", name);
        }
    }

    VarSym v;
    v.name = (char *)name;
    v.type = type;
    v.mutable_flag = mutable_flag;
    v.depth = c->depth;
    var_push(c, v);
}

static TypeKind check_expr(Checker *c, Expr *e);

static void require_type(Checker *c, Expr *e, TypeKind got, TypeKind expected, const char *what) {
    if (got != expected) {
        fatal_at(c->file, e->line, e->col, "%s expects %s, got %s", what, type_name(expected), type_name(got));
    }
}

static TypeKind check_call(Checker *c, Expr *e) {
    FnSym *fn = find_fn(c, e->as.call.name);
    if (!fn) {
        fatal_at(c->file, e->line, e->col, "unknown glyph '%s'", e->as.call.name);
    }

    if (e->as.call.args.len > (size_t)max_call_args()) {
        fatal_at(c->file, e->line, e->col, "glyph calls currently support at most %d arguments on this target", max_call_args());
    }

    if (fn->params.len != e->as.call.args.len) {
        fatal_at(c->file, e->line, e->col,
                 "glyph '%s' expects %zu arguments, got %zu",
                 e->as.call.name, fn->params.len, e->as.call.args.len);
    }

    for (size_t i = 0; i < e->as.call.args.len; i++) {
        TypeKind arg_t = check_expr(c, e->as.call.args.items[i]);
        TypeKind exp_t = fn->params.items[i].type;
        if (arg_t != exp_t) {
            fatal_at(c->file,
                     e->as.call.args.items[i]->line,
                     e->as.call.args.items[i]->col,
                     "argument %zu of '%s' expects %s, got %s",
                     i + 1,
                     e->as.call.name,
                     type_name(exp_t),
                     type_name(arg_t));
        }
    }

    return fn->ret;
}

static TypeKind check_expr(Checker *c, Expr *e) {
    TypeKind t = TYPE_ERROR;
    switch (e->kind) {
        case EXPR_INT:
            t = TYPE_INT;
            break;
        case EXPR_BOOL:
            t = TYPE_BOOL;
            break;
        case EXPR_STRING:
            t = TYPE_STRING;
            break;
        case EXPR_VAR: {
            VarSym *v = find_var(c, e->as.var_name);
            if (!v) {
                fatal_at(c->file, e->line, e->col, "unknown symbol '%s'", e->as.var_name);
            }
            t = v->type;
            break;
        }
        case EXPR_CALL:
            t = check_call(c, e);
            break;
        case EXPR_UNARY: {
            TypeKind inner = check_expr(c, e->as.unary.operand);
            if (e->as.unary.op == UN_NEG) {
                require_type(c, e->as.unary.operand, inner, TYPE_INT, "negation");
                t = TYPE_INT;
            } else {
                require_type(c, e->as.unary.operand, inner, TYPE_BOOL, "flip");
                t = TYPE_BOOL;
            }
            break;
        }
        case EXPR_BINARY: {
            TypeKind lt = check_expr(c, e->as.binary.left);
            TypeKind rt = check_expr(c, e->as.binary.right);
            switch (e->as.binary.op) {
                case BIN_ADD:
                case BIN_SUB:
                case BIN_MUL:
                case BIN_DIV:
                    if (lt != TYPE_INT || rt != TYPE_INT) {
                        fatal_at(c->file, e->line, e->col, "arithmetic needs ember operands");
                    }
                    t = TYPE_INT;
                    break;
                case BIN_BOTH:
                case BIN_EITHER:
                    if (lt != TYPE_BOOL || rt != TYPE_BOOL) {
                        fatal_at(c->file, e->line, e->col, "boolean chaining needs pulse operands");
                    }
                    t = TYPE_BOOL;
                    break;
                case BIN_LESS:
                case BIN_MORE:
                case BIN_ATMOST:
                case BIN_ATLEAST:
                    if (lt != TYPE_INT || rt != TYPE_INT) {
                        fatal_at(c->file, e->line, e->col, "comparison needs ember operands");
                    }
                    t = TYPE_BOOL;
                    break;
                case BIN_SAME:
                case BIN_DIFF:
                    if (lt != rt) {
                        fatal_at(c->file, e->line, e->col, "same/diff operands must share type");
                    }
                    t = TYPE_BOOL;
                    break;
            }
            break;
        }
    }
    e->inferred_type = t;
    return t;
}

static void check_block(Checker *c, Block *block);

static void check_stmt(Checker *c, Stmt *s) {
    switch (s->kind) {
        case STMT_BIND: {
            TypeKind t = check_expr(c, s->as.bind.value);
            define_var(c, s->as.bind.name, t, 0, s->line, s->col);
            break;
        }
        case STMT_MORPH: {
            TypeKind t = check_expr(c, s->as.morph.value);
            define_var(c, s->as.morph.name, t, 1, s->line, s->col);
            break;
        }
        case STMT_SHIFT: {
            VarSym *v = find_var(c, s->as.shift.name);
            if (!v) {
                fatal_at(c->file, s->line, s->col, "unknown symbol '%s'", s->as.shift.name);
            }
            if (!v->mutable_flag) {
                fatal_at(c->file, s->line, s->col, "cannot shift immutable symbol '%s'", s->as.shift.name);
            }
            TypeKind t = check_expr(c, s->as.shift.value);
            if (t != v->type) {
                fatal_at(c->file, s->line, s->col,
                         "shift type mismatch for '%s': expected %s, got %s",
                         s->as.shift.name, type_name(v->type), type_name(t));
            }
            break;
        }
        case STMT_FORK: {
            TypeKind cond = check_expr(c, s->as.fork.cond);
            if (cond != TYPE_BOOL) {
                fatal_at(c->file, s->line, s->col, "fork condition must be pulse");
            }
            begin_scope(c);
            check_block(c, s->as.fork.then_block);
            end_scope(c);

            if (s->as.fork.else_block) {
                begin_scope(c);
                check_block(c, s->as.fork.else_block);
                end_scope(c);
            }
            break;
        }
        case STMT_CYCLE: {
            TypeKind cond = check_expr(c, s->as.cycle.cond);
            if (cond != TYPE_BOOL) {
                fatal_at(c->file, s->line, s->col, "cycle condition must be pulse");
            }
            c->loop_depth++;
            begin_scope(c);
            check_block(c, s->as.cycle.body);
            end_scope(c);
            c->loop_depth--;
            break;
        }
        case STMT_BREAK: {
            if (c->loop_depth <= 0) {
                fatal_at(c->file, s->line, s->col, "break can only be used inside cycle");
            }
            break;
        }
        case STMT_CONTINUE: {
            if (c->loop_depth <= 0) {
                fatal_at(c->file, s->line, s->col, "continue can only be used inside cycle");
            }
            break;
        }
        case STMT_OFFER: {
            c->saw_offer = 1;
            if (c->current_fn->ret == TYPE_VOID) {
                if (s->as.offer.value) {
                    fatal_at(c->file, s->line, s->col, "mist glyph cannot offer a value");
                }
            } else {
                if (!s->as.offer.value) {
                    fatal_at(c->file, s->line, s->col, "glyph must offer %s value", type_name(c->current_fn->ret));
                }
                TypeKind t = check_expr(c, s->as.offer.value);
                if (t != c->current_fn->ret) {
                    fatal_at(c->file, s->line, s->col,
                             "offer mismatch: glyph yields %s but offered %s",
                             type_name(c->current_fn->ret), type_name(t));
                }
            }
            break;
        }
        case STMT_CHANT: {
            TypeKind t = check_expr(c, s->as.chant.value);
            if (t != TYPE_INT && t != TYPE_BOOL && t != TYPE_STRING) {
                fatal_at(c->file, s->line, s->col, "chant supports ember|pulse|text");
            }
            break;
        }
        case STMT_EXPR: {
            check_expr(c, s->as.expr.value);
            break;
        }
    }
}

static void check_block(Checker *c, Block *block) {
    for (size_t i = 0; i < block->stmts.len; i++) {
        check_stmt(c, block->stmts.items[i]);
    }
}

static void collect_functions(Checker *c) {
    for (size_t i = 0; i < c->program->functions.len; i++) {
        Function *f = &c->program->functions.items[i];
        if (find_fn(c, f->name)) {
            fatal_at(c->file, f->line, f->col, "duplicate glyph '%s'", f->name);
        }
        FnSym sym;
        sym.name = f->name;
        sym.ret = f->return_type;
        sym.params = f->params;
        sym.line = f->line;
        sym.col = f->col;
        fn_push(c, sym);
    }
}

static void check_function(Checker *c, Function *f) {
    c->var_len = 0;
    c->depth = 0;
    c->current_fn = find_fn(c, f->name);
    c->saw_offer = 0;
    c->loop_depth = 0;

    begin_scope(c);
    for (size_t i = 0; i < f->params.len; i++) {
        Param *p = &f->params.items[i];
        define_var(c, p->name, p->type, 0, p->line, p->col);
    }
    check_block(c, f->body);
    end_scope(c);

    if (f->return_type != TYPE_VOID && !c->saw_offer) {
        fatal_at(c->file, f->line, f->col, "glyph '%s' yields %s but has no offer", f->name, type_name(f->return_type));
    }
}

void semantic_check_program(const char *file, Program *program, SemanticResult *out_result) {
    Checker c;
    memset(&c, 0, sizeof(c));
    c.file = file;
    c.program = program;

    collect_functions(&c);

    FnSym *main_fn = find_fn(&c, "main");
    if (!main_fn) {
        fatal("program must define glyph main");
    }
    if (main_fn->params.len != 0) {
        fatal("glyph main must have [] parameter list");
    }
    if (main_fn->ret != TYPE_INT) {
        fatal("glyph main must yield ember");
    }

    for (size_t i = 0; i < program->functions.len; i++) {
        check_function(&c, &program->functions.items[i]);
    }

    free(c.fns);
    free(c.vars);

    out_result->ok = 1;
}
