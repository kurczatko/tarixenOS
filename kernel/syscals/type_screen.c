/* type_screen.c */
/* wywolanie systemowe napisania*/

#include "type_screen.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"

void type_screen(char *str) {
    vga_init();
    printf("%s", str);
}