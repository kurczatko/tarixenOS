#include "okno.h"
#include "czcionka.h"
#include "rysowanie.h"

#define KOLOR_RAMKI 0x00787C82
#define KOLOR_PASKA 0x00787C82
#define KOLOR_OKNA  0x00FFFFFF
#define KOLOR_X     0x00E74C3C

struct okno *aktywne_okno;
static uint8_t przeciaganie;
static int przesuniecie_x;
static int przesuniecie_y;
static int ramka_x;
static int ramka_y;
static uint8_t ramka_widoczna;

static void narysuj_przerywana_ramke(int x, int y, int szerokosc, int wysokosc)
{
    int i;
    for (i = 0; i < szerokosc; i++) {
        if ((i / 4) % 2 == 0) {
            rysuj_piksel(x + i, y, 0x00000000);
            rysuj_piksel(x + i, y + wysokosc - 1, 0x00000000);
        } else {
            rysuj_piksel(x + i, y, 0x00FFFFFF);
            rysuj_piksel(x + i, y + wysokosc - 1, 0x00FFFFFF);
        }
    }
    for (i = 0; i < wysokosc; i++) {
        if ((i / 4) % 2 == 0) {
            rysuj_piksel(x, y + i, 0x00000000);
            rysuj_piksel(x + szerokosc - 1, y + i, 0x00000000);
        } else {
            rysuj_piksel(x, y + i, 0x00FFFFFF);
            rysuj_piksel(x + szerokosc - 1, y + i, 0x00FFFFFF);
        }
    }
    odswiez_widok();
}

static void zmaz_przerywana_ramke(int x, int y, int szerokosc, int wysokosc)
{
    int ox;
    int oy;
    int ow;
    int oh;

    odtworz_fragment_tla(x, y, szerokosc, 1);
    odtworz_fragment_tla(x, y + wysokosc - 1, szerokosc, 1);
    odtworz_fragment_tla(x, y, 1, wysokosc);
    odtworz_fragment_tla(x + szerokosc - 1, y, 1, wysokosc);

    if (aktywne_okno && aktywne_okno->otwarte) {
        ox = aktywne_okno->x;
        oy = aktywne_okno->y;
        ow = aktywne_okno->szerokosc;
        oh = aktywne_okno->wysokosc;

        if (x < ox + ow && x + szerokosc > ox && y < oy + oh && y + wysokosc > oy) {
            okno_narysuj(aktywne_okno);
        }
    }
}

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
    rysuj_tekst_8x8(okno->x + 8, okno->y + 8, okno->tytul, 0x00FFFFFF);
    rysuj_prostokat(okno->x + okno->szerokosc - 22, okno->y + 4, 16, 16, KOLOR_X);
    rysuj_znak_8x8(okno->x + okno->szerokosc - 18, okno->y + 8, 'X', 0x00FFFFFF);
    if (okno->rysuj_zawartosc)
        okno->rysuj_zawartosc(okno->x + 8, okno->y + 30, okno->szerokosc - 16, okno->wysokosc - 38);
    odswiez_widok();
}

void okno_zamknij(struct okno *okno)
{
    if (!okno || !okno->otwarte) return;
    if (przeciaganie && aktywne_okno == okno) {
        okno_zakoncz_przeciaganie();
    }
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
        ramka_x = aktywne_okno->x;
        ramka_y = aktywne_okno->y;
        ramka_widoczna = 1;
        narysuj_przerywana_ramke(ramka_x, ramka_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
    } else if (aktywne_okno->obsluz_klik) {
        aktywne_okno->obsluz_klik(x - aktywne_okno->x, y - aktywne_okno->y);
    }
}

void okno_obsluz_ruch(int x, int y)
{
    int nx;
    int ny;
    if (!przeciaganie || !aktywne_okno || !aktywne_okno->otwarte) return;

    nx = x - przesuniecie_x;
    ny = y - przesuniecie_y;

    if (nx < 0) nx = 0;
    if (ny < 0) ny = 0;
    if (nx + aktywne_okno->szerokosc > EKRAN_SZEROKOSC)
        nx = EKRAN_SZEROKOSC - aktywne_okno->szerokosc;
    if (ny + aktywne_okno->wysokosc > EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC)
        ny = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - aktywne_okno->wysokosc;

    if (nx == ramka_x && ny == ramka_y) return;

    if (ramka_widoczna) {
        zmaz_przerywana_ramke(ramka_x, ramka_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
    }

    ramka_x = nx;
    ramka_y = ny;
    ramka_widoczna = 1;

    narysuj_przerywana_ramke(ramka_x, ramka_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
}

void okno_zakoncz_przeciaganie(void)
{
    int stare_x;
    int stare_y;
    int szerokosc_napisu;
    int tekst_x;
    int tekst_y;

    if (!przeciaganie) return;

    przeciaganie = 0;

    if (ramka_widoczna) {
        zmaz_przerywana_ramke(ramka_x, ramka_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
        ramka_widoczna = 0;
    }

    if (!aktywne_okno || !aktywne_okno->otwarte) return;

    stare_x = aktywne_okno->x;
    stare_y = aktywne_okno->y;

    if (stare_x != ramka_x || stare_y != ramka_y) {
        odtworz_fragment_tla(stare_x, stare_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
        
        szerokosc_napisu = 27 * 8;
        tekst_x = EKRAN_SZEROKOSC - szerokosc_napisu - 16;
        tekst_y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 16;
        
        if (stare_x < tekst_x + szerokosc_napisu && stare_x + aktywne_okno->szerokosc > tekst_x &&
            stare_y < tekst_y + 8 && stare_y + aktywne_okno->wysokosc > tekst_y) {
            rysuj_tekst_8x8(tekst_x, tekst_y, "TarixenOS Tryb Graficzny", 0x00FFFFFF);
        }

        aktywne_okno->x = ramka_x;
        aktywne_okno->y = ramka_y;
        okno_narysuj(aktywne_okno);
    }
}