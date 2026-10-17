#include "czcionka.h"
#include "rysowanie.h"
#include "chicago_font_data.h"

void rysuj_znak_chicago(int x, int y, char znak, unsigned int kolor)
{
    unsigned char indeks = (unsigned char)znak;
    if (indeks >= 128) indeks = '?';

    for (int pion = 0; pion < 8; pion++) {
        for (int poziom = 0; poziom < 8; poziom++) {
            if (font_olivetti_8x8[indeks][pion][poziom]) {
                rysuj_piksel(x + poziom, y + pion, kolor);
            }
        }
    }
}

void rysuj_tekst_chicago(int x, int y, const char *tekst, unsigned int kolor)
{
    while (*tekst) {
        rysuj_znak_chicago(x, y, *tekst++, kolor);
        x += 8;
    }
}

void rysuj_znak_8x8(int x, int y, char znak, unsigned int kolor)
{
    unsigned char indeks = (unsigned char)znak;
    if (indeks >= 128) indeks = '?';

    for (int pion = 0; pion < 16; pion++) {
        for (int poziom = 0; poziom < 12; poziom++) {
            if (font_olivetti_12x16[indeks][pion][poziom]) {
                rysuj_piksel(x + poziom, y + pion, kolor);
            }
        }
    }
}

void rysuj_tekst_8x8(int x, int y, const char *tekst, unsigned int kolor)
{
    while (*tekst) {
        rysuj_znak_8x8(x, y, *tekst++, kolor);
        x += 12;
    }
}