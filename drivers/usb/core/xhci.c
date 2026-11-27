#include <drivers/usb/core/xhci.h>
#include <drivers/usb/core/usb.h>

#define XHCI_MAX_CTRLS 2
#define XHCI_MAX_SLOTS 16
#define XHCI_RING_SIZE 32
#define XHCI_EVT_SIZE  64

#define TRB_TYPE_NORMAL        1
#define TRB_TYPE_SETUP         2
#define TRB_TYPE_DATA          3
#define TRB_TYPE_STATUS        4
#define TRB_TYPE_LINK          6
#define TRB_TYPE_ENABLE_SLOT   9
#define TRB_TYPE_DISABLE_SLOT  10
#define TRB_TYPE_ADDR_DEV      11
#define TRB_TYPE_CFG_EP        12
#define TRB_TYPE_EVAL_CTX      13
#define TRB_TYPE_TRANSFER_EVT  32
#define TRB_TYPE_CMD_COMP_EVT  33
#define TRB_TYPE_PORT_CHG_EVT  34

#define TRB_CYCLE              (1U << 0)
#define TRB_TC                 (1U << 1)
#define TRB_ISP                (1U << 2)
#define TRB_CH                 (1U << 4)
#define TRB_IOC                (1U << 5)
#define TRB_IDT                (1U << 6)
#define TRB_DIR_IN             (1U << 16)

typedef struct {
    volatile uint32_t param_lo;
    volatile uint32_t param_hi;
    volatile uint32_t status;
    volatile uint32_t control;
} __attribute__((packed, aligned(16))) xhci_trb_t;

typedef struct {
    volatile uint64_t addr;
    volatile uint32_t size;
    volatile uint32_t rsvd;
} __attribute__((packed, aligned(16))) xhci_erst_entry_t;

typedef struct {
    uint8_t  present;
    uint32_t mmio_base;
    uint32_t op_base;
    uint32_t rt_base;
    uint32_t db_base;
    uint8_t  max_slots;
    uint8_t  max_ports;
    uint8_t  ctx_size;
    uint32_t cmd_enq;
    uint8_t  cmd_cycle;
    uint32_t evt_deq;
    uint8_t  evt_cycle;
} xhci_ctrl_t;

typedef struct {
    uint8_t used;
    uint8_t armed;
    uint8_t ready;
    int     status;
    uint8_t buf[64];
    uint32_t enq;
    uint8_t cycle;
} xhci_ep_ring_state_t;

static xhci_ctrl_t xhci_ctrls[XHCI_MAX_CTRLS];
static int xhci_ctrl_count = 0;

static uint64_t xhci_dcbaa[XHCI_MAX_SLOTS + 1] __attribute__((aligned(64)));
static xhci_trb_t xhci_cmd_ring[XHCI_RING_SIZE] __attribute__((aligned(64)));
static xhci_trb_t xhci_evt_ring[XHCI_EVT_SIZE] __attribute__((aligned(64)));
static xhci_erst_entry_t xhci_erst[1] __attribute__((aligned(64)));

static uint8_t xhci_input_ctx[2112] __attribute__((aligned(64)));

static uint8_t xhci_dev_ctx[XHCI_MAX_SLOTS][2048] __attribute__((aligned(64)));

static xhci_trb_t xhci_xfer_rings[XHCI_MAX_SLOTS][16][XHCI_RING_SIZE] __attribute__((aligned(64)));
static xhci_ep_ring_state_t xhci_ep_state[XHCI_MAX_SLOTS][16];

static uint8_t xhci_port_enumerated[XHCI_MAX_CTRLS][32];

static void xhci_memzero(void *dst, uint32_t n) {
    uint8_t *p = (uint8_t *)dst;
    for (uint32_t i = 0; i < n; i++) p[i] = 0;
}

static inline uint32_t mmio_r32(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

static inline void mmio_w32(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

static inline void mmio_w64(uint32_t addr, uint64_t val) {
    *(volatile uint32_t *)addr = (uint32_t)(val & 0xFFFFFFFFU);
    *(volatile uint32_t *)(addr + 4) = (uint32_t)(val >> 32);
}

static void xhci_ring_db(xhci_ctrl_t *c, uint8_t slot_id, uint32_t target) {
    mmio_w32(c->db_base + (uint32_t)slot_id * 4U, target);
}

static uint8_t xhci_ep_dci(uint8_t ep_addr) {
    uint8_t num = ep_addr & 0x0F;
    if (num == 0) return 1;
    return (uint8_t)(num * 2 + ((ep_addr & 0x80) ? 1 : 0));
}

static void xhci_ring_enqueue(xhci_trb_t *ring, uint32_t *enq, uint8_t *cycle,
                              uint32_t plo, uint32_t phi, uint32_t status, uint32_t ctrl) {
    uint32_t idx = *enq;
    ring[idx].param_lo = plo;
    ring[idx].param_hi = phi;
    ring[idx].status = status;
    ring[idx].control = (ctrl & ~1U) | (*cycle ? 1U : 0U);
    idx++;
    if (idx >= XHCI_RING_SIZE - 1) {
        ring[idx].param_lo = (uint32_t)ring;
        ring[idx].param_hi = 0;
        ring[idx].status = 0;
        ring[idx].control = (TRB_TYPE_LINK << 10) | TRB_TC | (*cycle ? 1U : 0U);
        *enq = 0;
        *cycle ^= 1;
    } else {
        *enq = idx;
    }
}

static void xhci_process_events(xhci_ctrl_t *c, int *cmd_done, uint8_t *cmd_slot, uint8_t *cmd_cc,
                                uint8_t target_slot, uint8_t target_dci, int *xfer_done, int *xfer_cc) {
    while (1) {
        xhci_trb_t *ev = &xhci_evt_ring[c->evt_deq];
        uint32_t ctrl = ev->control;
        if ((ctrl & 1U) != c->evt_cycle) break;

        uint32_t type = (ctrl >> 10) & 0x3F;
        uint8_t cc = (uint8_t)((ev->status >> 24) & 0xFF);

        if (type == TRB_TYPE_CMD_COMP_EVT && cmd_done) {
            *cmd_done = 1;
            if (cmd_slot) *cmd_slot = (uint8_t)((ctrl >> 24) & 0xFF);
            if (cmd_cc) *cmd_cc = cc;
        } else if (type == TRB_TYPE_TRANSFER_EVT) {
            uint8_t s = (uint8_t)((ctrl >> 24) & 0xFF);
            uint8_t dci = (uint8_t)((ctrl >> 16) & 0x1F);
            if (xfer_done && s == target_slot && dci == target_dci) {
                *xfer_done = 1;
                if (xfer_cc) *xfer_cc = (int)cc;
            } else if (s > 0 && s < XHCI_MAX_SLOTS && dci > 0 && dci < 16) {
                xhci_ep_ring_state_t *st = &xhci_ep_state[s][dci];
                if (st->armed) {
                    st->armed = 0;
                    st->ready = 1;
                    st->status = (cc == 1 || cc == 13) ? 0 : -2;
                }
            }
        }

        c->evt_deq++;
        if (c->evt_deq >= XHCI_EVT_SIZE) {
            c->evt_deq = 0;
            c->evt_cycle ^= 1;
        }
        uint64_t erdp = (uint64_t)(uint32_t)&xhci_evt_ring[c->evt_deq];
        mmio_w64(c->rt_base + 0x20 + 0x18, erdp | (1ULL << 3));
    }
}

static int xhci_send_cmd(xhci_ctrl_t *c, uint32_t plo, uint32_t phi, uint32_t status, uint32_t ctrl, uint8_t *out_slot) {
    int done = 0;
    uint8_t slot = 0, cc = 0;
    xhci_ring_enqueue(xhci_cmd_ring, &c->cmd_enq, &c->cmd_cycle, plo, phi, status, ctrl);
    xhci_ring_db(c, 0, 0);

    for (uint32_t t = 0; t < 500; t++) {
        xhci_process_events(c, &done, &slot, &cc, 0, 0, 0, 0);
        if (done) {
            if (out_slot) *out_slot = slot;
            return (cc == 1) ? 0 : -(int)cc;
        }
        usb_delay_ms(1);
    }
    return -1;
}

static void xhci_bios_handoff(uint32_t mmio_base, uint32_t hccparams1) {
    uint32_t xecp = (hccparams1 >> 16) & 0xFFFF;
    if (!xecp) return;
    uint32_t addr = mmio_base + (xecp << 2);
    for (int guard = 0; guard < 32; guard++) {
        uint32_t cap = mmio_r32(addr);
        uint8_t id = (uint8_t)(cap & 0xFF);
        if (id == 1) {
            if (cap & (1U << 16)) {
                mmio_w32(addr, cap | (1U << 24));
                for (int i = 0; i < 200; i++) {
                    if (!(mmio_r32(addr) & (1U << 16))) break;
                    usb_delay_ms(1);
                }
            }
            uint32_t ctl = mmio_r32(addr + 4);
            mmio_w32(addr + 4, ctl & ~0x0000000FU);
            return;
        }
        uint8_t next = (uint8_t)((cap >> 8) & 0xFF);
        if (!next) break;
        addr += (uint32_t)next << 2;
    }
}

static uint32_t xhci_default_mps(uint8_t port_speed) {
    if (port_speed == 2) return 8;
    if (port_speed == 3) return 64;
    if (port_speed == 4) return 512;
    return 8;
}

static uint8_t xhci_map_speed(uint8_t port_speed) {
    if (port_speed == 2) return USB_SPEED_LOW;
    if (port_speed == 3) return USB_SPEED_HIGH;
    if (port_speed == 4) return USB_SPEED_SUPER;
    return USB_SPEED_FULL;
}

/*
 * Pole Interval w kontekscie endpointu (xHCI 6.2.3.6):
 *  - HS/SS: bInterval jest juz w potegach 2 (interwal = 2^(v) * 125 us), wiec v = bInterval-1.
 *  - LS/FS: bInterval jest w milisekundach, a v = floor(log2(bInterval * 8)).
 * Bez tej konwersji mysz z bInterval=10 w FS dostaje ~128 ms zamiast 10 ms.
 */
static uint32_t xhci_interval_value(uint8_t speed, uint8_t binterval) {
    uint32_t v = 0;
    if (speed == USB_SPEED_HIGH || speed == USB_SPEED_SUPER) {
        if (binterval == 0) binterval = 1;
        v = (uint32_t)binterval - 1U;
    } else {
        uint32_t ms = binterval ? binterval : 1U;
        while (ms > 1U) { ms >>= 1; v++; }
        v += 3U;
    }
    if (v > 15U) v = 15U;
    return v;
}

static void xhci_enumerate_port(int cidx, uint8_t port) {
    xhci_ctrl_t *c = &xhci_ctrls[cidx];
    uint32_t portsc_addr = c->op_base + 0x400 + (uint32_t)(port - 1) * 0x10;
    uint32_t portsc = mmio_r32(portsc_addr);
    if (!(portsc & 1U)) return;

    if (!(portsc & (1U << 1))) {
        mmio_w32(portsc_addr, (portsc & 0x0E01C3E0U) | (1U << 4));
        for (int i = 0; i < 200; i++) {
            portsc = mmio_r32(portsc_addr);
            if (!(portsc & (1U << 4)) && (portsc & (1U << 1))) break;
            usb_delay_ms(1);
        }
        usb_delay_ms(10);
    }
    portsc = mmio_r32(portsc_addr);
    if (!(portsc & (1U << 1))) return;

    mmio_w32(portsc_addr, (portsc & 0x0E01C3E0U) | 0x00FE0000U);

    uint8_t port_speed = (uint8_t)((portsc >> 10) & 0xF);
    uint8_t slot_id = 0;
    if (xhci_send_cmd(c, 0, 0, 0, (TRB_TYPE_ENABLE_SLOT << 10), &slot_id) != 0) return;
    if (slot_id == 0 || slot_id >= XHCI_MAX_SLOTS) return;

    xhci_memzero(xhci_dev_ctx[slot_id], 2048);
    xhci_dcbaa[slot_id] = (uint64_t)(uint32_t)xhci_dev_ctx[slot_id];

    xhci_memzero(xhci_xfer_rings[slot_id], sizeof(xhci_xfer_rings[slot_id]));
    xhci_memzero(xhci_ep_state[slot_id], sizeof(xhci_ep_state[slot_id]));
    xhci_ep_state[slot_id][1].used = 1;
    xhci_ep_state[slot_id][1].enq = 0;
    xhci_ep_state[slot_id][1].cycle = 1;

    xhci_memzero(xhci_input_ctx, sizeof(xhci_input_ctx));
    uint32_t *icc = (uint32_t *)xhci_input_ctx;
    icc[1] = 0x03U;

    uint32_t *slot_ctx = (uint32_t *)(xhci_input_ctx + c->ctx_size);
    slot_ctx[0] = ((uint32_t)1 << 27) | ((uint32_t)port_speed << 20);
    slot_ctx[1] = ((uint32_t)port << 16);

    uint32_t *ep0_ctx = (uint32_t *)(xhci_input_ctx + 2 * c->ctx_size);
    uint32_t mps0 = xhci_default_mps(port_speed);
    ep0_ctx[1] = (3U << 1) | (4U << 3) | (mps0 << 16);
    uint64_t tr_ptr = (uint64_t)(uint32_t)xhci_xfer_rings[slot_id][1];
    ep0_ctx[2] = (uint32_t)(tr_ptr | 1U);
    ep0_ctx[3] = 0;
    ep0_ctx[4] = 8;

    uint32_t addr_ctrl = (TRB_TYPE_ADDR_DEV << 10) | ((uint32_t)slot_id << 24);
    if (xhci_send_cmd(c, (uint32_t)xhci_input_ctx, 0, 0, addr_ctrl, 0) != 0) {
        xhci_send_cmd(c, 0, 0, 0, (TRB_TYPE_DISABLE_SLOT << 10) | ((uint32_t)slot_id << 24), 0);
        return;
    }
    usb_delay_ms(5);

    usb_device_t *dev = usb_alloc_device_slot();
    if (!dev) return;
    dev->controller_type = USB_CTRL_XHCI;
    dev->controller_idx = (uint8_t)cidx;
    dev->slot_id = slot_id;
    dev->address = slot_id;
    dev->port = port;
    dev->root_port = port;
    dev->speed = xhci_map_speed(port_speed);
    dev->max_packet_size0 = (uint8_t)(mps0 > 255 ? 64 : mps0);

    if (usb_enumerate_common(dev, 0) != 0) {
        usb_free_device_slot(dev);
        xhci_send_cmd(c, 0, 0, 0, (TRB_TYPE_DISABLE_SLOT << 10) | ((uint32_t)slot_id << 24), 0);
    } else {
        xhci_port_enumerated[cidx][port] = 1;
    }
}

int xhci_add_controller(uint8_t bus, uint8_t slot, uint8_t func, uint32_t bar0) {
    if (xhci_ctrl_count >= XHCI_MAX_CTRLS) return -1;
    uint32_t mmio = bar0 & 0xFFFFFFF0U;
    if (!mmio) return -1;

    if ((bar0 & 0x06U) == 0x04U) {
        uint32_t bar_hi = usb_pci_read32(bus, slot, func, 0x14);
        if (bar_hi != 0) return -1;
    }
    xhci_ctrl_t *c = &xhci_ctrls[xhci_ctrl_count++];
    xhci_memzero(c, sizeof(*c));
    c->present = 1;
    c->mmio_base = mmio;
    return 0;
}

int xhci_init_all(void) {
    int active = 0;
    for (int i = 0; i < xhci_ctrl_count; i++) {
        xhci_ctrl_t *c = &xhci_ctrls[i];
        if (!c->present) continue;

        uint32_t cap0 = mmio_r32(c->mmio_base + 0x00);
        uint8_t caplen = (uint8_t)(cap0 & 0xFF);
        uint32_t hcs1 = mmio_r32(c->mmio_base + 0x04);
        uint32_t hcc1 = mmio_r32(c->mmio_base + 0x10);
        uint32_t dboff = mmio_r32(c->mmio_base + 0x14) & ~0x3U;
        uint32_t rtsoff = mmio_r32(c->mmio_base + 0x18) & ~0x1FU;

        c->op_base = c->mmio_base + caplen;
        c->db_base = c->mmio_base + dboff;
        c->rt_base = c->mmio_base + rtsoff;
        c->max_slots = (uint8_t)(hcs1 & 0xFF);
        if (c->max_slots > XHCI_MAX_SLOTS - 1) c->max_slots = XHCI_MAX_SLOTS - 1;
        c->max_ports = (uint8_t)((hcs1 >> 24) & 0xFF);
        if (c->max_ports > 31) c->max_ports = 31;
        c->ctx_size = (hcc1 & (1U << 2)) ? 64 : 32;

        xhci_bios_handoff(c->mmio_base, hcc1);

        uint32_t cmd = mmio_r32(c->op_base + 0x00);
        mmio_w32(c->op_base + 0x00, cmd & ~1U);
        for (int t = 0; t < 100; t++) {
            if (mmio_r32(c->op_base + 0x04) & 1U) break;
            usb_delay_ms(1);
        }

        mmio_w32(c->op_base + 0x00, (1U << 1));
        for (int t = 0; t < 200; t++) {
            uint32_t usbcmd = mmio_r32(c->op_base + 0x00);
            uint32_t usbsts = mmio_r32(c->op_base + 0x04);
            if (!(usbcmd & (1U << 1)) && !(usbsts & (1U << 11))) break;
            usb_delay_ms(1);
        }

        mmio_w32(c->op_base + 0x38, c->max_slots);
        xhci_memzero(xhci_dcbaa, sizeof(xhci_dcbaa));
        mmio_w64(c->op_base + 0x30, (uint64_t)(uint32_t)xhci_dcbaa);

        xhci_memzero(xhci_cmd_ring, sizeof(xhci_cmd_ring));
        c->cmd_enq = 0;
        c->cmd_cycle = 1;
        mmio_w64(c->op_base + 0x18, (uint64_t)(uint32_t)xhci_cmd_ring | 1ULL);

        xhci_memzero(xhci_evt_ring, sizeof(xhci_evt_ring));
        xhci_memzero(xhci_erst, sizeof(xhci_erst));
        xhci_erst[0].addr = (uint64_t)(uint32_t)xhci_evt_ring;
        xhci_erst[0].size = XHCI_EVT_SIZE;
        c->evt_deq = 0;
        c->evt_cycle = 1;

        /* Kolejnosc wazna: ERSTSZ, potem ERSTBA, dopiero potem ERDP, na koncu IMAN. */
        mmio_w32(c->rt_base + 0x20 + 0x08, 1);                                 /* ERSTSZ */
        mmio_w64(c->rt_base + 0x20 + 0x10, (uint64_t)(uint32_t)xhci_erst);     /* ERSTBA */
        mmio_w64(c->rt_base + 0x20 + 0x18, (uint64_t)(uint32_t)xhci_evt_ring); /* ERDP */
        mmio_w32(c->rt_base + 0x20 + 0x00, 0);                                 /* IMAN */

        mmio_w32(c->op_base + 0x00, 1U);
        usb_delay_ms(20);

        for (uint8_t p = 1; p <= c->max_ports; p++) {
            xhci_port_enumerated[i][p] = 0;
            xhci_enumerate_port(i, p);
        }
        active++;
    }
    return active;
}

void xhci_poll(void) {
    for (int i = 0; i < xhci_ctrl_count; i++) {
        xhci_ctrl_t *c = &xhci_ctrls[i];
        if (!c->present) continue;
        xhci_process_events(c, 0, 0, 0, 0, 0, 0, 0);
        for (uint8_t p = 1; p <= c->max_ports; p++) {
            uint32_t portsc_addr = c->op_base + 0x400 + (uint32_t)(p - 1) * 0x10;
            uint32_t portsc = mmio_r32(portsc_addr);
            if ((portsc & 1U) && !xhci_port_enumerated[i][p]) {
                xhci_enumerate_port(i, p);
            } else if (!(portsc & 1U) && xhci_port_enumerated[i][p]) {
                xhci_port_enumerated[i][p] = 0;
            }
        }
    }
}

int xhci_update_ep0_mps(usb_device_t *dev, uint16_t mps) {
    xhci_ctrl_t *c = &xhci_ctrls[dev->controller_idx];
    uint8_t slot_id = dev->slot_id;
    xhci_memzero(xhci_input_ctx, sizeof(xhci_input_ctx));
    uint32_t *icc = (uint32_t *)xhci_input_ctx;
    icc[1] = 0x02U;
    uint32_t *ep0_ctx = (uint32_t *)(xhci_input_ctx + 2 * c->ctx_size);
    uint8_t *cur_ep0 = xhci_dev_ctx[slot_id] + c->ctx_size;
    for (uint32_t k = 0; k < c->ctx_size; k++) ((uint8_t *)ep0_ctx)[k] = cur_ep0[k];
    ep0_ctx[1] = (ep0_ctx[1] & 0x0000FFFFU) | ((uint32_t)mps << 16);
    uint32_t ctrl = (TRB_TYPE_EVAL_CTX << 10) | ((uint32_t)slot_id << 24);
    return xhci_send_cmd(c, (uint32_t)xhci_input_ctx, 0, 0, ctrl, 0);
}

int xhci_configure_endpoints(usb_device_t *dev) {
    xhci_ctrl_t *c = &xhci_ctrls[dev->controller_idx];
    uint8_t slot_id = dev->slot_id;
    xhci_memzero(xhci_input_ctx, sizeof(xhci_input_ctx));
    uint32_t *icc = (uint32_t *)xhci_input_ctx;
    uint32_t add_flags = 0x01U;
    uint8_t max_dci = 1;

    uint8_t *dst_slot = xhci_input_ctx + c->ctx_size;
    uint8_t *src_slot = xhci_dev_ctx[slot_id];
    for (uint32_t k = 0; k < c->ctx_size; k++) dst_slot[k] = src_slot[k];

    for (int i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (!dev->ep[i].used) continue;
        uint8_t dci = xhci_ep_dci(dev->ep[i].address);
        if (dci <= 1 || dci >= 16) continue;

        uint32_t ep_type = 0;
        if (dev->ep[i].type == USB_ENDPOINT_BULK) {
            ep_type = (dev->ep[i].address & 0x80) ? 6 : 2;
        } else if (dev->ep[i].type == USB_ENDPOINT_INTERRUPT) {
            ep_type = (dev->ep[i].address & 0x80) ? 7 : 3;
        } else {
            continue;
        }

        if (dci > max_dci) max_dci = dci;
        add_flags |= (1U << dci);

        xhci_ep_state[slot_id][dci].used = 1;
        xhci_ep_state[slot_id][dci].armed = 0;
        xhci_ep_state[slot_id][dci].ready = 0;
        xhci_ep_state[slot_id][dci].enq = 0;
        xhci_ep_state[slot_id][dci].cycle = 1;
        xhci_memzero(xhci_xfer_rings[slot_id][dci], sizeof(xhci_xfer_rings[slot_id][dci]));

        uint32_t *ep_ctx = (uint32_t *)(xhci_input_ctx + (1 + dci) * c->ctx_size);
        ep_ctx[0] = (ep_type == 3 || ep_type == 7)
                    ? (xhci_interval_value(dev->speed, dev->ep[i].interval) << 16)
                    : 0;
        ep_ctx[1] = (3U << 1) | (ep_type << 3) | ((uint32_t)dev->ep[i].max_packet << 16);
        uint64_t tr_ptr = (uint64_t)(uint32_t)xhci_xfer_rings[slot_id][dci];
        ep_ctx[2] = (uint32_t)(tr_ptr | 1U);
        ep_ctx[3] = 0;
        ep_ctx[4] = dev->ep[i].max_packet;
    }

    if (max_dci == 1) return 0;
    icc[1] = add_flags;
    uint32_t *sc = (uint32_t *)dst_slot;
    sc[0] = (sc[0] & 0x07FFFFFFU) | ((uint32_t)max_dci << 27);

    uint32_t ctrl = (TRB_TYPE_CFG_EP << 10) | ((uint32_t)slot_id << 24);
    return xhci_send_cmd(c, (uint32_t)xhci_input_ctx, 0, 0, ctrl, 0);
}

int xhci_control_transfer(usb_device_t *dev, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data) {
    xhci_ctrl_t *c = &xhci_ctrls[dev->controller_idx];
    uint8_t slot_id = dev->slot_id;
    xhci_ep_ring_state_t *st = &xhci_ep_state[slot_id][1];
    xhci_trb_t *ring = xhci_xfer_rings[slot_id][1];

    uint32_t setup_lo = (uint32_t)request_type | ((uint32_t)request << 8) | ((uint32_t)value << 16);
    uint32_t setup_hi = (uint32_t)index | ((uint32_t)length << 16);
    uint32_t trt = 0;
    if (length > 0) trt = (request_type & USB_DIR_IN) ? 3U : 2U;

    xhci_ring_enqueue(ring, &st->enq, &st->cycle, setup_lo, setup_hi, 8U,
                      (TRB_TYPE_SETUP << 10) | TRB_IDT | (trt << 16));

    if (length > 0 && data) {
        uint32_t dir = (request_type & USB_DIR_IN) ? TRB_DIR_IN : 0;
        xhci_ring_enqueue(ring, &st->enq, &st->cycle, (uint32_t)data, 0, (uint32_t)length,
                          (TRB_TYPE_DATA << 10) | dir);
    }

    uint32_t status_dir = (length > 0 && (request_type & USB_DIR_IN)) ? 0 : TRB_DIR_IN;
    xhci_ring_enqueue(ring, &st->enq, &st->cycle, 0, 0, 0,
                      (TRB_TYPE_STATUS << 10) | status_dir | TRB_IOC);

    int done = 0, cc = 0;
    xhci_ring_db(c, slot_id, 1);
    for (uint32_t t = 0; t < 500; t++) {
        xhci_process_events(c, 0, 0, 0, slot_id, 1, &done, &cc);
        if (done) return (cc == 1 || cc == 13) ? 0 : -2;
        usb_delay_ms(1);
    }
    return -1;
}

int xhci_bulk_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint16_t length, uint8_t direction) {
    (void)direction;
    xhci_ctrl_t *c = &xhci_ctrls[dev->controller_idx];
    uint8_t slot_id = dev->slot_id;
    uint8_t dci = xhci_ep_dci(ep_addr);
    if (dci <= 1 || dci >= 16) return -1;
    xhci_ep_ring_state_t *st = &xhci_ep_state[slot_id][dci];
    if (!st->used) return -1;

    xhci_ring_enqueue(xhci_xfer_rings[slot_id][dci], &st->enq, &st->cycle,
                      (uint32_t)data, 0, (uint32_t)length,
                      (TRB_TYPE_NORMAL << 10) | TRB_IOC | TRB_ISP);

    int done = 0, cc = 0;
    xhci_ring_db(c, slot_id, dci);
    for (uint32_t t = 0; t < 1000; t++) {
        xhci_process_events(c, 0, 0, 0, slot_id, dci, &done, &cc);
        if (done) return (cc == 1 || cc == 13) ? 0 : -2;
        usb_delay_ms(1);
    }
    return -1;
}

int xhci_interrupt_transfer(usb_device_t *dev, uint8_t ep_addr, void *data, uint8_t length) {
    xhci_ctrl_t *c = &xhci_ctrls[dev->controller_idx];
    uint8_t slot_id = dev->slot_id;
    uint8_t dci = xhci_ep_dci(ep_addr);
    if (dci <= 1 || dci >= 16 || length == 0 || length > 64) return -1;
    xhci_ep_ring_state_t *st = &xhci_ep_state[slot_id][dci];
    if (!st->used) return -1;

    xhci_process_events(c, 0, 0, 0, 0, 0, 0, 0);

    if (st->ready) {
        st->ready = 0;
        int ret = st->status;
        if (ret == 0) {
            uint8_t *dst = (uint8_t *)data;
            for (uint8_t i = 0; i < length; i++) dst[i] = st->buf[i];
        }
        st->armed = 1;
        xhci_ring_enqueue(xhci_xfer_rings[slot_id][dci], &st->enq, &st->cycle,
                          (uint32_t)st->buf, 0, (uint32_t)length,
                          (TRB_TYPE_NORMAL << 10) | TRB_IOC | TRB_ISP);
        xhci_ring_db(c, slot_id, dci);
        return ret;
    }

    if (!st->armed) {
        st->armed = 1;
        xhci_ring_enqueue(xhci_xfer_rings[slot_id][dci], &st->enq, &st->cycle,
                          (uint32_t)st->buf, 0, (uint32_t)length,
                          (TRB_TYPE_NORMAL << 10) | TRB_IOC | TRB_ISP);
        xhci_ring_db(c, slot_id, dci);
    }
    return -1;
}