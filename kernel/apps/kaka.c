#include "kaka.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"

void kaka() {
    keyboard_init();
    vga_init();
    printf("czy chcesz zdechnac?\n");
    printf("T/N\n");
    char odp = keyboard_getchar();

    if (odp == 'T' || odp == 't') {
        printf("zdechles!\n");
    } else if (odp == 'N' || odp == 'n') {
        printf("zyjesz!\n");
    } else {
        printf("naucz sie czytac\n");
    }
}
