#include "hid.h"
#include "usb.h"

#define HID_MAX_DEVICES 4
#define HID_REQ_GET_REPORT       0x01
#define HID_REQ_GET_IDLE         0x02
#define HID_REQ_GET_PROTOCOL     0x03
#define HID_REQ_SET_REPORT       0x09
#define HID_REQ_SET_IDLE         0x0A
#define HID_REQ_SET_PROTOCOL     0x0B
#define HID_DESCRIPTOR_TYPE      0x21
#define HID_REPORT_DESCRIPTOR    0x22

typedef struct {
    usb_device_t *dev;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t report_size;
    uint8_t report_buf[64];
    uint8_t protocol;
    int type;
    hid_keyboard_callback kbd_cb;
    hid_mouse_callback mouse_cb;
} hid_device_t;

static hid_device_t hid_devices[HID_MAX_DEVICES];
static int hid_device_num = 0;

static int hid_find_endpoint(usb_device_t *dev, uint8_t ep_type, uint8_t dir) {
    int i;
    for (i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (!dev->ep[i].used) continue;
        if (dev->ep[i].type != ep_type) continue;
        if ((dev->ep[i].address & 0x80) != (dir == 1 ? 0x80 : 0x00)) continue;
        return i;
    }
    return -1;
}

static int hid_get_descriptor(usb_device_t *dev, uint8_t desc_type, uint8_t index, uint16_t length, void *data) {
    uint8_t bmRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    uint16_t wValue = ((uint16_t)desc_type << 8) | index;
    return usb_control_transfer(dev, bmRequestType, HID_REQ_GET_REPORT, wValue, 0, length, data);
}

int hid_get_report_descriptor(usb_device_t *dev, void *buf, uint16_t size) {
    uint8_t bmRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
    uint16_t wValue = ((uint16_t)USB_DT_HID << 8) | 0;
    return usb_control_transfer(dev, bmRequestType, USB_REQ_GET_DESCRIPTOR, wValue, 0, size, buf);
}

int hid_set_protocol(usb_device_t *dev, uint8_t protocol) {
    uint8_t bmRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    return usb_control_transfer(dev, bmRequestType, HID_REQ_SET_PROTOCOL, protocol, 0, 0, 0);
}

int hid_register_keyboard(usb_device_t *dev, hid_keyboard_callback cb) {
    int ep = hid_find_endpoint(dev, USB_ENDPOINT_INTERRUPT, 1);
    if (ep < 0) return -1;
    if (hid_device_num >= HID_MAX_DEVICES) return -2;
    hid_devices[hid_device_num].dev = dev;
    hid_devices[hid_device_num].ep_in = dev->ep[ep].address;
    hid_devices[hid_device_num].ep_out = 0;
    hid_devices[hid_device_num].report_size = dev->ep[ep].max_packet;
    hid_devices[hid_device_num].protocol = 0;
    hid_devices[hid_device_num].type = 0;
    hid_devices[hid_device_num].kbd_cb = cb;
    hid_devices[hid_device_num].mouse_cb = 0;
    hid_device_num++;
    hid_set_protocol(dev, 0);
    return 0;
}

int hid_register_mouse(usb_device_t *dev, hid_mouse_callback cb) {
    int ep = hid_find_endpoint(dev, USB_ENDPOINT_INTERRUPT, 1);
    if (ep < 0) return -1;
    if (hid_device_num >= HID_MAX_DEVICES) return -2;
    hid_devices[hid_device_num].dev = dev;
    hid_devices[hid_device_num].ep_in = dev->ep[ep].address;
    hid_devices[hid_device_num].ep_out = 0;
    hid_devices[hid_device_num].report_size = dev->ep[ep].max_packet;
    hid_devices[hid_device_num].protocol = 0;
    hid_devices[hid_device_num].type = 1;
    hid_devices[hid_device_num].kbd_cb = 0;
    hid_devices[hid_device_num].mouse_cb = cb;
    hid_device_num++;
    hid_set_protocol(dev, 0);
    return 0;
}

static uint8_t hid_kbd_map[256];
static uint8_t kbd_map_init = 0;

static void init_kbd_map(void) {
    hid_kbd_map[0x00] = 0;
    hid_kbd_map[0x01] = 0;
    hid_kbd_map[0x02] = 0;
    hid_kbd_map[0x04] = 0x1D;
    hid_kbd_map[0x05] = 0x15;
    hid_kbd_map[0x06] = 0x1D;
    hid_kbd_map[0x07] = 0x24;
    hid_kbd_map[0x08] = 0x25;
    hid_kbd_map[0x09] = 0x26;
    hid_kbd_map[0x0A] = 0x27;
    hid_kbd_map[0x0B] = 0x2D;
    hid_kbd_map[0x0C] = 0x2E;
    hid_kbd_map[0x0D] = 0x2F;
    hid_kbd_map[0x0E] = 0x30;
    hid_kbd_map[0x0F] = 0x31;
    hid_kbd_map[0x10] = 0x32;
    hid_kbd_map[0x11] = 0x1C;
    hid_kbd_map[0x12] = 0x34;
    hid_kbd_map[0x13] = 0x35;
    hid_kbd_map[0x14] = 0x1A;
    hid_kbd_map[0x15] = 0x36;
    hid_kbd_map[0x16] = 0x37;
    hid_kbd_map[0x17] = 0x38;
    hid_kbd_map[0x18] = 0x39;
    hid_kbd_map[0x19] = 0x2B;
    hid_kbd_map[0x1A] = 0x0E;
    hid_kbd_map[0x1B] = 0x2C;
    hid_kbd_map[0x1C] = 0x2A;
    hid_kbd_map[0x1D] = 0x3B;
    hid_kbd_map[0x1E] = 0x0C;
    hid_kbd_map[0x1F] = 0x0D;
    hid_kbd_map[0x20] = 0x0B;
    hid_kbd_map[0x21] = 0x0A;
    hid_kbd_map[0x22] = 0x09;
    hid_kbd_map[0x23] = 0x08;
    hid_kbd_map[0x24] = 0x07;
    hid_kbd_map[0x25] = 0x06;
    hid_kbd_map[0x26] = 0x05;
    hid_kbd_map[0x27] = 0x04;
    hid_kbd_map[0x28] = 0x03;
    hid_kbd_map[0x29] = 0x02;
    hid_kbd_map[0x2A] = 0x21;
    hid_kbd_map[0x2B] = 0x22;
    hid_kbd_map[0x2C] = 0x23;
    hid_kbd_map[0x2D] = 0x17;
    hid_kbd_map[0x2E] = 0x24;
    hid_kbd_map[0x2F] = 0x25;
    hid_kbd_map[0x30] = 0x26;
    hid_kbd_map[0x31] = 0x27;
    hid_kbd_map[0x32] = 0x28;
    hid_kbd_map[0x33] = 0x29;
    hid_kbd_map[0x34] = 0x33;
    hid_kbd_map[0x35] = 0x19;
    hid_kbd_map[0x36] = 0x32;
    hid_kbd_map[0x37] = 0x31;
    hid_kbd_map[0x38] = 0x18;
    hid_kbd_map[0x39] = 0x10;
    hid_kbd_map[0x3A] = 0x11;
    hid_kbd_map[0x3B] = 0x12;
    hid_kbd_map[0x3C] = 0x13;
    hid_kbd_map[0x3D] = 0x14;
    hid_kbd_map[0x3E] = 0x15;
    hid_kbd_map[0x3F] = 0x16;
    hid_kbd_map[0x40] = 0x0F;
    hid_kbd_map[0x41] = 0x3A;
    hid_kbd_map[0x42] = 0x1B;
    hid_kbd_map[0x43] = 0x2B;
    hid_kbd_map[0x44] = 0x0C;
    hid_kbd_map[0x45] = 0x3D;
    hid_kbd_map[0x46] = 0x1E;
    hid_kbd_map[0x47] = 0x1F;
    hid_kbd_map[0x48] = 0x20;
    hid_kbd_map[0x49] = 0x12;
    hid_kbd_map[0x4A] = 0x21;
    hid_kbd_map[0x4B] = 0x22;
    hid_kbd_map[0x4C] = 0x23;
    hid_kbd_map[0x4D] = 0x24;
    hid_kbd_map[0x4E] = 0x25;
    hid_kbd_map[0x4F] = 0x26;
    hid_kbd_map[0x50] = 0x27;
    hid_kbd_map[0x51] = 0x28;
    hid_kbd_map[0x52] = 0x29;
    hid_kbd_map[0x53] = 0x33;
    hid_kbd_map[0x54] = 0x19;
    hid_kbd_map[0x55] = 0x32;
    hid_kbd_map[0x56] = 0x31;
    hid_kbd_map[0x57] = 0x18;
    hid_kbd_map[0x58] = 0x10;
    hid_kbd_map[0x59] = 0x11;
    hid_kbd_map[0x5A] = 0x12;
    hid_kbd_map[0x5B] = 0x13;
    hid_kbd_map[0x5C] = 0x14;
    hid_kbd_map[0x5D] = 0x15;
    hid_kbd_map[0x5E] = 0x16;
    hid_kbd_map[0x5F] = 0x17;
    hid_kbd_map[0x60] = 0x0F;
    hid_kbd_map[0x61] = 0x3A;
    hid_kbd_map[0x62] = 0x1B;
    hid_kbd_map[0x63] = 0x2B;
    hid_kbd_map[0x64] = 0x0C;
    hid_kbd_map[0x65] = 0x3D;
    kbd_map_init = 1;
}

void hid_init(void) {
    int i;
    for (i = 0; i < HID_MAX_DEVICES; i++) {
        hid_devices[i].dev = 0;
        hid_devices[i].ep_in = 0;
        hid_devices[i].ep_out = 0;
        hid_devices[i].report_size = 0;
        hid_devices[i].protocol = 0;
        hid_devices[i].type = -1;
        hid_devices[i].kbd_cb = 0;
        hid_devices[i].mouse_cb = 0;
    }
    hid_device_num = 0;
    init_kbd_map();
}

void hid_poll(void) {
    int i;
    for (i = 0; i < hid_device_num; i++) {
        hid_device_t *hdev = &hid_devices[i];
        if (!hdev->dev || !hdev->dev->configured) continue;
        if (hdev->ep_in == 0) continue;
        int ret = usb_interrupt_transfer(hdev->dev, hdev->ep_in, hdev->report_buf, hdev->report_size);
        if (ret != 0) continue;
        if (hdev->type == 0 && hdev->kbd_cb) {
            uint8_t modifier = hdev->report_buf[0];
            uint8_t j;
            for (j = 2; j < 8 && j < hdev->report_size; j++) {
                uint8_t keycode = hdev->report_buf[j];
                if (keycode == 0) continue;
                if (keycode >= 0x04 && keycode <= 0x65) {
                    uint8_t mapped = hid_kbd_map[keycode];
                    if (mapped != 0) {
                        uint8_t shift = (modifier & 0x22) != 0;
                        hdev->kbd_cb(mapped, 1);
                    }
                }
            }
        } else if (hdev->type == 1 && hdev->mouse_cb) {
            int8_t dx = (int8_t)hdev->report_buf[1];
            int8_t dy = (int8_t)hdev->report_buf[2];
            int8_t dz = 0;
            uint8_t buttons = hdev->report_buf[0];
            if (hdev->report_size >= 4) dz = (int8_t)hdev->report_buf[3];
            hdev->mouse_cb(dx, -dy, dz, buttons);
        }
    }
}
