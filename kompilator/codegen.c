#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cg_init(cg_t *cg) {
    memset(cg, 0, sizeof(*cg));
    cg->code_cap = 65536; cg->code = malloc(cg->code_cap);
    cg->rodata_cap = 65536; cg->rodata = malloc(cg->rodata_cap);
    cg->stack_depth = 0;
    cg->label_counter = 0;
}

void cg_emit(cg_t *cg, uint8_t b) {
    if (cg->code_size >= cg->code_cap) {
        cg->code_cap *= 2; cg->code = realloc(cg->code, cg->code_cap);
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
    cg_emit(cg, 0xB8); /* mov eax, imm32 */
    cg_emit32(cg, v);
}

int cg_add_var(cg_t *cg, const char *name) {
    if (cg->var_count >= 64) { cg->errors++; return -1; }
    var_t *v = &cg->vars[cg->var_count++];
    strncpy(v->name, name, 63); v->name[63] = 0;
    cg->stack_depth += 4;
    v->offset = -cg->stack_depth;
    v->used = 0;
    return v->offset;
}

int cg_find_var(cg_t *cg, const char *name) {
    for (int i = 0; i < cg->var_count; i++)
        if (strcmp(cg->vars[i].name, name) == 0) return i;
    return -1;
}

int cg_add_rodata(cg_t *cg, const char *s, int len) {
    int off = cg->rodata_size;
    if (cg->rodata_size + len + 1 > cg->rodata_cap) {
        cg->rodata_cap *= 2; cg->rodata = realloc(cg->rodata, cg->rodata_cap);
    }
    memcpy(cg->rodata + cg->rodata_size, s, len);
    cg->rodata_size += len;
    cg->rodata[cg->rodata_size++] = 0;
    return off;
}

void cg_free(cg_t *cg) {
    free(cg->code); free(cg->rodata);
}
