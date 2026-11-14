/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/pik.h>
#include <kernel/timer.h>

void pik_init(void) {
    __asm__ volatile (
        "movb $0x11, %%al; outb %%al, $0x20\n\t"
        "movb $0x11, %%al; outb %%al, $0xA0\n\t"
        "movb $0x20, %%al; outb %%al, $0x21\n\t"
        "movb $0x28, %%al; outb %%al, $0xA1\n\t"
        "movb $0x04, %%al; outb %%al, $0x21\n\t"
        "movb $0x02, %%al; outb %%al, $0xA1\n\t"
        "movb $0x01, %%al; outb %%al, $0x21\n\t"
        "movb $0x01, %%al; outb %%al, $0xA1\n\t"
        "movb $0x00, %%al; outb %%al, $0x21\n\t"
        "movb $0x00, %%al; outb %%al, $0xA1\n\t"
        :
        :
        : "al"
    );
}

void pik_enable_irq(uint8_t irq) {
    uint16_t port = (irq < 8) ? 0x21 : 0xA1;
    uint8_t irq_num = (irq < 8) ? irq : irq - 8;
    uint8_t mask;
    __asm__ volatile ("inb %1, %0" : "=a"(mask) : "Nd"(port));
    mask &= ~(1 << irq_num);
    __asm__ volatile ("outb %0, %1" : : "a"(mask), "Nd"(port));
}

void beep()
{
    unsigned short divisor = 1193180 / 1000;

    __asm__ __volatile__ (
        "movb $0xB6, %%al\n\t"
        "outb %%al, $0x43\n\t"
        :
        :
        : "al"
    );

    __asm__ __volatile__ (
        "mov %0, %%ax\n\t"
        "outb %%al, $0x42\n\t"
        "mov %%ah, %%al\n\t"
        "outb %%al, $0x42\n\t"
        :
        : "r"(divisor)
        : "ax"
    );

    __asm__ __volatile__ (
        "inb $0x61, %%al\n\t"
        "or $3, %%al\n\t"
        "outb %%al, $0x61\n\t"
        :
        :
        : "al"
    );

    for (volatile unsigned int i = 0; i < 1193180; i++) { __asm__ volatile ("nop"); }

    __asm__ __volatile__ (
        "inb $0x61, %%al\n\t"
        "and $0xFC, %%al\n\t"
        "outb %%al, $0x61\n\t"
        :
        :
        : "al"
    );
}
