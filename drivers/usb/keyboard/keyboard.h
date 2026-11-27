#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include <drivers/usb/core/usb.h>

int usb_keyboard_init(void);
void usb_keyboard_poll(void);
uint8_t usb_keyboard_dane_dostepne(void);
uint8_t usb_keyboard_try_getchar(char *character);
void usb_keyboard_discard_pending(void);
uint8_t usb_keyboard_windows_pressed(void);
uint8_t usb_keyboard_ctrl_pressed(void);
uint8_t usb_keyboard_alt_pressed(void);

#endif
