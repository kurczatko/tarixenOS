#include "../../../include/printf/types.h"
#include "usb.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define USB_REG_CMD        0x00
#define USB_REG_STS        0x02
#define USB_REG_INTR       0x04
#define USB_REG_FRNUM      0x06
#define USB_REG_FLBASEADD  0x08
#define USB_REG_SOFMOD     0x0C
#define USB_REG_PORTSC1    0x10

#define USB_CMD_RS         (1 << 0)
#define USB_CMD_HCRESET    (1 << 1)
#define USB_CMD_GRE        (1 << 3)
#define USB_CMD_FGR        (1 << 6)

#define USB_STS_HCH        (1 << 4)
#define USB_STS_PCD        (1 << 5)

#define USB_PSC_CCS        (1 << 0)
#define USB_PSC_PED        (1 << 1)
#define USB_PSC_PR         (1 << 6)
#define USB_PSC_LS         (1 << 9)

#define TD_ACTIVE          (1 << 15)
#define TD_STALLED         (1 << 13)
#define TD_BUFFERR         (1 << 12)
#define TD_BABBLE          (1 << 11)
#define TD_DATATOGERR      (1 << 10)
#define TD_CRCERR          (1 << 9)
#define TD_ISO             (1 << 8)
#define TD_IOC             (1 << 7)
#define TD_DATATOGGLE      (1 << 5)
#define TD_PID_SHIFT       0
#define TD_PID_MASK        0x0F
#define TD_PID_OUT         0x01
#define TD_PID_IN          0x09
#define TD_PID_SETUP       0x0D

#define QH_HEAD_TYPE_QH    0x01
#define QH_MAXLEN_SHIFT    0
#define QH_CONTROL         (1 << 15)
#define QH_LOWSPEED        (1 << 9)

#define FRAME_ENTRY_QH     0x01

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B
#define USB_REQ_SYNCH_FRAME       0x0C

#define MAX_TDS 32
#define MAX_QHS 16
#define MAX_DEVICES 8

static uint32_t frame_list[1024] __attribute__((aligned(4096)));

typedef struct {
    uint16_t next_link;
    uint16_t token;
    uint16_t buffer;
    uint16_t buffer_hi;
} __attribute__((packed, aligned(16))) uhci_td_t;

typedef struct {
    uint16_t head_link;
    uint16_t element_link;
    uint16_t max_len;
    uint16_t pad;
    uhci_td_t td_overlay __attribute__((aligned(16)));
} __attribute__((packed, aligned(32))) uhci_qh_t;

static uhci_td_t td_pool[MAX_TDS] __attribute__((aligned(16)));
static uint8_t td_used[MAX_TDS];
static uhci_qh_t qh_pool[MAX_QHS] __attribute__((aligned(32)));
static uint8_t qh_used[MAX_QHS];

static usb_device_t usb_devices[MAX_DEVICES];
static int usb_device_num = 0;
static uint16_t uhci_io_base = 0;

static int td_alloc(void) {
    int i;
    for (i = 0; i < MAX_TDS; i++) {
        if (!td_used[i]) {
            td_used[i] = 1;
            return i;
        }
    }
    return -1;
}

static void td_free(int idx) {
    if (idx >= 0 && idx < MAX_TDS) td_used[idx] = 0;
}

static int qh_alloc(void) {
    int i;
    for (i = 0; i < MAX_QHS; i++) {
        if (!qh_used[i]) {
            qh_used[i] = 1;
            return i;
        }
    }
    return -1;
}

static void qh_free(int idx) {
    if (idx >= 0 && idx < MAX_QHS) qh_used[idx] = 0;
}

static uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
    uint32_t addr = (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (reg & 0xFC);
    usb_outl(PCI_CONFIG_ADDRESS, addr);
    return (uint16_t)(usb_inl(PCI_CONFIG_DATA) >> ((reg & 2) * 8));
}

static uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
    uint32_t addr = (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (reg & 0xFC);
    usb_outl(PCI_CONFIG_ADDRESS, addr);
    return usb_inl(PCI_CONFIG_DATA);
}

static void pci_write_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint16_t val) {
    uint32_t addr = (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (reg & 0xFC);
    usb_outl(PCI_CONFIG_ADDRESS, addr);
    uint32_t data = usb_inl(PCI_CONFIG_DATA);
    if (reg & 2) data = (data & 0x0000FFFF) | ((uint32_t)val << 16);
    else data = (data & 0xFFFF0000) | val;
    usb_outl(PCI_CONFIG_DATA, data);
}

static int pci_scan(void) {
    uint8_t bus, slot, func;
    for (bus = 0; bus < 8; bus++) {
        for (slot = 0; slot < 32; slot++) {
            for (func = 0; func < 8; func++) {
                uint16_t vendor = pci_read_config_word(bus, slot, func, 0);
                if (vendor == 0xFFFF) continue;
                uint32_t classreg = pci_read_config_dword(bus, slot, func, 8);
                uint8_t class_code = (uint8_t)(classreg >> 24);
                uint8_t subclass = (uint8_t)(classreg >> 16);
                if (class_code == 0x0C && subclass == 0x03) {
                    uint16_t command = pci_read_config_word(bus, slot, func, 4);
                    command |= 0x1;
                    pci_write_config_word(bus, slot, func, 4, command);
                    uint32_t bar = pci_read_config_dword(bus, slot, func, 0x20) & 0xFFFFFFF0;
                    if (bar == 0) continue;
                    uhci_io_base = (uint16_t)bar;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static void uhci_reset(void) {
    usb_outw(uhci_io_base + USB_REG_CMD, usb_inw(uhci_io_base + USB_REG_CMD) | USB_CMD_HCRESET);
    volatile int i;
    for (i = 0; i < 1000; i++) {
        if (!(usb_inw(uhci_io_base + USB_REG_CMD) & USB_CMD_HCRESET)) break;
    }
    usb_outw(uhci_io_base + USB_REG_INTR, 0x3F);
    for (i = 0; i < 1024; i++) frame_list[i] = 0;
    usb_outl(uhci_io_base + USB_REG_FLBASEADD, (uint32_t)frame_list);
    usb_outw(uhci_io_base + USB_REG_CMD, usb_inw(uhci_io_base + USB_REG_CMD) | USB_CMD_RS);
}

static int uhci_port_connected(uint8_t port) {
    uint16_t reg = usb_inw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2);
    return (reg & USB_PSC_CCS) != 0;
}

static void uhci_port_reset(uint8_t port) {
    uint16_t reg = usb_inw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2);
    usb_outw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2, reg | USB_PSC_PR);
    volatile int i;
    for (i = 0; i < 1000; i++) {
        if (!(usb_inw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2) & USB_PSC_PR)) break;
    }
    usb_outw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2, usb_inw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2) & ~USB_PSC_PR);
}

static int uhci_low_speed(uint8_t port) {
    return (usb_inw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2) & USB_PSC_LS) != 0;
}

static int uhci_port_enabled(uint8_t port) {
    return (usb_inw(uhci_io_base + USB_REG_PORTSC1 + (port - 1) * 2) & USB_PSC_PED) != 0;
}

static void uhci_delay(void) {
    volatile int i;
    for (i = 0; i < 1000; i++) {}
}

static usb_device_t *usb_alloc_device(void) {
    int i;
    for (i = 0; i < MAX_DEVICES; i++) {
        if (!usb_devices[i].used) {
            usb_devices[i].used = 1;
            usb_devices[i].io_base = uhci_io_base;
            usb_devices[i].address = 0;
            usb_devices[i].max_packet_size0 = 8;
            usb_device_num++;
            return &usb_devices[i];
        }
    }
    return 0;
}

static void usb_free_device(usb_device_t *dev) {
    dev->used = 0;
    dev->address = 0;
    dev->configured = 0;
    usb_device_num--;
}

static int td_wait_complete(uhci_td_t *td) {
    volatile int i;
    for (i = 0; i < 2000; i++) {
        if (!(td->token & TD_ACTIVE)) break;
    }
    if (td->token & TD_ACTIVE) return -1;
    if (td->token & (TD_STALLED | TD_BUFFERR | TD_BABBLE | TD_DATATOGERR | TD_CRCERR)) return -2;
    return 0;
}

static int usb_ctrl_transfer(usb_device_t *dev, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, void *data) {
    int td_idx_setup, td_idx_data, td_idx_status;
    uhci_td_t *setup_td, *data_td, *status_td;
    uhci_qh_t *qh;
    int qh_idx;
    uint8_t setup_buf[8];
    uint16_t toggle = 0;
    (void)toggle;
    int direction = (bmRequestType & USB_DIR_IN) ? USB_DIR_IN : USB_DIR_OUT;
    uint16_t datalen = wLength;
    uint8_t pid_data = (direction == USB_DIR_IN) ? TD_PID_IN : TD_PID_OUT;
    uint8_t pid_status = (direction == USB_DIR_IN) ? TD_PID_OUT : TD_PID_IN;
    int ret = 0;
    int i;

    td_idx_setup = td_alloc();
    if (td_idx_setup < 0) return -3;
    setup_td = &td_pool[td_idx_setup];

    setup_buf[0] = bmRequestType;
    setup_buf[1] = bRequest;
    setup_buf[2] = (uint8_t)(wValue & 0xFF);
    setup_buf[3] = (uint8_t)(wValue >> 8);
    setup_buf[4] = (uint8_t)(wIndex & 0xFF);
    setup_buf[5] = (uint8_t)(wIndex >> 8);
    setup_buf[6] = (uint8_t)(wLength & 0xFF);
    setup_buf[7] = (uint8_t)(wLength >> 8);

    setup_td->next_link = 0xFFFF;
    setup_td->token = TD_ACTIVE | TD_IOC | (TD_PID_SETUP << TD_PID_SHIFT) | 0;
    setup_td->buffer = (uint16_t)((uint32_t)setup_buf & 0xFFFF);
    setup_td->buffer_hi = (uint16_t)(((uint32_t)setup_buf >> 16) & 0xFFFF);

    if (wLength > 0) {
        td_idx_data = td_alloc();
        if (td_idx_data < 0) { td_free(td_idx_setup); return -3; }
        data_td = &td_pool[td_idx_data];
        data_td->next_link = 0xFFFF;
        data_td->token = TD_ACTIVE | TD_IOC | TD_DATATOGGLE | (pid_data << TD_PID_SHIFT) | (datalen << 2);
        data_td->buffer = (uint16_t)((uint32_t)data & 0xFFFF);
        data_td->buffer_hi = (uint16_t)(((uint32_t)data >> 16) & 0xFFFF);
        toggle = 1;
    } else {
        td_idx_data = -1;
        data_td = 0;
    }

    td_idx_status = td_alloc();
    if (td_idx_status < 0) { td_free(td_idx_setup); if (td_idx_data >= 0) td_free(td_idx_data); return -3; }
    status_td = &td_pool[td_idx_status];
    status_td->next_link = 0xFFFF;
    status_td->token = TD_ACTIVE | TD_IOC | TD_DATATOGGLE | (pid_status << TD_PID_SHIFT) | 0;
    status_td->buffer = 0;
    status_td->buffer_hi = 0;

    qh_idx = qh_alloc();
    if (qh_idx < 0) { td_free(td_idx_setup); if (td_idx_data >= 0) td_free(td_idx_data); td_free(td_idx_status); return -3; }
    qh = &qh_pool[qh_idx];
    qh->head_link = 0xFFFF;
    qh->element_link = (uint16_t)((uint32_t)setup_td & 0xFFFFFFFE) | QH_HEAD_TYPE_QH;
    qh->max_len = dev->max_packet_size0 | QH_CONTROL;
    qh->pad = 0;
    qh->td_overlay.next_link = 0xFFFF;
    qh->td_overlay.token = 0;
    qh->td_overlay.buffer = 0;
    qh->td_overlay.buffer_hi = 0;

    frame_list[0] = ((uint32_t)qh & 0xFFFFFFE0) | FRAME_ENTRY_QH;
    for (i = 1; i < 1024; i++) frame_list[i] = 0xFFFFFFFF | FRAME_ENTRY_QH;

    ret = td_wait_complete(setup_td);
    if (ret != 0) goto cleanup;

    if (wLength > 0) {
        ret = td_wait_complete(data_td);
        if (ret != 0) goto cleanup;
    }

    ret = td_wait_complete(status_td);
    if (ret != 0) goto cleanup;

cleanup:
    td_free(td_idx_setup);
    if (td_idx_data >= 0) td_free(td_idx_data);
    td_free(td_idx_status);
    qh_free(qh_idx);
    frame_list[0] = 0;
    return ret;
}

static int usb_get_descriptor(usb_device_t *dev, uint8_t type, uint8_t index, uint16_t length, void *data) {
    uint8_t bmRequestType = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    uint16_t wValue = ((uint16_t)type << 8) | index;
    return usb_ctrl_transfer(dev, bmRequestType, USB_REQ_GET_DESCRIPTOR, wValue, 0, length, data);
}

static int usb_set_address(usb_device_t *dev, uint16_t address) {
    uint8_t bmRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    int ret = usb_ctrl_transfer(dev, bmRequestType, USB_REQ_SET_ADDRESS, address, 0, 0, 0);
    if (ret == 0) dev->address = (uint8_t)address;
    return ret;
}

static int usb_set_configuration(usb_device_t *dev, uint8_t configuration) {
    uint8_t bmRequestType = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    return usb_ctrl_transfer(dev, bmRequestType, USB_REQ_SET_CONFIGURATION, configuration, 0, 0, 0);
}

static usb_device_t *usb_enumerate_port(uint8_t port) {
    usb_device_t *dev = usb_alloc_device();
    if (!dev) return 0;
    dev->port = port;
    uhci_port_reset(port);
    uhci_delay();
    dev->speed = uhci_low_speed(port);
    dev->address = 0;

    uint8_t devdesc_buf[18];
    int ret = usb_get_descriptor(dev, USB_DT_DEVICE, 0, 8, devdesc_buf);
    if (ret != 0) { usb_free_device(dev); return 0; }

    usb_device_descriptor_t *dd = (usb_device_descriptor_t *)devdesc_buf;
    dev->max_packet_size0 = dd->bMaxPacketSize0;

    ret = usb_set_address(dev, usb_device_num);
    if (ret != 0) { usb_free_device(dev); return 0; }
    uhci_delay();

    ret = usb_get_descriptor(dev, USB_DT_DEVICE, 0, 18, devdesc_buf);
    if (ret != 0) { usb_free_device(dev); return 0; }
    dd = (usb_device_descriptor_t *)devdesc_buf;
    dev->id_vendor = dd->idVendor;
    dev->id_product = dd->idProduct;
    dev->class = dd->bDeviceClass;
    dev->subclass = dd->bDeviceSubClass;
    dev->protocol = dd->bDeviceProtocol;

    uint8_t config_buf[256];
    ret = usb_get_descriptor(dev, USB_DT_CONFIG, 0, 256, config_buf);
    if (ret != 0) { usb_free_device(dev); return 0; }

    usb_config_descriptor_t *cd = (usb_config_descriptor_t *)config_buf;
    uint16_t total = cd->wTotalLength;
    if (total > 256) total = 256;

    uint8_t *ptr = config_buf;
    uint8_t *end = config_buf + total;
    uint8_t iface_num = 0;
    (void)iface_num;
    uint8_t ep_idx = 0;

    while (ptr < end) {
        uint8_t bLength = ptr[0];
        uint8_t bType = ptr[1];
        if (bLength == 0) break;
        if (bType == USB_DT_INTERFACE) {
            usb_interface_descriptor_t *id = (usb_interface_descriptor_t *)ptr;
            dev->class = id->bInterfaceClass;
            dev->subclass = id->bInterfaceSubClass;
            dev->protocol = id->bInterfaceProtocol;
            iface_num = id->bInterfaceNumber;
            ep_idx = 0;
        } else if (bType == USB_DT_ENDPOINT) {
            usb_endpoint_descriptor_t *ed = (usb_endpoint_descriptor_t *)ptr;
            if (ep_idx < USB_MAX_ENDPOINTS) {
                dev->ep[ep_idx].used = 1;
                dev->ep[ep_idx].address = ed->bEndpointAddress;
                dev->ep[ep_idx].type = ed->bmAttributes & 0x03;
                dev->ep[ep_idx].max_packet = ed->wMaxPacketSize & 0x3FF;
                dev->ep[ep_idx].toggle = 0;
                ep_idx++;
            }
        }
        ptr += bLength;
    }

    ret = usb_set_configuration(dev, cd->bConfigurationValue);
    if (ret != 0) { usb_free_device(dev); return 0; }
    uhci_delay();

    dev->configured = 1;
    return dev;
}

void usb_init(void) {
    int i;
    for (i = 0; i < MAX_TDS; i++) td_used[i] = 0;
    for (i = 0; i < MAX_QHS; i++) qh_used[i] = 0;
    for (i = 0; i < MAX_DEVICES; i++) usb_devices[i].used = 0;
    usb_device_num = 0;

    if (!pci_scan()) {
        return;
    }
    uhci_reset();
    uint8_t port;
    for (port = 1; port <= 2; port++) {
        if (uhci_port_connected(port)) {
            uhci_delay();
            uhci_port_reset(port);
            uhci_delay();
            if (uhci_port_enabled(port)) {
                usb_enumerate_port(port);
            }
        }
    }
    uhci_delay();
}

void usb_poll(void) {
}

usb_device_t *usb_device_get(int index) {
    if (index < 0 || index >= MAX_DEVICES) return 0;
    if (!usb_devices[index].used) return 0;
    return &usb_devices[index];
}

int usb_device_count(void) {
    return usb_device_num;
}

int usb_control_transfer(usb_device_t *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data) {
    return usb_ctrl_transfer(dev, request_type, request, value, index, length, data);
}

int usb_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction) {
    int ep_idx = -1, i;
    for (i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (dev->ep[i].used && dev->ep[i].address == ep_addr) {
            ep_idx = i;
            break;
        }
    }
    if (ep_idx < 0) return -1;

    int td_idx = td_alloc();
    if (td_idx < 0) return -3;
    uhci_td_t *td = &td_pool[td_idx];
    uint8_t pid = (direction == USB_DIR_IN) ? TD_PID_IN : TD_PID_OUT;
    uint8_t toggle = dev->ep[ep_idx].toggle ? TD_DATATOGGLE : 0;

    td->next_link = 0xFFFF;
    td->token = TD_ACTIVE | TD_IOC | toggle | (pid << TD_PID_SHIFT) | (length << 2);
    td->buffer = (uint16_t)((uint32_t)data & 0xFFFF);
    td->buffer_hi = (uint16_t)(((uint32_t)data >> 16) & 0xFFFF);

    uhci_qh_t *qh = &qh_pool[0];
    qh->element_link = (uint16_t)((uint32_t)td & 0xFFFFFFFE) | QH_HEAD_TYPE_QH;
    qh->max_len = dev->ep[ep_idx].max_packet;

    frame_list[0] = ((uint32_t)qh & 0xFFFFFFE0) | FRAME_ENTRY_QH;

    int ret = td_wait_complete(td);
    if (ret == 0) dev->ep[ep_idx].toggle ^= TD_DATATOGGLE;

    td_free(td_idx);
    frame_list[0] = 0;
    return ret;
}

int usb_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length) {
    int ep_idx = -1, i;
    for (i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (dev->ep[i].used && dev->ep[i].address == ep_addr) {
            ep_idx = i;
            break;
        }
    }
    if (ep_idx < 0) return -1;

    int td_idx = td_alloc();
    if (td_idx < 0) return -3;
    uhci_td_t *td = &td_pool[td_idx];
    uint8_t pid = TD_PID_IN;
    uint8_t toggle = dev->ep[ep_idx].toggle ? TD_DATATOGGLE : 0;

    td->next_link = 0xFFFF;
    td->token = TD_ACTIVE | TD_IOC | toggle | (pid << TD_PID_SHIFT) | (length << 2);
    td->buffer = (uint16_t)((uint32_t)data & 0xFFFF);
    td->buffer_hi = (uint16_t)(((uint32_t)data >> 16) & 0xFFFF);

    uhci_qh_t *qh = &qh_pool[0];
    qh->element_link = (uint16_t)((uint32_t)td & 0xFFFFFFFE) | QH_HEAD_TYPE_QH;
    qh->max_len = dev->ep[ep_idx].max_packet;

    frame_list[0] = ((uint32_t)qh & 0xFFFFFFE0) | FRAME_ENTRY_QH;

    int ret = td_wait_complete(td);
    if (ret == 0) dev->ep[ep_idx].toggle ^= TD_DATATOGGLE;

    td_free(td_idx);
    frame_list[0] = 0;
    return ret;
}
