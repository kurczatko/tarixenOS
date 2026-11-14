/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/apps/kalendarz.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <include/printf/types.h>

static void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static uint8_t rtc_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static uint8_t bcd_to_bin(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

void kalendarz() {
    vga_init();

    uint8_t status_b = rtc_read(0x0B);
    int bcd = !(status_b & 0x04);

    uint8_t godzina = rtc_read(0x04);
    uint8_t minuta  = rtc_read(0x02);
    uint8_t dzien   = rtc_read(0x07);
    uint8_t miesiac = rtc_read(0x08);
    uint8_t rok     = rtc_read(0x09);

    if (bcd) {
        godzina = bcd_to_bin(godzina);
        minuta  = bcd_to_bin(minuta);
        dzien   = bcd_to_bin(dzien);
        miesiac = bcd_to_bin(miesiac);
        rok     = bcd_to_bin(rok);
    }

    printf(" data: %02d-%02d-%02d\n", dzien, miesiac, rok); //nareszcie sie wyswietla data i godzina oprocz tego ze kod wyglada mądrze
    printf(" godzina: %02d:%02d\n", godzina, minuta);
}

