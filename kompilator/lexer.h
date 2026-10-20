#ifndef CC_LEXER_H
#define CC_LEXER_H
#include "types.h"

typedef enum {
    TOK_EOF, TOK_INT, TOK_RETURN, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_PRINTF,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, TOK_SEMI, TOK_ASSIGN,
    TOK_EQ, TOK_NE, TOK_LT, TOK_GT, TOK_LE, TOK_GE, TOK_PLUS, TOK_MINUS,
    TOK_MUL, TOK_DIV, TOK_ANDAND, TOK_OROR, TOK_NUM, TOK_ID, TOK_STRING, TOK_COMMA, TOK_VOID
} tok_t;

typedef struct {
    tok_t kind;
    int32_t val;
    char *str;
} token_t;

typedef struct {
    char *p;
    char *end;
    token_t cur;
    char idbuf[256];
    char strbuf[1024];
} lex_t;

void lex_init(lex_t *lx, char *src, int len);
void lex_next(lex_t *lx);
#endif
