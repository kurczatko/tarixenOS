#ifndef VMXNET3_H
#define VMXNET3_H

#include <include/printf/types.h>

int vmxnet3_init(void);
int vmxnet3_is_ready(void);
int vmxnet3_get_mac(uint8_t mac[6]);
int vmxnet3_send(const void *frame, uint16_t length);
int vmxnet3_receive(void *frame, uint16_t capacity);
void vmxnet3_shutdown(void);
int vmxnet3_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                      uint32_t *tx_errors, uint32_t *rx_errors,
                      uint32_t *tx_dropped, uint32_t *rx_dropped);
void vmxnet3_stats_reset(void);
int vmxnet3_set_mtu(uint16_t mtu);
uint16_t vmxnet3_get_mtu(void);
int vmxnet3_link_up(void);

#endif /* VMXNET3_H */
