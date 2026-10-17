#include "okno.h"
#include "czcionka.h"
#include "rysowanie.h"

#define KOLOR_RAMKI 0x00787C82
#define KOLOR_PASKA 0x00787C82
#define KOLOR_PASKA_AKTYWNE 0x003B82F6
#define KOLOR_OKNA  0x00FFFFFF
#define KOLOR_X     0x00E74C3C
#define OKNA_MAKS 8

struct okno *aktywne_okno;
static struct okno *okna[OKNA_MAKS];
static int liczba_okien;
static uint8_t przeciaganie;
static int przesuniecie_x;
static int przesuniecie_y;
static int ramka_x;
static int ramka_y;
static uint8_t ramka_widoczna;

static void narysuj_okno_bez_odswiezania(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane) return;
    rysuj_prostokat(okno->x, okno->y, okno->szerokosc, okno->wysokosc, KOLOR_RAMKI);
    rysuj_prostokat(okno->x + 2, okno->y + 2, okno->szerokosc - 4, 20,
                    okno == aktywne_okno ? KOLOR_PASKA_AKTYWNE : KOLOR_PASKA);
    rysuj_prostokat(okno->x + 2, okno->y + 22, okno->szerokosc - 4, okno->wysokosc - 24, KOLOR_OKNA);
    rysuj_tekst_chicago(okno->x + 8, okno->y + 8, okno->tytul, 0x00FFFFFF);
    rysuj_prostokat(okno->x + okno->szerokosc - 42, okno->y + 4, 16, 16, 0x004A5D6E);
    rysuj_prostokat(okno->x + okno->szerokosc - 38, okno->y + 10, 8, 2, 0x00FFFFFF);
    rysuj_prostokat(okno->x + okno->szerokosc - 22, okno->y + 4, 16, 16, KOLOR_X);
    rysuj_znak_chicago(okno->x + okno->szerokosc - 18, okno->y + 8, 'X', 0x00FFFFFF);
    if (okno->rysuj_zawartosc)
        okno->rysuj_zawartosc(okno->x + 8, okno->y + 30, okno->szerokosc - 16, okno->wysokosc - 38);
}

static void okno_przenies_na_wierzch(struct okno *okno)
{
    int indeks;
    for (indeks = 0; indeks < liczba_okien; indeks++) {
        if (okna[indeks] == okno) break;
    }
    if (indeks == liczba_okien) {
        if (liczba_okien == OKNA_MAKS) return;
        okna[liczba_okien++] = okno;
        indeks = liczba_okien - 1;
    }
    for (; indeks < liczba_okien - 1; indeks++) okna[indeks] = okna[indeks + 1];
    okna[liczba_okien - 1] = okno;
    aktywne_okno = okno;
}

static void odrysuj_okna_w_obszarze(int x, int y, int szerokosc, int wysokosc)
{
    odtworz_fragment_tla_bufor(x, y, szerokosc, wysokosc);
    for (int indeks = 0; indeks < liczba_okien; indeks++) {
        struct okno *okno = okna[indeks];
        if (!okno || !okno->otwarte || okno->zminimalizowane || okno->x >= x + szerokosc ||
            okno->x + okno->szerokosc <= x || okno->y >= y + wysokosc ||
            okno->y + okno->wysokosc <= y) continue;
        narysuj_okno_bez_odswiezania(okno);
    }
}

static void okno_odrysuj_scene(int x1, int y1, int szerokosc1, int wysokosc1,
                               int x2, int y2, int szerokosc2, int wysokosc2)
{
    int lewy = x1 < x2 ? x1 : x2;
    int gora = y1 < y2 ? y1 : y2;
    int prawy1 = x1 + szerokosc1;
    int prawy2 = x2 + szerokosc2;
    int dol1 = y1 + wysokosc1;
    int dol2 = y2 + wysokosc2;
    int prawy;
    int dol;

    if (szerokosc1 <= 0 || wysokosc1 <= 0) {
        lewy = x2;
        gora = y2;
        prawy1 = x2 + szerokosc2;
        dol1 = y2 + wysokosc2;
        prawy2 = prawy1;
        dol2 = dol1;
    } else if (szerokosc2 <= 0 || wysokosc2 <= 0) {
        prawy2 = prawy1;
        dol2 = dol1;
    }
    prawy = prawy1 > prawy2 ? prawy1 : prawy2;
    dol = dol1 > dol2 ? dol1 : dol2;
    if (prawy <= lewy || dol <= gora) return;

    odrysuj_okna_w_obszarze(lewy, gora, prawy - lewy, dol - gora);
    odswiez_fragment_widoku(lewy, gora, prawy - lewy, dol - gora);
}

static void okno_odrysuj_wszystkie(void)
{
    for (int indeks = 0; indeks < liczba_okien; indeks++) {
        struct okno *okno = okna[indeks];
        if (okno && okno->otwarte)
            odtworz_fragment_tla_bufor(okno->x, okno->y, okno->szerokosc, okno->wysokosc);
    }
    for (int indeks = 0; indeks < liczba_okien; indeks++)
        if (okna[indeks] && okna[indeks]->otwarte && !okna[indeks]->zminimalizowane)
            narysuj_okno_bez_odswiezania(okna[indeks]);
    pasek_zadan_rysuj_ikony_minimalizowanych();
    odswiez_widok();
}

static void narysuj_przerywana_ramke(int x, int y, int poprzedni_x, int poprzedni_y,
                                     int szerokosc, int wysokosc)
{
    int obszar_x = poprzedni_x < x ? poprzedni_x : x;
    int obszar_y = poprzedni_y < y ? poprzedni_y : y;
    int obszar_prawy = (poprzedni_x + szerokosc) > (x + szerokosc) ?
                       poprzedni_x + szerokosc : x + szerokosc;
    int obszar_dol = (poprzedni_y + wysokosc) > (y + wysokosc) ?
                     poprzedni_y + wysokosc : y + wysokosc;
    odrysuj_okna_w_obszarze(obszar_x, obszar_y, obszar_prawy - obszar_x,
                            obszar_dol - obszar_y);
    for (int i = 0; i < szerokosc; i++) {
        uint32_t kolor = ((i / 4) % 2 == 0) ? 0x00000000 : 0x00FFFFFF;
        rysuj_piksel(x + i, y, kolor);
        rysuj_piksel(x + i, y + wysokosc - 1, kolor);
    }
    for (int i = 0; i < wysokosc; i++) {
        uint32_t kolor = ((i / 4) % 2 == 0) ? 0x00000000 : 0x00FFFFFF;
        rysuj_piksel(x, y + i, kolor);
        rysuj_piksel(x + szerokosc - 1, y + i, kolor);
    }
    odswiez_fragment_widoku(obszar_x, obszar_y, obszar_prawy - obszar_x,
                            obszar_dol - obszar_y);
}

void okno_ustaw_aktywne(struct okno *okno)
{
    if (okno && okno->otwarte) okno_przenies_na_wierzch(okno);
    else aktywne_okno = okno;
}

int okno_czy_aktywne(const struct okno *okno)
{
    return okno && okno == aktywne_okno && okno->otwarte;
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
    okno_przenies_na_wierzch(okno);
    okno_odrysuj_wszystkie();
}

void okno_ustaw_obsluge_klikniecia(struct okno *okno, okno_obsluz_klik obsluz_klik)
{
    if (okno) okno->obsluz_klik = obsluz_klik;
}

void okno_narysuj(struct okno *okno)
{
    if (!okno || !okno->otwarte) return;
    okno_odrysuj_wszystkie();
}

void okno_zamknij(struct okno *okno)
{
    if (!okno || !okno->otwarte) return;
    if (przeciaganie && aktywne_okno == okno) {
        okno_zakoncz_przeciaganie();
    }
    okno->otwarte = 0;
    for (int indeks = 0; indeks < liczba_okien; indeks++) {
        if (okna[indeks] == okno) {
            for (int pozostale = indeks; pozostale < liczba_okien - 1; pozostale++)
                okna[pozostale] = okna[pozostale + 1];
            okna[--liczba_okien] = 0;
            break;
        }
    }
    if (aktywne_okno == okno)
        aktywne_okno = liczba_okien ? okna[liczba_okien - 1] : 0;
    okno_odrysuj_wszystkie();
}

int okno_obsluz_klikniecie(int x, int y)
{
    struct okno *trafione = 0;
    for (int indeks = liczba_okien - 1; indeks >= 0; indeks--) {
        struct okno *kandydat = okna[indeks];
        if (kandydat && kandydat->otwarte && x >= kandydat->x &&
            x < kandydat->x + kandydat->szerokosc && y >= kandydat->y &&
            y < kandydat->y + kandydat->wysokosc) {
            trafione = kandydat;
            break;
        }
    }
    if (!trafione) return 0;

    if (trafione != aktywne_okno) {
        okno_przenies_na_wierzch(trafione);
        okno_odrysuj_wszystkie();
    }

    if (x >= trafione->x + trafione->szerokosc - 22 &&
        x < trafione->x + trafione->szerokosc - 6 &&
        y >= trafione->y + 4 && y < trafione->y + 20) {
        okno_zamknij(trafione);
        return 1;
    }
    if (x >= trafione->x + trafione->szerokosc - 42 &&
        x < trafione->x + trafione->szerokosc - 26 &&
        y >= trafione->y + 4 && y < trafione->y + 20) {
        okno_zminimalizuj(trafione);
        return 1;
    }
    if (x >= trafione->x + 2 && x < trafione->x + trafione->szerokosc - 22 &&
        y >= trafione->y + 2 && y < trafione->y + 22) {
        aktywne_okno = trafione;
        przeciaganie = 1;
        przesuniecie_x = x - trafione->x;
        przesuniecie_y = y - trafione->y;
        ramka_x = trafione->x;
        ramka_y = trafione->y;
        ramka_widoczna = 1;
        narysuj_przerywana_ramke(ramka_x, ramka_y, ramka_x, ramka_y,
                     trafione->szerokosc, trafione->wysokosc);
        return 1;
    }
    if (trafione->obsluz_klik) trafione->obsluz_klik(x - trafione->x, y - trafione->y);
    return 1;
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

    int stara_ramka_x = ramka_x;
    int stara_ramka_y = ramka_y;
    ramka_x = nx;
    ramka_y = ny;
    ramka_widoczna = 1;

    narysuj_przerywana_ramke(ramka_x, ramka_y, stara_ramka_x, stara_ramka_y,
                             aktywne_okno->szerokosc, aktywne_okno->wysokosc);
}

void okno_zakoncz_przeciaganie(void)
{
    int stare_x;
    int stare_y;
    int szerokosc;
    int wysokosc;

    if (!przeciaganie) return;

    przeciaganie = 0;

    if (!aktywne_okno || !aktywne_okno->otwarte) return;

    stare_x = aktywne_okno->x;
    stare_y = aktywne_okno->y;
    szerokosc = aktywne_okno->szerokosc;
    wysokosc = aktywne_okno->wysokosc;
    aktywne_okno->x = ramka_x;
    aktywne_okno->y = ramka_y;
    ramka_widoczna = 0;
    okno_odrysuj_scene(stare_x, stare_y, szerokosc, wysokosc,
                       ramka_x, ramka_y, szerokosc, wysokosc);
}

void okno_zminimalizuj(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane) return;
    okno->zminimalizowane = 1;
    if (aktywne_okno == okno) {
        aktywne_okno = 0;
        for (int i = liczba_okien - 1; i >= 0; i--) {
            if (okna[i] && okna[i]->otwarte && !okna[i]->zminimalizowane && okna[i] != okno) {
                aktywne_okno = okna[i];
                break;
            }
        }
    }
    okno_odrysuj_wszystkie();
}

void okno_przywroc(struct okno *okno)
{
    if (!okno || !okno->otwarte || !okno->zminimalizowane) return;
    okno->zminimalizowane = 0;
    okno_przenies_na_wierzch(okno);
    okno_odrysuj_wszystkie();
}

int okno_czy_zminimalizowane(const struct okno *okno)
{
    return okno && okno->zminimalizowane;
}

int okno_liczba_okien(void)
{
    return liczba_okien;
}

struct okno *okno_pobierz_okno(int indeks)
{
    if (indeks >= 0 && indeks < liczba_okien) return okna[indeks];
    return 0;
}