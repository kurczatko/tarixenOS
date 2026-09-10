#include "menu_start.h"
#include "czcionka.h"
#include "rysowanie.h"
#include "../drivers/mouse.h"
#include "../kernel/shutdown.h"

#define START_X 8
#define START_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 2)
#define START_SZEROKOSC 72
#define START_WYSOKOSC 16
#define MENU_X 8
#define MENU_Y 300
#define MENU_SZEROKOSC 104
#define MENU_WYSOKOSC 144

static uint8_t menu_otwarte;

void menu_start(void)
{
    menu_otwarte = 0;
    rysuj_prostokat(START_X, START_Y, START_SZEROKOSC, START_WYSOKOSC, 7);
    rysuj_tekst_8x8(START_X + 8, START_Y + 4, "start", 0);
    odswiez_widok();
}

static void menu_start_narysuj(void)
{
    rysuj_prostokat(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC, 15);
    rysuj_prostokat(MENU_X + 2, MENU_Y + 2, MENU_SZEROKOSC - 4, MENU_WYSOKOSC - 4, 3);
    rysuj_prostokat(MENU_X + 6, MENU_Y + 12, MENU_SZEROKOSC - 12, 28, 7);
    rysuj_tekst_8x8(MENU_X + 12, MENU_Y + 22, "WYLACZ", 0);
    odswiez_widok();
}

static void menu_start_wyczysc(void)
{
    rysuj_prostokat(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC, 1);
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
        shutdown();
        return;
    }
    if (menu_otwarte && (x < MENU_X || x >= MENU_X + MENU_SZEROKOSC ||
                         y < MENU_Y || y >= MENU_Y + MENU_WYSOKOSC)) {
        menu_otwarte = 0;
        menu_start_wyczysc();
    }
}
