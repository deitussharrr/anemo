#ifndef IR_H
#define IR_H

#include "ast.h"

#include <stddef.h>

typedef enum IRBinOp {
    IRBIN_ADD,
    IRBIN_SUB,
    IRBIN_MUL,
    IRBIN_DIV,
    IRBIN_BOTH,
    IRBIN_EITHER,
    IRBIN_SAME,
    IRBIN_DIFF,
    IRBIN_LESS,
    IRBIN_MORE,
    IRBIN_ATMOST,
    IRBIN_ATLEAST
} IRBinOp;

typedef enum IRUnOp {
    IRUN_NEG,
    IRUN_FLIP
} IRUnOp;

typedef enum IROp {
    IROP_LABEL,
    IROP_JMP,
    IROP_JMP_FALSE,

    IROP_IMM_INT,
    IROP_IMM_BOOL,
    IROP_IMM_STR,
    IROP_LOAD_VAR,
    IROP_STORE_VAR,

    IROP_BIN,
    IROP_UN,

    IROP_CALL,
    IROP_CHANT,
    IROP_RET
} IROp;

typedef struct IRInstr {
    IROp op;
    int line;
    int col;

    int dst;
    int src1;
    int src2;
    long imm;

    int var_index;
    int label;
    int label2;

    IRBinOp binop;
    IRUnOp unop;

    char *name;
    int argc;
    int args[6];

    TypeKind type;
    int has_value;
} IRInstr;

typedef struct IRInstrArray {
    IRInstr *items;
    size_t len;
    size_t cap;
} IRInstrArray;

typedef struct IRVar {
    char *name;
    TypeKind type;
    int mutable_flag;
    int is_param;
} IRVar;

typedef struct IRVarArray {
    IRVar *items;
    size_t len;
    size_t cap;
} IRVarArray;

typedef struct IRFunction {
    char *name;
    TypeKind return_type;
    IRVarArray vars;
    int param_count;
    int temp_count;
    IRInstrArray code;
} IRFunction;

typedef struct IRFunctionArray {
    IRFunction *items;
    size_t len;
    size_t cap;
} IRFunctionArray;

typedef struct IRString {
    int id;
    char *value;
} IRString;

typedef struct IRStringArray {
    IRString *items;
    size_t len;
    size_t cap;
} IRStringArray;

typedef struct IRProgram {
    IRFunctionArray functions;
    IRStringArray strings;
} IRProgram;

void ir_generate_program(const Program *ast, IRProgram *out_ir);
void free_ir_program(IRProgram *ir);

#endif