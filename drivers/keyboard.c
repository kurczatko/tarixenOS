#include "keyboard.h"
#include "../include/printf/types.h"

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a" (ret) : "Nd" (port) );
    return ret;
}

void keyboard_init(void) {
    while (inb(0x64) & 2);
}

char keyboard_getchar(void) {
    uint8_t scancode;
    do {
        while (!(inb(KEYBOARD_STATUS_PORT) & 1)); //nareszcie naprawiłem to ze miedzy literami jest odstep a nie powinno byc, łuhu!!!!!
        scancode = inb(KEYBOARD_DATA_PORT);
    } while (scancode >= 0x80);
    return keyboard_scancode_to_char(scancode);
}

uint8_t keyboard_scancode_to_char(uint8_t scancode) {
    if (scancode < 128) {
        return keyboard_us[scancode];
    }
    return 0;
}
