#include "rtl8139.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define RTL_REG_MAC0      0x00
#define RTL_REG_MAR0      0x08
#define RTL_REG_TSD0      0x10
#define RTL_REG_TSAD0     0x20
#define RTL_REG_RBSTART   0x18
#define RTL_REG_RCR       0x20
#define RTL_REG_TCR       0x24
#define RTL_REG_CMDR      0x30
#define RTL_REG_IMR       0x34
#define RTL_REG_ISR       0x36
#define RTL_REG_CAPR      0x38
#define RTL_REG_CBR       0x3A
#define RTL_REG_MPC       0x3C

#define RX_BUF_SIZE   8192
#define TX_BUF_SIZE   2048
#define TX_COUNT      4

static uint16_t io_base;
static uint8_t device_mac[6];
static uint8_t ready;
static uint8_t rx_buf[RX_BUF_SIZE + 16] __attribute__((aligned(16)));
static uint8_t tx_bufs[TX_COUNT][TX_BUF_SIZE] __attribute__((aligned(16)));
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
            uint32_t command = pci_read32((uint8_t)bus, slot, 0, 0x04);
            pci_write32((uint8_t)bus, slot, 0, 0x04, command | 0x00000006);
            return 1;
        }
    }
    return 0;
}

int rtl8139_init(void)
{
    if (ready) return 1;
    if (!rtl8139_find_device()) return 0;

    outb(io_base + RTL_REG_CMDR, 0x40);
    for (volatile uint32_t wait = 0; wait < 200000; wait++);
    if (inb(io_base + RTL_REG_CMDR) & 0x40) return 0;

    for (uint32_t i = 0; i < 6; i++) {
        device_mac[i] = inb(io_base + RTL_REG_MAC0 + i);
    }

    rx_ptr = 0;
    tx_ptr = 0;

    outl(io_base + RTL_REG_RBSTART, (uint32_t)rx_buf);

    for (uint32_t i = 0; i < TX_COUNT; i++) {
        outl(io_base + RTL_REG_TSAD0 + i * 4, (uint32_t)&tx_bufs[i][0]);
    }

    outl(io_base + RTL_REG_RCR, 0x0000002F);
    outl(io_base + RTL_REG_TCR, 0x00000000);

    outb(io_base + RTL_REG_IMR, 0x05);
    outb(io_base + RTL_REG_ISR, 0xFF);

    outb(io_base + RTL_REG_CMDR, 0x0C);

    outb(io_base + RTL_REG_CAPR, (uint8_t)rx_ptr);

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
    if (!ready || !frame || length == 0 || length > TX_BUF_SIZE) return -1;

    for (uint32_t i = 0; i < length; i++) {
        tx_bufs[tx_ptr][i] = ((const uint8_t *)frame)[i];
    }

    outl(io_base + RTL_REG_TSAD0 + tx_ptr * 4, (uint32_t)&tx_bufs[tx_ptr][0]);
    outl(io_base + RTL_REG_TSD0 + tx_ptr * 4, length);

    tx_ptr = (tx_ptr + 1) % TX_COUNT;
    return 1;
}

int rtl8139_receive(void *frame, uint16_t capacity)
{
    if (!ready || !frame || capacity == 0) return -1;

    uint8_t isr = inb(io_base + RTL_REG_ISR);
    if (!(isr & 0x01)) return 0;

    uint32_t offset = rx_ptr;

    uint16_t status = rx_buf[offset] | (rx_buf[offset + 1] << 8);
    uint16_t length = rx_buf[offset + 2] | (rx_buf[offset + 3] << 8);
    (void)status;

    if (length == 0 || length > capacity || length > RX_BUF_SIZE - 4) {
        outb(io_base + RTL_REG_ISR, 0x01);
        return 0;
    }

    for (uint32_t i = 0; i < length; i++) {
        ((uint8_t *)frame)[i] = rx_buf[(offset + 4 + i) % RX_BUF_SIZE];
    }

    rx_ptr = (offset + 4 + length + 3) & ~3;
    rx_ptr %= RX_BUF_SIZE;

    outb(io_base + RTL_REG_CAPR, (uint8_t)((rx_ptr - 16) & 0xFF));
    outb(io_base + RTL_REG_ISR, 0x01);

    return length;
}
