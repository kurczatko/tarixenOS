#include "rysowanie.h"

#define PAMIEC_GRAFICZNA ((uint8_t *)0xA0000)
#define PORT_SEKWENCERA 0x3C4
#define PORT_DANYCH_SEKWENCERA 0x3C5
#define PORT_KONTROLERA_GRAFIKI 0x3CE
#define PORT_DANYCH_GRAFIKI 0x3CF

#define CURSOR_W 12
#define CURSOR_H 18
#define KOLOR_TLA 0
#define KOLOR_PASKA_ZADAN 8

static const uint8_t kursor_bitmapa[CURSOR_H][CURSOR_W] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 1, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}
};

static int kursor_x;
static int kursor_y;
static uint8_t obraz[EKRAN_SZEROKOSC * EKRAN_WYSOKOSC];
static uint8_t kolor_tapety = KOLOR_TLA;

static void wybierz_plane(uint8_t numer);

static void ustaw_kontroler_grafiki(uint8_t rejestr, uint8_t wartosc)
{
    __asm__ volatile ("outb %0, %1" : : "a"(rejestr), "Nd"((uint16_t)PORT_KONTROLERA_GRAFIKI));
    __asm__ volatile ("outb %0, %1" : : "a"(wartosc), "Nd"((uint16_t)PORT_DANYCH_GRAFIKI));
}

static void wybierz_plane(uint8_t numer)
{
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)2), "Nd"((uint16_t)PORT_SEKWENCERA));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)(1 << numer)), "Nd"((uint16_t)PORT_DANYCH_SEKWENCERA));
}

static void odblokuj_wszystkie_plany(void)
{
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)2), "Nd"((uint16_t)PORT_SEKWENCERA));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)PORT_DANYCH_SEKWENCERA));
}

static void wypelnij_obraz_planami(uint8_t kolor)
{
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (uint32_t bajt = 0; bajt < 80 * EKRAN_WYSOKOSC; bajt++) {
            PAMIEC_GRAFICZNA[bajt] = (kolor & (1 << plane)) ? 0xFF : 0x00;
        }
    }
}

static void wypelnij_pasek_planami(uint8_t kolor)
{
    uint32_t poczatek = (uint32_t)(EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) * 80;
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (uint32_t bajt = 0; bajt < 80 * PASEK_ZADAN_WYSOKOSC; bajt++) {
            PAMIEC_GRAFICZNA[poczatek + bajt] = (kolor & (1 << plane)) ? 0xFF : 0x00;
        }
    }
}

static void odswiez_ekran(void)
{
    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(1, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (uint32_t indeks = 0; indeks < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC / 8; indeks++) {
            uint8_t bajt = 0;
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint32_t piksel = indeks * 8 + bit;
                if (obraz[piksel] & (1 << plane)) bajt |= (uint8_t)(0x80 >> bit);
            }
            PAMIEC_GRAFICZNA[indeks] = bajt;
        }
    }
}

void odswiez_widok(void)
{
    odswiez_ekran();
}

static void odswiez_fragment_z_bufora(int x, int y, int szerokosc, int wysokosc)
{
    if (x < 0) {
        szerokosc += x;
        x = 0;
    }
    if (y < 0) {
        wysokosc += y;
        y = 0;
    }
    if (x + szerokosc > EKRAN_SZEROKOSC) szerokosc = EKRAN_SZEROKOSC - x;
    if (y + wysokosc > EKRAN_WYSOKOSC) wysokosc = EKRAN_WYSOKOSC - y;
    if (szerokosc <= 0 || wysokosc <= 0) return;

    int pierwszy_bajt = x >> 3;
    int ostatni_bajt = (x + szerokosc - 1) >> 3;
    if (ostatni_bajt >= 80) ostatni_bajt = 79;

    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(1, 0);
    ustaw_kontroler_grafiki(8, 0xFF);

    for (uint8_t plane = 0; plane < 4; plane++) {
        uint8_t maska_plane = (uint8_t)(1 << plane);
        wybierz_plane(plane);

        for (int wiersz = 0; wiersz < wysokosc; wiersz++) {
            uint32_t row_obraz = (uint32_t)(y + wiersz) * EKRAN_SZEROKOSC;
            uint32_t row_vga = (uint32_t)(y + wiersz) * 80;

            for (int bajt = pierwszy_bajt; bajt <= ostatni_bajt; bajt++) {
                uint32_t px_base = row_obraz + (uint32_t)bajt * 8;
                uint8_t bajt_vga = 0;

                if (obraz[px_base + 0] & maska_plane) bajt_vga |= 0x80;
                if (obraz[px_base + 1] & maska_plane) bajt_vga |= 0x40;
                if (obraz[px_base + 2] & maska_plane) bajt_vga |= 0x20;
                if (obraz[px_base + 3] & maska_plane) bajt_vga |= 0x10;
                if (obraz[px_base + 4] & maska_plane) bajt_vga |= 0x08;
                if (obraz[px_base + 5] & maska_plane) bajt_vga |= 0x04;
                if (obraz[px_base + 6] & maska_plane) bajt_vga |= 0x02;
                if (obraz[px_base + 7] & maska_plane) bajt_vga |= 0x01;

                PAMIEC_GRAFICZNA[row_vga + bajt] = bajt_vga;
            }
        }
    }
}

void odtworz_fragment_tla(int x, int y, int szerokosc, int wysokosc)
{
    int dolny_y;
    int gorny_x;
    int dolny_x;

    if (x < 0) {
        szerokosc += x;
        x = 0;
    }
    if (y < 0) {
        wysokosc += y;
        y = 0;
    }
    if (x + szerokosc > EKRAN_SZEROKOSC) szerokosc = EKRAN_SZEROKOSC - x;
    if (y + wysokosc > EKRAN_WYSOKOSC) wysokosc = EKRAN_WYSOKOSC - y;
    if (szerokosc > 0 && wysokosc > 0) {
        dolny_y = y + wysokosc;
        gorny_x = x;
        dolny_x = x + szerokosc;
        for (int piksel_y = y; piksel_y < dolny_y; piksel_y++)
            for (int piksel_x = gorny_x; piksel_x < dolny_x; piksel_x++)
                obraz[piksel_y * EKRAN_SZEROKOSC + piksel_x] =
                    piksel_y >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC ?
                    KOLOR_PASKA_ZADAN : kolor_tapety;
        odswiez_fragment_z_bufora(x, y, szerokosc, wysokosc);
    }
}

void rysuj_piksel(int x, int y, uint8_t kolor)
{
    if (x < 0 || x >= EKRAN_SZEROKOSC || y < 0 || y >= EKRAN_WYSOKOSC) return;
    obraz[y * EKRAN_SZEROKOSC + x] = kolor & 0x0F;
}

void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint8_t kolor)
{
    for (int pion = 0; pion < wysokosc; pion++)
        for (int poziom = 0; poziom < szerokosc; poziom++)
            rysuj_piksel(x + poziom, y + pion, kolor);
}

void rysuj_kwadrat(int x, int y, int bok, uint8_t kolor)
{
    rysuj_prostokat(x, y, bok, bok, kolor);
}

static void narysuj_kursor_bitmapa(int x, int y)
{
    odblokuj_wszystkie_plany();
    ustaw_kontroler_grafiki(5, 2);

    for (int cy = 0; cy < CURSOR_H; cy++) {
        int py = y + cy;
        if (py < 0 || py >= EKRAN_WYSOKOSC) continue;
        uint32_t line_offset = (uint32_t)py * 80;

        for (int cx = 0; cx < CURSOR_W; cx++) {
            uint8_t typ = kursor_bitmapa[cy][cx];
            if (typ == 0) continue;

            int px = x + cx;
            if (px < 0 || px >= EKRAN_SZEROKOSC) continue;

            uint32_t offset = line_offset + (uint32_t)(px >> 3);
            uint8_t bitmask = (uint8_t)(0x80 >> (px & 7));
            uint8_t kolor = (typ == 1) ? 0 : 15;

            ustaw_kontroler_grafiki(8, bitmask);
            volatile uint8_t dummy = PAMIEC_GRAFICZNA[offset];
            PAMIEC_GRAFICZNA[offset] = kolor;
        }
    }

    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
}

void narysuj_pasek_zadan(void)
{
    uint32_t start = (uint32_t)(EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) * EKRAN_SZEROKOSC;
    uint32_t size = (uint32_t)PASEK_ZADAN_WYSOKOSC * EKRAN_SZEROKOSC;
    for (uint32_t i = 0; i < size; i++) {
        obraz[start + i] = KOLOR_PASKA_ZADAN;
    }
    wypelnij_pasek_planami(KOLOR_PASKA_ZADAN);
}

void wypelnij_ekran(uint8_t kolor)
{
    kolor_tapety = kolor & 0x0F;
    for (uint32_t i = 0; i < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC; i++) {
        obraz[i] = kolor_tapety;
    }
    wypelnij_obraz_planami(kolor_tapety);
}

void ustaw_kolor_tapety(uint8_t kolor)
{
    kolor_tapety = kolor & 0x0F;
    for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++)
        for (int x = 0; x < EKRAN_SZEROKOSC; x++)
            obraz[y * EKRAN_SZEROKOSC + x] = kolor_tapety;
    odswiez_ekran();
}

static void przywroc_tlo_kursora(void)
{
    odswiez_fragment_z_bufora(kursor_x, kursor_y, CURSOR_W, CURSOR_H);
}

void kursor_graficzny_init(void)
{
    kursor_x = EKRAN_SZEROKOSC / 2;
    kursor_y = EKRAN_WYSOKOSC / 2;
    narysuj_kursor_bitmapa(kursor_x, kursor_y);
}

void kursor_graficzny_przesun(int delta_x, int delta_y)
{
    if (delta_x == 0 && delta_y == 0) return;

    int stary_x = kursor_x;
    int stary_y = kursor_y;

    kursor_x += delta_x;
    kursor_y += delta_y;
    if (kursor_x < 0) kursor_x = 0;
    if (kursor_y < 0) kursor_y = 0;
    if (kursor_x > EKRAN_SZEROKOSC - CURSOR_W) kursor_x = EKRAN_SZEROKOSC - CURSOR_W;
    if (kursor_y > EKRAN_WYSOKOSC - CURSOR_H) kursor_y = EKRAN_WYSOKOSC - CURSOR_H;

    if (kursor_x == stary_x && kursor_y == stary_y) return;

    odswiez_fragment_z_bufora(stary_x, stary_y, CURSOR_W, CURSOR_H);
    narysuj_kursor_bitmapa(kursor_x, kursor_y);
}

void kursor_graficzny_pozycja(int *x, int *y)
{
    *x = kursor_x;
    *y = kursor_y;
}

void rysowanie_init(void)
{
    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
    for (uint32_t indeks = 0; indeks < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC; indeks++) obraz[indeks] = KOLOR_TLA;
    for (int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y < EKRAN_WYSOKOSC; y++)
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) obraz[y * EKRAN_SZEROKOSC + x] = KOLOR_PASKA_ZADAN;
    odswiez_ekran();
    kursor_graficzny_init();
}