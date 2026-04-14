#include "konfiguracja.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../include/blue.h"

void konfiguracja() {
    keyboard_init();
    vga_init();
    blue();
    printf("                                      XentariOS                                                                                                                                                                                                   \n");
    printf("Witam w Systemie XentariOS!                                                                                                              \n");
    printf("nasz system jest dopiero co stworzony wiec sie nie dziw ze jest niedokonczony                                                                                                                                                                          \n");
    printf("mam nadzieje ze ci sie spodoba                                                                                                                                         \n");
    printf("napisz cos aby wyjsc z konfiguracji                                                                                                                                                                                                             \n");
    printf("                                                                                                                                                                                                                                                   ");
    keyboard_getchar();
    blue();
}