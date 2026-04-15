#include "konfiguracja.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../include/blue.h"
#include <pik.h>

void konfiguracja() {
    keyboard_init();
    vga_init();
    blue();
    beep();
    printf("                                      tarixeniOS                                                                                                                                                                                                   \n");
    printf("Witam w Systemie tarixeniOS!                                                                                                              \n");
    printf("nasz system jest dopiero co stworzony wiec sie nie dziw ze jest niedokonczony                                                                                                                                                                          \n");
    printf("mam nadzieje ze ci sie spodoba                                                                                                                                         \n");
    printf("napisz cos aby wyjsc z konfiguracji                                                                                                                                                                                                             \n");
    printf("                                                                                                                                                                                                                                                   ");
    keyboard_getchar();
    blue();
    blue();
}
