/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <drivers/network/ne1000_isa.h>
#include <kernel/memory/idt.h>
#include <kernel/memory/gdt.h>
#include <kernel/sys/pik.h>

/* 1 = akceptuj tez karty z zdwojonym PROM (NE2000 pracujaca w trybie bajtowym, np. QEMU). */
#ifndef NE1000_ACCEPT_DOUBLED_PROM
#define NE1000_ACCEPT_DOUBLED_PROM 0
#endif

/* ------------------------------------------------------------------ */
/* Porty I/O (x86)                                                    */
/* ------------------------------------------------------------------ */

static void ne1000_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t ne1000_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* Krotkie opoznienie odzyskiwania szyny (port 0x80 - standardowy port opoznien). */
static void ne1000_delay(void)
{
    ne1000_outb(0x80, 0);
}

/* ------------------------------------------------------------------ */
/* Blokada: spinlock + wylaczone przerwania na czas sekcji            */
/* ------------------------------------------------------------------ */

typedef struct {
    volatile uint32_t locked;
} ne1000_spinlock_t;

static ne1000_spinlock_t ne1000_lock;

static void ne1000_spin_init(ne1000_spinlock_t *l)
{
    __atomic_store_n(&l->locked, 0U, __ATOMIC_RELEASE);
}

static void ne1000_spin_lock(ne1000_spinlock_t *l)
{
    while (__atomic_exchange_n(&l->locked, 1U, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&l->locked, __ATOMIC_RELAXED))
            __asm__ volatile("pause" ::: "memory");
    }
}

static void ne1000_spin_unlock(ne1000_spinlock_t *l)
{
    __atomic_store_n(&l->locked, 0U, __ATOMIC_RELEASE);
}

static unsigned long ne1000_lock_irqsave(void)
{
    unsigned long flags;
    __asm__ volatile("pushf\n\tpop %0\n\tcli" : "=r"(flags) : : "memory");
    ne1000_spin_lock(&ne1000_lock);
    return flags;
}

static void ne1000_unlock_irqrestore(unsigned long flags)
{
    ne1000_spin_unlock(&ne1000_lock);
    if (flags & 0x200UL) __asm__ volatile("sti" ::: "memory");
}

/* ------------------------------------------------------------------ */
/* Rejestry DS8390 / NE1000                                           */
/* ------------------------------------------------------------------ */

/* Strona 0 */
#define NE1000_REG_CR        0x00
#define NE1000_REG_PSTART    0x01
#define NE1000_REG_PSTOP     0x02
#define NE1000_REG_BNRY      0x03
#define NE1000_REG_TSR       0x04
#define NE1000_REG_TPSR      0x04
#define NE1000_REG_TBCR0     0x05
#define NE1000_REG_TBCR1     0x06
#define NE1000_REG_ISR       0x07
#define NE1000_REG_RSAR0     0x08
#define NE1000_REG_RSAR1     0x09
#define NE1000_REG_RBCR0     0x0A
#define NE1000_REG_RBCR1     0x0B
#define NE1000_REG_RCR       0x0C
#define NE1000_REG_TCR       0x0D
#define NE1000_REG_DCR       0x0E
#define NE1000_REG_IMR       0x0F
#define NE1000_REG_CNTR0     0x0D
#define NE1000_REG_CNTR1     0x0E
#define NE1000_REG_CNTR2     0x0F
#define NE1000_REG_DATAPORT  0x10
#define NE1000_REG_RESET     0x1F

/* Strona 1 */
#define NE1000_REG_PAR0      0x01
#define NE1000_REG_CURR      0x07
#define NE1000_REG_MAR0      0x08

/* CR */
#define NE1000_CR_STP      0x01
#define NE1000_CR_STA      0x02
#define NE1000_CR_TXP      0x04
#define NE1000_CR_RD0      0x08
#define NE1000_CR_RD1      0x10
#define NE1000_CR_RD2      0x20
#define NE1000_CR_PAGE0    0x00
#define NE1000_CR_PAGE1    0x40
#define NE1000_CR_PAGE2    0x80

/* ISR */
#define NE1000_ISR_PRX     0x01
#define NE1000_ISR_PTX     0x02
#define NE1000_ISR_RXE     0x04
#define NE1000_ISR_TXE     0x08
#define NE1000_ISR_OVW     0x10
#define NE1000_ISR_CNT     0x20
#define NE1000_ISR_RDC     0x40
#define NE1000_ISR_RST     0x80

/* RCR */
#define NE1000_RCR_SEP     0x01
#define NE1000_RCR_AR      0x02
#define NE1000_RCR_AB      0x04
#define NE1000_RCR_AM      0x08
#define NE1000_RCR_PRO     0x10
#define NE1000_RCR_MON     0x20

/* TCR */
#define NE1000_TCR_CRC     0x01
#define NE1000_TCR_LB0     0x02
#define NE1000_TCR_LB1     0x04
#define NE1000_TCR_ATD     0x08

/* DCR: karta 8-bitowa, WTS = 0 (transfery bajtowe), LS = normalna praca, FT = prog FIFO. */
#define NE1000_DCR_LS      0x08
#define NE1000_DCR_FT10    0x40
#define NE1000_DCR_VALUE   (NE1000_DCR_LS | NE1000_DCR_FT10)  /* 0x48 */

/* TSR */
#define NE1000_TSR_PTX     0x01
#define NE1000_TSR_COL     0x04
#define NE1000_TSR_ABT     0x08
#define NE1000_TSR_CRS     0x10
#define NE1000_TSR_FU      0x20

/* RSR */
#define NE1000_RSR_PRX     0x01
#define NE1000_RSR_CRC     0x02
#define NE1000_RSR_FAE     0x04
#define NE1000_RSR_FO      0x08
#define NE1000_RSR_MPA     0x10

/* ------------------------------------------------------------------ */
/* Uklad pamieci karty                                                */
/* ------------------------------------------------------------------ */

/* NE1000: 8 KB RAM pod 0x2000. */
#define NE1000_SM_TX_PAGE      0x20
#define NE1000_SM_RX_START     0x26
#define NE1000_SM_RX_STOP      0x40
/* Karty 16 KB (klony / emulatory): RAM pod 0x4000. */
#define NE1000_LG_TX_PAGE      0x40
#define NE1000_LG_RX_START     0x46
#define NE1000_LG_RX_STOP      0x80

#define NE1000_MAX_FRAME    1518
#define NE1000_MIN_FRAME    60
#define NE1000_PROM_ADDR        0x0000
#define NE1000_PROM_SIZE        32

#define NE1000_IRQ_MASK     (NE1000_ISR_PRX | NE1000_ISR_RXE | NE1000_ISR_OVW)

#define NE1000_POLL_LIMIT   200000U

static const uint16_t ne1000_isa_ports[4] = { 0x300, 0x320, 0x340, 0x360 };

/* ------------------------------------------------------------------ */
/* Stan sterownika                                                    */
/* ------------------------------------------------------------------ */

static uint16_t ne1000_io_base;
static uint8_t  ne1000_irq_num;
static uint8_t  ne1000_irq_enabled;
static uint8_t  ne1000_ready;
static uint8_t  ne1000_mac[6];
static uint8_t  ne1000_next_packet;
static uint8_t  ne1000_rcr;
static uint8_t  ne1000_tcr;
static uint16_t ne1000_mtu;
static uint8_t  ne1000_tx_page = NE1000_SM_TX_PAGE;      /* wybierane przy detekcji */
static uint8_t  ne1000_rx_start = NE1000_SM_RX_START;
static uint8_t  ne1000_rx_stop = NE1000_SM_RX_STOP;
static volatile uint8_t ne1000_isr_shadow;
static struct ne1000_isa_stats ne1000_stats;

static void ne1000_set_layout(uint8_t tx, uint8_t rx_start, uint8_t rx_stop)
{
    ne1000_tx_page = tx;
    ne1000_rx_start = rx_start;
    ne1000_rx_stop = rx_stop;
}

/* ------------------------------------------------------------------ */
/* Dostep do rejestrow                                                */
/* ------------------------------------------------------------------ */

static uint8_t ne1000_read_reg(uint8_t reg)
{
    return ne1000_inb((uint16_t)(ne1000_io_base + reg));
}

static void ne1000_write_reg(uint8_t reg, uint8_t value)
{
    ne1000_outb((uint16_t)(ne1000_io_base + reg), value);
}

static void ne1000_select_page(uint8_t page)
{
    uint8_t cr = ne1000_read_reg(NE1000_REG_CR) & (NE1000_CR_STA | NE1000_CR_STP);
    ne1000_write_reg(NE1000_REG_CR, (uint8_t)(cr | NE1000_CR_RD2 | ((page & 3) << 6)));
    ne1000_delay();
}

static int ne1000_wait_isr_bit(uint8_t bit, uint32_t timeout)
{
    uint32_t t;
    for (t = 0; t < timeout; t++) {
        if (ne1000_read_reg(NE1000_REG_ISR) & bit) return 1;
        __asm__ volatile("pause" ::: "memory");
    }
    return 0;
}

static uint8_t ne1000_isr_collect(void)
{
    uint8_t hw = ne1000_read_reg(NE1000_REG_ISR);
    uint8_t sh = __atomic_exchange_n(&ne1000_isr_shadow, 0, __ATOMIC_ACQ_REL);
    return (uint8_t)(hw | sh);
}

static void ne1000_apply_modes(void)
{
    ne1000_select_page(0);
    ne1000_write_reg(NE1000_REG_TCR, ne1000_tcr);
    ne1000_write_reg(NE1000_REG_RCR, ne1000_rcr);
}

static uint8_t ne1000_bnry_of(uint8_t next)
{
    return (uint8_t)(next == ne1000_rx_start ? ne1000_rx_stop - 1 : next - 1);
}

/* ------------------------------------------------------------------ */
/* Remote DMA (pamiec karty), tryb bajtowy (WTS = 0)                  */
/* ------------------------------------------------------------------ */

static int ne1000_dma_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_RDC);
    ne1000_write_reg(NE1000_REG_RBCR0, (uint8_t)(len & 0xFF));
    ne1000_write_reg(NE1000_REG_RBCR1, (uint8_t)(len >> 8));
    ne1000_write_reg(NE1000_REG_RSAR0, (uint8_t)(addr & 0xFF));
    ne1000_write_reg(NE1000_REG_RSAR1, (uint8_t)(addr >> 8));
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD0 | NE1000_CR_STA | NE1000_CR_PAGE0);

    for (i = 0; i < len; i++) {
        buf[i] = ne1000_inb((uint16_t)(ne1000_io_base + NE1000_REG_DATAPORT));
    }

    if (!ne1000_wait_isr_bit(NE1000_ISR_RDC, NE1000_POLL_LIMIT)) {
        ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STA | NE1000_CR_PAGE0);
        ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_RDC);
        return 0;
    }
    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_RDC);
    return 1;
}

static int ne1000_dma_write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_RDC);
    ne1000_write_reg(NE1000_REG_RBCR0, (uint8_t)(len & 0xFF));
    ne1000_write_reg(NE1000_REG_RBCR1, (uint8_t)(len >> 8));
    ne1000_write_reg(NE1000_REG_RSAR0, (uint8_t)(addr & 0xFF));
    ne1000_write_reg(NE1000_REG_RSAR1, (uint8_t)(addr >> 8));
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD1 | NE1000_CR_STA | NE1000_CR_PAGE0);

    for (i = 0; i < len; i++) {
        ne1000_outb((uint16_t)(ne1000_io_base + NE1000_REG_DATAPORT), buf[i]);
    }

    if (!ne1000_wait_isr_bit(NE1000_ISR_RDC, NE1000_POLL_LIMIT)) {
        ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STA | NE1000_CR_PAGE0);
        ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_RDC);
        return 0;
    }
    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_RDC);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Detekcja karty                                                     */
/* ------------------------------------------------------------------ */

static int ne1000_hw_reset(void)
{
    uint8_t v = ne1000_read_reg(NE1000_REG_RESET);
    ne1000_write_reg(NE1000_REG_RESET, v);
    return ne1000_wait_isr_bit(NE1000_ISR_RST, NE1000_POLL_LIMIT);
}

/* Zapis i odczyt wzorca w pamieci karty pod adresem addr. */
static int ne1000_mem_test(uint16_t addr)
{
    uint8_t pattern[64];
    uint8_t readback[64];
    uint16_t i;

    for (i = 0; i < 64; i++) pattern[i] = (uint8_t)(0xA5 ^ i);
    if (!ne1000_dma_write(addr, pattern, 64)) return 0;
    if (!ne1000_dma_read(addr, readback, 64)) return 0;
    for (i = 0; i < 64; i++) {
        if (readback[i] != pattern[i]) return 0;
    }
    return 1;
}

/* PROM NE2000 czytany bajtowo: kazdy bajt zdwojony. NE1000 ma bajty pojedyncze. */
static int ne1000_prom_doubled(const uint8_t *prom)
{
    uint32_t i;
    for (i = 0; i < 12; i += 2) {
        if (prom[i] != prom[i + 1]) return 0;
    }
    return 1;
}

/* Sprawdza obecnosc DS8390, rodzaj karty i uklad jej pamieci. */
static int ne1000_probe(uint16_t base)
{
    uint8_t prom[NE1000_PROM_SIZE];
    int doubled, ok = 0;

    ne1000_io_base = base;

    if (!ne1000_hw_reset()) return 0;

    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    ne1000_delay();
    /* Plywajaca szyna zwraca 0xFF - prawdziwy DS8390 oddaje to, co zapisano. */
    if ((ne1000_read_reg(NE1000_REG_CR) & (NE1000_CR_RD2 | NE1000_CR_TXP | NE1000_CR_STA | NE1000_CR_STP)) != (NE1000_CR_RD2 | NE1000_CR_STP))
        return 0;
    if (!(ne1000_read_reg(NE1000_REG_ISR) & NE1000_ISR_RST)) return 0;

    ne1000_write_reg(NE1000_REG_DCR, NE1000_DCR_VALUE);
    ne1000_write_reg(NE1000_REG_RBCR0, 0x00);
    ne1000_write_reg(NE1000_REG_RBCR1, 0x00);
    ne1000_write_reg(NE1000_REG_RCR, NE1000_RCR_MON);
    ne1000_write_reg(NE1000_REG_TCR, NE1000_TCR_LB0);
    ne1000_write_reg(NE1000_REG_IMR, 0x00);
    ne1000_write_reg(NE1000_REG_ISR, 0xFF);

    if (ne1000_dma_read(NE1000_PROM_ADDR, prom, NE1000_PROM_SIZE)) {
        doubled = ne1000_prom_doubled(prom);

        if (doubled && !NE1000_ACCEPT_DOUBLED_PROM) {
            ok = 0;                                    /* 16-bitowa NE2000 - nie ta karta */
        } else if (!doubled) {
            if (ne1000_mem_test((uint16_t)(NE1000_SM_TX_PAGE << 8))) {
                ne1000_set_layout(NE1000_SM_TX_PAGE, NE1000_SM_RX_START, NE1000_SM_RX_STOP);
                ok = 1;
            } else if (ne1000_mem_test((uint16_t)(NE1000_LG_TX_PAGE << 8))) {
                ne1000_set_layout(NE1000_LG_TX_PAGE, NE1000_LG_RX_START, NE1000_LG_RX_STOP);
                ok = 1;
            }
        } else if (ne1000_mem_test((uint16_t)(NE1000_LG_TX_PAGE << 8))) {
            ne1000_set_layout(NE1000_LG_TX_PAGE, NE1000_LG_RX_START, NE1000_LG_RX_STOP);
            ok = 1;
        }
    }

    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    ne1000_write_reg(NE1000_REG_ISR, 0xFF);
    return ok;
}

static int ne1000_isa_detect(void)
{
    uint32_t i;
    for (i = 0; i < (uint32_t)(sizeof(ne1000_isa_ports) / sizeof(ne1000_isa_ports[0])); i++) {
        if (ne1000_probe(ne1000_isa_ports[i])) return 1;
    }
    ne1000_io_base = 0;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Adres MAC                                                          */
/* ------------------------------------------------------------------ */

static int ne1000_mac_valid(const uint8_t *m)
{
    uint32_t i;
    int zero = 1, ones = 1;
    for (i = 0; i < 6; i++) {
        if (m[i] != 0x00) zero = 0;
        if (m[i] != 0xFF) ones = 0;
    }
    if (zero || ones) return 0;
    if (m[0] & 1U) return 0;
    return 1;
}

static int ne1000_read_mac(void)
{
    uint8_t prom[NE1000_PROM_SIZE];
    uint32_t i;

    /* PROM (adres 0): NE1000 ma bajty adresu pojedyncze, zdwojone oznaczaja NE2000. */
    if (ne1000_dma_read(NE1000_PROM_ADDR, prom, NE1000_PROM_SIZE)) {
        int doubled = ne1000_prom_doubled(prom);
        for (i = 0; i < 6; i++) ne1000_mac[i] = doubled ? prom[2 * i] : prom[i];
        if (ne1000_mac_valid(ne1000_mac)) return 1;
    }

    /* Zapas: PAR0..PAR5 (strona 1), np. gdy BIOS/PnP zaprogramowal adres. */
    ne1000_select_page(1);
    for (i = 0; i < 6; i++) ne1000_mac[i] = ne1000_read_reg((uint8_t)(NE1000_REG_PAR0 + i));
    ne1000_select_page(0);
    return ne1000_mac_valid(ne1000_mac);
}

/* ------------------------------------------------------------------ */
/* Inicjalizacja DS8390                                               */
/* ------------------------------------------------------------------ */

static int ne1000_hw_init(void)
{
    uint32_t i;

    if (!ne1000_hw_reset()) return 0;

    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    ne1000_write_reg(NE1000_REG_DCR, NE1000_DCR_VALUE);
    ne1000_write_reg(NE1000_REG_RBCR0, 0x00);
    ne1000_write_reg(NE1000_REG_RBCR1, 0x00);
    ne1000_write_reg(NE1000_REG_RCR, NE1000_RCR_MON);
    ne1000_write_reg(NE1000_REG_TCR, NE1000_TCR_LB0);
    ne1000_write_reg(NE1000_REG_IMR, 0x00);
    ne1000_write_reg(NE1000_REG_ISR, 0xFF);

    if (!ne1000_read_mac()) return 0;

    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    ne1000_delay();
    ne1000_write_reg(NE1000_REG_PSTART, ne1000_rx_start);
    ne1000_write_reg(NE1000_REG_PSTOP, ne1000_rx_stop);
    ne1000_write_reg(NE1000_REG_BNRY, ne1000_rx_start);
    ne1000_write_reg(NE1000_REG_TPSR, ne1000_tx_page);

    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE1);
    ne1000_delay();
    for (i = 0; i < 6; i++) ne1000_write_reg((uint8_t)(NE1000_REG_PAR0 + i), ne1000_mac[i]);
    ne1000_write_reg(NE1000_REG_CURR, (uint8_t)(ne1000_rx_start + 1));
    for (i = 0; i < 8; i++) ne1000_write_reg((uint8_t)(NE1000_REG_MAR0 + i), 0x00);
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    ne1000_delay();

    ne1000_next_packet = (uint8_t)(ne1000_rx_start + 1);

    ne1000_write_reg(NE1000_REG_ISR, 0xFF);
    ne1000_write_reg(NE1000_REG_IMR, 0x00);

    /* Start w trybie loopback, dopiero potem normalna praca. */
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STA | NE1000_CR_PAGE0);
    ne1000_delay();

    ne1000_tcr = 0x00;
    ne1000_rcr = NE1000_RCR_AB;
    ne1000_write_reg(NE1000_REG_TCR, ne1000_tcr);
    ne1000_write_reg(NE1000_REG_RCR, ne1000_rcr);

    ne1000_isr_shadow = 0;
    ne1000_mtu = 1500;
    ne1000_ready = 1;
    return 1;
}

/* ------------------------------------------------------------------ */
/* API sterownika                                                     */
/* ------------------------------------------------------------------ */

const char *ne1000_isa_driver_name(void)
{
    return "ne1000_isa";
}

int ne1000_isa_init(void)
{
    if (ne1000_ready) return 1;
    ne1000_spin_init(&ne1000_lock);

    if (!ne1000_isa_detect()) return 0;
    if (!ne1000_hw_init()) {
        ne1000_ready = 0;
        ne1000_io_base = 0;
        return 0;
    }
    return 1;
}

int ne1000_isa_is_ready(void)
{
    return ne1000_ready;
}

int ne1000_isa_get_mac(uint8_t mac[6])
{
    uint32_t i;
    if (!ne1000_ready || !mac) return 0;
    for (i = 0; i < 6; i++) mac[i] = ne1000_mac[i];
    return 1;
}

int ne1000_isa_send(const void *frame, uint16_t length)
{
    uint32_t t;
    uint16_t send_len;
    uint8_t isr = 0, tsr;
    const uint8_t *src = (const uint8_t *)frame;
    static uint8_t tx_buf[NE1000_MAX_FRAME + 2];
    unsigned long fl;

    if (!ne1000_ready || !frame || length == 0 || length > NE1000_MAX_FRAME) return -1;

    fl = ne1000_lock_irqsave();

    for (t = 0; t < NE1000_POLL_LIMIT; t++) {
        if (!(ne1000_read_reg(NE1000_REG_CR) & NE1000_CR_TXP)) break;
        __asm__ volatile("pause" ::: "memory");
    }
    if (t == NE1000_POLL_LIMIT) {
        ne1000_stats.tx_errors++;
        ne1000_stats.tx_timeouts++;
        ne1000_unlock_irqrestore(fl);
        return 0;
    }

    for (t = 0; t < length; t++) tx_buf[t] = src[t];
    send_len = length;
    while (send_len < NE1000_MIN_FRAME) tx_buf[send_len++] = 0x00;

    ne1000_select_page(0);
    if (!ne1000_dma_write((uint16_t)(ne1000_tx_page << 8), tx_buf, send_len)) {
        ne1000_stats.tx_errors++;
        ne1000_stats.dma_errors++;
        ne1000_unlock_irqrestore(fl);
        return 0;
    }

    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_PTX | NE1000_ISR_TXE);
    ne1000_write_reg(NE1000_REG_TPSR, ne1000_tx_page);
    ne1000_write_reg(NE1000_REG_TBCR0, (uint8_t)(send_len & 0xFF));
    ne1000_write_reg(NE1000_REG_TBCR1, (uint8_t)(send_len >> 8));
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_TXP | NE1000_CR_RD2 | NE1000_CR_STA | NE1000_CR_PAGE0);

    for (t = 0; t < NE1000_POLL_LIMIT; t++) {
        isr = ne1000_read_reg(NE1000_REG_ISR);
        if (isr & (NE1000_ISR_PTX | NE1000_ISR_TXE)) break;
        __asm__ volatile("pause" ::: "memory");
    }

    if (t == NE1000_POLL_LIMIT || (isr & NE1000_ISR_TXE)) {
        tsr = ne1000_read_reg(NE1000_REG_TSR);
        if (tsr & NE1000_TSR_ABT) ne1000_stats.tx_aborts++;
        if (tsr & NE1000_TSR_COL) ne1000_stats.tx_collisions++;
        if (tsr & NE1000_TSR_CRS) ne1000_stats.tx_carrier_lost++;
        if (tsr & NE1000_TSR_FU)  ne1000_stats.tx_fifo_underrun++;
        ne1000_stats.tx_errors++;
        if (t == NE1000_POLL_LIMIT) ne1000_stats.tx_timeouts++;
        ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_PTX | NE1000_ISR_TXE);
        ne1000_unlock_irqrestore(fl);
        return 0;
    }

    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_PTX);
    ne1000_stats.tx_packets++;
    ne1000_unlock_irqrestore(fl);
    return 1;
}

static void ne1000_rx_overrun_recover(void)
{
    uint32_t t;

    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    for (t = 0; t < 20000U; t++) {
        if (ne1000_read_reg(NE1000_REG_ISR) & NE1000_ISR_RST) break;
        __asm__ volatile("pause" ::: "memory");
    }
    ne1000_write_reg(NE1000_REG_RBCR0, 0x00);
    ne1000_write_reg(NE1000_REG_RBCR1, 0x00);
    ne1000_write_reg(NE1000_REG_TCR, NE1000_TCR_LB0);
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STA | NE1000_CR_PAGE0);
    ne1000_delay();
    ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_OVW);
    ne1000_write_reg(NE1000_REG_TCR, ne1000_tcr);

    ne1000_stats.rx_errors++;
    ne1000_stats.rx_overflow_errors++;
}

static void ne1000_rx_resync(uint8_t curr)
{
    ne1000_next_packet = curr;
    ne1000_write_reg(NE1000_REG_BNRY, ne1000_bnry_of(curr));
}

static int ne1000_receive_locked(void *frame, uint16_t capacity)
{
    uint8_t isr, curr, status, next, rx_hdr[4];
    uint16_t total, data_len;
    uint32_t addr, end;

    ne1000_select_page(0);
    isr = ne1000_isr_collect();

    if (isr & NE1000_ISR_OVW) ne1000_rx_overrun_recover();

    if (isr & NE1000_ISR_CNT) {
        ne1000_stats.rx_frame_errors   += ne1000_read_reg(NE1000_REG_CNTR0);
        ne1000_stats.rx_crc_errors     += ne1000_read_reg(NE1000_REG_CNTR1);
        ne1000_stats.rx_missed_packets += ne1000_read_reg(NE1000_REG_CNTR2);
        ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_CNT);
    }
    if (isr & (NE1000_ISR_PRX | NE1000_ISR_RXE)) ne1000_write_reg(NE1000_REG_ISR, NE1000_ISR_PRX | NE1000_ISR_RXE);

    ne1000_select_page(1);
    curr = ne1000_read_reg(NE1000_REG_CURR);
    ne1000_select_page(0);

    if (curr == ne1000_next_packet) return 0;

    addr = (uint32_t)ne1000_next_packet << 8;
    if (!ne1000_dma_read((uint16_t)addr, rx_hdr, 4)) {
        ne1000_stats.rx_errors++;
        ne1000_stats.dma_errors++;
        return 0;
    }
    status = rx_hdr[0];
    next   = rx_hdr[1];
    total  = (uint16_t)(rx_hdr[2] | ((uint16_t)rx_hdr[3] << 8));

    if (next < ne1000_rx_start || next >= ne1000_rx_stop ||
        total < 4 + 14 || total > (uint16_t)(NE1000_MAX_FRAME + 4 + 4)) {
        ne1000_stats.rx_errors++;
        ne1000_rx_resync(curr);
        return 0;
    }

    data_len = (uint16_t)(total - 4);

    if (!(status & NE1000_RSR_PRX) || data_len > capacity) {
        if (status & NE1000_RSR_CRC) ne1000_stats.rx_crc_errors++;
        if (status & NE1000_RSR_FAE) ne1000_stats.rx_frame_errors++;
        if (status & NE1000_RSR_FO)  ne1000_stats.rx_fifo_overrun++;
        if (status & NE1000_RSR_MPA) ne1000_stats.rx_missed_packets++;
        ne1000_stats.rx_errors++;
        ne1000_next_packet = next;
        ne1000_write_reg(NE1000_REG_BNRY, ne1000_bnry_of(next));
        return 0;
    }

    addr += 4;
    end = (uint32_t)ne1000_rx_stop << 8;
    if (addr + (uint32_t)data_len > end) {
        uint16_t first = (uint16_t)(end - addr);
        if (!ne1000_dma_read((uint16_t)addr, (uint8_t *)frame, first) ||
            !ne1000_dma_read((uint16_t)(ne1000_rx_start << 8), (uint8_t *)frame + first,
                             (uint16_t)(data_len - first))) {
            ne1000_stats.rx_errors++;
            ne1000_stats.dma_errors++;
            return 0;
        }
    } else if (!ne1000_dma_read((uint16_t)addr, (uint8_t *)frame, data_len)) {
        ne1000_stats.rx_errors++;
        ne1000_stats.dma_errors++;
        return 0;
    }

    ne1000_next_packet = next;
    ne1000_write_reg(NE1000_REG_BNRY, ne1000_bnry_of(next));
    ne1000_stats.rx_packets++;
    return (int)data_len;
}

int ne1000_isa_receive(void *frame, uint16_t capacity)
{
    unsigned long fl;
    int result;

    if (!ne1000_ready || !frame || capacity == 0) return -1;

    fl = ne1000_lock_irqsave();
    result = ne1000_receive_locked(frame, capacity);
    ne1000_unlock_irqrestore(fl);
    return result;
}

void ne1000_isa_shutdown(void)
{
    unsigned long fl;
    if (!ne1000_ready) return;
    fl = ne1000_lock_irqsave();
    ne1000_write_reg(NE1000_REG_CR, NE1000_CR_RD2 | NE1000_CR_STP | NE1000_CR_PAGE0);
    ne1000_write_reg(NE1000_REG_IMR, 0x00);
    ne1000_write_reg(NE1000_REG_ISR, 0xFF);
    ne1000_write_reg(NE1000_REG_RCR, NE1000_RCR_MON);
    ne1000_write_reg(NE1000_REG_TCR, NE1000_TCR_LB0);
    ne1000_ready = 0;
    ne1000_irq_enabled = 0;
    ne1000_unlock_irqrestore(fl);
}

int ne1000_isa_loopback_test(void)
{
    uint8_t test_frame[64];
    uint8_t rx_frame[NE1000_MAX_FRAME];
    uint32_t i, t;
    uint8_t old_rcr, old_tcr;
    unsigned long fl;
    int result = 0;

    if (!ne1000_ready) return 0;
    for (i = 0; i < 64; i++) test_frame[i] = (uint8_t)i;

    fl = ne1000_lock_irqsave();
    old_rcr = ne1000_rcr;
    old_tcr = ne1000_tcr;
    ne1000_tcr = NE1000_TCR_LB0;
    ne1000_rcr = NE1000_RCR_AB | NE1000_RCR_AM | NE1000_RCR_PRO;
    ne1000_apply_modes();
    ne1000_write_reg(NE1000_REG_ISR, 0xFF);
    ne1000_unlock_irqrestore(fl);

    if (ne1000_isa_send(test_frame, 64) > 0) {
        for (t = 0; t < 20000U && !result; t++) {
            int len = ne1000_isa_receive(rx_frame, sizeof(rx_frame));
            if (len >= 64) {
                int match = 1;
                for (i = 0; i < 64; i++) {
                    if (rx_frame[i] != test_frame[i]) { match = 0; break; }
                }
                if (match) result = 1;
            }
        }
    }

    fl = ne1000_lock_irqsave();
    ne1000_tcr = old_tcr;
    ne1000_rcr = old_rcr;
    ne1000_apply_modes();
    ne1000_unlock_irqrestore(fl);
    return result;
}

void ne1000_isa_stats_reset(void)
{
    uint8_t *p = (uint8_t *)&ne1000_stats;
    uint32_t i;
    unsigned long fl = ne1000_lock_irqsave();
    for (i = 0; i < sizeof(ne1000_stats); i++) p[i] = 0;
    ne1000_unlock_irqrestore(fl);
}

int ne1000_isa_stats_get(struct ne1000_isa_stats *out)
{
    unsigned long fl;
    if (!out) return 0;
    fl = ne1000_lock_irqsave();
    *out = ne1000_stats;
    ne1000_unlock_irqrestore(fl);
    return 1;
}

int ne1000_isa_set_multicast(const uint8_t *addr)
{
    uint32_t i;
    unsigned long fl;
    if (!ne1000_ready) return 0;
    fl = ne1000_lock_irqsave();
    ne1000_select_page(1);
    for (i = 0; i < 8; i++) ne1000_write_reg((uint8_t)(NE1000_REG_MAR0 + i), addr ? addr[i] : 0xFF);
    ne1000_select_page(0);
    ne1000_rcr |= NE1000_RCR_AM;
    ne1000_apply_modes();
    ne1000_unlock_irqrestore(fl);
    return 1;
}

int ne1000_isa_set_promiscuous(int enable)
{
    unsigned long fl;
    if (!ne1000_ready) return 0;
    fl = ne1000_lock_irqsave();
    if (enable) ne1000_rcr |= (NE1000_RCR_PRO | NE1000_RCR_AM);
    else        ne1000_rcr &= (uint8_t)~NE1000_RCR_PRO;
    ne1000_apply_modes();
    ne1000_unlock_irqrestore(fl);
    return 1;
}

int ne1000_isa_set_mtu(uint16_t mtu)
{
    if (!ne1000_ready || mtu < 68U || mtu > 1500U) return 0;
    ne1000_mtu = mtu;
    return 1;
}

uint16_t ne1000_isa_get_mtu(void)
{
    return ne1000_mtu;
}

/* DS8390 nie ma rejestru statusu linku (10BASE2/AUI) - karta zainicjalizowana = "link up". */
int ne1000_isa_link_up(void)
{
    return ne1000_ready ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Obsluga przerwan                                                   */
/* ------------------------------------------------------------------ */

extern void ne1000_isa_irq_stub(void);

#if defined(__i386__)
__asm__(
    ".text\n"
    ".globl ne1000_isa_irq_stub\n"
    "ne1000_isa_irq_stub:\n"
    "    pusha\n"
    "    cld\n"
    "    call ne1000_isa_irq_handler\n"
    "    popa\n"
    "    iret\n"
);
#endif

int ne1000_isa_irq_enable(uint8_t irq)
{
    unsigned long fl;
    if (!ne1000_ready || irq >= 16) return 0;

    fl = ne1000_lock_irqsave();
    ne1000_irq_num = irq;
    idt_set_gate((uint8_t)(0x20 + irq), (uint32_t)ne1000_isa_irq_stub,
                 GDT_RING0_CODE, 0x8E);
    pik_enable_irq(irq);
    ne1000_irq_enabled = 1;
    ne1000_select_page(0);
    ne1000_write_reg(NE1000_REG_ISR, 0xFF);
    ne1000_write_reg(NE1000_REG_IMR, NE1000_IRQ_MASK);
    ne1000_unlock_irqrestore(fl);
    return 1;
}

void ne1000_isa_irq_disable(void)
{
    unsigned long fl;
    if (!ne1000_ready) return;
    fl = ne1000_lock_irqsave();
    ne1000_select_page(0);
    ne1000_write_reg(NE1000_REG_IMR, 0x00);
    ne1000_write_reg(NE1000_REG_ISR, NE1000_IRQ_MASK);
    ne1000_irq_enabled = 0;
    ne1000_isr_shadow = 0;
    ne1000_unlock_irqrestore(fl);
}

int ne1000_isa_irq_handler(void)
{
    uint8_t isr;
    int handled = 0;

    if (ne1000_ready && ne1000_irq_enabled) {
        ne1000_spin_lock(&ne1000_lock);
        isr = (uint8_t)(ne1000_read_reg(NE1000_REG_ISR) & NE1000_IRQ_MASK);
        if (isr) {
            ne1000_write_reg(NE1000_REG_ISR, isr);
            __atomic_fetch_or(&ne1000_isr_shadow, isr, __ATOMIC_ACQ_REL);
            handled = 1;
        }
        ne1000_spin_unlock(&ne1000_lock);
    }

    /* EOI zawsze - inaczej PIC blokuje linie. */
    if (ne1000_irq_num >= 8) ne1000_outb(0xA0, 0x20);
    ne1000_outb(0x20, 0x20);
    return handled;
}

/* ------------------------------------------------------------------ */
/* Debug: odczyt rejestrow                                            */
/* ------------------------------------------------------------------ */

void ne1000_isa_register_dump(void)
{
    uint8_t cr, isr, bnry, curr;
    unsigned long fl;
    if (!ne1000_ready) return;
    fl = ne1000_lock_irqsave();
    ne1000_select_page(0);
    cr   = ne1000_read_reg(NE1000_REG_CR);
    isr  = ne1000_read_reg(NE1000_REG_ISR);
    bnry = ne1000_read_reg(NE1000_REG_BNRY);
    ne1000_select_page(1);
    curr = ne1000_read_reg(NE1000_REG_CURR);
    ne1000_select_page(0);
    ne1000_unlock_irqrestore(fl);
}