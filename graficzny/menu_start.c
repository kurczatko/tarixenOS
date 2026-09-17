#include "menu_start.h"
#include "czcionka.h"
#include "rysowanie.h"
#include "okno.h"
#include "../drivers/mouse.h"
#include "../kernel/shutdown.h"
#include "../kernel/notatnik.h"
#include "eksplorator_plikow.h"

#define START_X 0
#define START_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC)
#define START_SZEROKOSC 72
#define START_WYSOKOSC PASEK_ZADAN_WYSOKOSC
#define MENU_X 0
#define MENU_SZEROKOSC 104
#define MENU_WYSOKOSC 188
#define MENU_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - MENU_WYSOKOSC)

#define KOLOR_PRZYCISKU_START 0x004A5D6E
#define KOLOR_RAMKA_MENU      0x003A4B5C
#define KOLOR_TLO_MENU        0x001E293B
#define KOLOR_ELEMENT_MENU    0x00334155
#define KOLOR_TEKST_BIALY     0x00FFFFFF

static uint8_t menu_otwarte;

extern struct okno *aktywne_okno;

void menu_start(void)
{
    menu_otwarte = 0;
    rysuj_prostokat(START_X, START_Y, START_SZEROKOSC, START_WYSOKOSC, KOLOR_PRZYCISKU_START);
    rysuj_tekst_8x8(START_X + 16, START_Y + (START_WYSOKOSC / 2) - 4, "start", KOLOR_TEKST_BIALY);
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

    rysuj_prostokat(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC, KOLOR_RAMKA_MENU);
    rysuj_prostokat(MENU_X + 2, MENU_Y + 2, MENU_SZEROKOSC - 4, MENU_WYSOKOSC - 4, KOLOR_TLO_MENU);
    
    rysuj_prostokat(MENU_X + 6, MENU_Y + 12, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU);
    rysuj_tekst_8x8(MENU_X + 26, MENU_Y + 22, "NOTATNIK", KOLOR_TEKST_BIALY);

    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_notatnika[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 20 + iy, KOLOR_ZOLTY);
            }
        }
    }
    
    rysuj_prostokat(MENU_X + 6, MENU_Y + 48, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU);
    rysuj_tekst_8x8(MENU_X + 26, MENU_Y + 58, "WYLACZ", KOLOR_TEKST_BIALY);
    
    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_wylaczania[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 56 + iy, KOLOR_CZERWONY);
            }
        }
    }
    
    rysuj_prostokat(MENU_X + 6, MENU_Y + 84, MENU_SZEROKOSC - 12, 28, KOLOR_ELEMENT_MENU);
    rysuj_tekst_8x8(MENU_X + 26, MENU_Y + 94, "PLIKI", KOLOR_TEKST_BIALY);

    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_eksploratora_plikow[iy][ix]) {
                rysuj_piksel(MENU_X + 12 + ix, MENU_Y + 92 + iy, KOLOR_ZOLTY);
            }
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
        shutdown();
        return;
    }
    if (menu_otwarte && x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
        y >= MENU_Y + 84 && y < MENU_Y + 112) {
            eksplorator_plikow();
    }
    if (menu_otwarte && (x < MENU_X || x >= MENU_X + MENU_SZEROKOSC ||
                         y < MENU_Y || y >= MENU_Y + MENU_WYSOKOSC)) {
        menu_otwarte = 0;
        menu_start_wyczysc();
    }
}