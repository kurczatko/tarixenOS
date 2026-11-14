/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <include/printf/types.h>

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a" (ret) : "Nd" (port) );
    return ret;
}

static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;
static uint8_t extended_scancode = 0;
static uint8_t windows_key_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t altgr_pressed = 0;

static uint8_t utf8_kolejka[8];
static uint8_t utf8_glowa = 0;
static uint8_t utf8_liczba = 0;

static void utf8_wstaw(uint8_t bajt1, uint8_t bajt2)
{
    if (utf8_liczba > 6) return;
    uint8_t pozycja = (utf8_glowa + utf8_liczba) & 7;
    utf8_kolejka[pozycja] = bajt1;
    utf8_kolejka[(pozycja + 1) & 7] = bajt2;
    utf8_liczba += 2;
}

void keyboard_init(void) {
    while (inb(0x64) & 2);
}

uint8_t keyboard_dane_dostepne(void) {
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    return (status & 1) && !(status & 0x20);
}

uint8_t keyboard_try_getchar(char *character)
{
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    uint8_t scancode;
    char value;

    if (utf8_liczba > 0) {
        *character = (char)utf8_kolejka[utf8_glowa];
        utf8_glowa = (utf8_glowa + 1) & 7;
        utf8_liczba--;
        return 1;
    }

    if (!(status & 1) || (status & 0x20)) return 0;
    scancode = inb(KEYBOARD_DATA_PORT);

    if (extended_scancode) {
        extended_scancode = 0;
        if (scancode == 0x5B || scancode == 0x5C) {
            windows_key_pressed = 1;
        } else if (scancode == 0x38) {
            altgr_pressed = 1;
        } else if (scancode == 0xB8) {
            altgr_pressed = 0;
        }
        return 0;
    }
    if (scancode == 0xE0) {
        extended_scancode = 1;
        return 0;
    }
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return 0;
    }
    if (scancode == 0x1D) {
        ctrl_pressed = 1;
        return 0;
    }
    if (scancode == 0x9D) {
        ctrl_pressed = 0;
        return 0;
    }
    if (scancode == 0x38) {
        alt_pressed = 1;
        return 0;
    }
    if (scancode == 0xB8) {
        alt_pressed = 0;
        return 0;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return 0;
    }
    if (scancode & 0x80) return 0;

    if (altgr_pressed) {
        uint8_t baza = keyboard_us[scancode];
        uint8_t wielka = shift_pressed;
        if (baza == 'a') utf8_wstaw(0xC4, wielka ? 0x84 : 0x85);
        else if (baza == 'c') utf8_wstaw(0xC4, wielka ? 0x86 : 0x87);
        else if (baza == 'e') utf8_wstaw(0xC4, wielka ? 0x98 : 0x99);
        else if (baza == 'l') utf8_wstaw(0xC5, wielka ? 0x81 : 0x82);
        else if (baza == 'n') utf8_wstaw(0xC5, wielka ? 0x83 : 0x84);
        else if (baza == 'o') utf8_wstaw(0xC3, wielka ? 0x93 : 0xB3);
        else if (baza == 's') utf8_wstaw(0xC5, wielka ? 0x9A : 0x9B);
        else if (baza == 'x') utf8_wstaw(0xC5, wielka ? 0xBB : 0xBC);
        else if (baza == 'z') utf8_wstaw(0xC5, wielka ? 0xB9 : 0xBA);
        if (utf8_liczba > 0) {
            *character = (char)utf8_kolejka[utf8_glowa];
            utf8_glowa = (utf8_glowa + 1) & 7;
            utf8_liczba--;
            return 1;
        }
        return 0;
    }

    value = (char)keyboard_scancode_to_char(scancode, shift_pressed);
    if (value >= 'a' && value <= 'z' && (shift_pressed != caps_lock))
        value = value - 'a' + 'A';
    if (value == 0) return 0;
    *character = value;
    return 1;
}

void keyboard_discard_pending(void) {
    char ignored;
    keyboard_try_getchar(&ignored);
}

uint8_t keyboard_windows_pressed(void) {
    uint8_t tmp = windows_key_pressed;
    windows_key_pressed = 0;
    return tmp;
}

uint8_t keyboard_ctrl_pressed(void) {
    uint8_t tmp = ctrl_pressed;
    ctrl_pressed = 0;
    return tmp;
}

uint8_t keyboard_alt_pressed(void) {
    uint8_t tmp = alt_pressed;
    alt_pressed = 0;
    return tmp;
}

char keyboard_getchar(void) {
    char znak;
    while (1) {
        mouse_poll();
        if (utf8_liczba > 0) {
            znak = (char)utf8_kolejka[utf8_glowa];
            utf8_glowa = (utf8_glowa + 1) & 7;
            utf8_liczba--;
            return znak;
        }

        uint8_t scancode;
        while (!(inb(KEYBOARD_STATUS_PORT) & 1) ||
               (inb(KEYBOARD_STATUS_PORT) & 0x20)) {
            mouse_poll();
        }
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
            char character = keyboard_scancode_to_char(scancode, shift_pressed);

            if (character >= 'a' && character <= 'z' &&
                (shift_pressed != caps_lock)) {
                character = character - 'a' + 'A';
            }
            return character;
        }
    }
}

static const char keyboard_us_shift[128] = {
    0,  0,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',   // klawisze 0-15
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,  // klawisze 16-31
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',        // klawisze 32-47
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0         // klawisze 48-63
};

uint8_t keyboard_scancode_to_char(uint8_t scancode, uint8_t shift) {
    if (scancode < 128) {
        char c = keyboard_us[scancode];
        if (shift && !(c >= 'a' && c <= 'z')) {
            c = keyboard_us_shift[scancode];
        }
        return (uint8_t)c;
    }
    return 0;
}
