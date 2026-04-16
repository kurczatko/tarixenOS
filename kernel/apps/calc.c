#include "calc.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"

void calc() {
    keyboard_init();
    vga_init();
    printf(" siema w kalkulatorze\n");
    printf(" wpisz pierwsza liczbe:\n");
    char n1 = keyboard_getchar();
    printf(" teraz druga:\n");
    char n2 = keyboard_getchar();
    printf(" wynik: %d\n", (n1 - '0') + (n2 - '0'));
}