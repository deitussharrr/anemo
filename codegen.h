#ifndef CODEGEN_H
#define CODEGEN_H

#include "ir.h"

void codegen_emit_assembly(const IRProgram *ir, const char *asm_path);

#endif