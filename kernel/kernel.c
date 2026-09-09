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

void kernel_main(void)
{
    rysowanie_init();
    mouse_init();
    keyboard_init();
    while(1) {
        mouse_poll();
    }
}
