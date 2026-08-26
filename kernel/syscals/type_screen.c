/* type_screen.c */
/* wywolanie systemowe napisania*/

#include "type_screen.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"

void type_screen() {
    vga_init();
    char *type = "" // jesli tworzysz program i chcesz uzyc tego wywolania propsotu zapelnij ten string
    printf("%s", type);
}