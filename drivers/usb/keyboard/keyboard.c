#include <drivers/usb/keyboard/keyboard.h>
#include <drivers/usb/core/hid.h>
#include <drivers/input/keyboard.h>

static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t windows_pressed = 0;
static char kbd_buf[32];
static uint32_t kbd_buf_head = 0;
static uint32_t kbd_buf_tail = 0;

static char hid_to_ascii(uint8_t keycode, uint8_t upper) {
    if (keycode >= 0x04 && keycode <= 0x1D) {
        char base = (char)('a' + (keycode - 0x04));
        if (upper) base = (char)(base - 'a' + 'A');
        return base;
    }
    if (keycode >= 0x1E && keycode <= 0x27) {
        static const char num_lo[10] = { '1','2','3','4','5','6','7','8','9','0' };
        static const char num_hi[10] = { '!','@','#','$','%','^','&','*','(',')' };
        return upper ? num_hi[keycode - 0x1E] : num_lo[keycode - 0x1E];
    }
    switch (keycode) {
        case 0x28: return '\n';
        case 0x2A: return '\b';
        case 0x2B: return '\t';
        case 0x2C: return ' ';
        case 0x2D: return upper ? '_' : '-';
        case 0x2E: return upper ? '+' : '=';
        case 0x2F: return upper ? '{' : '[';
        case 0x30: return upper ? '}' : ']';
        case 0x31: return upper ? '|' : '\\';
        case 0x32: return upper ? '~' : '#';
        case 0x33: return upper ? ':' : ';';
        case 0x34: return upper ? '"' : '\'';
        case 0x35: return upper ? '~' : '`';
        case 0x36: return upper ? '<' : ',';
        case 0x37: return upper ? '>' : '.';
        case 0x38: return upper ? '?' : '/';
        case 0x4A: return KEYBOARD_HOME;
        case 0x4C: return KEYBOARD_DEL;
        case 0x4D: return KEYBOARD_END;
        case 0x4F: return KEYBOARD_RIGHT;
        case 0x50: return KEYBOARD_LEFT;
        case 0x51: return KEYBOARD_DOWN;
        case 0x52: return KEYBOARD_UP;
        case 0x54: return '/';
        case 0x55: return '*';
        case 0x56: return '-';
        case 0x57: return '+';
        case 0x58: return '\n';
        case 0x59: return '1';
        case 0x5A: return '2';
        case 0x5B: return '3';
        case 0x5C: return '4';
        case 0x5D: return '5';
        case 0x5E: return '6';
        case 0x5F: return '7';
        case 0x60: return '8';
        case 0x61: return '9';
        case 0x62: return '0';
        case 0x63: return '.';
        default: return 0;
    }
}

static void kbd_callback(uint8_t keycode, uint8_t pressed) {
    (void)pressed;
    if (keycode == 0xE0 || keycode == 0xE1) { shift_pressed = 1; return; }
    if (keycode == 0xE2 || keycode == 0xE3) { shift_pressed = 0; return; }
    if (keycode == 0xE4) { ctrl_pressed = 1; return; }
    if (keycode == 0xE5) { ctrl_pressed = 0; return; }
    if (keycode == 0xE6) { windows_pressed = 1; return; }
    if (keycode == 0xE7) { windows_pressed = 0; return; }
    if (keycode == 0xE8) { caps_lock = !caps_lock; return; }
    if (keycode == 0xE9) { alt_pressed = 1; return; }
    if (keycode == 0xEA) { alt_pressed = 0; return; }

    if (kbd_buf_head - kbd_buf_tail >= 32) return;
    uint8_t upper = shift_pressed;
    if (keycode >= 0x04 && keycode <= 0x1D) {
        upper = (uint8_t)(shift_pressed != caps_lock);
    }
    char c = hid_to_ascii(keycode, upper);
    if (c != 0) {
        kbd_buf[kbd_buf_head % 32] = c;
        kbd_buf_head++;
    }
}

int usb_keyboard_init(void) {
    int registered = 0;
    for (int i = 0; i < usb_device_count(); i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;
        if (dev->dev_class != USB_CLASS_HID) continue;
        /* hid_register_keyboard sam sprawdza z deskryptora raportu, czy to klawiatura. */
        if (hid_register_keyboard(dev, kbd_callback) == 0) registered++;
    }
    return registered > 0 ? 1 : 0;
}

void usb_keyboard_poll(void) {
}

uint8_t usb_keyboard_dane_dostepne(void) {
    return (kbd_buf_head != kbd_buf_tail) ? 1 : 0;
}

uint8_t usb_keyboard_try_getchar(char *character) {
    if (kbd_buf_head == kbd_buf_tail) return 0;
    *character = kbd_buf[kbd_buf_tail % 32];
    kbd_buf_tail++;
    return 1;
}

void usb_keyboard_discard_pending(void) {
    kbd_buf_head = 0;
    kbd_buf_tail = 0;
}

uint8_t usb_keyboard_windows_pressed(void) {
    uint8_t tmp = windows_pressed;
    windows_pressed = 0;
    return tmp;
}

uint8_t usb_keyboard_ctrl_pressed(void) {
    return ctrl_pressed;
}

uint8_t usb_keyboard_alt_pressed(void) {
    return alt_pressed;
}