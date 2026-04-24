#include "wyp.hpp"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"

void wyp() { // funkcja wypisywania tekstu
    vga_init();
    keyboard_init();
    printf(" napisz cos:\n");
    while(1) {
        char c = keyboard_getchar();
        printf("%c", c);
    }
}