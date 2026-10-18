#ifndef GDT_H
#define GDT_H

#include "../include/printf/types.h"

/* Selektory segmentow GDT (indeks << 3). */
#define GDT_NULL          0x00
#define GDT_RING0_CODE    0x08
#define GDT_RING0_DATA    0x10
#define GDT_RING3_CODE    0x18
#define GDT_RING3_DATA    0x20
#define GDT_TSS           0x28

/* Selektory z RPL=3 dla powrotu do userspace (iret ladowa CS/SS z RPL=3). */
#define GDT_SEL_RING3_CODE 0x1B
#define GDT_SEL_RING3_DATA 0x23

/* Liczba deskryptorow GDT (null, ring0 code/data, ring3 code/data, TSS low/high). */
#define GDT_ENTRIES 7

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct tss {
    uint16_t prev_link, prev_link_pad;
    uint32_t esp0;  uint16_t ss0,  ss0_pad;
    uint32_t esp1;  uint16_t ss1,  ss1_pad;
    uint32_t esp2;  uint16_t ss2,  ss2_pad;
    uint32_t cr3;
    uint32_t eip, eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint16_t es,  es_pad;  uint16_t cs,  cs_pad;
    uint16_t ss,  ss_pad;  uint16_t ds,  ds_pad;
    uint16_t fs,  fs_pad;  uint16_t gs,  gs_pad;
    uint16_t ldt, ldt_pad;
    uint16_t trace, io_bitmap_base;
} __attribute__((packed));

void gdt_install(void);
void gdt_set_ring(uint8_t ring);
struct tss *gdt_tss(void);
uint32_t gdt_kernel_stack_top(void);

#endif
