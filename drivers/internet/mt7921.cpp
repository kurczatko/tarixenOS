/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/internet/mt7921.h>

static inline uint8_t mt7921_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t mt7921_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t mt7921_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void mt7921_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void mt7921_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void mt7921_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t mt7921_pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFCU);
    mt7921_outl(0xCF8U, address);
    return mt7921_inl(0xCFCU);
}

static void mt7921_pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                                uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFCU);
    mt7921_outl(0xCF8U, address);
    mt7921_outl(0xCFCU, value);
}

enum mt7921_reg {
    MT7921_REG_MAC_ADDR      = 0x0000U,
    MT7921_REG_CHIP_ID       = 0x0008U,
    MT7921_REG_CFG           = 0x0010U,
    MT7921_REG_INT_STATUS    = 0x0020U,
    MT7921_REG_INT_MASK      = 0x0024U,
    MT7921_REG_TX_DESC_BASE  = 0x0100U,
    MT7921_REG_RX_DESC_BASE  = 0x0180U,
    MT7921_REG_TX_STATUS     = 0x0200U,
    MT7921_REG_RX_STATUS     = 0x0300U,
    MT7921_REG_WIFI_CTRL     = 0x0400U,
    MT7921_REG_WIFI_STATUS   = 0x0404U,
    MT7921_REG_CHANNEL       = 0x0408U,
    MT7921_REG_BAND          = 0x040CU,
    MT7921_REG_SSID          = 0x0410U,
    MT7921_REG_SSID_LEN      = 0x0430U,
    MT7921_REG_AUTH          = 0x0434U,
    MT7921_REG_RSSI          = 0x0438U,
    MT7921_REG_BSSID         = 0x0440U,
    MT7921_REG_SCAN_CTRL     = 0x0450U,
    MT7921_REG_SCAN_RESULT   = 0x0454U,
    MT7921_REG_POWER_MGMT    = 0x0460U,
    MT7921_REG_MULTICAST     = 0x0500U,
    MT7921_REG_STATS_TX_PKT  = 0x0600U,
    MT7921_REG_STATS_TX_BYTE = 0x0604U,
    MT7921_REG_STATS_TX_ERR  = 0x0608U,
    MT7921_REG_STATS_RX_PKT  = 0x060CU,
    MT7921_REG_STATS_RX_BYTE = 0x0610U,
    MT7921_REG_STATS_RX_ERR  = 0x0614U,
};

#define MT7921_CFG_RESET        (1U << 0)
#define MT7921_CFG_TX_ENABLE    (1U << 1)
#define MT7921_CFG_RX_ENABLE    (1U << 2)
#define MT7921_CFG_RADIO_ON     (1U << 3)

#define MT7921_INT_RX_OK        0x0001U
#define MT7921_INT_TX_OK        0x0002U
#define MT7921_INT_RX_ERR       0x0004U
#define MT7921_INT_TX_ERR       0x0008U
#define MT7921_INT_SCAN_DONE    0x0010U
#define MT7921_INT_ASSOC        0x0020U

#define MT7921_WIFI_CTRL_RADIO_ENABLE  (1U << 0)
#define MT7921_WIFI_CTRL_ASSOC         (1U << 1)
#define MT7921_WIFI_CTRL_POWER_SAVE    (1U << 2)
#define MT7921_WIFI_CTRL_SCAN          (1U << 3)

#define MT7921_WIFI_STATUS_LINK_UP     (1U << 0)
#define MT7921_WIFI_STATUS_ASSOCIATED  (1U << 1)
#define MT7921_WIFI_STATUS_SCAN_DONE   (1U << 2)

#define MT7921_AUTH_SUCCESS   1U
#define MT7921_AUTH_FAILED    2U

#define MT7921_BAND_2GHZ      0U
#define MT7921_BAND_5GHZ      1U

#define MT7921_TX_DESC_OWN      (1U << 31)
#define MT7921_TX_DESC_LENGTH_MASK 0x0000FFFFU
#define MT7921_RX_DESC_OWN      (1U << 31)
#define MT7921_RX_DESC_LENGTH_MASK 0x0000FFFFU

#define MTK_VENDOR_ID     0x14C3U
#define MTK_DEVICE_MT7921 0x0616U
#define MTK_DEVICE_MT7921_VAR1 0x7916U

#define MT7921_RX_BUF_SIZE 32768U
#define MT7921_TX_BUF_SIZE 2048U
#define MT7921_TX_DESC_COUNT 8U
#define MT7921_RX_DESC_COUNT 16U

static volatile uint8_t *mt7921_mmio = 0;
static uint8_t mt7921_bus = 0;
static uint8_t mt7921_slot = 0;
static uint8_t mt7921_func = 0;
static uint32_t mt7921_mmio_base = 0;
static uint8_t mt7921_mac[6];
static uint8_t mt7921_ready = 0;
static uint8_t mt7921_link = 0;
static uint8_t mt7921_associated = 0;
static uint8_t mt7921_full_duplex = 0;
static uint8_t mt7921_autoneg_done = 0;
static uint16_t mt7921_speed = 0;
static int32_t mt7921_rssi_val = 0;
static uint8_t mt7921_channel = 1;
static uint8_t mt7921_band = MT7921_BAND_2GHZ;
static uint32_t mt7921_link_changes = 0;
static uint32_t mt7921_auth_failures = 0;
static uint32_t mt7921_scan_results = 0;

static uint8_t mt7921_rx_buf[MT7921_RX_BUF_SIZE + 16] __attribute__((aligned(16)));
static uint8_t mt7921_tx_bufs[MT7921_TX_DESC_COUNT][MT7921_TX_BUF_SIZE] __attribute__((aligned(16)));
static uint32_t mt7921_rx_ptr = 0;
static uint32_t mt7921_tx_next = 0;

static struct mt7921_stats mt7921_stats = {};

static inline uint32_t mt7921_mmio_read32(uint16_t offset)
{
    return *(volatile uint32_t *)(mt7921_mmio + offset);
}

static inline void mt7921_mmio_write32(uint16_t offset, uint32_t value)
{
    *(volatile uint32_t *)(mt7921_mmio + offset) = value;
}

static inline uint16_t mt7921_mmio_read16(uint16_t offset)
{
    return *(volatile uint16_t *)(mt7921_mmio + offset);
}

static inline void mt7921_mmio_write16(uint16_t offset, uint16_t value)
{
    *(volatile uint16_t *)(mt7921_mmio + offset) = value;
}

static inline uint8_t mt7921_mmio_read8(uint16_t offset)
{
    return *(volatile uint8_t *)(mt7921_mmio + offset);
}

static inline void mt7921_mmio_write8(uint16_t offset, uint8_t value)
{
    *(volatile uint8_t *)(mt7921_mmio + offset) = value;
}

static void mt7921_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (uint32_t j = 0; j < 48U; j++);
    }
}

static int mt7921_find_device(void)
{
    for (uint16_t bus = 0; bus < 256U; bus++) {
        for (uint8_t slot = 0; slot < 32U; slot++) {
            for (uint8_t func = 0; func < 8U; func++) {
                uint32_t id0 = mt7921_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (vendor != MTK_VENDOR_ID) continue;
                if (device != MTK_DEVICE_MT7921 && device != MTK_DEVICE_MT7921_VAR1) continue;

                uint32_t bar0 = mt7921_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || (bar0 & 1U)) continue;

                uint32_t command = mt7921_pci_read32(bus, slot, func, 0x04U);
                mt7921_pci_write32(bus, slot, func, 0x04U, command | 0x00000006U);

                mt7921_bus = bus;
                mt7921_slot = slot;
                mt7921_func = func;
                mt7921_mmio_base = bar0 & 0xFFFFFFF0U;
                mt7921_mmio = (volatile uint8_t *)mt7921_mmio_base;
                return 1;
            }
        }
    }
    return 0;
}

static int mt7921_mac_read(void)
{
    uint32_t low = mt7921_mmio_read32(MT7921_REG_MAC_ADDR);
    uint32_t high = mt7921_mmio_read32(MT7921_REG_MAC_ADDR + 4U);
    if (low == 0xFFFFFFFFU || low == 0U || (high & 0xFFFFU) == 0U) return 0;
    mt7921_mac[0] = (uint8_t)low;
    mt7921_mac[1] = (uint8_t)(low >> 8);
    mt7921_mac[2] = (uint8_t)(low >> 16);
    mt7921_mac[3] = (uint8_t)(low >> 24);
    mt7921_mac[4] = (uint8_t)high;
    mt7921_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

static int mt7921_reset(void)
{
    mt7921_mmio_write8(MT7921_REG_CFG, (uint8_t)MT7921_CFG_RESET);
    for (uint32_t wait = 0; wait < 200000U; wait++) {
        if (!(mt7921_mmio_read8(MT7921_REG_CFG) & MT7921_CFG_RESET)) break;
    }
    if (mt7921_mmio_read8(MT7921_REG_CFG) & MT7921_CFG_RESET) return 0;

    mt7921_mmio_write8(MT7921_REG_CFG, 0x00U);
    mt7921_delay_us(100U);
    return 1;
}

static int mt7921_radio_enable(void)
{
    mt7921_mmio_write8(MT7921_REG_WIFI_CTRL,
                       (uint8_t)(MT7921_WIFI_CTRL_RADIO_ENABLE | MT7921_WIFI_CTRL_ASSOC));
    mt7921_delay_us(200U);

    for (uint32_t wait = 0; wait < 1000000U; wait++) {
        uint8_t status = mt7921_mmio_read8(MT7921_REG_WIFI_STATUS);
        if (status & MT7921_WIFI_STATUS_LINK_UP) {
            mt7921_link = 1U;
            mt7921_associated = 1U;
            break;
        }
    }

    mt7921_channel = 1U;
    mt7921_band = MT7921_BAND_2GHZ;
    mt7921_mmio_write8(MT7921_REG_CHANNEL, mt7921_channel);
    mt7921_mmio_write8(MT7921_REG_BAND, mt7921_band);

    return 1;
}

static int mt7921_update_link(void)
{
    if (!mt7921_ready) return 0;
    uint8_t status = mt7921_mmio_read8(MT7921_REG_WIFI_STATUS);
    uint8_t up = (uint8_t)((status & MT7921_WIFI_STATUS_LINK_UP) ? 1U : 0U);
    if (up && !mt7921_link) {
        mt7921_link = 1U;
        mt7921_link_changes++;
        mt7921_associated = (uint8_t)((status & MT7921_WIFI_STATUS_ASSOCIATED) ? 1U : 0U);
        mt7921_speed = 866U;
        mt7921_full_duplex = 1U;
        mt7921_rssi_val = -45;
    } else if (!up && mt7921_link) {
        mt7921_link = 0U;
        mt7921_link_changes++;
        mt7921_associated = 0U;
        mt7921_speed = 0U;
        mt7921_full_duplex = 0U;
        mt7921_rssi_val = 0;
    }
    return mt7921_link ? 1 : 0;
}

int mt7921_init(void)
{
    if (mt7921_ready) return 1;
    if (!mt7921_find_device()) return 0;

    mt7921_mmio_write8(MT7921_REG_CFG, 0x00U);
    if (!mt7921_reset()) return 0;
    if (!mt7921_mac_read()) return 0;

    mt7921_mmio_write16(MT7921_REG_INT_STATUS, 0xFFFFU);
    mt7921_mmio_write16(MT7921_REG_INT_MASK, 0x0000U);

    mt7921_mmio_write32(MT7921_REG_CFG,
                        (uint32_t)(MT7921_CFG_TX_ENABLE | MT7921_CFG_RX_ENABLE |
                                   MT7921_CFG_RADIO_ON));

    if (!mt7921_radio_enable()) return 0;

    mt7921_mmio_write32(MT7921_REG_RX_DESC_BASE, (uint32_t)&mt7921_rx_buf[0]);

    for (uint32_t i = 0U; i < MT7921_TX_DESC_COUNT; i++) {
        mt7921_mmio_write32(MT7921_REG_TX_DESC_BASE + i * 4U,
                            (uint32_t)&mt7921_tx_bufs[i][0]);
        mt7921_mmio_write32(MT7921_REG_TX_STATUS + i * 4U, 0U);
    }

    mt7921_rx_ptr = 0;
    mt7921_tx_next = 0;
    mt7921_link = 0U;
    mt7921_associated = 0U;
    mt7921_full_duplex = 0U;
    mt7921_autoneg_done = 0U;
    mt7921_speed = 0U;
    mt7921_channel = 1U;
    mt7921_band = MT7921_BAND_2GHZ;
    mt7921_rssi_val = 0;
    mt7921_link_changes = 0;
    mt7921_auth_failures = 0;
    mt7921_scan_results = 0;
    mt7921_stats = {};

    mt7921_ready = 1;
    return 1;
}

int mt7921_is_ready(void)
{
    return mt7921_ready;
}

int mt7921_get_mac(uint8_t mac[6])
{
    if (!mt7921_ready || !mac) return 0;
    for (uint32_t i = 0U; i < 6U; i++) mac[i] = mt7921_mac[i];
    return 1;
}

int mt7921_send(const void *frame, uint16_t length)
{
    if (!mt7921_ready || !frame || length == 0 ||
        length > MT7921_TX_BUF_SIZE) return -1;

    uint32_t index = mt7921_tx_next % MT7921_TX_DESC_COUNT;

    for (uint32_t wait = 0U; wait < 2000000U; wait++) {
        if (!(mt7921_mmio_read32(MT7921_REG_TX_STATUS + index * 4U) & MT7921_TX_DESC_OWN)) break;
    }
    if (mt7921_mmio_read32(MT7921_REG_TX_STATUS + index * 4U) & MT7921_TX_DESC_OWN) {
        mt7921_stats.tx_errors++;
        return 0;
    }

    for (uint32_t i = 0U; i < (uint32_t)length; i++) {
        mt7921_tx_bufs[index][i] = ((const uint8_t *)frame)[i];
    }
    __asm__ volatile("" : : : "memory");

    mt7921_mmio_write32(MT7921_REG_TX_STATUS + index * 4U,
                        (uint32_t)(MT7921_TX_DESC_OWN |
                                   ((uint32_t)length & MT7921_TX_DESC_LENGTH_MASK)));
    mt7921_tx_next++;

    for (uint32_t wait = 0U; wait < 2000000U; wait++) {
        if (!(mt7921_mmio_read32(MT7921_REG_TX_STATUS + index * 4U) & MT7921_TX_DESC_OWN)) break;
    }

    uint32_t tsd = mt7921_mmio_read32(MT7921_REG_TX_STATUS + index * 4U);
    if (tsd & 0x80000000U) {
    }

    mt7921_stats.tx_packets++;
    mt7921_stats.tx_bytes += (uint32_t)length;
    return 1;
}

int mt7921_receive(void *frame, uint16_t capacity)
{
    if (!mt7921_ready || !frame || capacity == 0) return -1;

    uint16_t isr = mt7921_mmio_read16(MT7921_REG_INT_STATUS);
    if (!(isr & MT7921_INT_RX_OK)) return 0;

    mt7921_mmio_write16(MT7921_REG_INT_STATUS,
                        (uint16_t)(MT7921_INT_RX_OK | MT7921_INT_RX_ERR));

    uint32_t offset = mt7921_rx_ptr % MT7921_RX_BUF_SIZE;

    uint16_t status = (uint16_t)(mt7921_rx_buf[offset] |
                                 (mt7921_rx_buf[offset + 1] << 8));
    (void)status;
    uint16_t length = (uint16_t)(mt7921_rx_buf[offset + 2] |
                                 (mt7921_rx_buf[offset + 3] << 8));

    if (length == 0 || length > capacity ||
        length > (uint16_t)(MT7921_RX_BUF_SIZE - 4U)) {
        mt7921_stats.rx_errors++;
        mt7921_rx_ptr = (offset + 4U) % MT7921_RX_BUF_SIZE;
        return 0;
    }

    for (uint32_t i = 0U; i < (uint32_t)length; i++) {
        ((uint8_t *)frame)[i] = mt7921_rx_buf[(offset + 4U + i) % MT7921_RX_BUF_SIZE];
    }

    mt7921_rx_ptr = (offset + 4U + (uint32_t)length + 3U) & ~3U;
    mt7921_rx_ptr %= MT7921_RX_BUF_SIZE;

    mt7921_stats.rx_packets++;
    mt7921_stats.rx_bytes += (uint32_t)length;
    return (int)length;
}

int mt7921_link_up(void)
{
    if (!mt7921_ready) return 0;
    return mt7921_update_link();
}

int mt7921_speed_mbps(void)
{
    if (!mt7921_ready) return 0;
    mt7921_update_link();
    return (int)mt7921_speed;
}

int mt7921_rssi(void)
{
    if (!mt7921_ready) return 0;
    mt7921_update_link();
    int32_t hw_rssi = (int32_t)(int8_t)mt7921_mmio_read8(MT7921_REG_RSSI);
    if (hw_rssi != 0) mt7921_rssi_val = hw_rssi;
    return mt7921_rssi_val;
}

int mt7921_channel_get(void)
{
    if (!mt7921_ready) return 0;
    uint8_t ch = mt7921_mmio_read8(MT7921_REG_CHANNEL);
    if (ch != 0) mt7921_channel = ch;
    return (int)mt7921_channel;
}

int mt7921_channel_set(uint8_t channel)
{
    if (!mt7921_ready || channel == 0 || channel > 165) return 0;
    mt7921_mmio_write8(MT7921_REG_CHANNEL, channel);
    mt7921_channel = channel;
    mt7921_band = (channel <= 14) ? MT7921_BAND_2GHZ : MT7921_BAND_5GHZ;
    mt7921_mmio_write8(MT7921_REG_BAND, mt7921_band);
    return 1;
}

int mt7921_ssid_scan(void)
{
    if (!mt7921_ready) return 0;
    mt7921_mmio_write8(MT7921_REG_WIFI_CTRL,
                       (uint8_t)(MT7921_WIFI_CTRL_RADIO_ENABLE | MT7921_WIFI_CTRL_SCAN));
    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        uint8_t status = mt7921_mmio_read8(MT7921_REG_WIFI_STATUS);
        if (status & MT7921_WIFI_STATUS_SCAN_DONE) {
            mt7921_scan_results = mt7921_mmio_read32(MT7921_REG_SCAN_RESULT);
            mt7921_mmio_write16(MT7921_REG_INT_STATUS, MT7921_INT_SCAN_DONE);
            return 1;
        }
    }
    return 0;
}

int mt7921_autonegotiate(void)
{
    if (!mt7921_ready) return 0;
    mt7921_mmio_write8(MT7921_REG_WIFI_CTRL,
                       (uint8_t)(MT7921_WIFI_CTRL_RADIO_ENABLE | MT7921_WIFI_CTRL_ASSOC));
    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        uint8_t auth = mt7921_mmio_read8(MT7921_REG_AUTH);
        if (auth == MT7921_AUTH_SUCCESS) {
            mt7921_associated = 1U;
            mt7921_autoneg_done = 1U;
            mt7921_update_link();
            return 1;
        }
        if (auth == MT7921_AUTH_FAILED) {
            mt7921_auth_failures++;
            mt7921_autoneg_done = 0U;
            return 0;
        }
    }
    mt7921_auth_failures++;
    mt7921_autoneg_done = 0U;
    return mt7921_link ? 1 : 0;
}

int mt7921_reset_link(void)
{
    if (!mt7921_ready) return 0;
    mt7921_mmio_write8(MT7921_REG_WIFI_CTRL, 0x00U);
    mt7921_delay_us(100U);
    return mt7921_autonegotiate();
}

int mt7921_loopback_test(const void *frame, uint16_t length)
{
    if (!mt7921_ready || !frame || length == 0 ||
        length > MT7921_TX_BUF_SIZE) return -1;

    uint32_t cfg = mt7921_mmio_read32(MT7921_REG_CFG);
    mt7921_mmio_write32(MT7921_REG_CFG, cfg | 0x00000010U);

    if (mt7921_send(frame, length) <= 0) {
        mt7921_mmio_write32(MT7921_REG_CFG, cfg);
        return 0;
    }

    uint8_t result[MT7921_TX_BUF_SIZE];
    int received = mt7921_receive(result, MT7921_TX_BUF_SIZE);
    mt7921_mmio_write32(MT7921_REG_CFG, cfg);

    if (received != (int)length) return 0;
    for (uint32_t i = 0U; i < (uint32_t)length; i++) {
        if (result[i] != ((const uint8_t *)frame)[i]) return 0;
    }
    return 1;
}

void mt7921_set_multicast(const uint8_t *addresses, uint32_t count)
{
    (void)addresses;
    (void)count;
    uint32_t hash = 0xFFFFFFFFU;
    mt7921_mmio_write32(MT7921_REG_MULTICAST, hash);
    mt7921_mmio_write32(MT7921_REG_MULTICAST + 4U, hash);
}

void mt7921_stats_get(struct mt7921_stats *out)
{
    if (!out) return;
    *out = mt7921_stats;
}

void mt7921_stats_reset(void)
{
    mt7921_stats = {};
}

void mt7921_shutdown(void)
{
    if (!mt7921_ready) return;
    mt7921_mmio_write8(MT7921_REG_CFG,
                       (uint8_t)(MT7921_CFG_RESET | 0x00U));
    mt7921_ready = 0;
    mt7921_link = 0U;
    mt7921_associated = 0U;
}

void mt7921_poll(void)
{
    if (!mt7921_ready) return;
    mt7921_update_link();
    uint16_t isr = mt7921_mmio_read16(MT7921_REG_INT_STATUS);
    if (isr & (MT7921_INT_RX_OK | MT7921_INT_TX_OK |
               MT7921_INT_RX_ERR | MT7921_INT_TX_ERR |
               MT7921_INT_SCAN_DONE | MT7921_INT_ASSOC)) {
        mt7921_mmio_write16(MT7921_REG_INT_STATUS, (uint16_t)isr);
    }
}
