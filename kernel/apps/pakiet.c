#include <kernel/apps/pakiet.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <drivers/keyboard.h>

void pakiet() {
    vga_init();
    keyboard_init();
    printf(" napisz dokument:\n");
    while(1) {
        char c = keyboard_getchar();
        printf("%c", c);
    }
}
