#ifndef CZCIONKA_H
#define CZCIONKA_H

#include "../include/printf/types.h"

extern const uint8_t font_8x8[128][8];

void rysuj_tekst_8x8(int x, int y, const char *tekst, uint32_t kolor);
void rysuj_znak_8x8(int x, int y, char znak, uint32_t kolor);

#endif