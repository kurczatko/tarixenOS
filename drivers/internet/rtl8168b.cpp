/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/internet/rtl8168b.h>

/*funkcje i/o portow*/

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

/*odczyt i zapis konfiguracji PCI*/

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

/*rejestry MMIO RTL8168B*/

enum rtl8168b_reg {
    REG_MAC0      = 0x00,   /*adres MAC bajty 0-3*/
    REG_MAC4      = 0x04,   /*adres MAC bajty 4-5*/
    REG_MAR0      = 0x08,   /*adres multicast*/
    REG_TSD0      = 0x10,   /*deskryptor statusu nadawania 0*/
    REG_TSAD0     = 0x18,   /*adres nadawania 0*/
    REG_RBSTART   = 0x30,   /*adres bufora odbiorczego*/
    REG_CMDR      = 0x37,   /*komenda chipu*/
    REG_IMR       = 0x3C,   /*maska przerwan*/
    REG_ISR       = 0x3E,   /*status przerwan*/
    REG_TCR       = 0x40,   /*konfiguracja nadawania*/
    REG_RCR       = 0x44,   /*konfiguracja odbioru*/
    REG_CFG9346   = 0x50,   /*zapis rejestrow konfiguracyjnych*/
    REG_PHYAR     = 0x6C,   /*dostep do PHY*/
};

/*bity CMDR*/

#define CMD_RESET_TX   0x20U
#define CMD_RESET_RX   0x10U
#define CMD_TX_ENABLE  0x08U
#define CMD_RX_ENABLE  0x04U
#define CMD_START      0x02U
#define CMD_STOP       0x01U

/*bity ISR/IMR*/

#define ISR_RX_OK      0x0002U
#define ISR_TX_OK      0x0001U
#define ISR_RX_ERR     0x0040U
#define ISR_TX_ERR     0x0080U
#define ISR_PCI_ERR    0x4000U
#define ISR_SYSTEM_ERR 0x8000U

/*bity RCR*/

#define RCR_ACCEPT_ERR      0x0001U
#define RCR_ACCEPT_RUNT     0x0002U
#define RCR_ACCEPT_BROADCAST 0x0004U
#define RCR_ACCEPT_MULTICAST 0x0008U
#define RCR_ACCEPT_MYMAC    0x0010U
#define RCR_ACCEPT_PHYS     0x0020U
#define RCR_WRAP            0x0100U
#define RCR_BUFFER_8K       0x0000U   /*bity 12-11 = 00*/

/*bity TSD0*/

#define TSD_OWN      (1U << 31)   /*karta jest wlascicielem deskryptora (ustawia sterownik)*/
#define TSD_LENGTH_MASK 0x0000FFFFU

/*id urzadzen rodziny RTL8168B (Realtek vendor 0x10EC)*/

#define RTL_VENDOR_ID   0x10ECU
#define RTL_DEVICE_8168B 0x8168U
#define RTL_DEVICE_8168   0x8168U
#define RTL_DEVICE_8167   0x8167U
#define RTL_DEVICE_8169   0x8169U
#define RTL_DEVICE_8171   0x8171U
#define RTL_DEVICE_8172   0x8172U

#define RX_BUF_SIZE 16384U
#define TX_BUF_SIZE 2048U
#define TX_DESC_COUNT 4U

/*stan urzadzenia i bufory*/

static volatile uint8_t *g_mmio = 0;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint8_t g_mac[6];
static uint8_t g_ready = 0;

static uint8_t g_rx_buf[RX_BUF_SIZE + 16] __attribute__((aligned(16)));
static uint8_t g_tx_bufs[TX_DESC_COUNT][TX_BUF_SIZE] __attribute__((aligned(16)));
static uint32_t g_rx_ptr = 0;
static uint32_t g_tx_next = 0;

static struct rtl8168b_stats g_stats = {};

/*czytanie i zapis MMIO*/

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

/*opoznienie mikrosekundowe*/

static void rtl8168b_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

/*wyszukanie urzadzenia RTL8168B na PCI*/

static int rtl8168b_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != RTL_VENDOR_ID) continue;
                if (device != RTL_DEVICE_8168B && device != RTL_DEVICE_8167 &&
                    device != RTL_DEVICE_8169 && device != RTL_DEVICE_8171 &&
                    device != RTL_DEVICE_8172) continue;

                uint32_t bar0 = pci_read32(bus, slot, func, 0x10);
                if (bar0 == 0 || bar0 == 0xFFFFFFFFU || (bar0 & 1U)) continue;

                uint32_t command = pci_read32(bus, slot, func, 0x04);
                pci_write32(bus, slot, func, 0x04, command | 0x00000006);

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

/*odczyt adresu MAC z chipu*/

static int rtl8168b_mac_read(void)
{
    uint32_t low = mmio_read32(REG_MAC0);
    uint32_t high = mmio_read32(REG_MAC4);
    if (low == 0xFFFFFFFFU || low == 0 || (high & 0xFFFF) == 0) return 0;
    g_mac[0] = (uint8_t)low;
    g_mac[1] = (uint8_t)(low >> 8);
    g_mac[2] = (uint8_t)(low >> 16);
    g_mac[3] = (uint8_t)(low >> 24);
    g_mac[4] = (uint8_t)high;
    g_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

/*reset chipu zatrzymanie i reset TX/RX*/

static int rtl8168b_reset(void)
{
    mmio_write8(REG_CMDR, (uint8_t)(CMD_STOP | CMD_RESET_TX | CMD_RESET_RX));
    for (uint32_t wait = 0; wait < 100000; wait++) {
        if (!(mmio_read8(REG_CMDR) & (CMD_RESET_TX | CMD_RESET_RX))) break;
    }
    if (mmio_read8(REG_CMDR) & (CMD_RESET_TX | CMD_RESET_RX)) return 0;

    mmio_write8(REG_CMDR, (uint8_t)CMD_STOP);
    rtl8168b_delay_us(100);
    return 1;
}

/*inicjalizacja sterownika: PCI, MMIO, reset, MAC, rejestry TCR/RCR, bufor odbiorczy, deskryptory nadawcze, wlaczenie TX i RX*/

int rtl8168b_init(void)
{
    if (g_ready) return 1;
    if (!rtl8168b_find_device()) return 0;

    mmio_write8(REG_CFG9346, 0x00U);   /*tryb normalny*/
    if (!rtl8168b_reset()) return 0;
    if (!rtl8168b_mac_read()) return 0;

    mmio_write16(REG_ISR, 0xFFFFU);   /*wyczyscoczekujace przerwania*/
    mmio_write16(REG_IMR, 0x0000U);   /*tryb poling bez przerwan*/

    mmio_write32(REG_TCR, 0x00000000U);   /*konfiguracja nadawania, bez loopback*/

    mmio_write32(REG_RCR,
                 (uint32_t)(RCR_ACCEPT_MYMAC | RCR_ACCEPT_BROADCAST |
                            RCR_ACCEPT_MULTICAST | RCR_ACCEPT_PHYS |
                            RCR_BUFFER_8K));   /*odbieranie wlasngo MAC, broadcast, multicast, ramki fizyczne, bufor 8K+4, bez wrap*/

    mmio_write32(REG_RBSTART, (uint32_t)&g_rx_buf[0]);   /*adres bufora odbiorczego*/

    for (uint32_t i = 0; i < TX_DESC_COUNT; i++) {
        mmio_write32(REG_TSAD0 + i * 4, (uint32_t)&g_tx_bufs[i][0]);
        mmio_write32(REG_TSD0 + i * 4, 0U);   /*adresy deskryptorow nadawczych*/
    }

    g_rx_ptr = 0;
    g_tx_next = 0;

    mmio_write8(REG_CMDR, (uint8_t)(CMD_START | CMD_TX_ENABLE | CMD_RX_ENABLE));   /*wlaczenie nadawania i odbioru*/

    g_ready = 1;
    return 1;
}

/*czy sterownik jest gotowy*/

int rtl8168b_is_ready(void)
{
    return g_ready;
}

/*pobranie adresu MAC*/

int rtl8168b_get_mac(uint8_t mac[6])
{
    if (!g_ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = g_mac[i];
    return 1;
}

/*wyslanie ramki ethernet; oczekiwanie az karta zwolni deskryptor nadawczy*/

int rtl8168b_send(const void *frame, uint16_t length)
{
    if (!g_ready || !frame || length == 0 || length > TX_BUF_SIZE) return -1;

    uint32_t index = g_tx_next % TX_DESC_COUNT;

    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        if (!(mmio_read32(REG_TSD0 + index * 4) & TSD_OWN)) break;
    }
    if (mmio_read32(REG_TSD0 + index * 4) & TSD_OWN) {
        g_stats.tx_errors++;
        return 0;
    }

    for (uint32_t i = 0; i < length; i++) {
        g_tx_bufs[index][i] = ((const uint8_t *)frame)[i];
    }
    __asm__ volatile("" : : : "memory");

    mmio_write32(REG_TSD0 + index * 4,
                 (uint32_t)(TSD_OWN | ((uint32_t)length & TSD_LENGTH_MASK)));
    g_tx_next++;

    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        if (!(mmio_read32(REG_TSD0 + index * 4) & TSD_OWN)) break;
    }

    g_stats.tx_packets++;
    g_stats.tx_bytes += length;
    return 1;
}

/*odbior ramki z bufora odbiorczego (tryb polling, sprawdzenie ISR); zwraca dlugosc ramki lub 0 gdy brak danych*/

int rtl8168b_receive(void *frame, uint16_t capacity)
{
    if (!g_ready || !frame || capacity == 0) return -1;

    uint16_t isr = mmio_read16(REG_ISR);
    if (!(isr & ISR_RX_OK)) return 0;

    mmio_write16(REG_ISR, (uint16_t)(ISR_RX_OK | ISR_RX_ERR));   /*potwierdzenie przerwania odbioru*/

    uint32_t offset = g_rx_ptr % RX_BUF_SIZE;

    uint16_t status = (uint16_t)(g_rx_buf[offset] | (g_rx_buf[offset + 1] << 8));
    uint16_t length = (uint16_t)(g_rx_buf[offset + 2] | (g_rx_buf[offset + 3] << 8));
    (void)status;

    if (length == 0 || length > capacity || length > RX_BUF_SIZE - 4) {
        g_stats.rx_errors++;
        g_rx_ptr = (offset + 4U) % RX_BUF_SIZE;
        return 0;
    }

    for (uint32_t i = 0; i < length; i++) {
        ((uint8_t *)frame)[i] = g_rx_buf[(offset + 4 + i) % RX_BUF_SIZE];
    }

    g_rx_ptr = (offset + 4 + length + 3U) & ~3U;
    g_rx_ptr %= RX_BUF_SIZE;

    g_stats.rx_packets++;
    g_stats.rx_bytes += length;
    return (int)length;
}

/*pobranie statystyk sterownika*/

void rtl8168b_stats_get(struct rtl8168b_stats *out)
{
    if (!out) return;
    *out = g_stats;
}
