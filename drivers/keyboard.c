#include "keyboard.h"
#include "mouse.h"
#include "../include/printf/types.h"

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a" (ret) : "Nd" (port) );
    return ret;
}

static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;

void keyboard_init(void) {
    while (inb(0x64) & 2);
}

char keyboard_getchar(void) {
    uint8_t scancode;
    while (1) {
        mouse_poll();
        while (!(inb(KEYBOARD_STATUS_PORT) & 1) ||
               (inb(KEYBOARD_STATUS_PORT) & 0x20)) {
            mouse_poll();
        } //nareszcie naprawiłem to ze miedzy literami jest odstep a nie powinno byc, łuhu!!!!!
        scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            continue;
        }

        if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = 0;
            continue;
        }

        if (scancode == 0x3A) {
            caps_lock = !caps_lock;
            continue;
        }

        if (scancode == 0xE0) {
            while (!(inb(KEYBOARD_STATUS_PORT) & 1));
            scancode = inb(KEYBOARD_DATA_PORT);
            if (scancode == 0x4B) return KEYBOARD_LEFT;
            if (scancode == 0x4D) return KEYBOARD_RIGHT;
            if (scancode == 0x48) return KEYBOARD_UP;
            if (scancode == 0x50) return KEYBOARD_DOWN;
            continue;
        }

        if (scancode < 0x80) {
            char character = keyboard_scancode_to_char(scancode);

            if (character >= 'a' && character <= 'z' &&
                (shift_pressed != caps_lock)) {
                character = character - 'a' + 'A';
            }
            return character;
        }
    }
}

uint8_t keyboard_scancode_to_char(uint8_t scancode) {
    if (scancode < 128) {
        return keyboard_us[scancode];
    }
    return 0;
}
