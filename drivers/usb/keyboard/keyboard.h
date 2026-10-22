#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include "../core/usb.h"

void usb_keyboard_init(void);
void usb_keyboard_poll(void);
uint8_t usb_keyboard_try_getchar(char *character);
void usb_keyboard_discard_pending(void);
uint8_t usb_keyboard_windows_pressed(void);
uint8_t usb_keyboard_ctrl_pressed(void);

#endif
