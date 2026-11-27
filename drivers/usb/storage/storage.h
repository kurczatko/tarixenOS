#ifndef USB_STORAGE_H
#define USB_STORAGE_H

#include <drivers/usb/core/usb.h>

int usb_storage_init(void);
void usb_storage_poll(void);
int usb_storage_present(void);
uint32_t usb_storage_read_sectors(uint32_t lba, uint32_t count, void *buffer);
uint32_t usb_storage_write_sectors(uint32_t lba, uint32_t count, const void *buffer);
uint32_t usb_storage_get_capacity(void);
uint32_t usb_storage_get_sector_size(void);

#endif
