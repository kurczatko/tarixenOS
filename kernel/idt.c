#include "idt.h"
#include "gdt.h"
#include "wyjatki.h"

static struct idt_entry idt[256];
static struct idt_ptr idtp;

extern void syscall_asm_handler(void);
extern void default_irq_handler(void);
extern void exc_0(void), exc_1(void), exc_3(void), exc_4(void), exc_5(void), exc_6(void), exc_7(void);
extern void exc_8(void), exc_9(void), exc_10(void), exc_11(void), exc_12(void), exc_13(void), exc_14(void);
extern void exc_15(void), exc_16(void), exc_18(void), exc_19(void);

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

    idt_set_gate(0,  (uint32_t)exc_0,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(1,  (uint32_t)exc_1,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(3,  (uint32_t)exc_3,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(4,  (uint32_t)exc_4,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(5,  (uint32_t)exc_5,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(6,  (uint32_t)exc_6,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(7,  (uint32_t)exc_7,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(8,  (uint32_t)exc_8,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(9,  (uint32_t)exc_9,  GDT_RING0_CODE, 0x8E);
    idt_set_gate(10, (uint32_t)exc_10, GDT_RING0_CODE, 0x8E);
    idt_set_gate(11, (uint32_t)exc_11, GDT_RING0_CODE, 0x8E);
    idt_set_gate(12, (uint32_t)exc_12, GDT_RING0_CODE, 0x8E);
    idt_set_gate(13, (uint32_t)exc_13, GDT_RING0_CODE, 0x8E);
    idt_set_gate(14, (uint32_t)exc_14, GDT_RING0_CODE, 0x8E);
    idt_set_gate(15, (uint32_t)exc_15, GDT_RING0_CODE, 0x8E);
    idt_set_gate(16, (uint32_t)exc_16, GDT_RING0_CODE, 0x8E);
    idt_set_gate(18, (uint32_t)exc_18, GDT_RING0_CODE, 0x8E);
    idt_set_gate(19, (uint32_t)exc_19, GDT_RING0_CODE, 0x8E);

    idt_set_gate(0x80, (uint32_t)syscall_asm_handler, GDT_RING0_CODE, 0xEE);

    asm volatile ("lidt %0" : : "m"(idtp));
}
