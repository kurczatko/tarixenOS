#ifndef RYSOWANIE_H
#define RYSOWANIE_H

#include "../include/printf/types.h"

#define EKRAN_SZEROKOSC 640
#define EKRAN_WYSOKOSC 480
#define PASEK_ZADAN_WYSOKOSC 20

void rysowanie_init(void);
void narysuj_pasek_zadan(void);
void wypelnij_ekran(uint8_t kolor);
void rysuj_piksel(int x, int y, uint8_t kolor);
void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint8_t kolor);
void rysuj_kwadrat(int x, int y, int bok, uint8_t kolor);
void kursor_graficzny_init(void);
void kursor_graficzny_przesun(int delta_x, int delta_y);

#endif