#include "codegen.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_ADDR 0x00100000
#define SYSCALL_OKNO_STWORZ 6
#define SYSCALL_TEKST_RYSUJ 5
#define SYSCALL_OKNO_RYSUJ 7
#define SYSCALL_WRITE_CONSOLE 20
#define SYSCALL_EXIT 21

typedef struct {
    uint32_t pos;
    int32_t rodata_off;
} strfix_t;

typedef struct {
    lex_t lx;
    cg_t cg;
    strfix_t *fixups;
    int fix_count;
    int fix_cap;
    int in_func;
} cc_t;

static void cc_error(cc_t *cc, const char *msg) {
    fprintf(stderr, "błąd: %s\n", msg);
    cc->cg.errors++;
}

static void emit_mov_imm(cc_t *cc, uint8_t op, uint32_t imm, int *fix_pos_out) {
    if (fix_pos_out) *fix_pos_out = cc->cg.code_size;
    cg_emit(&cc->cg, op);
    cg_emit32(&cc->cg, imm);
}

static void add_fixup(cc_t *cc, int pos, int off) {
    if (cc->fix_count >= cc->fix_cap) { cc->fix_cap = cc->fix_cap ? cc->fix_cap*2 : 16; cc->fixups = realloc(cc->fixups, cc->fix_cap*sizeof(strfix_t)); }
    cc->fixups[cc->fix_count].pos = pos;
    cc->fixups[cc->fix_count].rodata_off = off;
    cc->fix_count++;
}

static void expect(cc_t *cc, tok_t kind) {
    if (cc->lx.cur.kind != kind) { cc_error(cc, "nieoczekiwany token"); fprintf(stderr, "  got=%d expected=%d\n", cc->lx.cur.kind, kind); return; }
    lex_next(&cc->lx);
}

static void emit_jmp_placeholder(cc_t *cc, uint8_t op, int label) {
    (void)label;
    cg_emit(&cc->cg, op);
    cg_emit32(&cc->cg, 0);
}

static void cc_expr(cc_t *cc);

static void cc_primary(cc_t *cc) {
    token_t *t = &cc->lx.cur;
    if (t->kind == TOK_NUM) {
        cg_emit_imm32(&cc->cg, (uint32_t)t->val);
        lex_next(&cc->lx);
    } else if (t->kind == TOK_ID) {
        int vi = cg_find_var(&cc->cg, t->str);
        if (vi < 0) { cc_error(cc, "niezadeklarowana zmienna"); lex_next(&cc->lx); cg_emit_imm32(&cc->cg, 0); return; }
        cg_emit(&cc->cg, 0x8B); cg_emit(&cc->cg, 0x45); cg_emit(&cc->cg, (uint8_t)(cc->cg.vars[vi].offset & 0xFF));
        lex_next(&cc->lx);
    } else if (t->kind == TOK_STRING) {
        int off = cg_add_rodata(&cc->cg, t->str, (int)strlen(t->str));
        if (cc->fix_count >= cc->fix_cap) { cc->fix_cap = cc->fix_cap ? cc->fix_cap*2 : 16; cc->fixups = realloc(cc->fixups, cc->fix_cap*sizeof(strfix_t)); }
        cc->fixups[cc->fix_count].pos = cc->cg.code_size;
        cc->fixups[cc->fix_count].rodata_off = off;
        cc->fix_count++;
        cg_emit(&cc->cg, 0xB8); cg_emit32(&cc->cg, 0);
        lex_next(&cc->lx);
    } else if (t->kind == TOK_LPAREN) {
        lex_next(&cc->lx); cc_expr(cc); expect(cc, TOK_RPAREN);
    } else if (t->kind == TOK_MINUS) {
        lex_next(&cc->lx); cc_primary(cc); cg_emit(&cc->cg, 0xF7); cg_emit(&cc->cg, 0xD8);
    } else if (t->kind == TOK_ANDAND || t->kind == TOK_OROR || t->kind == TOK_EQ || t->kind == TOK_NE || t->kind == TOK_LT || t->kind == TOK_GT || t->kind == TOK_LE || t->kind == TOK_GE) {
        cc_error(cc, "operator w wyrażeniu pierwszym"); lex_next(&cc->lx); cg_emit_imm32(&cc->cg, 0);
    } else {
        cc_error(cc, "oczekiwano wyrażenia"); lex_next(&cc->lx); cg_emit_imm32(&cc->cg, 0);
    }
}

static void cc_muldiv(cc_t *cc) {
    cc_primary(cc);
    while (cc->lx.cur.kind == TOK_MUL || cc->lx.cur.kind == TOK_DIV) {
        tok_t op = cc->lx.cur.kind; lex_next(&cc->lx);
        cg_emit(&cc->cg, 0x50);
        cc_primary(cc);
        cg_emit(&cc->cg, 0x5B);
        if (op == TOK_MUL) { cg_emit(&cc->cg, 0x0F); cg_emit(&cc->cg, 0xAF); cg_emit(&cc->cg, 0xC3); }
        else { cg_emit(&cc->cg, 0x8B); cg_emit(&cc->cg, 0xC3); cg_emit(&cc->cg, 0x99); cg_emit(&cc->cg, 0xF7); cg_emit(&cc->cg, 0xFB); }
    }
}

static void cc_addsub(cc_t *cc) {
    cc_muldiv(cc);
    while (cc->lx.cur.kind == TOK_PLUS || cc->lx.cur.kind == TOK_MINUS) {
        tok_t op = cc->lx.cur.kind; lex_next(&cc->lx);
        cg_emit(&cc->cg, 0x50);
        cc_muldiv(cc);
        cg_emit(&cc->cg, 0x5B);
        if (op == TOK_PLUS) cg_emit(&cc->cg, 0x01); else cg_emit(&cc->cg, 0x29);
        cg_emit(&cc->cg, 0xC3);
    }
}

static void cc_relational(cc_t *cc) {
    cc_addsub(cc);
    while (cc->lx.cur.kind==TOK_LT||cc->lx.cur.kind==TOK_GT||cc->lx.cur.kind==TOK_LE||cc->lx.cur.kind==TOK_GE) {
        tok_t op=cc->lx.cur.kind; lex_next(&cc->lx);
        cg_emit(&cc->cg,0x50); cc_addsub(cc); cg_emit(&cc->cg,0x5B);
        cg_emit(&cc->cg,0x39); cg_emit(&cc->cg,0xC3);
        uint8_t setcc;
        switch(op){case TOK_LT:setcc=0x9C;break;case TOK_GT:setcc=0x9F;break;case TOK_LE:setcc=0x9E;break;case TOK_GE:setcc=0x9D;break;default:setcc=0;break;}
        cg_emit(&cc->cg,0x0F); cg_emit(&cc->cg,setcc); cg_emit(&cc->cg,0xC0); cg_emit(&cc->cg,0x0F); cg_emit(&cc->cg,0xB6); cg_emit(&cc->cg,0xC0);
    }
}

static void cc_equality(cc_t *cc) {
    cc_relational(cc);
    while (cc->lx.cur.kind==TOK_EQ||cc->lx.cur.kind==TOK_NE) {
        tok_t op=cc->lx.cur.kind; lex_next(&cc->lx);
        cg_emit(&cc->cg,0x50); cc_relational(cc); cg_emit(&cc->cg,0x5B);
        cg_emit(&cc->cg,0x39); cg_emit(&cc->cg,0xC3);
        uint8_t setcc = (op==TOK_EQ)?0x94:0x95;
        cg_emit(&cc->cg,0x0F); cg_emit(&cc->cg,setcc); cg_emit(&cc->cg,0xC0); cg_emit(&cc->cg,0x0F); cg_emit(&cc->cg,0xB6); cg_emit(&cc->cg,0xC0);
    }
}

static void cc_logand(cc_t *cc) {
    cc_equality(cc);
    while (cc->lx.cur.kind==TOK_ANDAND) {
        lex_next(&cc->lx);
        int l1=++cc->cg.label_counter, l2=++cc->cg.label_counter;
        cg_emit(&cc->cg,0x85); cg_emit(&cc->cg,0xC0);
        emit_jmp_placeholder(cc,0x74,l1);
        cc_equality(cc);
        cg_emit(&cc->cg,0x85); cg_emit(&cc->cg,0xC0);
        emit_jmp_placeholder(cc,0x74,l1);
        cg_emit_imm32(&cc->cg,1); emit_jmp_placeholder(cc,0xEB,l2);
        cg_emit_imm32(&cc->cg,0);
        (void)0;
    }
}

static void cc_logor(cc_t *cc) {
    cc_logand(cc);
    while (cc->lx.cur.kind==TOK_OROR) {
        lex_next(&cc->lx);
        int l1=++cc->cg.label_counter,l2=++cc->cg.label_counter;
        cg_emit(&cc->cg,0x85); cg_emit(&cc->cg,0xC0);
        emit_jmp_placeholder(cc,0x75,l1);
        cc_logand(cc);
        cg_emit(&cc->cg,0x85); cg_emit(&cc->cg,0xC0);
        emit_jmp_placeholder(cc,0x74,l1);
        cg_emit_imm32(&cc->cg,1); emit_jmp_placeholder(cc,0xEB,l2);
        cg_emit_imm32(&cc->cg,0);
        (void)0;
    }
}

static void cc_expr(cc_t *cc) {
    cc_logor(cc);
}

static void cc_assign(cc_t *cc) {
    char vname[64]; strncpy(vname, cc->lx.cur.str, 63); vname[63]=0;
    int vi=cg_find_var(&cc->cg,vname);
    if (vi<0) vi=cg_add_var(&cc->cg,vname);
    lex_next(&cc->lx); expect(cc,TOK_ASSIGN);
    cc_expr(cc);
    cg_emit(&cc->cg,0x89); cg_emit(&cc->cg,0x45); cg_emit(&cc->cg,(uint8_t)(cc->cg.vars[vi].offset&0xFF));
}

static void cc_stmt(cc_t *cc);

static void cc_block(cc_t *cc) {
    expect(cc, TOK_LBRACE);
    while (cc->lx.cur.kind != TOK_RBRACE && cc->lx.cur.kind != TOK_EOF)
        cc_stmt(cc);
    expect(cc, TOK_RBRACE);
}

static void cc_stmt(cc_t *cc) {
    token_t *t=&cc->lx.cur;
    if (t->kind==TOK_INT) {
        lex_next(&cc->lx);
        expect(cc, TOK_ID);
        char vname[64]; strncpy(vname, cc->lx.cur.str, 63); vname[63]=0;
        int vi=cg_find_var(&cc->cg,vname);
        if (vi<0) vi=cg_add_var(&cc->cg,vname);
        expect(cc, TOK_SEMI);
    } else if (t->kind==TOK_PRINTF) {
        lex_next(&cc->lx); expect(cc, TOK_LPAREN);
        if (cc->lx.cur.kind == TOK_STRING) {
            int off_msg = cg_add_rodata(&cc->cg, cc->lx.cur.str, (int)strlen(cc->lx.cur.str));
            int off_title = cg_add_rodata(&cc->cg, "terminal", 8);
            int fix_msg = -1, fix_title = -1;
            emit_mov_imm(cc, 0xBB, 100, 0);
            emit_mov_imm(cc, 0xB9, 100, 0);
            emit_mov_imm(cc, 0xBA, 400, 0);
            emit_mov_imm(cc, 0xBE, 300, 0);
            emit_mov_imm(cc, 0xBF, 0, &fix_title);
            emit_mov_imm(cc, 0xB8, SYSCALL_OKNO_STWORZ, 0);
            cg_emit(&cc->cg, 0xCD); cg_emit(&cc->cg, 0x80);
            emit_mov_imm(cc, 0xBB, 20, 0);
            emit_mov_imm(cc, 0xB9, 30, 0);
            emit_mov_imm(cc, 0xBA, 0, &fix_msg);
            emit_mov_imm(cc, 0xBE, 0x00FFFFFF, 0);
            emit_mov_imm(cc, 0xB8, SYSCALL_TEKST_RYSUJ, 0);
            cg_emit(&cc->cg, 0xCD); cg_emit(&cc->cg, 0x80);
            emit_mov_imm(cc, 0xB8, SYSCALL_OKNO_RYSUJ, 0);
            cg_emit(&cc->cg, 0xCD); cg_emit(&cc->cg, 0x80);
            if (fix_title >= 0) add_fixup(cc, fix_title, off_title);
            if (fix_msg >= 0) add_fixup(cc, fix_msg, off_msg);
            lex_next(&cc->lx);
        }
        expect(cc, TOK_RPAREN);
        expect(cc, TOK_SEMI);
    } else if (t->kind==TOK_IF) {
        lex_next(&cc->lx); expect(cc, TOK_LPAREN);
        cc_expr(cc); expect(cc, TOK_RPAREN);
        int l1=++cc->cg.label_counter, l2=++cc->cg.label_counter;
        cg_emit(&cc->cg,0x85); cg_emit(&cc->cg,0xC0);
        emit_jmp_placeholder(cc,0x74,l1);
        cc_stmt(cc);
        emit_jmp_placeholder(cc,0xEB,l2);
        if (cc->lx.cur.kind==TOK_ELSE) { lex_next(&cc->lx); cc_stmt(cc); }
        (void)0;
    } else if (t->kind==TOK_WHILE) {
        lex_next(&cc->lx); expect(cc, TOK_LPAREN);
        int l1=++cc->cg.label_counter, l2=++cc->cg.label_counter;
        cc_expr(cc); expect(cc, TOK_RPAREN);
        cg_emit(&cc->cg,0x85); cg_emit(&cc->cg,0xC0);
        emit_jmp_placeholder(cc,0x74,l2);
        cc_stmt(cc);
        emit_jmp_placeholder(cc,0xEB,l1);
        (void)0;
    } else if (t->kind==TOK_RETURN) {
        lex_next(&cc->lx);
        cc_expr(cc); expect(cc, TOK_SEMI);
        cg_emit(&cc->cg,0x89); cg_emit(&cc->cg,0xEC);
        cg_emit(&cc->cg,0x5D);
        cg_emit(&cc->cg,0xC3);
    } else if (t->kind==TOK_LBRACE) {
        cc_block(cc);
    } else {
        cc_assign(cc); expect(cc, TOK_SEMI);
    }
}

static void cc_func(cc_t *cc) {
    expect(cc, TOK_INT);
    expect(cc, TOK_ID);
    expect(cc, TOK_LPAREN);
    if (cc->lx.cur.kind==TOK_VOID)
        lex_next(&cc->lx);
    expect(cc, TOK_RPAREN);
    cc->in_func=1;
    cg_emit(&cc->cg,0x55);
    cg_emit(&cc->cg,0x89); cg_emit(&cc->cg,0xE5);
    cg_emit(&cc->cg,0x81); cg_emit(&cc->cg,0xEC);
    cg_emit32(&cc->cg,0);
    int subesp_pos = cc->cg.code_size - 4;
    cc_block(cc);
    if (cc->cg.code_size>0 && cc->cg.code[cc->cg.code_size-1]!=0xC3) {
        cg_emit(&cc->cg,0x89); cg_emit(&cc->cg,0xEC);
        cg_emit(&cc->cg,0x5D);
        cg_emit(&cc->cg,0xC3);
    }
    cc->cg.code[subesp_pos] = cc->cg.stack_depth & 0xFF;
    cc->cg.code[subesp_pos+1] = (cc->cg.stack_depth>>8)&0xFF;
    cc->cg.code[subesp_pos+2] = (cc->cg.stack_depth>>16)&0xFF;
    cc->cg.code[subesp_pos+3] = (cc->cg.stack_depth>>24)&0xFF;
    cc->in_func=0;
}

static void write_elf(cg_t *cg, const char *path) {
    uint32_t total = cg->code_size + cg->rodata_size;
    FILE *f = fopen(path, "wb");
    if (!f) { perror("fopen"); return; }
    uint8_t hdr[84];
    memset(hdr, 0, 84);
    hdr[0]=0x7F; hdr[1]='E'; hdr[2]='L'; hdr[3]='F';
    hdr[4]=1; hdr[5]=1; hdr[6]=1;
    hdr[16]=2; hdr[17]=0;
    hdr[18]=3; hdr[19]=0;
    hdr[24]=1;
    hdr[0x18]=0x00; hdr[0x19]=0x10; hdr[0x1A]=0x00; hdr[0x1B]=0x00;
    hdr[0x1C]=0x34; hdr[0x1D]=0x00;
    hdr[0x34]=0x04; hdr[0x35]=0x00;
    hdr[0x36]=0x20; hdr[0x37]=0x00;
    hdr[0x38]=0x01; hdr[0x39]=0x00;
    fwrite(hdr, 1, 52, f);
    uint8_t ph[32];
    memset(ph, 0, 32);
    ph[0]=1;
    ph[8]=0x00; ph[9]=0x10; ph[10]=0x00; ph[11]=0x00;
    ph[12]=0x00; ph[13]=0x10; ph[14]=0x00; ph[15]=0x00;
    ph[16]=total&0xFF; ph[17]=(total>>8)&0xFF; ph[18]=(total>>16)&0xFF; ph[19]=(total>>24)&0xFF;
    ph[20]=(total+4096)&0xFF; ph[21]=((total+4096)>>8)&0xFF; ph[22]=((total+4096)>>16)&0xFF; ph[23]=((total+4096)>>24)&0xFF;
    ph[24]=7;
    ph[28]=0x00; ph[29]=0x10; ph[30]=0x00; ph[31]=0x00;
    fwrite(ph, 1, 32, f);
    fwrite(cg->code, 1, cg->code_size, f);
    fwrite(cg->rodata, 1, cg->rodata_size, f);
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr, "użycie: cc <wejscie.c> <wyjscie.elf>\n"); return 1; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    char *src = malloc(len + 1); fread(src, 1, len, f); src[len] = 0; fclose(f);
    cc_t cc; memset(&cc, 0, sizeof(cc));
    cg_init(&cc.cg);
    lex_init(&cc.lx, src, (int)len);
    cg_emit(&cc.cg, 0xBA); cg_emit32(&cc.cg, 0x003FF000);
    cg_emit(&cc.cg, 0x83); cg_emit(&cc.cg, 0xE4); cg_emit(&cc.cg, 0xF0);
    cg_emit(&cc.cg, 0xE8); cg_emit32(&cc.cg, 0);
    int call_main_pos = cc.cg.code_size - 4;
    cg_emit(&cc.cg, 0x89); cg_emit(&cc.cg, 0xC3);
    cg_emit(&cc.cg, 0xB8); cg_emit32(&cc.cg, SYSCALL_EXIT);
    cg_emit(&cc.cg, 0xCD); cg_emit(&cc.cg, 0x80);
    cg_emit(&cc.cg, 0xF4);
    int main_pos = cc.cg.code_size;
    uint32_t rel = main_pos - (cc.cg.code_size - 4);
    cc.cg.code[call_main_pos] = rel & 0xFF; cc.cg.code[call_main_pos+1]=(rel>>8)&0xFF; cc.cg.code[call_main_pos+2]=(rel>>16)&0xFF; cc.cg.code[call_main_pos+3]=(rel>>24)&0xFF;
    cc_func(&cc);
    uint32_t rodata_base = BASE_ADDR + cc.cg.code_size;
    for (int i = 0; i < cc.fix_count; i++) {
        uint32_t addr = rodata_base + cc.fixups[i].rodata_off;
        uint8_t *p = cc.cg.code + cc.fixups[i].pos + 1;
        p[0]=addr&0xFF; p[1]=(addr>>8)&0xFF; p[2]=(addr>>16)&0xFF; p[3]=(addr>>24)&0xFF;
    }
    write_elf(&cc.cg, argv[2]);
    free(cc.fixups); cg_free(&cc.cg); free(src);
    if (cc.cg.errors) return 1;
    return 0;
}
