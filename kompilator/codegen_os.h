#ifndef CC_OS_CODEGEN_H
#define CC_OS_CODEGEN_H
#include "types.h"
#include "lexer_os.h"

typedef struct {
    char name[64];
    int32_t offset;
    int used;
} var_t;

typedef struct {
    var_t vars[64];
    int var_count;
    int stack_depth;
    uint8_t *code;
    int code_size;
    int code_cap;
    uint8_t *rodata;
    int rodata_size;
    int rodata_cap;
    int label_counter;
    int errors;
} cg_t;

void cg_init(cg_t *cg);
void cg_emit(cg_t *cg, uint8_t b);
void cg_emit32(cg_t *cg, uint32_t v);
void cg_emit_imm32(cg_t *cg, uint32_t v);
int cg_add_var(cg_t *cg, const char *name);
int cg_find_var(cg_t *cg, const char *name);
int cg_add_rodata(cg_t *cg, const char *s, int len);
void cg_free(cg_t *cg);

#endif
