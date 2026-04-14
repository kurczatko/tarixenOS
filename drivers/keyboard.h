#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../include/printf/types.h"

// Porty PS/2 keyboard
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Funkcje
void keyboard_init(void);
char keyboard_getchar(void);
uint8_t keyboard_scancode_to_char(uint8_t scancode);

// Mapa scancodes do char (US layout)
static const char keyboard_us[128] = {
    0,  0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  // 0-15
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,  // 16-31
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',       // 32-47
    'z', 'x', 'c', 'v', 'b', 'n', 'm', '^', '{', '}', 0, '*', 0, ' ', 0         // 48-63
};

#endif

