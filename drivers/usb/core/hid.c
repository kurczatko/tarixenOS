#include <drivers/usb/core/hid.h>
#include <drivers/usb/core/usb.h>

#define HID_MAX_DEVICES        8
#define HID_KIND_CACHE_SIZE    16
#define HID_DESC_MAX           256
#define HID_DT_HID             0x21
#define HID_REQ_GET_REPORT     0x01
#define HID_REQ_GET_IDLE       0x02
#define HID_REQ_GET_PROTOCOL   0x03
#define HID_REQ_SET_REPORT     0x09
#define HID_REQ_SET_IDLE       0x0A
#define HID_REQ_SET_PROTOCOL   0x0B

#define HID_KIND_NONE          0
#define HID_KIND_KBD           1
#define HID_KIND_MOUSE         2

typedef struct {
    usb_device_t *dev;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t report_size;
    uint8_t report_buf[64];
    uint8_t prev_keys[6];
    uint8_t prev_mod;
    uint8_t protocol;
    int type;                 /* 0 = klawiatura, 1 = mysz */
    hid_keyboard_callback kbd_cb;
    hid_mouse_callback mouse_cb;
} hid_device_t;

typedef struct {
    usb_device_t *dev;
    uint8_t iface;
    uint8_t kind;
    uint8_t valid;
} hid_kind_entry_t;

static hid_device_t hid_devices[HID_MAX_DEVICES];
static int hid_device_num = 0;
static hid_kind_entry_t hid_kind_cache[HID_KIND_CACHE_SIZE];

static int hid_find_endpoint(usb_device_t *dev, uint8_t ep_type, uint8_t dir) {
    for (int i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (!dev->ep[i].used) continue;
        if (dev->ep[i].type != ep_type) continue;
        if ((dev->ep[i].address & 0x80) != (dir == 1 ? 0x80 : 0x00)) continue;
        return i;
    }
    return -1;
}

static int hid_already_registered(usb_device_t *dev, int type) {
    for (int i = 0; i < hid_device_num; i++) {
        if (hid_devices[i].dev == dev && hid_devices[i].type == type) return 1;
    }
    return 0;
}

int hid_get_report_descriptor(usb_device_t *dev, void *buf, uint16_t size) {
    uint8_t bmRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
    uint16_t wValue = ((uint16_t)USB_DT_REPORT << 8) | 0;
    return usb_control_transfer(dev, bmRequestType, USB_REQ_GET_DESCRIPTOR, wValue, dev->interface_num, size, buf);
}

int hid_set_protocol(usb_device_t *dev, uint8_t protocol) {
    uint8_t bmRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    return usb_control_transfer(dev, bmRequestType, HID_REQ_SET_PROTOCOL, protocol, dev->interface_num, 0, 0);
}

static int hid_set_idle(usb_device_t *dev, uint8_t duration, uint8_t report_id) {
    uint8_t bmRequestType = USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
    uint16_t wValue = ((uint16_t)duration << 8) | report_id;
    return usb_control_transfer(dev, bmRequestType, HID_REQ_SET_IDLE, wValue, dev->interface_num, 0, 0);
}

static uint16_t hid_read_u16(const uint8_t *p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t hid_report_desc_length(usb_device_t *dev) {
    uint8_t hdr[9];
    uint8_t cfg[256];
    uint16_t total;
    uint16_t pos = 0;
    int in_iface = 0;

    for (uint32_t i = 0; i < sizeof(hdr); i++) hdr[i] = 0;
    if (usb_control_transfer(dev, USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
                             USB_REQ_GET_DESCRIPTOR, (uint16_t)((uint16_t)USB_DT_CONFIG << 8),
                             0, sizeof(hdr), hdr) != 0)
        return 0;

    total = hid_read_u16(&hdr[2]);
    if (total < 9) return 0;
    if (total > sizeof(cfg)) total = sizeof(cfg);

    for (uint32_t i = 0; i < sizeof(cfg); i++) cfg[i] = 0;
    if (usb_control_transfer(dev, USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
                             USB_REQ_GET_DESCRIPTOR, (uint16_t)((uint16_t)USB_DT_CONFIG << 8),
                             0, total, cfg) != 0)
        return 0;

    while (pos + 2 <= total) {
        uint8_t len = cfg[pos];
        uint8_t type = cfg[pos + 1];
        if (len < 2 || pos + len > total) break;

        if (type == USB_DT_INTERFACE && len >= 4) {
            in_iface = (cfg[pos + 2] == dev->interface_num && cfg[pos + 3] == 0);
        } else if (type == HID_DT_HID && in_iface && len >= 9) {
            return hid_read_u16(&cfg[pos + 7]);
        }
        pos = (uint16_t)(pos + len);
    }
    return 0;
}

static int hid_fetch_report_desc(usb_device_t *dev, uint8_t *buf, uint16_t len) {
    uint8_t bmRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
    uint16_t wValue = ((uint16_t)USB_DT_REPORT << 8);
    return usb_control_transfer(dev, bmRequestType, USB_REQ_GET_DESCRIPTOR, wValue,
                                dev->interface_num, len, buf);
}
static uint8_t hid_parse_top_collection(const uint8_t *d, uint16_t n) {
    uint32_t page = 0;
    uint32_t usage = 0;
    uint8_t have_usage = 0;
    uint16_t i = 0;

    while (i < n) {
        uint8_t prefix = d[i];
        uint8_t size;
        uint32_t val = 0;
        uint8_t item;

        if (prefix == 0xFE) {                      /* dlugi element */
            if ((uint16_t)(i + 2) >= n) break;
            i = (uint16_t)(i + 3 + d[i + 1]);
            continue;
        }

        size = prefix & 0x03U;
        if (size == 3) size = 4;
        if ((uint32_t)i + 1U + size > n) break;

        for (uint8_t k = 0; k < size; k++) val |= (uint32_t)d[i + 1 + k] << (8 * k);

        item = (uint8_t)(prefix & 0xFCU);
        if (item == 0x04) {                        /* Usage Page */
            page = val;
        } else if (item == 0x08) {                 /* Usage (lokalny) */
            if (!have_usage) { usage = val; have_usage = 1; }
        } else if (item == 0xA0) {                 /* Collection */
            if (page == 0x01U && have_usage) {
                if (usage == 0x06U) return HID_KIND_KBD;
                if (usage == 0x02U) return HID_KIND_MOUSE;
            }
            return HID_KIND_NONE;
        } else if (item == 0x80 || item == 0x90 || item == 0xB0 || item == 0xC0) {
            have_usage = 0;                        /* Input/Output/Feature/End Collection */
        }

        i = (uint16_t)(i + 1 + size);
    }
    return HID_KIND_NONE;
}

static uint8_t hid_kind_of(usb_device_t *dev) {
    uint8_t iface = dev->interface_num;
    uint8_t desc[HID_DESC_MAX];
    uint16_t len;
    uint8_t kind = HID_KIND_NONE;

    for (int i = 0; i < HID_KIND_CACHE_SIZE; i++) {
        if (hid_kind_cache[i].valid && hid_kind_cache[i].dev == dev &&
            hid_kind_cache[i].iface == iface)
            return hid_kind_cache[i].kind;
    }

    len = hid_report_desc_length(dev);
    if (len == 0) return HID_KIND_NONE;           /* sprobujemy ponownie pozniej */
    if (len > HID_DESC_MAX) len = HID_DESC_MAX;

    for (uint32_t i = 0; i < sizeof(desc); i++) desc[i] = 0;
    if (hid_fetch_report_desc(dev, desc, len) != 0) return HID_KIND_NONE;
    kind = hid_parse_top_collection(desc, len);

    for (int i = 0; i < HID_KIND_CACHE_SIZE; i++) {
        if (!hid_kind_cache[i].valid) {
            hid_kind_cache[i].valid = 1;
            hid_kind_cache[i].dev = dev;
            hid_kind_cache[i].iface = iface;
            hid_kind_cache[i].kind = kind;
            break;
        }
    }
    return kind;
}

int hid_register_keyboard(usb_device_t *dev, hid_keyboard_callback cb) {
    if (!dev || hid_already_registered(dev, 0)) return -1;
    if (hid_kind_of(dev) != HID_KIND_KBD) return -1;
    int ep = hid_find_endpoint(dev, USB_ENDPOINT_INTERRUPT, 1);
    if (ep < 0) return -1;
    if (hid_device_num >= HID_MAX_DEVICES) return -2;

    uint16_t rsize = dev->ep[ep].max_packet;
    if (rsize == 0 || rsize > 64) rsize = 8;

    hid_device_t *h = &hid_devices[hid_device_num++];
    h->dev = dev;
    h->ep_in = dev->ep[ep].address;
    h->ep_out = 0;
    h->report_size = (uint8_t)rsize;
    h->protocol = 0;
    h->type = 0;
    h->kbd_cb = cb;
    h->mouse_cb = 0;
    h->prev_mod = 0;
    for (int k = 0; k < 6; k++) h->prev_keys[k] = 0;

    hid_set_protocol(dev, 0);                     /* tryb boot: 8-bajtowy raport */
    hid_set_idle(dev, 0, 0);
    return 0;
}

int hid_register_mouse(usb_device_t *dev, hid_mouse_callback cb) {
    if (!dev || hid_already_registered(dev, 1)) return -1;
    if (hid_kind_of(dev) != HID_KIND_MOUSE) return -1;
    int ep = hid_find_endpoint(dev, USB_ENDPOINT_INTERRUPT, 1);
    if (ep < 0) return -1;
    if (hid_device_num >= HID_MAX_DEVICES) return -2;

    uint16_t rsize = dev->ep[ep].max_packet;
    if (rsize < 3) rsize = 3;
    if (rsize > 8) rsize = 8;

    hid_device_t *h = &hid_devices[hid_device_num++];
    h->dev = dev;
    h->ep_in = dev->ep[ep].address;
    h->ep_out = 0;
    h->report_size = (uint8_t)rsize;
    h->protocol = 1;
    h->type = 1;
    h->kbd_cb = 0;
    h->mouse_cb = cb;
    h->prev_mod = 0;
    for (int k = 0; k < 6; k++) h->prev_keys[k] = 0;

    hid_set_protocol(dev, 0);                     /* tryb boot: 3-bajtowy raport myszy */
    hid_set_idle(dev, 0, 0);
    return 0;
}

void hid_init(void) {
    for (int i = 0; i < HID_MAX_DEVICES; i++) {
        hid_devices[i].dev = 0;
        hid_devices[i].ep_in = 0;
        hid_devices[i].ep_out = 0;
        hid_devices[i].report_size = 0;
        hid_devices[i].protocol = 0;
        hid_devices[i].type = -1;
        hid_devices[i].kbd_cb = 0;
        hid_devices[i].mouse_cb = 0;
        hid_devices[i].prev_mod = 0;
        for (int k = 0; k < 6; k++) hid_devices[i].prev_keys[k] = 0;
    }
    for (int i = 0; i < HID_KIND_CACHE_SIZE; i++) hid_kind_cache[i].valid = 0;
    hid_device_num = 0;
}

static void hid_process_kbd_report(hid_device_t *hdev) {
    uint8_t mod = hdev->report_buf[0];
    uint8_t prev_mod = hdev->prev_mod;

    uint8_t shift_now = (mod & 0x22U) != 0;
    uint8_t shift_was = (prev_mod & 0x22U) != 0;
    if (shift_now != shift_was) hdev->kbd_cb(shift_now ? 0xE0 : 0xE2, shift_now);

    uint8_t ctrl_now = (mod & 0x11U) != 0;
    uint8_t ctrl_was = (prev_mod & 0x11U) != 0;
    if (ctrl_now != ctrl_was) hdev->kbd_cb(ctrl_now ? 0xE4 : 0xE5, ctrl_now);

    uint8_t alt_now = (mod & 0x44U) != 0;
    uint8_t alt_was = (prev_mod & 0x44U) != 0;
    if (alt_now != alt_was) hdev->kbd_cb(alt_now ? 0xE9 : 0xEA, alt_now);

    uint8_t gui_now = (mod & 0x88U) != 0;
    uint8_t gui_was = (prev_mod & 0x88U) != 0;
    if (gui_now != gui_was) hdev->kbd_cb(gui_now ? 0xE6 : 0xE7, gui_now);

    hdev->prev_mod = mod;

    for (int j = 2; j < 8 && j < hdev->report_size; j++) {
        uint8_t keycode = hdev->report_buf[j];
        if (keycode <= 0x03) continue;
        int already = 0;
        for (int k = 0; k < 6; k++) {
            if (hdev->prev_keys[k] == keycode) { already = 1; break; }
        }
        if (!already) {
            if (keycode == 0x39) hdev->kbd_cb(0xE8, 1);
            else                 hdev->kbd_cb(keycode, 1);
        }
    }

    for (int k = 0; k < 6; k++) {
        hdev->prev_keys[k] = (2 + k < hdev->report_size) ? hdev->report_buf[2 + k] : 0;
    }
}

void hid_poll(void) {
    for (int i = 0; i < hid_device_num; i++) {
        hid_device_t *hdev = &hid_devices[i];
        if (!hdev->dev || !hdev->dev->used || !hdev->dev->configured) continue;
        if (hdev->ep_in == 0) continue;

        int ret = usb_interrupt_transfer(hdev->dev, hdev->ep_in, hdev->report_buf, hdev->report_size);
        if (ret != 0) continue;

        if (hdev->type == 0 && hdev->kbd_cb) {
            hid_process_kbd_report(hdev);
        } else if (hdev->type == 1 && hdev->mouse_cb) {
            uint8_t buttons = hdev->report_buf[0];
            int8_t dx = (int8_t)hdev->report_buf[1];
            int8_t dy = (int8_t)hdev->report_buf[2];
            int8_t dz = 0;                        /* tryb boot nie ma kolka */
            hdev->mouse_cb(dx, dy, dz, buttons);
        }
    }
}