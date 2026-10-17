#ifndef OKNO_H
#define OKNO_H

#include "../include/printf/types.h"

typedef void (*okno_rysuj_zawartosc)(int x, int y, int szerokosc, int wysokosc);
typedef void (*okno_obsluz_klik)(int x, int y);

struct okno {
    int x;
    int y;
    int szerokosc;
    int wysokosc;
    int x_poprzednie;
    int y_poprzednie;
    int szerokosc_poprzednia;
    int wysokosc_poprzednia;
    uint8_t otwarte;
    uint8_t zminimalizowane;
    uint8_t zmaksymalizowane;
    const char *tytul;
    okno_rysuj_zawartosc rysuj_zawartosc;
    okno_obsluz_klik obsluz_klik;
};

void okno_stworz(struct okno *okno, int x, int y, int szerokosc, int wysokosc,
                 const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc);
void okno_narysuj(struct okno *okno);
void okno_zamknij(struct okno *okno);
int okno_obsluz_klikniecie(int x, int y);
void okno_obsluz_ruch(int x, int y);
void okno_zakoncz_przeciaganie(void);
void okno_ustaw_obsluge_klikniecia(struct okno *okno, okno_obsluz_klik obsluz_klik);
void okno_ustaw_aktywne(struct okno *okno);
int okno_czy_aktywne(const struct okno *okno);
void okno_zminimalizuj(struct okno *okno);
void okno_przywroc(struct okno *okno);
int okno_czy_zminimalizowane(const struct okno *okno);
void okno_maksymalizuj(struct okno *okno);
void okno_przywroc_rozmiar(struct okno *okno);
int okno_czy_zmaksymalizowane(const struct okno *okno);
int okno_liczba_okien(void);
struct okno *okno_pobierz_okno(int indeks);

#endif