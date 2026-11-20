#ifndef NE2000_H
#define NE2000_H

#include <include/printf/types.h>

int ne2000_init(void);
int ne2000_is_ready(void);
int ne2000_get_mac(uint8_t mac[6]);
int ne2000_send(const void *frame, uint16_t length);
int ne2000_receive(void *frame, uint16_t capacity);
void ne2000_shutdown(void);
int ne2000_loopback_test(void);
void ne2000_stats_reset(void);
int ne2000_stats_get(uint32_t *tx_packets, uint32_t *rx_packets, uint32_t *tx_errors, uint32_t *rx_errors);
int ne2000_set_multicast(const uint8_t *addr);

#endif
