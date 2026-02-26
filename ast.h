#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef enum TypeKind {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID,
    TYPE_ERROR
} TypeKind;

typedef enum ExprKind {
    EXPR_INT,
    EXPR_BOOL,
    EXPR_STRING,
    EXPR_VAR,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_CALL
} ExprKind;

typedef enum UnaryOp {
    UN_NEG,
    UN_FLIP
} UnaryOp;

typedef enum BinaryOp {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_BOTH,
    BIN_EITHER,
    BIN_SAME,
    BIN_DIFF,
    BIN_LESS,
    BIN_MORE,
    BIN_ATMOST,
    BIN_ATLEAST
} BinaryOp;

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Block Block;

typedef struct ExprArray {
    Expr **items;
    size_t len;
    size_t cap;
} ExprArray;

struct Expr {
    ExprKind kind;
    int line;
    int col;
    TypeKind inferred_type;
    union {
        long int_value;
        int bool_value;
        char *string_value;
        char *var_name;
        struct {
            UnaryOp op;
            Expr *operand;
        } unary;
        struct {
            BinaryOp op;
            Expr *left;
            Expr *right;
        } binary;
        struct {
            char *name;
            ExprArray args;
        } call;
    } as;
};

typedef enum StmtKind {
    STMT_BIND,
    STMT_MORPH,
    STMT_SHIFT,
    STMT_FORK,
    STMT_CYCLE,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_OFFER,
    STMT_CHANT,
    STMT_EXPR
} StmtKind;

typedef struct StmtArray {
    Stmt **items;
    size_t len;
    size_t cap;
} StmtArray;

struct Block {
    StmtArray stmts;
};

struct Stmt {
    StmtKind kind;
    int line;
    int col;
    union {
        struct {
            char *name;
            Expr *value;
        } bind;
        struct {
            char *name;
            Expr *value;
        } morph;
        struct {
            char *name;
            Expr *value;
        } shift;
        struct {
            Expr *cond;
            Block *then_block;
            Block *else_block;
        } fork;
        struct {
            Expr *cond;
            Block *body;
        } cycle;
        struct {
            Expr *value;
        } offer;
        struct {
            Expr *value;
        } chant;
        struct {
            Expr *value;
        } expr;
    } as;
};

typedef struct Param {
    char *name;
    TypeKind type;
    int line;
    int col;
} Param;

typedef struct ParamArray {
    Param *items;
    size_t len;
    size_t cap;
} ParamArray;

typedef struct Function {
    char *name;
    ParamArray params;
    TypeKind return_type;
    Block *body;
    int line;
    int col;
} Function;

typedef struct FunctionArray {
    Function *items;
    size_t len;
    size_t cap;
} FunctionArray;

typedef struct Program {
    FunctionArray functions;
} Program;

Expr *expr_new(ExprKind kind, int line, int col);
Stmt *stmt_new(StmtKind kind, int line, int col);
Block *block_new(void);

void expr_array_push(ExprArray *arr, Expr *expr);
void stmt_array_push(StmtArray *arr, Stmt *stmt);
void param_array_push(ParamArray *arr, Param param);
void function_array_push(FunctionArray *arr, Function fn);

void free_program(Program *program);

const char *type_name(TypeKind t);

#endif
