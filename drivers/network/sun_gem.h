#ifndef SUN_GEM_H
#define SUN_GEM_H

#include <include/printf/types.h>

/*
 * Zaawansowany sterownik karty sieciowej Sun GEM (Gigabit Ethernet MAC).
 * Karta Sun GEM to kontroler Ethernet 10/100/1000 Mbps produkcji Sun
 * Microsystems (vendor 0x108E, urzadzenia 0x2BAD / 0x1101 / 0x1001)
 * obecny m.in. w platformach Sun UltraSPARC oraz Apple UniNorth (GMAC).
 *
 * Opis zasteczajacy implementacji:
 *   - "sungem.h: Definitions for Sun GEM ethernet driver" (Linux kernel,
 *     drivers/net/ethernet/sun/sungem.h, David S. Miller),
 *   - "sungem.c: Sun GEM ethernet driver" (Linux kernel,
 *     drivers/net/ethernet/sun/sungem.c),
 *   - "Sun GEM device emulation" (Qemu, net/sungem.c).
 *
 * Sterownik operuje w trybie MMIO (BAR0) i buduje pierścienie deskryptorow
 * RX i TX w pamieci fizycznej, zgodnie ze specyfikacja GEM:
 *   - deskryptory TX: control_word (8B) + buffer (8B) = 16 bajtow,
 *   - deskryptory RX: status_word (8B) + buffer (8B) = 16 bajtow,
 *   - rejestra TXDMA/RXDMA/MAC pod stalymi offsetami MMIO,
 *   - brak przerwan (polling), zgodnie ze stosem sieciowym net.c.
 *
 * Rejestry oparte na dokumentacji Linux sungem.h (offsety wzgledem BAR0 MMIO):
 *   GREG_STAT 0x000C, GREG_IMASK 0x0010,
 *   TXDMA_KICK 0x2000, TXDMA_CFG 0x2004, TXDMA_DBLOW 0x2008, TXDMA_DBHI 0x200C,
 *   RXDMA_CFG 0x4000, RXDMA_DBLOW 0x4004, RXDMA_DBHI 0x4008,
 *   MAC_TXRST 0x6000, MAC_RXRST 0x6004, MAC_TXCFG 0x6030, MAC_RXCFG 0x6034.
 *
 * Integracja: net.c (net_init) oraz driver_manager_register w kernel.c.
 */

int sun_gem_init(void);
int sun_gem_is_ready(void);
int sun_gem_get_mac(uint8_t mac[6]);
int sun_gem_send(const void *frame, uint16_t length);
int sun_gem_receive(void *frame, uint16_t capacity);
void sun_gem_shutdown(void);
int sun_gem_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                      uint32_t *tx_errors, uint32_t *rx_errors,
                      uint32_t *tx_dropped, uint32_t *rx_dropped);
void sun_gem_stats_reset(void);
int sun_gem_set_mtu(uint16_t mtu);
uint16_t sun_gem_get_mtu(void);

#endif
