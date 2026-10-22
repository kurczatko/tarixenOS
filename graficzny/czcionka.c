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

void rysuj_znak_styl(int x, int y, char znak, unsigned int kolor, uint8_t rozmiar, uint8_t styl)
{
    unsigned char indeks = (unsigned char)znak;
    if (indeks >= 128) indeks = '?';
    uint8_t bold = styl & 1;
    uint8_t italic = styl & 2;
    uint8_t thin = styl & 4;

    if (rozmiar == 0) {
        for (int pion = 0; pion < 8; pion++) {
            int sx = x + (italic ? pion : 0);
            for (int poziom = 0; poziom < 8; poziom++) {
                if (thin && (poziom % 2 == 1)) continue;
                if (font_olivetti_8x8[indeks][pion][poziom]) {
                    rysuj_piksel(sx + poziom, y + pion, kolor);
                    if (bold) rysuj_piksel(sx + poziom + 1, y + pion, kolor);
                }
            }
        }
    } else if (rozmiar == 2) {
        for (int pion = 0; pion < 8; pion++) {
            int sy = y + pion * 2;
            int sx = x + (italic ? pion * 2 : 0);
            for (int poziom = 0; poziom < 8; poziom++) {
                if (thin && (poziom % 2 == 1)) continue;
                if (font_olivetti_8x8[indeks][pion][poziom]) {
                    rysuj_piksel(sx + poziom * 2, sy, kolor);
                    rysuj_piksel(sx + poziom * 2 + 1, sy, kolor);
                    rysuj_piksel(sx + poziom * 2, sy + 1, kolor);
                    rysuj_piksel(sx + poziom * 2 + 1, sy + 1, kolor);
                    if (bold) {
                        rysuj_piksel(sx + poziom * 2 + 2, sy, kolor);
                        rysuj_piksel(sx + poziom * 2 + 2, sy + 1, kolor);
                    }
                }
            }
        }
    } else {
        for (int pion = 0; pion < 16; pion++) {
            int sx = x + (italic ? pion : 0);
            for (int poziom = 0; poziom < 12; poziom++) {
                if (thin && (poziom % 2 == 1)) continue;
                if (font_olivetti_12x16[indeks][pion][poziom]) {
                    rysuj_piksel(sx + poziom, y + pion, kolor);
                    if (bold) rysuj_piksel(sx + poziom + 1, y + pion, kolor);
                }
            }
        }
    }
}

int znak_szerokosc(uint8_t rozmiar)
{
    if (rozmiar == 0) return 8;
    if (rozmiar == 2) return 16;
    return 12;
}