#include <graficzny/menu_start.h>
#include <graficzny/czcionka.h>
#include <graficzny/rysowanie.h>
#include <graficzny/okno.h>
#include <drivers/mouse.h>
#include <kernel/shutdown.h>
#include <kernel/notatnik.h>
#include <graficzny/eksplorator_plikow.h>
#include <kernel/browser.h>
#include <kernel/kalkulator.h>
#include <kernel/apps/gra.h>
#include <kernel/malowanie.h>
#include <kernel/zadania.h>

#define START_X 0
#define START_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC)
#define START_SZEROKOSC 72
#define START_WYSOKOSC PASEK_ZADAN_WYSOKOSC
#define MENU_X 0
#define MENU_SZEROKOSC 150
#define MENU_WYSOKOSC 340
#define MENU_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - MENU_WYSOKOSC)

#define KOLOR_PRZYCISKU_START 0x004A5D6E
#define KOLOR_RAMKA_MENU      0x003A4B5C
#define KOLOR_TLO_MENU        0x001E293B
#define KOLOR_ELEMENT_MENU    0x00334155
#define KOLOR_TEKST_BIALY     0x00FFFFFF

static uint8_t menu_otwarte;

extern struct okno *aktywne_okno;

void menu_start_rysuj_przycisk(void)
{
    static const uint8_t ikona_menu_start[16][16] = {
        {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0},
        {0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0}
    };
    rozmyj_prostokat(START_X, START_Y, START_SZEROKOSC, START_WYSOKOSC);
    rysuj_prostokat_polprzezroczysty(START_X, START_Y, START_SZEROKOSC, START_WYSOKOSC, KOLOR_PRZYCISKU_START, 70);
    
    for (int iy = 0; iy < 16; iy++) {
        for (int ix = 0; ix < 16; ix++) {
            if (ikona_menu_start[iy][ix]) { 
                rysuj_piksel(START_X + 6 + ix, START_Y + (START_WYSOKOSC / 2) - 8 + iy, KOLOR_ZOLTY);
            }
        }
    }
    
    rysuj_tekst_chicago(START_X + 26, START_Y + (START_WYSOKOSC / 2) - 4, "start", KOLOR_TEKST_BIALY);
}

void menu_start(void)
{
    menu_otwarte = 0;
    menu_start_rysuj_przycisk();
    odswiez_widok();
}

static void menu_start_narysuj(void)
{
    static const uint8_t ikona_wylaczania[12][12] = {
        {0,0,0,0,0,1,1,0,0,0,0,0},
        {0,0,1,1,1,1,1,1,1,1,0,0},
        {0,1,0,0,0,1,1,0,0,0,1,0},
        {1,0,0,0,0,1,1,0,0,0,0,1},
        {1,0,0,0,0,1,1,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {0,1,0,0,0,0,0,0,0,0,1,0},
        {0,0,1,0,0,0,0,0,0,1,0,0},
        {0,0,0,1,1,1,1,1,1,0,0,0}
    };

    static const uint8_t ikona_notatnika[12][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,0,0,0,0,1,1},
        {0,0,0,0,0,0,0,0,0,1,1,0},
        {0,0,0,0,0,0,0,0,1,1,0,0},
        {0,0,0,0,0,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,0,0,0,0,0},
        {0,0,0,0,1,1,0,0,0,0,0,0},
        {0,0,0,1,1,0,0,0,0,0,0,0},
        {0,0,1,1,0,0,0,0,0,0,0,0},
        {0,1,1,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,0,0,0,0,0,0}
    };

    static const uint8_t ikona_eksploratora_plikow[12][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,1,1,1,1,0,0,0,0,0,0},
        {0,1,1,1,1,1,1,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,0},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1}
    };

    rozmyj_prostokat(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC);
    rysuj_prostokat_polprzezroczysty(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC, KOLOR_RAMKA_MENU, 80);
    rysuj_prostokat_polprzezroczysty(MENU_X + 2, MENU_Y + 2, MENU_SZEROKOSC - 4, MENU_WYSOKOSC - 4, KOLOR_TLO_MENU, 75);
    
    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 12, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 22, "NOTATNIK", KOLOR_TEKST_BIALY);

    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_notatnika[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 20 + iy, KOLOR_ZOLTY);
            }
        }
    }
    
    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 48, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 58, "KONTA", KOLOR_TEKST_BIALY);

    static const uint8_t ikona_konta[12][12] = {
        {0,0,0,1,1,1,1,1,0,0,0,0},
        {0,0,1,1,1,1,1,1,1,0,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,0},
        {1,1,1,1,0,0,0,1,1,1,1,1},
        {1,1,1,0,0,0,0,0,1,1,1,1},
        {1,1,1,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,0,0,0,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,1,1,1,1,1,0,0,0,0}
    };
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_konta[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 56 + iy, KOLOR_ZOLTY);
            }
        }
    }
    
    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 84, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 94, "WYLACZ", KOLOR_TEKST_BIALY);
    
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_wylaczania[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 92 + iy, KOLOR_CZERWONY);
            }
        }
    }

    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 120, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 130, "PLIKI", KOLOR_TEKST_BIALY);

    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_eksploratora_plikow[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 128 + iy, KOLOR_ZOLTY);
            }
        }
    }

    static const uint8_t ikona_globu[12][12] = {
        {0,0,0,0,1,1,1,1,0,0,0,0},
        {0,0,1,1,1,2,2,1,1,1,0,0},
        {0,1,1,2,1,2,2,1,2,1,1,0},
        {1,1,2,1,1,2,2,1,1,2,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,2,2,2,2,2,2,2,2,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,2,1,1,2,2,1,1,2,1,1},
        {0,1,1,2,1,2,2,1,2,1,1,0},
        {0,0,1,1,1,2,2,1,1,1,0,0},
        {0,0,0,0,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };

    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 156, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 166, "WWW", KOLOR_TEKST_BIALY);
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            uint8_t piksel = ikona_globu[iy][ix];
            if (piksel)
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 164 + iy,
                             piksel == 1 ? KOLOR_NIEBIESKI : 0x0068B8FF);
        }
    }

    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 192, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 202, "KALKULATOR", KOLOR_TEKST_BIALY);

    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 228, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 238, "GRA", KOLOR_TEKST_BIALY);

    static const uint8_t ikona_pada[12][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,1,1,0,0,0,0,1,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,1,0},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,1,1,1,1,1,1,0,1,1},
        {1,0,0,0,1,0,0,1,0,1,0,1},
        {1,1,0,1,1,1,1,1,1,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,1,1,1},
        {0,1,1,0,0,0,0,0,0,1,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_pada[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 236 + iy, KOLOR_SZARY);
            }
        }
    }

    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 264, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 274, "MALOWANIE", KOLOR_TEKST_BIALY);

    rysuj_prostokat_polprzezroczysty(MENU_X + 6, MENU_Y + 300, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU, 85);
    rysuj_tekst_chicago(MENU_X + 26, MENU_Y + 310, "ZADANIA", KOLOR_TEKST_BIALY);

    static const uint8_t ikona_zadan[12][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,0,0,0,1,0,0,0,1,0,0},
        {0,1,0,0,0,1,0,0,0,1,0,0},
        {0,1,0,0,0,1,0,0,0,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,0,0,0,1,0,0,0,1,0,0},
        {0,1,0,0,0,1,0,0,0,1,0,0},
        {0,1,0,0,0,1,0,0,0,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_zadan[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 308 + iy, KOLOR_ZOLTY);
            }
        }
    }

    static const uint8_t ikona_palety[12][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,0,0,0,0,0,0,0,1,0,0},
        {0,1,0,1,1,0,1,1,0,1,0,0},
        {0,1,0,1,0,1,0,1,0,1,0,0},
        {0,1,0,1,1,0,1,1,0,1,0,0},
        {0,1,0,0,0,0,0,0,0,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_palety[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 272 + iy, KOLOR_ZOLTY);
            }
        }
    }

    static const uint8_t ikona_kalkulatora[12][12] = {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,2,2,2,1,1,2,2,1,0,0},
        {0,1,2,1,2,1,1,2,1,1,0,0},
        {0,1,2,1,2,1,1,2,1,1,0,0},
        {0,1,2,2,2,1,1,2,2,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,2,2,2,1,1,2,2,1,0,0},
        {0,1,2,1,2,1,1,2,1,1,0,0},
        {0,1,2,2,2,1,1,2,2,1,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            uint8_t piksel = ikona_kalkulatora[iy][ix];
            if (piksel == 1)
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 200 + iy, KOLOR_SZARY);
            else if (piksel == 2)
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 200 + iy, KOLOR_BIALY);
        }
    }

    odswiez_widok();
}

static void menu_start_wyczysc(void)
{
    odtworz_fragment_tla(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC);
    
    if (aktywne_okno && aktywne_okno->otwarte) {
        int ox = aktywne_okno->x;
        int oy = aktywne_okno->y;
        int ow = aktywne_okno->szerokosc;
        int oh = aktywne_okno->wysokosc;

        if (MENU_X < ox + ow && MENU_X + MENU_SZEROKOSC > ox &&
            MENU_Y < oy + oh && MENU_Y + MENU_WYSOKOSC > oy) {
            okno_narysuj(aktywne_okno);
        }
    }
    
    odswiez_widok();
}

void menu_start_przelacz(void)
{
    if (menu_otwarte) {
        menu_otwarte = 0;
        menu_start_wyczysc();
    } else {
        menu_otwarte = 1;
        menu_start_narysuj();
    }
}

void menu_start_mouse_click(void)
{
    int x;
    int y;

    mouse_pozycja(&x, &y);
    if (!menu_otwarte && x >= START_X && x < START_X + START_SZEROKOSC &&
        y >= START_Y && y < START_Y + START_WYSOKOSC) {
        menu_otwarte = 1;
        menu_start_narysuj();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 12 && y < MENU_Y + 40) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        notatnik_otworz();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 48 && y < MENU_Y + 76) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        konta_zarzadzaj();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 84 && y < MENU_Y + 112) {
        shutdown();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 120 && y < MENU_Y + 148) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        eksplorator_plikow();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 156 && y < MENU_Y + 184) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        browser_open();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 192 && y < MENU_Y + 220) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        kalkulator_otworz();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 228 && y < MENU_Y + 256) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        gra();
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 264 && y < MENU_Y + 292) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        malowanie_otworz();
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 300 && y < MENU_Y + 328) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        zadania_otworz();
    }
    if (menu_otwarte && (x < MENU_X || x >= MENU_X + MENU_SZEROKOSC ||
                         y < MENU_Y || y >= MENU_Y + MENU_WYSOKOSC)) {
        menu_otwarte = 0;
        menu_start_wyczysc();
    }
}
