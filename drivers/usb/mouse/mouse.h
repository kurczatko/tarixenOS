#ifndef USB_MOUSE_H
#define USB_MOUSE_H

#include <drivers/usb/core/usb.h>

void usb_mouse_init(void);
void usb_mouse_poll(void);
void usb_mouse_pozycja(int *x, int *y);

#endif
