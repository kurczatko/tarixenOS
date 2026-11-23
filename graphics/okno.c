/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <graphics/okno.h>
#include <graphics/czcionka.h>
#include <graphics/rysowanie.h>
#include <graphics/pulpit.h>
#include <graphics/menu_start.h>

/* --- uklad gornego paska i docka --- */
#ifndef PASEK_GORNY_WYSOKOSC
#define PASEK_GORNY_WYSOKOSC 24
#endif
#define DOCK_WYSOKOSC_PX    60
#define DOCK_MARGINES_DOLNY 10
#define DOCK_PAS_Y          (EKRAN_WYSOKOSC - DOCK_MARGINES_DOLNY - DOCK_WYSOKOSC_PX)
#define OKNA_DOL_MAKS       (DOCK_PAS_Y - 6)
extern void menu_start_rysuj_dock(void);
extern int menu_start_czy_w_docku(int x, int y);
void okno_odrysuj_dolny_pas(void);

#define KOLOR_RAMKI 0x00787C82
#define KOLOR_PASKA 0x00787C82
#define KOLOR_PASKA_AKTYWNE 0x003B82F6
#define KOLOR_OKNA  0x00FFFFFF
#define OKNA_MAKS 8
#define GENIE_KLATKI 26
#define GENIE_CZEKAJ 250000  

#define GENIE_ICON_W 24
#define GENIE_ICON_SPACJE 4

#define OKNO_PROMIEN 8
#define KOLOR_BTN_ZAMKNIJ   0x00FF5F57
#define KOLOR_BTN_MINIM     0x00FEBC2E
#define KOLOR_BTN_MAKS      0x0028C840
#define KOLOR_BTN_NIEAKTYW  0x00B4B4B4
#define BTN_ROZMIAR 12
#define BTN_X(i) (8 + 20 * (i))   /* 0 = czerwony, 1 = zolty, 2 = zielony */
#define BTN_Y 6
#define TYTUL_X 68

/* 1 = dwa przebiegi rozmycia tresci okna (jak dawniej), 0 = jeden przebieg.
 * Drugi przebieg wplywa na ok. 12% koloru tresci, wiec praktycznie go nie widac,
 * a kosztuje tyle co pierwszy. */
#ifndef OKNO_PODWOJNE_ROZMYCIE
#define OKNO_PODWOJNE_ROZMYCIE 0
#endif

struct okno *aktywne_okno;
static struct okno *okna[OKNA_MAKS];
static int liczba_okien;
static uint8_t przeciaganie;
static int przesuniecie_x;
static int przesuniecie_y;
static uint8_t zmienianie_rozmiaru;
static int kierunek_zmiany;
static char schowek[512];
static int schowek_dlugosc;

/* ====================================================================
 * Przycinanie: podczas odswiezania fragmentu ekranu rysujemy (i rozmywamy)
 * tylko to, co lezy w odswiezanym prostokacie, a nie cale okna.
 * ==================================================================== */
static int klip_wlaczony;
static int klip_x1, klip_y1, klip_x2, klip_y2;

static void klip_ustaw(int x, int y, int w, int h)
{
    klip_wlaczony = 1;
    klip_x1 = x;
    klip_y1 = y;
    klip_x2 = x + w;
    klip_y2 = y + h;
}

static void klip_wylacz(void)
{
    klip_wlaczony = 0;
}

/* czy prostokat ma jakakolwiek czesc wspolna z obszarem przycinania */
static int klip_przecina(int x, int y, int w, int h)
{
    if (!klip_wlaczony) return 1;
    return x < klip_x2 && x + w > klip_x1 && y < klip_y2 && y + h > klip_y1;
}

/* ile pikseli odciac z danego wiersza rogu (i = 0 to najwyzszy wiersz) */
static int inset_rogu(int promien, int i)
{
    if (i >= promien) return 0;
    int d = 2 * (promien - i) - 1;
    int lim = 4 * promien * promien - d * d;
    int s = 0;
    while ((s + 1) * (s + 1) * 4 <= lim) s++;
    return promien - s;
}

/* alfa < 0 = rozmycie, alfa >= 100 = pelny kolor, inaczej polprzezroczysty */
static void wypelnij_pas(int x, int y, int w, int h, uint32_t kolor, int alfa)
{
    if (w <= 0 || h <= 0) return;
    if (klip_wlaczony) {
        int x2 = x + w;
        int y2 = y + h;
        if (x < klip_x1) x = klip_x1;
        if (y < klip_y1) y = klip_y1;
        if (x2 > klip_x2) x2 = klip_x2;
        if (y2 > klip_y2) y2 = klip_y2;
        if (x2 <= x || y2 <= y) return;
        w = x2 - x;
        h = y2 - y;
    }
    if (alfa < 0) rozmyj_prostokat(x, y, w, h);
    else if (alfa >= 100) rysuj_prostokat(x, y, w, h, kolor);
    else rysuj_prostokat_polprzezroczysty(x, y, w, h, kolor, alfa);
}

/* prostokat z zaokraglonymi rogami: rg = promien gornych, rd = promien dolnych */
static void pas_zaokraglony(int x, int y, int w, int h, int rg, int rd,
                            uint32_t kolor, int alfa)
{
    if (w <= 0 || h <= 0) return;
    if (!klip_przecina(x, y, w, h)) return;   /* szybkie odrzucenie */
    if (rg > w / 2) rg = w / 2;
    if (rd > w / 2) rd = w / 2;
    if (rg > h / 2) rg = h / 2;
    if (rd > h / 2) rd = h / 2;

    for (int i = 0; i < rg; i++) {
        int in = inset_rogu(rg, i);
        wypelnij_pas(x + in, y + i, w - 2 * in, 1, kolor, alfa);
    }
    if (h - rd > rg)
        wypelnij_pas(x, y + rg, w, h - rd - rg, kolor, alfa);
    for (int i = 0; i < rd; i++) {
        int wiersz = h - 1 - i;
        if (wiersz < rg) continue;
        int in = inset_rogu(rd, i);
        wypelnij_pas(x + in, y + wiersz, w - 2 * in, 1, kolor, alfa);
    }
}

static int w_przycisku(const struct okno *okno, int x, int y, int i)
{
    int px = okno->x + BTN_X(i);
    int py = okno->y + BTN_Y;
    return x >= px && x < px + BTN_ROZMIAR && y >= py && y < py + BTN_ROZMIAR;
}

static void narysuj_okno_bez_odswiezania(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane) return;
    if (!klip_przecina(okno->x, okno->y, okno->szerokosc, okno->wysokosc)) return;
    if (okno->pelnoekranowy) {
        if (okno->rysuj_zawartosc)
            okno->rysuj_zawartosc(okno->x, okno->y, okno->szerokosc, okno->wysokosc);
        return;
    }
    {
        int x = okno->x, y = okno->y;
        int w = okno->szerokosc, h = okno->wysokosc;
        int akt = (okno == aktywne_okno);
        int r  = okno->zmaksymalizowane ? 0 : OKNO_PROMIEN;      /* zmaksymalizowane = ostre rogi */
        int rp = okno->zmaksymalizowane ? 0 : OKNO_PROMIEN - 2;  /* promien wewnetrzny */

        pas_zaokraglony(x, y, w, h, r, r, 0, -1);                       /* rozmycie calosci */
        pas_zaokraglony(x, y, w, h, r, r, KOLOR_RAMKI, 60);             /* ramka */
        pas_zaokraglony(x + 2, y + 2, w - 4, 20, rp, 0,
                        akt ? KOLOR_PASKA_AKTYWNE : KOLOR_PASKA, 85);   /* pasek tytulu */
#if OKNO_PODWOJNE_ROZMYCIE
        pas_zaokraglony(x + 2, y + 22, w - 4, h - 24, 0, rp, 0, -1);    /* rozmycie tresci */
#endif
        pas_zaokraglony(x + 2, y + 22, w - 4, h - 24, 0, rp, KOLOR_OKNA, 70);

        /* przyciski w stylu macOS (po lewej): czerwony, zolty, zielony */
        pas_zaokraglony(x + BTN_X(0), y + BTN_Y, BTN_ROZMIAR, BTN_ROZMIAR, 6, 6,
                        akt ? KOLOR_BTN_ZAMKNIJ : KOLOR_BTN_NIEAKTYW, 100);
        pas_zaokraglony(x + BTN_X(1), y + BTN_Y, BTN_ROZMIAR, BTN_ROZMIAR, 6, 6,
                        akt ? KOLOR_BTN_MINIM : KOLOR_BTN_NIEAKTYW, 100);
        pas_zaokraglony(x + BTN_X(2), y + BTN_Y, BTN_ROZMIAR, BTN_ROZMIAR, 6, 6,
                        akt ? KOLOR_BTN_MAKS : KOLOR_BTN_NIEAKTYW, 100);

        if (klip_przecina(x + TYTUL_X, y + 6, w - TYTUL_X, 16))
            rysuj_tekst_chicago(x + TYTUL_X, y + 8, okno->tytul, 0x00FFFFFF);
        if (okno->rysuj_zawartosc && klip_przecina(x + 8, y + 30, w - 16, h - 38))
            okno->rysuj_zawartosc(x + 8, y + 30, w - 16, h - 38);
    }
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

/* tapeta + okna w prostokacie, BEZ docka */
static void odrysuj_okna_surowe(int x, int y, int szerokosc, int wysokosc,
                                const struct okno *pomin)
{
    odtworz_fragment_tla_bufor(x, y, szerokosc, wysokosc);
    klip_ustaw(x, y, szerokosc, wysokosc);
    for (int indeks = 0; indeks < liczba_okien; indeks++) {
        struct okno *okno = okna[indeks];
        if (!okno || okno == pomin || !okno->otwarte || okno->zminimalizowane ||
            okno->x >= x + szerokosc ||
            okno->x + okno->szerokosc <= x || okno->y >= y + wysokosc ||
            okno->y + okno->wysokosc <= y) continue;
        narysuj_okno_bez_odswiezania(okno);
    }
    klip_wylacz();
}

/* Czysta scena (tapeta + okna) w pasie, w ktorym stoi dock. Dock rozmywa
 * to, co ma pod soba, wiec zawsze rysujemy go na swiezo odtworzonej scenie. */
void okno_odrysuj_dolny_pas(void)
{
    odrysuj_okna_surowe(0, DOCK_PAS_Y, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC - DOCK_PAS_Y, 0);
}

/* tapeta + okna w prostokacie; jesli dotyka pasa docka, dock jest rysowany od nowa */
static void odrysuj_okna_w_obszarze(int x, int y, int szerokosc, int wysokosc,
                                    const struct okno *pomin)
{
    odrysuj_okna_surowe(x, y, szerokosc, wysokosc, pomin);
    if (y + wysokosc > DOCK_PAS_Y && y < EKRAN_WYSOKOSC) {
        odrysuj_okna_surowe(0, DOCK_PAS_Y, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC - DOCK_PAS_Y, pomin);
        menu_start_rysuj_dock();
        odswiez_fragment_widoku(0, DOCK_PAS_Y, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC - DOCK_PAS_Y);
    }
}

/* Czesciowe odswiezenie: tylko podany prostokat (pod gornym paskiem). */
static void odswiez_obszar(int x, int y, int w, int h)
{
    int x2 = x + w;
    int y2 = y + h;
    if (x < 0) x = 0;
    if (y < PASEK_GORNY_WYSOKOSC) y = PASEK_GORNY_WYSOKOSC;
    if (x2 > EKRAN_SZEROKOSC) x2 = EKRAN_SZEROKOSC;
    if (y2 > EKRAN_WYSOKOSC) y2 = EKRAN_WYSOKOSC;
    if (x2 <= x || y2 <= y) return;
    odrysuj_okna_w_obszarze(x, y, x2 - x, y2 - y, 0);
    odswiez_fragment_widoku(x, y, x2 - x, y2 - y);
}

static void odswiez_pasek(void)
{
    pasek_zadan_rysuj_ikony_minimalizowanych();
    narysuj_pasek_zadan();
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

    odrysuj_okna_w_obszarze(lewy, gora, prawy - lewy, dol - gora, 0);
    odswiez_fragment_widoku(lewy, gora, prawy - lewy, dol - gora);
}

void okno_odrysuj_wszystkie(void)
{
    klip_wylacz();
    for (int indeks = 0; indeks < liczba_okien; indeks++) {
        struct okno *okno = okna[indeks];
        if (okno && okno->otwarte)
            odtworz_fragment_tla_bufor(okno->x, okno->y, okno->szerokosc, okno->wysokosc);
    }
    for (int indeks = 0; indeks < liczba_okien; indeks++)
        if (okna[indeks] && okna[indeks]->otwarte && !okna[indeks]->zminimalizowane)
            narysuj_okno_bez_odswiezania(okna[indeks]);
    pasek_zadan_rysuj_ikony_minimalizowanych();
    narysuj_pasek_zadan();      /* gorny pasek + dock (dock rozmywa scene pod soba) */
    odswiez_widok();
}

void okno_ustaw_aktywne(struct okno *okno)
{
    if (okno && okno->otwarte) okno_przenies_na_wierzch(okno);
    else aktywne_okno = okno;
}

int okno_czy_aktywne(const struct okno *okno)
{
    return okno && okno == aktywne_okno && okno->otwarte && !okno->zminimalizowane;
}

void okno_stworz(struct okno *okno, int x, int y, int szerokosc, int wysokosc,
                 const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc)
{
    /* okno nie moze zaczynac sie pod gornym paskiem ani zjechac pod dock */
    if (y + wysokosc > OKNA_DOL_MAKS) y = OKNA_DOL_MAKS - wysokosc;
    if (y < PASEK_GORNY_WYSOKOSC) y = PASEK_GORNY_WYSOKOSC;
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
    okno->obsluz_ruch = 0;
    okno->pole_tekstowe_enter = 0;
    okno->pole_tekstowe_wyjdz = 0;
    okno->pole_tekstowe_znak = 0;
    okno->otwarte = 1;
    okno->zminimalizowane = 0;
    okno->zmaksymalizowane = 0;
    okno->pelnoekranowy = 0;
    okno->w_polu_tekstowym = 0;
    okno->kursor_pozycja = 0;
    okno->zaznacz_poczatek = -1;
    okno->zaznacz_koniec = -1;
    okno->tekst_bufor = 0;
    okno->tekst_dlugosc = 0;
    okno->tekst_maks = 0;
    okno_przenies_na_wierzch(okno);
    pulp_zglosz_zmiane();
    okno_odrysuj_wszystkie();
}

void okno_stworz_pelnoekranowy(struct okno *okno, const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc)
{
    okno->x = 0;
    okno->y = PASEK_GORNY_WYSOKOSC;
    okno->szerokosc = EKRAN_SZEROKOSC;
    okno->wysokosc = OKNA_DOL_MAKS - PASEK_GORNY_WYSOKOSC;
    okno->x_poprzednie = 0;
    okno->y_poprzednie = PASEK_GORNY_WYSOKOSC;
    okno->szerokosc_poprzednia = EKRAN_SZEROKOSC;
    okno->wysokosc_poprzednia = OKNA_DOL_MAKS - PASEK_GORNY_WYSOKOSC;
    okno->tytul = tytul;
    okno->rysuj_zawartosc = rysuj_zawartosc;
    okno->obsluz_klik = 0;
    okno->obsluz_ruch = 0;
    okno->pole_tekstowe_enter = 0;
    okno->pole_tekstowe_wyjdz = 0;
    okno->pole_tekstowe_znak = 0;
    okno->otwarte = 1;
    okno->zminimalizowane = 0;
    okno->zmaksymalizowane = 0;
    okno->pelnoekranowy = 1;
    okno->w_polu_tekstowym = 0;
    okno->kursor_pozycja = 0;
    okno->zaznacz_poczatek = -1;
    okno->zaznacz_koniec = -1;
    okno->tekst_bufor = 0;
    okno->tekst_dlugosc = 0;
    okno->tekst_maks = 0;
    okno_przenies_na_wierzch(okno);
    pulp_zglosz_zmiane();
    okno_odrysuj_wszystkie();
}

void okno_ustaw_obsluge_klikniecia(struct okno *okno, okno_obsluz_klik obsluz_klik)
{
    if (okno) okno->obsluz_klik = obsluz_klik;
}

void okno_ustaw_obsluge_ruchu(struct okno *okno, okno_obsluz_ruch_fn obsluz_ruch)
{
    if (okno) okno->obsluz_ruch = obsluz_ruch;
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
    int zamkniety_x = okno->x;
    int zamkniety_y = okno->y;
    int zamkniety_szerokosc = okno->szerokosc;
    int zamkniety_wysokosc = okno->wysokosc;
    okno->otwarte = 0;
    pulp_zglosz_zmiane();
    for (int indeks = 0; indeks < liczba_okien; indeks++) {
        if (okna[indeks] == okno) {
            for (int pozostale = indeks; pozostale < liczba_okien - 1; pozostale++)
                okna[pozostale] = okna[pozostale + 1];
            okna[--liczba_okien] = 0;
            break;
        }
    }
    odtworz_fragment_tla_bufor(zamkniety_x, zamkniety_y, zamkniety_szerokosc, zamkniety_wysokosc);
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

int okno_obsluz_klikniecie(int x, int y)
{
    struct okno *trafione = 0;
    /* dock jest ponad oknami: klik w dock nie trafia w okno pod nim */
    if (menu_start_czy_w_docku(x, y)) return 0;
    for (int indeks = liczba_okien - 1; indeks >= 0; indeks--) {
        struct okno *kandydat = okna[indeks];
        if (kandydat && kandydat->otwarte && !kandydat->zminimalizowane && x >= kandydat->x &&
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
        struct okno *poprzednie = aktywne_okno;
        okno_przenies_na_wierzch(trafione);
        /* zmienia sie tylko wyglad dwoch okien: tracacego i zdobywajacego fokus */
        if (poprzednie && poprzednie != trafione && poprzednie->otwarte && !poprzednie->zminimalizowane)
            odswiez_obszar(poprzednie->x, poprzednie->y, poprzednie->szerokosc, poprzednie->wysokosc);
        odswiez_obszar(trafione->x, trafione->y, trafione->szerokosc, trafione->wysokosc);
        odswiez_pasek();
    }

    if (trafione->pelnoekranowy) {
        if (trafione->obsluz_klik) trafione->obsluz_klik(x - trafione->x, y - trafione->y);
        return 1;
    }

    if (w_przycisku(trafione, x, y, 0)) {          /* czerwony: zamknij */
        okno_zamknij(trafione);
        return 1;
    }
    if (w_przycisku(trafione, x, y, 1)) {          /* zolty: minimalizuj */
        okno_zminimalizuj(trafione);
        return 1;
    }
    if (w_przycisku(trafione, x, y, 2)) {          /* zielony: powieksz / przywroc */
        if (trafione->zmaksymalizowane) okno_przywroc_rozmiar(trafione);
        else okno_maksymalizuj(trafione);
        return 1;
    }
    if (!trafione->zmaksymalizowane &&
        x >= trafione->x + 2 && x < trafione->x + trafione->szerokosc - 2 &&
        y >= trafione->y + 2 && y < trafione->y + 22) {
        aktywne_okno = trafione;
        przeciaganie = 1;
        przesuniecie_x = x - trafione->x;
        przesuniecie_y = y - trafione->y;
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
    if (!przeciaganie && !zmienianie_rozmiaru) {
        if (aktywne_okno && aktywne_okno->otwarte && aktywne_okno->obsluz_ruch) {
            int wx = x - aktywne_okno->x;
            int wy = y - aktywne_okno->y;
            if (wx >= 4 && wx < aktywne_okno->szerokosc - 4 &&
                wy >= 22 && wy < aktywne_okno->wysokosc - 2) {
                aktywne_okno->obsluz_ruch(aktywne_okno, wx, wy);
            }
        }
        return;
    }
    if (!aktywne_okno || !aktywne_okno->otwarte || aktywne_okno->zminimalizowane) return;

    if (przeciaganie) {
        nx = x - przesuniecie_x;
        ny = y - przesuniecie_y;

        if (nx < 0) nx = 0;
        if (ny < PASEK_GORNY_WYSOKOSC) ny = PASEK_GORNY_WYSOKOSC;
        if (nx + aktywne_okno->szerokosc > EKRAN_SZEROKOSC)
            nx = EKRAN_SZEROKOSC - aktywne_okno->szerokosc;
        /* okno moze wejsc pod dock, ale pasek tytulu zostaje nad nim */
        if (ny > DOCK_PAS_Y - 24) ny = DOCK_PAS_Y - 24;

        if (nx == aktywne_okno->x && ny == aktywne_okno->y) return;

        int stare_x = aktywne_okno->x;
        int stare_y = aktywne_okno->y;
        int szerokosc = aktywne_okno->szerokosc;
        int wysokosc = aktywne_okno->wysokosc;

        aktywne_okno->x = nx;
        aktywne_okno->y = ny;
        okno_odrysuj_scene(stare_x, stare_y, szerokosc, wysokosc,
                   nx, ny, szerokosc, wysokosc);
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
        if (aktywne_okno->y + nowy_h > EKRAN_WYSOKOSC) nowy_h = EKRAN_WYSOKOSC - aktywne_okno->y;
        if (nowy_w == stare_w && nowy_h == stare_h) return;
        aktywne_okno->szerokosc = nowy_w;
        aktywne_okno->wysokosc = nowy_h;
        /* jedno przerysowanie sumy starego i nowego obszaru (dawniej dwa) */
        okno_odrysuj_scene(stare_x, stare_y, stare_w, stare_h, stare_x, stare_y, nowy_w, nowy_h);
    }
}

void okno_zakoncz_przeciaganie(void)
{
    if (!przeciaganie && !zmienianie_rozmiaru) return;

    przeciaganie = 0;
    zmienianie_rozmiaru = 0;

    if (!aktywne_okno || !aktywne_okno->otwarte) return;

    odtworz_fragment_tla_bufor(0, 0, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC);
    okno_odrysuj_wszystkie();
}

/* ====================================================================
 * Animacja (minimalizacja i przywracanie)
 *
 * Okno jest dzielone na poziome wiersze. Kazdy wiersz dostaje wlasna
 * pozycje i szerokosc, wyliczone z krzywej S laczacej oryginalne
 * krawedzie okna z waska "szyjka" przy docku. Dolna czesc okna
 * leci pierwsza i sie zweza, gorna rusza z opoznieniem, na koncu cala
 * zaslona zapada sie do szyjki. Obliczenia sa stalopozycyjne (bez float).
 *
 * Liczba klatek jest dobierana do liczby okien lezacych pod animowanym
 * oknem: gdy nic go nie przeslania, jest pelne GENIE_KLATKI.
 * ==================================================================== */

#define GENIE_FP 1024

#ifndef GENIE_MIGAWKA
#define GENIE_MIGAWKA 0
#endif
#ifndef GENIE_MIGAWKA_PIKSELE
#define GENIE_MIGAWKA_PIKSELE (1024 * 768)
#endif

#if GENIE_MIGAWKA
#if !defined(GENIE_CZYTAJ_PIKSEL) || !defined(GENIE_PISZ_PIKSEL)
#error "uwu"
#endif
static uint32_t genie_migawka[GENIE_MIGAWKA_PIKSELE];
#endif

struct genie {
    int sx, sy, sw, sh;     /* okno zrodlowe */
    int tl, tr, ty;         /* cel: lewa i prawa krawedz szyjki, gorna krawedz docka */
    int bx, by, bw, bh;     /* obszar zajety w poprzedniej klatce */
    int migawka;            /* 1 = uzywamy zdjecia okna */
    int top, wys;           /* biezaca klatka: gorna krawedz i wysokosc zaslony */
    int sq, c;              /* biezaca klatka: zwezenie i zapadanie (0..GENIE_FP) */
    int klatki;             /* liczba klatek tej animacji */
};

static void okno_genie_czekaj(void)
{
    int i;
    for (i = 0; i < GENIE_CZEKAJ; i++) {
        __asm__ volatile("nop");
    }
}

static int genie_min(int a, int b) { return a < b ? a : b; }
static int genie_max(int a, int b) { return a > b ? a : b; }

/* smoothstep: t i wynik w skali 0..GENIE_FP */
static int genie_gladko(int t)
{
    if (t < 0) t = 0;
    if (t > GENIE_FP) t = GENIE_FP;
    return t * t / GENIE_FP * (3 * GENIE_FP - 2 * t) / GENIE_FP;
}

/* mieszanie dwoch kolorow, t = 0..GENIE_FP (0 = a, GENIE_FP = b) */
static uint32_t genie_mieszaj(uint32_t a, uint32_t b, int t)
{
    if (t < 0) t = 0;
    if (t > GENIE_FP) t = GENIE_FP;
    uint32_t r  = (((a >> 16) & 255u) * (uint32_t)(GENIE_FP - t) + ((b >> 16) & 255u) * (uint32_t)t) / GENIE_FP;
    uint32_t g  = (((a >> 8) & 255u) * (uint32_t)(GENIE_FP - t) + ((b >> 8) & 255u) * (uint32_t)t) / GENIE_FP;
    uint32_t bl = ((a & 255u) * (uint32_t)(GENIE_FP - t) + (b & 255u) * (uint32_t)t) / GENIE_FP;
    return (r << 16) | (g << 8) | bl;
}

/*
 * Krawedzie zaslony dla wiersza o pozycji u (0 = gora okna, GENIE_FP = dol).
 * Wsp. 0 = oryginalne krawedzie, GENIE_FP = krawedzie szyjki.
 */
static void genie_krawedzie(const struct genie *g, int u, int *lx, int *rw)
{
    int gu = genie_gladko(u);
    int ksztalt = (gu * gu / GENIE_FP + gu) / 2;      /* im nizej, tym mocniej do szyjki */
    int wsp = ksztalt * g->sq / GENIE_FP;             /* rozwijanie sie lejka */
    wsp = wsp + (GENIE_FP - wsp) * g->c / GENIE_FP;   /* na koncu zapada sie cala zaslona */
    int l = g->sx + (g->tl - g->sx) * wsp / GENIE_FP;
    int r = (g->sx + g->sw) + (g->tr - (g->sx + g->sw)) * wsp / GENIE_FP;
    *lx = l;
    *rw = r - l;
    if (*rw < 1) *rw = 1;
}

/* odrysowuje tlo i inne okna w obszarze (z pominieciem animowanego okna) */
static void genie_odswiez_obszar(struct okno *okno, int x, int y, int w, int h, int dol,
                                 int pokaz)
{
    int x2 = x + w;
    int y2 = y + h;
    if (x < 0) x = 0;
    if (y < PASEK_GORNY_WYSOKOSC) y = PASEK_GORNY_WYSOKOSC;
    if (x2 > EKRAN_SZEROKOSC) x2 = EKRAN_SZEROKOSC;
    if (y2 > dol) y2 = dol;
    if (x2 <= x || y2 <= y) return;
    odrysuj_okna_w_obszarze(x, y, x2 - x, y2 - y, okno);
    if (pokaz) odswiez_fragment_widoku(x, y, x2 - x, y2 - y);
}

#if GENIE_MIGAWKA
/* robi zdjecie okna (okno + tlo pulpitu) do bufora; zwraca 1 gdy sie udalo */
static int genie_zrob_migawke(struct okno *okno, const struct genie *g)
{
    if (g->sw * g->sh > GENIE_MIGAWKA_PIKSELE) return 0;
    uint8_t zmin = okno->zminimalizowane;
    okno->zminimalizowane = 0;
    odtworz_fragment_tla_bufor(g->sx, g->sy, g->sw, g->sh);
    narysuj_okno_bez_odswiezania(okno);
    okno->zminimalizowane = zmin;
    for (int y = 0; y < g->sh; y++)
        for (int x = 0; x < g->sw; x++)
            genie_migawka[y * g->sw + x] = (uint32_t)GENIE_CZYTAJ_PIKSEL(g->sx + x, g->sy + y);
    return 1;
}

static void genie_wiersz_migawka(const struct genie *g, int v, int y, int lx, int rw)
{
    const uint32_t *src = &genie_migawka[v * g->sw];
    for (int x = 0; x < rw; x++)
        GENIE_PISZ_PIKSEL(lx + x, y, src[x * g->sw / rw]);
}
#else
static int genie_zrob_migawke(struct okno *okno, const struct genie *g)
{
    (void)okno;
    (void)g;
    return 0;
}
#endif

/*
 * Tryb strukturalny: jeden wiersz okna (v = wiersz zrodlowy) narysowany
 * na wysokosci y, w przedziale [lx, lx + rw). Rysuje to samo, co
 * narysuj_okno_bez_odswiezania, ale przeskalowane poziomo.
 */
static void genie_wiersz(const struct okno *okno, int v, int sw, int sh, int y, int lx, int rw)
{
    int sc = rw * GENIE_FP / sw;
    int r  = okno->zmaksymalizowane ? 0 : OKNO_PROMIEN;
    int rp = okno->zmaksymalizowane ? 0 : OKNO_PROMIEN - 2;
    int akt = (okno == aktywne_okno);
    int cin = 0;

    /* ramka z zaokraglonymi rogami */
    if (v < r) cin = inset_rogu(r, v);
    else if (v >= sh - r) cin = inset_rogu(r, sh - 1 - v);
    cin = cin * sc / GENIE_FP;
    wypelnij_pas(lx + cin, y, rw - 2 * cin, 1, KOLOR_RAMKI, 60);

    if (v >= 2 && v < sh - 2) {
        int bor = 2 * sc / GENIE_FP;
        int ci = 0;
        int alfa;
        uint32_t kolor;
        if (bor < 1) bor = 1;

        if (v < 22) {
            int i = v - 2;
            if (i < rp) ci = inset_rogu(rp, i);
            kolor = akt ? KOLOR_PASKA_AKTYWNE : KOLOR_PASKA;
            alfa = 85;
        } else {
            int j = (sh - 3) - v;
            if (j < rp) ci = inset_rogu(rp, j);
            kolor = KOLOR_OKNA;
            alfa = 70;
        }
        ci = ci * sc / GENIE_FP;
        /* w szyjce zaslona robi sie ciemniejsza, co daje wrazenie glebi */
        kolor = genie_mieszaj(kolor, 0x00000000, (GENIE_FP - sc) / 6);
        wypelnij_pas(lx + bor + ci, y, rw - 2 * (bor + ci), 1, kolor, alfa);
    }

    /* przyciski czerwony / zolty / zielony */
    if (v >= BTN_Y && v < BTN_Y + BTN_ROZMIAR) {
        int i = v - BTN_Y;
        int ri = i < BTN_ROZMIAR / 2 ? i : BTN_ROZMIAR - 1 - i;
        int in = inset_rogu(BTN_ROZMIAR / 2, ri);
        static const uint32_t kol[3] = { KOLOR_BTN_ZAMKNIJ, KOLOR_BTN_MINIM, KOLOR_BTN_MAKS };
        for (int b = 0; b < 3; b++) {
            int bx = lx + (BTN_X(b) + in) * sc / GENIE_FP;
            int bw = (BTN_ROZMIAR - 2 * in) * sc / GENIE_FP;
            if (bw >= 1)
                wypelnij_pas(bx, y, bw, 1, akt ? kol[b] : KOLOR_BTN_NIEAKTYW, 100);
        }
    }
}

static void genie_klatka(struct okno *okno, struct genie *g, int k)
{
    int p = k * GENIE_FP / g->klatki;
    int a = genie_gladko(p * 10 / 6);                       /* dol leci pierwszy (0..60% czasu) */
    int pt = genie_gladko((p - 358) * GENIE_FP / 666);      /* gora startuje z opoznieniem */
    int dolne = g->sy + g->sh;
    int top = g->sy + (g->ty - g->sy) * pt / GENIE_FP;
    int bot = dolne + (g->ty - dolne) * a / GENIE_FP;
    if (bot < top) bot = top;

    g->sq = a;
    g->c = genie_gladko((p - 512) * 2);                     /* zapadanie w drugiej polowie */
    g->top = top;
    g->wys = bot - top;

    /* nowy obszar zajmowany przez zaslone */
    int nx = genie_min(g->sx, g->tl);
    int nr = genie_max(g->sx + g->sw, g->tr);
    int nw = nr - nx;
    int nh = g->wys;

    /* suma poprzedniego i nowego obszaru: tyle trzeba odswiezyc */
    int ux = genie_min(g->bx, nx);
    int uy = genie_min(g->by, top);
    int ux2 = genie_max(g->bx + g->bw, nr);
    int uy2 = genie_max(g->by + g->bh, top + nh);
    genie_odswiez_obszar(okno, ux, uy, ux2 - ux, uy2 - uy, EKRAN_WYSOKOSC, 0);

    if (nh > 0) {
        for (int i = 0; i < nh; i++) {
            int den = nh > 1 ? nh - 1 : 1;
            int v = i * (g->sh - 1) / den;
            int u = i * GENIE_FP / den;
            int lx, rw;
            genie_krawedzie(g, u, &lx, &rw);
            if (g->migawka) {
#if GENIE_MIGAWKA
                genie_wiersz_migawka(g, v, top + i, lx, rw);
#endif
            } else {
                genie_wiersz(okno, v, g->sw, g->sh, top + i, lx, rw);
            }
        }
        /* tytul tylko gdy zaslona jest jeszcze prawie nieodksztalcona */
        if (!g->migawka && nh * 10 >= g->sh * 9) {
            int lx, rw;
            int yt = top + 8 * (nh - 1) / (g->sh - 1);
            genie_krawedzie(g, 8 * GENIE_FP / (g->sh - 1), &lx, &rw);
            if (rw * GENIE_FP / g->sw >= 940)
                rysuj_tekst_chicago(lx + TYTUL_X, yt, okno->tytul, 0x00FFFFFF);
        }
    }

    odswiez_fragment_widoku(genie_max(ux, 0), genie_max(uy, 0),
                            genie_min(ux2, EKRAN_SZEROKOSC) - genie_max(ux, 0),
                            genie_min(uy2, EKRAN_WYSOKOSC) - genie_max(uy, 0));

    g->bx = nx;
    g->by = top;
    g->bw = nw;
    g->bh = nh;
}

/*
 * odwrotnie = 0: minimalizacja (okno zwija sie do docka)
 * odwrotnie = 1: przywracanie  (okno wylania sie z docka)
 */
static void okno_genie_animuj(struct okno *okno, int odwrotnie)
{
    struct genie g;
    g.sx = okno->x;
    g.sy = okno->y;
    g.sw = okno->szerokosc;
    g.sh = okno->wysokosc;
    g.tl = g.sx + g.sw / 2 - GENIE_ICON_W / 2;
    g.tr = g.tl + GENIE_ICON_W;
    g.ty = DOCK_PAS_Y;
    g.bx = g.sx;
    g.by = g.sy;
    g.bw = g.sw;
    g.bh = g.sh;
    g.top = g.sy;
    g.wys = g.sh;
    g.sq = 0;
    g.c = 0;
    if (g.sw < 2 || g.sh < 4) return;
    g.migawka = genie_zrob_migawke(okno, &g);

    /* ile okien lezy pod animowanym oknem (kazde trzeba przerysowac w kazdej klatce) */
    int warstwy = 0;
    for (int i = 0; i < liczba_okien; i++) {
        struct okno *inne = okna[i];
        if (!inne || inne == okno || !inne->otwarte || inne->zminimalizowane) continue;
        if (inne->x >= g.sx + g.sw || inne->x + inne->szerokosc <= g.sx ||
            inne->y >= g.sy + g.sh || inne->y + inne->wysokosc <= g.sy) continue;
        warstwy++;
    }
    g.klatki = GENIE_KLATKI;
    if (warstwy > 5) g.klatki = GENIE_KLATKI / 2;
    else if (warstwy > 2) g.klatki = GENIE_KLATKI * 2 / 3;
    if (g.klatki < 8) g.klatki = 8;

    if (!odwrotnie) {
        for (int k = 1; k <= g.klatki; k++) {
            genie_klatka(okno, &g, k);
            okno_genie_czekaj();
        }
    } else {
        for (int k = g.klatki - 1; k >= 1; k--) {
            genie_klatka(okno, &g, k);
            okno_genie_czekaj();
        }
    }

    /* sprzatanie po ostatniej klatce */
    genie_odswiez_obszar(okno, g.bx, g.by, g.bw, g.bh, EKRAN_WYSOKOSC, 1);
}

void okno_zminimalizuj(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane) return;
    if (okno->pelnoekranowy) return;

    okno_genie_animuj(okno, 0);

    int zamkniety_x = okno->x;
    int zamkniety_y = okno->y;
    int zamkniety_szerokosc = okno->szerokosc;
    int zamkniety_wysokosc = okno->wysokosc;
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
    odtworz_fragment_tla_bufor(zamkniety_x, zamkniety_y, zamkniety_szerokosc, zamkniety_wysokosc);
    pulp_zglosz_zmiane();
    okno_odrysuj_wszystkie();
}

void okno_przywroc(struct okno *okno)
{
    if (!okno || !okno->otwarte || !okno->zminimalizowane) return;

    okno_genie_animuj(okno, 1);

    okno->zminimalizowane = 0;
    okno_przenies_na_wierzch(okno);
    pulp_zglosz_zmiane();
    okno_odrysuj_wszystkie();
}

void okno_maksymalizuj(struct okno *okno)
{
    if (!okno || !okno->otwarte || okno->zminimalizowane || okno->zmaksymalizowane) return;
    if (okno->pelnoekranowy) return;
    okno->x_poprzednie = okno->x;
    okno->y_poprzednie = okno->y;
    okno->szerokosc_poprzednia = okno->szerokosc;
    okno->wysokosc_poprzednia = okno->wysokosc;
    okno->x = 0;
    okno->y = PASEK_GORNY_WYSOKOSC;
    okno->szerokosc = EKRAN_SZEROKOSC;
    okno->wysokosc = OKNA_DOL_MAKS - PASEK_GORNY_WYSOKOSC;
    okno->zmaksymalizowane = 1;
    okno_przenies_na_wierzch(okno);
    pulp_zglosz_zmiane();
    okno_odrysuj_wszystkie();
}

void okno_przywroc_rozmiar(struct okno *okno)
{
    if (!okno || !okno->otwarte || !okno->zmaksymalizowane) return;
    int stare_x = okno->x;
    int stare_y = okno->y;
    int stare_w = okno->szerokosc;
    int stare_h = okno->wysokosc;
    okno->x = okno->x_poprzednie;
    okno->y = okno->y_poprzednie;
    okno->szerokosc = okno->szerokosc_poprzednia;
    okno->wysokosc = okno->wysokosc_poprzednia;
    okno->zmaksymalizowane = 0;
    odtworz_fragment_tla_bufor(stare_x, stare_y, stare_w, stare_h);
    okno_przenies_na_wierzch(okno);
    pulp_zglosz_zmiane();
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