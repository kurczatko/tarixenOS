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
static uint8_t zmienianie_rozmiaru;
static int kierunek_zmiany;
static char schowek[512];
static int schowek_dlugosc;

static void narysuj_okno_bez_odswiezania(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane) return;
    rysuj_prostokat(okno->x, okno->y, okno->szerokosc, okno->wysokosc, KOLOR_RAMKI);
    rysuj_prostokat(okno->x + 2, okno->y + 2, okno->szerokosc - 4, 20,
                    okno == aktywne_okno ? KOLOR_PASKA_AKTYWNE : KOLOR_PASKA);
    rysuj_prostokat(okno->x + 2, okno->y + 22, okno->szerokosc - 4, okno->wysokosc - 24, KOLOR_OKNA);
    rysuj_tekst_chicago(okno->x + 8, okno->y + 8, okno->tytul, 0x00FFFFFF);
    rysuj_prostokat(okno->x + okno->szerokosc - 64, okno->y + 4, 16, 16, 0x004A5D6E);
    rysuj_prostokat(okno->x + okno->szerokosc - 60, okno->y + 10, 8, 2, 0x00FFFFFF);
    rysuj_prostokat(okno->x + okno->szerokosc - 42, okno->y + 4, 16, 16, 0x004A5D6E);
    if (!okno->zmaksymalizowane) {
        rysuj_prostokat(okno->x + okno->szerokosc - 38, okno->y + 8, 8, 8, 0x00FFFFFF);
    } else {
        rysuj_prostokat(okno->x + okno->szerokosc - 40, okno->y + 6, 8, 8, 0x00FFFFFF);
        rysuj_prostokat(okno->x + okno->szerokosc - 34, okno->y + 12, 8, 8, 0x00FFFFFF);
    }
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
    okno->x_poprzednie = x;
    okno->y_poprzednie = y;
    okno->szerokosc_poprzednia = szerokosc;
    okno->wysokosc_poprzednia = wysokosc;
    okno->tytul = tytul;
    okno->rysuj_zawartosc = rysuj_zawartosc;
    okno->obsluz_klik = 0;
    okno->pole_tekstowe_enter = 0;
    okno->pole_tekstowe_wyjdz = 0;
    okno->pole_tekstowe_znak = 0;
    okno->otwarte = 1;
    okno->zminimalizowane = 0;
    okno->zmaksymalizowane = 0;
    okno->w_polu_tekstowym = 0;
    okno->kursor_pozycja = 0;
    okno->zaznacz_poczatek = -1;
    okno->zaznacz_koniec = -1;
    okno->tekst_bufor = 0;
    okno->tekst_dlugosc = 0;
    okno->tekst_maks = 0;
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
    if (zmienianie_rozmiaru && aktywne_okno == okno) {
        zmienianie_rozmiaru = 0;
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
    if (!trafione) {
        if (aktywne_okno && aktywne_okno->otwarte && aktywne_okno->w_polu_tekstowym) {
            okno_wyjdz_z_pola_tekstowego(aktywne_okno);
        }
        return 0;
    }

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
        if (trafione->zmaksymalizowane) okno_przywroc_rozmiar(trafione);
        else okno_maksymalizuj(trafione);
        return 1;
    }
    if (x >= trafione->x + trafione->szerokosc - 64 &&
        x < trafione->x + trafione->szerokosc - 48 &&
        y >= trafione->y + 4 && y < trafione->y + 20) {
        okno_zminimalizuj(trafione);
        return 1;
    }
    if (!trafione->zmaksymalizowane &&
        x >= trafione->x + 2 && x < trafione->x + trafione->szerokosc - 64 &&
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
    if (!trafione->zmaksymalizowane) {
        if (x >= trafione->x + trafione->szerokosc - 4 &&
            y >= trafione->y + trafione->wysokosc - 4) {
            aktywne_okno = trafione;
            zmienianie_rozmiaru = 1;
            kierunek_zmiany = 3;
            return 1;
        }
        if (x >= trafione->x + trafione->szerokosc - 4 &&
            y > trafione->y + 20 && y < trafione->y + trafione->wysokosc - 4) {
            aktywne_okno = trafione;
            zmienianie_rozmiaru = 1;
            kierunek_zmiany = 1;
            return 1;
        }
        if (y >= trafione->y + trafione->wysokosc - 4 &&
            x > trafione->x + 4 && x < trafione->x + trafione->szerokosc - 4) {
            aktywne_okno = trafione;
            zmienianie_rozmiaru = 1;
            kierunek_zmiany = 2;
            return 1;
        }
    }
    if (!trafione->zmaksymalizowane &&
        y >= trafione->y + 22 && y < trafione->y + trafione->wysokosc - 2 &&
        x >= trafione->x + 4 && x < trafione->x + trafione->szerokosc - 4 &&
        trafione->pole_tekstowe_enter) {
        okno_wejdz_do_pola_tekstowego(trafione);
        return 1;
    }
    if (trafione->obsluz_klik) trafione->obsluz_klik(x - trafione->x, y - trafione->y);
    return 1;
}

void okno_obsluz_ruch(int x, int y)
{
    int nx;
    int ny;
    if (!przeciaganie && !zmienianie_rozmiaru) return;
    if (!aktywne_okno || !aktywne_okno->otwarte) return;

    if (przeciaganie) {
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

    if (zmienianie_rozmiaru) {
        int stare_x = aktywne_okno->x;
        int stare_y = aktywne_okno->y;
        int stare_w = aktywne_okno->szerokosc;
        int stare_h = aktywne_okno->wysokosc;
        int nowy_w = stare_w;
        int nowy_h = stare_h;
        if (kierunek_zmiany & 1) nowy_w = x - aktywne_okno->x;
        if (kierunek_zmiany & 2) nowy_h = y - aktywne_okno->y;
        if (nowy_w < 100) nowy_w = 100;
        if (nowy_h < 100) nowy_h = 100;
        if (aktywne_okno->x + nowy_w > EKRAN_SZEROKOSC) nowy_w = EKRAN_SZEROKOSC - aktywne_okno->x;
        if (aktywne_okno->y + nowy_h > EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) nowy_h = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - aktywne_okno->y;
        if (nowy_w == stare_w && nowy_h == stare_h) return;
        okno_odrysuj_scene(stare_x, stare_y, stare_w, stare_h, stare_x, stare_y, nowy_w, nowy_h);
        aktywne_okno->szerokosc = nowy_w;
        aktywne_okno->wysokosc = nowy_h;
        okno_odrysuj_scene(stare_x, stare_y, stare_w, stare_h, stare_x, stare_y, nowy_w, nowy_h);
    }
}

void okno_zakoncz_przeciaganie(void)
{
    int stare_x;
    int stare_y;
    int szerokosc;
    int wysokosc;

    if (!przeciaganie && !zmienianie_rozmiaru) return;

    przeciaganie = 0;
    zmienianie_rozmiaru = 0;

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

void okno_maksymalizuj(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane || okno->zmaksymalizowane) return;
    okno->x_poprzednie = okno->x;
    okno->y_poprzednie = okno->y;
    okno->szerokosc_poprzednia = okno->szerokosc;
    okno->wysokosc_poprzednia = okno->wysokosc;
    okno->x = 0;
    okno->y = 0;
    okno->szerokosc = EKRAN_SZEROKOSC;
    okno->wysokosc = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC;
    okno->zmaksymalizowane = 1;
    okno_przenies_na_wierzch(okno);
    okno_odrysuj_wszystkie();
}

void okno_przywroc_rozmiar(struct okno *okno)
{
    if (!okno || !okno->otwarte || !okno->zmaksymalizowane) return;
    okno->x = okno->x_poprzednie;
    okno->y = okno->y_poprzednie;
    okno->szerokosc = okno->szerokosc_poprzednia;
    okno->wysokosc = okno->wysokosc_poprzednia;
    okno->zmaksymalizowane = 0;
    okno_przenies_na_wierzch(okno);
    okno_odrysuj_wszystkie();
}

int okno_czy_zminimalizowane(const struct okno *okno)
{
    return okno && okno->zminimalizowane;
}

int okno_czy_zmaksymalizowane(const struct okno *okno)
{
    return okno && okno->zmaksymalizowane;
}

void okno_ustaw_pole_tekstowe_enter(struct okno *okno, okno_pole_tekstowe fn)
{
    if (okno) okno->pole_tekstowe_enter = fn;
}

void okno_ustaw_pole_tekstowe_wyjdz(struct okno *okno, okno_pole_tekstowe fn)
{
    if (okno) okno->pole_tekstowe_wyjdz = fn;
}

void okno_ustaw_pole_tekstowe_znak(struct okno *okno, okno_pole_tekstowe_znak fn)
{
    if (okno) okno->pole_tekstowe_znak = fn;
}

void okno_wejdz_do_pola_tekstowego(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->w_polu_tekstowym) return;
    okno->w_polu_tekstowym = 1;
    okno->kursor_pozycja = 0;
    okno->zaznacz_poczatek = -1;
    okno->zaznacz_koniec = -1;
    if (okno->pole_tekstowe_enter) okno->pole_tekstowe_enter(okno);
}

void okno_wyjdz_z_pola_tekstowego(struct okno *okno)
{
    if (!okno || !okno->otwarte || !okno->w_polu_tekstowym) return;
    okno->w_polu_tekstowym = 0;
    okno->kursor_pozycja = 0;
    okno->zaznacz_poczatek = -1;
    okno->zaznacz_koniec = -1;
    if (okno->pole_tekstowe_wyjdz) okno->pole_tekstowe_wyjdz(okno);
}

int okno_czy_w_polu_tekstowym(const struct okno *okno)
{
    return okno && okno->w_polu_tekstowym;
}

int okno_pobierz_kursor(const struct okno *okno)
{
    return okno ? okno->kursor_pozycja : 0;
}

void okno_ustaw_kursor(struct okno *okno, int pozycja)
{
    if (okno) okno->kursor_pozycja = pozycja;
}

void okno_zaznacz_tekst(struct okno *okno, int poczatek, int koniec)
{
    if (!okno) return;
    okno->zaznacz_poczatek = poczatek;
    okno->zaznacz_koniec = koniec;
}

void okno_pobierz_zaznaczenie(const struct okno *okno, int *poczatek, int *koniec)
{
    if (!okno) return;
    if (poczatek) *poczatek = okno->zaznacz_poczatek;
    if (koniec) *koniec = okno->zaznacz_koniec;
}

int okno_skopiuj_zaznaczenie(struct okno *okno)
{
    if (!okno || !okno->tekst_bufor) return 0;
    int poczatek = okno->zaznacz_poczatek;
    int koniec = okno->zaznacz_koniec;
    if (poczatek < 0 || koniec < 0 || poczatek >= koniec) return 0;
    if (poczatek >= okno->tekst_dlugosc) return 0;
    if (koniec > okno->tekst_dlugosc) koniec = okno->tekst_dlugosc;
    int dlugosc = koniec - poczatek;
    if (dlugosc > 511) dlugosc = 511;
    for (int i = 0; i < dlugosc; i++) schowek[i] = okno->tekst_bufor[poczatek + i];
    schowek[dlugosc] = 0;
    schowek_dlugosc = dlugosc;
    return 1;
}

int okno_wklej_schowek(struct okno *okno)
{
    if (!okno || !okno->tekst_bufor || schowek_dlugosc <= 0) return 0;
    int pozycja = okno->kursor_pozycja;
    if (pozycja < 0) pozycja = 0;
    if (pozycja > okno->tekst_dlugosc) pozycja = okno->tekst_dlugosc;
    int dostepne = okno->tekst_maks - okno->tekst_dlugosc;
    if (dostepne <= 0) return 0;
    int wstaw = schowek_dlugosc;
    if (wstaw > dostepne) wstaw = dostepne;
    for (int i = okno->tekst_dlugosc; i >= pozycja; i--) {
        okno->tekst_bufor[i + wstaw] = okno->tekst_bufor[i];
    }
    for (int i = 0; i < wstaw; i++) {
        okno->tekst_bufor[pozycja + i] = schowek[i];
    }
    okno->tekst_dlugosc += wstaw;
    okno->tekst_bufor[okno->tekst_dlugosc] = 0;
    okno->kursor_pozycja = pozycja + wstaw;
    return 1;
}

void okno_ustaw_bufor_tekstu(struct okno *okno, char *bufor, int dlugosc, int maks)
{
    if (!okno) return;
    okno->tekst_bufor = bufor;
    okno->tekst_dlugosc = dlugosc;
    okno->tekst_maks = maks;
}

void okno_pobierz_bufor_tekstu(struct okno *okno, char **bufor, int *dlugosc)
{
    if (!okno) return;
    if (bufor) *bufor = okno->tekst_bufor;
    if (dlugosc) *dlugosc = okno->tekst_dlugosc;
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
