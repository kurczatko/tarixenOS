/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/apps/wyp.hpp>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <drivers/keyboard.h>

void wyp() { // funkcja wypisywania tekstu
    vga_init();
    keyboard_init();
    printf(" napisz cos:\n");
    while(1) {
        char c = keyboard_getchar();
        printf("%c", c);
    }
}