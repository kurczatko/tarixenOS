/* type_add.c */
/* wywolanie systemowe dodawania */

#include "type_add.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"

void type_add() {
    int liczba1 = 0; // zmien liczbe jesli tworzyz program i chcesz uzyc tego wywolania
    int liczba2 = 0;
    int wynik = liczba1 + liczba2;
    vga_init();
    printf("%d + %d = %d", liczba1, liczba2, wynik);
}