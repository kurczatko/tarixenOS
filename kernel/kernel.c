// clovaos by radoslaw sitarski 2026 v1.0

#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "shell.h"
#include "../include/blue.h"
#include "konfiguracja.h"

void kernel_main(void)
{
    while(1) {
        blue();
        konfiguracja();
        vga_init();
        printf("clovaos by radoslaw sitarski 2026 shell. write command!\n");
        shell();
    }
}
