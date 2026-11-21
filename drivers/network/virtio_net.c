/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Sterownik virtio-net (virtio 1.x, interfejs "modern" przez PCI
 * capabilities). Dziala w trybie pollingu, bez przerwan.
 * Obsluguje urzadzenie modern (0x1041) oraz transitional (0x1000),
 * w tym drugim przypadku korzysta tylko z czesci modern.
 * Zobacz drivers/network/virtio_net.h dla opisu funkcji.
 */

#include <drivers/network/virtio_net.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define VIRTIO_PCI_VENDOR        0x1AF4U
#define VIRTIO_NET_DEV_TRANS     0x1000U   /* transitional, subsystem 1 = net */
#define VIRTIO_NET_DEV_MODERN    0x1041U
#define VIRTIO_SUBSYS_NET        1U

/* Typy capabilities virtio. */
#define VIRTIO_PCI_CAP_COMMON_CFG 1U
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2U
#define VIRTIO_PCI_CAP_ISR_CFG    3U
#define VIRTIO_PCI_CAP_DEVICE_CFG 4U

/* Offsety struktury common config. */
#define CC_DEV_FEAT_SEL   0x00
#define CC_DEV_FEAT       0x04
#define CC_DRV_FEAT_SEL   0x08
#define CC_DRV_FEAT       0x0C
#define CC_MSIX_CONFIG    0x10
#define CC_NUM_QUEUES     0x12
#define CC_STATUS         0x14
#define CC_QUEUE_SELECT   0x16
#define CC_QUEUE_SIZE     0x18
#define CC_QUEUE_MSIX     0x1A
#define CC_QUEUE_ENABLE   0x1C
#define CC_QUEUE_NOTIFY   0x1E
#define CC_QUEUE_DESC     0x20
#define CC_QUEUE_DRIVER   0x28
#define CC_QUEUE_DEVICE   0x30

/* Bity statusu urzadzenia. */
#define ST_ACKNOWLEDGE 0x01U
#define ST_DRIVER      0x02U
#define ST_DRIVER_OK   0x04U
#define ST_FEATURES_OK 0x08U
#define ST_FAILED      0x80U

/* Feature'y. */
#define VIRTIO_NET_F_MAC  5U            /* bit w slowie 0 */
#define VIRTIO_NET_F_MTU  3U            /* bit 3 w slowie 0 */
#define VIRTIO_F_VERSION_1_BIT 0U       /* bit 32 = bit 0 w slowie 1 */

#define VRING_DESC_F_WRITE 2U
#define VRING_AVAIL_F_NO_INTERRUPT 1U

#define VQ_SIZE          64
#define VIRTIO_BUF_SIZE  2048
#define VIRTIO_HDR_SIZE  12      /* virtio_net_hdr z num_buffers (VERSION_1) */
#define VIRTIO_MAX_FRAME 1518
#define MIN_FRAME        60

/* Pozycje w konfiguracji urzadzenia virtio-net. */
#define NETCFG_MAC  0
#define NETCFG_MTU  10

struct virtio_net_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    uint16_t num_buffers;
} __attribute__((packed));

struct vring_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct vring_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[VQ_SIZE];
    uint16_t used_event;
} __attribute__((packed));

struct vring_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct vring_used {
    uint16_t flags;
    uint16_t idx;
    struct vring_used_elem ring[VQ_SIZE];
    uint16_t avail_event;
} __attribute__((packed));

struct virtio_queue {
    volatile struct vring_desc *desc;
    volatile struct vring_avail *avail;
    volatile struct vring_used *used;
    uint8_t (*buf)[VIRTIO_BUF_SIZE];
    uint16_t index;
    uint16_t size;
    uint16_t avail_idx;     /* liczone swobodnie modulo 65536 */
    uint16_t last_used;     /* j.w. */
    uint16_t notify_off;
    uint8_t  free_slot[VQ_SIZE];
};

/* Pamiec kolejek i buforow (statyczna, wyrownana). */
static struct vring_desc  rx_desc[VQ_SIZE]  __attribute__((aligned(16)));
static struct vring_avail rx_avail          __attribute__((aligned(16)));
static struct vring_used  rx_used           __attribute__((aligned(16)));
static struct vring_desc  tx_desc[VQ_SIZE]  __attribute__((aligned(16)));
static struct vring_avail tx_avail          __attribute__((aligned(16)));
static struct vring_used  tx_used           __attribute__((aligned(16)));
static uint8_t rx_buf[VQ_SIZE][VIRTIO_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buf[VQ_SIZE][VIRTIO_BUF_SIZE] __attribute__((aligned(16)));

static struct virtio_queue rx_vq;
static struct virtio_queue tx_vq;

static uint8_t pci_bus;
static uint8_t pci_slot;

static volatile uint8_t *common_cfg;
static volatile uint8_t *notify_base;
static volatile uint8_t *device_cfg;
static uint32_t notify_mult;

static uint8_t  device_mac[6];
static uint16_t device_mtu = 1500;
static uint8_t  ready;

static struct virtio_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_dropped;
    uint32_t rx_dropped;
} stats;

#define barrier() __asm__ volatile("" : : : "memory")

/* ------------------------------------------------------------------ */
/* I/O i PCI.                                                         */
/* ------------------------------------------------------------------ */

static void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                        uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

/* Dostep MMIO. */
static uint8_t r8(volatile uint8_t *base, uint32_t off)
{
    return *(volatile uint8_t *)(base + off);
}
static uint16_t r16(volatile uint8_t *base, uint32_t off)
{
    return *(volatile uint16_t *)(base + off);
}
static uint32_t r32(volatile uint8_t *base, uint32_t off)
{
    return *(volatile uint32_t *)(base + off);
}
static void w8(volatile uint8_t *base, uint32_t off, uint8_t v)
{
    *(volatile uint8_t *)(base + off) = v;
}
static void w16(volatile uint8_t *base, uint32_t off, uint16_t v)
{
    *(volatile uint16_t *)(base + off) = v;
}
static void w32(volatile uint8_t *base, uint32_t off, uint32_t v)
{
    *(volatile uint32_t *)(base + off) = v;
}

static uint64_t phys(const volatile void *p)
{
    /* zakladamy mapowanie identycznosciowe (adres wirtualny = fizyczny) */
    return (uint64_t)(unsigned long)p;
}

/* ------------------------------------------------------------------ */
/* Wykrywanie urzadzenia i capabilities.                              */
/* ------------------------------------------------------------------ */

/* Mapuje BAR o numerze bar i zwraca jego adres bazowy. */
static volatile uint8_t *map_bar(uint8_t bus, uint8_t slot, uint8_t bar)
{
    uint32_t lo, hi = 0;
    uint64_t addr;

    if (bar > 5) return 0;
    lo = pci_read32(bus, slot, 0, (uint8_t)(0x10 + bar * 4));
    if (lo & 1U) return 0;                       /* BAR I/O - nie dla modern */
    if ((lo & 6U) == 4U) {                       /* BAR 64-bitowy */
        if (bar == 5) return 0;
        hi = pci_read32(bus, slot, 0, (uint8_t)(0x10 + bar * 4 + 4));
    }
    if (hi && sizeof(unsigned long) == 4) return 0;   /* poza zasiegiem 32-bit */

    addr = ((uint64_t)hi << 32) | (uint64_t)(lo & 0xFFFFFFF0U);
    if (!addr) return 0;
    return (volatile uint8_t *)(unsigned long)addr;
}

/* Przechodzi liste capabilities w przestrzeni konfiguracyjnej PCI. */
static int virtio_parse_caps(uint8_t bus, uint8_t slot)
{
    uint32_t status = pci_read32(bus, slot, 0, 0x04) >> 16;
    uint8_t ptr;
    uint32_t guard = 48;
    int have_common = 0, have_notify = 0, have_device = 0;

    if (!(status & 0x10U)) return 0;             /* brak listy capabilities */
    ptr = (uint8_t)(pci_read32(bus, slot, 0, 0x34) & 0xFC);

    while (ptr >= 0x40 && guard--) {
        uint32_t d0 = pci_read32(bus, slot, 0, ptr);
        uint8_t id   = (uint8_t)(d0 & 0xFF);
        uint8_t next = (uint8_t)((d0 >> 8) & 0xFF);
        uint8_t len  = (uint8_t)((d0 >> 16) & 0xFF);
        uint8_t type = (uint8_t)(d0 >> 24);

        if (id == 0x09 && len >= 16) {           /* vendor specific = virtio */
            uint32_t d1 = pci_read32(bus, slot, 0, (uint8_t)(ptr + 4));
            uint32_t off = pci_read32(bus, slot, 0, (uint8_t)(ptr + 8));
            uint8_t bar = (uint8_t)(d1 & 0xFF);
            volatile uint8_t *base;

            if (type == VIRTIO_PCI_CAP_COMMON_CFG && !have_common) {
                base = map_bar(bus, slot, bar);
                if (base) { common_cfg = base + off; have_common = 1; }
            } else if (type == VIRTIO_PCI_CAP_NOTIFY_CFG && !have_notify && len >= 20) {
                base = map_bar(bus, slot, bar);
                if (base) {
                    notify_base = base + off;
                    notify_mult = pci_read32(bus, slot, 0, (uint8_t)(ptr + 16));
                    have_notify = 1;
                }
            } else if (type == VIRTIO_PCI_CAP_DEVICE_CFG && !have_device) {
                base = map_bar(bus, slot, bar);
                if (base) { device_cfg = base + off; have_device = 1; }
            }
        }
        ptr = (uint8_t)(next & 0xFC);
    }
    return have_common && have_notify && have_device;
}

static int virtio_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);

            if (vendor == 0xFFFF) continue;
            if (vendor != VIRTIO_PCI_VENDOR) continue;

            if (device == VIRTIO_NET_DEV_TRANS) {
                uint32_t sub = pci_read32((uint8_t)bus, slot, 0, 0x2C);
                if ((sub >> 16) != VIRTIO_SUBSYS_NET) continue;
            } else if (device != VIRTIO_NET_DEV_MODERN) {
                continue;
            }

            /* pamiec + bus master + wylaczenie INTx; nie ruszamy bitow statusu */
            uint32_t cmd = pci_read32((uint8_t)bus, slot, 0, 0x04) & 0xFFFFU;
            pci_write32((uint8_t)bus, slot, 0, 0x04, cmd | 0x0406U);

            if (!virtio_parse_caps((uint8_t)bus, slot)) continue;

            pci_bus = (uint8_t)bus;
            pci_slot = slot;
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Status urzadzenia.                                                 */
/* ------------------------------------------------------------------ */

static void add_status(uint8_t bits)
{
    w8(common_cfg, CC_STATUS, (uint8_t)(r8(common_cfg, CC_STATUS) | bits));
}

static int virtio_reset(void)
{
    uint32_t t;
    w8(common_cfg, CC_STATUS, 0);
    for (t = 0; t < 1000000U; t++) {
        if (r8(common_cfg, CC_STATUS) == 0) return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Virtqueue.                                                         */
/* ------------------------------------------------------------------ */

static int vq_setup(struct virtio_queue *q, uint16_t index,
                    struct vring_desc *d, struct vring_avail *a,
                    struct vring_used *u, uint8_t (*buf)[VIRTIO_BUF_SIZE])
{
    uint16_t max, size;
    uint64_t pa;
    volatile uint8_t *z;
    uint32_t i;

    w16(common_cfg, CC_QUEUE_SELECT, index);
    max = r16(common_cfg, CC_QUEUE_SIZE);
    if (max == 0) return 0;                      /* kolejka nie istnieje */
    size = max < VQ_SIZE ? max : VQ_SIZE;
    w16(common_cfg, CC_QUEUE_SIZE, size);
    w16(common_cfg, CC_QUEUE_MSIX, 0xFFFF);      /* bez MSI-X */

    q->desc = d;
    q->avail = a;
    q->used = u;
    q->buf = buf;
    q->index = index;
    q->size = size;
    q->avail_idx = 0;
    q->last_used = 0;

    z = (volatile uint8_t *)d;
    for (i = 0; i < sizeof(struct vring_desc) * VQ_SIZE; i++) z[i] = 0;
    z = (volatile uint8_t *)a;
    for (i = 0; i < sizeof(struct vring_avail); i++) z[i] = 0;
    z = (volatile uint8_t *)u;
    for (i = 0; i < sizeof(struct vring_used); i++) z[i] = 0;
    for (i = 0; i < VQ_SIZE; i++) q->free_slot[i] = 1;

    a->flags = VRING_AVAIL_F_NO_INTERRUPT;       /* polling */
    barrier();

    pa = phys(d);
    w32(common_cfg, CC_QUEUE_DESC, (uint32_t)pa);
    w32(common_cfg, CC_QUEUE_DESC + 4, (uint32_t)(pa >> 32));
    pa = phys(a);
    w32(common_cfg, CC_QUEUE_DRIVER, (uint32_t)pa);
    w32(common_cfg, CC_QUEUE_DRIVER + 4, (uint32_t)(pa >> 32));
    pa = phys(u);
    w32(common_cfg, CC_QUEUE_DEVICE, (uint32_t)pa);
    w32(common_cfg, CC_QUEUE_DEVICE + 4, (uint32_t)(pa >> 32));

    q->notify_off = r16(common_cfg, CC_QUEUE_NOTIFY);
    w16(common_cfg, CC_QUEUE_ENABLE, 1);
    return 1;
}

static void vq_notify(const struct virtio_queue *q)
{
    w16(notify_base, (uint32_t)q->notify_off * notify_mult, q->index);
}

/* Wstawia deskryptor do pierscienia avail (bez powiadamiania). */
static void vq_push(struct virtio_queue *q, uint16_t desc_id)
{
    q->avail->ring[q->avail_idx % q->size] = desc_id;
    barrier();
    q->avail_idx++;
    q->avail->idx = q->avail_idx;
    barrier();
}

/* Zwalnia deskryptory TX, ktore urzadzenie juz zuzylo. */
static void tx_reap(void)
{
    while (tx_vq.last_used != tx_vq.used->idx) {
        uint32_t id;
        barrier();
        id = tx_vq.used->ring[tx_vq.last_used % tx_vq.size].id;
        if (id < tx_vq.size) tx_vq.free_slot[id] = 1;
        tx_vq.last_used++;
    }
}

static void rx_recycle(uint16_t id)
{
    /* deskryptor zostaje bez zmian - tylko oddajemy go urzadzeniu */
    vq_push(&rx_vq, id);
    vq_notify(&rx_vq);
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int virtio_net_init(void)
{
    uint32_t f0, f1, drv0, drv1, i;

    if (ready) return 1;
    if (!virtio_find_device()) return 0;

    if (!virtio_reset()) return 0;
    add_status(ST_ACKNOWLEDGE);
    add_status(ST_DRIVER);

    /* Negocjacja feature'ow: VERSION_1 jest obowiazkowe. */
    w32(common_cfg, CC_DEV_FEAT_SEL, 0);
    f0 = r32(common_cfg, CC_DEV_FEAT);
    w32(common_cfg, CC_DEV_FEAT_SEL, 1);
    f1 = r32(common_cfg, CC_DEV_FEAT);

    if (!(f1 & (1U << VIRTIO_F_VERSION_1_BIT))) {
        add_status(ST_FAILED);
        return 0;
    }

    drv0 = f0 & ((1U << VIRTIO_NET_F_MAC) | (1U << VIRTIO_NET_F_MTU));
    drv1 = 1U << VIRTIO_F_VERSION_1_BIT;

    w32(common_cfg, CC_DRV_FEAT_SEL, 0);
    w32(common_cfg, CC_DRV_FEAT, drv0);
    w32(common_cfg, CC_DRV_FEAT_SEL, 1);
    w32(common_cfg, CC_DRV_FEAT, drv1);

    add_status(ST_FEATURES_OK);
    if (!(r8(common_cfg, CC_STATUS) & ST_FEATURES_OK)) {
        add_status(ST_FAILED);                   /* urzadzenie nie zaakceptowalo */
        return 0;
    }

    /* MAC i MTU z konfiguracji urzadzenia. */
    if (drv0 & (1U << VIRTIO_NET_F_MAC)) {
        for (i = 0; i < 6; i++) device_mac[i] = r8(device_cfg, NETCFG_MAC + i);
    } else {
        static const uint8_t fallback[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
        for (i = 0; i < 6; i++) {
            device_mac[i] = fallback[i];
            w8(device_cfg, NETCFG_MAC + i, fallback[i]);
        }
    }
    device_mtu = 1500;
    if (drv0 & (1U << VIRTIO_NET_F_MTU)) {
        uint16_t m = r16(device_cfg, NETCFG_MTU);
        if (m >= 68 && m <= 1500) device_mtu = m;
    }

    /* Kolejki: 0 = RX, 1 = TX. */
    if (!vq_setup(&rx_vq, 0, rx_desc, &rx_avail, &rx_used, rx_buf)) {
        add_status(ST_FAILED);
        return 0;
    }
    if (!vq_setup(&tx_vq, 1, tx_desc, &tx_avail, &tx_used, tx_buf)) {
        add_status(ST_FAILED);
        return 0;
    }

    /* Bufory odbiorcze: deskryptor i = bufor i, urzadzenie pisze. */
    for (i = 0; i < rx_vq.size; i++) {
        rx_vq.desc[i].addr = phys(&rx_buf[i][0]);
        rx_vq.desc[i].len = VIRTIO_BUF_SIZE;
        rx_vq.desc[i].flags = VRING_DESC_F_WRITE;
        rx_vq.desc[i].next = 0;
        rx_vq.avail->ring[i] = (uint16_t)i;
    }
    barrier();
    rx_vq.avail_idx = rx_vq.size;
    rx_vq.avail->idx = rx_vq.avail_idx;
    barrier();

    add_status(ST_DRIVER_OK);
    vq_notify(&rx_vq);

    ready = 1;
    return 1;
}

int virtio_net_is_ready(void)
{
    return ready;
}

int virtio_net_get_mac(uint8_t mac[6])
{
    if (!ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int virtio_net_send(const void *frame, uint16_t length)
{
    const uint8_t *src = (const uint8_t *)frame;
    uint32_t t, i, send_len;
    uint16_t slot = 0xFFFF;
    uint8_t *buf;

    if (!ready || !frame || length == 0 || length > VIRTIO_MAX_FRAME) return -1;

    /* Znajdz wolny deskryptor (w razie potrzeby poczekaj na urzadzenie). */
    for (t = 0; t < 5000000U && slot == 0xFFFF; t++) {
        tx_reap();
        for (i = 0; i < tx_vq.size; i++) {
            if (tx_vq.free_slot[i]) { slot = (uint16_t)i; break; }
        }
    }
    if (slot == 0xFFFF) {
        stats.tx_dropped++;
        return 0;
    }
    tx_vq.free_slot[slot] = 0;

    buf = &tx_buf[slot][0];

    for (i = 0; i < VIRTIO_HDR_SIZE; i++) buf[i] = 0;   /* naglowek bez offloadow */
    for (i = 0; i < length; i++) buf[VIRTIO_HDR_SIZE + i] = src[i];
    send_len = length;
    while (send_len < MIN_FRAME) buf[VIRTIO_HDR_SIZE + send_len++] = 0;

    tx_vq.desc[slot].addr = phys(buf);
    tx_vq.desc[slot].len = VIRTIO_HDR_SIZE + send_len;
    tx_vq.desc[slot].flags = 0;
    tx_vq.desc[slot].next = 0;
    barrier();

    vq_push(&tx_vq, slot);
    vq_notify(&tx_vq);

    stats.tx_packets++;
    return 1;
}

int virtio_net_receive(void *frame, uint16_t capacity)
{
    uint8_t *dst = (uint8_t *)frame;
    uint32_t tries;

    if (!ready || !frame || capacity == 0) return -1;

    for (tries = 0; tries < rx_vq.size; tries++) {
        uint32_t id, len, frame_len, i;

        if (rx_vq.last_used == rx_vq.used->idx) return 0;   /* nic nowego */
        barrier();

        id = rx_vq.used->ring[rx_vq.last_used % rx_vq.size].id;
        len = rx_vq.used->ring[rx_vq.last_used % rx_vq.size].len;
        rx_vq.last_used++;

        if (id >= rx_vq.size) {
            stats.rx_errors++;
            continue;
        }

        if (len <= VIRTIO_HDR_SIZE) {
            stats.rx_errors++;
            rx_recycle((uint16_t)id);
            continue;
        }

        frame_len = len - VIRTIO_HDR_SIZE;
        if (frame_len > VIRTIO_MAX_FRAME || frame_len > capacity) {
            stats.rx_dropped++;
            rx_recycle((uint16_t)id);
            continue;
        }

        for (i = 0; i < frame_len; i++) dst[i] = rx_buf[id][VIRTIO_HDR_SIZE + i];

        rx_recycle((uint16_t)id);
        stats.rx_packets++;
        return (int)frame_len;
    }
    return 0;
}

void virtio_net_shutdown(void)
{
    if (!ready) return;
    virtio_reset();
    ready = 0;
}

int virtio_net_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                         uint32_t *tx_errors, uint32_t *rx_errors,
                         uint32_t *tx_dropped, uint32_t *rx_dropped)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = stats.tx_packets;
    *rx_packets = stats.rx_packets;
    *tx_errors = stats.tx_errors;
    *rx_errors = stats.rx_errors;
    if (tx_dropped) *tx_dropped = stats.tx_dropped;
    if (rx_dropped) *rx_dropped = stats.rx_dropped;
    return 1;
}

void virtio_net_stats_reset(void)
{
    stats.tx_packets = 0;
    stats.rx_packets = 0;
    stats.tx_errors = 0;
    stats.rx_errors = 0;
    stats.tx_dropped = 0;
    stats.rx_dropped = 0;
}

int virtio_net_set_mtu(uint16_t mtu)
{
    if (!ready || mtu < 68 || mtu > 1500) return 0;
    device_mtu = mtu;
    return 1;
}

uint16_t virtio_net_get_mtu(void)
{
    return device_mtu;
}