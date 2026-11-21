#ifndef PCNET_H
#define PCNET_H

#include <include/printf/types.h>

/*
 * Zaawansowany sterownik karty sieciowej AMD PCnet (Am79C970 / Am79C971 /
 * Am79C972 / Am79C973 / Am79C974 / Am79C975) - karta sieciowa PCI 10/100 Mbps.
 *
 * Rejestry, blok inicjalizacji, struktury deskryptorow piercieni (RX/TX) oraz
 * sekwencja inicjalizacji sa oparte na dokumentacji technicznej AMD:
 *   - "Am79C970 / Am79C971 PCnet-PCI Ethernet Controller Data Sheet" (AMD),
 *   - "Am79C973 PCnet-PCI Ethernet Controller Data Sheet" (AMD),
 *   - sterownik Linux drivers/net/ethernet/amd/pcnet32.c,
 *   - sterownik NetBSD sys/dev/ic/am7990.c oraz naglowki am7990reg.h.
 *
 * Sterownik operuje w trybie 32-bitowym (CSR3 SWSTYLE = 32-bit), buduje
 * blok inicjalizacji oraz pierścienie deskryptorow odbiorczych i nadawczych
 * w pamieci, uzyskuje adres MAC (z EEPROM karty lub domyslny lokalnie
 * administrowany), obsluguje nadawanie i odbieranie ramek Ethernet w trybie
 * pollingu, prowadzi statystyki bledow (CRC, kolizje, przepelnienie,
 * nieodebrane ramki, bledy pamieci, babble), obsluguje filtr multicast,
 * tryb posluchawczy oraz test petli wewnetrznej (internal loopback).
 * Integracja: net.c (net_init) oraz driver_manager_register w kernel.c.
 */

int pcnet_init(void);
int pcnet_is_ready(void);
int pcnet_get_mac(uint8_t mac[6]);
int pcnet_send(const void *frame, uint16_t length);
int pcnet_receive(void *frame, uint16_t capacity);
void pcnet_shutdown(void);
int pcnet_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                    uint32_t *tx_errors, uint32_t *rx_errors,
                    uint32_t *tx_collisions, uint32_t *rx_crc_errors);
void pcnet_stats_reset(void);
int pcnet_set_multicast(const uint8_t *addr);
int pcnet_set_promiscuous(uint8_t enable);
int pcnet_loopback_test(void);

#endif
