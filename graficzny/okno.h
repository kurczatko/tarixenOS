#ifndef OKNO_H
#define OKNO_H

#include "../include/printf/types.h"

typedef void (*okno_rysuj_zawartosc)(int x, int y, int szerokosc, int wysokosc);
typedef void (*okno_obsluz_klik)(int x, int y);
typedef void (*okno_pole_tekstowe)(struct okno *okno);
typedef int (*okno_pole_tekstowe_znak)(struct okno *okno, char znak, uint8_t ctrl);

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
    uint8_t w_polu_tekstowym;
    int kursor_pozycja;
    int zaznacz_poczatek;
    int zaznacz_koniec;
    char *tekst_bufor;
    int tekst_dlugosc;
    int tekst_maks;
    const char *tytul;
    okno_rysuj_zawartosc rysuj_zawartosc;
    okno_obsluz_klik obsluz_klik;
    okno_pole_tekstowe pole_tekstowe_enter;
    okno_pole_tekstowe pole_tekstowe_wyjdz;
    okno_pole_tekstowe_znak pole_tekstowe_znak;
};

void okno_stworz(struct okno *okno, int x, int y, int szerokosc, int wysokosc,
                 const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc);
void okno_narysuj(struct okno *okno);
void okno_zamknij(struct okno *okno);
int okno_obsluz_klikniecie(int x, int y);
void okno_obsluz_ruch(int x, int y);
void okno_zakoncz_przeciaganie(void);
void okno_ustaw_obsluge_klikniecia(struct okno *okno, okno_obsluz_klik obsluz_klik);
void okno_ustaw_pole_tekstowe_enter(struct okno *okno, okno_pole_tekstowe fn);
void okno_ustaw_pole_tekstowe_wyjdz(struct okno *okno, okno_pole_tekstowe fn);
void okno_ustaw_pole_tekstowe_znak(struct okno *okno, okno_pole_tekstowe_znak fn);
void okno_ustaw_aktywne(struct okno *okno);
int okno_czy_aktywne(const struct okno *okno);
void okno_zminimalizuj(struct okno *okno);
void okno_przywroc(struct okno *okno);
int okno_czy_zminimalizowane(const struct okno *okno);
void okno_maksymalizuj(struct okno *okno);
void okno_przywroc_rozmiar(struct okno *okno);
int okno_czy_zmaksymalizowane(const struct okno *okno);
void okno_wejdz_do_pola_tekstowego(struct okno *okno);
void okno_wyjdz_z_pola_tekstowego(struct okno *okno);
int okno_czy_w_polu_tekstowym(const struct okno *okno);
int okno_pobierz_kursor(const struct okno *okno);
void okno_ustaw_kursor(struct okno *okno, int pozycja);
void okno_zaznacz_tekst(struct okno *okno, int poczatek, int koniec);
void okno_pobierz_zaznaczenie(const struct okno *okno, int *poczatek, int *koniec);
int okno_skopiuj_zaznaczenie(struct okno *okno);
int okno_wklej_schowek(struct okno *okno);
void okno_ustaw_bufor_tekstu(struct okno *okno, char *bufor, int dlugosc, int maks);
void okno_pobierz_bufor_tekstu(struct okno *okno, char **bufor, int *dlugosc);
int okno_liczba_okien(void);
struct okno *okno_pobierz_okno(int indeks);

#endif