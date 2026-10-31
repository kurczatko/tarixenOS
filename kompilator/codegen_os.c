#include "codegen_os.h"
#include "../include/printf/types.h"
#include "../system_plikow/system_plikow.h"

static uint8_t cg_heap[131072];
static int cg_heap_pos = 0;

static void *cg_os_malloc(int rozmiar) {
    void *p = cg_heap + cg_heap_pos;
    cg_heap_pos += rozmiar;
    return p;
}

static void __attribute__((unused)) cg_os_free(void *p) {
    (void)p;
}

void cg_init(cg_t *cg) {
    for (int i = 0; i < (int)sizeof(*cg) / 4; i++) ((uint32_t *)cg)[i] = 0;
    cg->code_cap = 65536; cg->code = cg_os_malloc(cg->code_cap);
    cg->rodata_cap = 65536; cg->rodata = cg_os_malloc(cg->rodata_cap);
    cg->stack_depth = 0;
    cg->label_counter = 0;
}

void cg_emit(cg_t *cg, uint8_t b) {
    if (cg->code_size >= cg->code_cap) {
        uint8_t *nowy = cg_os_malloc(cg->code_cap * 2);
        for (int i = 0; i < cg->code_size; i++) nowy[i] = cg->code[i];
        cg->code_cap *= 2; cg->code = nowy;
    }
    cg->code[cg->code_size++] = b;
}

void cg_emit32(cg_t *cg, uint32_t v) {
    cg_emit(cg, v & 0xFF);
    cg_emit(cg, (v >> 8) & 0xFF);
    cg_emit(cg, (v >> 16) & 0xFF);
    cg_emit(cg, (v >> 24) & 0xFF);
}

void cg_emit_imm32(cg_t *cg, uint32_t v) {
    cg_emit(cg, 0xB8);
    cg_emit32(cg, v);
}

int cg_add_var(cg_t *cg, const char *name) {
    if (cg->var_count >= 64) { cg->errors++; return -1; }
    var_t *v = &cg->vars[cg->var_count++];
    int i = 0;
    while (name[i] && i < 63) { v->name[i] = name[i]; i++; }
    v->name[i] = 0;
    cg->stack_depth += 4;
    v->offset = -cg->stack_depth;
    v->used = 0;
    return v->offset;
}

int cg_find_var(cg_t *cg, const char *name) {
    for (int i = 0; i < cg->var_count; i++) {
        int rowne = 1;
        for (int j = 0; cg->vars[i].name[j] || name[j]; j++)
            if (cg->vars[i].name[j] != name[j]) { rowne = 0; break; }
        if (rowne) return i;
    }
    return -1;
}

int cg_add_rodata(cg_t *cg, const char *s, int len) {
    int off = cg->rodata_size;
    if (cg->rodata_size + len + 1 > cg->rodata_cap) {
        uint8_t *nowy = cg_os_malloc(cg->rodata_cap * 2);
        for (int i = 0; i < cg->rodata_size; i++) nowy[i] = cg->rodata[i];
        cg->rodata_cap *= 2; cg->rodata = nowy;
    }
    for (int i = 0; i < len; i++) cg->rodata[cg->rodata_size + i] = s[i];
    cg->rodata_size += len;
    cg->rodata[cg->rodata_size++] = 0;
    return off;
}

void cg_free(cg_t *cg) {
    (void)cg;
}
