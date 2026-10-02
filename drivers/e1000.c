#include "e1000.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define E1000_RX_COUNT 32
#define E1000_TX_COUNT 8
#define E1000_BUFFER_SIZE 2048

struct e1000_descriptor {
    uint32_t address_low;
    uint32_t address_high;
    uint16_t length;
    uint8_t checksum_offset;
    uint8_t command;
    uint8_t status;
    uint8_t checksum_start;
    uint16_t special;
} __attribute__((packed));

typedef char e1000_descriptor_must_be_16_bytes[(sizeof(struct e1000_descriptor) == 16) ? 1 : -1];

static volatile uint8_t *registers;
static uint8_t device_mac[6];
static uint8_t ready;
static uint32_t rx_head;
static uint32_t tx_head;
static struct e1000_descriptor rx_descriptors[E1000_RX_COUNT] __attribute__((aligned(16)));
static struct e1000_descriptor tx_descriptors[E1000_TX_COUNT] __attribute__((aligned(16)));
static uint8_t rx_buffers[E1000_RX_COUNT][E1000_BUFFER_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buffers[E1000_TX_COUNT][E1000_BUFFER_SIZE] __attribute__((aligned(16)));

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

static uint32_t e1000_read(uint16_t offset)
{
    return *(volatile uint32_t *)(registers + offset);
}

static void e1000_write(uint16_t offset, uint32_t value)
{
    *(volatile uint32_t *)(registers + offset) = value;
}

static int e1000_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            if (vendor == 0xFFFF) continue;
            if (vendor != 0x8086 || (device != 0x100E && device != 0x100F &&
                                     device != 0x10D3 && device != 0x10EA &&
                                     device != 0x1502 && device != 0x153A)) continue;

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || (bar0 & 1)) return 0;
            uint32_t command = pci_read32((uint8_t)bus, slot, 0, 0x04);
            pci_write32((uint8_t)bus, slot, 0, 0x04, command | 0x00000006);
            registers = (volatile uint8_t *)(bar0 & 0xFFFFFFF0);
            return 1;
        }
    }
    return 0;
}

static int e1000_read_mac(void)
{
    uint32_t low = e1000_read(0x5400);
    uint32_t high = e1000_read(0x5404);
    if (low == 0xFFFFFFFF || low == 0 || (high & 0xFFFF) == 0) return 0;
    device_mac[0] = (uint8_t)low;
    device_mac[1] = (uint8_t)(low >> 8);
    device_mac[2] = (uint8_t)(low >> 16);
    device_mac[3] = (uint8_t)(low >> 24);
    device_mac[4] = (uint8_t)high;
    device_mac[5] = (uint8_t)(high >> 8);
    return 1;
}

int e1000_init(void)
{
    if (ready) return 1;
    if (!e1000_find_device()) return 0;

    e1000_write(0x0000, e1000_read(0x0000) | (1U << 26));
    for (volatile uint32_t wait = 0; wait < 100000; wait++);
    e1000_write(0x0000, e1000_read(0x0000) & ~(1U << 26));
    e1000_write(0x00D0, 0xFFFFFFFF);

    if (!e1000_read_mac()) return 0;

    for (uint32_t i = 0; i < E1000_RX_COUNT; i++) {
        rx_descriptors[i].address_low = (uint32_t)&rx_buffers[i][0];
        rx_descriptors[i].address_high = 0;
        rx_descriptors[i].status = 0;
    }
    e1000_write(0x2800, (uint32_t)&rx_descriptors[0]);
    e1000_write(0x2804, 0);
    e1000_write(0x2808, sizeof(rx_descriptors));
    e1000_write(0x2810, 0);
    e1000_write(0x2818, E1000_RX_COUNT - 1);
    e1000_write(0x0100, (1U << 0) | (1U << 1) | (1U << 15) | (1U << 26));
    e1000_write(0x0D00, 0x00000000);

    for (uint32_t i = 0; i < E1000_TX_COUNT; i++) {
        tx_descriptors[i].address_low = (uint32_t)&tx_buffers[i][0];
        tx_descriptors[i].address_high = 0;
        tx_descriptors[i].status = 1;
        tx_descriptors[i].length = 0;
    }
    e1000_write(0x3800, (uint32_t)&tx_descriptors[0]);
    e1000_write(0x3804, 0);
    e1000_write(0x3808, sizeof(tx_descriptors));
    e1000_write(0x3810, 0);
    e1000_write(0x3818, 0);
    e1000_write(0x0400, 0x0004010A);
    e1000_write(0x0410, 0x0060200A);
    e1000_write(0x0428, (1U << 0) | (1U << 1));

    rx_head = 0;
    tx_head = 0;
    ready = 1;
    return 1;
}

int e1000_is_ready(void)
{
    return ready;
}

int e1000_get_mac(uint8_t mac[6])
{
    if (!ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int e1000_send(const void *frame, uint16_t length)
{
    struct e1000_descriptor *descriptor;
    if (!ready || !frame || length == 0 || length > E1000_BUFFER_SIZE) return -1;
    descriptor = &tx_descriptors[tx_head];
    if (!(descriptor->status & 1)) return 0;
    for (uint32_t i = 0; i < length; i++) tx_buffers[tx_head][i] = ((const uint8_t *)frame)[i];
    descriptor->length = length;
    descriptor->checksum_offset = 0;
    descriptor->command = 0x0B;
    descriptor->status = 0;
    descriptor->checksum_start = 0;
    descriptor->special = 0;
    __asm__ volatile("" : : : "memory");
    tx_head = (tx_head + 1) % E1000_TX_COUNT;
    e1000_write(0x3818, tx_head);
    return 1;
}

int e1000_receive(void *frame, uint16_t capacity)
{
    struct e1000_descriptor *descriptor;
    uint16_t length;
    if (!ready || !frame || capacity == 0) return -1;
    descriptor = &rx_descriptors[rx_head];
    if (!(descriptor->status & 1)) return 0;
    length = descriptor->length;
    if (length > capacity) length = capacity;
    for (uint32_t i = 0; i < length; i++) ((uint8_t *)frame)[i] = rx_buffers[rx_head][i];
    descriptor->status = 0;
    descriptor->length = 0;
    __asm__ volatile("" : : : "memory");
    e1000_write(0x2818, rx_head);
    rx_head = (rx_head + 1) % E1000_RX_COUNT;
    return length;
}
