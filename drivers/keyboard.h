#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../include/printf/types.h"

// porty
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// funkcje klawiaturowe
void keyboard_init(void);
char keyboard_getchar(void);
uint8_t keyboard_scancode_to_char(uint8_t scancode);

// mapa klawiszy chińska, a nie sory, znaczy angielska (czy tam USAńska)
static const char keyboard_us[128] = {
    0,  0,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   // klawisze 0-15
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,  // klawisze 16-31
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',       // klawisze 32-47
    'z', 'x', 'c', 'v', 'b', 'n', 'm', '^', '{', '}', 0, '*', 0, ' ', 0         // klawisze 48-63
};

// ten uklad tez zostanie wywalony

/* static const char keyboard_pr[128] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0
}; */

#endif

