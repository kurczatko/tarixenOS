#include "rtl8111f.h"

static inline uint8_t rtl8111f_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t rtl8111f_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t rtl8111f_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void rtl8111f_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void rtl8111f_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void rtl8111f_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t rtl8111f_pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFCU);
    rtl8111f_outl(0xCF8U, address);
    return rtl8111f_inl(0xCFCU);
}

static void rtl8111f_pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                                 uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFCU);
    rtl8111f_outl(0xCF8U, address);
    rtl8111f_outl(0xCFCU, value);
}

static volatile uint8_t *rtl8111f_mmio = 0;
static uint8_t rtl8111f_bus = 0;
static uint8_t rtl8111f_slot = 0;
static uint8_t rtl8111f_func = 0;
static uint32_t rtl8111f_mmio_base = 0;
static uint8_t rtl8111f_mac[6];
static uint8_t rtl8111f_ready = 0;
static uint8_t rtl8111f_link = 0;
static uint8_t rtl8111f_full_duplex = 0;
static uint8_t rtl8111f_autoneg_done = 0;
static uint16_t rtl8111f_speed = 0;
static uint32_t rtl8111f_link_changes = 0;
static uint32_t rtl8111f_autoneg_failures = 0;

static uint8_t rtl8111f_rx_buf[RTL8111F_RX_BUF_SIZE + 16] __attribute__((aligned(16)));
static uint8_t rtl8111f_tx_bufs[RTL8111F_TX_DESC_COUNT][RTL8111F_TX_BUF_SIZE] __attribute__((aligned(16)));
static uint32_t rtl8111f_rx_ptr = 0;
static uint32_t rtl8111f_tx_next = 0;
static uint32_t rtl8111f_tx_pending = 0;

static struct rtl8111f_stats rtl8111f_stats = {};

static inline uint32_t rtl8111f_mmio_read32(uint16_t offset)
{
    return *(volatile uint32_t *)(rtl8111f_mmio + offset);
}

static inline void rtl8111f_mmio_write32(uint16_t offset, uint32_t value)
{
    *(volatile uint32_t *)(rtl8111f_mmio + offset) = value;
}

static inline uint16_t rtl8111f_mmio_read16(uint16_t offset)
{
    return *(volatile uint16_t *)(rtl8111f_mmio + offset);
}

static inline void rtl8111f_mmio_write16(uint16_t offset, uint16_t value)
{
    *(volatile uint16_t *)(rtl8111f_mmio + offset) = value;
}

static inline uint8_t rtl8111f_mmio_read8(uint16_t offset)
{
    return *(volatile uint8_t *)(rtl8111f_mmio + offset);
}

static inline void rtl8111f_mmio_write8(uint16_t offset, uint8_t value)
{
    *(volatile uint8_t *)(rtl8111f_mmio + offset) = value;
}

static void rtl8111f_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile uint32_t j = 0; j < 48U; j++);
    }
}

static int rtl8111f_find_device(void)
{
    for (uint16_t bus = 0; bus < 256U; bus++) {
        for (uint8_t slot = 0; slot < 32U; slot++) {
            for (uint8_t func = 0; func < 8U; func++) {
                uint32_t id0 = rtl8111f_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (vendor != RTL8111F_VENDOR_ID) continue;
                if (device != RTL8111F_DEVICE_ID &&
                    device != RTL8111F_DEVICE_ID_VAR1 &&
                    device != RTL8111F_DEVICE_ID_VAR2 &&
                    device != RTL8111F_DEVICE_ID_VAR3 &&
                    device != RTL8111F_DEVICE_ID_VAR4) continue;

                uint32_t bar0 = rtl8111f_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || (bar0 & 1U)) continue;

                uint32_t command = rtl8111f_pci_read32(bus, slot, func, 0x04U);
                rtl8111f_pci_write32(bus, slot, func, 0x04U, command | 0x00000006U);

                rtl8111f_bus = bus;
                rtl8111f_slot = slot;
                rtl8111f_func = func;
                rtl8111f_mmio_base = bar0 & 0xFFFFFFF0U;
                rtl8111f_mmio = (volatile uint8_t *)rtl8111f_mmio_base;
                return 1;
            }
        }
    }
    return 0;
}

static int rtl8111f_mac_read(void)
{
    uint32_t low = rtl8111f_mmio_read32(RTL8111F_REG_MAC0);
    uint32_t high = rtl8111f_mmio_read32(RTL8111F_REG_MAC4);
    if (low == 0xFFFFFFFFU || low == 0U || (high & 0xFFFFU) == 0U) return 0;
    rtl8111f_mac[0] = (uint8_t)low;
    rtl8111f_mac[1] = (uint8_t)(low >> 8);
    rtl8111f_mac[2] = (uint8_t)(low >> 16);
    rtl8111f_mac[3] = (uint8_t)(low >> 24);
    rtl8111f_mac[4] = (uint8_t)high;
    rtl8111f_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

static int rtl8111f_reset(void)
{
    rtl8111f_mmio_write8(RTL8111F_REG_CMDR,
                         (uint8_t)(RTL8111F_CMD_STOP | RTL8111F_CMD_RESET_TX | RTL8111F_CMD_RESET_RX));
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(rtl8111f_mmio_read8(RTL8111F_REG_CMDR) &
              (RTL8111F_CMD_RESET_TX | RTL8111F_CMD_RESET_RX))) break;
    }
    if (rtl8111f_mmio_read8(RTL8111F_REG_CMDR) &
        (RTL8111F_CMD_RESET_TX | RTL8111F_CMD_RESET_RX)) return 0;

    rtl8111f_mmio_write8(RTL8111F_REG_CMDR, (uint8_t)RTL8111F_CMD_STOP);
    rtl8111f_delay_us(100U);
    return 1;
}

static uint16_t rtl8111f_phy_read(uint8_t reg)
{
    rtl8111f_mmio_write32(RTL8111F_REG_PHYAR,
                          (1U << 31U) | ((uint32_t)(reg & 0xFFU) << 16U));
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(rtl8111f_mmio_read32(RTL8111F_REG_PHYAR) & (1U << 31U))) break;
    }
    return (uint16_t)(rtl8111f_mmio_read32(RTL8111F_REG_PHYAR) & 0xFFFFU);
}

static void rtl8111f_phy_write(uint8_t reg, uint16_t value)
{
    rtl8111f_mmio_write32(RTL8111F_REG_PHYAR,
                          (1U << 31U) | (1U << 30U) |
                          ((uint32_t)(reg & 0xFFU) << 16U) | (uint32_t)(value & 0xFFFFU));
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(rtl8111f_mmio_read32(RTL8111F_REG_PHYAR) & (1U << 31U))) break;
    }
}

static int rtl8111f_update_link(void)
{
    uint16_t bmsr = rtl8111f_phy_read(RTL8111F_PHY_REG_BMSR);
    uint8_t up = (uint8_t)((bmsr & RTL8111F_BMSR_LINK) ? 1U : 0U);
    if (up && !rtl8111f_link) {
        rtl8111f_link = 1U;
        rtl8111f_link_changes++;
        uint16_t bmcr = rtl8111f_phy_read(RTL8111F_PHY_REG_BMCR);
        rtl8111f_full_duplex = (uint8_t)((bmcr & RTL8111F_BMCR_DUPLEX) ? 1U : 0U);
        rtl8111f_autoneg_done = (uint8_t)((bmsr & RTL8111F_BMSR_AN_DONE) ? 1U : 0U);
        if (bmsr & RTL8111F_BMSR_1000FD) rtl8111f_speed = 1000U;
        else if (bmsr & (RTL8111F_BMSR_100FD | RTL8111F_BMSR_100HD)) rtl8111f_speed = 100U;
        else rtl8111f_speed = 10U;
    } else if (!up && rtl8111f_link) {
        rtl8111f_link = 0U;
        rtl8111f_link_changes++;
        rtl8111f_full_duplex = 0U;
        rtl8111f_autoneg_done = 0U;
        rtl8111f_speed = 0U;
    }
    return rtl8111f_link ? 1 : 0;
}

int rtl8111f_init(void)
{
    if (rtl8111f_ready) return 1;
    if (!rtl8111f_find_device()) return 0;

    rtl8111f_mmio_write8(RTL8111F_REG_CFG9346, 0x00U);
    if (!rtl8111f_reset()) return 0;
    if (!rtl8111f_mac_read()) return 0;

    rtl8111f_mmio_write16(RTL8111F_REG_ISR, 0xFFFFU);
    rtl8111f_mmio_write16(RTL8111F_REG_IMR, 0x0000U);

    rtl8111f_mmio_write32(RTL8111F_REG_TCR, 0x00000000U);

    rtl8111f_mmio_write32(RTL8111F_REG_RCR,
                          (uint32_t)(RTL8111F_RCR_ACCEPT_MYMAC |
                                     RTL8111F_RCR_ACCEPT_BROADCAST |
                                     RTL8111F_RCR_ACCEPT_MULTICAST |
                                     RTL8111F_RCR_ACCEPT_PHYS |
                                     RTL8111F_RCR_BUFFER_16K));

    rtl8111f_mmio_write32(RTL8111F_REG_RBSTART, (uint32_t)&rtl8111f_rx_buf[0]);

    for (uint32_t i = 0U; i < RTL8111F_TX_DESC_COUNT; i++) {
        rtl8111f_mmio_write32(RTL8111F_REG_TSAD0 + i * 4U,
                              (uint32_t)&rtl8111f_tx_bufs[i][0]);
        rtl8111f_mmio_write32(RTL8111F_REG_TSD0 + i * 4U, 0U);
    }

    rtl8111f_rx_ptr = 0;
    rtl8111f_tx_next = 0;
    rtl8111f_tx_pending = 0;
    rtl8111f_link = 0U;
    rtl8111f_full_duplex = 0U;
    rtl8111f_autoneg_done = 0U;
    rtl8111f_speed = 0U;
    rtl8111f_stats = {};

    rtl8111f_mmio_write8(RTL8111F_REG_CMDR,
                         (uint8_t)(RTL8111F_CMD_START | RTL8111F_CMD_TX_ENABLE | RTL8111F_CMD_RX_ENABLE));

    rtl8111f_ready = 1;
    return 1;
}

int rtl8111f_is_ready(void)
{
    return rtl8111f_ready;
}

int rtl8111f_get_mac(uint8_t mac[6])
{
    if (!rtl8111f_ready || !mac) return 0;
    for (uint32_t i = 0U; i < 6U; i++) mac[i] = rtl8111f_mac[i];
    return 1;
}

int rtl8111f_send(const void *frame, uint16_t length)
{
    if (!rtl8111f_ready || !frame || length == 0 ||
        length > RTL8111F_TX_BUF_SIZE) return -1;

    uint32_t index = rtl8111f_tx_next % RTL8111F_TX_DESC_COUNT;

    for (uint32_t wait = 0U; wait < 2000000U; wait++) {
        if (!(rtl8111f_mmio_read32(RTL8111F_REG_TSD0 + index * 4U) & RTL8111F_TSD_OWN)) break;
    }
    if (rtl8111f_mmio_read32(RTL8111F_REG_TSD0 + index * 4U) & RTL8111F_TSD_OWN) {
        rtl8111f_stats.tx_errors++;
        return 0;
    }

    for (uint32_t i = 0U; i < (uint32_t)length; i++) {
        rtl8111f_tx_bufs[index][i] = ((const uint8_t *)frame)[i];
    }
    __asm__ volatile("" : : : "memory");

    rtl8111f_mmio_write32(RTL8111F_REG_TSAD0 + index * 4U,
                          (uint32_t)&rtl8111f_tx_bufs[index][0]);
    rtl8111f_mmio_write32(RTL8111F_REG_TSD0 + index * 4U,
                          (uint32_t)(RTL8111F_TSD_OWN |
                                     ((uint32_t)length & RTL8111F_TSD_LENGTH_MASK)));
    rtl8111f_tx_next++;
    rtl8111f_tx_pending++;

    for (uint32_t wait = 0U; wait < 2000000U; wait++) {
        if (!(rtl8111f_mmio_read32(RTL8111F_REG_TSD0 + index * 4U) & RTL8111F_TSD_OWN)) break;
    }

    uint32_t tsd = rtl8111f_mmio_read32(RTL8111F_REG_TSD0 + index * 4U);
    if (tsd & RTL8111F_TSD_TX_UNDERRUN) rtl8111f_stats.tx_underrun++;
    if (tsd & RTL8111F_TSD_TX_ABORTED) rtl8111f_stats.tx_aborted++;
    if (tsd & RTL8111F_TSD_TX_CARRIER) rtl8111f_stats.tx_carrier++;

    rtl8111f_tx_pending--;
    rtl8111f_stats.tx_packets++;
    rtl8111f_stats.tx_bytes += (uint32_t)length;
    return 1;
}

int rtl8111f_receive(void *frame, uint16_t capacity)
{
    if (!rtl8111f_ready || !frame || capacity == 0) return -1;

    uint16_t isr = rtl8111f_mmio_read16(RTL8111F_REG_ISR);
    if (!(isr & RTL8111F_ISR_RX_OK)) return 0;

    rtl8111f_mmio_write16(RTL8111F_REG_ISR,
                          (uint16_t)(RTL8111F_ISR_RX_OK | RTL8111F_ISR_RX_ERR));

    uint32_t offset = rtl8111f_rx_ptr % RTL8111F_RX_BUF_SIZE;

    uint16_t status = (uint16_t)(rtl8111f_rx_buf[offset] |
                                 (rtl8111f_rx_buf[offset + 1] << 8));
    (void)status;
    uint16_t length = (uint16_t)(rtl8111f_rx_buf[offset + 2] |
                                 (rtl8111f_rx_buf[offset + 3] << 8));

    if (length == 0 || length > capacity ||
        length > (uint16_t)(RTL8111F_RX_BUF_SIZE - 4U)) {
        rtl8111f_stats.rx_errors++;
        rtl8111f_rx_ptr = (offset + 4U) % RTL8111F_RX_BUF_SIZE;
        return 0;
    }

    for (uint32_t i = 0U; i < (uint32_t)length; i++) {
        ((uint8_t *)frame)[i] = rtl8111f_rx_buf[(offset + 4U + i) % RTL8111F_RX_BUF_SIZE];
    }

    rtl8111f_rx_ptr = (offset + 4U + (uint32_t)length + 3U) & ~3U;
    rtl8111f_rx_ptr %= RTL8111F_RX_BUF_SIZE;

    rtl8111f_stats.rx_packets++;
    rtl8111f_stats.rx_bytes += (uint32_t)length;
    return (int)length;
}

int rtl8111f_link_up(void)
{
    if (!rtl8111f_ready) return 0;
    return rtl8111f_update_link();
}

int rtl8111f_link_info(struct rtl8111f_link_info *info)
{
    if (!info) return 0;
    if (!rtl8111f_ready) return 0;
    rtl8111f_update_link();
    info->link_up = rtl8111f_link;
    info->full_duplex = rtl8111f_full_duplex;
    info->autoneg_done = rtl8111f_autoneg_done;
    info->speed_mbps = rtl8111f_speed;
    return 1;
}

int rtl8111f_speed_mbps(void)
{
    if (!rtl8111f_ready) return 0;
    rtl8111f_update_link();
    return (int)rtl8111f_speed;
}

int rtl8111f_duplex_full(void)
{
    if (!rtl8111f_ready) return 0;
    rtl8111f_update_link();
    return rtl8111f_full_duplex ? 1 : 0;
}

int rtl8111f_autonegotiate(void)
{
    if (!rtl8111f_ready) return 0;
    rtl8111f_phy_write(RTL8111F_PHY_REG_BMCR,
                       (uint16_t)(RTL8111F_BMCR_RESET | RTL8111F_BMCR_AUTONEG));
    rtl8111f_delay_us(100U);
    for (uint32_t wait = 0U; wait < 2000000U; wait++) {
        uint16_t bmsr = rtl8111f_phy_read(RTL8111F_PHY_REG_BMSR);
        if ((bmsr & RTL8111F_BMSR_AN_DONE) && (bmsr & RTL8111F_BMSR_LINK)) {
            rtl8111f_update_link();
            return 1;
        }
    }
    rtl8111f_update_link();
    rtl8111f_autoneg_failures++;
    return rtl8111f_link ? 1 : 0;
}

int rtl8111f_reset_link(void)
{
    if (!rtl8111f_ready) return 0;
    rtl8111f_phy_write(RTL8111F_PHY_REG_BMCR, (uint16_t)RTL8111F_BMCR_RESET);
    rtl8111f_delay_us(100U);
    return rtl8111f_autonegotiate();
}

int rtl8111f_loopback_test(const void *frame, uint16_t length)
{
    if (!rtl8111f_ready || !frame || length == 0 ||
        length > RTL8111F_TX_BUF_SIZE) return -1;

    uint32_t tcr = rtl8111f_mmio_read32(RTL8111F_REG_TCR);
    rtl8111f_mmio_write32(RTL8111F_REG_TCR, tcr | 0x00000001U);

    if (rtl8111f_send(frame, length) <= 0) {
        rtl8111f_mmio_write32(RTL8111F_REG_TCR, tcr);
        return 0;
    }

    uint8_t result[RTL8111F_TX_BUF_SIZE];
    int received = rtl8111f_receive(result, RTL8111F_TX_BUF_SIZE);
    rtl8111f_mmio_write32(RTL8111F_REG_TCR, tcr);

    if (received != (int)length) return 0;
    for (uint32_t i = 0U; i < (uint32_t)length; i++) {
        if (result[i] != ((const uint8_t *)frame)[i]) return 0;
    }
    return 1;
}

void rtl8111f_set_multicast(const uint8_t *addresses, uint32_t count)
{
    (void)addresses;
    (void)count;
    uint32_t hash = 0xFFFFFFFFU;
    rtl8111f_mmio_write32(RTL8111F_REG_MAR0, hash);
    rtl8111f_mmio_write32(RTL8111F_REG_MAR0 + 4U, hash);
}

void rtl8111f_stats_get(struct rtl8111f_stats *out)
{
    if (!out) return;
    *out = rtl8111f_stats;
}

void rtl8111f_stats_reset(void)
{
    rtl8111f_stats = {};
}

void rtl8111f_shutdown(void)
{
    if (!rtl8111f_ready) return;
    rtl8111f_mmio_write8(RTL8111F_REG_CMDR,
                         (uint8_t)(RTL8111F_CMD_STOP | RTL8111F_CMD_RESET_TX | RTL8111F_CMD_RESET_RX));
    rtl8111f_ready = 0;
    rtl8111f_link = 0U;
}
