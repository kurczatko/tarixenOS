#include "gdt.h"

static struct gdt_entry gdt[6];
static struct gdt_ptr gp;

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
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    gdt_set_gate(5, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    asm volatile ("lgdt %0" : : "m"(gp));
    asm volatile ("movw $0x10, %%ax; movw %%ax, %%ds; movw %%ax, %%es; movw %%ax, %%fs; movw %%ax, %%gs; movw %%ax, %%ss" ::: "eax");
}

void gdt_set_ring(uint8_t ring)
{
    (void)ring;
    asm volatile ("movw $0x10, %%ax; movw %%ax, %%ds" ::: "eax");
}
