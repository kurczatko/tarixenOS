#ifndef E1000E_H
#define E1000E_H

#include <include/printf/types.h>

int e1000e_init(void);
int e1000e_is_ready(void);
int e1000e_get_mac(uint8_t mac[6]);
int e1000e_send(const void *frame, uint16_t length);
int e1000e_receive(void *frame, uint16_t capacity);
void e1000e_shutdown(void);
int e1000e_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                     uint32_t *tx_errors, uint32_t *rx_errors,
                     uint32_t *tx_dropped, uint32_t *rx_dropped);
void e1000e_stats_reset(void);
int e1000e_set_mtu(uint16_t mtu);
uint16_t e1000e_get_mtu(void);
int e1000e_link_up(void);

#endif
