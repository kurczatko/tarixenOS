#include "mouse.h"
#include "../core/hid.h"
#include "../../../graficzny/rysowanie.h"

static int usb_mouse_x = 0;
static int usb_mouse_y = 0;
static uint8_t usb_mouse_buttons = 0;

static void mouse_callback(int8_t dx, int8_t dy, int8_t dz, uint8_t buttons) {
    (void)dz;
    usb_mouse_buttons = buttons;
    usb_mouse_x += dx;
    usb_mouse_y += dy;
    if (usb_mouse_x < 0) usb_mouse_x = 0;
    if (usb_mouse_y < 0) usb_mouse_y = 0;
    kursor_graficzny_przesun(dx, -dy);
}

void usb_mouse_init(void) {
    int i;
    for (i = 0; i < usb_device_count(); i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;
        if (dev->class == USB_CLASS_HID) {
            if (hid_register_mouse(dev, mouse_callback) == 0) {
                return;
            }
        }
    }
}

void usb_mouse_poll(void) {
}

void usb_mouse_pozycja(int *x, int *y) {
    *x = usb_mouse_x;
    *y = usb_mouse_y;
}
