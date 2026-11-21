/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Sterownik 3Com EtherLink III (3C509 / 3C509B) - karta sieciowa ISA 10 Mbps.
 * Rejestry i sekwencja inicjalizacji oparte na:
 *   - "EtherLink III Parallel Tasking ISA, EISA, Micro Channel, and PCMCIA
 *     Adapter Drivers Technical Reference" (3Com),
 *   - sterowniku Linux drivers/net/ethernet/3com/3c509.c (Donald Becker),
 *   - naglowku NetBSD sys/dev/ic/elink3reg.h (Herb Peyerl).
 *
 * Karta nie jest urzadzeniem PCI; jej adres I/O i IRQ sa przechowywane w
 * EEPROM i aktywowane za pomoca dokumentowanej sekwencji selekcji ISA (ID
 * port + sekwencja ID). Sterownik najpierw probuje odczytac konfiguracje z
 * EEPROM przez port ID, a dopiero w ostatecznosci sonduje standardowe adresy
 * I/O weryfikujac sygnator producenta (0x6d50 = 3Com). Dziala w trybie
 * pollingu (brak przerwan), zgodnie ze stosem sieciowym net.c.
 */

#include <drivers/network/3c509.h>

#define EL3_DATA      0x00   /* port danych (wszystkie okna) */
#define EL3_CMD       0x0e   /* rejestr polecen (zapis) / status (odczyt) */
#define EL3_STATUS    0x0e
#define EEPROM_READ   0x80   /* bit polecenia odczytu EEPROM */
#define EEPROM_BUSY   0x8000 /* bit zajetosci EEPROM w W0_EEPROM_CMD */

#define EL3_IO_EXTENT 16

/* Polecenia rejestru EL3_CMD: bity 15-11 kod polecenia, 10-0 argument. */
#define CMD_TOTAL_RESET     ((uint16_t)(0 << 11))
#define CMD_SELECT_WINDOW   ((uint16_t)(1 << 11))
#define CMD_START_COAX      ((uint16_t)(2 << 11))
#define CMD_RX_DISABLE      ((uint16_t)(3 << 11))
#define CMD_RX_ENABLE       ((uint16_t)(4 << 11))
#define CMD_RX_RESET        ((uint16_t)(5 << 11))
#define CMD_RX_DISCARD      ((uint16_t)(8 << 11))
#define CMD_TX_ENABLE       ((uint16_t)(9 << 11))
#define CMD_TX_DISABLE      ((uint16_t)(10 << 11))
#define CMD_TX_RESET        ((uint16_t)(11 << 11))
#define CMD_ACK_INTR        ((uint16_t)(13 << 11))
#define CMD_SET_INTR_ENB    ((uint16_t)(14 << 11))
#define CMD_SET_STATUS_ENB  ((uint16_t)(15 << 11))
#define CMD_SET_RX_FILTER   ((uint16_t)(16 << 11))
#define CMD_STATS_ENABLE    ((uint16_t)(21 << 11))
#define CMD_STATS_DISABLE   ((uint16_t)(22 << 11))

/* Bity rejestru statusu (EL3_STATUS, wszystkie okna). */
#define ST_INT_LATCH    ((uint16_t)0x0001)
#define ST_ADAPTER_FAIL ((uint16_t)0x0002)
#define ST_TX_COMPLETE  ((uint16_t)0x0004)   /* w 3C509: blad nadawania */
#define ST_TX_AVAILABLE ((uint16_t)0x0008)
#define ST_RX_COMPLETE  ((uint16_t)0x0010)
#define ST_RX_EARLY     ((uint16_t)0x0020)
#define ST_INT_REQ      ((uint16_t)0x0040)
#define ST_STATS_FULL   ((uint16_t)0x0080)
#define ST_CMD_BUSY     ((uint16_t)0x1000)

#define RF_STATION   ((uint16_t)0x0001)
#define RF_MULTICAST ((uint16_t)0x0002)
#define RF_BROADCAST ((uint16_t)0x0004)
#define RF_PROM      ((uint16_t)0x0008)

/* Okno 1 - zestaw operacyjny (offsety od bazy I/O). */
#define W1_TX_FIFO   0x00
#define W1_RX_FIFO   0x00
#define W1_RX_STATUS 0x08
#define W1_TX_STATUS 0x0B
#define W1_TX_FREE   0x0C

/* Okno 0 - konfiguracja. */
#define W0_MFG_ID       0x00
#define W0_PRODUCT_ID   0x02
#define W0_CONF_CTRL    0x04   /* zapis 0x0001 = wlacz adapter */
#define W0_ADDR_CONF    0x06
#define W0_IRQ          0x08   /* IRQ w bitach 12-15 */
#define W0_EEPROM_CMD   0x0A
#define W0_EEPROM_DATA  0x0C

/* Okno 2 - adres stacji (MAC). */
#define W2_ADDR_0 0x00

/* Okno 4 - diagnostyka / medium. */
#define W4_MEDIA  0x0A
#define MEDIA_TP  0x00C0   /* link beat + jabber guard dla 10baseT */

/* Okno 6 - statystyki. */
#define W6_TX_COLLISIONS 0x04
#define W6_TX_FRAMES_OK  0x06
#define W6_RX_FRAMES_OK  0x07
#define W6_TX_TOTAL_OK   0x0C
#define W6_RX_TOTAL_OK   0x0A

/* Indeksy slow EEPROM. */
#define EEP_NODE0  0x00
#define EEP_NODE1  0x01
#define EEP_NODE2  0x02
#define EEP_MFG_ID 0x07
#define EEP_ADDR   0x08
#define EEP_IRQ    0x09

#define MFG_ID_3COM 0x6d50  /* sygnator producenta 3Com w EEPROM */

#define MAX_FRAME 1518
#define MIN_FRAME 60

static uint16_t io_addr;
static uint8_t device_mac[6];
static uint8_t ready;
static uint8_t irq_line;
static uint8_t if_port;     /* 0 = 10baseT, 1 = AUI, 3 = BNC */
static struct c509_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_collisions;
    uint32_t tx_aborted_errors;
    uint32_t rx_crc_errors;
    uint32_t rx_frame_errors;
    uint32_t rx_over_errors;
    uint32_t rx_length_errors;
} stats;
static uint8_t tx_frame[MAX_FRAME + 8];

/* UWAGA: kolejnosc argumentow jak w Linuksie: (wartosc, port). */
static void outb(uint8_t value, uint16_t port)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void outw(uint16_t value, uint16_t port)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void c509_io_delay(void)
{
    inb(0x84);
    inb(0x84);
}

static void c509_delay(uint32_t n)
{
    uint32_t t;
    for (t = 0; t < n; t++) inb(0x84);   /* ok. 1 us na iteracje */
}

static void c509_eeprom_delay(void)
{
    c509_delay(2000);   /* karta potrzebuje >= 162 us */
}

/* Czeka az karta skonczy wykonywac polecenie. */
static void c509_wait_cmd(uint16_t addr)
{
    uint32_t t;
    for (t = 0; t < 100000UL; t++) {
        if (!(inw(addr + EL3_STATUS) & ST_CMD_BUSY)) return;
        inb(0x84);
    }
}

/* ------------------------------------------------------------------ */
/* Wyszukiwanie portu ID (sekwencja selekcji ISA, dokumentacja 3Com). */
/* ------------------------------------------------------------------ */

static uint16_t id_port;

/* Znajduje port I/O w zakresie 0x110-0x1F0 nadajacy sie jako port ID. */
static int id_port_discover(void)
{
    uint16_t port;
    for (port = 0x110; port < 0x200; port += 0x10) {
        outb(0x00, port);
        outb(0xff, port);
        if ((inb(port) & 0x01) != 0) {
            id_port = port;
            return 1;
        }
    }
    return 0;
}

/* Odczytuje slowo z EEPROM karty w trybie selekcji ID. */
static uint16_t id_read_eeprom(uint8_t index)
{
    int bit;
    uint16_t word = 0;
    outb((uint8_t)(EEPROM_READ + index), id_port);
    c509_eeprom_delay();
    for (bit = 15; bit >= 0; bit--) {
        word = (uint16_t)((word << 1) + (inb(id_port) & 0x01));
    }
    return word;
}

/* Odczytuje slowo z EEPROM karty aktywnej pod adresem addr (okno 0). */
static uint16_t read_eeprom(uint16_t addr, uint8_t index)
{
    uint32_t t;
    outw((uint16_t)(EEPROM_READ + index), addr + W0_EEPROM_CMD);
    c509_eeprom_delay();
    for (t = 0; t < 100000UL; t++) {
        if (!(inw(addr + W0_EEPROM_CMD) & EEPROM_BUSY)) break;
        inb(0x84);
    }
    return inw(addr + W0_EEPROM_DATA);
}

/* Sekwencja ID 3Com: wprowadza karty w stan selekcji i kasuje tagi. */
static void c509_id_sequence(void)
{
    short lrs_state = 0xff;
    int i;
    outb(0x00, id_port);
    outb(0x00, id_port);
    for (i = 0; i < 255; i++) {
        outb((uint8_t)lrs_state, id_port);
        lrs_state <<= 1;
        if (lrs_state & 0x100) lrs_state ^= 0xcf;
    }
    outb(0xd0, id_port);  /* wyczysc tagi wszystkich kart */
}

static void store_mac(uint16_t w0, uint16_t w1, uint16_t w2)
{
    device_mac[0] = (uint8_t)(w0 >> 8);
    device_mac[1] = (uint8_t)(w0 & 0xFF);
    device_mac[2] = (uint8_t)(w1 >> 8);
    device_mac[3] = (uint8_t)(w1 & 0xFF);
    device_mac[4] = (uint8_t)(w2 >> 8);
    device_mac[5] = (uint8_t)(w2 & 0xFF);
}

/* Sondowanie standardowych adresow I/O (ostatnia instancja; dziala tylko
 * jesli karta byla juz aktywowana, np. przez BIOS lub program konfiguracyjny).
 * Kazdy adres weryfikowany jest sygnatorem producenta w oknie 0. */
static int probe_standard_addresses(void)
{
    static const uint16_t addrs[] = {0x300, 0x320, 0x340, 0x360, 0x280, 0x2e0, 0x380, 0x3c0};
    uint32_t i;
    for (i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
        uint16_t a = addrs[i];
        if (inw(a + EL3_STATUS) == 0xFFFF) continue;  /* nic pod adresem */
        outw(CMD_SELECT_WINDOW | 0, a + EL3_CMD);
        c509_io_delay();
        if (inw(a + W0_MFG_ID) != MFG_ID_3COM) continue;
        io_addr = a;
        store_mac(read_eeprom(a, EEP_NODE0),
                  read_eeprom(a, EEP_NODE1),
                  read_eeprom(a, EEP_NODE2));
        if_port = (uint8_t)(read_eeprom(a, EEP_ADDR) >> 14);
        irq_line = (uint8_t)(read_eeprom(a, EEP_IRQ) >> 12);
        if (irq_line == 0) irq_line = 3;
        return 1;
    }
    return 0;
}

/* Lokalizuje karte i aktywuje ja pod adresem z EEPROM.
 * Zwraca 1 gdy karta znaleziona (io_addr, device_mac, irq_line, if_port
 * ustawione), 0 gdy brak karty. */
static int locate_card(void)
{
    /* 3C509B w trybie PnP: przejscie do stanu WaitForKey, inaczej karta
     * nie reaguje na sekwencje ID. */
    outb(0x02, 0x279);
    outb(0x02, 0xa79);

    if (!id_port_discover()) return probe_standard_addresses();

    c509_id_sequence();
    if (id_read_eeprom(EEP_MFG_ID) != MFG_ID_3COM) {
        return probe_standard_addresses();
    }

    {
        uint16_t w0 = id_read_eeprom(EEP_NODE0);
        uint16_t w1 = id_read_eeprom(EEP_NODE1);
        uint16_t w2 = id_read_eeprom(EEP_NODE2);
        store_mac(w0, w1, w2);
    }

    {
        uint16_t cfg = id_read_eeprom(EEP_ADDR);
        io_addr = (uint16_t)(0x200 + ((cfg & 0x1fU) << 4));
        if_port = (uint8_t)(cfg >> 14);
        irq_line = (uint8_t)(id_read_eeprom(EEP_IRQ) >> 12);
        if (irq_line == 0) irq_line = 3;
    }

    /* Tag karty (kolejna karta moze byc odczytana pozniej). */
    outb(0xd1, id_port);

    /* Aktywacja karty pod adresem z EEPROM (wyjscie z trybu selekcji ID). */
    outb((uint8_t)((io_addr >> 4) | 0xe0), id_port);
    c509_io_delay();

    /* Potwierdzenie ze karta odpowiada pod swoim adresem. */
    outw(CMD_SELECT_WINDOW | 0, io_addr + EL3_CMD);
    c509_io_delay();
    if (inw(io_addr + W0_MFG_ID) != MFG_ID_3COM) {
        return probe_standard_addresses();
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Inicjalizacja sprzetu po aktywacji adresu.                       */
/* ------------------------------------------------------------------ */

static int hardware_configure(void)
{
    uint32_t i;

    outw(CMD_SELECT_WINDOW | 0, io_addr + EL3_CMD);
    if (inw(io_addr + W0_MFG_ID) != MFG_ID_3COM) return 0;

    /* Wlaczenie adaptera (wymagane!). */
    outw(0x0001, io_addr + W0_CONF_CTRL);

    /* Selekcja linii IRQ (bity 12-15 rejestru W0_IRQ). */
    outw((uint16_t)(((uint16_t)irq_line << 12) | 0x0f00U), io_addr + W0_IRQ);

    /* Adres MAC do okna 2 (adres stacji). */
    outw(CMD_SELECT_WINDOW | 2, io_addr + EL3_CMD);
    for (i = 0; i < 6; i++) {
        outb(device_mac[i], (uint16_t)(io_addr + W2_ADDR_0 + i));
    }

    /* Medium: BNC -> StartCoax, 10baseT -> link beat w oknie 4. */
    if ((if_port & 3) == 3) {
        outw(CMD_START_COAX, io_addr + EL3_CMD);
        c509_delay(800);
    } else if ((if_port & 3) == 0) {
        outw(CMD_SELECT_WINDOW | 4, io_addr + EL3_CMD);
        outw((uint16_t)(inw(io_addr + W4_MEDIA) | MEDIA_TP), io_addr + W4_MEDIA);
    }

    /* Statystyki wylaczone (w pollingu nie czytamy ich z okna 6). */
    outw(CMD_STATS_DISABLE, io_addr + EL3_CMD);

    /* Okno 1 - zestaw operacyjny. */
    outw(CMD_SELECT_WINDOW | 1, io_addr + EL3_CMD);

    outw(CMD_TX_RESET, io_addr + EL3_CMD);
    c509_wait_cmd(io_addr);
    outw(CMD_RX_RESET, io_addr + EL3_CMD);
    c509_wait_cmd(io_addr);

    /* Filtr odbioru: wlasny adres + rozgloszeniowy. */
    outw(CMD_SET_RX_FILTER | RF_STATION | RF_BROADCAST, io_addr + EL3_CMD);

    /* Nadajnik i odbiornik wlaczone. */
    outw(CMD_TX_ENABLE, io_addr + EL3_CMD);
    outw(CMD_RX_ENABLE, io_addr + EL3_CMD);

    /* Widoczne bity statusu. */
    outw(CMD_SET_STATUS_ENB | 0xFF, io_addr + EL3_CMD);

    /* Potwierdzenie oczekujacych zdarzen. */
    outw(CMD_ACK_INTR | ST_INT_LATCH | ST_TX_AVAILABLE | ST_RX_EARLY | ST_INT_REQ,
         io_addr + EL3_CMD);

    /* Polling: zadnych przerwan (flagi nadal widac w rejestrze statusu). */
    outw(CMD_SET_INTR_ENB | 0x0000, io_addr + EL3_CMD);

    c509_wait_cmd(io_addr);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                            */
/* ------------------------------------------------------------------ */

int c509_init(void)
{
    if (ready) return 1;
    if (!locate_card()) return 0;
    if (!hardware_configure()) return 0;
    ready = 1;
    return 1;
}

int c509_is_ready(void)
{
    return ready;
}

int c509_get_mac(uint8_t mac[6])
{
    uint32_t i;
    if (!ready || !mac) return 0;
    for (i = 0; i < 6; i++) mac[i] = device_mac[i];
    return 1;
}

int c509_send(const void *frame, uint16_t length)
{
    uint32_t t, i;
    uint32_t send_len, padded;
    uint16_t needed;
    const uint8_t *src = (const uint8_t *)frame;

    if (!ready || !frame || length == 0 || length > MAX_FRAME) return -1;

    send_len = length < MIN_FRAME ? MIN_FRAME : length;
    padded = (send_len + 3U) & ~3U;               /* dane do granicy 4 bajtow */
    needed = (uint16_t)(4 + padded);              /* 4 bajty naglowka + dane */

    /* Czekaj az karta nie jest zajeta i TX FIFO ma miejsce. */
    for (t = 0; t < 3000000UL; t++) {
        if (!(inw(io_addr + EL3_STATUS) & ST_CMD_BUSY) &&
            inw(io_addr + W1_TX_FREE) >= needed) break;
        __asm__ volatile("pause" ::: "memory");
    }
    if (t == 3000000UL) {
        stats.tx_errors++;
        return 0;
    }

    for (i = 0; i < length; i++) tx_frame[i] = src[i];
    for (; i < padded; i++) tx_frame[i] = 0;

    /* Naglowek: [dlugosc][0x0000], potem dane. */
    outw((uint16_t)send_len, io_addr + W1_TX_FIFO);
    outw(0x0000, io_addr + W1_TX_FIFO);
    for (i = 0; i < padded; i += 2) {
        uint16_t w = (uint16_t)(tx_frame[i] | (tx_frame[i + 1] << 8));
        outw(w, io_addr + W1_TX_FIFO);
    }

    /* W 3C509 TX_COMPLETE oznacza blad nadawania - sukces nie wymaga
     * czekania, karta wysyla ramke sama. Sprawdzamy tylko stos statusow. */
    if (inw(io_addr + EL3_STATUS) & ST_TX_COMPLETE) {
        int failed = 0;
        uint32_t k;
        for (k = 0; k < 4; k++) {
            uint8_t txs = inb(io_addr + W1_TX_STATUS);
            if (txs == 0) break;
            if (txs & 0x38U) { stats.tx_aborted_errors++; failed = 1; }
            if (txs & 0x30U) {
                outw(CMD_TX_RESET, io_addr + EL3_CMD);
                c509_wait_cmd(io_addr);
            }
            if (txs & 0x3CU) outw(CMD_TX_ENABLE, io_addr + EL3_CMD);
            outb(0x00, io_addr + W1_TX_STATUS);  /* zdjecie ze stosu */
        }
        if (failed) {
            stats.tx_errors++;
            return 0;
        }
    }

    stats.tx_packets++;
    return 1;
}

static void rx_discard(void)
{
    outw(CMD_RX_DISCARD, io_addr + EL3_CMD);
    c509_wait_cmd(io_addr);
}

int c509_receive(void *frame, uint16_t capacity)
{
    uint16_t rx_status;
    uint16_t pkt_len;
    uint32_t i;
    uint8_t *dst = (uint8_t *)frame;

    if (!ready || !frame || capacity == 0) return -1;

    rx_status = inw(io_addr + W1_RX_STATUS);

    if (rx_status & 0x8000U) return 0;  /* niekompletny / FIFO pusty */
    if (rx_status == 0x0000) return 0;

    if (rx_status & 0x4000U) {          /* blad w pakiecie */
        rx_discard();
        stats.rx_errors++;
        switch (rx_status & 0x3800U) {
        case 0x0000: stats.rx_over_errors++;   break;
        case 0x0800: stats.rx_length_errors++; break;
        case 0x1000: stats.rx_frame_errors++;  break;
        case 0x1800: stats.rx_length_errors++; break;
        case 0x2000: stats.rx_frame_errors++;  break;
        case 0x2800: stats.rx_crc_errors++;    break;
        default: break;
        }
        return 0;
    }

    pkt_len = (uint16_t)(rx_status & 0x07FFU);
    if (pkt_len == 0 || pkt_len > capacity) {
        rx_discard();
        stats.rx_errors++;
        return 0;
    }

    /* Odczyt danych z RX FIFO jako slow. */
    for (i = 0; i < pkt_len; i += 2) {
        uint16_t w = inw(io_addr + W1_RX_FIFO);
        dst[i] = (uint8_t)(w & 0xFF);
        if (i + 1 < pkt_len) dst[i + 1] = (uint8_t)(w >> 8);
    }

    /* Zdjecie pakietu ze szczytu kolejki odbiorczej. */
    rx_discard();
    stats.rx_packets++;
    return (int)pkt_len;
}

void c509_shutdown(void)
{
    if (!ready) return;
    outw(CMD_STATS_DISABLE, io_addr + EL3_CMD);
    outw(CMD_RX_DISABLE, io_addr + EL3_CMD);
    outw(CMD_TX_DISABLE, io_addr + EL3_CMD);
    outw(CMD_SET_INTR_ENB | 0x0000, io_addr + EL3_CMD);
    ready = 0;
}

/* UWAGA: 3C509 nie odbiera wlasnych ramek, wiec ten test przejdzie tylko
 * gdy ktos na sieci odesle identyczna ramke. Do diagnostyki lepsze jest
 * sprawdzenie, czy c509_send() zwraca 1 i czy c509_receive() widzi ruch. */
int c509_loopback_test(void)
{
    uint8_t test_frame[64];
    uint8_t rx_frame[256];
    uint32_t i;
    int rc;

    if (!ready) return 0;
    for (i = 0; i < 64; i++) test_frame[i] = (uint8_t)i;

    rc = c509_send(test_frame, 64);
    if (rc <= 0) return 0;

    for (i = 0; i < 2000000UL; i++) {
        int len = c509_receive(rx_frame, sizeof(rx_frame));
        if (len >= 64) {
            int match = 1;
            for (uint32_t j = 0; j < 64; j++) {
                if (rx_frame[j] != test_frame[j]) { match = 0; break; }
            }
            return match;
        }
        __asm__ volatile("pause" ::: "memory");
    }
    return 0;
}

void c509_stats_reset(void)
{
    uint8_t *p = (uint8_t *)&stats;
    uint32_t i;
    for (i = 0; i < sizeof(stats); i++) p[i] = 0;
}

int c509_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                   uint32_t *tx_errors, uint32_t *rx_errors)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = stats.tx_packets;
    *rx_packets = stats.rx_packets;
    *tx_errors = stats.tx_errors;
    *rx_errors = stats.rx_errors;
    return 1;
}

int c509_set_multicast(const uint8_t *addr)
{
    (void)addr;
    if (!ready) return 0;
    outw(CMD_SET_RX_FILTER | RF_STATION | RF_MULTICAST | RF_BROADCAST,
         io_addr + EL3_CMD);
    return 1;
}