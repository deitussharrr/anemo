#include "lexer.h"

#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Lexer {
    const char *file;
    const char *src;
    size_t pos;
    int line;
    int col;
    TokenArray out;
} Lexer;

static void token_push(TokenArray *arr, Token tok) {
    if (arr->len == arr->cap) {
        size_t next = arr->cap == 0 ? 64 : arr->cap * 2;
        arr->data = xrealloc(arr->data, next * sizeof(Token));
        arr->cap = next;
    }
    arr->data[arr->len++] = tok;
}

static char peek(Lexer *lx) {
    return lx->src[lx->pos];
}

static char bump(Lexer *lx) {
    char c = lx->src[lx->pos++];
    if (c == '\n') {
        lx->line++;
        lx->col = 1;
    } else {
        lx->col++;
    }
    return c;
}

static void emit_simple(Lexer *lx, TokenKind kind, int line, int col) {
    Token t;
    t.kind = kind;
    t.lexeme = NULL;
    t.int_value = 0;
    t.line = line;
    t.col = col;
    token_push(&lx->out, t);
}

static char *slice_dup(const char *src, size_t start, size_t end) {
    size_t n = end - start;
    char *s = xmalloc(n + 1);
    memcpy(s, src + start, n);
    s[n] = '\0';
    return s;
}

static TokenKind keyword_kind(const char *s) {
    if (strcmp(s, "glyph") == 0) return TOK_K_GLYPH;
    if (strcmp(s, "yields") == 0) return TOK_K_YIELDS;
    if (strcmp(s, "bind") == 0) return TOK_K_BIND;
    if (strcmp(s, "morph") == 0) return TOK_K_MORPH;
    if (strcmp(s, "shift") == 0) return TOK_K_SHIFT;
    if (strcmp(s, "fork") == 0) return TOK_K_FORK;
    if (strcmp(s, "otherwise") == 0) return TOK_K_OTHERWISE;
    if (strcmp(s, "cycle") == 0) return TOK_K_CYCLE;
    if (strcmp(s, "offer") == 0) return TOK_K_OFFER;
    if (strcmp(s, "invoke") == 0) return TOK_K_INVOKE;
    if (strcmp(s, "with") == 0) return TOK_K_WITH;
    if (strcmp(s, "chant") == 0) return TOK_K_CHANT;
    if (strcmp(s, "seal") == 0) return TOK_K_SEAL;

    if (strcmp(s, "ember") == 0) return TOK_K_EMBER;
    if (strcmp(s, "pulse") == 0) return TOK_K_PULSE;
    if (strcmp(s, "text") == 0) return TOK_K_TEXT;
    if (strcmp(s, "mist") == 0) return TOK_K_MIST;
    if (strcmp(s, "yes") == 0) return TOK_K_YES;
    if (strcmp(s, "no") == 0) return TOK_K_NO;

    if (strcmp(s, "both") == 0) return TOK_K_BOTH;
    if (strcmp(s, "either") == 0) return TOK_K_EITHER;
    if (strcmp(s, "flip") == 0) return TOK_K_FLIP;
    if (strcmp(s, "same") == 0) return TOK_K_SAME;
    if (strcmp(s, "diff") == 0) return TOK_K_DIFF;
    if (strcmp(s, "less") == 0) return TOK_K_LESS;
    if (strcmp(s, "more") == 0) return TOK_K_MORE;
    if (strcmp(s, "atmost") == 0) return TOK_K_ATMOST;
    if (strcmp(s, "atleast") == 0) return TOK_K_ATLEAST;

    return TOK_IDENT;
}

static void lex_number(Lexer *lx, int line, int col) {
    size_t start = lx->pos;
    while (isdigit((unsigned char)peek(lx))) {
        bump(lx);
    }
    char *digits = slice_dup(lx->src, start, lx->pos);
    Token t;
    t.kind = TOK_INT;
    t.lexeme = digits;
    t.int_value = strtol(digits, NULL, 10);
    t.line = line;
    t.col = col;
    token_push(&lx->out, t);
}

static void lex_ident_or_kw(Lexer *lx, int line, int col) {
    size_t start = lx->pos;
    while (isalnum((unsigned char)peek(lx)) || peek(lx) == '_') {
        bump(lx);
    }
    char *text = slice_dup(lx->src, start, lx->pos);
    TokenKind kind = keyword_kind(text);
    Token t;
    t.kind = kind;
    t.lexeme = text;
    t.int_value = 0;
    t.line = line;
    t.col = col;
    token_push(&lx->out, t);
}

static void lex_string(Lexer *lx, int line, int col) {
    bump(lx);
    size_t cap = 16;
    size_t len = 0;
    char *buf = xmalloc(cap);

    for (;;) {
        char c = peek(lx);
        if (c == '\0') {
            fatal_at(lx->file, line, col, "unterminated string literal");
        }
        if (c == '"') {
            bump(lx);
            break;
        }
        if (c == '\n') {
            fatal_at(lx->file, line, col, "newline in string literal");
        }
        if (c == '\\') {
            bump(lx);
            char esc = peek(lx);
            if (esc == '\0') {
                fatal_at(lx->file, line, col, "unterminated string escape");
            }
            bump(lx);
            switch (esc) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '"': c = '"'; break;
                case '\\': c = '\\'; break;
                default:
                    fatal_at(lx->file, line, col, "unsupported escape sequence \\%c", esc);
            }
        } else {
            bump(lx);
        }

        if (len + 1 >= cap) {
            cap *= 2;
            buf = xrealloc(buf, cap);
        }
        buf[len++] = c;
    }

    buf[len] = '\0';
    Token t;
    t.kind = TOK_STRING;
    t.lexeme = buf;
    t.int_value = 0;
    t.line = line;
    t.col = col;
    token_push(&lx->out, t);
}

void lex_source(const char *file, const char *src, TokenArray *out_tokens) {
    Lexer lx;
    lx.file = file;
    lx.src = src;
    lx.pos = 0;
    lx.line = 1;
    lx.col = 1;
    lx.out.data = NULL;
    lx.out.len = 0;
    lx.out.cap = 0;

    while (peek(&lx) != '\0') {
        char c = peek(&lx);
        int line = lx.line;
        int col = lx.col;

        if (c == ' ' || c == '\t' || c == '\r') {
            bump(&lx);
            continue;
        }
        if (c == '#') {
            while (peek(&lx) != '\0' && peek(&lx) != '\n') {
                bump(&lx);
            }
            continue;
        }
        if (c == '\n') {
            bump(&lx);
            emit_simple(&lx, TOK_NEWLINE, line, col);
            continue;
        }

        if (isdigit((unsigned char)c)) {
            lex_number(&lx, line, col);
            continue;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            lex_ident_or_kw(&lx, line, col);
            continue;
        }
        if (c == '"') {
            lex_string(&lx, line, col);
            continue;
        }

        switch (c) {
            case '+': bump(&lx); emit_simple(&lx, TOK_PLUS, line, col); break;
            case '-': bump(&lx); emit_simple(&lx, TOK_MINUS, line, col); break;
            case '*': bump(&lx); emit_simple(&lx, TOK_STAR, line, col); break;
            case '/': bump(&lx); emit_simple(&lx, TOK_SLASH, line, col); break;
            case '=': bump(&lx); emit_simple(&lx, TOK_ASSIGN, line, col); break;
            case ',': bump(&lx); emit_simple(&lx, TOK_COMMA, line, col); break;
            case ':': bump(&lx); emit_simple(&lx, TOK_COLON, line, col); break;
            case '[': bump(&lx); emit_simple(&lx, TOK_LBRACKET, line, col); break;
            case ']': bump(&lx); emit_simple(&lx, TOK_RBRACKET, line, col); break;
            default:
                fatal_at(file, line, col, "unexpected character '%c'", c);
        }
    }

    emit_simple(&lx, TOK_EOF, lx.line, lx.col);
    *out_tokens = lx.out;
}

void free_tokens(TokenArray *tokens) {
    if (!tokens || !tokens->data) {
        return;
    }
    for (size_t i = 0; i < tokens->len; i++) {
        free(tokens->data[i].lexeme);
    }
    free(tokens->data);
    tokens->data = NULL;
    tokens->len = 0;
    tokens->cap = 0;
}

const char *token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOK_EOF: return "end-of-file";
        case TOK_NEWLINE: return "newline";
        case TOK_IDENT: return "identifier";
        case TOK_INT: return "integer";
        case TOK_STRING: return "string";
        case TOK_PLUS: return "+";
        case TOK_MINUS: return "-";
        case TOK_STAR: return "*";
        case TOK_SLASH: return "/";
        case TOK_ASSIGN: return "=";
        case TOK_COMMA: return ",";
        case TOK_COLON: return ":";
        case TOK_LBRACKET: return "[";
        case TOK_RBRACKET: return "]";
        case TOK_K_GLYPH: return "glyph";
        case TOK_K_YIELDS: return "yields";
        case TOK_K_BIND: return "bind";
        case TOK_K_MORPH: return "morph";
        case TOK_K_SHIFT: return "shift";
        case TOK_K_FORK: return "fork";
        case TOK_K_OTHERWISE: return "otherwise";
        case TOK_K_CYCLE: return "cycle";
        case TOK_K_OFFER: return "offer";
        case TOK_K_INVOKE: return "invoke";
        case TOK_K_WITH: return "with";
        case TOK_K_CHANT: return "chant";
        case TOK_K_SEAL: return "seal";
        case TOK_K_EMBER: return "ember";
        case TOK_K_PULSE: return "pulse";
        case TOK_K_TEXT: return "text";
        case TOK_K_MIST: return "mist";
        case TOK_K_YES: return "yes";
        case TOK_K_NO: return "no";
        case TOK_K_BOTH: return "both";
        case TOK_K_EITHER: return "either";
        case TOK_K_FLIP: return "flip";
        case TOK_K_SAME: return "same";
        case TOK_K_DIFF: return "diff";
        case TOK_K_LESS: return "less";
        case TOK_K_MORE: return "more";
        case TOK_K_ATMOST: return "atmost";
        case TOK_K_ATLEAST: return "atleast";
    }
    return "<unknown-token>";
}
