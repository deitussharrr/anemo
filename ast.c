#include "ast.h"

#include "utils.h"

#include <stdlib.h>
#include <string.h>

Expr *expr_new(ExprKind kind, int line, int col) {
    Expr *e = xcalloc(1, sizeof(Expr));
    e->kind = kind;
    e->line = line;
    e->col = col;
    e->inferred_type = TYPE_ERROR;
    return e;
}

Stmt *stmt_new(StmtKind kind, int line, int col) {
    Stmt *s = xcalloc(1, sizeof(Stmt));
    s->kind = kind;
    s->line = line;
    s->col = col;
    return s;
}

Block *block_new(void) {
    return xcalloc(1, sizeof(Block));
}

static void grow(void **items, size_t *cap, size_t elem_size) {
    size_t next = *cap == 0 ? 8 : (*cap * 2);
    *items = xrealloc(*items, next * elem_size);
    *cap = next;
}

void expr_array_push(ExprArray *arr, Expr *expr) {
    if (arr->len == arr->cap) {
        grow((void **)&arr->items, &arr->cap, sizeof(Expr *));
    }
    arr->items[arr->len++] = expr;
}

void stmt_array_push(StmtArray *arr, Stmt *stmt) {
    if (arr->len == arr->cap) {
        grow((void **)&arr->items, &arr->cap, sizeof(Stmt *));
    }
    arr->items[arr->len++] = stmt;
}

void param_array_push(ParamArray *arr, Param param) {
    if (arr->len == arr->cap) {
        grow((void **)&arr->items, &arr->cap, sizeof(Param));
    }
    arr->items[arr->len++] = param;
}

void function_array_push(FunctionArray *arr, Function fn) {
    if (arr->len == arr->cap) {
        grow((void **)&arr->items, &arr->cap, sizeof(Function));
    }
    arr->items[arr->len++] = fn;
}

static void free_expr(Expr *expr);
static void free_stmt(Stmt *stmt);

static void free_block(Block *block) {
    if (!block) {
        return;
    }
    for (size_t i = 0; i < block->stmts.len; i++) {
        free_stmt(block->stmts.items[i]);
    }
    free(block->stmts.items);
    free(block);
}

static void free_expr(Expr *expr) {
    if (!expr) {
        return;
    }
    switch (expr->kind) {
        case EXPR_STRING:
            free(expr->as.string_value);
            break;
        case EXPR_VAR:
            free(expr->as.var_name);
            break;
        case EXPR_UNARY:
            free_expr(expr->as.unary.operand);
            break;
        case EXPR_BINARY:
            free_expr(expr->as.binary.left);
            free_expr(expr->as.binary.right);
            break;
        case EXPR_CALL:
            free(expr->as.call.name);
            for (size_t i = 0; i < expr->as.call.args.len; i++) {
                free_expr(expr->as.call.args.items[i]);
            }
            free(expr->as.call.args.items);
            break;
        case EXPR_INT:
        case EXPR_BOOL:
            break;
    }
    free(expr);
}

static void free_stmt(Stmt *stmt) {
    if (!stmt) {
        return;
    }
    switch (stmt->kind) {
        case STMT_BIND:
            free(stmt->as.bind.name);
            free_expr(stmt->as.bind.value);
            break;
        case STMT_MORPH:
            free(stmt->as.morph.name);
            free_expr(stmt->as.morph.value);
            break;
        case STMT_SHIFT:
            free(stmt->as.shift.name);
            free_expr(stmt->as.shift.value);
            break;
        case STMT_FORK:
            free_expr(stmt->as.fork.cond);
            free_block(stmt->as.fork.then_block);
            free_block(stmt->as.fork.else_block);
            break;
        case STMT_CYCLE:
            free_expr(stmt->as.cycle.cond);
            free_block(stmt->as.cycle.body);
            break;
        case STMT_OFFER:
            free_expr(stmt->as.offer.value);
            break;
        case STMT_CHANT:
            free_expr(stmt->as.chant.value);
            break;
        case STMT_EXPR:
            free_expr(stmt->as.expr.value);
            break;
    }
    free(stmt);
}

void free_program(Program *program) {
    if (!program) {
        return;
    }
    for (size_t i = 0; i < program->functions.len; i++) {
        Function *fn = &program->functions.items[i];
        free(fn->name);
        for (size_t p = 0; p < fn->params.len; p++) {
            free(fn->params.items[p].name);
        }
        free(fn->params.items);
        free_block(fn->body);
    }
    free(program->functions.items);
    memset(program, 0, sizeof(*program));
}

const char *type_name(TypeKind t) {
    switch (t) {
        case TYPE_INT: return "ember";
        case TYPE_BOOL: return "pulse";
        case TYPE_STRING: return "text";
        case TYPE_VOID: return "mist";
        case TYPE_ERROR: return "<error>";
    }
    return "<unknown-type>";
}