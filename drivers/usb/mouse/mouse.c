#include <drivers/usb/mouse/mouse.h>
#include <drivers/usb/core/hid.h>
#include <drivers/input/mouse.h>
#include <graphics/rysowanie.h>

static uint8_t usb_mouse_buttons = 0;

static void mouse_callback(int8_t dx, int8_t dy, int8_t dz, uint8_t buttons) {
    usb_mouse_buttons = buttons;
    mouse_process_event((int)dx, (int)dy, (int)dz, buttons);
}

int usb_mouse_init(void) {
    int registered = 0;
    for (int i = 0; i < usb_device_count(); i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;
        if (dev->dev_class != USB_CLASS_HID) continue;
        /* hid_register_mouse sam sprawdza z deskryptora raportu, czy to mysz. */
        if (hid_register_mouse(dev, mouse_callback) == 0) registered++;
    }
    return registered > 0 ? 1 : 0;
}

/* Odpytywanie HID robi hid_poll() wywolywane z petli glownej; tu nic nie robimy,
 * zeby raporty nie byly czytane dwa razy na jeden obrot petli. */
void usb_mouse_poll(void) {
}

void usb_mouse_pozycja(int *x, int *y) {
    kursor_graficzny_pozycja(x, y);
}