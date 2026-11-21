/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Sterownik virtio-net w trybie LEGACY (virtio 0.9.5 po PCI, rejestry w
 * przestrzeni I/O BAR0). Polling, bez przerwan.
 * Dziala z urzadzeniem transitional (0x1000), np. domyslne virtio-net-pci
 * w QEMU. Zobacz drivers/network/virtio_net_legacy.h dla opisu funkcji.
 */

#include <drivers/network/virtio_net_legacy.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define VIRTIO_PCI_VENDOR     0x1AF4U
#define VIRTIO_NET_DEV_LEGACY 0x1000U
#define VIRTIO_SUBSYS_NET     1U

/* Rejestry legacy (offsety od bazy I/O). */
#define VIO_HOST_FEATURES  0x00
#define VIO_GUEST_FEATURES 0x04
#define VIO_QUEUE_PFN      0x08
#define VIO_QUEUE_SIZE     0x0C
#define VIO_QUEUE_SELECT   0x0E
#define VIO_QUEUE_NOTIFY   0x10
#define VIO_STATUS         0x12
#define VIO_ISR            0x13
#define VIO_CONFIG         0x14   /* konfiguracja virtio-net (bez MSI-X) */

/* Bity statusu. */
#define ST_ACKNOWLEDGE 0x01U
#define ST_DRIVER      0x02U
#define ST_DRIVER_OK   0x04U
#define ST_FAILED      0x80U

/* Feature'y (legacy, 32 bity). */
#define VIRTIO_NET_F_MAC     5U
#define VIRTIO_F_ANY_LAYOUT 27U

#define VRING_DESC_F_WRITE         2U
#define VRING_AVAIL_F_NO_INTERRUPT 1U
#define VRING_USED_F_NO_NOTIFY     1U

#define VQ_MAX_SIZE      1024          /* najwiekszy rozmiar kolejki, jaki obslugujemy */
#define VQ_MEM_SIZE      32768         /* wystarcza na kolejke 1024 elementow */
#define RX_BUFS          64
#define TX_BUFS          32
#define VIRTIO_BUF_SIZE  2048
#define VIRTIO_HDR_SIZE  10            /* virtio_net_hdr bez num_buffers */
#define VIRTIO_MAX_FRAME 1518
#define MIN_FRAME        60

struct virtio_net_hdr {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
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
    uint16_t ring[];
} __attribute__((packed));

struct vring_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

struct vring_used {
    uint16_t flags;
    uint16_t idx;
    struct vring_used_elem ring[];
} __attribute__((packed));

struct vq {
    volatile struct vring_desc *desc;
    volatile struct vring_avail *avail;
    volatile struct vring_used *used;
    uint16_t index;
    uint16_t size;        /* rozmiar kolejki podany przez urzadzenie */
    uint16_t nbuf;        /* ile deskryptorow/buforow uzywamy */
    uint16_t avail_idx;   /* liczone swobodnie modulo 65536 */
    uint16_t last_used;   /* j.w. */
    uint8_t  free_slot[RX_BUFS > TX_BUFS ? RX_BUFS : TX_BUFS];
};

/* Pamiec kolejek (legacy: ciagly blok wyrownany do strony) i bufory. */
static uint8_t rx_ring_mem[VQ_MEM_SIZE] __attribute__((aligned(4096)));
static uint8_t tx_ring_mem[VQ_MEM_SIZE] __attribute__((aligned(4096)));
static uint8_t rx_buf[RX_BUFS][VIRTIO_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buf[TX_BUFS][VIRTIO_BUF_SIZE] __attribute__((aligned(16)));

static struct vq rx_vq;
static struct vq tx_vq;

static uint16_t io_base;
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

static void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

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

static uint32_t phys(const volatile void *p)
{
    /* zakladamy mapowanie identycznosciowe (adres wirtualny = fizyczny) */
    return (uint32_t)(unsigned long)p;
}

/* ------------------------------------------------------------------ */
/* Wykrywanie urzadzenia i status.                                    */
/* ------------------------------------------------------------------ */

static int vdev_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            if (vendor == 0xFFFF) continue;
            if (vendor != VIRTIO_PCI_VENDOR || device != VIRTIO_NET_DEV_LEGACY) continue;

            uint32_t sub = pci_read32((uint8_t)bus, slot, 0, 0x2C);
            if ((sub >> 16) != VIRTIO_SUBSYS_NET) continue;

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || !(bar0 & 1)) continue;
            io_base = (uint16_t)(bar0 & 0xFFFC);

            /* I/O space + bus master + wylaczenie INTx (polling).
             * Zapisujemy tylko 16 bitow komendy, bez rejestru statusu. */
            uint32_t cmd = pci_read32((uint8_t)bus, slot, 0, 0x04) & 0xFFFFU;
            pci_write32((uint8_t)bus, slot, 0, 0x04, cmd | 0x0405U);
            return 1;
        }
    }
    return 0;
}

static void add_status(uint8_t bits)
{
    outb((uint16_t)(io_base + VIO_STATUS),
         (uint8_t)(inb((uint16_t)(io_base + VIO_STATUS)) | bits));
}

static void vdev_reset(void)
{
    uint32_t t;
    outb((uint16_t)(io_base + VIO_STATUS), 0);
    for (t = 0; t < 1000000U; t++) {
        if (inb((uint16_t)(io_base + VIO_STATUS)) == 0) break;
    }
}

/* ------------------------------------------------------------------ */
/* Virtqueue (legacy).                                                */
/* ------------------------------------------------------------------ */

static int vq_setup(struct vq *q, uint16_t index, uint8_t *mem, uint16_t max_bufs)
{
    uint16_t size;
    uint32_t avail_off, used_off, i;
    uint32_t *z;

    outw((uint16_t)(io_base + VIO_QUEUE_SELECT), index);
    size = inw((uint16_t)(io_base + VIO_QUEUE_SIZE));
    if (size == 0 || size > VQ_MAX_SIZE) return 0;

    z = (uint32_t *)mem;
    for (i = 0; i < VQ_MEM_SIZE / 4; i++) z[i] = 0;

    /* Uklad legacy: desc | avail | (padding do 4096) | used */
    avail_off = 16U * size;
    used_off = (avail_off + 6U + 2U * size + 4095U) & ~4095U;

    q->desc  = (volatile struct vring_desc *)(mem);
    q->avail = (volatile struct vring_avail *)(mem + avail_off);
    q->used  = (volatile struct vring_used *)(mem + used_off);
    q->index = index;
    q->size = size;
    q->nbuf = size < max_bufs ? size : max_bufs;
    q->avail_idx = 0;
    q->last_used = 0;
    for (i = 0; i < q->nbuf; i++) q->free_slot[i] = 1;

    q->avail->flags = VRING_AVAIL_F_NO_INTERRUPT;   /* polling */
    barrier();

    /* Urzadzenie dostaje tylko numer strony bloku kolejki. */
    outl((uint16_t)(io_base + VIO_QUEUE_PFN), phys(mem) >> 12);
    return 1;
}

static void vq_kick(const struct vq *q)
{
    barrier();
    if (!(q->used->flags & VRING_USED_F_NO_NOTIFY))
        outw((uint16_t)(io_base + VIO_QUEUE_NOTIFY), q->index);
}

static void vq_push(struct vq *q, uint16_t desc_id)
{
    q->avail->ring[q->avail_idx % q->size] = desc_id;
    barrier();
    q->avail_idx++;
    q->avail->idx = q->avail_idx;
    barrier();
}

static void tx_reap(void)
{
    while (tx_vq.last_used != tx_vq.used->idx) {
        uint32_t id;
        barrier();
        id = tx_vq.used->ring[tx_vq.last_used % tx_vq.size].id;
        if (id < tx_vq.nbuf) tx_vq.free_slot[id] = 1;
        tx_vq.last_used++;
    }
}

static void rx_recycle(uint16_t id)
{
    vq_push(&rx_vq, id);
    vq_kick(&rx_vq);
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int virtio_net_legacy_init(void)
{
    uint32_t host, guest, i;

    if (ready) return 1;
    if (!vdev_find_device()) return 0;

    vdev_reset();
    add_status(ST_ACKNOWLEDGE);
    add_status(ST_DRIVER);

    /* Negocjacja feature'ow (legacy: jedno 32-bitowe slowo). */
    host = inl((uint16_t)(io_base + VIO_HOST_FEATURES));
    guest = host & ((1U << VIRTIO_NET_F_MAC) | (1U << VIRTIO_F_ANY_LAYOUT));
    outl((uint16_t)(io_base + VIO_GUEST_FEATURES), guest);

    /* MAC z konfiguracji urzadzenia (I/O pod 0x14). */
    if (guest & (1U << VIRTIO_NET_F_MAC)) {
        for (i = 0; i < 6; i++)
            device_mac[i] = inb((uint16_t)(io_base + VIO_CONFIG + i));
    } else {
        static const uint8_t fallback[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
        for (i = 0; i < 6; i++) device_mac[i] = fallback[i];
    }
    device_mtu = 1500;

    /* Kolejki: 0 = RX, 1 = TX. */
    if (!vq_setup(&rx_vq, 0, rx_ring_mem, RX_BUFS) ||
        !vq_setup(&tx_vq, 1, tx_ring_mem, TX_BUFS)) {
        add_status(ST_FAILED);
        return 0;
    }

    /* Bufory odbiorcze: deskryptor i = bufor i, urzadzenie pisze. */
    for (i = 0; i < rx_vq.nbuf; i++) {
        rx_vq.desc[i].addr = (uint64_t)phys(&rx_buf[i][0]);
        rx_vq.desc[i].len = VIRTIO_BUF_SIZE;
        rx_vq.desc[i].flags = VRING_DESC_F_WRITE;
        rx_vq.desc[i].next = 0;
        rx_vq.avail->ring[i] = (uint16_t)i;
    }
    barrier();
    rx_vq.avail_idx = rx_vq.nbuf;
    rx_vq.avail->idx = rx_vq.avail_idx;
    barrier();

    add_status(ST_DRIVER_OK);
    outw((uint16_t)(io_base + VIO_QUEUE_NOTIFY), 0);   /* RX ma bufory */

    ready = 1;
    return 1;
}

int virtio_net_legacy_is_ready(void)
{
    return ready;
}

int virtio_net_legacy_get_mac(uint8_t mac[6])
{
    if (!ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int virtio_net_legacy_send(const void *frame, uint16_t length)
{
    const uint8_t *src = (const uint8_t *)frame;
    uint32_t t, i, send_len;
    uint16_t slot = 0xFFFF;
    uint8_t *buf;

    if (!ready || !frame || length == 0 || length > VIRTIO_MAX_FRAME) return -1;

    /* Znajdz wolny bufor (w razie potrzeby poczekaj na urzadzenie). */
    for (t = 0; t < 5000000U && slot == 0xFFFF; t++) {
        tx_reap();
        for (i = 0; i < tx_vq.nbuf; i++) {
            if (tx_vq.free_slot[i]) { slot = (uint16_t)i; break; }
        }
    }
    if (slot == 0xFFFF) {
        stats.tx_dropped++;
        return 0;
    }
    tx_vq.free_slot[slot] = 0;

    buf = &tx_buf[slot][0];

    for (i = 0; i < VIRTIO_HDR_SIZE; i++) buf[i] = 0;        /* naglowek bez offloadow */
    for (i = 0; i < length; i++) buf[VIRTIO_HDR_SIZE + i] = src[i];
    send_len = length;
    while (send_len < MIN_FRAME) buf[VIRTIO_HDR_SIZE + send_len++] = 0;

    tx_vq.desc[slot].addr = (uint64_t)phys(buf);
    tx_vq.desc[slot].len = VIRTIO_HDR_SIZE + send_len;
    tx_vq.desc[slot].flags = 0;
    tx_vq.desc[slot].next = 0;
    barrier();

    vq_push(&tx_vq, slot);
    vq_kick(&tx_vq);

    stats.tx_packets++;
    return 1;
}

int virtio_net_legacy_receive(void *frame, uint16_t capacity)
{
    uint8_t *dst = (uint8_t *)frame;
    uint32_t tries;

    if (!ready || !frame || capacity == 0) return -1;

    for (tries = 0; tries < rx_vq.nbuf; tries++) {
        uint32_t id, len, frame_len, i;

        if (rx_vq.last_used == rx_vq.used->idx) return 0;    /* nic nowego */
        barrier();

        id = rx_vq.used->ring[rx_vq.last_used % rx_vq.size].id;
        len = rx_vq.used->ring[rx_vq.last_used % rx_vq.size].len;
        rx_vq.last_used++;

        if (id >= rx_vq.nbuf) {
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

void virtio_net_legacy_shutdown(void)
{
    if (!ready) return;
    vdev_reset();
    ready = 0;
}

int virtio_net_legacy_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
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

void virtio_net_legacy_stats_reset(void)
{
    stats.tx_packets = 0;
    stats.rx_packets = 0;
    stats.tx_errors = 0;
    stats.rx_errors = 0;
    stats.tx_dropped = 0;
    stats.rx_dropped = 0;
}

int virtio_net_legacy_set_mtu(uint16_t mtu)
{
    if (!ready || mtu < 68 || mtu > 1500) return 0;
    device_mtu = mtu;
    return 1;
}

uint16_t virtio_net_legacy_get_mtu(void)
{
    return device_mtu;
}