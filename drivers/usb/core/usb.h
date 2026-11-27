#ifndef USB_H
#define USB_H

#include <include/printf/types.h>
#include <drivers/usb/core/usb_hal.h>

#define USB_CLASS_HID          0x03
#define USB_CLASS_MASS_STORAGE 0x08
#define USB_CLASS_HUB          0x09

#define USB_DIR_OUT 0x00
#define USB_DIR_IN  0x80
#define USB_TYPE_STANDARD 0x00
#define USB_TYPE_CLASS    0x20
#define USB_TYPE_VENDOR   0x40
#define USB_RECIP_DEVICE  0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT 0x02
#define USB_RECIP_OTHER   0x03

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B

#define USB_MAX_DEVICES 16
#define USB_MAX_ENDPOINTS 8

#define USB_SPEED_FULL  0
#define USB_SPEED_LOW   1
#define USB_SPEED_HIGH  2
#define USB_SPEED_SUPER 3

#define USB_CTRL_UHCI 0
#define USB_CTRL_XHCI 1

#define USB_DT_DEVICE        0x01
#define USB_DT_CONFIG        0x02
#define USB_DT_STRING        0x03
#define USB_DT_INTERFACE     0x04
#define USB_DT_ENDPOINT      0x05
#define USB_DT_HID           0x21
#define USB_DT_REPORT        0x22
#define USB_DT_HUB           0x29

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} __attribute__((packed)) usb_config_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
} __attribute__((packed)) usb_interface_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} __attribute__((packed)) usb_endpoint_descriptor_t;

typedef enum {
    USB_ENDPOINT_CONTROL = 0,
    USB_ENDPOINT_ISO = 1,
    USB_ENDPOINT_BULK = 2,
    USB_ENDPOINT_INTERRUPT = 3
} usb_endpoint_type_t;

typedef struct {
    uint8_t  used;
    uint8_t  address;
    uint8_t  speed;
    uint8_t  configured;
    uint8_t  max_packet_size0;
    uint16_t id_vendor;
    uint16_t id_product;
    uint8_t  dev_class;
    uint8_t  subclass;
    uint8_t  protocol;
    uint8_t  port;
    uint16_t io_base;
    uint8_t  interface_num;
    uint8_t  controller_type;
    uint8_t  controller_idx;
    uint8_t  slot_id;
    uint8_t  root_port;
    uint32_t route_string;

    struct {
        uint8_t  used;
        uint8_t  address;
        uint8_t  type;
        uint16_t max_packet;
        uint8_t  toggle;
        uint8_t  interval;
    } ep[USB_MAX_ENDPOINTS];

    void *class_data;
} usb_device_t;

int usb_init(void);
void usb_poll(void);
usb_device_t *usb_device_get(int index);
int usb_device_count(void);
int usb_control_transfer(usb_device_t *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data);
int usb_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction);
int usb_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length);


usb_device_t *usb_alloc_device_slot(void);
void usb_free_device_slot(usb_device_t *dev);
int usb_enumerate_common(usb_device_t *dev, uint8_t assign_address);
uint32_t usb_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);
uint16_t usb_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);
void usb_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint16_t val);
void usb_delay_ms(uint32_t ms);

#endif
