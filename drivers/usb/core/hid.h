#ifndef HID_H
#define HID_H

#include "../core/usb.h"

typedef void (*hid_keyboard_callback)(uint8_t keycode, uint8_t pressed);
typedef void (*hid_mouse_callback)(int8_t dx, int8_t dy, int8_t dz, uint8_t buttons);

void hid_init(void);
void hid_poll(void);
int hid_register_keyboard(usb_device_t *dev, hid_keyboard_callback cb);
int hid_register_mouse(usb_device_t *dev, hid_mouse_callback cb);
int hid_set_protocol(usb_device_t *dev, uint8_t protocol);
int hid_get_report_descriptor(usb_device_t *dev, void *buf, uint16_t size);

#endif
