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

void __attribute__((section(".text.entry"))) kernel_main(void)
{
    rysowanie_init();
    menu_start();
    mouse_init();
    keyboard_init();
    while(1) {
        mouse_poll();
    }
}
