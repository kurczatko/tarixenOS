#ifndef USB_H
#define USB_H

#include "../../../include/printf/types.h"
#include "usb_hal.h"

#define USB_CLASS_HID          0x03
#define USB_CLASS_MASS_STORAGE 0x08

#define USB_DIR_OUT 0x00
#define USB_DIR_IN  0x80
#define USB_TYPE_STANDARD 0x00
#define USB_TYPE_CLASS    0x20
#define USB_TYPE_VENDOR   0x40
#define USB_RECIP_DEVICE  0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT 0x02

#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_SET_CONFIGURATION 0x09

#define USB_MAX_DEVICES 8
#define USB_MAX_ENDPOINTS 4

#define USB_DT_DEVICE        0x01
#define USB_DT_CONFIG        0x02
#define USB_DT_STRING        0x03
#define USB_DT_INTERFACE     0x04
#define USB_DT_ENDPOINT      0x05
#define USB_DT_HID           0x21
#define USB_DT_REPORT        0x22

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
    USB_ENDPOINT_INTERRUPT = 1,
    USB_ENDPOINT_BULK = 2,
    USB_ENDPOINT_ISO = 3
} usb_endpoint_type_t;
typedef struct {
    uint8_t  used;
    uint8_t  address;
    uint8_t  speed;
    uint8_t  configured;
    uint8_t  max_packet_size0;
    uint16_t id_vendor;
    uint16_t id_product;
    uint8_t  class;
    uint8_t  subclass;
    uint8_t  protocol;
    uint8_t  port;
    uint16_t io_base;

    struct {
        uint8_t  used;
        uint8_t  address;
        uint8_t  type;
        uint8_t  max_packet;
        uint8_t  toggle;
    } ep[USB_MAX_ENDPOINTS];

    void *class_data;
} usb_device_t;

void usb_init(void);
void usb_poll(void);
usb_device_t *usb_device_get(int index);
int usb_device_count(void);
int usb_control_transfer(usb_device_t *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data);
int usb_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction);
int usb_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length);

#endif
