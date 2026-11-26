#ifndef INTEL_82599ES_H
#define INTEL_82599ES_H

#include <include/printf/types.h>

int intel_82599es_init(void);
int intel_82599es_is_ready(void);
int intel_82599es_get_mac(uint8_t mac[6]);
int intel_82599es_send(const void *frame, uint16_t length);
int intel_82599es_receive(void *frame, uint16_t capacity);
void intel_82599es_shutdown(void);
int intel_82599es_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                            uint32_t *tx_errors, uint32_t *rx_errors,
                            uint32_t *tx_dropped, uint32_t *rx_dropped);
void intel_82599es_stats_reset(void);
int intel_82599es_set_mtu(uint16_t mtu);
uint16_t intel_82599es_get_mtu(void);
int intel_82599es_link_up(void);
int intel_82599es_last_error(void);

#endif /* INTEL_82599ES_H */
