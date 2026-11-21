#ifndef VIRTIO_NET_H
#define VIRTIO_NET_H

#include <include/printf/types.h>

/*
 * Zaawansowany sterownik karty sieciowej virtio-net (nowoczesny interfejs
 * virtio 1.0+, nie legacy). Karta virtio-net to wirtualizowana karta
 * Ethernet PCI (vendor 0x1AF4, urzadzenia 0x1046/0x1048/0x1049) ktora
 * komunikuje sie z goscciem za pomoca virtqueues (kolejki deskryptorow,
 * pierscieni dostepnych i uzytych) w pamieci fizycznej.
 *
 * Opis zasteczajacy implementacji:
 *   - specyfikacja "virtio: I/O Virtualization Devices" (OASIS, v1.1+),
 *     sekcja urzadzenia sieciowego (virtio-net) i transportu PCI (virtio-pci),
 *   - "Virtio PCI Card Specification" (modern/interfejs nowoczesny z
 *     capabilities w BAR0),
 *   - sterownik Linux drivers/net/virtio_net.c (koncepcje kolejki RX/TX,
 *     negocjacja featureow, obsługa MAC i MTU).
 *
 * Sterownik operuje w trybie pollingu (brak przerwan), zgodnie ze stosem
 * sieciowym net.c. Inicjalizacja wykrywa karte po PCI, negocjuje feature'y,
 * buduje kolejki RX i TX virtqueue, pobiera adres MAC z konfiguracji urzadzenia
 * oraz wystawia interfejs send/receive uzywane przez net.c.
 */

int virtio_net_init(void);
int virtio_net_is_ready(void);
int virtio_net_get_mac(uint8_t mac[6]);
int virtio_net_send(const void *frame, uint16_t length);
int virtio_net_receive(void *frame, uint16_t capacity);
void virtio_net_shutdown(void);
int virtio_net_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                         uint32_t *tx_errors, uint32_t *rx_errors,
                         uint32_t *tx_dropped, uint32_t *rx_dropped);
void virtio_net_stats_reset(void);
int virtio_net_set_mtu(uint16_t mtu);
uint16_t virtio_net_get_mtu(void);

#endif
