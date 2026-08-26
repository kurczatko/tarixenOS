#include "ekran_bootowania.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../include/blue.h"
#include "../../drivers/keyboard.h"

void ekran_bootowania() {
    vga_init();
    blue();
    printf("tarixenOS\n");
    printf(".\n");
    printf("..\n");
    printf("...\n");
    printf("TarixenOS Beta\n");
    keyboard_getchar();
}
