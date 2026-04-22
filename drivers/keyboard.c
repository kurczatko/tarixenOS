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
    while (!(inb(KEYBOARD_STATUS_PORT) & 1));
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    return keyboard_scancode_to_char(scancode);
}

uint8_t keyboard_scancode_to_char(uint8_t scancode) {
    if (scancode < 128) {
        return keyboard_us[scancode];
    }
    return 0;
}
