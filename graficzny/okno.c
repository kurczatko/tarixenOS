#include "okno.h"
#include "czcionka.h"
#include "rysowanie.h"

#define KOLOR_RAMKI 15
#define KOLOR_PASKA 7
#define KOLOR_OKNA 3
#define KOLOR_X 4

static struct okno *aktywne_okno;

void okno_ustaw_aktywne(struct okno *okno)
{
    aktywne_okno = okno;
}

void okno_stworz(struct okno *okno, int x, int y, int szerokosc, int wysokosc,
                 const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc)
{
    okno->x = x;
    okno->y = y;
    okno->szerokosc = szerokosc;
    okno->wysokosc = wysokosc;
    okno->tytul = tytul;
    okno->rysuj_zawartosc = rysuj_zawartosc;
    okno->otwarte = 1;
    aktywne_okno = okno;
    okno_narysuj(okno);
}

void okno_narysuj(struct okno *okno)
{
    if (!okno || !okno->otwarte) return;
    rysuj_prostokat(okno->x, okno->y, okno->szerokosc, okno->wysokosc, KOLOR_RAMKI);
    rysuj_prostokat(okno->x + 2, okno->y + 2, okno->szerokosc - 4, 20, KOLOR_PASKA);
    rysuj_prostokat(okno->x + 2, okno->y + 22, okno->szerokosc - 4, okno->wysokosc - 24, KOLOR_OKNA);
    rysuj_tekst_8x8(okno->x + 8, okno->y + 8, okno->tytul, 0);
    rysuj_prostokat(okno->x + okno->szerokosc - 22, okno->y + 4, 16, 16, KOLOR_X);
    rysuj_znak_8x8(okno->x + okno->szerokosc - 18, okno->y + 8, 'X', 15);
    if (okno->rysuj_zawartosc)
        okno->rysuj_zawartosc(okno->x + 8, okno->y + 30, okno->szerokosc - 16, okno->wysokosc - 38);
    odswiez_widok();
}

void okno_zamknij(struct okno *okno)
{
    if (!okno || !okno->otwarte) return;
    okno->otwarte = 0;
    rysuj_prostokat(okno->x, okno->y, okno->szerokosc, okno->wysokosc, 1);
    odswiez_widok();
    if (aktywne_okno == okno) aktywne_okno = 0;
}

void okno_obsluz_klikniecie(int x, int y)
{
    if (!aktywne_okno || !aktywne_okno->otwarte) return;
    if (x >= aktywne_okno->x + aktywne_okno->szerokosc - 22 &&
        x < aktywne_okno->x + aktywne_okno->szerokosc - 6 &&
        y >= aktywne_okno->y + 4 && y < aktywne_okno->y + 20)
        okno_zamknij(aktywne_okno);
}