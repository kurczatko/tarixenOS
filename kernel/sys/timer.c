/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/sys/timer.h>
#include <kernel/core/proces.h>

static uint32_t g_timer_ticks = 0;

void timer_handler(void) {
    g_timer_ticks++;
    proces_tick();
}

uint32_t timer_ticks_get(void) {
    return g_timer_ticks;
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
