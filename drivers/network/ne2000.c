/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/network/ne2000.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

/* Rejestry NE2000 (DS8390) */
#define NE_REG_CR       0x00
#define NE_REG_PSTART   0x01  /* strona 0, zapis */
#define NE_REG_PSTOP    0x02  /* strona 0, zapis */
#define NE_REG_BNRY     0x03
#define NE_REG_TSR      0x04  /* odczyt */
#define NE_REG_TPSR     0x04  /* zapis */
#define NE_REG_TBCR0    0x05
#define NE_REG_TBCR1    0x06
#define NE_REG_ISR      0x07
#define NE_REG_RSAR0    0x08
#define NE_REG_RSAR1    0x09
#define NE_REG_RBCR0    0x0A
#define NE_REG_RBCR1    0x0B
#define NE_REG_RCR      0x0C
#define NE_REG_TCR      0x0D
#define NE_REG_DCR      0x0E
#define NE_REG_IMR      0x0F
#define NE_REG_DATAPORT 0x10
#define NE_REG_RESET    0x1F

/* Strona 1 */
#define NE_REG_PAR0     0x01
#define NE_REG_CURR     0x07
#define NE_REG_MAR0     0x08

/* CR */
#define CR_STP    0x01
#define CR_STA    0x02
#define CR_TXP    0x04
#define CR_RD0    0x08  /* odczyt zdalny */
#define CR_RD1    0x10  /* zapis zdalny */
#define CR_RD2    0x20  /* abort / brak DMA */
#define CR_PAGE0  0x00
#define CR_PAGE1  0x40
#define CR_PAGE2  0x80

/* ISR */
#define ISR_PRX   0x01
#define ISR_PTX   0x02
#define ISR_RXE   0x04
#define ISR_TXE   0x08
#define ISR_OVW   0x10
#define ISR_CNT   0x20
#define ISR_RDC   0x40
#define ISR_RST   0x80

/* RCR */
#define RCR_SEP   0x01
#define RCR_AR    0x02
#define RCR_AB    0x04
#define RCR_AM    0x08
#define RCR_PRO   0x10
#define RCR_MON   0x20

/* TCR */
#define TCR_CRC   0x01
#define TCR_LOOP  0x02

/* DCR */
#define DCR_WTS   0x01
#define DCR_LS    0x08
#define DCR_FT10  0x40
#define DCR_VALUE (DCR_WTS | DCR_LS | DCR_FT10)  /* 0x49 */

/* TSR */
#define TSR_PTX   0x01
#define TSR_COL   0x04
#define TSR_ABT   0x08
#define TSR_CRS   0x10
#define TSR_FU    0x20

/* RSR */
#define RSR_PRX   0x01
#define RSR_CRC   0x02
#define RSR_FAE   0x04
#define RSR_FO    0x08
#define RSR_MPA   0x10

/* Pamiec karty: strony 0x40..0x7F (16 KB) */
#define NE_TX_START_PAGE   0x40  /* 6 stron = 1536 B na ramke TX */
#define NE_RX_START_PAGE   0x46
#define NE_RX_STOP_PAGE    0x80

/* PCI */
#define PCI_VENDOR_REALTEK   0x10EC
#define PCI_DEVICE_RTL8029   0x8029
#define PCI_VENDOR_WINBOND   0x1050
#define PCI_DEVICE_WB89C940  0x0940
#define PCI_VENDOR_COMPEX    0x10D8
#define PCI_DEVICE_RL2000    0x0002
#define PCI_VENDOR_KTI       0x10E3
#define PCI_DEVICE_ET32P2    0x0002
#define PCI_VENDOR_NETVIN    0x10BD
#define PCI_DEVICE_NV5000SC  0x0002
#define PCI_VENDOR_VIA       0x1106
#define PCI_DEVICE_VT86C926  0x0926

#define NE_MAX_FRAME    1518
#define NE_MIN_FRAME    60

struct ne2000_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_collisions;
    uint32_t tx_aborts;
    uint32_t tx_carrier_lost;
    uint32_t tx_fifo_underrun;
    uint32_t rx_crc_errors;
    uint32_t rx_frame_errors;
    uint32_t rx_fifo_overrun;
    uint32_t rx_missed_packets;
};

static uint16_t io_base;
static uint8_t device_mac[6];
static uint8_t ready;
static uint8_t next_packet;   /* strona nastepnego pakietu do odczytu */
static struct ne2000_stats stats;
static uint8_t tx_buf[NE_MAX_FRAME + 2];
static uint8_t rx_tmp[4];

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

static void io_delay(void)
{
    inb(0x84);
    inb(0x84);
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

static uint8_t ne_read_reg(uint8_t reg)
{
    return inb(io_base + reg);
}

static void ne_write_reg(uint8_t reg, uint8_t value)
{
    outb(io_base + reg, value);
}

/* Wybor strony rejestrow; zachowuje STA/STP, konczy ewentualne DMA (RD2). */
static void ne_select_page(uint8_t page)
{
    uint8_t cr = ne_read_reg(NE_REG_CR) & (CR_STA | CR_STP);
    outb(io_base + NE_REG_CR, cr | CR_RD2 | (uint8_t)((page & 3) << 6));
}

static int ne_wait_isr_bit(uint8_t bit, uint32_t timeout)
{
    uint32_t t;
    for (t = 0; t < timeout; t++) {
        if (ne_read_reg(NE_REG_ISR) & bit) return 1;
        __asm__ volatile("pause" ::: "memory");
    }
    return 0;
}

/* Zdalne DMA: odczyt len bajtow spod adresu addr w pamieci karty. Strona 0. */
static int ne_dma_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t cnt = (uint16_t)((len + 1) & ~1);
    uint16_t i;

    ne_write_reg(NE_REG_ISR, ISR_RDC);
    ne_write_reg(NE_REG_RBCR0, (uint8_t)(cnt & 0xFF));
    ne_write_reg(NE_REG_RBCR1, (uint8_t)(cnt >> 8));
    ne_write_reg(NE_REG_RSAR0, (uint8_t)(addr & 0xFF));
    ne_write_reg(NE_REG_RSAR1, (uint8_t)(addr >> 8));
    ne_write_reg(NE_REG_CR, CR_RD0 | CR_STA | CR_PAGE0);

    for (i = 0; i < cnt; i += 2) {
        uint16_t w = inw(io_base + NE_REG_DATAPORT);
        buf[i] = (uint8_t)(w & 0xFF);
        if (i + 1 < len) buf[i + 1] = (uint8_t)(w >> 8);
    }

    if (!ne_wait_isr_bit(ISR_RDC, 100000)) return 0;
    ne_write_reg(NE_REG_ISR, ISR_RDC);
    return 1;
}

/* Zdalne DMA: zapis len bajtow (uzupelnione do parzystej liczby) pod addr. */
static int ne_dma_write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    uint16_t cnt = (uint16_t)((len + 1) & ~1);
    uint16_t i;

    ne_write_reg(NE_REG_ISR, ISR_RDC);
    ne_write_reg(NE_REG_RBCR0, (uint8_t)(cnt & 0xFF));
    ne_write_reg(NE_REG_RBCR1, (uint8_t)(cnt >> 8));
    ne_write_reg(NE_REG_RSAR0, (uint8_t)(addr & 0xFF));
    ne_write_reg(NE_REG_RSAR1, (uint8_t)(addr >> 8));
    ne_write_reg(NE_REG_CR, CR_RD1 | CR_STA | CR_PAGE0);

    for (i = 0; i < cnt; i += 2) {
        uint16_t lo = buf[i];
        uint16_t hi = (i + 1 < len) ? buf[i + 1] : 0;
        outw(io_base + NE_REG_DATAPORT, (uint16_t)(lo | (hi << 8)));
    }

    if (!ne_wait_isr_bit(ISR_RDC, 100000)) return 0;
    ne_write_reg(NE_REG_ISR, ISR_RDC);
    return 1;
}

static void ne_set_boundary(uint8_t next)
{
    uint8_t bnry = (next <= NE_RX_START_PAGE) ? (NE_RX_STOP_PAGE - 1) : (uint8_t)(next - 1);
    ne_write_reg(NE_REG_BNRY, bnry);
}

static uint8_t ne_read_curr(void)
{
    uint8_t curr;
    ne_select_page(1);
    curr = ne_read_reg(NE_REG_CURR);
    ne_select_page(0);
    return curr;
}

/* Obsluga przepelnienia pierscienia: odrzucamy zawartosc i startujemy od nowa. */
static void ne_handle_overflow(void)
{
    uint8_t curr;
    uint32_t t;

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    for (t = 0; t < 10000; t++) io_delay();
    ne_write_reg(NE_REG_RBCR0, 0);
    ne_write_reg(NE_REG_RBCR1, 0);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);

    curr = ne_read_curr();
    next_packet = curr;
    ne_set_boundary(next_packet);

    ne_write_reg(NE_REG_ISR, ISR_OVW);
    ne_write_reg(NE_REG_TCR, 0x00);
    stats.rx_fifo_overrun++;
}

static int ne_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            uint16_t device = (uint16_t)(id0 >> 16);
            uint32_t class_code;
            uint32_t bar0;
            uint32_t command;
            int match = 0;

            if (vendor == 0xFFFF) continue;

            /* klasa bazowa 0x02 = sieciowa (bity 31:24) */
            class_code = pci_read32((uint8_t)bus, slot, 0, 0x08);
            if (((class_code >> 24) & 0xFF) != 0x02) continue;

            if (vendor == PCI_VENDOR_REALTEK && device == PCI_DEVICE_RTL8029) match = 1;
            else if (vendor == PCI_VENDOR_WINBOND && device == PCI_DEVICE_WB89C940) match = 1;
            else if (vendor == PCI_VENDOR_COMPEX && device == PCI_DEVICE_RL2000) match = 1;
            else if (vendor == PCI_VENDOR_KTI && device == PCI_DEVICE_ET32P2) match = 1;
            else if (vendor == PCI_VENDOR_NETVIN && device == PCI_DEVICE_NV5000SC) match = 1;
            else if (vendor == PCI_VENDOR_VIA && device == PCI_DEVICE_VT86C926) match = 1;
            if (!match) continue;

            bar0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            if (bar0 == 0 || bar0 == 0xFFFFFFFF || !(bar0 & 1)) return 0;
            io_base = (uint16_t)(bar0 & 0xFFFC);

            /* I/O space (bit0) + memory (bit1) + bus master (bit2) */
            command = pci_read32((uint8_t)bus, slot, 0, 0x04);
            pci_write32((uint8_t)bus, slot, 0, 0x04, (command & 0xFFFF) | 0x00000007);

            return 1;
        }
    }
    return 0;
}

static int ne_read_mac_from_prom(void)
{
    uint8_t prom[32];
    uint32_t i;
    int doubled = 1;
    int all_zero = 1;

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);
    if (!ne_dma_read(0, prom, 32)) return 0;

    /* karty 16-bitowe maja kazdy bajt PROM zdublowany */
    for (i = 0; i < 6; i++) {
        if (prom[2 * i] != prom[2 * i + 1]) doubled = 0;
    }
    for (i = 0; i < 6; i++) {
        device_mac[i] = doubled ? prom[2 * i] : prom[i];
        if (device_mac[i]) all_zero = 0;
    }
    return !all_zero;
}

int ne2000_init(void)
{
    uint32_t i;
    uint8_t v;

    if (ready) return 1;
    if (!ne_find_device()) return 0;

    /* Reset sprzetowy przez port 0x1F */
    v = ne_read_reg(NE_REG_RESET);
    ne_write_reg(NE_REG_RESET, v);
    ne_wait_isr_bit(ISR_RST, 100000);
    ne_write_reg(NE_REG_ISR, 0xFF);

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_write_reg(NE_REG_DCR, DCR_VALUE);
    ne_write_reg(NE_REG_RBCR0, 0x00);
    ne_write_reg(NE_REG_RBCR1, 0x00);
    ne_write_reg(NE_REG_RCR, RCR_MON);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_write_reg(NE_REG_IMR, 0x00);
    ne_write_reg(NE_REG_ISR, 0xFF);

    if (!ne_read_mac_from_prom()) return 0;

    /* Pierscien odbiorczy */
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_write_reg(NE_REG_PSTART, NE_RX_START_PAGE);
    ne_write_reg(NE_REG_PSTOP, NE_RX_STOP_PAGE);
    ne_write_reg(NE_REG_BNRY, NE_RX_START_PAGE);
    ne_write_reg(NE_REG_TPSR, NE_TX_START_PAGE);

    /* Strona 1: adres fizyczny, CURR, MAR */
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE1);
    for (i = 0; i < 6; i++) ne_write_reg((uint8_t)(NE_REG_PAR0 + i), device_mac[i]);
    ne_write_reg(NE_REG_CURR, NE_RX_START_PAGE + 1);
    for (i = 0; i < 8; i++) ne_write_reg((uint8_t)(NE_REG_MAR0 + i), 0x00);
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);

    next_packet = NE_RX_START_PAGE + 1;

    ne_write_reg(NE_REG_ISR, 0xFF);
    ne_write_reg(NE_REG_IMR, 0x00);   /* pracujemy w trybie pollingu */
    ne_write_reg(NE_REG_RCR, RCR_AB);
    ne_write_reg(NE_REG_TCR, 0x00);
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);

    ready = 1;
    return 1;
}

int ne2000_is_ready(void)
{
    return ready;
}

int ne2000_get_mac(uint8_t mac[6])
{
    uint32_t i;
    if (!ready || !mac) return 0;
    for (i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int ne2000_send(const void *frame, uint16_t length)
{
    uint32_t t;
    uint16_t send_len;
    uint8_t isr;
    const uint8_t *src = (const uint8_t *)frame;

    if (!ready || !frame || length == 0 || length > NE_MAX_FRAME) return -1;

    /* poczekaj az poprzednia ramka zostanie nadana */
    for (t = 0; t < 1000000U; t++) {
        if (!(ne_read_reg(NE_REG_CR) & CR_TXP)) break;
        __asm__ volatile("pause" ::: "memory");
    }
    if (t == 1000000U) { stats.tx_errors++; return 0; }

    for (t = 0; t < length; t++) tx_buf[t] = src[t];
    send_len = length;
    while (send_len < NE_MIN_FRAME) tx_buf[send_len++] = 0;

    ne_select_page(0);
    if (!ne_dma_write((uint16_t)(NE_TX_START_PAGE << 8), tx_buf, send_len)) {
        stats.tx_errors++;
        return 0;
    }

    ne_write_reg(NE_REG_ISR, ISR_PTX | ISR_TXE);
    ne_write_reg(NE_REG_TPSR, NE_TX_START_PAGE);
    ne_write_reg(NE_REG_TBCR0, (uint8_t)(send_len & 0xFF));
    ne_write_reg(NE_REG_TBCR1, (uint8_t)(send_len >> 8));
    ne_write_reg(NE_REG_CR, CR_TXP | CR_RD2 | CR_STA | CR_PAGE0);

    for (t = 0; t < 1000000U; t++) {
        isr = ne_read_reg(NE_REG_ISR);
        if (isr & (ISR_PTX | ISR_TXE)) break;
        __asm__ volatile("pause" ::: "memory");
    }

    if (t == 1000000U || (isr & ISR_TXE)) {
        uint8_t tsr = ne_read_reg(NE_REG_TSR);
        if (tsr & TSR_ABT) stats.tx_aborts++;
        if (tsr & TSR_COL) stats.tx_collisions++;
        if (tsr & TSR_CRS) stats.tx_carrier_lost++;
        if (tsr & TSR_FU) stats.tx_fifo_underrun++;
        stats.tx_errors++;
        ne_write_reg(NE_REG_ISR, ISR_PTX | ISR_TXE);
        return 0;
    }

    ne_write_reg(NE_REG_ISR, ISR_PTX);
    stats.tx_packets++;
    return 1;
}

int ne2000_receive(void *frame, uint16_t capacity)
{
    uint8_t isr;
    uint8_t curr;
    uint8_t status, next;
    uint16_t total, data_len;
    uint32_t addr, end;

    if (!ready || !frame || capacity == 0) return -1;

    ne_select_page(0);
    isr = ne_read_reg(NE_REG_ISR);
    if (isr & ISR_OVW) {
        ne_handle_overflow();
        return 0;
    }
    if (isr & (ISR_PRX | ISR_RXE)) ne_write_reg(NE_REG_ISR, ISR_PRX | ISR_RXE);

    curr = ne_read_curr();
    if (curr == next_packet) return 0;

    /* naglowek: status, next_page, len_lo, len_hi */
    addr = (uint32_t)next_packet << 8;
    if (!ne_dma_read((uint16_t)addr, rx_tmp, 4)) { stats.rx_errors++; return 0; }
    status = rx_tmp[0];
    next   = rx_tmp[1];
    total  = (uint16_t)(rx_tmp[2] | ((uint16_t)rx_tmp[3] << 8));

    if (next < NE_RX_START_PAGE || next >= NE_RX_STOP_PAGE || total < 4 + 14 || total > NE_MAX_FRAME + 4 + 4) {
        /* uszkodzony pierscien - zsynchronizuj z CURR */
        stats.rx_errors++;
        next_packet = curr;
        ne_set_boundary(next_packet);
        return 0;
    }

    data_len = (uint16_t)(total - 4);

    if (!(status & RSR_PRX) || data_len > capacity) {
        if (status & RSR_CRC) stats.rx_crc_errors++;
        if (status & RSR_FAE) stats.rx_frame_errors++;
        if (status & RSR_FO) stats.rx_fifo_overrun++;
        if (status & RSR_MPA) stats.rx_missed_packets++;
        stats.rx_errors++;
        next_packet = next;
        ne_set_boundary(next_packet);
        return 0;
    }

    /* dane zaczynaja sie 4 bajty za poczatkiem strony; obsluz zawiniecie pierscienia */
    addr += 4;
    end = (uint32_t)NE_RX_STOP_PAGE << 8;
    if (addr + data_len > end) {
        uint16_t first = (uint16_t)(end - addr);
        if (!ne_dma_read((uint16_t)addr, (uint8_t *)frame, first) ||
            !ne_dma_read((uint16_t)(NE_RX_START_PAGE << 8), (uint8_t *)frame + first,
                         (uint16_t)(data_len - first))) {
            stats.rx_errors++;
            return 0;
        }
    } else {
        if (!ne_dma_read((uint16_t)addr, (uint8_t *)frame, data_len)) {
            stats.rx_errors++;
            return 0;
        }
    }

    next_packet = next;
    ne_set_boundary(next_packet);
    stats.rx_packets++;
    return (int)data_len;
}

void ne2000_shutdown(void)
{
    if (!ready) return;
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_write_reg(NE_REG_IMR, 0x00);
    ne_write_reg(NE_REG_ISR, 0xFF);
    ne_write_reg(NE_REG_RCR, RCR_MON);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ready = 0;
}

int ne2000_loopback_test(void)
{
    uint8_t test_frame[64];
    uint8_t rx_frame[256];
    uint32_t i;
    int rc;

    if (!ready) return 0;
    for (i = 0; i < 64; i++) test_frame[i] = (uint8_t)i;

    ne_select_page(0);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_write_reg(NE_REG_RCR, RCR_AB | RCR_AM | RCR_PRO);

    rc = ne2000_send(test_frame, 64);
    if (rc <= 0) {
        ne_write_reg(NE_REG_TCR, 0x00);
        ne_write_reg(NE_REG_RCR, RCR_AB);
        return 0;
    }

    for (i = 0; i < 1000000; i++) {
        int len = ne2000_receive(rx_frame, sizeof(rx_frame));
        if (len >= 64) {
            int match = 1;
            for (uint32_t j = 0; j < 64; j++) {
                if (rx_frame[j] != test_frame[j]) { match = 0; break; }
            }
            ne_write_reg(NE_REG_TCR, 0x00);
            ne_write_reg(NE_REG_RCR, RCR_AB);
            return match;
        }
        __asm__ volatile("pause" ::: "memory");
    }

    ne_write_reg(NE_REG_TCR, 0x00);
    ne_write_reg(NE_REG_RCR, RCR_AB);
    return 0;
}

void ne2000_stats_reset(void)
{
    uint8_t *p = (uint8_t *)&stats;
    uint32_t i;
    for (i = 0; i < sizeof(stats); i++) p[i] = 0;
}

int ne2000_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                     uint32_t *tx_errors, uint32_t *rx_errors)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = stats.tx_packets;
    *rx_packets = stats.rx_packets;
    *tx_errors = stats.tx_errors;
    *rx_errors = stats.rx_errors;
    return 1;
}

int ne2000_set_multicast(const uint8_t *addr)
{
    uint32_t i;
    (void)addr;
    if (!ready) return 0;
    ne_select_page(1);
    for (i = 0; i < 8; i++) ne_write_reg((uint8_t)(NE_REG_MAR0 + i), 0xFF);  /* przyjmij cały multicast */
    ne_select_page(0);
    ne_write_reg(NE_REG_RCR, RCR_AB | RCR_AM);
    return 1;
}