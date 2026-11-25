/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <drivers/network/ne2000_isa.h>
#include <kernel/memory/idt.h>
#include <kernel/memory/gdt.h>
#include <kernel/sys/pik.h>

/* ------------------------------------------------------------------ */
/* Porty I/O (x86)                                                    */
/* ------------------------------------------------------------------ */

static void ne_isa_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t ne_isa_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void ne_isa_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static uint16_t ne_isa_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* Krotkie opoznienie odzyskiwania szyny (port 0x80 - standardowy port opoznien). */
static void ne_isa_delay(void)
{
    ne_isa_outb(0x80, 0);
}

/* ------------------------------------------------------------------ */
/* Blokada: spinlock + wylaczone przerwania na czas sekcji            */
/* ------------------------------------------------------------------ */

typedef struct {
    volatile uint32_t locked;
} ne_spinlock_t;

static ne_spinlock_t ne_lock;

static void ne_spin_init(ne_spinlock_t *l)
{
    __atomic_store_n(&l->locked, 0U, __ATOMIC_RELEASE);
}

static void ne_spin_lock(ne_spinlock_t *l)
{
    while (__atomic_exchange_n(&l->locked, 1U, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&l->locked, __ATOMIC_RELAXED))
            __asm__ volatile("pause" ::: "memory");
    }
}

static void ne_spin_unlock(ne_spinlock_t *l)
{
    __atomic_store_n(&l->locked, 0U, __ATOMIC_RELEASE);
}

/* Zapamietaj flagi, wylacz przerwania, zajmij blokade. */
static unsigned long ne_lock_irqsave(void)
{
    unsigned long flags;
    __asm__ volatile("pushf\n\tpop %0\n\tcli" : "=r"(flags) : : "memory");
    ne_spin_lock(&ne_lock);
    return flags;
}

static void ne_unlock_irqrestore(unsigned long flags)
{
    ne_spin_unlock(&ne_lock);
    if (flags & 0x200UL) __asm__ volatile("sti" ::: "memory");
}

/* ------------------------------------------------------------------ */
/* Rejestry DS8390 / NE2000                                           */
/* ------------------------------------------------------------------ */

/* Strona 0 */
#define NE_REG_CR        0x00
#define NE_REG_PSTART    0x01  /* zapis */
#define NE_REG_PSTOP     0x02  /* zapis */
#define NE_REG_BNRY      0x03
#define NE_REG_TSR       0x04  /* odczyt */
#define NE_REG_TPSR      0x04  /* zapis */
#define NE_REG_TBCR0     0x05
#define NE_REG_TBCR1     0x06
#define NE_REG_ISR       0x07
#define NE_REG_RSAR0     0x08
#define NE_REG_RSAR1     0x09
#define NE_REG_RBCR0     0x0A
#define NE_REG_RBCR1     0x0B
#define NE_REG_RCR       0x0C  /* zapis */
#define NE_REG_TCR       0x0D  /* zapis */
#define NE_REG_DCR       0x0E  /* zapis */
#define NE_REG_IMR       0x0F  /* zapis */
#define NE_REG_CNTR0     0x0D  /* odczyt: bledy wyrownania ramki */
#define NE_REG_CNTR1     0x0E  /* odczyt: bledy CRC */
#define NE_REG_CNTR2     0x0F  /* odczyt: utracone pakiety */
#define NE_REG_DATAPORT  0x10
#define NE_REG_RESET     0x1F

/* Strona 1 */
#define NE_REG_PAR0      0x01
#define NE_REG_CURR      0x07
#define NE_REG_MAR0      0x08

/* CR */
#define CR_STP      0x01
#define CR_STA      0x02
#define CR_TXP      0x04
#define CR_RD0      0x08  /* Remote DMA read */
#define CR_RD1      0x10  /* Remote DMA write */
#define CR_RD2      0x20  /* abort / brak DMA */
#define CR_PAGE0    0x00
#define CR_PAGE1    0x40
#define CR_PAGE2    0x80

/* ISR */
#define ISR_PRX     0x01
#define ISR_PTX     0x02
#define ISR_RXE     0x04
#define ISR_TXE     0x08
#define ISR_OVW     0x10
#define ISR_CNT     0x20
#define ISR_RDC     0x40
#define ISR_RST     0x80

/* RCR */
#define RCR_SEP     0x01
#define RCR_AR      0x02
#define RCR_AB      0x04
#define RCR_AM      0x08
#define RCR_PRO     0x10
#define RCR_MON     0x20

/* TCR */
#define TCR_CRC     0x01
#define TCR_LB0     0x02
#define TCR_LB1     0x04
#define TCR_ATD     0x08
#define TCR_LOOP    TCR_LB0   /* wewnetrzny loopback (tryb 1) */

/* DCR: WTS = transfery 16-bitowe, LS = normalna praca (bez loopbacku), FT = prog FIFO */
#define DCR_WTS     0x01
#define DCR_LS      0x08
#define DCR_FT10    0x40
#define DCR_VALUE   (DCR_WTS | DCR_LS | DCR_FT10)  /* 0x49 */

/* TSR */
#define TSR_PTX     0x01
#define TSR_COL     0x04
#define TSR_ABT     0x08
#define TSR_CRS     0x10
#define TSR_FU      0x20

/* RSR */
#define RSR_PRX     0x01
#define RSR_CRC     0x02
#define RSR_FAE     0x04
#define RSR_FO      0x08
#define RSR_MPA     0x10

/* ------------------------------------------------------------------ */
/* Uklad pamieci karty                                                */
/* ------------------------------------------------------------------ */

#define NE_TX_START_PAGE   0x40  /* bufor nadawczy: strony 0x40..0x45 (1536 B) */
#define NE_RX_START_PAGE   0x46  /* pierscien odbiorczy: 0x46..0x7F */
#define NE_RX_STOP_PAGE    0x80

#define NE_MAX_FRAME    1518
#define NE_MIN_FRAME    60
#define NE_DMA_SCRATCH_ADDR 0x4000
#define NE_PROM_ADDR        0x0000
#define NE_PROM_SIZE        32

#define NE_IRQ_MASK     (ISR_PRX | ISR_RXE | ISR_OVW)

#define NE_POLL_LIMIT   200000U

static const uint16_t ne_isa_ports[4] = { 0x300, 0x320, 0x340, 0x360 };

/* ------------------------------------------------------------------ */
/* Stan sterownika                                                    */
/* ------------------------------------------------------------------ */

static uint16_t ne_io_base;
static uint8_t  ne_irq_num;
static uint8_t  ne_irq_enabled;
static uint8_t  ne_ready;
static uint8_t  ne_mac[6];
static uint8_t  ne_next_packet;
static uint8_t  ne_rcr;                 /* biezacy RCR (zachowywany) */
static uint8_t  ne_tcr;                 /* biezacy TCR (0 = normalna praca) */
static volatile uint8_t ne_isr_shadow;  /* bity ISR skasowane przez handler IRQ */
static struct ne2000_isa_stats ne_stats;

/* ------------------------------------------------------------------ */
/* Dostep do rejestrow                                                */
/* ------------------------------------------------------------------ */

static uint8_t ne_read_reg(uint8_t reg)
{
    return ne_isa_inb((uint16_t)(ne_io_base + reg));
}

static void ne_write_reg(uint8_t reg, uint8_t value)
{
    ne_isa_outb((uint16_t)(ne_io_base + reg), value);
}

/* Wybor strony rejestrow; zachowuje STA/STP. */
static void ne_select_page(uint8_t page)
{
    uint8_t cr = ne_read_reg(NE_REG_CR) & (CR_STA | CR_STP);
    ne_write_reg(NE_REG_CR, (uint8_t)(cr | CR_RD2 | ((page & 3) << 6)));
    ne_isa_delay();
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

/* ISR z ukladu + bity skasowane wczesniej przez handler przerwania. */
static uint8_t ne_isr_collect(void)
{
    uint8_t hw = ne_read_reg(NE_REG_ISR);
    uint8_t sh = __atomic_exchange_n(&ne_isr_shadow, 0, __ATOMIC_ACQ_REL);
    return (uint8_t)(hw | sh);
}

static void ne_apply_modes(void)
{
    ne_select_page(0);
    ne_write_reg(NE_REG_TCR, ne_tcr);
    ne_write_reg(NE_REG_RCR, ne_rcr);
}

static uint8_t ne_bnry_of(uint8_t next)
{
    return (uint8_t)(next == NE_RX_START_PAGE ? NE_RX_STOP_PAGE - 1 : next - 1);
}

/* ------------------------------------------------------------------ */
/* Remote DMA (pamiec karty). Wymaga skonfigurowanego DCR (WTS=1).    */
/* ------------------------------------------------------------------ */

static int ne_dma_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t cnt = (uint16_t)((len + 1) & ~1U);
    uint16_t i;

    ne_write_reg(NE_REG_ISR, ISR_RDC);
    ne_write_reg(NE_REG_RBCR0, (uint8_t)(cnt & 0xFF));
    ne_write_reg(NE_REG_RBCR1, (uint8_t)(cnt >> 8));
    ne_write_reg(NE_REG_RSAR0, (uint8_t)(addr & 0xFF));
    ne_write_reg(NE_REG_RSAR1, (uint8_t)(addr >> 8));
    ne_write_reg(NE_REG_CR, CR_RD0 | CR_STA | CR_PAGE0);

    for (i = 0; i < cnt; i += 2) {
        uint16_t w = ne_isa_inw((uint16_t)(ne_io_base + NE_REG_DATAPORT));
        buf[i] = (uint8_t)(w & 0xFF);
        if (i + 1 < len) buf[i + 1] = (uint8_t)(w >> 8);
    }

    if (!ne_wait_isr_bit(ISR_RDC, NE_POLL_LIMIT)) {
        ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);
        ne_write_reg(NE_REG_ISR, ISR_RDC);
        return 0;
    }
    ne_write_reg(NE_REG_ISR, ISR_RDC);
    return 1;
}

static int ne_dma_write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    uint16_t cnt = (uint16_t)((len + 1) & ~1U);
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
        ne_isa_outw((uint16_t)(ne_io_base + NE_REG_DATAPORT), (uint16_t)(lo | (hi << 8)));
    }

    if (!ne_wait_isr_bit(ISR_RDC, NE_POLL_LIMIT)) {
        ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);
        ne_write_reg(NE_REG_ISR, ISR_RDC);
        return 0;
    }
    ne_write_reg(NE_REG_ISR, ISR_RDC);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Detekcja karty                                                     */
/* ------------------------------------------------------------------ */

/* Reset sprzetowy: odczyt + zapis portu 0x1F, potem czekanie na ISR.RST. */
static int ne_hw_reset(void)
{
    uint8_t v = ne_read_reg(NE_REG_RESET);
    ne_write_reg(NE_REG_RESET, v);
    return ne_wait_isr_bit(ISR_RST, NE_POLL_LIMIT);
}

/* Sprawdza obecnosc DS8390 i poprawnosc pamieci buforowej. */
static int ne_probe_memory(uint16_t base)
{
    uint8_t pattern[64];
    uint8_t readback[64];
    uint16_t i;
    int ok = 1;

    ne_io_base = base;

    if (!ne_hw_reset()) return 0;

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_isa_delay();
    /* Plywajaca szyna zwraca 0xFF - prawdziwy DS8390 oddaje to, co zapisano. */
    if ((ne_read_reg(NE_REG_CR) & (CR_RD2 | CR_TXP | CR_STA | CR_STP)) != (CR_RD2 | CR_STP))
        return 0;
    if (!(ne_read_reg(NE_REG_ISR) & ISR_RST)) return 0;

    /* Konfiguracja potrzebna do poprawnego DMA slowami; odbior wylaczony (monitor), loopback. */
    ne_write_reg(NE_REG_DCR, DCR_VALUE);
    ne_write_reg(NE_REG_RBCR0, 0x00);
    ne_write_reg(NE_REG_RBCR1, 0x00);
    ne_write_reg(NE_REG_RCR, RCR_MON);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_write_reg(NE_REG_IMR, 0x00);
    ne_write_reg(NE_REG_ISR, 0xFF);

    for (i = 0; i < 64; i++) pattern[i] = (uint8_t)(0xA5 ^ i);
    if (!ne_dma_write(NE_DMA_SCRATCH_ADDR, pattern, 64)) ok = 0;
    if (ok && !ne_dma_read(NE_DMA_SCRATCH_ADDR, readback, 64)) ok = 0;
    if (ok) {
        for (i = 0; i < 64; i++) {
            if (readback[i] != pattern[i]) { ok = 0; break; }
        }
    }

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_write_reg(NE_REG_ISR, 0xFF);
    return ok;
}

static int ne_isa_detect(void)
{
    uint32_t i;
    for (i = 0; i < (uint32_t)(sizeof(ne_isa_ports) / sizeof(ne_isa_ports[0])); i++) {
        if (ne_probe_memory(ne_isa_ports[i])) return 1;
    }
    ne_io_base = 0;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Adres MAC                                                          */
/* ------------------------------------------------------------------ */

static int ne_mac_valid(const uint8_t *m)
{
    uint32_t i;
    int zero = 1, ones = 1;
    for (i = 0; i < 6; i++) {
        if (m[i] != 0x00) zero = 0;
        if (m[i] != 0xFF) ones = 0;
    }
    if (zero || ones) return 0;
    if (m[0] & 1U) return 0;   /* bit multicast - nieprawidlowy adres stacji */
    return 1;
}

/*
 * Zrodlo podstawowe: PROM karty (pamiec pod adresem 0). W 16-bitowej NE2000
 * kazdy bajt PROM jest zdwojony (word = MAC[i] | MAC[i] << 8). Rejestry
 * PAR0..5 po resecie sa zerowe, wiec sa tylko zrodlem zapasowym.
 */
static int ne_read_mac(void)
{
    uint8_t prom[NE_PROM_SIZE];
    uint32_t i;
    int dup = 1;

    if (ne_dma_read(NE_PROM_ADDR, prom, NE_PROM_SIZE)) {
        for (i = 0; i < 12; i += 2) {
            if (prom[i] != prom[i + 1]) dup = 0;
        }
        for (i = 0; i < 6; i++) ne_mac[i] = dup ? prom[2 * i] : prom[i];
        if (ne_mac_valid(ne_mac)) return 1;
    }

    /* Zapas: PAR0..PAR5 (np. gdy BIOS/PnP juz je zaprogramowal). */
    ne_select_page(1);
    for (i = 0; i < 6; i++) ne_mac[i] = ne_read_reg((uint8_t)(NE_REG_PAR0 + i));
    ne_select_page(0);
    return ne_mac_valid(ne_mac);
}

/* ------------------------------------------------------------------ */
/* Inicjalizacja DS8390                                               */
/* ------------------------------------------------------------------ */

static int ne_hw_init(void)
{
    uint32_t i;

    if (!ne_hw_reset()) return 0;

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_write_reg(NE_REG_DCR, DCR_VALUE);
    ne_write_reg(NE_REG_RBCR0, 0x00);
    ne_write_reg(NE_REG_RBCR1, 0x00);
    ne_write_reg(NE_REG_RCR, RCR_MON);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_write_reg(NE_REG_IMR, 0x00);
    ne_write_reg(NE_REG_ISR, 0xFF);

    if (!ne_read_mac()) return 0;

    /* Pierscien odbiorczy i bufor nadawczy. */
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_isa_delay();
    ne_write_reg(NE_REG_PSTART, NE_RX_START_PAGE);
    ne_write_reg(NE_REG_PSTOP, NE_RX_STOP_PAGE);
    ne_write_reg(NE_REG_BNRY, NE_RX_START_PAGE);
    ne_write_reg(NE_REG_TPSR, NE_TX_START_PAGE);

    /* Strona 1: adres fizyczny, CURR, filtr multicast. */
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE1);
    ne_isa_delay();
    for (i = 0; i < 6; i++) ne_write_reg((uint8_t)(NE_REG_PAR0 + i), ne_mac[i]);
    ne_write_reg(NE_REG_CURR, (uint8_t)(NE_RX_START_PAGE + 1));
    for (i = 0; i < 8; i++) ne_write_reg((uint8_t)(NE_REG_MAR0 + i), 0x00);
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_isa_delay();

    ne_next_packet = (uint8_t)(NE_RX_START_PAGE + 1);

    ne_write_reg(NE_REG_ISR, 0xFF);
    ne_write_reg(NE_REG_IMR, 0x00);

    /* Start w trybie loopback, dopiero potem normalna praca. */
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);
    ne_isa_delay();

    ne_tcr = 0x00;
    ne_rcr = RCR_AB;
    ne_write_reg(NE_REG_TCR, ne_tcr);
    ne_write_reg(NE_REG_RCR, ne_rcr);

    ne_isr_shadow = 0;
    ne_ready = 1;
    return 1;
}

/* ------------------------------------------------------------------ */
/* API sterownika                                                     */
/* ------------------------------------------------------------------ */

const char *ne2000_isa_driver_name(void)
{
    return "ne2000_isa";
}

int ne2000_isa_init(void)
{
    if (ne_ready) return 1;
    ne_spin_init(&ne_lock);

    if (!ne_isa_detect()) return 0;
    if (!ne_hw_init()) {
        ne_ready = 0;
        ne_io_base = 0;
        return 0;
    }
    return 1;
}

int ne2000_isa_is_ready(void)
{
    return ne_ready;
}

int ne2000_isa_get_mac(uint8_t mac[6])
{
    uint32_t i;
    if (!ne_ready || !mac) return 0;
    for (i = 0; i < 6; i++) mac[i] = ne_mac[i];
    return 1;
}

int ne2000_isa_send(const void *frame, uint16_t length)
{
    uint32_t t;
    uint16_t send_len;
    uint8_t isr = 0, tsr;
    const uint8_t *src = (const uint8_t *)frame;
    static uint8_t tx_buf[NE_MAX_FRAME + 2];
    unsigned long fl;

    if (!ne_ready || !frame || length == 0 || length > NE_MAX_FRAME) return -1;

    fl = ne_lock_irqsave();

    /* Poprzednie nadawanie musi sie skonczyc (bit TXP w CR). */
    for (t = 0; t < NE_POLL_LIMIT; t++) {
        if (!(ne_read_reg(NE_REG_CR) & CR_TXP)) break;
        __asm__ volatile("pause" ::: "memory");
    }
    if (t == NE_POLL_LIMIT) {
        ne_stats.tx_errors++;
        ne_stats.tx_timeouts++;
        ne_unlock_irqrestore(fl);
        return 0;
    }

    for (t = 0; t < length; t++) tx_buf[t] = src[t];
    send_len = length;
    while (send_len < NE_MIN_FRAME) tx_buf[send_len++] = 0x00;

    ne_select_page(0);
    if (!ne_dma_write((uint16_t)(NE_TX_START_PAGE << 8), tx_buf, send_len)) {
        ne_stats.tx_errors++;
        ne_stats.dma_errors++;
        ne_unlock_irqrestore(fl);
        return 0;
    }

    ne_write_reg(NE_REG_ISR, ISR_PTX | ISR_TXE);
    ne_write_reg(NE_REG_TPSR, NE_TX_START_PAGE);
    ne_write_reg(NE_REG_TBCR0, (uint8_t)(send_len & 0xFF));
    ne_write_reg(NE_REG_TBCR1, (uint8_t)(send_len >> 8));
    ne_write_reg(NE_REG_CR, CR_TXP | CR_RD2 | CR_STA | CR_PAGE0);

    for (t = 0; t < NE_POLL_LIMIT; t++) {
        isr = ne_read_reg(NE_REG_ISR);
        if (isr & (ISR_PTX | ISR_TXE)) break;
        __asm__ volatile("pause" ::: "memory");
    }

    if (t == NE_POLL_LIMIT || (isr & ISR_TXE)) {
        tsr = ne_read_reg(NE_REG_TSR);
        if (tsr & TSR_ABT) ne_stats.tx_aborts++;
        if (tsr & TSR_COL) ne_stats.tx_collisions++;
        if (tsr & TSR_CRS) ne_stats.tx_carrier_lost++;
        if (tsr & TSR_FU)  ne_stats.tx_fifo_underrun++;
        ne_stats.tx_errors++;
        if (t == NE_POLL_LIMIT) ne_stats.tx_timeouts++;
        ne_write_reg(NE_REG_ISR, ISR_PTX | ISR_TXE);
        ne_unlock_irqrestore(fl);
        return 0;
    }

    ne_write_reg(NE_REG_ISR, ISR_PTX);
    ne_stats.tx_packets++;
    ne_unlock_irqrestore(fl);
    return 1;
}

/* Odzyskiwanie po przepelnieniu pierscienia RX (procedura z dokumentacji DS8390). */
static void ne_rx_overrun_recover(void)
{
    uint32_t t;

    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    /* Uklad zatrzymuje sie do ok. 1,6 ms - czekaj na ISR.RST. */
    for (t = 0; t < 20000U; t++) {
        if (ne_read_reg(NE_REG_ISR) & ISR_RST) break;
        __asm__ volatile("pause" ::: "memory");
    }
    ne_write_reg(NE_REG_RBCR0, 0x00);
    ne_write_reg(NE_REG_RBCR1, 0x00);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STA | CR_PAGE0);
    ne_isa_delay();
    ne_write_reg(NE_REG_ISR, ISR_OVW);
    ne_write_reg(NE_REG_TCR, ne_tcr);

    ne_stats.rx_errors++;
    ne_stats.rx_overflow_errors++;
}

/* Gdy pierscien jest uszkodzony: porzuc wszystko do CURR. */
static void ne_rx_resync(uint8_t curr)
{
    ne_next_packet = curr;
    ne_write_reg(NE_REG_BNRY, ne_bnry_of(curr));
}

static int ne_receive_locked(void *frame, uint16_t capacity)
{
    uint8_t isr, curr, status, next, rx_hdr[4];
    uint16_t total, data_len;
    uint32_t addr, end;

    ne_select_page(0);
    isr = ne_isr_collect();

    if (isr & ISR_OVW) ne_rx_overrun_recover();

    if (isr & ISR_CNT) {
        ne_stats.rx_frame_errors   += ne_read_reg(NE_REG_CNTR0);
        ne_stats.rx_crc_errors     += ne_read_reg(NE_REG_CNTR1);
        ne_stats.rx_missed_packets += ne_read_reg(NE_REG_CNTR2);
        ne_write_reg(NE_REG_ISR, ISR_CNT);
    }
    if (isr & (ISR_PRX | ISR_RXE)) ne_write_reg(NE_REG_ISR, ISR_PRX | ISR_RXE);

    /* CURR (strona 1) = strona, do ktorej uklad aktualnie zapisuje. */
    ne_select_page(1);
    curr = ne_read_reg(NE_REG_CURR);
    ne_select_page(0);

    if (curr == ne_next_packet) return 0;

    addr = (uint32_t)ne_next_packet << 8;
    if (!ne_dma_read((uint16_t)addr, rx_hdr, 4)) {
        ne_stats.rx_errors++;
        ne_stats.dma_errors++;
        return 0;
    }
    status = rx_hdr[0];
    next   = rx_hdr[1];
    total  = (uint16_t)(rx_hdr[2] | ((uint16_t)rx_hdr[3] << 8));

    if (next < NE_RX_START_PAGE || next >= NE_RX_STOP_PAGE ||
        total < 4 + 14 || total > (uint16_t)(NE_MAX_FRAME + 4 + 4)) {
        ne_stats.rx_errors++;
        ne_rx_resync(curr);
        return 0;
    }

    data_len = (uint16_t)(total - 4);

    if (!(status & RSR_PRX) || data_len > capacity) {
        if (status & RSR_CRC) ne_stats.rx_crc_errors++;
        if (status & RSR_FAE) ne_stats.rx_frame_errors++;
        if (status & RSR_FO)  ne_stats.rx_fifo_overrun++;
        if (status & RSR_MPA) ne_stats.rx_missed_packets++;
        ne_stats.rx_errors++;
        ne_next_packet = next;
        ne_write_reg(NE_REG_BNRY, ne_bnry_of(next));
        return 0;
    }

    /* Dane zaczynaja sie 4 bajty za poczatkiem strony; obsluz zawiniecie pierscienia. */
    addr += 4;
    end = (uint32_t)NE_RX_STOP_PAGE << 8;
    if (addr + (uint32_t)data_len > end) {
        uint16_t first = (uint16_t)(end - addr);
        if (!ne_dma_read((uint16_t)addr, (uint8_t *)frame, first) ||
            !ne_dma_read((uint16_t)(NE_RX_START_PAGE << 8), (uint8_t *)frame + first,
                         (uint16_t)(data_len - first))) {
            ne_stats.rx_errors++;
            ne_stats.dma_errors++;
            return 0;
        }
    } else if (!ne_dma_read((uint16_t)addr, (uint8_t *)frame, data_len)) {
        ne_stats.rx_errors++;
        ne_stats.dma_errors++;
        return 0;
    }

    ne_next_packet = next;
    ne_write_reg(NE_REG_BNRY, ne_bnry_of(next));
    ne_stats.rx_packets++;
    return (int)data_len;
}

int ne2000_isa_receive(void *frame, uint16_t capacity)
{
    unsigned long fl;
    int result;

    if (!ne_ready || !frame || capacity == 0) return -1;

    fl = ne_lock_irqsave();
    result = ne_receive_locked(frame, capacity);
    ne_unlock_irqrestore(fl);
    return result;
}

void ne2000_isa_shutdown(void)
{
    unsigned long fl;
    if (!ne_ready) return;
    fl = ne_lock_irqsave();
    ne_write_reg(NE_REG_CR, CR_RD2 | CR_STP | CR_PAGE0);
    ne_write_reg(NE_REG_IMR, 0x00);
    ne_write_reg(NE_REG_ISR, 0xFF);
    ne_write_reg(NE_REG_RCR, RCR_MON);
    ne_write_reg(NE_REG_TCR, TCR_LOOP);
    ne_ready = 0;
    ne_irq_enabled = 0;
    ne_unlock_irqrestore(fl);
}

int ne2000_isa_loopback_test(void)
{
    uint8_t test_frame[64];
    uint8_t rx_frame[NE_MAX_FRAME];
    uint32_t i, t;
    uint8_t old_rcr, old_tcr;
    unsigned long fl;
    int result = 0;

    if (!ne_ready) return 0;
    for (i = 0; i < 64; i++) test_frame[i] = (uint8_t)i;

    fl = ne_lock_irqsave();
    old_rcr = ne_rcr;
    old_tcr = ne_tcr;
    ne_tcr = TCR_LB0;                              /* loopback wewnetrzny DS8390 */
    ne_rcr = RCR_AB | RCR_AM | RCR_PRO;
    ne_apply_modes();
    ne_write_reg(NE_REG_ISR, 0xFF);
    ne_unlock_irqrestore(fl);

    if (ne2000_isa_send(test_frame, 64) > 0) {
        for (t = 0; t < 20000U && !result; t++) {
            int len = ne2000_isa_receive(rx_frame, sizeof(rx_frame));
            if (len >= 64) {
                int match = 1;
                for (i = 0; i < 64; i++) {
                    if (rx_frame[i] != test_frame[i]) { match = 0; break; }
                }
                if (match) result = 1;   /* stare/obce ramki w pierscieniu pomijamy */
            }
        }
    }

    fl = ne_lock_irqsave();
    ne_tcr = old_tcr;
    ne_rcr = old_rcr;
    ne_apply_modes();
    ne_unlock_irqrestore(fl);
    return result;
}

void ne2000_isa_stats_reset(void)
{
    uint8_t *p = (uint8_t *)&ne_stats;
    uint32_t i;
    unsigned long fl = ne_lock_irqsave();
    for (i = 0; i < sizeof(ne_stats); i++) p[i] = 0;
    ne_unlock_irqrestore(fl);
}

int ne2000_isa_stats_get(struct ne2000_isa_stats *out)
{
    unsigned long fl;
    if (!out) return 0;
    fl = ne_lock_irqsave();
    *out = ne_stats;
    ne_unlock_irqrestore(fl);
    return 1;
}

/* addr = 8-bajtowy filtr hashujacy MAR0..7; NULL = przyjmuj wszystkie multicasty. */
int ne2000_isa_set_multicast(const uint8_t *addr)
{
    uint32_t i;
    unsigned long fl;
    if (!ne_ready) return 0;
    fl = ne_lock_irqsave();
    ne_select_page(1);
    for (i = 0; i < 8; i++) ne_write_reg((uint8_t)(NE_REG_MAR0 + i), addr ? addr[i] : 0xFF);
    ne_select_page(0);
    ne_rcr |= RCR_AM;
    ne_apply_modes();
    ne_unlock_irqrestore(fl);
    return 1;
}

int ne2000_isa_set_promiscuous(int enable)
{
    unsigned long fl;
    if (!ne_ready) return 0;
    fl = ne_lock_irqsave();
    if (enable) ne_rcr |= (RCR_PRO | RCR_AM);
    else        ne_rcr &= (uint8_t)~RCR_PRO;
    ne_apply_modes();
    ne_unlock_irqrestore(fl);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Obsluga przerwan (opcjonalna; domyslnie wylaczona)                 */
/* ------------------------------------------------------------------ */

extern void ne2000_isa_irq_stub(void);

#if defined(__i386__)
/* Wejscie IDT: zachowaj rejestry, wywolaj handler w C, wroc przez iret. */
__asm__(
    ".text\n"
    ".globl ne2000_isa_irq_stub\n"
    "ne2000_isa_irq_stub:\n"
    "    pusha\n"
    "    cld\n"
    "    call ne2000_isa_irq_handler\n"
    "    popa\n"
    "    iret\n"
);
#endif

int ne2000_isa_irq_enable(uint8_t irq)
{
    unsigned long fl;
    if (!ne_ready || irq >= 16) return 0;

    fl = ne_lock_irqsave();
    ne_irq_num = irq;
    idt_set_gate((uint8_t)(0x20 + irq), (uint32_t)ne2000_isa_irq_stub,
                 GDT_RING0_CODE, 0x8E);
    pik_enable_irq(irq);
    ne_irq_enabled = 1;
    ne_select_page(0);
    ne_write_reg(NE_REG_ISR, 0xFF);
    ne_write_reg(NE_REG_IMR, NE_IRQ_MASK);
    ne_unlock_irqrestore(fl);
    return 1;
}

void ne2000_isa_irq_disable(void)
{
    unsigned long fl;
    if (!ne_ready) return;
    fl = ne_lock_irqsave();
    ne_select_page(0);
    ne_write_reg(NE_REG_IMR, 0x00);
    ne_write_reg(NE_REG_ISR, NE_IRQ_MASK);
    ne_irq_enabled = 0;
    ne_isr_shadow = 0;
    ne_unlock_irqrestore(fl);
}

/*
 * Handler IRQ (wywolywany ze stubu, IF=0). Tylko potwierdza zdarzenia i zapisuje
 * je w cieniu ISR - sama obsluge (odczyt ramek, odzyskiwanie po overflow) robi
 * ne2000_isa_receive(). Statystyk tu nie liczymy, zeby nie dublowac petli pollingowej.
 */
int ne2000_isa_irq_handler(void)
{
    uint8_t isr;
    int handled = 0;

    if (ne_ready && ne_irq_enabled) {
        ne_spin_lock(&ne_lock);
        isr = (uint8_t)(ne_read_reg(NE_REG_ISR) & NE_IRQ_MASK);
        if (isr) {
            ne_write_reg(NE_REG_ISR, isr);
            __atomic_fetch_or(&ne_isr_shadow, isr, __ATOMIC_ACQ_REL);
            handled = 1;
        }
        ne_spin_unlock(&ne_lock);
    }

    /* EOI zawsze (takze gdy przerwanie nie bylo nasze) - inaczej PIC blokuje linie. */
    if (ne_irq_num >= 8) ne_isa_outb(0xA0, 0x20);
    ne_isa_outb(0x20, 0x20);
    return handled;
}

/* ------------------------------------------------------------------ */
/* Debug: odczyt rejestrow                                            */
/* ------------------------------------------------------------------ */

void ne2000_isa_register_dump(void)
{
    uint8_t cr, isr, rcr_dummy, imr_dummy, bnry, curr;
    unsigned long fl;
    if (!ne_ready) return;
    fl = ne_lock_irqsave();
    ne_select_page(0);
    cr   = ne_read_reg(NE_REG_CR);
    isr  = ne_read_reg(NE_REG_ISR);
    bnry = ne_read_reg(NE_REG_BNRY);
    ne_select_page(1);
    curr = ne_read_reg(NE_REG_CURR);
    ne_select_page(0);
    ne_unlock_irqrestore(fl);
    rcr_dummy = ne_rcr;
    imr_dummy = ne_tcr;
    (void)cr; (void)isr; (void)bnry; (void)curr; (void)rcr_dummy; (void)imr_dummy;
}