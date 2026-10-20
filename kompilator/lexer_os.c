#include "lexer_os.h"

static int os_strcmp(const char *s, const char *kw) {
    while (*s && *s == *kw) { s++; kw++; }
    return (unsigned char)*s - (unsigned char)*kw;
}

void lex_init(lex_t *lx, char *src, int len) {
    lx->p = src; lx->end = src + len;
    lx->cur.kind = TOK_EOF; lx->cur.val = 0; lx->cur.str = 0;
    lx->idbuf[0] = 0; lx->strbuf[0] = 0;
    lex_next(lx);
}

static void skip_ws(lex_t *lx) {
    while (lx->p < lx->end) {
        char c = *lx->p;
        if (c==' '||c=='\t'||c=='\n'||c=='\r') lx->p++;
        else if (c=='/' && lx->p+1<lx->end && lx->p[1]=='/') {
            while (lx->p<lx->end && *lx->p!='\n') lx->p++;
        } else break;
    }
}

void lex_next(lex_t *lx) {
    skip_ws(lx);
    if (lx->p >= lx->end) { lx->cur.kind = TOK_EOF; lx->cur.val = 0; lx->cur.str = 0; return; }
    char c = *lx->p++;
    if (c>='0'&&c<='9') {
        lx->p--; int32_t v=0; while(lx->p<lx->end && *lx->p>='0'&&*lx->p<='9') { v=v*10+(*lx->p-'0'); lx->p++; }
        lx->cur.kind=TOK_NUM; lx->cur.val=v; lx->cur.str=0; return;
    }
    if ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_') {
        lx->p--; char *b=lx->idbuf; while(lx->p<lx->end && ((*lx->p>='a'&&*lx->p<='z')||(*lx->p>='A'&&*lx->p<='Z')||(*lx->p>='0'&&*lx->p<='9')||*lx->p=='_')) *b++=*lx->p++; *b=0;
        if (os_strcmp(lx->idbuf,"int")==0) lx->cur.kind=TOK_INT;
        else if (os_strcmp(lx->idbuf,"return")==0) lx->cur.kind=TOK_RETURN;
        else if (os_strcmp(lx->idbuf,"if")==0) lx->cur.kind=TOK_IF;
        else if (os_strcmp(lx->idbuf,"else")==0) lx->cur.kind=TOK_ELSE;
        else if (os_strcmp(lx->idbuf,"while")==0) lx->cur.kind=TOK_WHILE;
        else if (os_strcmp(lx->idbuf,"printf")==0) lx->cur.kind=TOK_PRINTF;
        else if (os_strcmp(lx->idbuf,"void")==0) lx->cur.kind=TOK_VOID;
        else { lx->cur.kind=TOK_ID; lx->cur.str=lx->idbuf; }
        return;
    }
    if (c=='\"') {
        char *b=lx->strbuf; while(lx->p<lx->end && *lx->p!='\"') { char d=*lx->p++; if (d=='\\' && lx->p<lx->end && *lx->p=='n') { *b++='\n'; lx->p++; } else *b++=d; }
        if (lx->p<lx->end) lx->p++; *b=0;
        lx->cur.kind=TOK_STRING; lx->cur.str=lx->strbuf; return;
    }
    switch(c) {
        case '(': lx->cur.kind=TOK_LPAREN; return;
        case ')': lx->cur.kind=TOK_RPAREN; return;
        case '{': lx->cur.kind=TOK_LBRACE; return;
        case '}': lx->cur.kind=TOK_RBRACE; return;
        case ';': lx->cur.kind=TOK_SEMI; return;
        case '=': if (lx->p<lx->end && *lx->p=='=') { lx->p++; lx->cur.kind=TOK_EQ; } else lx->cur.kind=TOK_ASSIGN; return;
        case '!': if (lx->p<lx->end && *lx->p=='=') { lx->p++; lx->cur.kind=TOK_NE; } return;
        case '<': if (lx->p<lx->end && *lx->p=='=') { lx->p++; lx->cur.kind=TOK_LE; } else lx->cur.kind=TOK_LT; return;
        case '>': if (lx->p<lx->end && *lx->p=='=') { lx->p++; lx->cur.kind=TOK_GE; } else lx->cur.kind=TOK_GT; return;
        case '+': lx->cur.kind=TOK_PLUS; return;
        case '-': lx->cur.kind=TOK_MINUS; return;
        case '*': lx->cur.kind=TOK_MUL; return;
        case '/': lx->cur.kind=TOK_DIV; return;
        case '&': if (lx->p<lx->end && *lx->p=='&') { lx->p++; lx->cur.kind=TOK_ANDAND; } return;
        case '|': if (lx->p<lx->end && *lx->p=='|') { lx->p++; lx->cur.kind=TOK_OROR; } return;
        case ',': lx->cur.kind=TOK_COMMA; return;
        default: lx->cur.kind=TOK_EOF; return;
    }
}
