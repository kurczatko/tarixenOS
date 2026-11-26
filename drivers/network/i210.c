/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <drivers/network/i210.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define I210_VENDOR        0x8086U

/* Urzadzenia I210 / I211 z wbudowanym PHY miedzianym. */
#define I210_DEV_I210_COPPER           0x1533U
#define I210_DEV_I210_COPPER_FLASHLESS 0x157BU
#define I210_DEV_I211_COPPER           0x1539U

/* MMIO rejestry I210 (offsety od BAR0). */
#define I210_CTRL          0x0000U
#define I210_STATUS        0x0008U
#define I210_EEC           0x0010U
#define I210_CTRL_EXT      0x0018U
#define I210_MDIC          0x0020U
#define I210_ICR           0x00C0U
#define I210_ITR           0x00C4U
#define I210_IMS           0x00D0U
#define I210_IMC           0x00D8U
#define I210_RCTL          0x0100U
#define I210_TCTL          0x0400U
#define I210_TIPG          0x0410U
#define I210_EIMC          0x1528U
#define I210_RDBAL         0x2800U
#define I210_RDBAH         0x2804U
#define I210_RDLEN         0x2808U
#define I210_SRRCTL        0x280CU
#define I210_RDH           0x2810U
#define I210_RDT           0x2818U
#define I210_RXDCTL        0x2828U
#define I210_RXCSUM        0x5000U
#define I210_RFCTL         0x5008U
#define I210_MTA           0x5200U
#define I210_RAL           0x5400U
#define I210_RAH           0x5404U
#define I210_TDBAL         0x3800U
#define I210_TDBAH         0x3804U
#define I210_TDLEN         0x3808U
#define I210_TDH           0x3810U
#define I210_TDT           0x3818U
#define I210_TXDCTL        0x3828U
#define I210_SWSM          0x5B50U
#define I210_SW_FW_SYNC    0x5B5CU

/* CTRL bity. */
#define I210_CTRL_FD        (1U << 0)
#define I210_CTRL_ASDE      (1U << 5)
#define I210_CTRL_SLU       (1U << 6)
#define I210_CTRL_ILOS      (1U << 7)
#define I210_CTRL_SPD_100   (1U << 8)
#define I210_CTRL_SPD_1000  (1U << 9)
#define I210_CTRL_FRCSPD    (1U << 11)
#define I210_CTRL_FRCDPX    (1U << 12)
#define I210_CTRL_RST       (1U << 26)

/* CTRL_EXT bity. */
#define I210_CTRL_EXT_DRV_LOAD (1U << 28)

/* EEC bity. */
#define I210_EEC_AUTO_RD    (1U << 9)

/* STATUS bity. */
#define I210_STATUS_FD      (1U << 0)
#define I210_STATUS_LU      (1U << 1)

/* SWSM / SW_FW_SYNC (semafor wspoldzielony z firmware). */
#define I210_SWSM_SMBI      (1U << 0)
#define I210_SWSM_SWESMBI   (1U << 1)
#define I210_SWFW_PHY0_SM   0x02U

/* MDIC bity (dostep do PHY przez MDIO). */
#define I210_MDIC_WRITE     (1U << 26)
#define I210_MDIC_READ      (2U << 26)
#define I210_MDIC_READY     (1U << 28)
#define I210_MDIC_ERROR     (1U << 30)
#define I210_MDIC_REG_SHIFT 16U
#define I210_MDIC_PHY_SHIFT 21U
#define I210_MDIC_DATA_MASK 0xFFFFU

/* MII rejestry PHY. */
#define MII_BMCR       0U
#define MII_BMSR       1U
#define MII_PHYSID1    2U
#define MII_PHYSID2    3U
#define MII_ADVERTISE  4U
#define MII_LPA        5U
#define MII_CTRL1000   9U
#define MII_STAT1000   10U
#define PHY_PAGE_SELECT 31U

/* RCTL bity. Bufor 2048 B (BSIZE=00, BSEX=0). */
#define I210_RCTL_EN       (1U << 1)
#define I210_RCTL_UPE      (1U << 3)
#define I210_RCTL_MPE      (1U << 4)
#define I210_RCTL_BAM      (1U << 15)
#define I210_RCTL_SECRC    (1U << 26)

/* TCTL bity. */
#define I210_TCTL_EN       (1U << 1)
#define I210_TCTL_PSP      (1U << 3)
#define I210_TCTL_CT_SHIFT 4U
#define I210_TCTL_COLD_SHIFT 12U
#define I210_TCTL_RTLC     (1U << 24)

/* TIPG dla miedzi: IPGT=8, IPGR1=8, IPGR2=6. */
#define I210_TIPG_COPPER   (8U | (8U << 10) | (6U << 20))

/* SRRCTL: BSIZEPACKET w jednostkach 1 KB (bity 6:0), DESCTYPE (bity 27:25) = 0 -> legacy. */
#define I210_SRRCTL_BSIZE_2KB   2U

/* RXDCTL / TXDCTL (progi jak w sterowniku igb). */
#define I210_XDCTL_ENABLE       (1U << 25)
#define I210_RXDCTL_VALUE       (I210_XDCTL_ENABLE | 8U | (8U << 8) | (1U << 16))
#define I210_TXDCTL_VALUE       (I210_XDCTL_ENABLE | 8U | (1U << 8) | (1U << 16))

/* Deskryptor TX - pole cmd. */
#define I210_TXD_CMD_EOP   (1U << 0)
#define I210_TXD_CMD_IFCS  (1U << 1)
#define I210_TXD_CMD_IC    (1U << 2)
#define I210_TXD_CMD_RS    (1U << 3)
#define I210_TXD_CMD_RPS   (1U << 4)

/* Deskryptor TX/RX - pole status. */
#define I210_RXD_STAT_DD   (1U << 0)
#define I210_RXD_STAT_EOP  (1U << 1)
#define I210_TXD_STAT_DD   (1U << 0)

/* Bledy RX, ktore naprawde oznaczaja uszkodzona ramke (CE|SE|SEQ|RXE). */
#define I210_RXD_ERR_FATAL 0x87U

#define I210_RX_COUNT      64U
#define I210_TX_COUNT      16U
#define I210_BUF_SIZE      2048U
#define I210_MIN_FRAME     60U
#define I210_MAX_FRAME     1518U

/* Kody bledow zwracane przez i210_last_error(). */
#define I210_ERR_NONE      0U
#define I210_ERR_NO_DEVICE 1U
#define I210_ERR_RESET     2U
#define I210_ERR_MAC       3U
#define I210_ERR_NO_LINK   4U
#define I210_ERR_NO_PHY    5U

/*
 * Deskryptor TX i RX (16 bajtow, format legacy).
 * TX: addr_low, addr_high, length, cso, cmd, status, css, special
 * RX: addr_low, addr_high, length, cso/cmd, status/errors, css, special
 */
struct i210_desc {
    uint32_t addr_low;
    uint32_t addr_high;
    uint16_t length;
    uint8_t  cso;
    uint8_t  cmd;
    uint8_t  status;
    uint8_t  css;
    uint16_t special;
} __attribute__((packed));

typedef char i210_desc_must_be_16_bytes[(sizeof(struct i210_desc) == 16) ? 1 : -1];

static volatile uint8_t *i210_mmio;
static uint8_t  i210_mac[6];
static uint8_t  i210_phy_addr;
static uint8_t  i210_phy_ok;
static uint8_t  i210_error;
static uint16_t i210_mtu = 1500;
static uint8_t  i210_ready;
static uint8_t  i210_link;
static uint32_t i210_rx_head;
static uint32_t i210_tx_head;
static uint32_t i210_tsc_per_us;   /* 0 = brak kalibracji, uzywamy outb 0x80 */
/* Baza pierscieni musi byc wyrownana do 128 B - bierzemy cala strone. */
static struct i210_desc i210_rx_desc[I210_RX_COUNT] __attribute__((aligned(4096)));
static struct i210_desc i210_tx_desc[I210_TX_COUNT] __attribute__((aligned(4096)));
static uint8_t  i210_rx_buf[I210_RX_COUNT][I210_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t  i210_tx_buf[I210_TX_COUNT][I210_BUF_SIZE] __attribute__((aligned(16)));

static struct i210_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_dropped;
    uint32_t rx_dropped;
} i210_stats;

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

/* ------------------------------------------------------------------ */
/* Czas: TSC skalibrowany wzgledem PIT kanal 2.                       */
/* ------------------------------------------------------------------ */

static uint64_t i210_rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void i210_calibrate_tsc(void)
{
    uint8_t saved;
    uint64_t t0, t1;
    uint32_t n, delta, per_us;

    if (i210_tsc_per_us) return;

    saved = inb(0x61);
    outb(0x61, (uint8_t)((saved & 0xFC) | 0x01));
    outb(0x43, 0xB0);
    outb(0x42, (uint8_t)(11932 & 0xFF));
    outb(0x42, (uint8_t)(11932 >> 8));

    t0 = i210_rdtsc();
    for (n = 0; n < 20000000U; n++) {
        if (inb(0x61) & 0x20) break;
    }
    t1 = i210_rdtsc();
    outb(0x61, saved);

    if (n == 20000000U) return;
    delta = (uint32_t)(t1 - t0);
    per_us = delta / 10000U;
    if (per_us < 100U || per_us > 10000U) return;
    i210_tsc_per_us = per_us;
}

/* ------------------------------------------------------------------ */
/* MMIO I210.                                                         */
/* ------------------------------------------------------------------ */

static uint32_t i210_read_reg(uint32_t offset)
{
    return *(volatile uint32_t *)(i210_mmio + offset);
}

static void i210_write_reg(uint32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(i210_mmio + offset) = value;
}

/* Wymusza zakonczenie zapisow (odczyt STATUS). */
static void i210_flush(void)
{
    (void)i210_read_reg(I210_STATUS);
}

/* Opoznienie w mikrosekundach (TSC; zapas: zapis do portu 0x80, ok. 1 us). */
static void i210_delay_us(uint32_t us)
{
    if (i210_tsc_per_us) {
        uint64_t end = i210_rdtsc() + (uint64_t)us * i210_tsc_per_us;
        while (i210_rdtsc() < end) __asm__ volatile("pause");
        return;
    }
    while (us--) outb(0x80, 0);
}

/* ------------------------------------------------------------------ */
/* Semafor PHY (SWSM + SW_FW_SYNC) - firmware tez uzywa PHY.          */
/* ------------------------------------------------------------------ */

static int i210_get_hw_semaphore(void)
{
    uint32_t i, swsm;

    for (i = 0; i < 100; i++) {                       /* semafor programowy (odczyt SMBI = 0 -> nasz) */
        swsm = i210_read_reg(I210_SWSM);
        if (!(swsm & I210_SWSM_SMBI)) break;
        i210_delay_us(50);
    }
    if (i == 100) return 0;

    for (i = 0; i < 100; i++) {                       /* semafor firmware */
        swsm = i210_read_reg(I210_SWSM);
        i210_write_reg(I210_SWSM, swsm | I210_SWSM_SWESMBI);
        if (i210_read_reg(I210_SWSM) & I210_SWSM_SWESMBI) return 1;
        i210_delay_us(50);
    }
    i210_write_reg(I210_SWSM, i210_read_reg(I210_SWSM) & ~(I210_SWSM_SMBI | I210_SWSM_SWESMBI));
    return 0;
}

static void i210_put_hw_semaphore(void)
{
    i210_write_reg(I210_SWSM, i210_read_reg(I210_SWSM) & ~(I210_SWSM_SMBI | I210_SWSM_SWESMBI));
}

/* 1 = zasob PHY nasz; 0 = nie udalo sie (wolajacy moze mimo to sprobowac dostepu). */
static int i210_phy_acquire(void)
{
    uint32_t i, sync, mask = I210_SWFW_PHY0_SM;

    for (i = 0; i < 20; i++) {
        if (!i210_get_hw_semaphore()) return 0;
        sync = i210_read_reg(I210_SW_FW_SYNC);
        if (!(sync & (mask | (mask << 16)))) {
            i210_write_reg(I210_SW_FW_SYNC, sync | mask);
            i210_put_hw_semaphore();
            return 1;
        }
        i210_put_hw_semaphore();
        i210_delay_us(5000);
    }
    return 0;
}

static void i210_phy_release(void)
{
    if (!i210_get_hw_semaphore()) return;
    i210_write_reg(I210_SW_FW_SYNC, i210_read_reg(I210_SW_FW_SYNC) & ~I210_SWFW_PHY0_SM);
    i210_put_hw_semaphore();
}

/* ------------------------------------------------------------------ */
/* MDIC / PHY (I210.mdio).                                            */
/* ------------------------------------------------------------------ */

#define I210_MDIC_POLLS    200U

static uint16_t i210_mdio_read_raw(uint8_t phy_addr, uint8_t reg)
{
    uint32_t cmd = I210_MDIC_READ |
                   ((uint32_t)phy_addr << I210_MDIC_PHY_SHIFT) |
                   ((uint32_t)reg << I210_MDIC_REG_SHIFT);
    uint32_t i;
    i210_write_reg(I210_MDIC, cmd);
    for (i = 0; i < I210_MDIC_POLLS; i++) {
        cmd = i210_read_reg(I210_MDIC);
        if (cmd & I210_MDIC_READY) {
            if (cmd & I210_MDIC_ERROR) return 0xFFFFU;
            return (uint16_t)(cmd & I210_MDIC_DATA_MASK);
        }
        i210_delay_us(50);
    }
    return 0xFFFFU;
}

static void i210_mdio_write_raw(uint8_t phy_addr, uint8_t reg, uint16_t value)
{
    uint32_t cmd = I210_MDIC_WRITE |
                   ((uint32_t)phy_addr << I210_MDIC_PHY_SHIFT) |
                   ((uint32_t)reg << I210_MDIC_REG_SHIFT) |
                   (uint32_t)value;
    uint32_t i;
    i210_write_reg(I210_MDIC, cmd);
    for (i = 0; i < I210_MDIC_POLLS; i++) {
        if (i210_read_reg(I210_MDIC) & I210_MDIC_READY) return;
        i210_delay_us(50);
    }
}

static uint16_t i210_mdio_read(uint8_t phy_addr, uint8_t reg)
{
    uint16_t v;
    int held = i210_phy_acquire();
    v = i210_mdio_read_raw(phy_addr, reg);
    if (held) i210_phy_release();
    return v;
}

static void i210_mdio_write(uint8_t phy_addr, uint8_t reg, uint16_t value)
{
    int held = i210_phy_acquire();
    i210_mdio_write_raw(phy_addr, reg, value);
    if (held) i210_phy_release();
}

static int i210_phy_id_valid(uint8_t addr)
{
    uint16_t id1 = i210_mdio_read(addr, MII_PHYSID1);
    uint16_t id2;
    if (id1 == 0xFFFFU || id1 == 0) return 0;
    id2 = i210_mdio_read(addr, MII_PHYSID2);
    return id2 != 0xFFFFU && id2 != 0;
}

static int i210_phy_find(uint8_t *phy_addr)
{
    uint32_t i;
    /* I210 ma zintegrowany PHY, najczesciej pod adresem 1. */
    static const uint8_t first[2] = { 1, 2 };
    for (i = 0; i < 2; i++) {
        if (i210_phy_id_valid(first[i])) {
            *phy_addr = first[i];
            return 1;
        }
    }
    for (i = 0; i < 32; i++) {
        if (i210_phy_id_valid((uint8_t)i)) {
            *phy_addr = (uint8_t)i;
            return 1;
        }
    }
    return 0;
}

static void i210_phy_autoneg(void)
{
    i210_mdio_write(i210_phy_addr, MII_ADVERTISE, 0x01E1U);
    i210_mdio_write(i210_phy_addr, MII_CTRL1000, 0x0300U);
    i210_mdio_write(i210_phy_addr, MII_BMCR, 0x1200U);
}

static void i210_phy_soft_reset(void)
{
    uint32_t i;
    i210_mdio_write(i210_phy_addr, MII_BMCR, 0x8000U);
    for (i = 0; i < 500; i++) {
        if (!(i210_mdio_read(i210_phy_addr, MII_BMCR) & 0x8000U)) break;
        i210_delay_us(1000);
    }
}

/* Link: STATUS.LU (MAC) lub BMSR (PHY) po autonegocjacji. */
static int i210_link_ok(void)
{
    uint16_t bmsr;
    if (i210_read_reg(I210_STATUS) & I210_STATUS_LU) return 1;
    if (!i210_phy_ok) return 0;
    (void)i210_mdio_read(i210_phy_addr, MII_BMSR);
    bmsr = i210_mdio_read(i210_phy_addr, MII_BMSR);
    if (bmsr == 0xFFFFU) return 0;
    return (bmsr & 0x0004U) && (bmsr & 0x0020U);
}

/* ------------------------------------------------------------------ */
/* Wykrywanie urzadzenia PCI I210.                                    */
/* ------------------------------------------------------------------ */

static int i210_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            if (vendor == 0xFFFF) continue;
            if (vendor != I210_VENDOR) continue;
            switch (device) {
            case I210_DEV_I210_COPPER:
            case I210_DEV_I210_COPPER_FLASHLESS:
            case I210_DEV_I211_COPPER:
                break;
            default: continue;
            }

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || (bar0 & 1)) continue;
            if ((bar0 & 6U) == 4U && pci_read32((uint8_t)bus, slot, 0, 0x14) != 0) continue;

            uint32_t cmd = pci_read32((uint8_t)bus, slot, 0, 0x04) & 0xFFFFU;
            pci_write32((uint8_t)bus, slot, 0, 0x04, cmd | 0x00000006U);
            i210_mmio = (volatile uint8_t *)(bar0 & 0xFFFFFFF0U);
            return 1;
        }
    }
    return 0;
}

static int i210_mac_valid(const uint8_t *m)
{
    uint32_t i;
    int zero = 1, ones = 1;
    for (i = 0; i < 6; i++) {
        if (m[i] != 0x00) zero = 0;
        if (m[i] != 0xFF) ones = 0;
    }
    if (zero || ones) return 0;
    if (m[0] & 1U) return 0;                  /* bit multicast */
    return 1;
}

static int i210_read_mac(void)
{
    uint32_t low = i210_read_reg(I210_RAL);
    uint32_t high = i210_read_reg(I210_RAH);
    i210_mac[0] = (uint8_t)low;
    i210_mac[1] = (uint8_t)(low >> 8);
    i210_mac[2] = (uint8_t)(low >> 16);
    i210_mac[3] = (uint8_t)(low >> 24);
    i210_mac[4] = (uint8_t)high;
    i210_mac[5] = (uint8_t)(high >> 8);
    return i210_mac_valid(i210_mac);
}

/* ------------------------------------------------------------------ */
/* Inicjalizacja sprzetu.                                             */
/* ------------------------------------------------------------------ */

static void i210_mask_irqs(void)
{
    i210_write_reg(I210_IMC, 0xFFFFFFFFU);
    i210_write_reg(I210_EIMC, 0xFFFFFFFFU);
}

static void i210_quiesce(void)
{
    i210_mask_irqs();
    i210_write_reg(I210_RCTL, 0);
    i210_write_reg(I210_TCTL, I210_TCTL_PSP);
    i210_flush();
    i210_delay_us(10000);
    (void)i210_read_reg(I210_ICR);
}

static int i210_reset(void)
{
    uint32_t t;
    i210_quiesce();
    i210_write_reg(I210_CTRL, i210_read_reg(I210_CTRL) | I210_CTRL_RST);
    i210_delay_us(1000);
    for (t = 0; t < 10000; t++) {
        if (!(i210_read_reg(I210_CTRL) & I210_CTRL_RST)) break;
        i210_delay_us(10);
    }
    if (t == 10000) return 0;

    /* Poczekaj, az uklad skonczy automatyczny odczyt NVM (MAC i konfiguracja). */
    for (t = 0; t < 1000; t++) {
        if (i210_read_reg(I210_EEC) & I210_EEC_AUTO_RD) break;
        i210_delay_us(100);
    }
    i210_delay_us(10000);

    i210_mask_irqs();
    (void)i210_read_reg(I210_ICR);
    return 1;
}

static void i210_set_link_up(void)
{
    uint32_t ctrl = i210_read_reg(I210_CTRL);
    /* Predkosc i duplex ustala PHY - nie wymuszamy (jak igb dla miedzi: tylko SLU). */
    ctrl &= ~(I210_CTRL_FRCSPD | I210_CTRL_FRCDPX | I210_CTRL_ILOS);
    ctrl |= I210_CTRL_SLU;
    i210_write_reg(I210_CTRL, ctrl);
    i210_flush();
}

static void i210_init_rings(void)
{
    uint32_t i, t;

    i210_write_reg(I210_RXDCTL, i210_read_reg(I210_RXDCTL) & ~I210_XDCTL_ENABLE);
    i210_write_reg(I210_TXDCTL, i210_read_reg(I210_TXDCTL) & ~I210_XDCTL_ENABLE);
    for (t = 0; t < 1000; t++) {
        if (!(i210_read_reg(I210_RXDCTL) & I210_XDCTL_ENABLE) &&
            !(i210_read_reg(I210_TXDCTL) & I210_XDCTL_ENABLE)) break;
        i210_delay_us(10);
    }

    /* Bez moderacji przerwan i bez offloadu sum kontrolnych RX. */
    i210_write_reg(I210_ITR, 0);
    i210_write_reg(I210_RXCSUM, 0);
    i210_write_reg(I210_RFCTL, 0);

    for (i = 0; i < I210_RX_COUNT; i++) {
        i210_rx_desc[i].addr_low = (uint32_t)&i210_rx_buf[i][0];
        i210_rx_desc[i].addr_high = 0;
        i210_rx_desc[i].length = 0;
        i210_rx_desc[i].cso = 0;
        i210_rx_desc[i].cmd = 0;
        i210_rx_desc[i].status = 0;
        i210_rx_desc[i].css = 0;
        i210_rx_desc[i].special = 0;
    }
    i210_write_reg(I210_RDBAL, (uint32_t)&i210_rx_desc[0]);
    i210_write_reg(I210_RDBAH, 0);
    i210_write_reg(I210_RDLEN, sizeof(i210_rx_desc));
    i210_write_reg(I210_SRRCTL, I210_SRRCTL_BSIZE_2KB);   /* bufor 2 KB, deskryptor legacy */
    i210_write_reg(I210_RDH, 0);
    i210_write_reg(I210_RDT, 0);

    for (i = 0; i < I210_TX_COUNT; i++) {
        i210_tx_desc[i].addr_low = (uint32_t)&i210_tx_buf[i][0];
        i210_tx_desc[i].addr_high = 0;
        i210_tx_desc[i].length = 0;
        i210_tx_desc[i].cso = 0;
        i210_tx_desc[i].cmd = 0;
        i210_tx_desc[i].status = I210_TXD_STAT_DD;
        i210_tx_desc[i].css = 0;
        i210_tx_desc[i].special = 0;
    }
    i210_write_reg(I210_TDBAL, (uint32_t)&i210_tx_desc[0]);
    i210_write_reg(I210_TDBAH, 0);
    i210_write_reg(I210_TDLEN, sizeof(i210_tx_desc));
    i210_write_reg(I210_TDH, 0);
    i210_write_reg(I210_TDT, 0);

    i210_rx_head = 0;
    i210_tx_head = 0;
    __asm__ volatile("" : : : "memory");

    i210_write_reg(I210_RXDCTL, I210_RXDCTL_VALUE);
    i210_write_reg(I210_TXDCTL, I210_TXDCTL_VALUE);
    for (t = 0; t < 1000; t++) {
        if ((i210_read_reg(I210_RXDCTL) & I210_XDCTL_ENABLE) &&
            (i210_read_reg(I210_TXDCTL) & I210_XDCTL_ENABLE)) break;
        i210_delay_us(10);
    }

    /* Ogon pierscienia RX dopiero po wlaczeniu kolejki (wymog rodziny igb). */
    i210_write_reg(I210_RDT, I210_RX_COUNT - 1);
}

static void i210_init_mac(void)
{
    uint32_t i;

    i210_write_reg(I210_RCTL, 0);
    i210_write_reg(I210_TCTL, 0);

    i210_write_reg(I210_RAL, (uint32_t)i210_mac[0] |
                             ((uint32_t)i210_mac[1] << 8) |
                             ((uint32_t)i210_mac[2] << 16) |
                             ((uint32_t)i210_mac[3] << 24));
    i210_write_reg(I210_RAH, (uint32_t)i210_mac[4] |
                             ((uint32_t)i210_mac[5] << 8) | (1U << 31));

    for (i = 0; i < 128; i++) i210_write_reg(I210_MTA + i * 4U, 0);
    i210_flush();
}

static void i210_apply_link(void)
{
    uint32_t tctl = 0;

    i210_write_reg(I210_RCTL, 0);
    i210_write_reg(I210_TCTL, 0);
    for (uint32_t t = 0; t < 10000; t++) {
        if (!(i210_read_reg(I210_RCTL) & I210_RCTL_EN) &&
            !(i210_read_reg(I210_TCTL) & I210_TCTL_EN)) break;
        i210_delay_us(10);
    }

    i210_set_link_up();
    i210_write_reg(I210_TIPG, I210_TIPG_COPPER);

    tctl = I210_TCTL_EN | I210_TCTL_PSP |
           (0x0FU << I210_TCTL_CT_SHIFT) |
           (0x3FU << I210_TCTL_COLD_SHIFT) |
           I210_TCTL_RTLC;
    i210_write_reg(I210_TCTL, tctl);

    i210_write_reg(I210_RCTL, I210_RCTL_EN | I210_RCTL_UPE |
                              I210_RCTL_MPE | I210_RCTL_BAM |
                              I210_RCTL_SECRC);

    __asm__ volatile("" : : : "memory");
    i210_write_reg(I210_RDT,
                   (i210_rx_head + I210_RX_COUNT - 1U) % I210_RX_COUNT);
    i210_link = 1;
}

static void i210_link_poll(void)
{
    static uint32_t poll_count = 0;
    if ((++poll_count & 0x3FFU) != 0) return;
    if (i210_link) {
        if (!(i210_read_reg(I210_STATUS) & I210_STATUS_LU) && !i210_link_ok())
            i210_link = 0;
        return;
    }
    if (i210_link_ok()) i210_apply_link();
}

static void i210_rx_recycle(void)
{
    struct i210_desc *desc = &i210_rx_desc[i210_rx_head];
    desc->length = 0;
    desc->cso = 0;
    desc->cmd = 0;
    desc->status = 0;
    desc->css = 0;
    desc->special = 0;
    __asm__ volatile("" : : : "memory");
    i210_write_reg(I210_RDT, i210_rx_head);
    i210_rx_head = (i210_rx_head + 1U) % I210_RX_COUNT;
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int i210_init(void)
{
    uint32_t i;

    if (i210_ready) return 1;
    i210_error = I210_ERR_NONE;
    i210_link = 0;

    i210_calibrate_tsc();

    if (!i210_find_device()) { i210_error = I210_ERR_NO_DEVICE; return 0; }
    if (!i210_reset()) { i210_error = I210_ERR_RESET; return 0; }

    /* Poinformuj firmware, ze sterownik jest zaladowany. */
    i210_write_reg(I210_CTRL_EXT, i210_read_reg(I210_CTRL_EXT) | I210_CTRL_EXT_DRV_LOAD);

    if (!i210_read_mac()) { i210_error = I210_ERR_MAC; return 0; }

    i210_phy_ok = (uint8_t)i210_phy_find(&i210_phy_addr);
    if (!i210_phy_ok) i210_error = I210_ERR_NO_PHY;

    i210_set_link_up();
    if (i210_phy_ok) {
        i210_phy_soft_reset();
        i210_phy_autoneg();
    }

    i210_init_rings();
    i210_init_mac();

    for (i = 0; i < 3000 && !i210_link; i++) {
        if (i210_link_ok()) i210_apply_link();
        else i210_delay_us(1000);
    }

    if (!i210_link && i210_phy_ok) {
        i210_phy_autoneg();
        for (i = 0; i < 5000 && !i210_link; i++) {
            if (i210_link_ok()) i210_apply_link();
            else i210_delay_us(1000);
        }
    }

    if (!i210_link && i210_error == I210_ERR_NONE)
        i210_error = I210_ERR_NO_LINK;

    i210_ready = 1;
    return 1;
}

int i210_last_error(void)
{
    return (int)i210_error;
}

int i210_is_ready(void)
{
    return i210_ready;
}

int i210_get_mac(uint8_t mac[6])
{
    if (!i210_ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = i210_mac[i];
    return 1;
}

int i210_send(const void *frame, uint16_t length)
{
    const uint8_t *src = (const uint8_t *)frame;
    volatile struct i210_desc *desc;
    uint32_t i, t, idx, send_len;

    if (!i210_ready || !frame || length == 0 || length > I210_MAX_FRAME) return -1;

    i210_link_poll();
    if (!i210_link) { i210_stats.tx_dropped++; return 0; }

    idx = i210_tx_head;
    desc = &i210_tx_desc[idx];
    __asm__ volatile("" : : : "memory");
    if (!(desc->status & I210_TXD_STAT_DD)) {
        i210_stats.tx_dropped++;
        return 0;
    }

    for (i = 0; i < length; i++) i210_tx_buf[idx][i] = src[i];
    send_len = length;
    while (send_len < I210_MIN_FRAME) i210_tx_buf[idx][send_len++] = 0;

    desc->addr_low = (uint32_t)&i210_tx_buf[idx][0];
    desc->addr_high = 0;
    desc->length = (uint16_t)send_len;
    desc->cso = 0;
    desc->cmd = I210_TXD_CMD_EOP | I210_TXD_CMD_IFCS | I210_TXD_CMD_RS;
    desc->status = 0;
    desc->css = 0;
    desc->special = 0;
    __asm__ volatile("" : : : "memory");

    i210_tx_head = (i210_tx_head + 1U) % I210_TX_COUNT;
    i210_write_reg(I210_TDT, i210_tx_head);

    for (t = 0; t < 1000000U; t++) {
        __asm__ volatile("" : : : "memory");
        if (desc->status & I210_TXD_STAT_DD) break;
    }
    if (t == 1000000U) {
        i210_stats.tx_errors++;
        return 0;
    }

    i210_stats.tx_packets++;
    return 1;
}

int i210_receive(void *frame, uint16_t capacity)
{
    uint8_t *dst = (uint8_t *)frame;
    volatile struct i210_desc *desc;
    uint32_t i, len;
    uint8_t status, errors;

    if (!i210_ready || !frame || capacity == 0) return -1;

    i210_link_poll();
    if (!i210_link) return 0;

    desc = &i210_rx_desc[i210_rx_head];
    __asm__ volatile("" : : : "memory");
    status = desc->status;
    if (!(status & I210_RXD_STAT_DD)) return 0;
    __asm__ volatile("" : : : "memory");

    len = desc->length;
    errors = desc->css;

    if (!(status & I210_RXD_STAT_EOP) || (errors & I210_RXD_ERR_FATAL)) {
        i210_stats.rx_errors++;
        i210_rx_recycle();
        return 0;
    }

    if (len < 14 || len > (uint32_t)I210_MAX_FRAME) {
        i210_stats.rx_errors++;
        i210_rx_recycle();
        return 0;
    }

    if (len > (uint32_t)capacity) {
        i210_stats.rx_dropped++;
        i210_rx_recycle();
        return 0;
    }

    for (i = 0; i < len; i++) dst[i] = i210_rx_buf[i210_rx_head][i];
    i210_stats.rx_packets++;
    i210_rx_recycle();
    return (int)len;
}

void i210_shutdown(void)
{
    if (!i210_ready) return;
    i210_write_reg(I210_RCTL, 0);
    i210_write_reg(I210_TCTL, 0);
    i210_write_reg(I210_RXDCTL, i210_read_reg(I210_RXDCTL) & ~I210_XDCTL_ENABLE);
    i210_write_reg(I210_TXDCTL, i210_read_reg(I210_TXDCTL) & ~I210_XDCTL_ENABLE);
    i210_mask_irqs();
    i210_write_reg(I210_CTRL_EXT, i210_read_reg(I210_CTRL_EXT) & ~I210_CTRL_EXT_DRV_LOAD);
    i210_link = 0;
    i210_ready = 0;
}

int i210_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                   uint32_t *tx_errors, uint32_t *rx_errors,
                   uint32_t *tx_dropped, uint32_t *rx_dropped)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = i210_stats.tx_packets;
    *rx_packets = i210_stats.rx_packets;
    *tx_errors = i210_stats.tx_errors;
    *rx_errors = i210_stats.rx_errors;
    if (tx_dropped) *tx_dropped = i210_stats.tx_dropped;
    if (rx_dropped) *rx_dropped = i210_stats.rx_dropped;
    return 1;
}

void i210_stats_reset(void)
{
    i210_stats.tx_packets = 0;
    i210_stats.rx_packets = 0;
    i210_stats.tx_errors = 0;
    i210_stats.rx_errors = 0;
    i210_stats.tx_dropped = 0;
    i210_stats.rx_dropped = 0;
}

/* Bufory 2 KB i brak LPE: ramki do 1518 B, wiec MTU maks. 1500. */
int i210_set_mtu(uint16_t mtu)
{
    if (!i210_ready || mtu < 68 || mtu > 1500) return 0;
    i210_mtu = mtu;
    return 1;
}

uint16_t i210_get_mtu(void)
{
    return i210_mtu;
}

int i210_link_up(void)
{
    return i210_link ? 1 : 0;
}