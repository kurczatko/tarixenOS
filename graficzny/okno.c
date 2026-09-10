#include "okno.h"
#include "czcionka.h"
#include "rysowanie.h"

#define KOLOR_RAMKI 15
#define KOLOR_PASKA 7
#define KOLOR_OKNA 3
#define KOLOR_X 4

static struct okno *aktywne_okno;
static uint8_t przeciaganie;
static int przesuniecie_x;
static int przesuniecie_y;

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
    okno->obsluz_klik = 0;
    okno->otwarte = 1;
    aktywne_okno = okno;
    okno_narysuj(okno);
}

void okno_ustaw_obsluge_klikniecia(struct okno *okno, okno_obsluz_klik obsluz_klik)
{
    if (okno) okno->obsluz_klik = obsluz_klik;
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
    odtworz_fragment_tla(okno->x, okno->y, okno->szerokosc, okno->wysokosc);
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
    else if (x >= aktywne_okno->x + 2 && x < aktywne_okno->x + aktywne_okno->szerokosc - 22 &&
             y >= aktywne_okno->y + 2 && y < aktywne_okno->y + 22) {
        przeciaganie = 1;
        przesuniecie_x = x - aktywne_okno->x;
        przesuniecie_y = y - aktywne_okno->y;
    } else if (aktywne_okno->obsluz_klik) {
        aktywne_okno->obsluz_klik(x - aktywne_okno->x, y - aktywne_okno->y);
    }
}

void okno_obsluz_ruch(int x, int y)
{
    int stare_x;
    int stare_y;
    if (!przeciaganie || !aktywne_okno || !aktywne_okno->otwarte) return;
    stare_x = aktywne_okno->x;
    stare_y = aktywne_okno->y;
    aktywne_okno->x = x - przesuniecie_x;
    aktywne_okno->y = y - przesuniecie_y;
    if (aktywne_okno->x < 0) aktywne_okno->x = 0;
    if (aktywne_okno->y < 0) aktywne_okno->y = 0;
    if (aktywne_okno->x + aktywne_okno->szerokosc > EKRAN_SZEROKOSC)
        aktywne_okno->x = EKRAN_SZEROKOSC - aktywne_okno->szerokosc;
    if (aktywne_okno->y + aktywne_okno->wysokosc > EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC)
        aktywne_okno->y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - aktywne_okno->wysokosc;
    odtworz_fragment_tla(stare_x, stare_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
    okno_narysuj(aktywne_okno);
}

void okno_zakoncz_przeciaganie(void)
{
    przeciaganie = 0;
}