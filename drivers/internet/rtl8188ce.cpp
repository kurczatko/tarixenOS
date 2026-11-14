/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/internet/rtl8188ce.h>

/*funkcje io portow*/

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

/*rejestry MMIO Realtek RTL8188CE*/

enum rtl8188ce_reg {
    REG_SYS_CFG         = 0x0000U,   /*konfiguracja systemu*/
    REG_SYS_FUNC_EN     = 0x0004U,   /*wlaczenie funkcji*/
    REG_MAC_ADDR        = 0x0100U,   /*adres MAC (6 bajtow)*/
    REG_RX_DESC_BASE    = 0x0300U,   /*adres bazowy deskryptorow odbioru*/
    REG_TX_DESC_BASE    = 0x0400U,   /*adres bazowy deskryptorow nadawania*/
    REG_RX_STATUS       = 0x0500U,   /*status odbioru*/
    REG_TX_STATUS       = 0x0600U,   /*status nadawania*/
    REG_IMR             = 0x0700U,   /*maska przerwan*/
    REG_ISR             = 0x0704U,   /*status przerwan*/
    REG_WIFI_CTRL       = 0x0800U,   /*kontrola WiFi*/
    REG_WIFI_STATUS     = 0x0804U,   /*status WiFi*/
    REG_CHANNEL         = 0x0808U,   /*kanal WiFi*/
    REG_BEACON_INTERVAL = 0x080CU,   /*interwał beaconów*/
    REG_RX_BUF_SIZE     = 0x0900U,   /*rozmiar bufora odbiorczego*/
    REG_TX_BUF_SIZE     = 0x0904U,   /*rozmiar bufora nadawczego*/
};

/*bity SYS_CFG*/

#define SYS_CFG_RESET     (1U << 0)
#define SYS_CFG_TX_ENABLE (1U << 1)
#define SYS_CFG_RX_ENABLE (1U << 2)

/*bity SYS_FUNC_EN*/

#define FUNC_EN_WIFI      (1U << 0)
#define FUNC_EN_MAC       (1U << 1)
#define FUNC_EN_HOST      (1U << 2)

/*bity ISR / IMR*/

#define ISR_RX_OK         0x0002U
#define ISR_TX_OK         0x0001U
#define ISR_RX_ERR        0x0040U
#define ISR_TX_ERR        0x0080U
#define ISR_WIFI_ASSOC    0x0100U

/*bity WIFI_CTRL*/

#define WIFI_CTRL_ENABLE  (1U << 0)
#define WIFI_CTRL_ASSOC   (1U << 1)
#define WIFI_CTRL_POWER   (1U << 2)

/*id urzadzen Realtek RTL8188CE (rodzina 802.11n)*/

#define RTL_VENDOR_ID   0x10ECU
#define RTL_DEVICE_8188CE 0x8179U
#define RTL_DEVICE_8188   0x8188U

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
static uint8_t g_wifi_associated = 0;

static uint8_t g_rx_buf[RX_BUF_SIZE + 16] __attribute__((aligned(16)));
static uint8_t g_tx_bufs[TX_DESC_COUNT][TX_BUF_SIZE] __attribute__((aligned(16)));
static uint32_t g_rx_ptr = 0;
static uint32_t g_tx_next = 0;

static struct rtl8188ce_stats g_stats = {};

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

/*opoznienie mikrosekundowe (obrotowa petla CPU)*/

static void rtl8188ce_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (int j = 0; j < 40; j++);
    }
}

/*wyszukanie urzadzenia RTL8188CE na PCI*/

static int rtl8188ce_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != RTL_VENDOR_ID) continue;
                if (device != RTL_DEVICE_8188CE && device != RTL_DEVICE_8188) continue;

                uint32_t bar0 = pci_read32(bus, slot, func, 0x10);
                if (bar0 == 0 || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;

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

/*odczyt adresu MAC z chipu WiFi*/

static int rtl8188ce_mac_read(void)
{
    uint32_t low = mmio_read32(REG_MAC_ADDR);
    uint32_t high = mmio_read32(REG_MAC_ADDR + 4U);
    if (low == 0xFFFFFFFFU || low == 0 || (high & 0xFFFF) == 0) return 0;
    g_mac[0] = (uint8_t)low;
    g_mac[1] = (uint8_t)(low >> 8);
    g_mac[2] = (uint8_t)(low >> 16);
    g_mac[3] = (uint8_t)(low >> 24);
    g_mac[4] = (uint8_t)high;
    g_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

/*reset chipu WiFi*/

static int rtl8188ce_reset(void)
{
    mmio_write8(REG_SYS_CFG, (uint8_t)SYS_CFG_RESET);
    for (uint32_t wait = 0; wait < 100000; wait++) {
        if (!(mmio_read8(REG_SYS_CFG) & SYS_CFG_RESET)) break;
    }
    if (mmio_read8(REG_SYS_CFG) & SYS_CFG_RESET) return 0;

    rtl8188ce_delay_us(100);
    return 1;
}

/*inicjalizacja modulu WiFi (asocjacja, kanal, moc nadawania)*/

static int rtl8188ce_wifi_init(void)
{
    /*wlacz funkcje WiFi i MAC*/
    mmio_write8(REG_SYS_FUNC_EN,
                (uint8_t)(FUNC_EN_WIFI | FUNC_EN_MAC | FUNC_EN_HOST));

    /*wlacz kontroler WiFi*/
    mmio_write8(REG_WIFI_CTRL, (uint8_t)WIFI_CTRL_ENABLE);

    /*ustaw kanal domyslny (1 dla 2.4 GHz)*/
    mmio_write8(REG_CHANNEL, 1U);

    /*ustaw interwał beaconow*/
    mmio_write16(REG_BEACON_INTERVAL, 100U);

    /*czekaj na asocjacje (tryb polling, bez przerwan)*/
    for (uint32_t wait = 0; wait < 1000000U; wait++) {
        if (mmio_read8(REG_WIFI_STATUS) & WIFI_CTRL_ASSOC) {
            g_wifi_associated = 1;
            break;
        }
    }

    /*nie wymagamy asocjacji zeby sterownik byl gotowy (mozna wyswietlac ramki bez polaczenia)*/
    return 1;
}

/*inicjalizacja sterownika RTL8188CE*/

int rtl8188ce_init(void)
{
    if (g_ready) return 1;
    if (!rtl8188ce_find_device()) return 0;

    mmio_write8(REG_SYS_CFG, 0x00U);   /*tryb normalny*/
    if (!rtl8188ce_reset()) return 0;
    if (!rtl8188ce_mac_read()) return 0;

    /*wyczysc oczekujace przerwania*/
    mmio_write16(REG_ISR, 0xFFFFU);
    mmio_write16(REG_IMR, 0x0000U);   /*tryb polling, bez przerwan*/

    /*wlacz nadawanie i odbieranie*/
    mmio_write8(REG_SYS_CFG, (uint8_t)(SYS_CFG_TX_ENABLE | SYS_CFG_RX_ENABLE));

    /*inicjalizuj moduł WiFi*/
    if (!rtl8188ce_wifi_init()) return 0;

    /*ustaw adresy buforow odbiorczych*/
    mmio_write32(REG_RX_DESC_BASE, (uint32_t)&g_rx_buf[0]);

    /*ustaw adresy deskryptorow nadawczych*/
    for (uint32_t i = 0; i < TX_DESC_COUNT; i++) {
        mmio_write32(REG_TX_DESC_BASE + i * 4U, (uint32_t)&g_tx_bufs[i][0]);
    }

    g_rx_ptr = 0;
    g_tx_next = 0;

    g_ready = 1;
    return 1;
}

/*czy sterownik jest gotowy*/

int rtl8188ce_is_ready(void)
{
    return g_ready;
}

/*pobranie adresu MAC*/

int rtl8188ce_get_mac(uint8_t mac[6])
{
    if (!g_ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = g_mac[i];
    return 1;
}

/*wyslanie ramki ethernet przez kartę WiFi; oczekiwanie az karta zwolni deskryptor nadawczy*/

int rtl8188ce_send(const void *frame, uint16_t length)
{
    if (!g_ready || !frame || length == 0 || length > TX_BUF_SIZE) return -1;

    uint32_t index = g_tx_next % TX_DESC_COUNT;

    /*czekaj az deskryptor nadawczy zostanie zwolniony przez karte*/
    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        if (!(mmio_read32(REG_TX_STATUS + index * 4U) & 0x80000000U)) break;
    }
    if (mmio_read32(REG_TX_STATUS + index * 4U) & 0x80000000U) {
        g_stats.tx_errors++;
        return 0;
    }

    for (uint32_t i = 0; i < length; i++) {
        g_tx_bufs[index][i] = ((const uint8_t *)frame)[i];
    }
    __asm__ volatile("" : : : "memory");

    mmio_write32(REG_TX_STATUS + index * 4U,
                 (uint32_t)(0x80000000U | (uint32_t)length));
    g_tx_next++;

    /*czekaj az karta zakonczy nadawanie*/
    for (uint32_t wait = 0; wait < 2000000U; wait++) {
        if (!(mmio_read32(REG_TX_STATUS + index * 4U) & 0x80000000U)) break;
    }

    g_stats.tx_packets++;
    g_stats.tx_bytes += length;
    return 1;
}

/*odbior ramki z bufora odbiorczego (tryb polling, sprawdzenie ISR); zwraca dlugosc ramki lub 0 gdy brak danych*/

int rtl8188ce_receive(void *frame, uint16_t capacity)
{
    if (!g_ready || !frame || capacity == 0) return -1;

    uint16_t isr = mmio_read16(REG_ISR);
    if (!(isr & ISR_RX_OK)) return 0;

    /*potwierdzenie przerwania odbioru*/
    mmio_write16(REG_ISR, (uint16_t)(ISR_RX_OK | ISR_RX_ERR));

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

void rtl8188ce_stats_get(struct rtl8188ce_stats *out)
{
    if (!out) return;
    *out = g_stats;
}
