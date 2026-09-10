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
    uint8_t otwarte;
    const char *tytul;
    okno_rysuj_zawartosc rysuj_zawartosc;
    okno_obsluz_klik obsluz_klik;
};

void okno_stworz(struct okno *okno, int x, int y, int szerokosc, int wysokosc,
                 const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc);
void okno_narysuj(struct okno *okno);
void okno_zamknij(struct okno *okno);
void okno_obsluz_klikniecie(int x, int y);
void okno_obsluz_ruch(int x, int y);
void okno_zakoncz_przeciaganie(void);
void okno_ustaw_obsluge_klikniecia(struct okno *okno, okno_obsluz_klik obsluz_klik);
void okno_ustaw_aktywne(struct okno *okno);

#endif