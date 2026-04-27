
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "shell.h"
#include "../include/blue.h"
#include "konfiguracja.h"
#include "../scripts/ekran_bootowania.h"
#include "../../drivers/keyboard.h"

void kernel_main(void)
{
    while(1) {
        keyboard_init();
        blue();
        konfiguracja();
        vga_init();
        printf("system jest prosty w uzyciu, napisz 'h' aby wejsc w menu pomocy.\n");
        printf("tarixenOS by radoslaw sitarski 2026 shell. write command!\n");
        shell();
    }
}
