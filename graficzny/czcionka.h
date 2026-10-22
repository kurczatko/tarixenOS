#ifndef CZCIONKA_H
#define CZCIONKA_H

#include "../include/printf/types.h"

extern const unsigned char font_olivetti_8x8[128][8][8];
extern const unsigned char font_olivetti_12x16[128][16][12];

void rysuj_znak_chicago(int x, int y, char znak, unsigned int kolor);
void rysuj_tekst_chicago(int x, int y, const char *tekst, unsigned int kolor);
void rysuj_znak_8x8(int x, int y, char znak, unsigned int kolor);
void rysuj_tekst_8x8(int x, int y, const char *tekst, unsigned int kolor);
void rysuj_znak_styl(int x, int y, char znak, unsigned int kolor, uint8_t rozmiar, uint8_t styl);
int znak_szerokosc(uint8_t rozmiar);

#endif