#include "../include/printf/vga.h"
#include "shell.h"
#include "../include/blue.h"
#include "konfiguracja.h"
#include "../scripts/ekran_bootowania.h"
#include "../../drivers/keyboard.h"
#include "panika.h"
#include "../drivers/mouse.h"
#include "../system_plikow/system_plikow.h"

void kernel_main(void)
{
    vga_init();
    mouse_init();
    keyboard_init();
    blue();
    ekran_bootowania();
    konfiguracja();
    init_fs();
    while(1) {
        shell();
    }
    panika();   // kernel panic jak shell sie skonczy (nie powinien sie skonczyc)
}
