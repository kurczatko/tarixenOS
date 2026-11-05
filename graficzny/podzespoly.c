#include "podzespoly.h"
#include "czcionka.h"
#include "okno.h"
#include "rysowanie.h"
#include "../kernel/driver_manager.h"
#include "../include/printf/types.h"

#define PODZESPOLY_SZEROKOSC 560
#define PODZESPOLY_WYSOKOSC 420

static struct okno okno_podzespoly;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static void u32_do_tekstu(uint32_t v, char *buf) {
    char tmp[12];
    int i = 0;
    int j = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }
    while (v > 0) {
        tmp[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = 0;
}

static void podzespoly_rysuj(int x, int y, int szerokosc, int wysokosc)
{
    (void)szerokosc;
    (void)wysokosc;
    int wy = y + 24;
    char buf[32];
    uint16_t base_low  = cmos_read(0x15);
    uint16_t base_high = cmos_read(0x16);
    uint32_t base_mem  = (base_high << 8) | base_low;
    uint16_t ext_low  = cmos_read(0x17);
    uint16_t ext_high = cmos_read(0x18);
    uint32_t ext_mem  = (ext_high << 8) | ext_low;
    uint16_t ext16_low  = cmos_read(0x30);
    uint16_t ext16_high = cmos_read(0x31);
    uint32_t ext16_mem  = (ext16_high << 8) | ext16_low;
    uint32_t total_kb = base_mem + ext_mem + ext16_mem;
    uint32_t total_mb = total_kb / 1024;

    rysuj_tekst_chicago(x + 4, y, "PODZESPOLY SYSTEMU", 0x00334155);
    rysuj_tekst_chicago(x + 4, wy, "System:        TarixenOS", 0x00334155);
    rysuj_tekst_chicago(x + 4, wy + 20, "Architektura:    x86 (i386)", 0x00334155);
    rysuj_tekst_chicago(x + 4, wy + 40, "Ekran:          1024 x 768", 0x00334155);
    rysuj_tekst_chicago(x + 4, wy + 60, "Pamiec RAM:", 0x00334155);
    u32_do_tekstu(total_kb, buf);
    rysuj_tekst_chicago(x + 100, wy + 60, buf, 0x00334155);
    rysuj_tekst_chicago(x + 130, wy + 60, "KB", 0x00334155);
    u32_do_tekstu(total_mb, buf);
    rysuj_tekst_chicago(x + 150, wy + 60, buf, 0x00334155);
    rysuj_tekst_chicago(x + 180, wy + 60, "MB", 0x00334155);
    rysuj_tekst_chicago(x + 4, wy + 80, "Sterowniki:", 0x00334155);

    char nazwy[32][64];
    int ile = driver_manager_get_driver_list(nazwy, 32);
    int sy = wy + 100;
    for (int i = 0; i < ile; i++) {
        rysuj_tekst_chicago(x + 20, sy + i * 20, nazwy[i], 0x00334155);
    }
    rysuj_tekst_chicago(x + 4, wy + 100 + ile * 20 + 20, "Nacisnij X aby zamknac", 0x00334155);
}

static void podzespoly_klik(int x, int y)
{
    (void)x;
    (void)y;
}

void podzespoly_otworz(void)
{
    okno_stworz(&okno_podzespoly, 200, 140, PODZESPOLY_SZEROKOSC, PODZESPOLY_WYSOKOSC,
                "PODZESPOLY", podzespoly_rysuj);
    okno_ustaw_obsluge_klikniecia(&okno_podzespoly, podzespoly_klik);
}
