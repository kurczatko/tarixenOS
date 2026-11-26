#ifndef I210_H
#define I210_H

#include <include/printf/types.h>

int i210_init(void);
int i210_is_ready(void);
int i210_get_mac(uint8_t mac[6]);
int i210_send(const void *frame, uint16_t length);
int i210_receive(void *frame, uint16_t capacity);
void i210_shutdown(void);
int i210_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                   uint32_t *tx_errors, uint32_t *rx_errors,
                   uint32_t *tx_dropped, uint32_t *rx_dropped);
void i210_stats_reset(void);
int i210_set_mtu(uint16_t mtu);
uint16_t i210_get_mtu(void);
int i210_link_up(void);
int i210_last_error(void);

#endif
