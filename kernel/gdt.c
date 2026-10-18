#include "gdt.h"

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;
static struct tss tss;

/* Stos rdzeniowy uzywany przez TSS przy wejsciu z userspace (ring 3 -> ring 0). */
static uint32_t stos_rdzeniowy[4096 / sizeof(uint32_t)];

static void gdt_set_gate(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[idx].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[idx].base_low = (uint16_t)(base & 0xFFFF);
    gdt[idx].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[idx].access = access;
    gdt[idx].granularity = (uint8_t)((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[idx].base_high = (uint8_t)((base >> 24) & 0xFF);
}

void gdt_install(void)
{
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                                   /* null */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);                    /* ring0 kod */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);                    /* ring0 dane */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);                    /* ring3 kod */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);                    /* ring3 dane */
    gdt_set_gate(5, (uint32_t)&tss, sizeof(struct tss) - 1, 0xE9, 0x00); /* TSS (dostepny) */
    gdt_set_gate(6, 0, 0, 0, 0);                                   /* TSS high (unused) */

    asm volatile ("lgdt %0" : : "m"(gp));

    /* Zaladuj TSS do rejestru TR. */
    asm volatile ("ltr %%ax" :: "a"(GDT_TSS));

    /* Segmenty danych na ring 0. */
    asm volatile ("movw $0x10, %%ax; movw %%ax, %%ds; movw %%ax, %%es;"
                  "movw %%ax, %%fs; movw %%ax, %%gs; movw %%ax, %%ss" ::: "eax");

    /* Skonfiguruj TSS: stos ring 0 obsugujacy przerwania/syscally z userspace. */
    tss.ss0 = GDT_RING0_DATA;
    tss.esp0 = (uint32_t)&stos_rdzeniowy[sizeof(stos_rdzeniowy) / sizeof(uint32_t)];
    tss.ss = GDT_RING0_DATA;
    tss.esp = tss.esp0;
    tss.cs = GDT_RING0_CODE;
    tss.ds = GDT_RING0_DATA;
    tss.es = GDT_RING0_DATA;
    tss.fs = GDT_RING0_DATA;
    tss.gs = GDT_RING0_DATA;
    tss.io_bitmap_base = sizeof(struct tss);
}

void gdt_set_ring(uint8_t ring)
{
    (void)ring;
    asm volatile ("movw $0x10, %%ax; movw %%ax, %%ds" ::: "eax");
}

struct tss *gdt_tss(void)
{
    return &tss;
}

uint32_t gdt_kernel_stack_top(void)
{
    return tss.esp0;
}
