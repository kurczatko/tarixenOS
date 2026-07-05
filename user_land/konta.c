#include "konta.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../include/blue.h"
#include "../kernel/shell.h"

void konta() {
    vga_init();
    keyboard_init();
    printf(" prosimy o utworzenie konta.\n");
    printf(" jak chcesz sie nazywac? (jedna litera)\n");
    char nazwa = keyboard_getchar();
    printf(" jakie haslo? (jedna litera) \n");
    char haslo = keyboard_getchar();
    blue();
    printf("%c\n", nazwa);
    printf(" wpisz haslo:");
    char zgdh = keyboard_getchar();
    if (zgdh == haslo) {
        printf(" dobrze!\n");
    } else {
        printf(" zle! wywalaj wlamywaczu\n");
        while(1) {
            keyboard_getchar();
        }
    }
}