#ifndef RYSOWANIE_H
#define RYSOWANIE_H

#include "../include/printf/types.h"

struct okno;

#define PASEK_ICON_START_X 80

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
void odswiez_fragment_widoku(int x, int y, int szerokosc, int wysokosc);
void odtworz_fragment_tla(int x, int y, int szerokosc, int wysokosc);
void odtworz_fragment_tla_bufor(int x, int y, int szerokosc, int wysokosc);
void ustaw_kolor_tapety(uint32_t kolor);
void ustaw_status_internetu(uint8_t dostepny);
void ustaw_czas_paska(const char *czas);
int tapeta_wczytaj(void);
void tapeta_zapisz(void);
void pasek_zadan_rysuj_ikony_minimalizowanych(void);
struct okno *pasek_zadan_ikona_na_pozycji(int x, int y);
void kursor_tekstowy_tick(void);
uint8_t kursor_tekstowy_widoczny(void);
void kursor_tekstowy_rysuj(int x, int y);

#endif