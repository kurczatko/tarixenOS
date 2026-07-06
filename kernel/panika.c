#include "panika.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../include/blue.h"

void panika() {
    blue();
    vga_init();
    keyboard_init();
    printf(" !!!!!!!!!!!!!!!!!!!\n");
    printf(" system sie popsul!\n");
    printf(" this is the end!\n");
    printf(" ==================\n");
    printf(" jesli system sie popsul bo uzyles do tego komendy to ok\n");
    printf(" jesli samo to wyslij to (skarge) na kurczatkojestslodkie@gmail.com\n");
    int zawsze = 1;
    while(zawsze) {
        keyboard_getchar();
    }
}