/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <scripts/mem.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <include/printf/types.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static uint8_t cmos_read(uint8_t reg) { // cmos umie czytać!
    outb(0x70, reg);
    return inb(0x71);
}

void mem() {
    uint16_t base_low  = cmos_read(0x15);
    uint16_t base_high = cmos_read(0x16);
    uint32_t base_mem  = (base_high << 8) | base_low; // mądre rzeczy typu odczyty cmos

    uint16_t ext_low  = cmos_read(0x17);
    uint16_t ext_high = cmos_read(0x18);
    uint32_t ext_mem  = (ext_high << 8) | ext_low;

    uint16_t ext16_low  = cmos_read(0x30);
    uint16_t ext16_high = cmos_read(0x31);
    uint32_t ext16_mem  = (ext16_high << 8) | ext16_low;

    uint32_t total_kb = base_mem + ext_mem + ext16_mem; // ehh niestety nie sluchałem na matematyce, dodawanie to nie moja bajka
    uint32_t total_mb = total_kb / 1024;

    printf(" pamiec ram: %u KB (%u MB)\n", total_kb, total_mb); // dlaczego ku**a jak uruchamiam w qemu to tam zamiast zmiennej jest literalnie napisane "%u kb"
    // bedzie trzeba to naprawic
}
