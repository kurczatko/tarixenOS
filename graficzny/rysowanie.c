#include "rysowanie.h"
#include "czcionka.h"

#define CURSOR_W 12
#define CURSOR_H 18
#define KOLOR_TLA 0x0098C74C
#define KOLOR_PASKA_ZADAN 0x002D3E50
#define KOLOR_TEKST_BIALY 0x00FFFFFF

static uint32_t *pamiec_graficzna = (uint32_t *)0xFD000000;

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
    {1, 0, 0, 1, 1, 2, 2, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}
};

static int kursor_x;
static int kursor_y;

uint32_t *obraz = (uint32_t *)0x00400000;
static uint32_t kolor_tapety = KOLOR_TLA;

static void kopiuj_pamiec(void *dst, const void *src, uint32_t bajty)
{
    uint32_t *d = (uint32_t *)dst;
    const uint32_t *s = (const uint32_t *)src;
    uint32_t dwords = bajty / 4;
    for (uint32_t i = 0; i < dwords; i++) {
        d[i] = s[i];
    }
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

    for (int wiersz = 0; wiersz < wysokosc; wiersz++) {
        uint32_t offset = (uint32_t)(y + wiersz) * EKRAN_SZEROKOSC + (uint32_t)x;
        kopiuj_pamiec(&pamiec_graficzna[offset], &obraz[offset], szerokosc * 4);
    }
}

static void narysuj_napis_systemowy(void)
{
    int szerokosc_napisu = 27 * 8;
    int tekst_x = EKRAN_SZEROKOSC - szerokosc_napisu - 16;
    int tekst_y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 16;
    rysuj_tekst_8x8(tekst_x, tekst_y, "TarixenOS Tryb Graficzny", KOLOR_TEKST_BIALY);
}

void odswiez_widok(void)
{
    kopiuj_pamiec(pamiec_graficzna, obraz, EKRAN_SZEROKOSC * EKRAN_WYSOKOSC * 4);
}

void odtworz_fragment_tla(int x, int y, int szerokosc, int wysokosc)
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

    for (int py = y; py < y + wysokosc; py++) {
        uint32_t kolor = (py >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) ? KOLOR_PASKA_ZADAN : kolor_tapety;
        uint32_t offset = (uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)x;
        for (int px = 0; px < szerokosc; px++) {
            obraz[offset + px] = kolor;
        }
    }
    odswiez_fragment_z_bufora(x, y, szerokosc, wysokosc);
}

void rysuj_piksel(int x, int y, uint32_t kolor)
{
    if (x < 0 || x >= EKRAN_SZEROKOSC || y < 0 || y >= EKRAN_WYSOKOSC) return;
    obraz[y * EKRAN_SZEROKOSC + x] = kolor;
}

void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint32_t kolor)
{
    for (int pion = 0; pion < wysokosc; pion++) {
        int py = y + pion;
        if (py < 0 || py >= EKRAN_WYSOKOSC) continue;
        uint32_t offset = (uint32_t)py * EKRAN_SZEROKOSC;
        for (int poziom = 0; poziom < szerokosc; poziom++) {
            int px = x + poziom;
            if (px >= 0 && px < EKRAN_SZEROKOSC) {
                obraz[offset + px] = kolor;
            }
        }
    }
}

void rysuj_kwadrat(int x, int y, int bok, uint32_t kolor)
{
    rysuj_prostokat(x, y, bok, bok, kolor);
}

static void narysuj_kursor_bitmapa(int x, int y)
{
    for (int cy = 0; cy < CURSOR_H; cy++) {
        int py = y + cy;
        if (py < 0 || py >= EKRAN_WYSOKOSC) continue;
        uint32_t offset = (uint32_t)py * EKRAN_SZEROKOSC;

        for (int cx = 0; cx < CURSOR_W; cx++) {
            uint8_t typ = kursor_bitmapa[cy][cx];
            if (typ == 0) continue;

            int px = x + cx;
            if (px < 0 || px >= EKRAN_SZEROKOSC) continue;

            uint32_t kolor = (typ == 1) ? 0x00000000 : 0x00FFFFFF;
            pamiec_graficzna[offset + px] = kolor;
        }
    }
}

void narysuj_pasek_zadan(void)
{
    uint32_t start = (uint32_t)(EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) * EKRAN_SZEROKOSC;
    uint32_t size = (uint32_t)PASEK_ZADAN_WYSOKOSC * EKRAN_SZEROKOSC;
    for (uint32_t i = 0; i < size; i++) {
        obraz[start + i] = KOLOR_PASKA_ZADAN;
    }
    odswiez_fragment_z_bufora(0, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC, EKRAN_SZEROKOSC, PASEK_ZADAN_WYSOKOSC);
}

void wypelnij_ekran(uint32_t kolor)
{
    kolor_tapety = kolor;
    for (uint32_t i = 0; i < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC; i++) {
        obraz[i] = kolor_tapety;
    }
    narysuj_napis_systemowy();
    odswiez_widok();
}

void ustaw_kolor_tapety(uint32_t kolor)
{
    kolor_tapety = kolor;
    for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
        uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            obraz[offset + x] = kolor_tapety;
        }
    }
    narysuj_napis_systemowy();
    odswiez_fragment_z_bufora(0, 0, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC);
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
    uint32_t lfb_address = *(uint32_t *)0x7E28;

    if (lfb_address != 0) {
        pamiec_graficzna = (uint32_t *)lfb_address;
    }

    for (uint32_t i = 0; i < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC; i++) {
        obraz[i] = KOLOR_TLA;
    }

    for (int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y < EKRAN_WYSOKOSC; y++) {
        uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            obraz[offset + x] = KOLOR_PASKA_ZADAN;
        }
    }

    narysuj_napis_systemowy();

    odswiez_widok();
    kursor_graficzny_init();
}