#ifndef GDT_H
#define GDT_H

#include "../include/printf/types.h"

#define GDT_RING0_CODE 0x08
#define GDT_RING0_DATA 0x10
#define GDT_RING1_CODE 0x18
#define GDT_RING1_DATA 0x20
#define GDT_RING2_CODE 0x28
#define GDT_RING2_DATA 0x30
#define GDT_RING3_CODE 0x38
#define GDT_RING3_DATA 0x40

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

void gdt_install(void);
void gdt_set_ring(uint8_t ring);

#endif
