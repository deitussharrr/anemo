#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

void parse_program(const char *file, const TokenArray *tokens, Program *out_program);

#endif