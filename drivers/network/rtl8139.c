/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/network/rtl8139.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define RTL_REG_MAC0      0x00
#define RTL_REG_MAR0      0x08
#define RTL_REG_TSD0      0x10
#define RTL_REG_TSAD0     0x20
#define RTL_REG_RBSTART   0x30
#define RTL_REG_CMDR      0x37
#define RTL_REG_CAPR      0x38
#define RTL_REG_CBR       0x3A
#define RTL_REG_IMR       0x3C
#define RTL_REG_ISR       0x3E
#define RTL_REG_TCR       0x40
#define RTL_REG_RCR       0x44
#define RTL_REG_CONFIG1   0x52

#define CMD_BUFE          0x01
#define CMD_TE            0x04
#define CMD_RE            0x08
#define CMD_RST           0x10

#define TSD_OWN           (1U << 13)
#define RCR_VALUE         0x0000008FU   /* AAP|APM|AM|AB + WRAP, bufor 8K+16 */

#define RX_RING_SIZE  8192
#define RX_BUF_SIZE   (8192 + 16 + 1536)   /* +1536 bo WRAP=1 */
#define TX_BUF_SIZE   2048
#define TX_MAX_FRAME  1792
#define TX_COUNT      4

static uint16_t io_base;
static uint8_t device_mac[6];
static uint8_t ready;
static volatile uint8_t rx_buf[RX_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_bufs[TX_COUNT][TX_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_used[TX_COUNT];
static uint32_t rx_ptr;
static uint32_t tx_ptr;

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

static int rtl8139_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            if (vendor == 0xFFFF) continue;
            if (vendor != 0x10EC || (device != 0x8139 && device != 0x8129 &&
                                     device != 0x8138 && device != 0x8130)) continue;

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || !(bar0 & 1)) return 0;
            io_base = (uint16_t)(bar0 & 0xFFFC);

            /* PCI command: I/O space (bit0) + memory (bit1) + bus master (bit2) */
            uint32_t command = pci_read32((uint8_t)bus, slot, 0, 0x04);
            command |= 0x00000007;
            pci_write32((uint8_t)bus, slot, 0, 0x04, command);
            return 1;
        }
    }
    return 0;
}

static void rtl8139_rx_setup(void)
{
    rx_ptr = 0;
    outl(io_base + RTL_REG_RBSTART, (uint32_t)rx_buf);
    outl(io_base + RTL_REG_RCR, RCR_VALUE);
    outw(io_base + RTL_REG_CAPR, 0xFFF0);
}

static void rtl8139_rx_reset(void)
{
    outb(io_base + RTL_REG_CMDR, CMD_TE);           /* wylacz RX */
    outb(io_base + RTL_REG_CMDR, CMD_TE | CMD_RE);  /* wlacz RX ponownie */
    rtl8139_rx_setup();
    outw(io_base + RTL_REG_ISR, 0xFFFF);
}

int rtl8139_init(void)
{
    if (ready) return 1;
    if (!rtl8139_find_device()) return 0;

    /* zasilanie */
    outb(io_base + RTL_REG_CONFIG1, 0x00);

    /* software reset - czekamy az bit RST sam sie wyczysci */
    outb(io_base + RTL_REG_CMDR, CMD_RST);
    uint32_t timeout = 1000000;
    while ((inb(io_base + RTL_REG_CMDR) & CMD_RST) && --timeout);
    if (!timeout) return 0;

    for (uint32_t i = 0; i < 6; i++) {
        device_mac[i] = inb(io_base + RTL_REG_MAC0 + i);
    }

    tx_ptr = 0;
    for (uint32_t i = 0; i < TX_COUNT; i++) tx_used[i] = 0;

    /* wszystkie grupy multicast */
    outl(io_base + RTL_REG_MAR0, 0xFFFFFFFF);
    outl(io_base + RTL_REG_MAR0 + 4, 0xFFFFFFFF);

    rtl8139_rx_setup();

    outl(io_base + RTL_REG_TCR, 0x03000700);   /* IFG standard, DMA burst unlimited */

    outw(io_base + RTL_REG_IMR, 0x0005);       /* ROK | TOK (i tak pollujemy) */
    outw(io_base + RTL_REG_ISR, 0xFFFF);

    outb(io_base + RTL_REG_CMDR, CMD_RE | CMD_TE);

    ready = 1;
    return 1;
}

int rtl8139_is_ready(void)
{
    return ready;
}

int rtl8139_get_mac(uint8_t mac[6])
{
    if (!ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int rtl8139_send(const void *frame, uint16_t length)
{
    if (!ready || !frame || length == 0 || length > TX_MAX_FRAME) return -1;

    /* poczekaj az poprzednie uzycie tego deskryptora sie skonczy */
    if (tx_used[tx_ptr]) {
        uint32_t timeout = 1000000;
        while (!(inl(io_base + RTL_REG_TSD0 + tx_ptr * 4) & TSD_OWN) && --timeout);
    }

    for (uint32_t i = 0; i < length; i++) {
        tx_bufs[tx_ptr][i] = ((const uint8_t *)frame)[i];
    }

    uint32_t send_len = length;
    while (send_len < 60) tx_bufs[tx_ptr][send_len++] = 0;   /* minimalna ramka Ethernet */

    outl(io_base + RTL_REG_TSAD0 + tx_ptr * 4, (uint32_t)&tx_bufs[tx_ptr][0]);
    outl(io_base + RTL_REG_TSD0 + tx_ptr * 4, send_len);   /* zapis dlugosci startuje wysylanie */

    tx_used[tx_ptr] = 1;
    tx_ptr = (tx_ptr + 1) % TX_COUNT;
    return 1;
}

int rtl8139_receive(void *frame, uint16_t capacity)
{
    if (!ready || !frame || capacity == 0) return -1;

    /* bufor pusty? */
    if (inb(io_base + RTL_REG_CMDR) & CMD_BUFE) return 0;

    uint32_t offset = rx_ptr;
    uint16_t status = (uint16_t)(rx_buf[offset] | (rx_buf[offset + 1] << 8));
    uint16_t length = (uint16_t)(rx_buf[offset + 2] | (rx_buf[offset + 3] << 8));

    /* length zawiera 4 bajty CRC */
    if (!(status & 0x0001) || length < 8 || length > 1518 + 4) {
        rtl8139_rx_reset();    /* uszkodzony naglowek - zresetuj odbiornik */
        return 0;
    }

    uint16_t frame_len = (uint16_t)(length - 4);
    int result = 0;

    if (frame_len <= capacity) {
        for (uint32_t i = 0; i < frame_len; i++) {
            ((uint8_t *)frame)[i] = rx_buf[offset + 4 + i];   /* WRAP=1: dane liniowo */
        }
        result = frame_len;
    }

    rx_ptr = (offset + length + 4 + 3) & ~3U;
    rx_ptr %= RX_RING_SIZE;

    outw(io_base + RTL_REG_CAPR, (uint16_t)(rx_ptr - 16));
    outw(io_base + RTL_REG_ISR, 0x0001 | 0x0002 | 0x0010 | 0x0020);

    return result;
}