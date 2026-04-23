#include "pik.h"

void beep()
{
    unsigned short divisor = 1193180 / 1000; // 1000 Hz

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
