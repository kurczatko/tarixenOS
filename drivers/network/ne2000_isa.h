#ifndef NE2000_ISA_H
#define NE2000_ISA_H

#include <include/printf/types.h>

/*
 * Sterownik sieciowy NE2000 (ISA) oparty na National Semiconductor DS8390
 * (NIC - Network Interface Controller).
 *
 * Karta NE2000 to karta ISA (16-bit) posiadajaca 16 KB pamieci buforowej
 * podzielonej na strony po 256 bajtow oraz rejestry DS8390 odmapowane w
 * przestrzeni I/O pod adresem bazowym (domyslnie 0x300, alternatywnie
 * 0x320 / 0x340 / 0x360).
 *
 * Interfejs jest zgodny ze stosem sieciowym net.c (net_send_fn /
 * net_receive_fn) i automatycznie dziala z przegladarka internetowa.
 */

/* Nazwa sterownika (uzywana przez driver manager i logi). */
extern const char *ne2000_isa_driver_name(void);

/* Inicjalizacja: probuje wykryc karte NE2000 na portach ISA.
 * Zwraca 1 przy sukcesie, 0 gdy karta nie zostala wykryta. */
int ne2000_isa_init(void);
int ne2000_isa_is_ready(void);

/* Adres MAC odczytany z rejestrow PAR0..PAR5 strony 1 DS8390. */
int ne2000_isa_get_mac(uint8_t mac[6]);

/* Wyslanie ramki Ethernet. Zwraca 1 (sukces), 0 (blad sprzetowy), -1 (zly argument). */
int ne2000_isa_send(const void *frame, uint16_t length);

/* Odbicie jednej ramki. Zwraca dlugosc ramki (>0), 0 (brak ramki), -1 (blad). */
int ne2000_isa_receive(void *frame, uint16_t capacity);

/* Zatrzymanie karty i zwolnienie zasobow. */
void ne2000_isa_shutdown(void);

/* Test wlaczenia (loopback): wysyla i odbiera ramke probna. Zwraca 1 gdy OK. */
int ne2000_isa_loopback_test(void);

/* Statystyki. */
struct ne2000_isa_stats {
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
    uint32_t rx_overflow_errors;
    uint32_t tx_timeouts;
    uint32_t dma_errors;
};

void ne2000_isa_stats_reset(void);
int ne2000_isa_stats_get(struct ne2000_isa_stats *out);

/* Filtry odbioru. */
int ne2000_isa_set_multicast(const uint8_t *addr);
int ne2000_isa_set_promiscuous(int enable);

/* Obsluga przerwan (opcjonalna; domyslnie tryb pollingowy).
 * ne2000_isa_irq_handler jest wywoływana z procedury przerwania IDT. */
int ne2000_isa_irq_enable(uint8_t irq);
void ne2000_isa_irq_disable(void);
int ne2000_isa_irq_handler(void);

/* Pomocnicze: wypisanie rejestrow DS8390 (debug). */
void ne2000_isa_register_dump(void);

#endif /* NE2000_ISA_H */
