#ifndef OKNO_USER_H
#define OKNO_USER_H

#include "../include/printf/types.h"

#define SYSCALL_EKRAN_PIKSEL 3
#define SYSCALL_EKRAN_PROSTOKAT 4
#define SYSCALL_TEKST_RYSUJ 5
#define SYSCALL_OKNO_STWORZ 6
#define SYSCALL_OKNO_RYSUJ 7
#define SYSCALL_MYSZ_POZYCJA 8
#define SYSCALL_OKNO_ZAMKNIJ 35
#define SYSCALL_OKNO_PIKSEL 36
#define SYSCALL_OKNO_PROSTOKAT 37
#define SYSCALL_OKNO_TEKST 38
#define SYSCALL_OKNO_KLIK 39
#define SYSCALL_OKNO_RUCH 40
#define SYSCALL_KLAWIATURA_GETCHAR 41
#define SYSCALL_WRITE_CONSOLE 20
#define SYSCALL_EXIT 21
#define SYSCALL_BRK 22
#define SYSCALL_OPEN 23
#define SYSCALL_READ 24
#define SYSCALL_CLOSE 25
#define SYSCALL_WRITE_FILE 26
#define SYSCALL_PROCES_STWORZ 27
#define SYSCALL_PROCES_ZAKONCZ 28
#define SYSCALL_PROCES_YIELD 29
#define SYSCALL_PROCES_POBIERZ_PID 30
#define SYSCALL_FS_LIST 31
#define SYSCALL_FS_CD 32
#define SYSCALL_FS_PWD 33
#define SYSCALL_FS_RESET_DIR 34
#define SYSCALL_PROCES_URUCHOM_ELF 42
#define SYSCALL_PROCES_YIELD 29

#define KOLOR_CZARNY    0x00000000
#define KOLOR_BIALY     0x00FFFFFF
#define KOLOR_SZARY     0x00808080
#define KOLOR_JASNOSZARY 0x00C0C0C0
#define KOLOR_NIEBIESKI 0x000000FF
#define KOLOR_CZERWONY  0x00FF0000
#define KOLOR_ZIELONY   0x0000FF00
#define KOLOR_ZOLTY     0x00FFFF00
#define KOLOR_RAMKI     0x00787C82
#define KOLOR_PASKA     0x00787C82
#define KOLOR_PASKA_AKTYWNE 0x003B82F6
#define KOLOR_OKNA      0x00FFFFFF
#define KOLOR_X         0x00E74C3C
#define KOLOR_PRZYCISK_MIN  0x004A5D6E
#define KOLOR_PRZYCISK_MAX  0x004A5D6E
#define KOLOR_PRZYCISK_ZAMKNIJ 0x00E74C3C

#define EKRAN_SZEROKOSC 1024
#define EKRAN_WYSOKOSC  768
#define PASKA_WYSOKOSC  22
#define RAMKA_GRUBOSC   2
#define PRZYCISK_ROZMIAR 16

#define OKNA_MAKS 8

typedef int okno_handle_t;

typedef void (*okno_rysuj_zawartosc_fn)(int x, int y, int szerokosc, int wysokosc);
typedef void (*okno_klik_fn)(int x, int y);
typedef void (*okno_pole_tekstowe_fn)(void);
typedef int (*okno_pole_tekstowe_znak_fn)(char znak, uint8_t ctrl);

struct okno_user {
    okno_handle_t handle;
    int x;
    int y;
    int szerokosc;
    int wysokosc;
    int x_poprzednie;
    int y_poprzednie;
    int szerokosc_poprzednia;
    int wysokosc_poprzednia;
    const char *tytul;
    okno_rysuj_zawartosc_fn rysuj_zawartosc;
    okno_klik_fn obsluz_klik;
    okno_pole_tekstowe_fn pole_tekstowe_enter;
    okno_pole_tekstowe_fn pole_tekstowe_wyjdz;
    okno_pole_tekstowe_znak_fn pole_tekstowe_znak;
    char *tekst_bufor;
    int tekst_dlugosc;
    int tekst_maks;
    uint8_t otwarte;
    uint8_t zminimalizowane;
    uint8_t zmaksymalizowane;
    uint8_t w_polu_tekstowym;
    int kursor_pozycja;
    int zaznacz_poczatek;
    int zaznacz_koniec;
};

typedef struct okno_user okno_user_t;

extern okno_user_t *obecne_okno;
extern okno_user_t *aktywne_okno;
extern okno_user_t *okna_lista[OKNA_MAKS];
extern int okna_liczba;

okno_handle_t okno_stworz_user(okno_user_t *okno, int x, int y, int w, int h, const char *tytul, void (*rysuj_zawartosc)(int x, int y, int w, int h));
void okno_rysuj_user(okno_user_t *okno);
void okno_zamknij_user(okno_user_t *okno);
int okno_czy_otwarte_user(okno_user_t *okno);
void okno_ustaw_aktywne_user(okno_user_t *okno);
int okno_czy_aktywne_user(okno_user_t *okno);
void okno_zminimalizuj_user(okno_user_t *okno);
void okno_przywroc_user(okno_user_t *okno);
void okno_maksymalizuj_user(okno_user_t *okno);
void okno_przywroc_rozmiar_user(okno_user_t *okno);
int okno_czy_zminimalizowane_user(okno_user_t *okno);
int okno_czy_zmaksymalizowane_user(okno_user_t *okno);

void okno_przetworz_zdarzenia_user(void);
int okno_obsluz_klikniecie_user(int x, int y);
void okno_obsluz_ruch_user(int x, int y);
void okno_zakoncz_przeciaganie_user(void);

void okno_ustaw_obsluge_klikniecia_user(okno_user_t *okno, okno_klik_fn fn);
void okno_ustaw_pole_tekstowe_enter_user(okno_user_t *okno, okno_pole_tekstowe_fn fn);
void okno_ustaw_pole_tekstowe_wyjdz_user(okno_user_t *okno, okno_pole_tekstowe_fn fn);
void okno_ustaw_pole_tekstowe_znak_user(okno_user_t *okno, okno_pole_tekstowe_znak_fn fn);
void okno_ustaw_bufor_tekstu_user(okno_user_t *okno, char *bufor, int dlugosc, int maks);

int okno_czy_w_polu_tekstowym_user(okno_user_t *okno);
int okno_pobierz_kursor_user(okno_user_t *okno);
void okno_ustaw_kursor_user(okno_user_t *okno, int pozycja);
void okno_zaznacz_tekst_user(okno_user_t *okno, int poczatek, int koniec);
void okno_pobierz_zaznaczenie_user(okno_user_t *okno, int *poczatek, int *koniec);
int okno_skopiuj_zaznaczenie_user(okno_user_t *okno);
int okno_wklej_schowek_user(okno_user_t *okno);
void okno_wejdz_do_pola_tekstowego_user(okno_user_t *okno);
void okno_wyjdz_z_pola_tekstowego_user(okno_user_t *okno);

void rysuj_piksel_user(okno_user_t *okno, int x, int y, uint32_t color);
void rysuj_prostokat_user(okno_user_t *okno, int x, int y, int w, int h, uint32_t color);
void rysuj_tekst_user(okno_user_t *okno, int x, int y, const char *text, uint32_t color);
void rysuj_znak_user(okno_user_t *okno, int x, int y, char znak, uint32_t color);

void rysuj_piksel_ekran_user(int x, int y, uint32_t color);
void rysuj_prostokat_ekran_user(int x, int y, int w, int h, uint32_t color);
void rysuj_tekst_ekran_user(int x, int y, const char *text, uint32_t color);

okno_handle_t okno_stworz(int x, int y, int w, int h, const char *tytul, okno_rysuj_zawartosc_fn rysuj_zawartosc);
void okno_rysuj(void);
void okno_zamknij(void);
int okno_czy_otwarte(void);
void okno_ustaw_obsluge_klikniecia(okno_klik_fn fn);
void okno_ustaw_pole_tekstowe_enter(okno_pole_tekstowe_fn fn);
void okno_ustaw_pole_tekstowe_wyjdz(okno_pole_tekstowe_fn fn);
void okno_ustaw_pole_tekstowe_znak(okno_pole_tekstowe_znak_fn fn);
void okno_ustaw_bufor_tekstu(char *bufor, int dlugosc, int maks);

void rysuj_piksel(int x, int y, uint32_t color);
void rysuj_prostokat(int x, int y, int w, int h, uint32_t color);
void rysuj_tekst(int x, int y, const char *text, uint32_t color);
void rysuj_znak(int x, int y, char znak, uint32_t color);

int klawiatura_getchar_user(char *znak, uint8_t *ctrl);
int mysz_pozycja_user(int *x, int *y);
void proces_yield_user(void);

int okno_liczba_okien_user(void);
okno_user_t *okno_pobierz_okno_user(int indeks);

#endif
