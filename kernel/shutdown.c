/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/shutdown.h>

static void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "d"(port));
    return value;
}

static void shutdown_acpi(void) {
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
}

static void shutdown_ps2(void) {
    uint8_t val = inb(0x64);
    if (val & 0x02) return;
    outb(0x64, 0xFE);
}

static void shutdown_rtc_cmos(void) {
    outb(0x70, 0x0F | 0x80);
    outb(0x71, 0x00);
}

void wylaczanie(void) {
    shutdown_acpi();
    shutdown_ps2();
    shutdown_rtc_cmos();
}

void shutdown(void) {
    wylaczanie();
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
