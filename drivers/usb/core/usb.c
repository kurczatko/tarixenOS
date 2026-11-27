#include <include/printf/types.h>
#include <drivers/usb/core/usb.h>
#include <drivers/usb/core/xhci.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define UHCI_REG_CMD       0x00
#define UHCI_REG_STS       0x02
#define UHCI_REG_INTR      0x04
#define UHCI_REG_FRNUM     0x06
#define UHCI_REG_FLBASEADD 0x08
#define UHCI_REG_SOFMOD    0x0C
#define UHCI_REG_PORTSC1   0x10

#define UHCI_CMD_RS        (1U << 0)
#define UHCI_CMD_HCRESET   (1U << 1)
#define UHCI_CMD_GRESET    (1U << 2)
#define UHCI_CMD_CF        (1U << 6)
#define UHCI_CMD_MAXP      (1U << 7)

#define UHCI_PSC_CCS       (1U << 0)
#define UHCI_PSC_CSC       (1U << 1)
#define UHCI_PSC_PED       (1U << 2)
#define UHCI_PSC_PEDC      (1U << 3)
#define UHCI_PSC_LSDA      (1U << 8)
#define UHCI_PSC_PR        (1U << 9)

#define UHCI_LINK_TERM     0x01U
#define UHCI_LINK_QH       0x02U
#define UHCI_LINK_VF       0x04U

#define TD_CTRL_SPD        (1U << 29)
#define TD_CTRL_CERR3      (3U << 27)
#define TD_CTRL_LS         (1U << 26)
#define TD_CTRL_IOC        (1U << 24)
#define TD_CTRL_ACTIVE     (1U << 23)
#define TD_CTRL_STALLED    (1U << 22)
#define TD_CTRL_BUFFERR    (1U << 21)
#define TD_CTRL_BABBLE     (1U << 20)
#define TD_CTRL_NAK        (1U << 19)
#define TD_CTRL_CRCERR     (1U << 18)
#define TD_CTRL_BITSTUFF   (1U << 17)
#define TD_CTRL_ERR_MASK   (TD_CTRL_STALLED | TD_CTRL_BUFFERR | TD_CTRL_BABBLE | TD_CTRL_CRCERR | TD_CTRL_BITSTUFF)

#define TD_PID_OUT         0xE1U
#define TD_PID_IN          0x69U
#define TD_PID_SETUP       0x2DU

#define TD_TOKEN_TOGGLE    (1U << 19)
#define TD_TOKEN_LEN(n)    ((((uint32_t)((n) == 0 ? 0x7FFU : ((uint32_t)(n) - 1U))) & 0x7FFU) << 21)
#define TD_TOKEN_EP(ep)    (((uint32_t)((ep) & 0x0FU)) << 15)
#define TD_TOKEN_ADDR(a)   (((uint32_t)((a) & 0x7FU)) << 8)

#define MAX_UHCI_CTRLS     4
#define MAX_TDS            64
#define MAX_INTR_SLOTS     8

typedef struct {
    volatile uint32_t link_ptr;
    volatile uint32_t ctrl_status;
    volatile uint32_t token;
    volatile uint32_t buffer;
} __attribute__((packed, aligned(16))) uhci_td_t;

typedef struct {
    volatile uint32_t head_link;
    volatile uint32_t element_link;
    uint32_t pad[2];
} __attribute__((packed, aligned(16))) uhci_qh_t;

typedef struct {
    uint8_t  present;
    uint16_t io_base;
    uint8_t  port_enumerated[2];
} uhci_ctrl_t;

typedef struct {
    uint8_t     used;
    usb_device_t *dev;
    uint8_t     ep_addr;
    uint8_t     ep_idx;
    uint8_t     length;
    uhci_qh_t   qh __attribute__((aligned(16)));
    uhci_td_t   td __attribute__((aligned(16)));
    uint8_t     buf[64] __attribute__((aligned(16)));
} uhci_intr_slot_t;

static uhci_ctrl_t uhci_ctrls[MAX_UHCI_CTRLS];
static int uhci_ctrl_count = 0;

static uint32_t uhci_frame_lists[MAX_UHCI_CTRLS][1024] __attribute__((aligned(4096)));
static uhci_qh_t uhci_ctrl_qhs[MAX_UHCI_CTRLS] __attribute__((aligned(16)));
static uhci_td_t td_pool[MAX_TDS] __attribute__((aligned(16)));
static uhci_intr_slot_t intr_slots[MAX_INTR_SLOTS] __attribute__((aligned(16)));

static uint8_t usb_setup_pkt[8] __attribute__((aligned(16)));
static usb_device_t usb_devices[USB_MAX_DEVICES];
static uint8_t usb_next_addr = 1;

void usb_delay_ms(uint32_t ms) {
    for (uint32_t m = 0; m < ms; m++) {
        for (uint32_t i = 0; i < 1000; i++) {
            (void)usb_inb(0x80);
        }
    }
}

uint32_t usb_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
    uint32_t addr = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                    ((uint32_t)func << 8) | (reg & 0xFCU);
    usb_outl(PCI_CONFIG_ADDRESS, addr);
    return usb_inl(PCI_CONFIG_DATA);
}

uint16_t usb_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg) {
    uint32_t val = usb_pci_read32(bus, slot, func, reg);
    return (uint16_t)(val >> ((reg & 2U) * 8U));
}

void usb_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg, uint16_t val) {
    uint32_t addr = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                    ((uint32_t)func << 8) | (reg & 0xFCU);
    usb_outl(PCI_CONFIG_ADDRESS, addr);
    uint32_t data = usb_inl(PCI_CONFIG_DATA);
    if (reg & 2U) data = (data & 0x0000FFFFU) | ((uint32_t)val << 16);
    else          data = (data & 0xFFFF0000U) | (uint32_t)val;
    usb_outl(PCI_CONFIG_DATA, data);
}

static void ehci_handoff_to_companions(uint32_t bar0) {
    uint32_t mmio = bar0 & 0xFFFFFFF0U;
    if (!mmio) return;
    uint32_t cap = *(volatile uint32_t *)(mmio + 0x00);
    uint8_t caplen = (uint8_t)(cap & 0xFFU);
    uint32_t op = mmio + caplen;

    *(volatile uint32_t *)(op + 0x00) &= ~1U;
    usb_delay_ms(5);
    *(volatile uint32_t *)(op + 0x00) |= (1U << 1);
    for (int i = 0; i < 50; i++) {
        if (!(*(volatile uint32_t *)(op + 0x00) & (1U << 1))) break;
        usb_delay_ms(1);
    }
    *(volatile uint32_t *)(op + 0x40) = 0U;
}

static void pci_scan_controllers(void) {
    for (uint16_t bus = 0; bus < 32; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor0 = usb_pci_read16((uint8_t)bus, slot, 0, 0x00);
            if (vendor0 == 0xFFFF) continue;
            uint8_t hdr = (uint8_t)(usb_pci_read32((uint8_t)bus, slot, 0, 0x0C) >> 16);
            uint8_t max_func = (hdr & 0x80U) ? 8 : 1;

            for (uint8_t func = 0; func < max_func; func++) {
                uint16_t vendor = usb_pci_read16((uint8_t)bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue;
                uint32_t classreg = usb_pci_read32((uint8_t)bus, slot, func, 0x08);
                uint8_t class_code = (uint8_t)(classreg >> 24);
                uint8_t subclass   = (uint8_t)(classreg >> 16);
                uint8_t prog_if    = (uint8_t)(classreg >> 8);
                if (class_code != 0x0C || subclass != 0x03) continue;

                uint16_t cmd = usb_pci_read16((uint8_t)bus, slot, func, 0x04);
                cmd |= 0x07U;
                usb_pci_write16((uint8_t)bus, slot, func, 0x04, cmd);

                if (prog_if == 0x00 && uhci_ctrl_count < MAX_UHCI_CTRLS) {
                    uint32_t bar4 = usb_pci_read32((uint8_t)bus, slot, func, 0x20);
                    if ((bar4 & 0x01U) && (bar4 & 0xFFFCU)) {
                        usb_pci_write16((uint8_t)bus, slot, func, 0xC0, 0x8F00U);
                        uhci_ctrl_t *uc = &uhci_ctrls[uhci_ctrl_count++];
                        uc->present = 1;
                        uc->io_base = (uint16_t)(bar4 & 0xFFFCU);
                        uc->port_enumerated[0] = 0;
                        uc->port_enumerated[1] = 0;
                    }
                } else if (prog_if == 0x20) {
                    uint32_t bar0 = usb_pci_read32((uint8_t)bus, slot, func, 0x10);
                    ehci_handoff_to_companions(bar0);
                } else if (prog_if == 0x30) {
                    uint32_t bar0 = usb_pci_read32((uint8_t)bus, slot, func, 0x10);
                    xhci_add_controller((uint8_t)bus, slot, func, bar0);
                }
            }
        }
    }
}

static void uhci_reset_ctrl(int idx) {
    uint16_t io = uhci_ctrls[idx].io_base;
    usb_outw(io + UHCI_REG_CMD, UHCI_CMD_GRESET);
    usb_delay_ms(20);
    usb_outw(io + UHCI_REG_CMD, 0);
    usb_delay_ms(5);

    usb_outw(io + UHCI_REG_CMD, UHCI_CMD_HCRESET);
    for (int i = 0; i < 100; i++) {
        if (!(usb_inw(io + UHCI_REG_CMD) & UHCI_CMD_HCRESET)) break;
        usb_delay_ms(1);
    }

    usb_outw(io + UHCI_REG_INTR, 0);
    usb_outw(io + UHCI_REG_FRNUM, 0);
    usb_outb(io + UHCI_REG_SOFMOD, 0x40);

    uhci_ctrl_qhs[idx].head_link = UHCI_LINK_TERM;
    uhci_ctrl_qhs[idx].element_link = UHCI_LINK_TERM;

    uint32_t qh_link = ((uint32_t)&uhci_ctrl_qhs[idx]) | UHCI_LINK_QH;
    for (int i = 0; i < 1024; i++) {
        uhci_frame_lists[idx][i] = qh_link;
    }

    usb_outl(io + UHCI_REG_FLBASEADD, (uint32_t)uhci_frame_lists[idx]);
    usb_outw(io + UHCI_REG_STS, 0xFFFFU);
    usb_outw(io + UHCI_REG_CMD, UHCI_CMD_RS | UHCI_CMD_CF | UHCI_CMD_MAXP);
    usb_delay_ms(5);
}

/*
 * Reset portu UHCI: PR na 50 ms, potem zwolnienie. Port wlacza sie zapisem PED=1,
 * a bity zmian (CSC, PEDC) kasujemy zapisem jedynki. Zapis zerami czyscilby tylko
 * rejestr, a nie bity zdarzen.
 */
static int uhci_port_reset(uint16_t io, uint8_t port) {
    uint16_t reg = io + UHCI_REG_PORTSC1 + (uint16_t)(port - 1) * 2U;
    uint16_t val = usb_inw(reg);
    if (!(val & UHCI_PSC_CCS)) return 0;

    usb_outw(reg, UHCI_PSC_PR);
    usb_delay_ms(50);
    usb_outw(reg, 0);
    usb_delay_ms(10);

    for (int i = 0; i < 20; i++) {
        val = usb_inw(reg);
        if (!(val & UHCI_PSC_CCS)) return 0;

        usb_outw(reg, (uint16_t)((val & ~UHCI_PSC_PR) | UHCI_PSC_CSC | UHCI_PSC_PEDC | UHCI_PSC_PED));
        usb_delay_ms(10);

        val = usb_inw(reg);
        if (!(val & UHCI_PSC_CCS)) return 0;
        if (val & UHCI_PSC_PED) return 1;
    }
    return 0;
}

static int uhci_run_td_chain(uint8_t cidx, uhci_td_t *tds, int count) {
    if (count <= 0) return -1;
    uhci_qh_t *qh = &uhci_ctrl_qhs[cidx];
    uint16_t io = uhci_ctrls[cidx].io_base;

    for (int i = 0; i < count - 1; i++) {
        tds[i].link_ptr = ((uint32_t)&tds[i + 1]) | UHCI_LINK_VF;
    }
    tds[count - 1].link_ptr = UHCI_LINK_TERM;

    usb_outw(io + UHCI_REG_STS, 0xFFFFU);
    qh->element_link = (uint32_t)&tds[0];

    int ret = -1;
    for (uint32_t t = 0; t < 500; t++) {
        int all_done = 1;
        for (int i = 0; i < count; i++) {
            uint32_t cs = tds[i].ctrl_status;
            if (cs & TD_CTRL_ERR_MASK) {
                ret = -2;
                goto done;
            }
            if (cs & TD_CTRL_ACTIVE) {
                all_done = 0;
                break;
            }
        }
        if (all_done) {
            ret = 0;
            goto done;
        }
        usb_delay_ms(1);
    }

done:
    qh->element_link = UHCI_LINK_TERM;
    for (int i = 0; i < count; i++) {
        tds[i].ctrl_status &= ~TD_CTRL_ACTIVE;
    }
    return ret;
}

static int uhci_control_transfer(usb_device_t *dev, uint8_t bmRequestType, uint8_t bRequest,
                                 uint16_t wValue, uint16_t wIndex, uint16_t wLength, void *data) {
    uint8_t ls = (dev->speed == USB_SPEED_LOW) ? 1 : 0;
    uint32_t ls_bit = ls ? TD_CTRL_LS : 0;
    uint16_t mps = dev->max_packet_size0 ? dev->max_packet_size0 : 8;
    int td_count = 0;

    usb_setup_pkt[0] = bmRequestType;
    usb_setup_pkt[1] = bRequest;
    usb_setup_pkt[2] = (uint8_t)(wValue & 0xFFU);
    usb_setup_pkt[3] = (uint8_t)(wValue >> 8);
    usb_setup_pkt[4] = (uint8_t)(wIndex & 0xFFU);
    usb_setup_pkt[5] = (uint8_t)(wIndex >> 8);
    usb_setup_pkt[6] = (uint8_t)(wLength & 0xFFU);
    usb_setup_pkt[7] = (uint8_t)(wLength >> 8);

    td_pool[td_count].ctrl_status = TD_CTRL_ACTIVE | TD_CTRL_CERR3 | ls_bit;
    td_pool[td_count].token = TD_PID_SETUP | TD_TOKEN_ADDR(dev->address) | TD_TOKEN_EP(0) | TD_TOKEN_LEN(8);
    td_pool[td_count].buffer = (uint32_t)usb_setup_pkt;
    td_count++;

    uint8_t pid_data = (bmRequestType & USB_DIR_IN) ? TD_PID_IN : TD_PID_OUT;
    uint8_t toggle = 1;
    uint16_t offset = 0;
    uint8_t *buf = (uint8_t *)data;

    while (offset < wLength && td_count < MAX_TDS - 1) {
        uint16_t chunk = wLength - offset;
        if (chunk > mps) chunk = mps;
        td_pool[td_count].ctrl_status = TD_CTRL_ACTIVE | TD_CTRL_CERR3 | ls_bit;
        if (bmRequestType & USB_DIR_IN) td_pool[td_count].ctrl_status |= TD_CTRL_SPD;
        td_pool[td_count].token = pid_data | TD_TOKEN_ADDR(dev->address) | TD_TOKEN_EP(0) |
                                  (toggle ? TD_TOKEN_TOGGLE : 0) | TD_TOKEN_LEN(chunk);
        td_pool[td_count].buffer = (uint32_t)(buf + offset);
        td_count++;
        offset += chunk;
        toggle ^= 1;
    }

    uint8_t pid_status = (bmRequestType & USB_DIR_IN) ? TD_PID_OUT : TD_PID_IN;
    td_pool[td_count].ctrl_status = TD_CTRL_ACTIVE | TD_CTRL_CERR3 | ls_bit | TD_CTRL_IOC;
    td_pool[td_count].token = pid_status | TD_TOKEN_ADDR(dev->address) | TD_TOKEN_EP(0) |
                              TD_TOKEN_TOGGLE | TD_TOKEN_LEN(0);
    td_pool[td_count].buffer = 0;
    td_count++;

    return uhci_run_td_chain(dev->controller_idx, td_pool, td_count);
}

static int find_ep_idx(usb_device_t *dev, uint8_t ep_addr) {
    for (int i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (dev->ep[i].used && dev->ep[i].address == ep_addr) return i;
    }
    return -1;
}

static int uhci_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction) {
    int ep_idx = find_ep_idx(dev, ep_addr);
    if (ep_idx < 0) return -1;

    uint16_t mps = dev->ep[ep_idx].max_packet ? dev->ep[ep_idx].max_packet : 64;
    uint8_t pid = (direction == USB_DIR_IN) ? TD_PID_IN : TD_PID_OUT;
    uint8_t ep_num = ep_addr & 0x0FU;
    uint8_t *buf = (uint8_t *)data;
    uint16_t offset = 0;

    while (offset < length || (length == 0 && offset == 0)) {
        int td_count = 0;
        while ((offset < length || (length == 0 && td_count == 0)) && td_count < MAX_TDS) {
            uint16_t chunk = length - offset;
            if (chunk > mps) chunk = mps;
            td_pool[td_count].ctrl_status = TD_CTRL_ACTIVE | TD_CTRL_CERR3;
            td_pool[td_count].token = pid | TD_TOKEN_ADDR(dev->address) | TD_TOKEN_EP(ep_num) |
                                      (dev->ep[ep_idx].toggle ? TD_TOKEN_TOGGLE : 0) | TD_TOKEN_LEN(chunk);
            td_pool[td_count].buffer = (uint32_t)(buf + offset);
            td_count++;
            dev->ep[ep_idx].toggle ^= 1;
            offset += chunk;
            if (length == 0) break;
        }
        td_pool[td_count - 1].ctrl_status |= TD_CTRL_IOC;
        int ret = uhci_run_td_chain(dev->controller_idx, td_pool, td_count);
        if (ret != 0) return ret;
        if (length == 0) break;
    }
    return 0;
}

static void uhci_arm_intr_slot(uhci_intr_slot_t *s) {
    uint8_t ls_bit = (s->dev->speed == USB_SPEED_LOW) ? 1 : 0;
    uint8_t ep_num = s->ep_addr & 0x0FU;
    s->td.link_ptr = UHCI_LINK_TERM;
    s->td.ctrl_status = TD_CTRL_ACTIVE | TD_CTRL_CERR3 | (ls_bit ? TD_CTRL_LS : 0) | TD_CTRL_IOC | TD_CTRL_SPD;
    s->td.token = TD_PID_IN | TD_TOKEN_ADDR(s->dev->address) | TD_TOKEN_EP(ep_num) |
                  (s->dev->ep[s->ep_idx].toggle ? TD_TOKEN_TOGGLE : 0) | TD_TOKEN_LEN(s->length);
    s->td.buffer = (uint32_t)s->buf;
    s->qh.element_link = (uint32_t)&s->td;
}

static int uhci_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length) {
    int ep_idx = find_ep_idx(dev, ep_addr);
    if (ep_idx < 0 || length == 0 || length > 64) return -1;

    uhci_intr_slot_t *s = 0;
    for (int i = 0; i < MAX_INTR_SLOTS; i++) {
        if (intr_slots[i].used && intr_slots[i].dev == dev && intr_slots[i].ep_addr == ep_addr) {
            s = &intr_slots[i];
            break;
        }
    }

    if (!s) {
        int slot_idx = -1;
        for (int i = 0; i < MAX_INTR_SLOTS; i++) {
            if (!intr_slots[i].used) { slot_idx = i; break; }
        }
        if (slot_idx < 0) return -1;
        s = &intr_slots[slot_idx];
        s->used = 1;
        s->dev = dev;
        s->ep_addr = ep_addr;
        s->ep_idx = (uint8_t)ep_idx;
        s->length = length;

        uint8_t cidx = dev->controller_idx;
        s->qh.head_link = ((uint32_t)&uhci_ctrl_qhs[cidx]) | UHCI_LINK_QH;
        uhci_arm_intr_slot(s);

        uint32_t intr_link = ((uint32_t)&s->qh) | UHCI_LINK_QH;
        for (int f = slot_idx; f < 1024; f += 8) {
            uhci_frame_lists[cidx][f] = intr_link;
        }
        return -1;
    }

    uint32_t cs = s->td.ctrl_status;
    if (cs & TD_CTRL_ACTIVE) return -1;

    if (cs & TD_CTRL_ERR_MASK) {
        uhci_arm_intr_slot(s);
        return -2;
    }

    uint8_t *dst = (uint8_t *)data;
    for (uint8_t i = 0; i < length; i++) dst[i] = s->buf[i];
    dev->ep[ep_idx].toggle ^= 1;
    uhci_arm_intr_slot(s);
    return 0;
}

usb_device_t *usb_alloc_device_slot(void) {
    for (int i = 0; i < USB_MAX_DEVICES; i++) {
        if (!usb_devices[i].used) {
            uint8_t *p = (uint8_t *)&usb_devices[i];
            for (uint32_t k = 0; k < sizeof(usb_device_t); k++) p[k] = 0;
            usb_devices[i].used = 1;
            usb_devices[i].max_packet_size0 = 8;
            return &usb_devices[i];
        }
    }
    return 0;
}

void usb_free_device_slot(usb_device_t *dev) {
    if (!dev) return;
    for (int i = 0; i < MAX_INTR_SLOTS; i++) {
        if (intr_slots[i].used && intr_slots[i].dev == dev) {
            intr_slots[i].used = 0;
            intr_slots[i].td.ctrl_status = 0;
            intr_slots[i].qh.element_link = UHCI_LINK_TERM;
        }
    }
    dev->used = 0;
    dev->configured = 0;
}

static int usb_get_descriptor(usb_device_t *dev, uint8_t type, uint8_t index, uint16_t length, void *data) {
    uint8_t rt = USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
    return usb_control_transfer(dev, rt, USB_REQ_GET_DESCRIPTOR, ((uint16_t)type << 8) | index, 0, length, data);
}

static void usb_enumerate_hub(usb_device_t *hub) {
    uint8_t hub_desc[16];
    for (int i = 0; i < 16; i++) hub_desc[i] = 0;
    if (usb_control_transfer(hub, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
                             USB_REQ_GET_DESCRIPTOR, (USB_DT_HUB << 8), 0, 9, hub_desc) != 0) {
        return;
    }
    uint8_t nports = hub_desc[2];
    if (nports > 8) nports = 8;

    for (uint8_t p = 1; p <= nports; p++) {
        usb_control_transfer(hub, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER,
                             USB_REQ_SET_FEATURE, 8, p, 0, 0);
    }
    usb_delay_ms(20);

    for (uint8_t p = 1; p <= nports; p++) {
        uint32_t st = 0;
        if (usb_control_transfer(hub, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER,
                                 USB_REQ_GET_STATUS, 0, p, 4, &st) != 0) continue;
        if (!(st & 1U)) continue;

        usb_control_transfer(hub, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_OTHER,
                             USB_REQ_SET_FEATURE, 4, p, 0, 0);
        usb_delay_ms(50);
        if (usb_control_transfer(hub, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_OTHER,
                                 USB_REQ_GET_STATUS, 0, p, 4, &st) != 0) continue;
        if (!(st & 2U)) continue;

        if (hub->controller_type == USB_CTRL_UHCI) {
            usb_device_t *child = usb_alloc_device_slot();
            if (!child) continue;
            child->controller_type = USB_CTRL_UHCI;
            child->controller_idx = hub->controller_idx;
            child->io_base = hub->io_base;
            child->port = p;
            child->speed = (st & (1U << 9)) ? USB_SPEED_LOW : ((st & (1U << 10)) ? USB_SPEED_HIGH : USB_SPEED_FULL);
            if (usb_enumerate_common(child, 1) != 0) {
                usb_free_device_slot(child);
            }
        }
    }
}

int usb_enumerate_common(usb_device_t *dev, uint8_t assign_address) {
    uint8_t devdesc[18];
    for (int i = 0; i < 18; i++) devdesc[i] = 0;

    if (usb_get_descriptor(dev, USB_DT_DEVICE, 0, 8, devdesc) != 0) return -1;
    usb_device_descriptor_t *dd = (usb_device_descriptor_t *)devdesc;
    if (dd->bMaxPacketSize0) {
        dev->max_packet_size0 = dd->bMaxPacketSize0;
        if (dev->controller_type == USB_CTRL_XHCI) {
            xhci_update_ep0_mps(dev, dev->max_packet_size0);
        }
    }

    if (assign_address) {
        uint8_t addr = usb_next_addr++;
        if (usb_next_addr > 120) usb_next_addr = 1;
        if (usb_control_transfer(dev, USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
                                 USB_REQ_SET_ADDRESS, addr, 0, 0, 0) != 0) {
            return -1;
        }
        dev->address = addr;
        usb_delay_ms(5);
    }

    if (usb_get_descriptor(dev, USB_DT_DEVICE, 0, 18, devdesc) != 0) return -1;
    dd = (usb_device_descriptor_t *)devdesc;
    dev->id_vendor  = dd->idVendor;
    dev->id_product = dd->idProduct;
    dev->dev_class  = dd->bDeviceClass;
    dev->subclass   = dd->bDeviceSubClass;
    dev->protocol   = dd->bDeviceProtocol;

    uint8_t cfg_hdr[9];
    if (usb_get_descriptor(dev, USB_DT_CONFIG, 0, 9, cfg_hdr) != 0) return -1;
    usb_config_descriptor_t *cd = (usb_config_descriptor_t *)cfg_hdr;
    uint16_t total = cd->wTotalLength;
    if (total < 9) total = 9;
    if (total > 256) total = 256;

    uint8_t config_buf[256];
    if (usb_get_descriptor(dev, USB_DT_CONFIG, 0, total, config_buf) != 0) return -1;
    cd = (usb_config_descriptor_t *)config_buf;
    uint8_t cfg_val = cd->bConfigurationValue;

    uint8_t *ptr = config_buf;
    uint8_t *end = config_buf + total;
    usb_device_t *cur_dev = dev;
    int iface_count = 0;
    uint8_t ep_idx = 0;

    while (ptr + 2 <= end) {
        uint8_t bLength = ptr[0];
        uint8_t bType = ptr[1];
        if (bLength == 0 || ptr + bLength > end) break;

        if (bType == USB_DT_INTERFACE && bLength >= sizeof(usb_interface_descriptor_t)) {
            usb_interface_descriptor_t *id = (usb_interface_descriptor_t *)ptr;
            if (id->bAlternateSetting == 0) {
                if (iface_count > 0) {
                    usb_device_t *sub = usb_alloc_device_slot();
                    if (sub) {
                        *sub = *dev;
                        for (int k = 0; k < USB_MAX_ENDPOINTS; k++) sub->ep[k].used = 0;
                        sub->class_data = 0;
                        cur_dev = sub;
                    } else {
                        cur_dev = 0;
                    }
                }
                if (cur_dev) {
                    cur_dev->dev_class = id->bInterfaceClass;
                    cur_dev->subclass = id->bInterfaceSubClass;
                    cur_dev->protocol = id->bInterfaceProtocol;
                    cur_dev->interface_num = id->bInterfaceNumber;
                    ep_idx = 0;
                }
                iface_count++;
            } else {
                cur_dev = 0;
            }
        } else if (bType == USB_DT_ENDPOINT && cur_dev && bLength >= sizeof(usb_endpoint_descriptor_t)) {
            usb_endpoint_descriptor_t *ed = (usb_endpoint_descriptor_t *)ptr;
            if (ep_idx < USB_MAX_ENDPOINTS) {
                cur_dev->ep[ep_idx].used = 1;
                cur_dev->ep[ep_idx].address = ed->bEndpointAddress;
                cur_dev->ep[ep_idx].type = ed->bmAttributes & 0x03U;
                cur_dev->ep[ep_idx].max_packet = ed->wMaxPacketSize & 0x07FFU;
                cur_dev->ep[ep_idx].toggle = 0;
                cur_dev->ep[ep_idx].interval = ed->bInterval;
                ep_idx++;
            }
        }
        ptr += bLength;
    }

    if (dev->controller_type == USB_CTRL_XHCI) {
        for (int i = 0; i < USB_MAX_DEVICES; i++) {
            if (usb_devices[i].used && usb_devices[i].controller_type == USB_CTRL_XHCI &&
                usb_devices[i].slot_id == dev->slot_id) {
                xhci_configure_endpoints(&usb_devices[i]);
            }
        }
    }

    if (usb_control_transfer(dev, USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE,
                             USB_REQ_SET_CONFIGURATION, cfg_val, 0, 0, 0) != 0) {
        return -1;
    }
    usb_delay_ms(5);

    for (int i = 0; i < USB_MAX_DEVICES; i++) {
        if (usb_devices[i].used && usb_devices[i].controller_type == dev->controller_type &&
            usb_devices[i].controller_idx == dev->controller_idx &&
            usb_devices[i].address == dev->address) {
            usb_devices[i].configured = 1;
        }
    }

    if (dev->dev_class == USB_CLASS_HUB) {
        usb_enumerate_hub(dev);
    }
    return 0;
}

static void uhci_enumerate_port(int cidx, uint8_t port) {
    uint16_t io = uhci_ctrls[cidx].io_base;
    if (!uhci_port_reset(io, port)) return;

    uint16_t status = usb_inw(io + UHCI_REG_PORTSC1 + (uint16_t)(port - 1) * 2U);
    usb_device_t *dev = usb_alloc_device_slot();
    if (!dev) return;

    dev->controller_type = USB_CTRL_UHCI;
    dev->controller_idx = (uint8_t)cidx;
    dev->io_base = io;
    dev->port = port;
    dev->speed = (status & UHCI_PSC_LSDA) ? USB_SPEED_LOW : USB_SPEED_FULL;

    if (usb_enumerate_common(dev, 1) != 0) {
        usb_free_device_slot(dev);
    } else {
        uhci_ctrls[cidx].port_enumerated[port - 1] = 1;
    }
}

int usb_init(void) {
    for (int i = 0; i < USB_MAX_DEVICES; i++) usb_devices[i].used = 0;
    for (int i = 0; i < MAX_INTR_SLOTS; i++) intr_slots[i].used = 0;
    for (int i = 0; i < MAX_UHCI_CTRLS; i++) uhci_ctrls[i].present = 0;
    uhci_ctrl_count = 0;
    usb_next_addr = 1;

    pci_scan_controllers();

    for (int i = 0; i < uhci_ctrl_count; i++) {
        uhci_reset_ctrl(i);
        for (uint8_t port = 1; port <= 2; port++) {
            uint16_t st = usb_inw(uhci_ctrls[i].io_base + UHCI_REG_PORTSC1 + (uint16_t)(port - 1) * 2U);
            if (st & UHCI_PSC_CCS) {
                uhci_enumerate_port(i, port);
            }
        }
    }

    int xhci_count = xhci_init_all();
    return (uhci_ctrl_count > 0 || xhci_count > 0) ? 1 : 0;
}

void usb_poll(void) {
    for (int i = 0; i < uhci_ctrl_count; i++) {
        uint16_t io = uhci_ctrls[i].io_base;
        for (uint8_t port = 1; port <= 2; port++) {
            uint16_t reg = io + UHCI_REG_PORTSC1 + (uint16_t)(port - 1) * 2U;
            uint16_t st = usb_inw(reg);
            if ((st & UHCI_PSC_CCS) && !uhci_ctrls[i].port_enumerated[port - 1]) {
                uhci_enumerate_port(i, port);
            } else if (!(st & UHCI_PSC_CCS) && uhci_ctrls[i].port_enumerated[port - 1]) {
                uhci_ctrls[i].port_enumerated[port - 1] = 0;
            }
        }
    }
    xhci_poll();
}

usb_device_t *usb_device_get(int index) {
    int seen = 0;
    for (int i = 0; i < USB_MAX_DEVICES; i++) {
        if (usb_devices[i].used) {
            if (seen == index) return &usb_devices[i];
            seen++;
        }
    }
    return 0;
}

int usb_device_count(void) {
    int count = 0;
    for (int i = 0; i < USB_MAX_DEVICES; i++) {
        if (usb_devices[i].used) count++;
    }
    return count;
}

int usb_control_transfer(usb_device_t *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data) {
    if (!dev || !dev->used) return -1;
    if (dev->controller_type == USB_CTRL_XHCI) {
        return xhci_control_transfer(dev, request_type, request, value, index, length, data);
    }
    return uhci_control_transfer(dev, request_type, request, value, index, length, data);
}

int usb_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction) {
    if (!dev || !dev->used) return -1;
    if (dev->controller_type == USB_CTRL_XHCI) {
        return xhci_bulk_transfer(dev, ep_addr, data, length, direction);
    }
    return uhci_bulk_transfer(dev, ep_addr, data, length, direction);
}

int usb_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length) {
    if (!dev || !dev->used) return -1;
    if (dev->controller_type == USB_CTRL_XHCI) {
        return xhci_interrupt_transfer(dev, ep_addr, data, length);
    }
    return uhci_interrupt_transfer(dev, ep_addr, data, length);
}