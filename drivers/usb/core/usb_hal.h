#ifndef USB_HAL_H
#define USB_HAL_H

#include "../../../include/printf/types.h"


uint8_t usb_hal_inb(uint16_t port);
void    usb_hal_outb(uint16_t port, uint8_t value);
uint16_t usb_hal_inw(uint16_t port);
void    usb_hal_outw(uint16_t port, uint16_t value);
uint32_t usb_hal_inl(uint16_t port);
void    usb_hal_outl(uint16_t port, uint32_t value);

static inline uint8_t usb_inb(uint16_t port) {
    return usb_hal_inb(port);
}
static inline void usb_outb(uint16_t port, uint8_t value) {
    usb_hal_outb(port, value);
}
static inline uint16_t usb_inw(uint16_t port) {
    return usb_hal_inw(port);
}
static inline void usb_outw(uint16_t port, uint16_t value) {
    usb_hal_outw(port, value);
}
static inline uint32_t usb_inl(uint16_t port) {
    return usb_hal_inl(port);
}
static inline void usb_outl(uint16_t port, uint32_t value) {
    usb_hal_outl(port, value);
}

#endif
