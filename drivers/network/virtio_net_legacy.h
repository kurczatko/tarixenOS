#ifndef VIRTIO_NET_LEGACY_H
#define VIRTIO_NET_LEGACY_H

#include <include/printf/types.h>

/*
 * Sterownik karty sieciowej virtio-net w trybie LEGACY (stary interfejs
 * virtio, nie nowoczesny/virtio 1.0+). Karta virtio-net to wirtualizowana
 * karta Ethernet PCI (vendor 0x1AF4, urzadzenia 0x1046/0x1048/0x1049) ktora
 * w trybie legacy komunikuje sie z goscia za pomoca rejestrow w przestrzeni
 * I/O (BAR0) oraz virtqueues (kolejki deskryptorow, pierscieni dostepnych
 * i uzytych) w pamieci fizycznej.
 *
 * Opis zasteczajacy implementacji:
 *   - "virtio: I/O Virtualization Devices" (OASIS virtio v1.0/v1.1),
 *     dodatek legacy (Appendix: Virtio over PCI legacy interface),
 *   - "Virtio PCI Card Specification" (tryb legacy, rejestry w BAR0 I/O),
 *   - koncepcje kolejki RX/TX virtqueue i negocjacji featureow ze sterownika
 *     Linux drivers/net/virtio_net.c (tryb legacy).
 *
 * Roznice wzgledem wersji nowoczesnej (virtio_net.c):
 *   - brak lancucha capabilities w BAR0; rejestry pod stalymi offsetami I/O,
 *   - konfiguracja urzadzenia (MAC, MTU) odczytywana z PCI config space,
 *   - rejestr device_status pod stalym offsetem 0x70 w BAR0,
 *   - powiadomienie urzadzenia (notify) przez zapis indeksu kolejki do portu I/O,
 *   - tryb pollingu, zgodnie ze stosem sieciowym net.c.
 */

int virtio_net_legacy_init(void);
int virtio_net_legacy_is_ready(void);
int virtio_net_legacy_get_mac(uint8_t mac[6]);
int virtio_net_legacy_send(const void *frame, uint16_t length);
int virtio_net_legacy_receive(void *frame, uint16_t capacity);
void virtio_net_legacy_shutdown(void);
int virtio_net_legacy_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                                uint32_t *tx_errors, uint32_t *rx_errors,
                                uint32_t *tx_dropped, uint32_t *rx_dropped);
void virtio_net_legacy_stats_reset(void);
int virtio_net_legacy_set_mtu(uint16_t mtu);
uint16_t virtio_net_legacy_get_mtu(void);

#endif
