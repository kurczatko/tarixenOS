#include "notatnik.h"
#include "../drivers/keyboard.h"
#include "../graficzny/czcionka.h"
#include "../graficzny/okno.h"
#include "../graficzny/rysowanie.h"
#include "../system_plikow/system_plikow.h"

#define NOTATNIK_PLIK "NOTATKA.TXT"
#define NOTATNIK_MAKS 512
#define NOTATNIK_SZEROKOSC 520
#define NOTATNIK_WYSOKOSC 360
#define NOTATNIK_PRZYCISK_Y 300
#define NOTATNIK_PRZYCISK_OKNA_Y 330

#define KOLOR_TEKST_NOTATKI   0x00334155
#define KOLOR_PRZYCISKU       0x004A5D6E
#define KOLOR_TEKST_PRZYCISKU 0x00FFFFFF

static struct okno okno_notatnik;
static uint8_t notatnik_otwarty;
static char notatka[NOTATNIK_MAKS + 1];
static int notatka_dlugosc;
static char notatnik_status[32];

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
    int kolumna = 0;
    (void)szerokosc;
    (void)wysokosc;
    for (indeks = 0; indeks < notatka_dlugosc; indeks++) {
        if (notatka[indeks] == '\n' || kolumna == 62) {
            linia++;
            kolumna = 0;
            if (notatka[indeks] == '\n') continue;
        }
        rysuj_znak_8x8(x + kolumna * 8, y + linia * 8, notatka[indeks], KOLOR_TEKST_NOTATKI);
        kolumna++;
    }
    rysuj_prostokat(x, y + NOTATNIK_PRZYCISK_Y, 88, 24, KOLOR_PRZYCISKU);
    rysuj_tekst_8x8(x + 8, y + NOTATNIK_PRZYCISK_Y + 8, "ODCZYTAJ", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + 104, y + NOTATNIK_PRZYCISK_Y, 56, 24, KOLOR_PRZYCISKU);
    rysuj_tekst_8x8(x + 112, y + NOTATNIK_PRZYCISK_Y + 8, "ZAPISZ", KOLOR_TEKST_PRZYCISKU);
    rysuj_tekst_8x8(x + 176, y + NOTATNIK_PRZYCISK_Y + 8, notatnik_status, KOLOR_TEKST_PRZYCISKU);
}

static void notatnik_odczytaj(void)
{
    int odczyt = read_file(NOTATNIK_PLIK, notatka, NOTATNIK_MAKS);
    if (odczyt < 0) {
        notatka_dlugosc = 0;
        notatka[0] = 0;
        notatnik_ustaw_status("brak pliku");
        return;
    }
    notatka_dlugosc = odczyt;
    notatka[notatka_dlugosc] = 0;
    notatnik_ustaw_status("odczytano");
}

static void notatnik_zapisz(void)
{
    if (write_file(NOTATNIK_PLIK, notatka, notatka_dlugosc) != 0) {
        if (create_file(NOTATNIK_PLIK, 0) != 0 ||
            write_file(NOTATNIK_PLIK, notatka, notatka_dlugosc) != 0) {
            notatnik_ustaw_status("blad zapisu");
            return;
        }
    }
    notatnik_ustaw_status("zapisano");
}

static void notatnik_klik(int x, int y)
{
    if (y >= NOTATNIK_PRZYCISK_OKNA_Y && y < NOTATNIK_PRZYCISK_OKNA_Y + 24) {
        if (x >= 8 && x < 96) notatnik_odczytaj();
        if (x >= 112 && x < 168) notatnik_zapisz();
        okno_narysuj(&okno_notatnik);
    }
}

void notatnik_otworz(void)
{
    if (notatnik_otwarty) return;
    notatnik_otwarty = 1;
    notatka_dlugosc = 0;
    notatka[0] = 0;
    notatnik_ustaw_status("gotowy");
    notatnik_odczytaj();
    okno_stworz(&okno_notatnik, 56, 50, NOTATNIK_SZEROKOSC,
                NOTATNIK_WYSOKOSC, "NOTATNIK", notatnik_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_notatnik, notatnik_klik);
}

void notatnik_obsluz_klawiature(void)
{
    char znak;
    if (!notatnik_otwarty) return;
    if (!okno_notatnik.otwarte) {
        notatnik_otwarty = 0;
        return;
    }
    if (!keyboard_dane_dostepne()) return;
    znak = keyboard_getchar();
    if (znak == '\b') {
        if (notatka_dlugosc > 0) notatka_dlugosc--;
    } else if ((znak == '\n' || znak == '\r') && notatka_dlugosc < NOTATNIK_MAKS) {
        notatka[notatka_dlugosc++] = '\n';
    } else if (znak >= 32 && znak <= 126 && notatka_dlugosc < NOTATNIK_MAKS) {
        notatka[notatka_dlugosc++] = znak;
    } else {
        return;
    }
    notatka[notatka_dlugosc] = 0;
    okno_narysuj(&okno_notatnik);
}