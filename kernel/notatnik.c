#include "notatnik.h"
#include "../drivers/keyboard.h"
#include "../graficzny/czcionka.h"
#include "../graficzny/okno.h"
#include "../graficzny/rysowanie.h"
#include "../system_plikow/system_plikow.h"

#define NOTATNIK_PLIK "NOTATKA.NTF"
#define NOTATNIK_MAKS 512
#define NOTATNIK_SZEROKOSC 640
#define NOTATNIK_WYSOKOSC 440
#define NOTATNIK_PRZYCISK_Y 400
#define NOTATNIK_PRZYCISK_OKNA_Y 430
#define CONTENT_X 8
#define CONTENT_Y 30

#define KOLOR_TEKST_NOTATKI   0x00334155
#define KOLOR_PRZYCISKU       0x004A5D6E
#define KOLOR_TEKST_PRZYCISKU 0x00FFFFFF

static struct okno okno_notatnik;
static uint8_t notatnik_otwarty;
static char notatka[NOTATNIK_MAKS + 1];
static uint8_t notatka_rozmiar[NOTATNIK_MAKS + 1];
static uint8_t notatka_styl[NOTATNIK_MAKS + 1];
static uint8_t notatnik_biezacy_rozmiar = 1;
static uint8_t notatnik_biezacy_styl = 0;
static char notatnik_plik[16] = NOTATNIK_PLIK;
static int notatka_dlugosc;
static char notatnik_status[32];

static void notatnik_ustaw_plik(const char *nazwa_pliku)
{
    int indeks = 0;
    while (nazwa_pliku[indeks] && indeks < (int)sizeof(notatnik_plik) - 1) {
        notatnik_plik[indeks] = nazwa_pliku[indeks];
        indeks++;
    }
    notatnik_plik[indeks] = 0;
}

static void notatnik_ustaw_status(const char *tekst)
{
    int indeks = 0;
    while (tekst[indeks] && indeks < 31) {
        notatnik_status[indeks] = tekst[indeks];
        indeks++;
    }
    notatnik_status[indeks] = 0;
}

static void notatnik_narysuj(int x, int y, int szerokosc, int wysokosc)
{
    int indeks;
    int linia = 0;
    int line_x = x;
    int cena;
    (void)wysokosc;
    for (indeks = 0; indeks < notatka_dlugosc; indeks++) {
        if (notatka[indeks] == '\n') {
            linia++;
            line_x = x;
            continue;
        }
        cena = znak_szerokosc(notatka_rozmiar[indeks]);
        if (line_x + cena > x + szerokosc - 16) {
            linia++;
            line_x = x;
        }
        if (y + linia * 16 + 16 > y + NOTATNIK_PRZYCISK_Y) continue;
        rysuj_znak_styl(line_x, y + linia * 16, notatka[indeks], KOLOR_TEKST_NOTATKI,
                        notatka_rozmiar[indeks], notatka_styl[indeks]);
        line_x += cena;
    }
    rysuj_prostokat(x + 8, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 12, y + NOTATNIK_PRZYCISK_Y + 4, "KURSA", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 60, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 64, y + NOTATNIK_PRZYCISK_Y + 4, "POGR", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 112, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 116, y + NOTATNIK_PRZYCISK_Y + 4, "CIEN", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 164, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 168, y + NOTATNIK_PRZYCISK_Y + 4, "MALY", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 216, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 220, y + NOTATNIK_PRZYCISK_Y + 4, "NOR", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 268, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 272, y + NOTATNIK_PRZYCISK_Y + 4, "DUZY", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 320, y + NOTATNIK_PRZYCISK_Y, 80, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 324, y + NOTATNIK_PRZYCISK_Y + 4, "ODCZYTAJ", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 410, y + NOTATNIK_PRZYCISK_Y, 70, 20, KOLOR_PRZYCISKU);
    rysuj_tekst_chicago(x + 414, y + NOTATNIK_PRZYCISK_Y + 4, "ZAPISZ", KOLOR_TEKST_PRZYCISKU);
    rysuj_tekst_chicago(x + 490, y + NOTATNIK_PRZYCISK_Y + 4, notatnik_status, KOLOR_TEKST_PRZYCISKU);
    if (okno_notatnik.w_polu_tekstowym && kursor_tekstowy_widoczny()) {
        int cx = x;
        int cy = y;
        for (indeks = 0; indeks < okno_notatnik.kursor_pozycja && indeks < notatka_dlugosc; indeks++) {
            if (notatka[indeks] == '\n') { cx = x; cy += 16; continue; }
            cena = znak_szerokosc(notatka_rozmiar[indeks]);
            if (cx + cena > x + szerokosc - 16) { cx = x; cy += 16; }
            cx += cena;
        }
        kursor_tekstowy_rysuj(cx, cy);
    }
}

static void notatnik_odczytaj(void)
{
    int i;
    int odczyt;
    int count;
    char buf[NOTATNIK_MAKS * 3 + 8];
    for (i = 0; i <= NOTATNIK_MAKS; i++) {
        notatka[i] = 0;
        notatka_rozmiar[i] = 1;
        notatka_styl[i] = 0;
    }
    notatka_dlugosc = 0;
    odczyt = read_file(notatnik_plik, buf, sizeof(buf));
    if (odczyt < 0) {
        notatka_dlugosc = 0;
        notatka[0] = 0;
        notatnik_ustaw_status("brak pliku");
        return;
    }
    if (odczyt < 6 || buf[0] != 'N' || buf[1] != 'T' || buf[2] != 'F') {
        notatnik_ustaw_status("zly format");
        return;
    }
    count = (unsigned char)buf[4] | ((unsigned char)buf[5] << 8);
    if (count > NOTATNIK_MAKS) count = NOTATNIK_MAKS;
    if (6 + count * 3 > odczyt) count = (odczyt - 6) / 3;
    for (i = 0; i < count; i++) {
        notatka[i] = buf[6 + i * 3];
        notatka_rozmiar[i] = buf[6 + i * 3 + 1];
        notatka_styl[i] = buf[6 + i * 3 + 2];
    }
    notatka_dlugosc = count;
    notatka[notatka_dlugosc] = 0;
    notatnik_ustaw_status("odczytano");
}

static void notatnik_zapisz(void)
{
    char buf[NOTATNIK_MAKS * 3 + 8];
    int i;
    int pos = 0;
    buf[pos++] = 'N';
    buf[pos++] = 'T';
    buf[pos++] = 'F';
    buf[pos++] = 0;
    buf[pos++] = (unsigned char)(notatka_dlugosc & 0xFF);
    buf[pos++] = (unsigned char)(notatka_dlugosc >> 8);
    for (i = 0; i < notatka_dlugosc; i++) {
        buf[pos++] = notatka[i];
        buf[pos++] = notatka_rozmiar[i];
        buf[pos++] = notatka_styl[i];
    }
    create_file(notatnik_plik, (uint32_t)pos);
    if (write_file(notatnik_plik, buf, (uint32_t)pos) != 0) {
        notatnik_ustaw_status("blad zapisu");
        return;
    }
    notatnik_ustaw_status("zapisano");
}

static void notatnik_klik(int x, int y)
{
    int cx = x - CONTENT_X;
    int cy = y - CONTENT_Y;
    int line_x;
    int linia;
    int indeks;
    int cena;
    if (cy >= NOTATNIK_PRZYCISK_Y && cy < NOTATNIK_PRZYCISK_Y + 20) {
        if (cx >= 8 && cx < 52) notatnik_biezacy_styl ^= 2;
        if (cx >= 60 && cx < 104) notatnik_biezacy_styl ^= 1;
        if (cx >= 112 && cx < 156) notatnik_biezacy_styl ^= 4;
        if (cx >= 164 && cx < 208) notatnik_biezacy_rozmiar = 0;
        if (cx >= 216 && cx < 260) notatnik_biezacy_rozmiar = 1;
        if (cx >= 268 && cx < 312) notatnik_biezacy_rozmiar = 2;
        if (cx >= 320 && cx < 400) notatnik_odczytaj();
        if (cx >= 410 && cx < 480) notatnik_zapisz();
        okno_narysuj(&okno_notatnik);
        return;
    }
    line_x = 0;
    linia = 0;
    indeks = 0;
    while (indeks < notatka_dlugosc) {
        if (notatka[indeks] == '\n') { line_x = 0; linia++; indeks++; continue; }
        cena = znak_szerokosc(notatka_rozmiar[indeks]);
        if (cx >= line_x && cx < line_x + cena && cy >= linia * 16 && cy < (linia + 1) * 16) {
            okno_wejdz_do_pola_tekstowego(&okno_notatnik);
            okno_notatnik.kursor_pozycja = indeks;
            return;
        }
        line_x += cena;
        indeks++;
    }
    okno_wejdz_do_pola_tekstowego(&okno_notatnik);
    okno_notatnik.kursor_pozycja = notatka_dlugosc;
}

static void notatnik_pole_tekstowe_wyjdz(struct okno *okno);
static int notatnik_pole_tekstowe_znak(struct okno *okno, char znak, uint8_t ctrl);

static void notatnik_otworz_biezacy_plik(void)
{
    if (notatnik_otwarty && okno_notatnik.otwarte) {
        notatka_dlugosc = 0;
        notatnik_odczytaj();
        okno_narysuj(&okno_notatnik);
        return;
    }
    notatnik_otwarty = 1;
    notatka_dlugosc = 0;
    notatka[0] = 0;
    notatka_rozmiar[0] = 1;
    notatka_styl[0] = 0;
    notatnik_ustaw_status("gotowy");
    notatnik_odczytaj();
    okno_stworz(&okno_notatnik, 56, 50, NOTATNIK_SZEROKOSC,
                NOTATNIK_WYSOKOSC, "NOTATNIK", notatnik_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_notatnik, notatnik_klik);
    okno_ustaw_pole_tekstowe_wyjdz(&okno_notatnik, (okno_pole_tekstowe)notatnik_pole_tekstowe_wyjdz);
    okno_ustaw_pole_tekstowe_znak(&okno_notatnik, (okno_pole_tekstowe_znak)notatnik_pole_tekstowe_znak);
    okno_ustaw_bufor_tekstu(&okno_notatnik, notatka, notatka_dlugosc, NOTATNIK_MAKS);
}

void notatnik_otworz(void)
{
    notatnik_ustaw_plik(NOTATNIK_PLIK);
    notatnik_otworz_biezacy_plik();
}

void notatnik_otworz_plik(const char *nazwa_pliku)
{
    notatnik_ustaw_plik(nazwa_pliku);
    notatnik_otworz_biezacy_plik();
}

void notatnik_obsluz_klawiature(void)
{
    char znak;
    if (!notatnik_otwarty) return;
    if (!okno_notatnik.otwarte) {
        notatnik_otwarty = 0;
        return;
    }
    if (okno_notatnik.w_polu_tekstowym) return;
    if (!okno_czy_aktywne(&okno_notatnik)) return;
    if (!keyboard_try_getchar(&znak)) return;
    if (znak == '\b') {
        if (notatka_dlugosc > 0) notatka_dlugosc--;
    } else if ((znak == '\n' || znak == '\r') && notatka_dlugosc < NOTATNIK_MAKS) {
        notatka[notatka_dlugosc] = '\n';
        notatka_rozmiar[notatka_dlugosc] = notatnik_biezacy_rozmiar;
        notatka_styl[notatka_dlugosc] = notatnik_biezacy_styl;
        notatka_dlugosc++;
    } else if (znak >= 32 && znak <= 126 && notatka_dlugosc < NOTATNIK_MAKS) {
        notatka[notatka_dlugosc] = znak;
        notatka_rozmiar[notatka_dlugosc] = notatnik_biezacy_rozmiar;
        notatka_styl[notatka_dlugosc] = notatnik_biezacy_styl;
        notatka_dlugosc++;
    } else {
        return;
    }
    notatka[notatka_dlugosc] = 0;
    okno_narysuj(&okno_notatnik);
}

static void notatnik_pole_tekstowe_wyjdz(struct okno *okno)
{
    (void)okno;
    notatnik_ustaw_status("edytor zakonczony");
}

static int notatnik_pole_tekstowe_znak(struct okno *okno, char znak, uint8_t ctrl)
{
    (void)okno;
    if (ctrl) {
        if (znak == 'c') {
            okno_skopiuj_zaznaczenie(&okno_notatnik);
            return 1;
        }
        if (znak == 'v') {
            okno_wklej_schowek(&okno_notatnik);
            notatka[notatka_dlugosc] = 0;
            okno_narysuj(&okno_notatnik);
            return 1;
        }
        return 1;
    }
    if (znak == '\b') {
        if (notatka_dlugosc > 0 && okno_notatnik.kursor_pozycja > 0) {
            for (int i = okno_notatnik.kursor_pozycja - 1; i < notatka_dlugosc - 1; i++) {
                notatka[i] = notatka[i + 1];
                notatka_rozmiar[i] = notatka_rozmiar[i + 1];
                notatka_styl[i] = notatka_styl[i + 1];
            }
            notatka_dlugosc--;
            okno_notatnik.kursor_pozycja--;
        }
    } else if ((znak == '\n' || znak == '\r') && notatka_dlugosc < NOTATNIK_MAKS) {
        for (int i = notatka_dlugosc; i >= okno_notatnik.kursor_pozycja; i--) {
            notatka[i + 1] = notatka[i];
            notatka_rozmiar[i + 1] = notatka_rozmiar[i];
            notatka_styl[i + 1] = notatka_styl[i];
        }
        notatka[okno_notatnik.kursor_pozycja] = '\n';
        notatka_rozmiar[okno_notatnik.kursor_pozycja] = notatnik_biezacy_rozmiar;
        notatka_styl[okno_notatnik.kursor_pozycja] = notatnik_biezacy_styl;
        notatka_dlugosc++;
        okno_notatnik.kursor_pozycja++;
    } else if (znak >= 32 && znak <= 126 && notatka_dlugosc < NOTATNIK_MAKS) {
        for (int i = notatka_dlugosc; i >= okno_notatnik.kursor_pozycja; i--) {
            notatka[i + 1] = notatka[i];
            notatka_rozmiar[i + 1] = notatka_rozmiar[i];
            notatka_styl[i + 1] = notatka_styl[i];
        }
        notatka[okno_notatnik.kursor_pozycja] = znak;
        notatka_rozmiar[okno_notatnik.kursor_pozycja] = notatnik_biezacy_rozmiar;
        notatka_styl[okno_notatnik.kursor_pozycja] = notatnik_biezacy_styl;
        notatka_dlugosc++;
        okno_notatnik.kursor_pozycja++;
    } else {
        return 1;
    }
    notatka[notatka_dlugosc] = 0;
    okno_narysuj(&okno_notatnik);
    return 1;
}
