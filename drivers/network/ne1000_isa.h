#ifndef NE1000_ISA_H
#define NE1000_ISA_H

#include <include/printf/types.h>

/*
 * Sterownik sieciowej karty NE1000 (ISA, 8-bit) oparty na National
 * Semiconductor DS8390 (Network Interface Controller).
 *
 * NE1000 to wersja 8-bitowa NE2000. Oba uzywajia tego samego ukladu DS8390,
 * rejestrow i mapy pamieci buforowej (16 KB, strony po 256 bajtow). Roznica
 * polega na szynie danych: NE1000 posiada 8-bitowa szowe danych do szyny ISA,
 * wiec DCR.WTS = 0 (transfery bajtowe) i wszystkie operacje na porcie
 * danych (DATAPORT) sa wykonywane bajtowo. Porty I/O: 0x300, 0x320, 0x340,
 * 0x360 (takie same jak NE2000). IRQ: 2/9, 3, 4 lub 5 (zworka na karcie).
 *
 * Implementacja opiera sie na dokumentacji DS8390 (National Semiconductor):
 * rejestry strony 0 i 1, Remote DMA, pierscien odbiorczy, bufor nadawczy,
 * odzyskiwanie po przepelnieniu RX, filtry multicast (MAR0-7) i tryb
 * promiskuiczny.
 *
 * Interfejs jest zgodny ze stosem sieciowym net.c (net_send_fn /
 * net_receive_fn) i automatycznie dziala z przegladarka internetowa.
 */

/* Nazwa sterownika (uzywana przez driver manager i logi). */
extern const char *ne1000_isa_driver_name(void);

/* Inicjalizacja: probuje wykryc karte NE1000 na portach ISA.
 * Zwraca 1 przy sukcesie, 0 gdy karta nie zostala wykryta. */
int ne1000_isa_init(void);
int ne1000_isa_is_ready(void);

/* Adres MAC odczytany z PROM karty (bufory 0x00..0x05) z zapasem PAR0..5. */
int ne1000_isa_get_mac(uint8_t mac[6]);

/* Wyslanie ramki Ethernet. Zwraca 1 (sukces), 0 (blad sprzetowy), -1 (zly argument). */
int ne1000_isa_send(const void *frame, uint16_t length);

/* Odbicie jednej ramki. Zwraca dlugosc ramki (>0), 0 (brak ramki), -1 (blad). */
int ne1000_isa_receive(void *frame, uint16_t capacity);

/* Zatrzymanie karty i zwolnienie zasobow. */
void ne1000_isa_shutdown(void);

/* Test wlaczenia (loopback): wysyla i odbiera ramke probna. Zwraca 1 gdy OK. */
int ne1000_isa_loopback_test(void);

/* Statystyki. */
struct ne1000_isa_stats {
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t tx_collisions;
    uint32_t tx_aborts;
    uint32_t tx_carrier_lost;
    uint32_t tx_fifo_underrun;
    uint32_t tx_timeouts;
    uint32_t rx_crc_errors;
    uint32_t rx_frame_errors;
    uint32_t rx_fifo_overrun;
    uint32_t rx_missed_packets;
    uint32_t rx_overflow_errors;
    uint32_t dma_errors;
};

void ne1000_isa_stats_reset(void);
int ne1000_isa_stats_get(struct ne1000_isa_stats *out);

/* Filtry odbioru. */
int ne1000_isa_set_multicast(const uint8_t *addr);
int ne1000_isa_set_promiscuous(int enable);

/* MTU. */
int ne1000_isa_set_mtu(uint16_t mtu);
uint16_t ne1000_isa_get_mtu(void);

/* Stan lacza (carrier sense z TSR.CRS). */
int ne1000_isa_link_up(void);

/* Obsluga przerwan (opcjonalna; domyslnie tryb pollingowy).
 * ne1000_isa_irq_handler jest wywolywana z procedury przerwania IDT. */
int ne1000_isa_irq_enable(uint8_t irq);
void ne1000_isa_irq_disable(void);
int ne1000_isa_irq_handler(void);

/* Pomocnicze: wypisanie rejestrow DS8390 (debug). */
void ne1000_isa_register_dump(void);

#endif /* NE1000_ISA_H */
