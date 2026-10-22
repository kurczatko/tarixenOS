#include "keyboard.h"
#include "../core/hid.h"

static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t windows_pressed = 0;
static char kbd_buf[16];
static int kbd_buf_head = 0;
static int kbd_buf_tail = 0;

static const char hid_to_ascii_lower[256] = {
    0,0,0,0,'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
    'p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6',
    '7','8','9','0',0,0,0,0,0,0,0,0,0,0,0,0,' ',0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const char hid_to_ascii_upper[256] = {
    0,0,0,0,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
    'P','Q','R','S','T','U','V','W','X','Y','Z','!','@','#','$','%','^',
    '&','*','(',')',0,0,0,0,0,0,0,0,0,0,0,0,' ',0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static void kbd_callback(uint8_t keycode, uint8_t pressed) {
    (void)pressed;
    if (keycode == 0xE0 || keycode == 0xE1) {
        shift_pressed = 1;
        return;
    }
    if (keycode == 0xE2 || keycode == 0xE3) {
        shift_pressed = 0;
        return;
    }
    if (keycode == 0xE4) {
        ctrl_pressed = 1;
        return;
    }
    if (keycode == 0xE5) {
        ctrl_pressed = 0;
        return;
    }
    if (keycode == 0xE6) {
        windows_pressed = 1;
        return;
    }
    if (keycode == 0xE7) {
        windows_pressed = 0;
        return;
    }
    if (keycode == 0xE8) {
        caps_lock = !caps_lock;
        return;
    }
    if (kbd_buf_head - kbd_buf_tail >= 16) return;
    char c = 0;
    if (keycode >= 0x04 && keycode <= 0x65) {
        if (shift_pressed) c = hid_to_ascii_upper[keycode];
        else c = hid_to_ascii_lower[keycode];
        if (caps_lock && c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    }
    if (c != 0) {
        kbd_buf[kbd_buf_head % 16] = c;
        kbd_buf_head++;
    }
}

void usb_keyboard_init(void) {
    int i;
    for (i = 0; i < usb_device_count(); i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;
        if (dev->class == USB_CLASS_HID) {
            if (hid_register_keyboard(dev, kbd_callback) == 0) {
                return;
            }
        }
    }
}

void usb_keyboard_poll(void) {
}

uint8_t usb_keyboard_try_getchar(char *character) {
    if (kbd_buf_head == kbd_buf_tail) return 0;
    *character = kbd_buf[kbd_buf_tail % 16];
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
    uint8_t tmp = ctrl_pressed;
    ctrl_pressed = 0;
    return tmp;
}
