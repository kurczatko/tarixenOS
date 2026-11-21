/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Sterownik karty sieciowej AMD PCnet (Am79C970 / 971 / 972 / 973 / 975).
 * Tryb WIO (16-bitowy dostep do rejestrow), struktury 32-bitowe (SWSTYLE 2),
 * polling bez przerwan.
 * Zobacz drivers/network/pcnet.h dla opisu funkcji.
 */

#include <drivers/network/pcnet.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_VENDOR_AMD 0x1022U
#define PCNET_DEVICE_AM79C970 0x2000U
#define PCNET_DEVICE_AM79C971 0x2001U
#define PCNET_DEVICE_AM79C972 0x2624U
#define PCNET_DEVICE_AM79C973 0x2625U
#define PCNET_DEVICE_AM79C975 0x2626U

/* Porty I/O (tryb WIO, offsety od bazy I/O w BAR0). */
#define PCNET_APROM 0x00   /* adres MAC (6 bajtow) */
#define PCNET_RDP   0x10
#define PCNET_RAP   0x12
#define PCNET_RESET 0x14
#define PCNET_BDP   0x16

/* CSR / BCR. */
#define CSR0 0
#define CSR1 1
#define CSR2 2
#define BCR2  2
#define BCR20 20

/* Bity CSR0. */
#define CSR0_ERR   (1U << 15)
#define CSR0_BABL  (1U << 14)
#define CSR0_CERR  (1U << 13)
#define CSR0_MISS  (1U << 12)
#define CSR0_MERR  (1U << 11)
#define CSR0_RINT  (1U << 10)
#define CSR0_TINT  (1U << 9)
#define CSR0_IDON  (1U << 8)
#define CSR0_INTR  (1U << 7)
#define CSR0_IENA  (1U << 6)
#define CSR0_RXON  (1U << 5)
#define CSR0_TXON  (1U << 4)
#define CSR0_TDMD  (1U << 3)
#define CSR0_STOP  (1U << 2)
#define CSR0_STRT  (1U << 1)
#define CSR0_INIT  (1U << 0)

/* Bity trybu (mode) w bloku inicjalizacji. */
#define MODE_PROM  0x8000U
#define MODE_INTL  0x0040U
#define MODE_LOOP  0x0004U

#define BCR2_ASEL  0x0002U
#define BCR20_SWSTYLE_32 0x0002U   /* struktury 32-bitowe (SSIZE32) */

#define RX_RLEN 3
#define TX_RLEN 3
#define RX_RING_COUNT (1U << RX_RLEN)
#define TX_RING_COUNT (1U << TX_RLEN)

#define RX_BUF_SIZE 2048
#define TX_BUF_SIZE 2048
#define MAX_FRAME 1518
#define MIN_FRAME 60

/* Blok inicjalizacji w trybie 32-bitowym (28 bajtow). */
struct pcnet_init_block {
    uint16_t mode;
    uint16_t tlen_rlen;    /* TLEN w bitach 15:12, RLEN w bitach 7:4 */
    uint8_t  padr[6];
    uint16_t reserved;
    uint32_t ladrf[2];
    uint32_t rdra;
    uint32_t tdra;
} __attribute__((packed));

/* Deskryptory w trybie 32-bitowym (16 bajtow). */
struct pcnet_rmd {
    uint32_t addr;
    uint16_t bcnt;     /* -rozmiar bufora (uzupelnienie do dwoch) */
    uint16_t status;
    uint32_t mcnt;     /* bity 11:0 = dlugosc ramki wraz z FCS */
    uint32_t reserved;
} __attribute__((packed));

struct pcnet_tmd {
    uint32_t addr;
    uint16_t bcnt;     /* -dlugosc ramki */
    uint16_t status;
    uint32_t misc;
    uint32_t reserved;
} __attribute__((packed));

#define RMD_OWN  0x8000U
#define RMD_ERR  0x4000U
#define RMD_FRAM 0x2000U
#define RMD_OFLO 0x1000U
#define RMD_CRC  0x0800U
#define RMD_BUFF 0x0400U
#define RMD_STP  0x0200U
#define RMD_ENP  0x0100U

#define TMD_OWN  0x8000U
#define TMD_ERR  0x4000U
#define TMD_MORE 0x1000U
#define TMD_ONE  0x0800U
#define TMD_DEF  0x0400U
#define TMD_STP  0x0200U
#define TMD_ENP  0x0100U

static uint16_t io_base;
static uint8_t device_mac[6];
static uint8_t ready;
static uint8_t irq_line;

static struct pcnet_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_collisions;
    uint32_t tx_aborted;
    uint32_t rx_crc_errors;
    uint32_t rx_frame_errors;
    uint32_t rx_over_errors;
    uint32_t rx_buffer_errors;
    uint32_t rx_missed;
    uint32_t tx_babble;
    uint32_t mem_errors;
} stats;

static volatile struct pcnet_init_block init_block __attribute__((aligned(16)));
static volatile struct pcnet_rmd rx_ring[RX_RING_COUNT] __attribute__((aligned(16)));
static volatile struct pcnet_tmd tx_ring[TX_RING_COUNT] __attribute__((aligned(16)));
static uint8_t rx_buffers[RX_RING_COUNT][RX_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t tx_buffers[TX_RING_COUNT][TX_BUF_SIZE] __attribute__((aligned(16)));

static uint32_t rx_cur;
static uint32_t tx_cur;

/* ------------------------------------------------------------------ */
/* I/O i PCI.                                                         */
/* ------------------------------------------------------------------ */

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

static uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
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

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function,
                           uint8_t offset)
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
/* Rejestry CSR / BCR.                                                */
/* ------------------------------------------------------------------ */

static void pcnet_csr_write(uint8_t reg, uint16_t value)
{
    outw(io_base + PCNET_RAP, (uint16_t)reg);
    outw(io_base + PCNET_RDP, value);
}

static uint16_t pcnet_csr_read(uint8_t reg)
{
    outw(io_base + PCNET_RAP, (uint16_t)reg);
    return inw(io_base + PCNET_RDP);
}

static void pcnet_bcr_write(uint8_t reg, uint16_t value)
{
    outw(io_base + PCNET_RAP, (uint16_t)reg);
    outw(io_base + PCNET_BDP, value);
}

static uint16_t pcnet_bcr_read(uint8_t reg)
{
    outw(io_base + PCNET_RAP, (uint16_t)reg);
    return inw(io_base + PCNET_BDP);
}

/* Czeka az w CSR zapali sie podany bit. */
static int pcnet_wait_set(uint8_t reg, uint16_t bit, uint32_t timeout)
{
    while (timeout--) {
        if (pcnet_csr_read(reg) & bit) return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Sprzet.                                                            */
/* ------------------------------------------------------------------ */

static int pcnet_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            if (vendor == 0xFFFF) continue;
            if (vendor != PCI_VENDOR_AMD) continue;
            if (device != PCNET_DEVICE_AM79C970 && device != PCNET_DEVICE_AM79C971 &&
                device != PCNET_DEVICE_AM79C972 && device != PCNET_DEVICE_AM79C973 &&
                device != PCNET_DEVICE_AM79C975) continue;

            uint32_t bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || !(bar0 & 1)) return 0;
            io_base = (uint16_t)(bar0 & 0xFFFC);

            uint32_t command = pci_read32((uint8_t)bus, slot, 0, 0x04);
            command |= 0x00000007U;   /* I/O + memory + bus master */
            pci_write32((uint8_t)bus, slot, 0, 0x04, command);

            irq_line = (uint8_t)(pci_read32((uint8_t)bus, slot, 0, 0x3C) & 0xFF);
            return 1;
        }
    }
    return 0;
}

static int pcnet_sw_reset(void)
{
    uint32_t t;
    (void)inw(io_base + PCNET_RESET);      /* odczyt resetuje chip */
    outw(io_base + PCNET_RESET, 0x0000);   /* zapis tez (zgodnosc z PCnet-PCI) */
    for (t = 0; t < 100000U; t++) inb(0x84);

    /* Po resecie chip jest zatrzymany (STOP = 1). */
    return (pcnet_csr_read(CSR0) & CSR0_STOP) ? 1 : 0;
}

/* Zatrzymuje chip, odtwarza pierscienie, wgrywa blok inicjalizacji
 * (mode musi byc juz ustawione w init_block.mode) i uruchamia karte. */
static int pcnet_start(void)
{
    uint32_t init_addr = (uint32_t)&init_block;
    uint32_t i;

    pcnet_csr_write(CSR0, CSR0_STOP);

    for (i = 0; i < RX_RING_COUNT; i++) {
        rx_ring[i].addr = (uint32_t)&rx_buffers[i][0];
        rx_ring[i].bcnt = (uint16_t)(-(int)RX_BUF_SIZE);
        rx_ring[i].mcnt = 0;
        rx_ring[i].reserved = 0;
        rx_ring[i].status = RMD_OWN;
    }
    for (i = 0; i < TX_RING_COUNT; i++) {
        tx_ring[i].addr = (uint32_t)&tx_buffers[i][0];
        tx_ring[i].bcnt = 0;
        tx_ring[i].misc = 0;
        tx_ring[i].reserved = 0;
        tx_ring[i].status = 0;
    }
    rx_cur = 0;
    tx_cur = 0;

    init_block.tlen_rlen = (uint16_t)(((uint16_t)TX_RLEN << 12) | ((uint16_t)RX_RLEN << 4));
    for (i = 0; i < 6; i++) init_block.padr[i] = device_mac[i];
    init_block.reserved = 0;
    init_block.rdra = (uint32_t)&rx_ring[0];
    init_block.tdra = (uint32_t)&tx_ring[0];

    __asm__ volatile("" : : : "memory");

    pcnet_csr_write(CSR1, (uint16_t)(init_addr & 0xFFFF));
    pcnet_csr_write(CSR2, (uint16_t)(init_addr >> 16));

    pcnet_csr_write(CSR0, CSR0_INIT);
    if (!pcnet_wait_set(CSR0, CSR0_IDON, 1000000U)) return 0;
    pcnet_csr_write(CSR0, CSR0_IDON);          /* potwierdz IDON */
    pcnet_csr_write(CSR0, CSR0_STRT);          /* start, bez IENA = polling */
    return 1;
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int pcnet_init(void)
{
    uint32_t i;
    int valid = 0;

    if (ready) return 1;
    if (!pcnet_find_device()) return 0;
    if (!pcnet_sw_reset()) return 0;

    /* Struktury 32-bitowe (chip musi byc zatrzymany). */
    pcnet_bcr_write(BCR20, BCR20_SWSTYLE_32);
    /* Automatyczny wybor medium. */
    pcnet_bcr_write(BCR2, (uint16_t)(pcnet_bcr_read(BCR2) | BCR2_ASEL));

    /* MAC z APROM (pierwsze 6 bajtow portow I/O karty). */
    for (i = 0; i < 6; i++) device_mac[i] = inb(io_base + PCNET_APROM + i);
    for (i = 0; i < 6; i++) {
        if (device_mac[i] != 0x00 && device_mac[i] != 0xFF) valid = 1;
    }
    if (!valid) {
        device_mac[0] = 0x02; device_mac[1] = 0x00; device_mac[2] = 0x00;
        device_mac[3] = 0x52; device_mac[4] = 0x54; device_mac[5] = 0x53;
    }

    init_block.mode = 0;
    init_block.ladrf[0] = 0;
    init_block.ladrf[1] = 0;

    if (!pcnet_start()) return 0;

    ready = 1;
    return 1;
}

int pcnet_is_ready(void)
{
    return ready;
}

int pcnet_get_mac(uint8_t mac[6])
{
    uint32_t i;
    if (!ready || !mac) return 0;
    for (i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int pcnet_send(const void *frame, uint16_t length)
{
    volatile struct pcnet_tmd *tmd;
    uint32_t idx, i, t;
    uint32_t send_len;
    const uint8_t *src = (const uint8_t *)frame;

    if (!ready || !frame || length == 0 || length > MAX_FRAME) return -1;

    idx = tx_cur;
    tmd = &tx_ring[idx];

    /* Poczekaj az karta zwolni deskryptor (OWN = 0). */
    for (t = 0; t < 5000000U; t++) {
        if (!(tmd->status & TMD_OWN)) break;
        __asm__ volatile("pause" ::: "memory");
    }
    if (tmd->status & TMD_OWN) {
        stats.tx_errors++;
        return 0;
    }

    for (i = 0; i < length; i++) tx_buffers[idx][i] = src[i];
    send_len = length;
    while (send_len < MIN_FRAME) tx_buffers[idx][send_len++] = 0;   /* dopelnienie */

    tmd->bcnt = (uint16_t)(-(int)send_len);
    tmd->misc = 0;
    __asm__ volatile("" : : : "memory");
    tmd->status = (uint16_t)(TMD_OWN | TMD_STP | TMD_ENP);
    __asm__ volatile("" : : : "memory");

    tx_cur = (tx_cur + 1) % TX_RING_COUNT;

    /* Zapotrzebowanie na natychmiastowe nadawanie. */
    pcnet_csr_write(CSR0, CSR0_TDMD);

    for (t = 0; t < 10000000U; t++) {
        if (!(tmd->status & TMD_OWN)) break;
    }
    if (tmd->status & TMD_OWN) {
        stats.tx_errors++;
        return 0;
    }

    if (tmd->status & TMD_ERR) {
        stats.tx_errors++;
        return 0;
    }
    if (tmd->status & TMD_MORE) stats.tx_aborted++;
    if (tmd->status & (TMD_MORE | TMD_ONE)) stats.tx_collisions++;

    stats.tx_packets++;
    return 1;
}

int pcnet_receive(void *frame, uint16_t capacity)
{
    volatile struct pcnet_rmd *rmd;
    uint16_t status, csr0;
    uint32_t length, i, tries;
    uint8_t *dst = (uint8_t *)frame;

    if (!ready || !frame || capacity == 0) return -1;

    for (tries = 0; tries < RX_RING_COUNT; tries++) {
        rmd = &rx_ring[rx_cur];

        if (rmd->status & RMD_OWN) {
            /* nic nowego - przy okazji kasujemy flagi bledow */
            csr0 = pcnet_csr_read(CSR0);
            if (csr0 & CSR0_MERR) { stats.mem_errors++; pcnet_csr_write(CSR0, CSR0_MERR); }
            if (csr0 & CSR0_MISS) { stats.rx_missed++;  pcnet_csr_write(CSR0, CSR0_MISS); }
            if (csr0 & CSR0_BABL) { stats.tx_babble++;  pcnet_csr_write(CSR0, CSR0_BABL); }
            return 0;
        }

        status = rmd->status;
        length = rmd->mcnt & 0x0FFFU;

        int good = 0;
        if (status & RMD_ERR) {
            stats.rx_errors++;
            if (status & RMD_CRC)  stats.rx_crc_errors++;
            if (status & RMD_FRAM) stats.rx_frame_errors++;
            if (status & RMD_OFLO) stats.rx_over_errors++;
            if (status & RMD_BUFF) stats.rx_buffer_errors++;
        } else if ((status & (RMD_STP | RMD_ENP)) == (RMD_STP | RMD_ENP) && length > 4) {
            length -= 4;                       /* mcnt zawiera 4 bajty FCS */
            if (length <= capacity) {
                for (i = 0; i < length; i++) dst[i] = rx_buffers[rx_cur][i];
                good = 1;
            } else {
                stats.rx_errors++;
            }
        } else {
            stats.rx_errors++;
        }

        /* Zwroc deskryptor karcie. */
        rmd->bcnt = (uint16_t)(-(int)RX_BUF_SIZE);
        rmd->mcnt = 0;
        __asm__ volatile("" : : : "memory");
        rmd->status = RMD_OWN;
        __asm__ volatile("" : : : "memory");
        rx_cur = (rx_cur + 1) % RX_RING_COUNT;

        if (good) {
            stats.rx_packets++;
            return (int)length;
        }
        /* zla ramka: sprawdz nastepny deskryptor */
    }
    return 0;
}

void pcnet_shutdown(void)
{
    if (!ready) return;
    pcnet_csr_write(CSR0, CSR0_STOP);
    ready = 0;
}

int pcnet_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                    uint32_t *tx_errors, uint32_t *rx_errors,
                    uint32_t *tx_collisions, uint32_t *rx_crc_errors)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = stats.tx_packets;
    *rx_packets = stats.rx_packets;
    *tx_errors = stats.tx_errors;
    *rx_errors = stats.rx_errors;
    if (tx_collisions) *tx_collisions = stats.tx_collisions;
    if (rx_crc_errors) *rx_crc_errors = stats.rx_crc_errors;
    return 1;
}

void pcnet_stats_reset(void)
{
    stats.tx_packets = 0;
    stats.rx_packets = 0;
    stats.tx_errors = 0;
    stats.rx_errors = 0;
    stats.tx_collisions = 0;
    stats.tx_aborted = 0;
    stats.rx_crc_errors = 0;
    stats.rx_frame_errors = 0;
    stats.rx_over_errors = 0;
    stats.rx_buffer_errors = 0;
    stats.rx_missed = 0;
    stats.tx_babble = 0;
    stats.mem_errors = 0;
}

/* CRC32 Ethernet (odbity, 0xEDB88320) do haszowania multicast. */
static uint32_t pcnet_crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFU;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint32_t b = 0; b < 8; b++)
            crc = (crc & 1U) ? (0xEDB88320U ^ (crc >> 1)) : (crc >> 1);
    }
    return crc;   /* bez negacji: tak liczy to PCnet / Linux ether_crc_le */
}

/* Dodaje adres multicast do filtra logicznego (bity [31:26] CRC). */
int pcnet_set_multicast(const uint8_t *addr)
{
    uint32_t crc, bit;

    if (!ready || !addr) return 0;

    crc = pcnet_crc32(addr, 6);
    bit = (crc >> 26) & 0x3FU;

    init_block.ladrf[bit >> 5] |= (1U << (bit & 31U));
    return pcnet_start();
}

/* Wlacza / wylacza tryb promiscuous. */
int pcnet_set_promiscuous(uint8_t enable)
{
    if (!ready) return 0;
    if (enable) init_block.mode |= MODE_PROM;
    else init_block.mode = (uint16_t)(init_block.mode & ~MODE_PROM);
    return pcnet_start();
}

/* Test petli wewnetrznej: ramka adresowana do tej karty wraca przez RX. */
int pcnet_loopback_test(void)
{
    uint8_t test_frame[64];
    uint8_t rx_frame[256];
    uint32_t i, t;
    int ok = 0;

    if (!ready) return 0;

    for (i = 0; i < 64; i++) test_frame[i] = (uint8_t)i;
    for (i = 0; i < 6; i++) test_frame[i] = device_mac[i];   /* adres docelowy = my */

    init_block.mode |= (uint16_t)(MODE_LOOP | MODE_INTL);
    if (pcnet_start() && pcnet_send(test_frame, 64) > 0) {
        for (t = 0; t < 5000000U; t++) {
            int len = pcnet_receive(rx_frame, sizeof(rx_frame));
            if (len >= 64) {
                int match = 1;
                for (i = 0; i < 64; i++)
                    if (rx_frame[i] != test_frame[i]) { match = 0; break; }
                ok = match;
                break;
            }
            __asm__ volatile("pause" ::: "memory");
        }
    }

    /* Powrot do normalnej pracy. */
    init_block.mode = (uint16_t)(init_block.mode & ~(MODE_LOOP | MODE_INTL));
    pcnet_start();

    return ok;
}