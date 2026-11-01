#include "../okno_user.h"
#include "syscall.h"
#include "../libc/string.h"

#define EKSPLORATOR_SZEROKOSC 500
#define EKSPLORATOR_WYSOKOSC 400
#define MAX_PLIKOW 64
#define NAZWA_MAKS 12

struct plik_info {
    char nazwa[NAZWA_MAKS + 1];
    uint8_t katalog;
};

static struct okno_user okno_eksplorator;
static struct plik_info pliki[MAX_PLIKOW];
static int liczba_plikow = 0;
static int wybrany_plik = -1;
static char sciezka_biezaca[128] = "";
static uint8_t odswiez_potrzebne = 1;

static void eksplorator_odczytaj_katalog(void) {
    struct fs_entry_info entries[MAX_PLIKOW];
    fs_reset_dir_user();
    liczba_plikow = fs_list_user((char *)entries, MAX_PLIKOW * sizeof(struct fs_entry_info));
    if (liczba_plikow > MAX_PLIKOW) liczba_plikow = MAX_PLIKOW;
    for (int i = 0; i < liczba_plikow; i++) {
        int len = 0;
        while (entries[i].name[len] && len < NAZWA_MAKS) {
            pliki[i].nazwa[len] = entries[i].name[len];
            len++;
        }
        pliki[i].nazwa[len] = 0;
        pliki[i].katalog = entries[i].is_directory;
    }
    wybrany_plik = -1;
    odswiez_potrzebne = 0;
}

static void eksplorator_narysuj(int x, int y, int szerokosc, int wysokosc) {
    if (odswiez_potrzebne) eksplorator_odczytaj_katalog();

    rysuj_prostokat(x, y + 22, szerokosc, 20, 0x00334155);
    fs_pwd_user(sciezka_biezaca);
    rysuj_tekst(x + 4, y + 26, sciezka_biezaca[0] ? sciezka_biezaca : "/", KOLOR_BIALY);

    for (int i = 0; i < liczba_plikow; i++) {
        int item_y = y + 50 + i * 20;
        if (item_y + 20 > y + wysokosc) break;

        if (i == wybrany_plik) {
            rysuj_prostokat(x + 4, item_y, szerokosc - 8, 18, 0x003B82F6);
        }

        if (pliki[i].katalog) {
            rysuj_tekst(x + 8, item_y + 2, "[DIR] ", KOLOR_ZOLTY);
            rysuj_tekst(x + 48, item_y + 2, pliki[i].nazwa, KOLOR_BIALY);
        } else {
            rysuj_tekst(x + 8, item_y + 2, "      ", KOLOR_SZARY);
            rysuj_tekst(x + 48, item_y + 2, pliki[i].nazwa, KOLOR_BIALY);
        }
    }

    rysuj_tekst(x + 4, y + 4, "Eksplorator plikow - ENTER: otworz, BACKSPACE: gora", KOLOR_SZARY);
}

static void eksplorator_klik(int mx, int my) {
    if (my < 50) return;
    int indeks = (my - 50) / 20;
    if (indeks >= 0 && indeks < liczba_plikow) {
        wybrany_plik = indeks;
        okno_rysuj();
    }
}

static void eksplorator_pole_tekstowe_enter(void) {
}

static void eksplorator_pole_tekstowe_wyjdz(void) {
}

static int eksplorator_pole_tekstowe_znak(char znak, uint8_t ctrl) {
    (void)ctrl;
    if (znak == '\n' || znak == '\r') {
        if (wybrany_plik >= 0 && wybrany_plik < liczba_plikow) {
            if (pliki[wybrany_plik].katalog) {
                fs_cd_user(pliki[wybrany_plik].nazwa);
                odswiez_potrzebne = 1;
                wybrany_plik = -1;
                okno_rysuj();
            }
        }
    } else if (znak == '\b') {
        fs_cd_user("..");
        odswiez_potrzebne = 1;
        wybrany_plik = -1;
        okno_rysuj();
    }
    return 1;
}

int main(void) {
    eksplorator_odczytaj_katalog();

    okno_stworz_user(&okno_eksplorator, 100, 100, EKSPLORATOR_SZEROKOSC,
                     EKSPLORATOR_WYSOKOSC, "EKSPLORATOR PLIKOW", eksplorator_narysuj);
    okno_ustaw_obsluge_klikniecia_user(&okno_eksplorator, eksplorator_klik);
    okno_ustaw_pole_tekstowe_enter_user(&okno_eksplorator, eksplorator_pole_tekstowe_enter);
    okno_ustaw_pole_tekstowe_wyjdz_user(&okno_eksplorator, eksplorator_pole_tekstowe_wyjdz);
    okno_ustaw_pole_tekstowe_znak_user(&okno_eksplorator, eksplorator_pole_tekstowe_znak);

    while (okno_czy_otwarte_user(&okno_eksplorator)) {
        okno_rysuj();
        proces_yield_user();
    }

    return 0;
}
