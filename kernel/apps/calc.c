#include <kernel/apps/calc.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <drivers/keyboard.h>

void calc() {
        keyboard_init();
        vga_init();
        printf(" siema w kalkulatorze\n");
        printf(" wpisz pierwsza liczbe:\n");
        char n1 = keyboard_getchar();
        // printf(" napisz jakie dzialanie\n");
        // char d = keyboard_getchar();
        printf(" teraz druga:\n");
        char n2 = keyboard_getchar();
        printf(" wynik: %d\n", (n1 - '0') + (n2 - '0'));
        /*
        if (d == '-') {
            printf(" wynik:");
            printf(n1 - n2);
        }
        if (d == '/') {
            if (n1 && n2 == 0) {
                printf(" dzielenie przez zero (ciesz sie ze wylapal to system bo kernel bys rozwalil se)\n");
            } else {
                printf(" wynik:");
                printf(n1 / n2);
            }
        } */
}