#include "lexer_os.h"
#include "codegen_os.h"
#include "../include/printf/printf.h"
#include "../system_plikow/system_plikow.h"
#include "../kernel/loader.h"
#include "../include/printf/types.h"

#define BASE_ADDR 0x00100000
#define SYSCALL_OKNO_STWORZ 6
#define SYSCALL_TEKST_RYSUJ 5
#define SYSCALL_OKNO_RYSUJ 7
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

static uint8_t cc_heap[262144];
static int cc_heap_pos = 0;

static void *cc_os_malloc(int rozmiar) {
    void *p = cc_heap + cc_heap_pos;
    cc_heap_pos += rozmiar;
    return p;
}

static void __attribute__((unused)) cc_os_free(void *p) {
    (void)p;
}

static void cc_error(cc_t *cc, const char *msg) {
    printf("blad: %s\n", msg);
    cc->cg.errors++;
}

static void emit_mov_imm(cc_t *cc, uint8_t op, uint32_t imm, int *fix_pos_out) {
    if (fix_pos_out) *fix_pos_out = cc->cg.code_size;
    cg_emit(&cc->cg, op);
    cg_emit32(&cc->cg, imm);
}

static void add_fixup(cc_t *cc, int pos, int off) {
    if (cc->fix_count >= cc->fix_cap) { cc->fix_cap = cc->fix_cap ? cc->fix_cap * 2 : 16; cc->fixups = cc_os_malloc(cc->fix_cap * sizeof(strfix_t)); }
    cc->fixups[cc->fix_count].pos = pos;
    cc->fixups[cc->fix_count].rodata_off = off;
    cc->fix_count++;
}

static void expect(cc_t *cc, tok_t kind) {
    if (cc->lx.cur.kind != kind) { cc_error(cc, "nieoczekiwany token"); return; }
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
        int off = cg_add_rodata(&cc->cg, t->str, 0);
        int dlugosc = 0; while (t->str[dlugosc]) dlugosc++;
        off = cg_add_rodata(&cc->cg, t->str, dlugosc);
        if (cc->fix_count >= cc->fix_cap) { cc->fix_cap = cc->fix_cap ? cc->fix_cap * 2 : 16; cc->fixups = cc_os_malloc(cc->fix_cap * sizeof(strfix_t)); }
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
    char vname[64]; int i = 0; while (cc->lx.cur.str[i] && i < 63) { vname[i] = cc->lx.cur.str[i]; i++; } vname[i] = 0;
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
        char vname[64]; int i = 0; while (cc->lx.cur.str[i] && i < 63) { vname[i] = cc->lx.cur.str[i]; i++; } vname[i] = 0;
        int vi=cg_find_var(&cc->cg,vname);
        if (vi<0) vi=cg_add_var(&cc->cg,vname);
        expect(cc, TOK_SEMI);
    } else if (t->kind==TOK_PRINTF) {
        lex_next(&cc->lx); expect(cc, TOK_LPAREN);
        if (cc->lx.cur.kind == TOK_STRING) {
            int dlugosc = 0; while (cc->lx.cur.str[dlugosc]) dlugosc++;
            int off_msg = cg_add_rodata(&cc->cg, cc->lx.cur.str, dlugosc);
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

static int zapisz_elf_do_bufora(cg_t *cg, uint8_t *bufor, int max_rozmiar) {
    uint32_t total = cg->code_size + cg->rodata_size;
    if (52 + 32 + total > (uint32_t)max_rozmiar) return -1;
    uint8_t *p = bufor;
    for (int i = 0; i < 84; i++) p[i] = 0;
    p[0]=0x7F; p[1]='E'; p[2]='L'; p[3]='F';
    p[4]=1; p[5]=1; p[6]=1;
    p[16]=2; p[17]=0;
    p[18]=3; p[19]=0;
    p[24]=1;
    p[0x18]=0x00; p[0x19]=0x10; p[0x1A]=0x00; p[0x1B]=0x00;
    p[0x1C]=0x34; p[0x1D]=0x00;
    p[0x34]=0x04; p[0x35]=0x00;
    p[0x36]=0x20; p[0x37]=0x00;
    p[0x38]=0x01; p[0x39]=0x00;
    p += 52;
    for (int i = 0; i < 32; i++) p[i] = 0;
    p[0]=1;
    p[8]=0x00; p[9]=0x10; p[10]=0x00; p[11]=0x00;
    p[12]=0x00; p[13]=0x10; p[14]=0x00; p[15]=0x00;
    p[16]=total&0xFF; p[17]=(total>>8)&0xFF; p[18]=(total>>16)&0xFF; p[19]=(total>>24)&0xFF;
    p[20]=(total+4096)&0xFF; p[21]=((total+4096)>>8)&0xFF; p[22]=((total+4096)>>16)&0xFF; p[23]=((total+4096)>>24)&0xFF;
    p[24]=7;
    p[28]=0x00; p[29]=0x10; p[30]=0x00; p[31]=0x00;
    p += 32;
    for (int i = 0; i < cg->code_size; i++) p[i] = cg->code[i];
    p += cg->code_size;
    for (int i = 0; i < cg->rodata_size; i++) p[i] = cg->rodata[i];
    return 52 + 32 + (int)total;
}

int kompiluj_plik(const char *wejsciowa, const char *wyjsciowa) {
    char *src = cc_os_malloc(65536 + 1);
    int len = read_file(wejsciowa, src, 65536);
    if (len <= 0) { printf("nie mozna odczytac: %s\n", wejsciowa); return -1; }
    src[len] = 0;

    cc_t cc; for (int i = 0; i < (int)sizeof(cc) / 4; i++) ((uint32_t *)&cc)[i] = 0;
    cg_init(&cc.cg);
    lex_init(&cc.lx, src, len);
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

    uint8_t elf_buf[131072];
    int elf_size = zapisz_elf_do_bufora(&cc.cg, elf_buf, sizeof(elf_buf));
    if (elf_size < 0) { printf("elf za duzy\n"); return -1; }

    if (create_file(wyjsciowa, (uint32_t)elf_size) != 0) {
        printf("nie mozna utworzyc: %s\n", wyjsciowa);
        return -1;
    }
    if (write_file(wyjsciowa, (const char *)elf_buf, (uint32_t)elf_size) != 0) {
        printf("blad zapisu: %s\n", wyjsciowa);
        return -1;
    }

    if (cc.cg.errors) return 1;
    return 0;
}

int kompiluj_i_uruchom(const char *wejsciowa) {
    char wyjsciowa[32];
    int i = 0;
    while (wejsciowa[i] && i < 30) { wyjsciowa[i] = wejsciowa[i]; i++; }
    // zamien .c na .elf
    int kropekka = -1;
    for (int j = 0; wejsciowa[j]; j++) if (wejsciowa[j] == '.') kropekka = j;
    if (kropekka >= 0) { wyjsciowa[kropekka] = 0; }
    int n = i;
    if (kropekka >= 0) n = kropekka;
    wyjsciowa[n] = '.';
    wyjsciowa[n+1] = 'e';
    wyjsciowa[n+2] = 'l';
    wyjsciowa[n+3] = 'f';
    wyjsciowa[n+4] = 0;

    int wynik = kompiluj_plik(wejsciowa, wyjsciowa);
    if (wynik != 0) { printf("kompilacja nieudana (kod %d)\n", wynik); return wynik; }

    printf("uruchamiam: %s\n", wyjsciowa);
    return userspace_load_and_run(wyjsciowa);
}
