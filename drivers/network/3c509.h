#ifndef C509_H
#define C509_H

#include <include/printf/types.h>

/*
 * Sterownik karty sieciowej 3Com EtherLink III (3C509 / 3C509B).
 * Karta ISA 10 Mbps. Rejestry oparte na "EtherLink III Parallel Tasking
 * ISA, EISA, Micro Channel, and PCMCIA Adapter Drivers Technical Reference"
 * oraz sterowniku Linux 3c509.c (Donald Becker).
 */

int  c509_init(void);
int  c509_is_ready(void);
int  c509_get_mac(uint8_t mac[6]);
int  c509_send(const void *frame, uint16_t length);
int  c509_receive(void *frame, uint16_t capacity);
void c509_shutdown(void);
int  c509_loopback_test(void);
void c509_stats_reset(void);
int  c509_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                    uint32_t *tx_errors, uint32_t *rx_errors);
int  c509_set_multicast(const uint8_t *addr);

#endif
