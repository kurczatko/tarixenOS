#include "../include/printf/vga.h"
#include "shell.h"
#include "../include/blue.h"
#include "konfiguracja.h"
#include "../scripts/ekran_bootowania.h"
#include "../../drivers/keyboard.h"
#include "panika.h"
#include "../drivers/mouse.h"
#include "../system_plikow/system_plikow.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/menu_start.h"
#include "../graficzny/okno.h"
#include "../graficzny/czcionka.h"
#include "../system_plikow/system_plikow.h"
#include "notatnik.h"

static struct okno okno_tapeta;

static void obsluz_wybor_tapety(int x, int y)
{
    static const uint8_t kolory[] = {1, 2, 4, 5, 9, 10, 12, 14};
    int kolumna;
    int wiersz;
    if (x < 16 || x >= 144 || y < 48 || y >= 112) return;
    kolumna = (x - 16) / 32;
    wiersz = (y - 48) / 32;
    if (kolumna < 0 || kolumna >= 4 || wiersz < 0 || wiersz >= 2) return;
    ustaw_kolor_tapety(kolory[wiersz * 4 + kolumna]);
}

static void rysuj_wybor_tapety(int x, int y, int szerokosc, int wysokosc)
{
    static const uint8_t kolory[] = {1, 2, 4, 5, 9, 10, 12, 14};
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_8x8(x, y, "WYBIERZ TAPETE", 15);
    for (int indeks = 0; indeks < 8; indeks++)
        rysuj_prostokat(x + 8 + (indeks % 4) * 32, y + 18 + (indeks / 4) * 32, 24, 24, kolory[indeks]);
}

void __attribute__((section(".text.entry"))) kernel_main(void)
{
    rysowanie_init();
    init_fs();
    okno_stworz(&okno_tapeta, 248, 80, 360, 170, "TAPETA", rysuj_wybor_tapety);
    okno_ustaw_obsluge_klikniecia(&okno_tapeta, obsluz_wybor_tapety);
    menu_start();
    mouse_init();
    keyboard_init();
    while(1) {
        mouse_poll();
        notatnik_obsluz_klawiature();
    }
}
