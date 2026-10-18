#include "idt.h"
#include "gdt.h"

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void syscall_asm_handler(void);
extern void default_irq_handler(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].offset_low = (uint16_t)(base & 0xFFFF);
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
    idt[num].offset_mid = (uint16_t)((base >> 16) & 0xFFFF);
}

void idt_install(void)
{
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    for (int i = 0; i < 256; i++)
        idt_set_gate(i, (uint32_t)default_irq_handler, GDT_RING0_CODE, 0x8E);

    idt_set_gate(0x80, (uint32_t)syscall_asm_handler, GDT_RING0_CODE, 0x8E);

    asm volatile ("lidt %0" : : "m"(idtp));
}
