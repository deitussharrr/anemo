#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

typedef struct SemanticResult {
    int ok;
} SemanticResult;

void semantic_check_program(const char *file, Program *program, SemanticResult *out_result);

#endif