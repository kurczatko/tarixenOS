/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Sterownik Sun GEM (Gigabit Ethernet MAC), PCI vendor 0x108E.
 * Rejestry i sekwencja inicjalizacji wzorowane na sungem.h / sungem.c z Linuksa.
 * Rejestry MMIO w BAR0, polling, bez przerwan, PHY MII przez MIF.
 * Zobacz drivers/network/sun_gem.h dla opisu funkcji.
 */

#include <drivers/network/sun_gem.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define SUN_GEM_VENDOR         0x108EU
#define SUN_GEM_DEVICE_GEM     0x2BADU
#define SUN_GEM_DEVICE_RIO_GEM 0x1101U

/* Rejestry globalne. */
#define GREG_IMASK   0x0010U
#define GREG_SWRST   0x1010U
#define GREG_SWRST_TXRST 0x00000001U
#define GREG_SWRST_RXRST 0x00000002U

/* TX DMA. */
#define TXDMA_KICK   0x2000U
#define TXDMA_CFG    0x2004U
#define TXDMA_DBLOW  0x2008U
#define TXDMA_DBHI   0x200CU
#define TXDMA_TXDONE 0x2100U

#define TXDMA_CFG_ENABLE   0x00000001U
#define TXDMA_CFG_RINGSZ_128 0x00000004U
#define TXDMA_CFG_FTHRESH  (0x7ffU << 10)
#define TXDMA_CFG_PMODE    0x01000000U

/* RX DMA. */
#define RXDMA_CFG    0x4000U
#define RXDMA_DBLOW  0x4004U
#define RXDMA_DBHI   0x4008U
#define RXDMA_KICK   0x4100U

#define RXDMA_CFG_ENABLE     0x00000001U
#define RXDMA_CFG_RINGSZ_128 0x00000004U
#define RXDMA_CFG_FTHRESH_128 0x01000000U

/* MAC. */
#define MAC_SNDPAUSE 0x6008U
#define MAC_TXMASK   0x6020U
#define MAC_RXMASK   0x6024U
#define MAC_MCMASK   0x6028U
#define MAC_TXCFG    0x6030U
#define MAC_RXCFG    0x6034U
#define MAC_MCCFG    0x6038U
#define MAC_XIFCFG   0x603CU
#define MAC_IPG0     0x6040U
#define MAC_IPG1     0x6044U
#define MAC_IPG2     0x6048U
#define MAC_STIME    0x604CU
#define MAC_MINFSZ   0x6050U
#define MAC_MAXFSZ   0x6054U
#define MAC_PASIZE   0x6058U
#define MAC_JAMSIZE  0x605CU
#define MAC_ATTLIM   0x6060U
#define MAC_MCTYPE   0x6064U
#define MAC_ADDR0    0x6080U
#define MAC_ADDR1    0x6084U
#define MAC_ADDR2    0x6088U
#define MAC_ADDR3    0x608CU   /* ADDR3-5: adresy alternatywne, zerujemy */
#define MAC_ADDR6    0x6098U   /* ADDR6-8: adres ramek PAUSE */
#define MAC_ADDR_FILTER0 0x60A4U
#define MAC_HASH0    0x60C0U   /* HASH0..HASH15 */

#define MAC_TXCFG_ENAB 0x00000001U
#define MAC_TXCFG_ICS  0x00000002U
#define MAC_TXCFG_ICOLL 0x00000004U

#define MAC_RXCFG_ENAB 0x00000001U
#define MAC_RXCFG_SFCS 0x00000004U   /* odetnij FCS */

#define MAC_XIFCFG_OE   0x00000001U
#define MAC_XIFCFG_DISE 0x00000004U
#define MAC_XIFCFG_GMII 0x00000008U

/* MIF (dostep do PHY). */
#define MIF_FRAME 0x620CU
#define MIF_CFG   0x6210U
#define MIF_CFG_PSELECT 0x00000001U
#define MIF_CFG_POLL    0x00000002U
#define MIF_CFG_BBMODE  0x00000004U
#define MIF_CFG_MDI1    0x00000200U
#define MIF_FRAME_TAMSB 0x00020000U
#define MIF_FRAME_TALSB 0x00010000U

/* Rejestry MII. */
#define MII_BMCR      0
#define MII_BMSR      1
#define MII_PHYSID1   2
#define MII_ADVERTISE 4
#define MII_LPA       5
#define MII_CTRL1000  9
#define MII_STAT1000  10

/* Pierscienie i bufory. */
#define RING_SIZE      128U
#define BUF_SIZE       2048U
#define RX_OFFSET      2U
#define RX_FIELD_SIZE  1792U       /* rozmiar bufora zglaszany karcie */
#define SUN_GEM_MAX_FRAME 1518
#define MIN_FRAME      60

struct gem_txd {
    uint64_t control_word;
    uint64_t buffer;
};

struct gem_rxd {
    uint64_t status_word;
    uint64_t buffer;
};

#define TXDCTRL_BUFSZ 0x0000000000007fffULL
#define TXDCTRL_EOF   0x0000000040000000ULL
#define TXDCTRL_SOF   0x0000000080000000ULL

#define RXDCTRL_BUFSZ 0x000000007fff0000ULL
#define RXDCTRL_OWN   0x0000000080000000ULL
#define RXDCTRL_BAD   0x4000000000000000ULL
#define RXDCTRL_FRESH (((uint64_t)RX_FIELD_SIZE << 16) | RXDCTRL_OWN)

struct gem_block {
    struct gem_txd tx[RING_SIZE];
    struct gem_rxd rx[RING_SIZE];
};

static volatile struct gem_block blk __attribute__((aligned(4096)));
static uint8_t tx_buf[RING_SIZE][BUF_SIZE] __attribute__((aligned(64)));
static uint8_t rx_buf[RING_SIZE][BUF_SIZE] __attribute__((aligned(64)));

static volatile uint32_t *mmio_base;
static uint8_t  device_mac[6];
static uint16_t device_mtu = 1500;
static uint8_t  ready;
static uint8_t  link_up;
static uint8_t  phy_addr;
static uint8_t  phy_gig;
static uint32_t rx_cur;
static uint32_t tx_cur;
static uint32_t link_poll_count;

static struct sun_gem_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_dropped;
    uint32_t rx_dropped;
} stats;

#define barrier() __asm__ volatile("" : : : "memory")

/* ------------------------------------------------------------------ */
/* I/O, PCI, MMIO.                                                    */
/* ------------------------------------------------------------------ */

static uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
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

static void wr(uint32_t off, uint32_t value)
{
    mmio_base[off / 4U] = value;
}

static uint32_t rd(uint32_t off)
{
    return mmio_base[off / 4U];
}

static uint32_t phys(const volatile void *p)
{
    /* zakladamy mapowanie identycznosciowe (adres wirtualny = fizyczny) */
    return (uint32_t)(unsigned long)p;
}

/* ok. 1 us na iteracje (odczyt portu 0x84) */
static void delay_us(uint32_t n)
{
    while (n--) inb(0x84);
}

/* ------------------------------------------------------------------ */
/* MII przez MIF.                                                     */
/* ------------------------------------------------------------------ */

static uint16_t mii_read_at(uint8_t addr, uint8_t reg)
{
    uint32_t cmd = (1U << 30) | (2U << 28) | ((uint32_t)addr << 23) |
                   ((uint32_t)reg << 18) | MIF_FRAME_TAMSB;
    uint32_t i;
    wr(MIF_FRAME, cmd);
    for (i = 0; i < 1000; i++) {
        cmd = rd(MIF_FRAME);
        if (cmd & MIF_FRAME_TALSB) return (uint16_t)(cmd & 0xFFFFU);
        delay_us(10);
    }
    return 0xFFFF;
}

static uint16_t mii_read(uint8_t reg)
{
    return mii_read_at(phy_addr, reg);
}

static void mii_write(uint8_t reg, uint16_t value)
{
    uint32_t cmd = (1U << 30) | (1U << 28) | ((uint32_t)phy_addr << 23) |
                   ((uint32_t)reg << 18) | MIF_FRAME_TAMSB | value;
    uint32_t i;
    wr(MIF_FRAME, cmd);
    for (i = 0; i < 1000; i++) {
        if (rd(MIF_FRAME) & MIF_FRAME_TALSB) return;
        delay_us(10);
    }
}

static int gem_phy_init(void)
{
    uint32_t cfg = rd(MIF_CFG);
    uint32_t i;
    int found = 0;

    cfg &= ~(MIF_CFG_POLL | MIF_CFG_BBMODE);
    if (cfg & MIF_CFG_MDI1) cfg |= MIF_CFG_PSELECT;   /* zewnetrzny PHY */
    else cfg &= ~MIF_CFG_PSELECT;                     /* wewnetrzny PHY */
    wr(MIF_CFG, cfg);
    delay_us(100);

    for (i = 0; i < 32; i++) {
        uint16_t id = mii_read_at((uint8_t)i, MII_PHYSID1);
        if (id != 0xFFFFU && id != 0) { phy_addr = (uint8_t)i; found = 1; break; }
    }
    if (!found) return 0;

    mii_write(MII_BMCR, 0x8000);                      /* reset PHY */
    for (i = 0; i < 500; i++) {
        if (!(mii_read(MII_BMCR) & 0x8000U)) break;
        delay_us(1000);
    }

    phy_gig = (mii_read(MII_BMSR) & 0x0100U) ? 1 : 0; /* extended status = gigabit */
    mii_write(MII_ADVERTISE, 0x01E1);                 /* 10/100 HD/FD */
    if (phy_gig) mii_write(MII_CTRL1000, 0x0300);     /* 1000 HD/FD */
    mii_write(MII_BMCR, 0x1200);                      /* autoneg + restart */
    return 1;
}

/* Zwraca 1 gdy lacze wstalo i autonegocjacja sie skonczyla. */
static int gem_link_check(int *speed, int *full)
{
    uint16_t bmsr, adv, lpa, common;

    (void)mii_read(MII_BMSR);                         /* wpis jest zatrzasniety */
    bmsr = mii_read(MII_BMSR);
    if (!(bmsr & 0x0004U) || !(bmsr & 0x0020U)) return 0;

    adv = mii_read(MII_ADVERTISE);
    lpa = mii_read(MII_LPA);
    common = (uint16_t)(adv & lpa);
    *speed = 10;
    *full = 0;

    if (phy_gig) {
        uint16_t g = (uint16_t)(mii_read(MII_CTRL1000) & (mii_read(MII_STAT1000) >> 2));
        if (g & 0x0200U) { *speed = 1000; *full = 1; return 1; }
        if (g & 0x0100U) { *speed = 1000; *full = 0; return 1; }
    }
    if (common & 0x0100U)      { *speed = 100; *full = 1; }
    else if (common & 0x0080U) { *speed = 100; *full = 0; }
    else if (common & 0x0040U) { *speed = 10;  *full = 1; }
    else                       { *speed = 10;  *full = 0; }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Sprzet.                                                            */
/* ------------------------------------------------------------------ */

static int sun_gem_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            if (vendor == 0xFFFF) continue;
            if (vendor != SUN_GEM_VENDOR) continue;
            if (device != SUN_GEM_DEVICE_GEM && device != SUN_GEM_DEVICE_RIO_GEM) continue;

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || (bar0 & 1)) continue;
            mmio_base = (volatile uint32_t *)(unsigned long)(bar0 & 0xFFFFFFF0U);

            /* memory space + bus master + wylaczenie INTx; bez ruszania statusu */
            uint32_t cmd = pci_read32((uint8_t)bus, slot, 0, 0x04) & 0xFFFFU;
            pci_write32((uint8_t)bus, slot, 0, 0x04, cmd | 0x0406U);
            return 1;
        }
    }
    return 0;
}

static void gem_read_mac(void)
{
    uint32_t a0 = rd(MAC_ADDR0), a1 = rd(MAC_ADDR1), a2 = rd(MAC_ADDR2);
    int valid;

    device_mac[0] = (uint8_t)(a2 >> 8);
    device_mac[1] = (uint8_t)a2;
    device_mac[2] = (uint8_t)(a1 >> 8);
    device_mac[3] = (uint8_t)a1;
    device_mac[4] = (uint8_t)(a0 >> 8);
    device_mac[5] = (uint8_t)a0;

    valid = (device_mac[0] | device_mac[1] | device_mac[2] |
             device_mac[3] | device_mac[4] | device_mac[5]) != 0;
    if (device_mac[0] & 1U) valid = 0;                /* multicast */
    if (!valid) {
        device_mac[0] = 0x52; device_mac[1] = 0x54; device_mac[2] = 0x00;
        device_mac[3] = 0x12; device_mac[4] = 0x34; device_mac[5] = 0x57;
    }
}

static int gem_reset(void)
{
    uint32_t t;
    wr(GREG_SWRST, GREG_SWRST_TXRST | GREG_SWRST_RXRST);
    for (t = 0; t < 10000; t++) {
        if (!(rd(GREG_SWRST) & (GREG_SWRST_TXRST | GREG_SWRST_RXRST))) break;
        delay_us(10);
    }
    if (t == 10000) return 0;
    wr(GREG_IMASK, 0xFFFFFFFFU);                      /* polling: wszystko zamaskowane */
    return 1;
}

static void rx_arm(uint32_t i)
{
    blk.rx[i].buffer = (uint64_t)phys(&rx_buf[i][0]);
    barrier();
    blk.rx[i].status_word = RXDCTRL_FRESH;
}

static void gem_init_rings(void)
{
    uint32_t i;
    for (i = 0; i < RING_SIZE; i++) {
        blk.tx[i].control_word = 0;
        blk.tx[i].buffer = (uint64_t)phys(&tx_buf[i][0]);
        rx_arm(i);
    }
    rx_cur = 0;
    tx_cur = 0;
    barrier();
}

static void gem_init_mac(void)
{
    uint32_t i;

    wr(MAC_TXCFG, 0);
    wr(MAC_RXCFG, 0);

    wr(MAC_SNDPAUSE, 0x1bf0);
    wr(MAC_IPG0, 0x00);
    wr(MAC_IPG1, 0x08);
    wr(MAC_IPG2, 0x04);
    wr(MAC_STIME, 0x40);
    wr(MAC_MINFSZ, 0x40);
    wr(MAC_MAXFSZ, 0x20000000U | (SUN_GEM_MAX_FRAME + 4));
    wr(MAC_PASIZE, 0x07);
    wr(MAC_JAMSIZE, 0x04);
    wr(MAC_ATTLIM, 0x10);
    wr(MAC_MCTYPE, 0x8808);
    wr(MAC_MCCFG, 0);

    wr(MAC_ADDR0, ((uint32_t)device_mac[4] << 8) | device_mac[5]);
    wr(MAC_ADDR1, ((uint32_t)device_mac[2] << 8) | device_mac[3]);
    wr(MAC_ADDR2, ((uint32_t)device_mac[0] << 8) | device_mac[1]);
    for (i = 0; i < 3; i++) wr(MAC_ADDR3 + i * 4U, 0);
    wr(MAC_ADDR6, 0x0001);                            /* adres ramek PAUSE */
    wr(MAC_ADDR6 + 4U, 0xc200);
    wr(MAC_ADDR6 + 8U, 0x0180);
    for (i = 0; i < 5; i++) wr(MAC_ADDR_FILTER0 + i * 4U, 0);
    for (i = 0; i < 16; i++) wr(MAC_HASH0 + i * 4U, 0);

    wr(MAC_TXMASK, 0xFFFFFFFFU);
    wr(MAC_RXMASK, 0xFFFFFFFFU);
    wr(MAC_MCMASK, 0xFFFFFFFFU);
}

static void gem_init_dma(void)
{
    wr(TXDMA_DBHI, 0);
    wr(TXDMA_DBLOW, phys(&blk.tx[0]));
    wr(TXDMA_KICK, 0);
    wr(TXDMA_CFG, TXDMA_CFG_RINGSZ_128 | TXDMA_CFG_FTHRESH | TXDMA_CFG_PMODE);

    wr(RXDMA_DBHI, 0);
    wr(RXDMA_DBLOW, phys(&blk.rx[0]));
    wr(RXDMA_CFG, RXDMA_CFG_RINGSZ_128 | (RX_OFFSET << 10) | ((14U / 2U) << 13) |
                  RXDMA_CFG_FTHRESH_128);

    wr(TXDMA_CFG, rd(TXDMA_CFG) | TXDMA_CFG_ENABLE);
    wr(RXDMA_CFG, rd(RXDMA_CFG) | RXDMA_CFG_ENABLE);
}

/* Ustawia MAC pod wynegocjowana predkosc/duplex i wlacza TX/RX. */
static void gem_apply_link(int speed, int full)
{
    uint32_t txcfg = 0, xif = MAC_XIFCFG_OE, t;

    wr(MAC_TXCFG, 0);
    wr(MAC_RXCFG, 0);
    for (t = 0; t < 10000; t++) {
        if (!(rd(MAC_TXCFG) & MAC_TXCFG_ENAB) && !(rd(MAC_RXCFG) & MAC_RXCFG_ENAB)) break;
        delay_us(10);
    }

    if (full) txcfg |= MAC_TXCFG_ICS | MAC_TXCFG_ICOLL;
    if (speed == 1000) xif |= MAC_XIFCFG_GMII;
    if (full) xif |= MAC_XIFCFG_DISE;

    wr(MAC_STIME, speed == 1000 ? 0x200U : 0x40U);
    wr(MAC_XIFCFG, xif);
    wr(MAC_TXCFG, txcfg | MAC_TXCFG_ENAB);
    wr(MAC_RXCFG, MAC_RXCFG_SFCS | MAC_RXCFG_ENAB);

    barrier();
    wr(RXDMA_KICK, RING_SIZE - 4U);                   /* ostatnie 4 wpisy zostaja zapasem */
    link_up = 1;
}

/* Wolane leniwie z send/receive, gdy lacze jeszcze nie wstalo. */
static void gem_link_poll(void)
{
    int speed, full;
    if (link_up) return;
    if ((++link_poll_count & 0x3FFU) != 0) return;
    if (gem_link_check(&speed, &full)) gem_apply_link(speed, full);
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int sun_gem_init(void)
{
    uint32_t i;
    int speed, full;

    if (ready) return 1;
    if (!sun_gem_find_device()) return 0;

    gem_read_mac();                                   /* przed resetem */
    if (!gem_reset()) return 0;
    if (!gem_phy_init()) return 0;

    gem_init_rings();
    gem_init_mac();
    gem_init_dma();

    /* Czekamy na autonegocjacje (do ok. 3 s). */
    link_up = 0;
    for (i = 0; i < 3000; i++) {
        if (gem_link_check(&speed, &full)) { gem_apply_link(speed, full); break; }
        delay_us(1000);
    }

    ready = 1;       /* gdy lacza brak, dokonczy sie leniwie w gem_link_poll() */
    return 1;
}

int sun_gem_is_ready(void)
{
    return ready;
}

int sun_gem_get_mac(uint8_t mac[6])
{
    if (!ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int sun_gem_send(const void *frame, uint16_t length)
{
    const uint8_t *src = (const uint8_t *)frame;
    uint32_t i, t, idx, send_len;

    if (!ready || !frame || length == 0 || length > SUN_GEM_MAX_FRAME) return -1;

    gem_link_poll();
    if (!link_up) { stats.tx_dropped++; return 0; }

    idx = tx_cur;
    for (i = 0; i < length; i++) tx_buf[idx][i] = src[i];
    send_len = length;
    while (send_len < MIN_FRAME) tx_buf[idx][send_len++] = 0;

    blk.tx[idx].buffer = (uint64_t)phys(&tx_buf[idx][0]);
    blk.tx[idx].control_word = ((uint64_t)send_len & TXDCTRL_BUFSZ) | TXDCTRL_SOF | TXDCTRL_EOF;
    barrier();

    tx_cur = (tx_cur + 1U) & (RING_SIZE - 1U);
    wr(TXDMA_KICK, tx_cur);

    for (t = 0; t < 1000000U; t++) {
        if (rd(TXDMA_TXDONE) == tx_cur) break;
    }
    if (t == 1000000U) {
        stats.tx_errors++;
        return 0;
    }

    stats.tx_packets++;
    return 1;
}

int sun_gem_receive(void *frame, uint16_t capacity)
{
    uint8_t *dst = (uint8_t *)frame;
    uint32_t tries;

    if (!ready || !frame || capacity == 0) return -1;

    gem_link_poll();
    if (!link_up) return 0;

    for (tries = 0; tries < RING_SIZE; tries++) {
        uint64_t st = blk.rx[rx_cur].status_word;
        uint32_t len, i;
        int good = 0;

        if (st & RXDCTRL_OWN) return 0;               /* karta jeszcze nic tu nie wpisala */
        barrier();

        len = (uint32_t)((st & RXDCTRL_BUFSZ) >> 16);

        if (st & RXDCTRL_BAD) {
            stats.rx_errors++;
        } else if (len < 14 || len > (uint32_t)SUN_GEM_MAX_FRAME + 4U) {
            stats.rx_errors++;
        } else if (len > capacity) {
            stats.rx_dropped++;
        } else {
            for (i = 0; i < len; i++) dst[i] = rx_buf[rx_cur][RX_OFFSET + i];
            good = 1;
        }

        rx_arm(rx_cur);
        rx_cur = (rx_cur + 1U) & (RING_SIZE - 1U);
        if ((rx_cur & 3U) == 0) {
            barrier();
            wr(RXDMA_KICK, (rx_cur + RING_SIZE - 4U) & (RING_SIZE - 1U));
        }

        if (good) {
            stats.rx_packets++;
            return (int)len;
        }
    }
    return 0;
}

void sun_gem_shutdown(void)
{
    if (!ready) return;
    wr(MAC_TXCFG, 0);
    wr(MAC_RXCFG, 0);
    wr(TXDMA_CFG, rd(TXDMA_CFG) & ~TXDMA_CFG_ENABLE);
    wr(RXDMA_CFG, rd(RXDMA_CFG) & ~RXDMA_CFG_ENABLE);
    wr(GREG_IMASK, 0xFFFFFFFFU);
    link_up = 0;
    ready = 0;
}

int sun_gem_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
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

void sun_gem_stats_reset(void)
{
    stats.tx_packets = 0;
    stats.rx_packets = 0;
    stats.tx_errors = 0;
    stats.rx_errors = 0;
    stats.tx_dropped = 0;
    stats.rx_dropped = 0;
}

int sun_gem_set_mtu(uint16_t mtu)
{
    if (!ready || mtu < 68 || mtu > 1500) return 0;
    device_mtu = mtu;
    return 1;
}

uint16_t sun_gem_get_mtu(void)
{
    return device_mtu;
}