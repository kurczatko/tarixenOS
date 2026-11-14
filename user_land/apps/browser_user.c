/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <user_land/okno_user.h>
#include <kernel/syscall.h>
#include <libc/string.h>

#define BROWSER_SZEROKOSC 760
#define BROWSER_WYSOKOSC 570
#define ADRES_MAKS 128
#define STATUS_MAKS 64
#define STRONA_MAKS 8192

static struct okno_user okno_browser;
static char adres[ADRES_MAKS] = "http://textfiles.com/";
static char strona[STRONA_MAKS];
static char status[STATUS_MAKS] = "Wpisz adres i nacisnij ENTER";
static int adres_dlugosc = 0;
static uint8_t adres_focused = 1;
static int strona_scroll = 0;

static void browser_narysuj(int x, int y, int szerokosc, int wysokosc) {
    rysuj_prostokat(x + 8, y + 30, 610, 24, adres_focused ? 0x00FFF4CC : 0x00FFFFFF);
    rysuj_tekst(x + 12, y + 44, adres, 0x00000000);
    rysuj_prostokat(x + 620, y + 30, 72, 24, 0x00334155);
    rysuj_tekst(x + 636, y + 44, "IDZ", KOLOR_BIALY);
    rysuj_tekst(x + 8, y + 60, status, 0x00000000);
    rysuj_prostokat(x + 8, y + 80, szerokosc - 16, wysokosc - 90, 0x00FFFFFF);

    int linia = 0;
    int char_x = 0;
    for (int i = 0; strona[i] && linia * 8 < wysokosc - 100; i++) {
        char c = strona[i];
        if (c == '\n') {
            linia++;
            char_x = 0;
            continue;
        }
        if (linia < strona_scroll) {
            char_x += 8;
            continue;
        }
        int cy = y + 82 + (linia - strona_scroll) * 8;
        if (cy + 8 > y + wysokosc - 8) break;
        int cx = x + 12 + char_x;
        if (cx + 8 > x + szerokosc - 12) {
            linia++;
            char_x = 0;
            i--;
            continue;
        }
        rysuj_znak(cx, cy, c, 0x00000000);
        char_x += 8;
    }
}

static void browser_klik(int mx, int my) {
    if (my >= 30 && my < 58) {
        if (mx >= 620 && mx < 692) {
            adres_focused = 1;
            okno_rysuj();
            return;
        } else if (mx >= 8 && mx < 620) {
            adres_focused = 1;
            okno_rysuj();
            return;
        }
    }
}

static void browser_pole_tekstowe_enter(void) {
}

static void browser_pole_tekstowe_wyjdz(void) {
}

static int browser_pole_tekstowe_znak(char znak, uint8_t ctrl) {
    (void)ctrl;
    if (!adres_focused) return 1;

    if (znak == '\b') {
        if (adres_dlugosc > 0) {
            adres_dlugosc--;
            adres[adres_dlugosc] = 0;
            okno_rysuj();
        }
    } else if (znak == '\n' || znak == '\r') {
        adres_focused = 0;
        if (adres_dlugosc < ADRES_MAKS - 7 && !strona[0]) {
            // Prosty test - kopiujemy do statusu
            for (int i = 0; i < 63 && status[i]; i++) {}
            status[0] = 'L'; status[1] = 'a'; status[2] = 'd'; status[3] = 'u'; status[4] = 'j'; status[5] = 'e'; status[6] = 0;
        }
        okno_rysuj();
    } else if (znak >= 32 && znak <= 126 && adres_dlugosc < ADRES_MAKS - 1) {
        adres[adres_dlugosc++] = znak;
        adres[adres_dlugosc] = 0;
        okno_rysuj();
    }
    return 1;
}

int main(void) {
    adres_dlugosc = 0;
    while (adres[adres_dlugosc]) adres_dlugosc++;

    okno_stworz_user(&okno_browser, 120, 80, BROWSER_SZEROKOSC,
                     BROWSER_WYSOKOSC, "PRZEGLADARKA", browser_narysuj);
    okno_ustaw_obsluge_klikniecia_user(&okno_browser, browser_klik);
    okno_ustaw_pole_tekstowe_enter_user(&okno_browser, browser_pole_tekstowe_enter);
    okno_ustaw_pole_tekstowe_wyjdz_user(&okno_browser, browser_pole_tekstowe_wyjdz);
    okno_ustaw_pole_tekstowe_znak_user(&okno_browser, browser_pole_tekstowe_znak);

    while (okno_czy_otwarte_user(&okno_browser)) {
        okno_rysuj();
        proces_yield_user();
    }

    return 0;
}
