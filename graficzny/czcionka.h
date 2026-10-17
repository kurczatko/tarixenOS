#ifndef CZCIONKA_H
#define CZCIONKA_H

extern const unsigned char font_olivetti_8x8[128][8][8];
extern const unsigned char font_olivetti_12x16[128][16][12];

void rysuj_znak_chicago(int x, int y, char znak, unsigned int kolor);
void rysuj_tekst_chicago(int x, int y, const char *tekst, unsigned int kolor);
void rysuj_znak_8x8(int x, int y, char znak, unsigned int kolor);
void rysuj_tekst_8x8(int x, int y, const char *tekst, unsigned int kolor);

#endif