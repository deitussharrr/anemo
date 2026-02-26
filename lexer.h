#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum TokenKind {
    TOK_EOF = 0,
    TOK_NEWLINE,

    TOK_IDENT,
    TOK_INT,
    TOK_STRING,

    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_ASSIGN,
    TOK_COMMA,
    TOK_COLON,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_LPAREN,
    TOK_RPAREN,

    TOK_K_GLYPH,
    TOK_K_YIELDS,
    TOK_K_BIND,
    TOK_K_MORPH,
    TOK_K_SHIFT,
    TOK_K_FORK,
    TOK_K_ELSEIF,
    TOK_K_OTHERWISE,
    TOK_K_CYCLE,
    TOK_K_BREAK,
    TOK_K_CONTINUE,
    TOK_K_OFFER,
    TOK_K_INVOKE,
    TOK_K_WITH,
    TOK_K_CHANT,
    TOK_K_SEAL,

    TOK_K_EMBER,
    TOK_K_PULSE,
    TOK_K_TEXT,
    TOK_K_MIST,
    TOK_K_YES,
    TOK_K_NO,

    TOK_K_BOTH,
    TOK_K_EITHER,
    TOK_K_FLIP,
    TOK_K_SAME,
    TOK_K_DIFF,
    TOK_K_LESS,
    TOK_K_MORE,
    TOK_K_ATMOST,
    TOK_K_ATLEAST
} TokenKind;

typedef struct Token {
    TokenKind kind;
    char *lexeme;
    long int_value;
    int line;
    int col;
} Token;

typedef struct TokenArray {
    Token *data;
    size_t len;
    size_t cap;
} TokenArray;

void lex_source(const char *file, const char *src, TokenArray *out_tokens);
void free_tokens(TokenArray *tokens);
const char *token_kind_name(TokenKind kind);

#endif
