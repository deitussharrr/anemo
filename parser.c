#include "parser.h"

#include "utils.h"

#include <string.h>

typedef struct Parser {
    const char *file;
    const TokenArray *tokens;
    size_t pos;
} Parser;

static const Token *peek(Parser *p) {
    return &p->tokens->data[p->pos];
}

static const Token *prev(Parser *p) {
    return &p->tokens->data[p->pos - 1];
}

static int is_at_end(Parser *p) {
    return peek(p)->kind == TOK_EOF;
}

static const Token *advance(Parser *p) {
    if (!is_at_end(p)) {
        p->pos++;
    }
    return prev(p);
}

static int check(Parser *p, TokenKind kind) {
    return peek(p)->kind == kind;
}

static int match(Parser *p, TokenKind kind) {
    if (!check(p, kind)) {
        return 0;
    }
    advance(p);
    return 1;
}

static const Token *expect(Parser *p, TokenKind kind, const char *message) {
    if (check(p, kind)) {
        return advance(p);
    }
    const Token *t = peek(p);
    fatal_at(p->file, t->line, t->col, "%s (found %s)", message, token_kind_name(t->kind));
    return NULL;
}

static void skip_newlines(Parser *p) {
    while (match(p, TOK_NEWLINE)) {
    }
}

static TypeKind parse_type(Parser *p) {
    const Token *t = peek(p);
    if (match(p, TOK_K_EMBER)) return TYPE_INT;
    if (match(p, TOK_K_PULSE)) return TYPE_BOOL;
    if (match(p, TOK_K_TEXT)) return TYPE_STRING;
    if (match(p, TOK_K_MIST)) return TYPE_VOID;
    fatal_at(p->file, t->line, t->col, "expected type keyword ember|pulse|text|mist");
    return TYPE_ERROR;
}

static Expr *parse_expr(Parser *p);
static Block *parse_block_until(Parser *p, TokenKind end_a, TokenKind end_b);

static Expr *parse_call(Parser *p) {
    const Token *kw = expect(p, TOK_K_INVOKE, "expected invoke");
    const Token *name_tok = expect(p, TOK_IDENT, "expected function name after invoke");

    Expr *call = expr_new(EXPR_CALL, kw->line, kw->col);
    call->as.call.name = xstrdup(name_tok->lexeme);

    if (match(p, TOK_K_WITH)) {
        Expr *arg = parse_expr(p);
        expr_array_push(&call->as.call.args, arg);
        while (match(p, TOK_COMMA)) {
            expr_array_push(&call->as.call.args, parse_expr(p));
        }
    }
    return call;
}

static Expr *parse_primary(Parser *p) {
    const Token *t = peek(p);

    if (match(p, TOK_INT)) {
        Expr *e = expr_new(EXPR_INT, t->line, t->col);
        e->as.int_value = t->int_value;
        return e;
    }
    if (match(p, TOK_STRING)) {
        Expr *e = expr_new(EXPR_STRING, t->line, t->col);
        e->as.string_value = xstrdup(t->lexeme);
        return e;
    }
    if (match(p, TOK_K_YES)) {
        Expr *e = expr_new(EXPR_BOOL, t->line, t->col);
        e->as.bool_value = 1;
        return e;
    }
    if (match(p, TOK_K_NO)) {
        Expr *e = expr_new(EXPR_BOOL, t->line, t->col);
        e->as.bool_value = 0;
        return e;
    }
    if (check(p, TOK_K_INVOKE)) {
        return parse_call(p);
    }
    if (match(p, TOK_IDENT)) {
        Expr *e = expr_new(EXPR_VAR, t->line, t->col);
        e->as.var_name = xstrdup(t->lexeme);
        return e;
    }

    fatal_at(p->file, t->line, t->col, "expected expression");
    return NULL;
}

static Expr *parse_unary(Parser *p) {
    if (match(p, TOK_MINUS)) {
        const Token *op = prev(p);
        Expr *e = expr_new(EXPR_UNARY, op->line, op->col);
        e->as.unary.op = UN_NEG;
        e->as.unary.operand = parse_unary(p);
        return e;
    }
    if (match(p, TOK_K_FLIP)) {
        const Token *op = prev(p);
        Expr *e = expr_new(EXPR_UNARY, op->line, op->col);
        e->as.unary.op = UN_FLIP;
        e->as.unary.operand = parse_unary(p);
        return e;
    }
    return parse_primary(p);
}

static Expr *parse_mul(Parser *p) {
    Expr *expr = parse_unary(p);
    while (check(p, TOK_STAR) || check(p, TOK_SLASH)) {
        const Token *op = advance(p);
        Expr *rhs = parse_unary(p);
        Expr *bin = expr_new(EXPR_BINARY, op->line, op->col);
        bin->as.binary.left = expr;
        bin->as.binary.right = rhs;
        bin->as.binary.op = (op->kind == TOK_STAR) ? BIN_MUL : BIN_DIV;
        expr = bin;
    }
    return expr;
}

static Expr *parse_add(Parser *p) {
    Expr *expr = parse_mul(p);
    while (check(p, TOK_PLUS) || check(p, TOK_MINUS)) {
        const Token *op = advance(p);
        Expr *rhs = parse_mul(p);
        Expr *bin = expr_new(EXPR_BINARY, op->line, op->col);
        bin->as.binary.left = expr;
        bin->as.binary.right = rhs;
        bin->as.binary.op = (op->kind == TOK_PLUS) ? BIN_ADD : BIN_SUB;
        expr = bin;
    }
    return expr;
}

static Expr *parse_cmp(Parser *p) {
    Expr *expr = parse_add(p);
    while (check(p, TOK_K_LESS) || check(p, TOK_K_MORE) || check(p, TOK_K_ATMOST) || check(p, TOK_K_ATLEAST)) {
        const Token *op = advance(p);
        Expr *rhs = parse_add(p);
        Expr *bin = expr_new(EXPR_BINARY, op->line, op->col);
        bin->as.binary.left = expr;
        bin->as.binary.right = rhs;
        switch (op->kind) {
            case TOK_K_LESS: bin->as.binary.op = BIN_LESS; break;
            case TOK_K_MORE: bin->as.binary.op = BIN_MORE; break;
            case TOK_K_ATMOST: bin->as.binary.op = BIN_ATMOST; break;
            case TOK_K_ATLEAST: bin->as.binary.op = BIN_ATLEAST; break;
            default: break;
        }
        expr = bin;
    }
    return expr;
}

static Expr *parse_eq(Parser *p) {
    Expr *expr = parse_cmp(p);
    while (check(p, TOK_K_SAME) || check(p, TOK_K_DIFF)) {
        const Token *op = advance(p);
        Expr *rhs = parse_cmp(p);
        Expr *bin = expr_new(EXPR_BINARY, op->line, op->col);
        bin->as.binary.left = expr;
        bin->as.binary.right = rhs;
        bin->as.binary.op = (op->kind == TOK_K_SAME) ? BIN_SAME : BIN_DIFF;
        expr = bin;
    }
    return expr;
}

static Expr *parse_both(Parser *p) {
    Expr *expr = parse_eq(p);
    while (match(p, TOK_K_BOTH)) {
        const Token *op = prev(p);
        Expr *rhs = parse_eq(p);
        Expr *bin = expr_new(EXPR_BINARY, op->line, op->col);
        bin->as.binary.left = expr;
        bin->as.binary.right = rhs;
        bin->as.binary.op = BIN_BOTH;
        expr = bin;
    }
    return expr;
}

static Expr *parse_either(Parser *p) {
    Expr *expr = parse_both(p);
    while (match(p, TOK_K_EITHER)) {
        const Token *op = prev(p);
        Expr *rhs = parse_both(p);
        Expr *bin = expr_new(EXPR_BINARY, op->line, op->col);
        bin->as.binary.left = expr;
        bin->as.binary.right = rhs;
        bin->as.binary.op = BIN_EITHER;
        expr = bin;
    }
    return expr;
}

static Expr *parse_expr(Parser *p) {
    return parse_either(p);
}

static void expect_line_end(Parser *p) {
    if (match(p, TOK_NEWLINE)) {
        while (match(p, TOK_NEWLINE)) {
        }
        return;
    }
    if (check(p, TOK_EOF) || check(p, TOK_K_SEAL) || check(p, TOK_K_OTHERWISE)) {
        return;
    }
    const Token *t = peek(p);
    fatal_at(p->file, t->line, t->col, "expected newline");
}

static Stmt *parse_stmt(Parser *p) {
    const Token *t = peek(p);

    if (match(p, TOK_K_BIND)) {
        Stmt *s = stmt_new(STMT_BIND, t->line, t->col);
        const Token *name = expect(p, TOK_IDENT, "expected identifier after bind");
        s->as.bind.name = xstrdup(name->lexeme);
        expect(p, TOK_ASSIGN, "expected '=' in bind statement");
        s->as.bind.value = parse_expr(p);
        expect_line_end(p);
        return s;
    }

    if (match(p, TOK_K_MORPH)) {
        Stmt *s = stmt_new(STMT_MORPH, t->line, t->col);
        const Token *name = expect(p, TOK_IDENT, "expected identifier after morph");
        s->as.morph.name = xstrdup(name->lexeme);
        expect(p, TOK_ASSIGN, "expected '=' in morph statement");
        s->as.morph.value = parse_expr(p);
        expect_line_end(p);
        return s;
    }

    if (match(p, TOK_K_SHIFT)) {
        Stmt *s = stmt_new(STMT_SHIFT, t->line, t->col);
        const Token *name = expect(p, TOK_IDENT, "expected identifier after shift");
        s->as.shift.name = xstrdup(name->lexeme);
        expect(p, TOK_ASSIGN, "expected '=' in shift statement");
        s->as.shift.value = parse_expr(p);
        expect_line_end(p);
        return s;
    }

    if (match(p, TOK_K_FORK)) {
        Stmt *s = stmt_new(STMT_FORK, t->line, t->col);
        s->as.fork.cond = parse_expr(p);
        expect(p, TOK_NEWLINE, "expected newline after fork condition");
        skip_newlines(p);
        s->as.fork.then_block = parse_block_until(p, TOK_K_OTHERWISE, TOK_K_SEAL);
        if (match(p, TOK_K_OTHERWISE)) {
            expect(p, TOK_NEWLINE, "expected newline after otherwise");
            skip_newlines(p);
            s->as.fork.else_block = parse_block_until(p, TOK_K_SEAL, TOK_K_SEAL);
        }
        expect(p, TOK_K_SEAL, "expected seal to close fork");
        expect_line_end(p);
        return s;
    }

    if (match(p, TOK_K_CYCLE)) {
        Stmt *s = stmt_new(STMT_CYCLE, t->line, t->col);
        s->as.cycle.cond = parse_expr(p);
        expect(p, TOK_NEWLINE, "expected newline after cycle condition");
        skip_newlines(p);
        s->as.cycle.body = parse_block_until(p, TOK_K_SEAL, TOK_K_SEAL);
        expect(p, TOK_K_SEAL, "expected seal to close cycle");
        expect_line_end(p);
        return s;
    }

    if (match(p, TOK_K_OFFER)) {
        Stmt *s = stmt_new(STMT_OFFER, t->line, t->col);
        if (check(p, TOK_NEWLINE) || check(p, TOK_K_SEAL) || check(p, TOK_K_OTHERWISE) || check(p, TOK_EOF)) {
            s->as.offer.value = NULL;
        } else {
            s->as.offer.value = parse_expr(p);
        }
        expect_line_end(p);
        return s;
    }

    if (match(p, TOK_K_CHANT)) {
        Stmt *s = stmt_new(STMT_CHANT, t->line, t->col);
        s->as.chant.value = parse_expr(p);
        expect_line_end(p);
        return s;
    }

    Expr *expr = parse_expr(p);
    Stmt *s = stmt_new(STMT_EXPR, expr->line, expr->col);
    s->as.expr.value = expr;
    expect_line_end(p);
    return s;
}

static Block *parse_block_until(Parser *p, TokenKind end_a, TokenKind end_b) {
    Block *block = block_new();
    while (!check(p, TOK_EOF) && !check(p, end_a) && !check(p, end_b)) {
        if (match(p, TOK_NEWLINE)) {
            continue;
        }
        stmt_array_push(&block->stmts, parse_stmt(p));
    }
    return block;
}

static Function parse_function(Parser *p) {
    const Token *kw = expect(p, TOK_K_GLYPH, "expected glyph");
    const Token *name = expect(p, TOK_IDENT, "expected function name after glyph");

    Function fn;
    memset(&fn, 0, sizeof(fn));
    fn.name = xstrdup(name->lexeme);
    fn.line = kw->line;
    fn.col = kw->col;

    expect(p, TOK_LBRACKET, "expected '[' to start parameter list");
    if (!check(p, TOK_RBRACKET)) {
        for (;;) {
            const Token *pn = expect(p, TOK_IDENT, "expected parameter name");
            expect(p, TOK_COLON, "expected ':' after parameter name");
            TypeKind pt = parse_type(p);

            Param param;
            param.name = xstrdup(pn->lexeme);
            param.type = pt;
            param.line = pn->line;
            param.col = pn->col;
            param_array_push(&fn.params, param);

            if (!match(p, TOK_COMMA)) {
                break;
            }
        }
    }
    expect(p, TOK_RBRACKET, "expected ']' to close parameter list");
    expect(p, TOK_K_YIELDS, "expected yields after parameter list");
    fn.return_type = parse_type(p);
    expect(p, TOK_NEWLINE, "expected newline after function signature");
    skip_newlines(p);

    fn.body = parse_block_until(p, TOK_K_SEAL, TOK_K_SEAL);
    expect(p, TOK_K_SEAL, "expected seal to close function");
    expect_line_end(p);

    return fn;
}

void parse_program(const char *file, const TokenArray *tokens, Program *out_program) {
    Parser p;
    p.file = file;
    p.tokens = tokens;
    p.pos = 0;

    Program program;
    memset(&program, 0, sizeof(program));

    skip_newlines(&p);
    while (!check(&p, TOK_EOF)) {
        function_array_push(&program.functions, parse_function(&p));
        skip_newlines(&p);
    }

    if (program.functions.len == 0) {
        const Token *t = peek(&p);
        fatal_at(file, t->line, t->col, "program must declare at least one glyph");
    }

    *out_program = program;
}