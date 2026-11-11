#ifndef RYSOWANIE_H
#define RYSOWANIE_H

#include "../include/printf/types.h"

struct okno;

#define PASEK_ICON_START_X 80

#define EKRAN_SZEROKOSC 1920
#define EKRAN_WYSOKOSC 1080
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
void rozmyj_prostokat(int x, int y, int szerokosc, int wysokosc);
void rysuj_prostokat_polprzezroczysty(int x, int y, int szerokosc, int wysokosc, uint32_t kolor, uint32_t alpha);
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
uint32_t pobierz_kolor_tapety(void);
void ustaw_kolor_tapety(uint32_t kolor);
void ustaw_status_internetu(uint8_t dostepny);
void ustaw_czas_paska(const char *czas);
int tapeta_wczytaj(void);
void tapeta_zapisz(void);
int tapeta_czy_jasna(void);
int tapeta_ustaw_obraz(int jasna);
void pasek_zadan_rysuj_ikony_minimalizowanych(void);
struct okno *pasek_zadan_ikona_na_pozycji(int x, int y);
void pasek_zadan_rysuj_ikone(int x, int y, const char *tytul);
void kursor_tekstowy_tick(void);
uint8_t kursor_tekstowy_widoczny(void);
void kursor_tekstowy_rysuj(int x, int y);

#define KAMERA_IKONA_X (EKRAN_SZEROKOSC - 74)
#define KAMERA_IKONA_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 4)
#define KAMERA_IKONA_W 24
#define KAMERA_IKONA_H 22

void quickcam_ui_init(void);
void kamera_ui_init(void);
void quickcam_ui_rysuj_pasek(void);
int quickcam_ui_icon_at(int x, int y);
int quickcam_ui_mouse_click(int x, int y);
void quickcam_ui_toggle(void);
void kamera_ui_toggle(void);

#endif