/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/internet/rtl8111e.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                        uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

enum rtl8111e_reg {
    REG_MAC0  = 0x00,
    REG_MAC4  = 0x04,
    REG_MAR0  = 0x08,
    REG_TSD0  = 0x10,
    REG_TSD1  = 0x14,
    REG_TSD2  = 0x18,
    REG_TSD3  = 0x1C,
    REG_TSAD0 = 0x20,
    REG_TSAD1 = 0x24,
    REG_TSAD2 = 0x28,
    REG_TSAD3 = 0x2C,
    REG_RBSTART = 0x30,
    REG_CMDR    = 0x37,
    REG_IMR     = 0x3C,
    REG_ISR     = 0x3E,
    REG_TCR     = 0x40,
    REG_RCR     = 0x44,
    REG_CFG9346 = 0x50,
    REG_PHYAR   = 0x6C,
};

#define CMD_STOP      0x01U
#define CMD_START     0x02U
#define CMD_RX_ENABLE 0x04U
#define CMD_TX_ENABLE 0x08U
#define CMD_RESET_RX  0x10U
#define CMD_RESET_TX  0x20U

#define ISR_TX_OK   0x0001U
#define ISR_RX_OK   0x0002U
#define ISR_RX_ERR  0x0040U
#define ISR_TX_ERR  0x0080U
#define ISR_PCI_ERR 0x4000U
#define ISR_SYS_ERR 0x8000U

#define RCR_ACCEPT_MYMAC    0x0010U
#define RCR_ACCEPT_PHYS     0x0020U
#define RCR_ACCEPT_MULTICAST 0x0008U
#define RCR_ACCEPT_BROADCAST 0x0004U
#define RCR_BUFFER_8K   0x0000U
#define RCR_BUFFER_16K  0x0800U
#define RCR_BUFFER_32K  0x1000U
#define RCR_BUFFER_64K  0x1800U

#define TSD_OWN         (1U << 31)
#define TSD_LENGTH_MASK 0x0000FFFFU

#define RTL_VENDOR_ID   0x10ECU
#define RTL_DEVICE_8111E 0x8168U
#define RTL_DEVICE_8168B 0x8168U
#define RTL_DEVICE_8169  0x8169U

#define RX_BUF_SIZE 16384U
#define TX_BUF_SIZE 2048U
#define TX_DESC_COUNT 4U

static volatile uint8_t *g_mmio = 0;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint8_t g_mac[6];
static uint8_t g_ready = 0;
static uint8_t g_link = 0;
static uint8_t g_full_duplex = 0;
static uint16_t g_speed = 1000;
static uint32_t g_link_changes = 0;

static uint8_t g_rx_buf[RX_BUF_SIZE + 16] __attribute__((aligned(16)));
static uint8_t g_tx_bufs[TX_DESC_COUNT][TX_BUF_SIZE] __attribute__((aligned(16)));
static uint32_t g_rx_ptr = 0;
static uint32_t g_tx_next = 0;

static struct rtl8111e_stats g_stats = {};

static inline uint32_t mmio_read32(uint16_t offset)
{
    return *(volatile uint32_t *)(g_mmio + offset);
}

static inline void mmio_write32(uint16_t offset, uint32_t value)
{
    *(volatile uint32_t *)(g_mmio + offset) = value;
}

static inline uint16_t mmio_read16(uint16_t offset)
{
    return *(volatile uint16_t *)(g_mmio + offset);
}

static inline void mmio_write16(uint16_t offset, uint16_t value)
{
    *(volatile uint16_t *)(g_mmio + offset) = value;
}

static inline uint8_t mmio_read8(uint16_t offset)
{
    return *(volatile uint8_t *)(g_mmio + offset);
}

static inline void mmio_write8(uint16_t offset, uint8_t value)
{
    *(volatile uint8_t *)(g_mmio + offset) = value;
}

static void rtl8111e_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (uint32_t j = 0; j < 48; j++);
    }
}

static int rtl8111e_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (vendor != RTL_VENDOR_ID) continue;
                if (device != RTL_DEVICE_8111E && device != RTL_DEVICE_8168B &&
                    device != RTL_DEVICE_8169) continue;

                uint32_t bar0 = pci_read32(bus, slot, func, 0x10);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || (bar0 & 1U)) continue;

                uint32_t command = pci_read32(bus, slot, func, 0x04);
                pci_write32(bus, slot, func, 0x04, command | 0x00000006U);

                g_bus = bus;
                g_slot = slot;
                g_func = func;
                g_mmio_base = bar0 & 0xFFFFFFF0U;
                g_mmio = (volatile uint8_t *)g_mmio_base;
                return 1;
            }
        }
    }
    return 0;
}

static int rtl8111e_mac_read(void)
{
    uint32_t low = mmio_read32(REG_MAC0);
    uint32_t high = mmio_read32(REG_MAC4);
    if (low == 0xFFFFFFFFU || low == 0U || (high & 0xFFFFU) == 0U) return 0;
    g_mac[0] = (uint8_t)low;
    g_mac[1] = (uint8_t)(low >> 8);
    g_mac[2] = (uint8_t)(low >> 16);
    g_mac[3] = (uint8_t)(low >> 24);
    g_mac[4] = (uint8_t)high;
    g_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

static int rtl8111e_reset(void)
{
    mmio_write8(REG_CMDR, (uint8_t)(CMD_STOP | CMD_RESET_TX | CMD_RESET_RX));
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(mmio_read8(REG_CMDR) & (CMD_RESET_TX | CMD_RESET_RX))) break;
    }
    if (mmio_read8(REG_CMDR) & (CMD_RESET_TX | CMD_RESET_RX)) return 0;
    mmio_write8(REG_CMDR, (uint8_t)CMD_STOP);
    rtl8111e_delay_us(100);
    return 1;
}

static uint16_t rtl8111e_phy_read(uint8_t reg)
{
    mmio_write32(REG_PHYAR, (1U << 31) | ((uint32_t)(reg & 0xFFU) << 16));
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(mmio_read32(REG_PHYAR) & (1U << 31))) break;
    }
    return (uint16_t)(mmio_read32(REG_PHYAR) & 0xFFFFU);
}

static void rtl8111e_phy_write(uint8_t reg, uint16_t value)
{
    mmio_write32(REG_PHYAR, (1U << 31) | (1U << 30) |
                   ((uint32_t)(reg & 0xFFU) << 16) | (uint32_t)(value & 0xFFFFU));
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(mmio_read32(REG_PHYAR) & (1U << 31))) break;
    }
}

#define PHY_REG_BMCR 0x00U
#define PHY_REG_BMSR 0x01U
#define BMCR_RESET   (1U << 15)
#define BMCR_AUTONEG (1U << 12)
#define BMSR_LINK    (1U << 2)
#define BMSR_AN_DONE (1U << 5)

static int rtl8111e_update_link(void)
{
    uint16_t bmsr = rtl8111e_phy_read(PHY_REG_BMSR);
    uint8_t up = (uint8_t)((bmsr & BMSR_LINK) ? 1 : 0);
    if (up && !g_link) {
        g_link = 1;
        g_link_changes++;
        uint16_t bmcr = rtl8111e_phy_read(PHY_REG_BMCR);
        g_full_duplex = (uint8_t)((bmcr & (1U << 8)) ? 1 : 0);
        g_speed = 1000;
    } else if (!up && g_link) {
        g_link = 0;
        g_link_changes++;
        g_full_duplex = 0;
        g_speed = 0;
    }
    return g_link ? 1 : 0;
}

int rtl8111e_link_up(void)
{
    if (!g_ready) return 0;
    return rtl8111e_update_link();
}

int rtl8111e_speed_mbps(void)
{
    if (!g_ready) return 0;
    rtl8111e_update_link();
    return (int)g_speed;
}

int rtl8111e_duplex_full(void)
{
    if (!g_ready) return 0;
    rtl8111e_update_link();
    return g_full_duplex ? 1 : 0;
}

int rtl8111e_autonegotiate(void)
{
    if (!g_ready) return 0;
    rtl8111e_phy_write(PHY_REG_BMCR, (uint16_t)(BMCR_RESET | BMCR_AUTONEG));
    rtl8111e_delay_us(100);
    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        uint16_t bmsr = rtl8111e_phy_read(PHY_REG_BMSR);
        if ((bmsr & BMSR_AN_DONE) && (bmsr & BMSR_LINK)) {
            rtl8111e_update_link();
            return 1;
        }
    }
    rtl8111e_update_link();
    return g_link ? 1 : 0;
}

int rtl8111e_reset_link(void)
{
    if (!g_ready) return 0;
    rtl8111e_phy_write(PHY_REG_BMCR, (uint16_t)BMCR_RESET);
    rtl8111e_delay_us(100);
    return rtl8111e_autonegotiate();
}

int rtl8111e_init(void)
{
    if (g_ready) return 1;
    if (!rtl8111e_find_device()) return 0;

    mmio_write8(REG_CFG9346, 0x00U);
    if (!rtl8111e_reset()) return 0;
    if (!rtl8111e_mac_read()) return 0;

    mmio_write16(REG_ISR, 0xFFFFU);
    mmio_write16(REG_IMR, 0x0000U);

    mmio_write32(REG_TCR, 0x00000000U);

    mmio_write32(REG_RCR,
                 (uint32_t)(RCR_ACCEPT_MYMAC | RCR_ACCEPT_BROADCAST |
                            RCR_ACCEPT_MULTICAST | RCR_ACCEPT_PHYS |
                            RCR_BUFFER_16K));

    mmio_write32(REG_RBSTART, (uint32_t)&g_rx_buf[0]);

    for (uint32_t i = 0; i < TX_DESC_COUNT; i++) {
        mmio_write32(REG_TSAD0 + i * 4U, (uint32_t)&g_tx_bufs[i][0]);
        mmio_write32(REG_TSD0 + i * 4U, 0U);
    }

    g_rx_ptr = 0;
    g_tx_next = 0;
    g_link = 0;
    g_full_duplex = 0;
    g_speed = 1000;
    g_stats = {};

    mmio_write8(REG_CMDR, (uint8_t)(CMD_START | CMD_TX_ENABLE | CMD_RX_ENABLE));

    g_ready = 1;
    return 1;
}

int rtl8111e_is_ready(void)
{
    return g_ready;
}

int rtl8111e_get_mac(uint8_t mac[6])
{
    if (!g_ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = g_mac[i];
    return 1;
}

int rtl8111e_send(const void *frame, uint16_t length)
{
    if (!g_ready || !frame || length == 0 || length > TX_BUF_SIZE) return -1;

    uint32_t index = g_tx_next % TX_DESC_COUNT;

    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        if (!(mmio_read32(REG_TSD0 + index * 4U) & TSD_OWN)) break;
    }
    if (mmio_read32(REG_TSD0 + index * 4U) & TSD_OWN) {
        g_stats.tx_errors++;
        return 0;
    }

    for (uint32_t i = 0; i < (uint32_t)length; i++) {
        g_tx_bufs[index][i] = ((const uint8_t *)frame)[i];
    }
    __asm__ volatile("" : : : "memory");

    mmio_write32(REG_TSAD0 + index * 4U, (uint32_t)&g_tx_bufs[index][0]);
    mmio_write32(REG_TSD0 + index * 4U,
                 (uint32_t)(TSD_OWN | ((uint32_t)length & TSD_LENGTH_MASK)));
    g_tx_next++;

    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        if (!(mmio_read32(REG_TSD0 + index * 4U) & TSD_OWN)) break;
    }

    g_stats.tx_packets++;
    g_stats.tx_bytes += (uint32_t)length;
    return 1;
}

int rtl8111e_receive(void *frame, uint16_t capacity)
{
    if (!g_ready || !frame || capacity == 0) return -1;

    uint16_t isr = mmio_read16(REG_ISR);
    if (!(isr & ISR_RX_OK)) return 0;

    mmio_write16(REG_ISR, (uint16_t)(ISR_RX_OK | ISR_RX_ERR));

    uint32_t offset = g_rx_ptr % RX_BUF_SIZE;

    uint16_t status = (uint16_t)(g_rx_buf[offset] | (g_rx_buf[offset + 1] << 8));
    uint16_t length = (uint16_t)(g_rx_buf[offset + 2] | (g_rx_buf[offset + 3] << 8));
    (void)status;

    if (length == 0 || length > capacity || length > (uint16_t)(RX_BUF_SIZE - 4U)) {
        g_stats.rx_errors++;
        g_rx_ptr = (offset + 4U) % RX_BUF_SIZE;
        return 0;
    }

    for (uint32_t i = 0; i < (uint32_t)length; i++) {
        ((uint8_t *)frame)[i] = g_rx_buf[(offset + 4U + i) % RX_BUF_SIZE];
    }

    g_rx_ptr = (offset + 4U + (uint32_t)length + 3U) & ~3U;
    g_rx_ptr %= RX_BUF_SIZE;

    g_stats.rx_packets++;
    g_stats.rx_bytes += (uint32_t)length;
    return (int)length;
}

int rtl8111e_loopback_test(const void *frame, uint16_t length)
{
    if (!g_ready || !frame || length == 0 || length > TX_BUF_SIZE) return -1;

    uint32_t tcr = mmio_read32(REG_TCR);
    mmio_write32(REG_TCR, tcr | 0x00000001U);

    if (rtl8111e_send(frame, length) <= 0) {
        mmio_write32(REG_TCR, tcr);
        return 0;
    }

    uint8_t result[TX_BUF_SIZE];
    int received = rtl8111e_receive(result, TX_BUF_SIZE);
    mmio_write32(REG_TCR, tcr);

    if (received != (int)length) return 0;
    for (uint32_t i = 0; i < (uint32_t)length; i++) {
        if (result[i] != ((const uint8_t *)frame)[i]) return 0;
    }
    return 1;
}

void rtl8111e_set_multicast(const uint8_t *addresses, uint32_t count)
{
    (void)addresses;
    (void)count;
    uint32_t hash = 0xFFFFFFFFU;
    mmio_write32(REG_MAR0, hash);
    mmio_write32(REG_MAR0 + 4U, hash);
}

void rtl8111e_stats_get(struct rtl8111e_stats *out)
{
    if (!out) return;
    *out = g_stats;
}
