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
#include "gdt.h"
#include "idt.h"
#include "syscall.h"
#include "../user_land/tapeta.h"

extern struct okno *aktywne_okno;

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
    gdt_install();
    idt_install();
    syscall_install();
    pulp_init();
    tapeta_app();
    __asm__ volatile ("movb $'G', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    menu_start();
    __asm__ volatile ("movb $'H', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    mouse_init();
    __asm__ volatile ("movb $'I', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    keyboard_init();
    __asm__ volatile ("movb $'J', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
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
