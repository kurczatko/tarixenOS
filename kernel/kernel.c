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
#include "../graficzny/eksplorator_plikow.h"
#include "../system_plikow/system_plikow.h"
#include "notatnik.h"
#include "../drivers/sound/intel_hd.h"
#include "../drivers/net.h"
#include "browser.h"
#include "zegar.h"

static struct okno okno_tapeta;
static char odpowiedz_czasu_http[768];

static void obsluz_wybor_tapety(int x, int y)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
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
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_8x8(x, y, "WYBIERZ TAPETE", 0x00000000); // tworzenie okna z wyborem tapety (nie powinno sie to znalesc w kernelu ale kto mi zabroni)
    for (int indeks = 0; indeks < 8; indeks++)
        rysuj_prostokat(x + 8 + (indeks % 4) * 32, y + 18 + (indeks / 4) * 32, 24, 24, kolory[indeks]);
}

void __attribute__((section(".text.entry"))) kernel_main(void)
{
    rysowanie_init();
    zegar_init();
    {
        char tekst_czasu[9];
        zegar_pobierz(tekst_czasu);
        ustaw_czas_paska(tekst_czasu);
    }
    init_fs();
    okno_stworz(&okno_tapeta, 248, 80, 360, 170, "TAPETA", rysuj_wybor_tapety);
    okno_ustaw_obsluge_klikniecia(&okno_tapeta, obsluz_wybor_tapety);
    menu_start();
    mouse_init();
    keyboard_init();
    net_init();
    ustaw_status_internetu((uint8_t)net_ready());
    if (net_ready()) {
        int dlugosc_odpowiedzi = net_http_get("info.cern.ch", "/", odpowiedz_czasu_http,
                                               sizeof(odpowiedz_czasu_http));
        if (dlugosc_odpowiedzi > 0)
            zegar_synchronizuj_http(odpowiedz_czasu_http, (uint32_t)dlugosc_odpowiedzi);
    }
    init_intel_hda();
    while(1) {
        char tekst_czasu[9];
        if (zegar_pobierz(tekst_czasu)) ustaw_czas_paska(tekst_czasu);
        mouse_poll();
        browser_handle_keyboard();
        eksplorator_plikow_obsluz_klawiature();
        notatnik_obsluz_klawiature();
        keyboard_discard_pending();
    }
}