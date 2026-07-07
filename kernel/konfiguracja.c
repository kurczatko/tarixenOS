#include "konfiguracja.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../include/blue.h"

void konfiguracja() { /* nie dotykać jeśli nie trzeba bo to raczej ważne co pierwsze widzi uzytkownik */
    keyboard_init();
    vga_init();
    blue();
    printf("===================================================================================\n"); // zwracanie uwagi uzytkownikowi
    printf("|                                      tarixenOS                                 X|                                                                                                                                                                \n");
    printf("| Witam w Systemie tarixenOS!                                                     |                                                          \n");
    printf("| nasz system jest dopiero co stworzony wiec sie nie dziw ze jest niedokonczony   |                                                                                                                                                                         \n");
    printf("| mam nadzieje ze ci sie spodoba                                                  |                                                                                       \n");
    printf("| kliknij enter aby wyjsc z konfiguracji                                          |                                                                                                                                                                   \n");
    printf("===================================================================================                                                                                                                                                                                                                                                   ");
    keyboard_getchar();
}
