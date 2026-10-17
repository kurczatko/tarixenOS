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
#include "../graficzny/pulpit.h"
#include "../system_plikow/system_plikow.h"
#include "notatnik.h"
#include "../drivers/sound/intel_hd.h"
#include "../drivers/net.h"
#include "browser.h"
#include "zegar.h"
#include <../user_land/konta.h>

static struct okno okno_tapeta;
static char odpowiedz_czasu_http[768];
extern struct okno *aktywne_okno;

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
    tapeta_zapisz();
}

static void rysuj_wybor_tapety(int x, int y, int szerokosc, int wysokosc)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_chicago(x, y, "WYBIERZ TAPETE", 0x00000000);
    for (int indeks = 0; indeks < 8; indeks++)
        rysuj_prostokat(x + 8 + (indeks % 4) * 32, y + 18 + (indeks / 4) * 32, 24, 24, kolory[indeks]);
}

void __attribute__((section(".text.entry"))) kernel_main(void)
{
    __asm__ volatile ("movb $'A', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    rysowanie_init();
    __asm__ volatile ("movb $'B', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    zegar_init();
    __asm__ volatile ("movb $'C', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    {
        char tekst_czasu[9];
        zegar_pobierz(tekst_czasu);
        ustaw_czas_paska(tekst_czasu);
    }
    __asm__ volatile ("movb $'D', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    init_fs();
    __asm__ volatile ("movb $'E', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    tapeta_wczytaj();
    okno_stworz(&okno_tapeta, 248, 80, 360, 170, "TAPETA", rysuj_wybor_tapety);
    okno_ustaw_obsluge_klikniecia(&okno_tapeta, obsluz_wybor_tapety);
    __asm__ volatile ("movb $'F', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    menu_start();
    __asm__ volatile ("movb $'G', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    mouse_init();
    __asm__ volatile ("movb $'H', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    keyboard_init();
    __asm__ volatile ("movb $'I', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    __asm__ volatile ("movb $'J', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    ustaw_status_internetu((uint8_t)net_ready());
    if (net_ready()) {
        int dlugosc_odpowiedzi = net_http_get("info.cern.ch", "/", odpowiedz_czasu_http,
                                               sizeof(odpowiedz_czasu_http));
        if (dlugosc_odpowiedzi > 0)
            zegar_synchronizuj_http(odpowiedz_czasu_http, (uint32_t)dlugosc_odpowiedzi);
    }
    __asm__ volatile ("movb $'K', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    init_intel_hda();
    __asm__ volatile ("movb $'L', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    pulp_init();
    __asm__ volatile ("movb $'M', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    while(1) {
        char tekst_czasu[9];
        if (zegar_pobierz(tekst_czasu)) ustaw_czas_paska(tekst_czasu);
        mouse_poll();
        kursor_tekstowy_tick();
        if (aktywne_okno && aktywne_okno->otwarte && aktywne_okno->w_polu_tekstowym) {
            char znak;
            while (keyboard_try_getchar(&znak)) {
                uint8_t ctrl = keyboard_ctrl_pressed();
                if (aktywne_okno->pole_tekstowe_znak) {
                    if (!aktywne_okno->pole_tekstowe_znak(aktywne_okno, znak, ctrl)) break;
                }
            }
        } else {
            browser_handle_keyboard();
            eksplorator_plikow_obsluz_klawiature();
            notatnik_obsluz_klawiature();
        }
        if (keyboard_windows_pressed()) {
            menu_start_przełącz();
        }
        keyboard_discard_pending();
        if (!aktywne_okno || !aktywne_okno->otwarte) pulp_odswiez();
    }
}