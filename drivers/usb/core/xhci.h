#ifndef XHCI_H
#define XHCI_H

#include <drivers/usb/core/usb.h>

int xhci_add_controller(uint8_t bus, uint8_t slot, uint8_t func, uint32_t bar0);
int xhci_init_all(void);
void xhci_poll(void);
int xhci_control_transfer(usb_device_t *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data);
int xhci_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction);
int xhci_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length);
int xhci_configure_endpoints(usb_device_t *dev);
int xhci_update_ep0_mps(usb_device_t *dev, uint16_t mps);

#endif
