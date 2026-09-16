#ifndef RYSOWANIE_H
#define RYSOWANIE_H

#include "../include/printf/types.h"

#define EKRAN_SZEROKOSC 1024
#define EKRAN_WYSOKOSC 768
#define PASEK_ZADAN_WYSOKOSC 30

#define KOLOR_CZARNY    0x00000000
#define KOLOR_BIALY     0x00FFFFFF
#define KOLOR_SZARY     0x00808080
#define KOLOR_JASNOSZARY 0x00C0C0C0
#define KOLOR_NIEBIESKI 0x000000FF
#define KOLOR_CZERWONY  0x00FF0000
#define KOLOR_ZIELONY   0x0000FF00
#define KOLOR_ZOLTY     0x00FFFF00

void rysowanie_init(void);
void narysuj_pasek_zadan(void);
void wypelnij_ekran(uint32_t kolor);
void rysuj_piksel(int x, int y, uint32_t kolor);
void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint32_t kolor);
void rysuj_kwadrat(int x, int y, int bok, uint32_t kolor);
void kursor_graficzny_init(void);
void kursor_graficzny_przesun(int delta_x, int delta_y);
void kursor_graficzny_pozycja(int *x, int *y);
void odswiez_widok(void);
void odtworz_fragment_tla(int x, int y, int szerokosc, int wysokosc);
void ustaw_kolor_tapety(uint32_t kolor);

#endif