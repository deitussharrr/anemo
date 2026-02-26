#include "ir.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct ScopeEntry {
    char *name;
    int var_index;
    int depth;
} ScopeEntry;

typedef struct ScopeArray {
    ScopeEntry *items;
    size_t len;
    size_t cap;
} ScopeArray;

typedef struct IRBuilder {
    const Program *ast;
    IRProgram *out;

    IRFunction *fn;
    ScopeArray scope;
    int depth;
    int next_temp;
    int next_label;

    int loop_head_stack[128];
    int loop_end_stack[128];
    int loop_depth;
} IRBuilder;

static int max_call_args(void) {
#ifdef _WIN32
    return 4;
#else
    return 6;
#endif
}

static void grow(void **items, size_t *cap, size_t elem_size) {
    size_t next = *cap == 0 ? 8 : *cap * 2;
    *items = xrealloc(*items, next * elem_size);
    *cap = next;
}

static void push_instr(IRFunction *fn, IRInstr ins) {
    if (fn->code.len == fn->code.cap) {
        grow((void **)&fn->code.items, &fn->code.cap, sizeof(IRInstr));
    }
    fn->code.items[fn->code.len++] = ins;
}

static int add_var(IRFunction *fn, const char *name, TypeKind type, int mutable_flag, int is_param) {
    if (fn->vars.len == fn->vars.cap) {
        grow((void **)&fn->vars.items, &fn->vars.cap, sizeof(IRVar));
    }
    int idx = (int)fn->vars.len;
    IRVar v;
    v.name = xstrdup(name);
    v.type = type;
    v.mutable_flag = mutable_flag;
    v.is_param = is_param;
    fn->vars.items[fn->vars.len++] = v;
    return idx;
}

static void scope_push(IRBuilder *b, const char *name, int var_index) {
    if (b->scope.len == b->scope.cap) {
        grow((void **)&b->scope.items, &b->scope.cap, sizeof(ScopeEntry));
    }
    ScopeEntry e;
    e.name = (char *)name;
    e.var_index = var_index;
    e.depth = b->depth;
    b->scope.items[b->scope.len++] = e;
}

static int scope_find(IRBuilder *b, const char *name) {
    for (size_t i = b->scope.len; i > 0; i--) {
        ScopeEntry *e = &b->scope.items[i - 1];
        if (strcmp(e->name, name) == 0) {
            return e->var_index;
        }
    }
    return -1;
}

static void begin_scope(IRBuilder *b) {
    b->depth++;
}

static void end_scope(IRBuilder *b) {
    while (b->scope.len > 0 && b->scope.items[b->scope.len - 1].depth == b->depth) {
        b->scope.len--;
    }
    b->depth--;
}

static int new_temp(IRBuilder *b) {
    return b->next_temp++;
}

static int new_label(IRBuilder *b) {
    return b->next_label++;
}

static int intern_string(IRProgram *p, const char *value) {
    for (size_t i = 0; i < p->strings.len; i++) {
        if (strcmp(p->strings.items[i].value, value) == 0) {
            return p->strings.items[i].id;
        }
    }
    if (p->strings.len == p->strings.cap) {
        grow((void **)&p->strings.items, &p->strings.cap, sizeof(IRString));
    }
    int id = (int)p->strings.len;
    IRString s;
    s.id = id;
    s.value = xstrdup(value);
    p->strings.items[p->strings.len++] = s;
    return id;
}

static int gen_expr(IRBuilder *b, const Expr *e);

static int emit_load_var(IRBuilder *b, int var_index, int line, int col) {
    int t = new_temp(b);
    IRInstr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = IROP_LOAD_VAR;
    ins.line = line;
    ins.col = col;
    ins.dst = t;
    ins.var_index = var_index;
    push_instr(b->fn, ins);
    return t;
}

static void emit_store_var(IRBuilder *b, int var_index, int src, int line, int col) {
    IRInstr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = IROP_STORE_VAR;
    ins.line = line;
    ins.col = col;
    ins.var_index = var_index;
    ins.src1 = src;
    push_instr(b->fn, ins);
}

static int gen_call(IRBuilder *b, const Expr *e) {
    int arg_temps[6];
    if (e->as.call.args.len > (size_t)max_call_args()) {
        fatal_at("<internal>", e->line, e->col, "codegen currently supports up to %d call arguments on this target", max_call_args());
    }
    for (size_t i = 0; i < e->as.call.args.len; i++) {
        arg_temps[i] = gen_expr(b, e->as.call.args.items[i]);
    }

    IRInstr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = IROP_CALL;
    ins.line = e->line;
    ins.col = e->col;
    ins.name = xstrdup(e->as.call.name);
    ins.argc = (int)e->as.call.args.len;
    for (int i = 0; i < ins.argc; i++) {
        ins.args[i] = arg_temps[i];
    }

    if (e->inferred_type == TYPE_VOID) {
        ins.dst = -1;
        push_instr(b->fn, ins);
        return -1;
    }

    int t = new_temp(b);
    ins.dst = t;
    push_instr(b->fn, ins);
    return t;
}

static int gen_expr(IRBuilder *b, const Expr *e) {
    IRInstr ins;
    memset(&ins, 0, sizeof(ins));

    switch (e->kind) {
        case EXPR_INT: {
            int t = new_temp(b);
            ins.op = IROP_IMM_INT;
            ins.line = e->line;
            ins.col = e->col;
            ins.dst = t;
            ins.imm = e->as.int_value;
            push_instr(b->fn, ins);
            return t;
        }
        case EXPR_BOOL: {
            int t = new_temp(b);
            ins.op = IROP_IMM_BOOL;
            ins.line = e->line;
            ins.col = e->col;
            ins.dst = t;
            ins.imm = e->as.bool_value ? 1 : 0;
            push_instr(b->fn, ins);
            return t;
        }
        case EXPR_STRING: {
            int t = new_temp(b);
            ins.op = IROP_IMM_STR;
            ins.line = e->line;
            ins.col = e->col;
            ins.dst = t;
            ins.imm = intern_string(b->out, e->as.string_value);
            push_instr(b->fn, ins);
            return t;
        }
        case EXPR_VAR: {
            int vi = scope_find(b, e->as.var_name);
            if (vi < 0) {
                fatal_at("<internal>", e->line, e->col, "unknown var in IR gen: %s", e->as.var_name);
            }
            return emit_load_var(b, vi, e->line, e->col);
        }
        case EXPR_CALL:
            return gen_call(b, e);
        case EXPR_UNARY: {
            int src = gen_expr(b, e->as.unary.operand);
            int t = new_temp(b);
            ins.op = IROP_UN;
            ins.line = e->line;
            ins.col = e->col;
            ins.dst = t;
            ins.src1 = src;
            ins.unop = (e->as.unary.op == UN_NEG) ? IRUN_NEG : IRUN_FLIP;
            push_instr(b->fn, ins);
            return t;
        }
        case EXPR_BINARY: {
            int left = gen_expr(b, e->as.binary.left);
            int right = gen_expr(b, e->as.binary.right);
            int t = new_temp(b);
            ins.op = IROP_BIN;
            ins.line = e->line;
            ins.col = e->col;
            ins.dst = t;
            ins.src1 = left;
            ins.src2 = right;
            switch (e->as.binary.op) {
                case BIN_ADD: ins.binop = IRBIN_ADD; break;
                case BIN_SUB: ins.binop = IRBIN_SUB; break;
                case BIN_MUL: ins.binop = IRBIN_MUL; break;
                case BIN_DIV: ins.binop = IRBIN_DIV; break;
                case BIN_BOTH: ins.binop = IRBIN_BOTH; break;
                case BIN_EITHER: ins.binop = IRBIN_EITHER; break;
                case BIN_SAME: ins.binop = IRBIN_SAME; break;
                case BIN_DIFF: ins.binop = IRBIN_DIFF; break;
                case BIN_LESS: ins.binop = IRBIN_LESS; break;
                case BIN_MORE: ins.binop = IRBIN_MORE; break;
                case BIN_ATMOST: ins.binop = IRBIN_ATMOST; break;
                case BIN_ATLEAST: ins.binop = IRBIN_ATLEAST; break;
            }
            push_instr(b->fn, ins);
            return t;
        }
    }

    return -1;
}

static void emit_label(IRBuilder *b, int label) {
    IRInstr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = IROP_LABEL;
    ins.label = label;
    push_instr(b->fn, ins);
}

static void emit_jmp(IRBuilder *b, int label) {
    IRInstr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = IROP_JMP;
    ins.label = label;
    push_instr(b->fn, ins);
}

static void emit_jmp_false(IRBuilder *b, int cond_temp, int label) {
    IRInstr ins;
    memset(&ins, 0, sizeof(ins));
    ins.op = IROP_JMP_FALSE;
    ins.src1 = cond_temp;
    ins.label = label;
    push_instr(b->fn, ins);
}

static void gen_stmt(IRBuilder *b, const Stmt *s);

static void gen_block(IRBuilder *b, const Block *block) {
    for (size_t i = 0; i < block->stmts.len; i++) {
        gen_stmt(b, block->stmts.items[i]);
    }
}

static void gen_stmt(IRBuilder *b, const Stmt *s) {
    switch (s->kind) {
        case STMT_BIND: {
            int src = gen_expr(b, s->as.bind.value);
            int var = add_var(b->fn, s->as.bind.name, s->as.bind.value->inferred_type, 0, 0);
            scope_push(b, s->as.bind.name, var);
            emit_store_var(b, var, src, s->line, s->col);
            break;
        }
        case STMT_MORPH: {
            int src = gen_expr(b, s->as.morph.value);
            int var = add_var(b->fn, s->as.morph.name, s->as.morph.value->inferred_type, 1, 0);
            scope_push(b, s->as.morph.name, var);
            emit_store_var(b, var, src, s->line, s->col);
            break;
        }
        case STMT_SHIFT: {
            int var = scope_find(b, s->as.shift.name);
            int src = gen_expr(b, s->as.shift.value);
            emit_store_var(b, var, src, s->line, s->col);
            break;
        }
        case STMT_FORK: {
            int cond = gen_expr(b, s->as.fork.cond);
            int l_else = new_label(b);
            int l_end = new_label(b);
            emit_jmp_false(b, cond, l_else);

            begin_scope(b);
            gen_block(b, s->as.fork.then_block);
            end_scope(b);
            emit_jmp(b, l_end);

            emit_label(b, l_else);
            if (s->as.fork.else_block) {
                begin_scope(b);
                gen_block(b, s->as.fork.else_block);
                end_scope(b);
            }
            emit_label(b, l_end);
            break;
        }
        case STMT_CYCLE: {
            int l_head = new_label(b);
            int l_end = new_label(b);
            if (b->loop_depth >= 128) {
                fatal_at("<internal>", s->line, s->col, "loop nesting too deep");
            }
            b->loop_head_stack[b->loop_depth] = l_head;
            b->loop_end_stack[b->loop_depth] = l_end;
            b->loop_depth++;

            emit_label(b, l_head);
            int cond = gen_expr(b, s->as.cycle.cond);
            emit_jmp_false(b, cond, l_end);

            begin_scope(b);
            gen_block(b, s->as.cycle.body);
            end_scope(b);
            emit_jmp(b, l_head);
            emit_label(b, l_end);
            b->loop_depth--;
            break;
        }
        case STMT_BREAK: {
            if (b->loop_depth <= 0) {
                fatal_at("<internal>", s->line, s->col, "break used outside loop during IR gen");
            }
            emit_jmp(b, b->loop_end_stack[b->loop_depth - 1]);
            break;
        }
        case STMT_CONTINUE: {
            if (b->loop_depth <= 0) {
                fatal_at("<internal>", s->line, s->col, "continue used outside loop during IR gen");
            }
            emit_jmp(b, b->loop_head_stack[b->loop_depth - 1]);
            break;
        }
        case STMT_OFFER: {
            IRInstr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = IROP_RET;
            ins.line = s->line;
            ins.col = s->col;
            if (s->as.offer.value) {
                ins.has_value = 1;
                ins.src1 = gen_expr(b, s->as.offer.value);
            }
            push_instr(b->fn, ins);
            break;
        }
        case STMT_CHANT: {
            IRInstr ins;
            memset(&ins, 0, sizeof(ins));
            ins.op = IROP_CHANT;
            ins.line = s->line;
            ins.col = s->col;
            ins.src1 = gen_expr(b, s->as.chant.value);
            ins.type = s->as.chant.value->inferred_type;
            push_instr(b->fn, ins);
            break;
        }
        case STMT_EXPR: {
            (void)gen_expr(b, s->as.expr.value);
            break;
        }
    }
}

static void gen_function(IRBuilder *b, const Function *f) {
    IRFunction fn;
    memset(&fn, 0, sizeof(fn));
    fn.name = xstrdup(f->name);
    fn.return_type = f->return_type;

    b->fn = &fn;
    b->scope.len = 0;
    b->depth = 0;
    b->next_temp = 0;
    b->next_label = 0;
    b->loop_depth = 0;

    begin_scope(b);
    for (size_t i = 0; i < f->params.len; i++) {
        Param p = f->params.items[i];
        int vi = add_var(&fn, p.name, p.type, 0, 1);
        scope_push(b, p.name, vi);
    }
    fn.param_count = (int)f->params.len;

    gen_block(b, f->body);
    end_scope(b);

    if (fn.return_type == TYPE_VOID) {
        IRInstr ins;
        memset(&ins, 0, sizeof(ins));
        ins.op = IROP_RET;
        ins.has_value = 0;
        push_instr(&fn, ins);
    }

    fn.temp_count = b->next_temp;

    if (b->out->functions.len == b->out->functions.cap) {
        grow((void **)&b->out->functions.items, &b->out->functions.cap, sizeof(IRFunction));
    }
    b->out->functions.items[b->out->functions.len++] = fn;
}

void ir_generate_program(const Program *ast, IRProgram *out_ir) {
    memset(out_ir, 0, sizeof(*out_ir));
    IRBuilder b;
    memset(&b, 0, sizeof(b));
    b.ast = ast;
    b.out = out_ir;

    for (size_t i = 0; i < ast->functions.len; i++) {
        gen_function(&b, &ast->functions.items[i]);
    }

    free(b.scope.items);
}

void free_ir_program(IRProgram *ir) {
    if (!ir) {
        return;
    }
    for (size_t i = 0; i < ir->functions.len; i++) {
        IRFunction *fn = &ir->functions.items[i];
        free(fn->name);
        for (size_t v = 0; v < fn->vars.len; v++) {
            free(fn->vars.items[v].name);
        }
        for (size_t j = 0; j < fn->code.len; j++) {
            free(fn->code.items[j].name);
        }
        free(fn->vars.items);
        free(fn->code.items);
    }
    for (size_t i = 0; i < ir->strings.len; i++) {
        free(ir->strings.items[i].value);
    }
    free(ir->functions.items);
    free(ir->strings.items);
    memset(ir, 0, sizeof(*ir));
}
