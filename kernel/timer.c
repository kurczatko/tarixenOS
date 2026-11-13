#include <kernel/timer.h>
#include <kernel/proces.h>

void timer_handler(void) {
    proces_tick();
}

void timer_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;

    __asm__ volatile (
        "movb $0x36, %%al\n\t"
        "outb %%al, $0x43\n\t"
        :
        :
        : "al"
    );

    __asm__ volatile (
        "movb %b0, %%al\n\t"
        "outb %%al, $0x40\n\t"
        "movb %h0, %%al\n\t"
        "outb %%al, $0x40\n\t"
        :
        : "r"(divisor)
        : "al"
    );
}
