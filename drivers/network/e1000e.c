/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Zaawansowany sterownik Intel e1000e (82574/82578/82579/I217 Gigabit Ethernet).
 * Polling, bez przerwan, MAC i PHY przez MMIO BAR0.
 *
 * Dwa tryby pracy:
 *  - 82574L (PCIe, np. QEMU "-device e1000e"): pelny reset, PHY pod adresem 1.
 *  - Warianty PCH (82578 / 82579 / I217): PHY zintegrowany z chipsetem.
 *    Bez pelnego resetu (zachowujemy stan PHY ustawiony przez BIOS), dostep
 *    do MDIO przez flage SWFLAG, adres PHY 2/1, wolny tryb MDIO dla 82577/8,
 *    wylaczone K1 dla 82578, ewentualny restart zasilania PHY przez LANPHYPC.
 */

#include <drivers/network/e1000e.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define E1000E_VENDOR      0x8086U

/* e1000e device IDs (Intel Gigabit Ethernet, nie e1000): */
#define E1000E_DEV_82574L    0x10D3U
#define E1000E_DEV_82578DM   0x10EFU
#define E1000E_DEV_82578DC   0x10F0U
#define E1000E_DEV_82579LM   0x1502U
#define E1000E_DEV_82579V    0x1503U
#define E1000E_DEV_I217LM    0x153AU
#define E1000E_DEV_I217V     0x153BU

/* Rodzaj kontrolera. */
#define E1000E_TYPE_82574    0U
#define E1000E_TYPE_PCHLAN   1U   /* 82577/82578 */
#define E1000E_TYPE_PCH2LAN  2U   /* 82579 */
#define E1000E_TYPE_PCH_LPT  3U   /* I217 */
#define E1000E_IS_PCH        (e1000e_type != E1000E_TYPE_82574)

/* MMIO rejestry e1000e (offsety od BAR0). */
#define E1000E_CTRL          0x0000U
#define E1000E_STATUS        0x0008U
#define E1000E_CTRL_EXT      0x0018U
#define E1000E_MDIC          0x0020U
#define E1000E_FEXTNVM3      0x003CU
#define E1000E_KMRNCTRLSTA   0x0034U
#define E1000E_ICR           0x00C0U
#define E1000E_ITR           0x00C4U
#define E1000E_IMS           0x00D0U
#define E1000E_IMC           0x00D8U
#define E1000E_RCTL          0x0100U
#define E1000E_TCTL          0x0400U
#define E1000E_TIPG          0x0410U
#define E1000E_RDBAL         0x2800U
#define E1000E_RDBAH         0x2804U
#define E1000E_RDLEN         0x2808U
#define E1000E_RDH           0x2810U
#define E1000E_RDT           0x2818U
#define E1000E_RDTR          0x2820U
#define E1000E_RXDCTL        0x2828U
#define E1000E_RADV          0x282CU
#define E1000E_TDBAL         0x3800U
#define E1000E_TDBAH         0x3804U
#define E1000E_TDLEN         0x3808U
#define E1000E_TDH           0x3810U
#define E1000E_TDT           0x3818U
#define E1000E_TIDV          0x3820U
#define E1000E_TXDCTL        0x3828U
#define E1000E_TADV          0x382CU
#define E1000E_RXCSUM        0x5000U
#define E1000E_RFCTL         0x5008U
#define E1000E_MTA           0x5200U
#define E1000E_RAL           0x5400U
#define E1000E_RAH           0x5404U
#define E1000E_EXTCNF_CTRL   0x0F00U

/* CTRL bity. */
#define E1000E_CTRL_FD        (1U << 0)
#define E1000E_CTRL_ASDE      (1U << 5)
#define E1000E_CTRL_SLU       (1U << 6)
#define E1000E_CTRL_ILOS      (1U << 7)
#define E1000E_CTRL_SPD_100   (1U << 8)
#define E1000E_CTRL_SPD_1000  (1U << 9)
#define E1000E_CTRL_FRCSPD    (1U << 11)
#define E1000E_CTRL_FRCDPX    (1U << 12)
#define E1000E_CTRL_LANPHYPC_OVERRIDE (1U << 16)
#define E1000E_CTRL_LANPHYPC_VALUE    (1U << 17)
#define E1000E_CTRL_RST       (1U << 26)

/* CTRL_EXT bity. */
#define E1000E_CTRL_EXT_FORCE_SMBUS (1U << 11)
#define E1000E_CTRL_EXT_LPCD        (1U << 14)
#define E1000E_CTRL_EXT_SPD_BYPS    (1U << 15)
#define E1000E_CTRL_EXT_DRV_LOAD    (1U << 28)

/* STATUS bity. */
#define E1000E_STATUS_FD      (1U << 0)
#define E1000E_STATUS_LU      (1U << 1)

/* EXTCNF_CTRL: wlasnosc MDIO dla oprogramowania. */
#define E1000E_EXTCNF_SWFLAG  (1U << 5)

/* FEXTNVM3: licznik konfiguracji PHY. */
#define E1000E_PHY_CFG_COUNTER_MASK   0x0C000000U
#define E1000E_PHY_CFG_COUNTER_50MSEC 0x08000000U

/* KMRN (interfejs MAC-PHY) i konfiguracja K1. */
#define E1000E_KMRN_OFFSET_MASK   0x001F0000U
#define E1000E_KMRN_OFFSET_SHIFT  16U
#define E1000E_KMRN_REN           0x00200000U
#define E1000E_KMRN_K1_CONFIG     0x7U
#define E1000E_KMRN_K1_ENABLE     0x0002U

/* RXDCTL / TXDCTL. GRAN = progi liczone w deskryptorach, WTHRESH = 1. */
#define E1000E_XDCTL_ENABLE       (1U << 25)
#define E1000E_XDCTL_GRAN         (1U << 24)
#define E1000E_XDCTL_WTHRESH1     (1U << 16)
#define E1000E_TXDCTL_COUNT_DESC  (1U << 22)

/* MDIC bity (dostep do PHY przez MDIO). */
#define E1000E_MDIC_WRITE     (1U << 26)
#define E1000E_MDIC_READ      (2U << 26)
#define E1000E_MDIC_READY     (1U << 28)
#define E1000E_MDIC_ERROR     (1U << 30)
#define E1000E_MDIC_REG_SHIFT 16U
#define E1000E_MDIC_PHY_SHIFT 21U
#define E1000E_MDIC_DATA_MASK 0xFFFFU

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

/* Strona 769 rejestr 16 PHY HV (82577/82578): sterowanie trybem KMRN. */
#define HV_KMRN_MODE_PAGE   769U
#define HV_KMRN_MODE_REG    16U
#define HV_KMRN_MDIO_SLOW   0x0400U

/* RCTL bity. Rozmiar bufora 2048 B = BSIZE 00 przy BSEX = 0. */
#define E1000E_RCTL_EN       (1U << 1)
#define E1000E_RCTL_UPE      (1U << 3)
#define E1000E_RCTL_MPE      (1U << 4)
#define E1000E_RCTL_BAM      (1U << 15)
#define E1000E_RCTL_SECRC    (1U << 26)

/* TCTL bity. */
#define E1000E_TCTL_EN       (1U << 1)
#define E1000E_TCTL_PSP      (1U << 3)
#define E1000E_TCTL_CT_SHIFT 4U
#define E1000E_TCTL_COLD_SHIFT 12U
#define E1000E_TCTL_RTLC     (1U << 24)

/* Domyslne TIPG dla miedzi: IPGT=8, IPGR1=8, IPGR2=6. */
#define E1000E_TIPG_COPPER   (8U | (8U << 10) | (6U << 20))

/* Deskryptor TX - pole cmd. */
#define E1000E_TXD_CMD_EOP   (1U << 0)
#define E1000E_TXD_CMD_IFCS  (1U << 1)
#define E1000E_TXD_CMD_IC    (1U << 2)
#define E1000E_TXD_CMD_RS    (1U << 3)
#define E1000E_TXD_CMD_RPS   (1U << 4)

/* Deskryptor TX/RX - pole status. */
#define E1000E_RXD_STAT_DD   (1U << 0)
#define E1000E_RXD_STAT_EOP  (1U << 1)
#define E1000E_TXD_STAT_DD   (1U << 0)

/* Bledy RX, ktore naprawde oznaczaja uszkodzona ramke (CE|SE|SEQ|RXE).
 * Bity TCPE/IPE (sumy kontrolne) ignorujemy - sumy sprawdza stos sieciowy. */
#define E1000E_RXD_ERR_FATAL 0x87U

#define E1000E_RX_COUNT      32U
#define E1000E_TX_COUNT      8U
#define E1000E_BUF_SIZE      2048U
#define E1000E_MIN_FRAME     60U
#define E1000E_MAX_FRAME     1518U

/* Kody bledow zwracane przez e1000e_last_error(). */
#define E1000E_ERR_NONE      0U
#define E1000E_ERR_NO_DEVICE 1U   /* nie znaleziono karty / BAR0 */
#define E1000E_ERR_RESET     2U   /* reset nie zakonczyl sie */
#define E1000E_ERR_MAC       3U   /* nieprawidlowy MAC w RAL/RAH */
#define E1000E_ERR_NO_LINK   4U   /* brak linku (sterownik nadal dziala) */
#define E1000E_ERR_NO_PHY    5U   /* PHY nie odpowiada na MDIO (uzywamy STATUS.LU) */

/*
 * Uklad wspolny dla deskryptora TX i legacy RX:
 *   TX: length, cso, cmd, status, css, special
 *   RX: length, checksum(cso+cmd), status, errors(css), special
 */
struct e1000e_desc {
    uint32_t addr_low;
    uint32_t addr_high;
    uint16_t length;
    uint8_t  cso;
    uint8_t  cmd;
    uint8_t  status;
    uint8_t  css;
    uint16_t special;
} __attribute__((packed));

typedef char e1000e_desc_must_be_16_bytes[(sizeof(struct e1000e_desc) == 16) ? 1 : -1];

static volatile uint8_t *e1000e_mmio;
static uint8_t  e1000e_type;
static uint8_t  e1000e_mac[6];
static uint8_t  e1000e_phy_addr;
static uint8_t  e1000e_phy_ok;
static uint8_t  e1000e_error;
static uint16_t e1000e_mtu = 1500;
static uint8_t  e1000e_ready;
static uint8_t  e1000e_link;
static uint32_t e1000e_rx_head;
static uint32_t e1000e_tx_head;
static uint32_t e1000e_tsc_per_us;   /* 0 = brak kalibracji, uzywamy outb 0x80 */
static struct e1000e_desc e1000e_rx_desc[E1000E_RX_COUNT] __attribute__((aligned(16)));
static struct e1000e_desc e1000e_tx_desc[E1000E_TX_COUNT] __attribute__((aligned(16)));
static uint8_t  e1000e_rx_buf[E1000E_RX_COUNT][E1000E_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t  e1000e_tx_buf[E1000E_TX_COUNT][E1000E_BUF_SIZE] __attribute__((aligned(16)));

static struct e1000e_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_dropped;
    uint32_t rx_dropped;
} e1000e_stats;

static void barrier(void)
{
    __asm__ volatile("" : : : "memory");
}

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
/* Czas: TSC skalibrowany wzgledem PIT (kanal 2, niezalezny od timera  */
/* systemowego na kanale 0).                                          */
/* ------------------------------------------------------------------ */

static uint64_t e1000e_rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void e1000e_calibrate_tsc(void)
{
    uint8_t saved;
    uint64_t t0, t1;
    uint32_t n, delta, per_us;

    if (e1000e_tsc_per_us) return;

    saved = inb(0x61);
    outb(0x61, (uint8_t)((saved & 0xFC) | 0x01));   /* gate=1, glosnik wyl. */
    outb(0x43, 0xB0);                               /* kanal 2, lo/hi, tryb 0 */
    outb(0x42, (uint8_t)(11932 & 0xFF));            /* 11932 / 1193182 Hz = 10 ms */
    outb(0x42, (uint8_t)(11932 >> 8));

    t0 = e1000e_rdtsc();
    for (n = 0; n < 20000000U; n++) {
        if (inb(0x61) & 0x20) break;
    }
    t1 = e1000e_rdtsc();
    outb(0x61, saved);

    if (n == 20000000U) return;
    delta = (uint32_t)(t1 - t0);
    per_us = delta / 10000U;
    if (per_us < 100U || per_us > 10000U) return;   /* nierealne - ignoruj */
    e1000e_tsc_per_us = per_us;
}

/* ------------------------------------------------------------------ */
/* MMIO e1000e.                                                       */
/* ------------------------------------------------------------------ */

static uint32_t e1000e_read_reg(uint32_t offset)
{
    return *(volatile uint32_t *)(e1000e_mmio + offset);
}

static void e1000e_write_reg(uint32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(e1000e_mmio + offset) = value;
}

/* Wymusza zakonczenie zapisow (odczyt STATUS). */
static void e1000e_flush(void)
{
    (void)e1000e_read_reg(E1000E_STATUS);
}

/* Opoznienie w mikrosekundach. TSC jesli skalibrowany, inaczej zapas przez outb 0x80. */
static void e100e_delay_us(uint32_t us)
{
    if (e1000e_tsc_per_us) {
        uint64_t end = e1000e_rdtsc() + (uint64_t)us * e1000e_tsc_per_us;
        while (e1000e_rdtsc() < end) __asm__ volatile("pause");
        return;
    }
    while (us--) outb(0x80, 0);
}

/* ------------------------------------------------------------------ */
/* Wlasnosc MDIO (tylko PCH: firmware/ME tez uzywa PHY).              */
/* ------------------------------------------------------------------ */

static int e1000e_swflag_acquire(void)
{
    uint32_t t;

    if (!E1000E_IS_PCH) return 1;

    for (t = 0; t < 1000; t++) {
        if (!(e1000e_read_reg(E1000E_EXTCNF_CTRL) & E1000E_EXTCNF_SWFLAG)) break;
        e100e_delay_us(1000);
    }
    if (t == 1000) return 0;

    e1000e_write_reg(E1000E_EXTCNF_CTRL,
                     e1000e_read_reg(E1000E_EXTCNF_CTRL) | E1000E_EXTCNF_SWFLAG);
    for (t = 0; t < 1000; t++) {
        if (e1000e_read_reg(E1000E_EXTCNF_CTRL) & E1000E_EXTCNF_SWFLAG) return 1;
        e100e_delay_us(1000);
    }
    e1000e_write_reg(E1000E_EXTCNF_CTRL,
                     e1000e_read_reg(E1000E_EXTCNF_CTRL) & ~E1000E_EXTCNF_SWFLAG);
    return 0;
}

static void e1000e_swflag_release(void)
{
    if (!E1000E_IS_PCH) return;
    e1000e_write_reg(E1000E_EXTCNF_CTRL,
                     e1000e_read_reg(E1000E_EXTCNF_CTRL) & ~E1000E_EXTCNF_SWFLAG);
}

/* ------------------------------------------------------------------ */
/* MDIC / PHY (e1000e.mdio).                                          */
/* ------------------------------------------------------------------ */

#define E1000E_MDIC_POLLS    200U   /* 200 * 50 us = 10 ms (wczesniej 100 ms) */

/* Dostep bez blokady SWFLAG - wolajacy odpowiada za wlasnosc MDIO. */
static uint16_t e1000e_mdio_read_raw(uint8_t phy_addr, uint8_t reg)
{
    uint32_t cmd = E1000E_MDIC_READ |
                   ((uint32_t)phy_addr << E1000E_MDIC_PHY_SHIFT) |
                   ((uint32_t)reg << E1000E_MDIC_REG_SHIFT);
    uint32_t i;
    e1000e_write_reg(E1000E_MDIC, cmd);
    for (i = 0; i < E1000E_MDIC_POLLS; i++) {
        cmd = e1000e_read_reg(E1000E_MDIC);
        if (cmd & E1000E_MDIC_READY) {
            if (cmd & E1000E_MDIC_ERROR) return 0xFFFFU;
            return (uint16_t)(cmd & E1000E_MDIC_DATA_MASK);
        }
        e100e_delay_us(50);
    }
    return 0xFFFFU;
}

static void e1000e_mdio_write_raw(uint8_t phy_addr, uint8_t reg, uint16_t value)
{
    uint32_t cmd = E1000E_MDIC_WRITE |
                   ((uint32_t)phy_addr << E1000E_MDIC_PHY_SHIFT) |
                   ((uint32_t)reg << E1000E_MDIC_REG_SHIFT) |
                   (uint32_t)value;
    uint32_t i;
    e1000e_write_reg(E1000E_MDIC, cmd);
    for (i = 0; i < E1000E_MDIC_POLLS; i++) {
        if (e1000e_read_reg(E1000E_MDIC) & E1000E_MDIC_READY) return;
        e100e_delay_us(50);
    }
}

static uint16_t e1000e_mdio_read(uint8_t phy_addr, uint8_t reg)
{
    uint16_t v;
    if (!e1000e_swflag_acquire()) return 0xFFFFU;
    v = e1000e_mdio_read_raw(phy_addr, reg);
    e1000e_swflag_release();
    return v;
}

static void e1000e_mdio_write(uint8_t phy_addr, uint8_t reg, uint16_t value)
{
    if (!e1000e_swflag_acquire()) return;
    e1000e_mdio_write_raw(phy_addr, reg, value);
    e1000e_swflag_release();
}

/*
 * PHY HV (82577/82578): przed innymi dostepami do MDIO wlacz wolny tryb MDIO
 * (strona 769, rejestr 16, bit 10). Rejestry stron >= 768 sa pod adresem 1.
 */
static void e1000e_pch_mdio_slow(void)
{
    uint16_t v;

    if (e1000e_type != E1000E_TYPE_PCHLAN) return;
    if (!e1000e_swflag_acquire()) return;

    e1000e_mdio_write_raw(1, PHY_PAGE_SELECT, (uint16_t)(HV_KMRN_MODE_PAGE << 5));
    v = e1000e_mdio_read_raw(1, HV_KMRN_MODE_REG);
    if (v != 0xFFFFU)
        e1000e_mdio_write_raw(1, HV_KMRN_MODE_REG, (uint16_t)(v | HV_KMRN_MDIO_SLOW));
    e1000e_mdio_write_raw(1, PHY_PAGE_SELECT, 0);

    e1000e_swflag_release();
}

static int e1000e_phy_id_valid(uint8_t addr)
{
    uint16_t id1 = e1000e_mdio_read(addr, MII_PHYSID1);
    uint16_t id2;
    if (id1 == 0xFFFFU || id1 == 0) return 0;   /* nie czytaj drugiego rejestru bez potrzeby */
    id2 = e1000e_mdio_read(addr, MII_PHYSID2);
    return id2 != 0xFFFFU && id2 != 0;
}

/* PCH: strony 0..15 PHY HV pod adresem 2, potem 1; reszta: skan 0..31. */
static int e1000e_phy_find(uint8_t *phy_addr)
{
    uint32_t i;

    if (E1000E_IS_PCH) {
        static const uint8_t first[2] = { 2, 1 };
        for (i = 0; i < 2; i++) {
            if (e1000e_phy_id_valid(first[i])) {
                *phy_addr = first[i];
                return 1;
            }
        }
    } else {
        /* 82574: PHY jest pod adresem 1 - sprawdz go najpierw. */
        if (e1000e_phy_id_valid(1)) {
            *phy_addr = 1;
            return 1;
        }
    }
    for (i = 0; i < 32; i++) {
        if (e1000e_phy_id_valid((uint8_t)i)) {
            *phy_addr = (uint8_t)i;
            return 1;
        }
    }
    return 0;
}

/* Wlacza autonegocjacje i restartuje ja. */
static void e1000e_phy_autoneg(void)
{
    e1000e_mdio_write(e1000e_phy_addr, MII_ADVERTISE, 0x01E1U);
    e1000e_mdio_write(e1000e_phy_addr, MII_CTRL1000, 0x0300U);
    e1000e_mdio_write(e1000e_phy_addr, MII_BMCR, 0x1200U);
}

/* Programowy reset PHY (tylko 82574; na PCH zachowujemy stan z BIOS). */
static void e1000e_phy_soft_reset(void)
{
    uint32_t i;
    e1000e_mdio_write(e1000e_phy_addr, MII_BMCR, 0x8000U);
    for (i = 0; i < 500; i++) {
        if (!(e1000e_mdio_read(e1000e_phy_addr, MII_BMCR) & 0x8000U)) break;
        e100e_delay_us(1000);
    }
}

/* Cykl zasilania PHY przez LANPHYPC, gdy PHY nie odpowiada na MDIO. */
static void e1000e_pch_toggle_lanphypc(void)
{
    uint32_t v, t;

    if (e1000e_type >= E1000E_TYPE_PCH_LPT) {
        v = e1000e_read_reg(E1000E_FEXTNVM3);
        v &= ~E1000E_PHY_CFG_COUNTER_MASK;
        v |= E1000E_PHY_CFG_COUNTER_50MSEC;
        e1000e_write_reg(E1000E_FEXTNVM3, v);
    }

    v = e1000e_read_reg(E1000E_CTRL);
    v |= E1000E_CTRL_LANPHYPC_OVERRIDE;
    v &= ~E1000E_CTRL_LANPHYPC_VALUE;
    e1000e_write_reg(E1000E_CTRL, v);
    e1000e_flush();
    e100e_delay_us(20);

    v &= ~E1000E_CTRL_LANPHYPC_OVERRIDE;
    e1000e_write_reg(E1000E_CTRL, v);
    e1000e_flush();

    if (e1000e_type < E1000E_TYPE_PCH_LPT) {
        e100e_delay_us(50000);
    } else {
        for (t = 0; t < 100; t++) {
            e100e_delay_us(5000);
            if (e1000e_read_reg(E1000E_CTRL_EXT) & E1000E_CTRL_EXT_LPCD) break;
        }
        e100e_delay_us(30000);
    }

    /* Zdejmij wymuszony tryb SMBus, inaczej MDIO jest niedostepne. */
    v = e1000e_read_reg(E1000E_CTRL_EXT);
    if (v & E1000E_CTRL_EXT_FORCE_SMBUS)
        e1000e_write_reg(E1000E_CTRL_EXT, v & ~E1000E_CTRL_EXT_FORCE_SMBUS);
}

/* K1 wylaczone dla 82577/82578 (znany problem z linkiem gigabitowym). */
static void e1000e_pch_k1_disable(void)
{
    uint32_t v, ctrl, ctrl_ext;
    uint32_t sel = (E1000E_KMRN_K1_CONFIG << E1000E_KMRN_OFFSET_SHIFT) & E1000E_KMRN_OFFSET_MASK;

    if (e1000e_type != E1000E_TYPE_PCHLAN) return;
    if (!e1000e_swflag_acquire()) return;

    e1000e_write_reg(E1000E_KMRNCTRLSTA, sel | E1000E_KMRN_REN);
    e1000e_flush();
    e100e_delay_us(2);
    v = e1000e_read_reg(E1000E_KMRNCTRLSTA) & 0xFFFFU;
    v &= ~E1000E_KMRN_K1_ENABLE;
    e1000e_write_reg(E1000E_KMRNCTRLSTA, sel | v);
    e1000e_flush();
    e100e_delay_us(20);

    ctrl = e1000e_read_reg(E1000E_CTRL);
    ctrl_ext = e1000e_read_reg(E1000E_CTRL_EXT);
    ctrl &= ~(E1000E_CTRL_SPD_1000 | E1000E_CTRL_SPD_100);
    e1000e_write_reg(E1000E_CTRL, ctrl | E1000E_CTRL_FRCSPD);
    e1000e_write_reg(E1000E_CTRL_EXT, ctrl_ext | E1000E_CTRL_EXT_SPD_BYPS);
    e1000e_flush();
    e100e_delay_us(20);

    e1000e_write_reg(E1000E_CTRL, ctrl);
    e1000e_write_reg(E1000E_CTRL_EXT, ctrl_ext);
    e1000e_flush();
    e100e_delay_us(20);

    e1000e_swflag_release();
}

/* Link: STATUS.LU (MAC) albo, gdy PHY odpowiada, BMSR z zakonczona autonegocjacja. */
static int e1000e_link_ok(void)
{
    uint16_t bmsr;

    if (e1000e_read_reg(E1000E_STATUS) & E1000E_STATUS_LU) return 1;
    if (!e1000e_phy_ok) return 0;

    (void)e1000e_mdio_read(e1000e_phy_addr, MII_BMSR);
    bmsr = e1000e_mdio_read(e1000e_phy_addr, MII_BMSR);
    if (bmsr == 0xFFFFU) return 0;
    return (bmsr & 0x0004U) && (bmsr & 0x0020U);
}

/* ------------------------------------------------------------------ */
/* Wykrywanie urzadzenia PCI e1000e.                                  */
/* ------------------------------------------------------------------ */

static int e1000e_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            uint8_t type;
            if (vendor == 0xFFFF) continue;
            if (vendor != E1000E_VENDOR) continue;

            switch (device) {
            case E1000E_DEV_82574L:  type = E1000E_TYPE_82574;   break;
            case E1000E_DEV_82578DM:
            case E1000E_DEV_82578DC: type = E1000E_TYPE_PCHLAN;  break;
            case E1000E_DEV_82579LM:
            case E1000E_DEV_82579V:  type = E1000E_TYPE_PCH2LAN; break;
            case E1000E_DEV_I217LM:
            case E1000E_DEV_I217V:   type = E1000E_TYPE_PCH_LPT; break;
            default: continue;
            }

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || (bar0 & 1)) continue;
            /* 64-bitowy BAR powyzej 4 GiB jest nieosiagalny w trybie 32-bitowym. */
            if ((bar0 & 6U) == 4U && pci_read32((uint8_t)bus, slot, 0, 0x14) != 0) continue;

            /* Tylko dolne 16 bitow (komenda); gorne to status z bitami W1C. */
            uint32_t cmd = pci_read32((uint8_t)bus, slot, 0, 0x04) & 0xFFFFU;
            pci_write32((uint8_t)bus, slot, 0, 0x04, cmd | 0x00000006U);
            e1000e_mmio = (volatile uint8_t *)(bar0 & 0xFFFFFFF0U);
            e1000e_type = type;
            return 1;
        }
    }
    return 0;
}

static int e1000e_read_mac(void)
{
    uint32_t low = e1000e_read_reg(E1000E_RAL);
    uint32_t high = e1000e_read_reg(E1000E_RAH);
    if ((low == 0xFFFFFFFFU && high == 0xFFFFFFFFU) || low == 0 ||
        !(high & (1U << 31))) return 0;
    e1000e_mac[0] = (uint8_t)low;
    e1000e_mac[1] = (uint8_t)(low >> 8);
    e1000e_mac[2] = (uint8_t)(low >> 16);
    e1000e_mac[3] = (uint8_t)(low >> 24);
    e1000e_mac[4] = (uint8_t)high;
    e1000e_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Inicjalizacja sprzetu.                                             */
/* ------------------------------------------------------------------ */

/* Zatrzymaj RX/TX i zamaskuj przerwania (bez resetu). */
static void e1000e_quiesce(void)
{
    e1000e_write_reg(E1000E_IMC, 0xFFFFFFFFU);
    e1000e_write_reg(E1000E_RCTL, 0);
    e1000e_write_reg(E1000E_TCTL, E1000E_TCTL_PSP);
    e1000e_flush();
    e100e_delay_us(10000);
    (void)e1000e_read_reg(E1000E_ICR);
}

/* Pelny reset MAC (82574). Na PCH go nie uzywamy - patrz naglowek pliku. */
static int e1000e_reset(void)
{
    uint32_t t;

    e1000e_quiesce();

    e1000e_write_reg(E1000E_CTRL, e1000e_read_reg(E1000E_CTRL) | E1000E_CTRL_RST);
    e100e_delay_us(1000);
    for (t = 0; t < 10000; t++) {
        if (!(e1000e_read_reg(E1000E_CTRL) & E1000E_CTRL_RST)) break;
        e100e_delay_us(10);
    }
    if (t == 10000) return 0;
    e100e_delay_us(10000);

    e1000e_write_reg(E1000E_IMC, 0xFFFFFFFFU);
    (void)e1000e_read_reg(E1000E_ICR);
    return 1;
}

static void e1000e_set_link_up(void)
{
    uint32_t ctrl = e1000e_read_reg(E1000E_CTRL);
    /* Predkosc i duplex wykrywa MAC sam (ASDE), nie wymuszamy ich. */
    ctrl &= ~(E1000E_CTRL_FRCSPD | E1000E_CTRL_FRCDPX | E1000E_CTRL_ILOS);
    ctrl |= E1000E_CTRL_SLU | E1000E_CTRL_ASDE;
    e1000e_write_reg(E1000E_CTRL, ctrl);
    e1000e_flush();
}

static void e1000e_init_rings(void)
{
    uint32_t i, t;

    /* Przed zmiana adresow pierscieni zatrzymaj kolejki (stan po BIOS/PXE). */
    e1000e_write_reg(E1000E_RXDCTL, e1000e_read_reg(E1000E_RXDCTL) & ~E1000E_XDCTL_ENABLE);
    e1000e_write_reg(E1000E_TXDCTL, e1000e_read_reg(E1000E_TXDCTL) & ~E1000E_XDCTL_ENABLE);
    for (t = 0; t < 1000; t++) {
        if (!(e1000e_read_reg(E1000E_RXDCTL) & E1000E_XDCTL_ENABLE) &&
            !(e1000e_read_reg(E1000E_TXDCTL) & E1000E_XDCTL_ENABLE)) break;
        e100e_delay_us(10);
    }

    /* Bez opoznien/moderacji przerwan i bez offloadu sum kontrolnych RX:
     * stan po BIOS/PXE moze miec je wlaczone, co psuje polling i gubi UDP. */
    e1000e_write_reg(E1000E_RDTR, 0);
    e1000e_write_reg(E1000E_RADV, 0);
    e1000e_write_reg(E1000E_TIDV, 0);
    e1000e_write_reg(E1000E_TADV, 0);
    e1000e_write_reg(E1000E_ITR, 0);
    e1000e_write_reg(E1000E_RXCSUM, 0);
    e1000e_write_reg(E1000E_RFCTL, 0);

    for (i = 0; i < E1000E_RX_COUNT; i++) {
        e1000e_rx_desc[i].addr_low = (uint32_t)&e1000e_rx_buf[i][0];
        e1000e_rx_desc[i].addr_high = 0;
        e1000e_rx_desc[i].length = 0;
        e1000e_rx_desc[i].cso = 0;
        e1000e_rx_desc[i].cmd = 0;
        e1000e_rx_desc[i].status = 0;
        e1000e_rx_desc[i].css = 0;
        e1000e_rx_desc[i].special = 0;
    }
    e1000e_write_reg(E1000E_RDBAL, (uint32_t)&e1000e_rx_desc[0]);
    e1000e_write_reg(E1000E_RDBAH, 0);
    e1000e_write_reg(E1000E_RDLEN, sizeof(e1000e_rx_desc));
    e1000e_write_reg(E1000E_RDH, 0);
    /* Sprzet posiada deskryptory [head, tail), wiec tail = RX_COUNT-1. */
    e1000e_write_reg(E1000E_RDT, E1000E_RX_COUNT - 1);

    for (i = 0; i < E1000E_TX_COUNT; i++) {
        e1000e_tx_desc[i].addr_low = (uint32_t)&e1000e_tx_buf[i][0];
        e1000e_tx_desc[i].addr_high = 0;
        e1000e_tx_desc[i].length = 0;
        e1000e_tx_desc[i].cso = 0;
        e1000e_tx_desc[i].cmd = 0;
        e1000e_tx_desc[i].status = E1000E_TXD_STAT_DD;
        e1000e_tx_desc[i].css = 0;
        e1000e_tx_desc[i].special = 0;
    }
    e1000e_write_reg(E1000E_TDBAL, (uint32_t)&e1000e_tx_desc[0]);
    e1000e_write_reg(E1000E_TDBAH, 0);
    e1000e_write_reg(E1000E_TDLEN, sizeof(e1000e_tx_desc));
    e1000e_write_reg(E1000E_TDH, 0);
    e1000e_write_reg(E1000E_TDT, 0);

    e1000e_rx_head = 0;
    e1000e_tx_head = 0;
    barrier();

    /* Wlacz kolejki RX/TX. Jawnie ustawiamy progi: zapis deskryptora po kazdym pakiecie. */
    e1000e_write_reg(E1000E_RXDCTL, E1000E_XDCTL_ENABLE | E1000E_XDCTL_GRAN |
                                    E1000E_XDCTL_WTHRESH1);
    e1000e_write_reg(E1000E_TXDCTL, E1000E_XDCTL_ENABLE | E1000E_XDCTL_GRAN |
                                    E1000E_XDCTL_WTHRESH1 | E1000E_TXDCTL_COUNT_DESC);
    for (t = 0; t < 1000; t++) {
        if ((e1000e_read_reg(E1000E_RXDCTL) & E1000E_XDCTL_ENABLE) &&
            (e1000e_read_reg(E1000E_TXDCTL) & E1000E_XDCTL_ENABLE)) break;
        e100e_delay_us(10);
    }
}

static void e1000e_init_mac(void)
{
    uint32_t i;

    e1000e_write_reg(E1000E_RCTL, 0);
    e1000e_write_reg(E1000E_TCTL, 0);

    /* RAL = bajty 0..3, RAH = bajty 4..5 + bit AV. */
    e1000e_write_reg(E1000E_RAL, (uint32_t)e1000e_mac[0] |
                                 ((uint32_t)e1000e_mac[1] << 8) |
                                 ((uint32_t)e1000e_mac[2] << 16) |
                                 ((uint32_t)e1000e_mac[3] << 24));
    e1000e_write_reg(E1000E_RAH, (uint32_t)e1000e_mac[4] |
                                 ((uint32_t)e1000e_mac[5] << 8) | (1U << 31));

    /* Wyczysc tablice multicast (stan po BIOS/PXE moze byc dowolny). */
    for (i = 0; i < 128; i++) e1000e_write_reg(E1000E_MTA + i * 4U, 0);
    e1000e_flush();
}

static void e1000e_apply_link(void)
{
    uint32_t tctl = 0;

    e1000e_pch_k1_disable();

    e1000e_write_reg(E1000E_RCTL, 0);
    e1000e_write_reg(E1000E_TCTL, 0);
    for (uint32_t t = 0; t < 10000; t++) {
        if (!(e1000e_read_reg(E1000E_RCTL) & E1000E_RCTL_EN) &&
            !(e1000e_read_reg(E1000E_TCTL) & E1000E_TCTL_EN)) break;
        e100e_delay_us(10);
    }

    e1000e_set_link_up();
    e1000e_write_reg(E1000E_TIPG, E1000E_TIPG_COPPER);

    tctl = E1000E_TCTL_EN | E1000E_TCTL_PSP |
           (0x0FU << E1000E_TCTL_CT_SHIFT) |
           (0x3FU << E1000E_TCTL_COLD_SHIFT) |
           E1000E_TCTL_RTLC;
    e1000e_write_reg(E1000E_TCTL, tctl);

    /* Bufor 2048 B (BSIZE=00, BSEX=0), bez ramek blednych i jumbo. */
    e1000e_write_reg(E1000E_RCTL, E1000E_RCTL_EN | E1000E_RCTL_UPE |
                                  E1000E_RCTL_MPE | E1000E_RCTL_BAM |
                                  E1000E_RCTL_SECRC);

    barrier();
    /* tail = indeks przed pierwszym deskryptorem do odczytu przez sterownik. */
    e1000e_write_reg(E1000E_RDT,
                     (e1000e_rx_head + E1000E_RX_COUNT - 1U) % E1000E_RX_COUNT);
    e1000e_link = 1;
}

/* Gdy link jest -> co jakis czas sprawdz, czy nie zniknal; gdy go nie ma -> sprobuj go podniesc. */
static void e1000e_link_poll(void)
{
    static uint32_t poll_count = 0;

    if ((++poll_count & 0x3FFU) != 0) return;

    if (e1000e_link) {
        if (!(e1000e_read_reg(E1000E_STATUS) & E1000E_STATUS_LU) && !e1000e_link_ok())
            e1000e_link = 0;
        return;
    }
    if (e1000e_link_ok()) e1000e_apply_link();
}

/* Zwraca deskryptor RX sterownikowi: tail = indeks wlasnie przetworzonego. */
static void e1000e_rx_recycle(void)
{
    struct e1000e_desc *desc = &e1000e_rx_desc[e1000e_rx_head];

    desc->length = 0;
    desc->cso = 0;
    desc->cmd = 0;
    desc->status = 0;
    desc->css = 0;
    desc->special = 0;
    barrier();
    e1000e_write_reg(E1000E_RDT, e1000e_rx_head);
    e1000e_rx_head = (e1000e_rx_head + 1U) % E1000E_RX_COUNT;
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int e1000e_init(void)
{
    uint32_t i;
    int pch;

    if (e1000e_ready) return 1;
    e1000e_error = E1000E_ERR_NONE;
    e1000e_link = 0;

    e1000e_calibrate_tsc();

    if (!e1000e_find_device()) { e1000e_error = E1000E_ERR_NO_DEVICE; return 0; }
    pch = E1000E_IS_PCH;

    if (pch) {
        e1000e_quiesce();
    } else if (!e1000e_reset()) {
        e1000e_error = E1000E_ERR_RESET;
        return 0;
    }

    /* Poinformuj firmware, ze sterownik jest zaladowany. */
    e1000e_write_reg(E1000E_CTRL_EXT,
                     e1000e_read_reg(E1000E_CTRL_EXT) | E1000E_CTRL_EXT_DRV_LOAD);

    if (!e1000e_read_mac()) { e1000e_error = E1000E_ERR_MAC; return 0; }

    if (pch) e1000e_pch_mdio_slow();
    e1000e_phy_ok = (uint8_t)e1000e_phy_find(&e1000e_phy_addr);
    if (!e1000e_phy_ok && pch) {
        e1000e_pch_toggle_lanphypc();
        e1000e_pch_mdio_slow();
        e1000e_phy_ok = (uint8_t)e1000e_phy_find(&e1000e_phy_addr);
    }
    if (!e1000e_phy_ok) e1000e_error = E1000E_ERR_NO_PHY;

    e1000e_set_link_up();
    if (e1000e_phy_ok && !pch) {
        e1000e_phy_soft_reset();
        e1000e_phy_autoneg();
    }

    e1000e_init_rings();
    e1000e_init_mac();

    /* Czekaj na link (do ok. 3 s). */
    for (i = 0; i < 3000 && !e1000e_link; i++) {
        if (e1000e_link_ok()) e1000e_apply_link();
        else e100e_delay_us(1000);
    }

    /* PCH: jesli BIOS nie zostawil dzialajacego linku, uruchom autonegocjacje. */
    if (!e1000e_link && e1000e_phy_ok && pch) {
        e1000e_phy_autoneg();
        for (i = 0; i < 5000 && !e1000e_link; i++) {
            if (e1000e_link_ok()) e1000e_apply_link();
            else e100e_delay_us(1000);
        }
    }

    if (!e1000e_link && e1000e_error == E1000E_ERR_NONE)
        e1000e_error = E1000E_ERR_NO_LINK;

    e1000e_ready = 1;
    return 1;
}

/* Kod ostatniego problemu z inicjalizacji (E1000E_ERR_*), do diagnostyki. */
int e1000e_last_error(void)
{
    return (int)e1000e_error;
}

int e1000e_is_ready(void)
{
    return e1000e_ready;
}

int e1000e_get_mac(uint8_t mac[6])
{
    if (!e1000e_ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = e1000e_mac[i];
    return 1;
}

int e1000e_send(const void *frame, uint16_t length)
{
    const uint8_t *src = (const uint8_t *)frame;
    volatile struct e1000e_desc *desc;
    uint32_t i, t, idx, send_len;

    if (!e1000e_ready || !frame || length == 0 || length > E1000E_MAX_FRAME) return -1;

    e1000e_link_poll();
    if (!e1000e_link) { e1000e_stats.tx_dropped++; return 0; }

    idx = e1000e_tx_head;
    desc = &e1000e_tx_desc[idx];
    barrier();
    if (!(desc->status & E1000E_TXD_STAT_DD)) {
        e1000e_stats.tx_dropped++;
        return 0;
    }

    for (i = 0; i < length; i++) e1000e_tx_buf[idx][i] = src[i];
    send_len = length;
    while (send_len < E1000E_MIN_FRAME) e1000e_tx_buf[idx][send_len++] = 0;

    desc->addr_low = (uint32_t)&e1000e_tx_buf[idx][0];
    desc->addr_high = 0;
    desc->length = (uint16_t)send_len;
    desc->cso = 0;
    desc->cmd = E1000E_TXD_CMD_EOP | E1000E_TXD_CMD_IFCS | E1000E_TXD_CMD_RS;
    desc->status = 0;
    desc->css = 0;
    desc->special = 0;
    barrier();

    e1000e_tx_head = (e1000e_tx_head + 1U) % E1000E_TX_COUNT;
    e1000e_write_reg(E1000E_TDT, e1000e_tx_head);

    for (t = 0; t < 1000000U; t++) {
        barrier();
        if (desc->status & E1000E_TXD_STAT_DD) break;
    }
    if (t == 1000000U) {
        e1000e_stats.tx_errors++;
        return 0;
    }

    e1000e_stats.tx_packets++;
    return 1;
}

int e1000e_receive(void *frame, uint16_t capacity)
{
    uint8_t *dst = (uint8_t *)frame;
    volatile struct e1000e_desc *desc;
    uint32_t i, len;
    uint8_t status, errors;

    if (!e1000e_ready || !frame || capacity == 0) return -1;

    e1000e_link_poll();
    if (!e1000e_link) return 0;

    desc = &e1000e_rx_desc[e1000e_rx_head];
    barrier();
    status = desc->status;
    if (!(status & E1000E_RXD_STAT_DD)) return 0;
    barrier();

    len = desc->length;
    errors = desc->css;       /* w deskryptorze RX to pole "errors" */

    /* Odrzucaj tylko prawdziwie uszkodzone ramki; bity sum kontrolnych ignorujemy. */
    if (!(status & E1000E_RXD_STAT_EOP) || (errors & E1000E_RXD_ERR_FATAL)) {
        e1000e_stats.rx_errors++;
        e1000e_rx_recycle();
        return 0;
    }

    if (len < 14 || len > (uint32_t)E1000E_MAX_FRAME) {
        e1000e_stats.rx_errors++;
        e1000e_rx_recycle();
        return 0;
    }

    if (len > (uint32_t)capacity) {
        e1000e_stats.rx_dropped++;
        e1000e_rx_recycle();
        return 0;
    }

    for (i = 0; i < len; i++) dst[i] = e1000e_rx_buf[e1000e_rx_head][i];
    e1000e_stats.rx_packets++;
    e1000e_rx_recycle();
    return (int)len;
}

void e1000e_shutdown(void)
{
    if (!e1000e_ready) return;
    e1000e_write_reg(E1000E_RCTL, 0);
    e1000e_write_reg(E1000E_TCTL, 0);
    e1000e_write_reg(E1000E_IMC, 0xFFFFFFFFU);
    e1000e_write_reg(E1000E_CTRL_EXT,
                     e1000e_read_reg(E1000E_CTRL_EXT) & ~E1000E_CTRL_EXT_DRV_LOAD);
    e1000e_link = 0;
    e1000e_ready = 0;
}

int e1000e_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                     uint32_t *tx_errors, uint32_t *rx_errors,
                     uint32_t *tx_dropped, uint32_t *rx_dropped)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = e1000e_stats.tx_packets;
    *rx_packets = e1000e_stats.rx_packets;
    *tx_errors = e1000e_stats.tx_errors;
    *rx_errors = e1000e_stats.rx_errors;
    if (tx_dropped) *tx_dropped = e1000e_stats.tx_dropped;
    if (rx_dropped) *rx_dropped = e1000e_stats.rx_dropped;
    return 1;
}

void e1000e_stats_reset(void)
{
    e1000e_stats.tx_packets = 0;
    e1000e_stats.rx_packets = 0;
    e1000e_stats.tx_errors = 0;
    e1000e_stats.rx_errors = 0;
    e1000e_stats.tx_dropped = 0;
    e1000e_stats.rx_dropped = 0;
}

int e1000e_set_mtu(uint16_t mtu)
{
    if (!e1000e_ready || mtu < 68 || mtu > 9000) return 0;
    e1000e_mtu = mtu;
    return 1;
}

uint16_t e1000e_get_mtu(void)
{
    return e1000e_mtu;
}

int e1000e_link_up(void)
{
    return e1000e_link ? 1 : 0;
}