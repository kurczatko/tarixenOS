#ifndef RTL8111E_H
#define RTL8111E_H

#include "../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int rtl8111e_init(void);
int rtl8111e_is_ready(void);
int rtl8111e_get_mac(uint8_t mac[6]);
int rtl8111e_send(const void *frame, uint16_t length);
int rtl8111e_receive(void *frame, uint16_t capacity);

int rtl8111e_link_up(void);
int rtl8111e_speed_mbps(void);
int rtl8111e_duplex_full(void);
int rtl8111e_autonegotiate(void);
int rtl8111e_reset_link(void);
int rtl8111e_loopback_test(const void *frame, uint16_t length);
void rtl8111e_set_multicast(const uint8_t *addresses, uint32_t count);

struct rtl8111e_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t resets;
    uint32_t link_changes;
};
void rtl8111e_stats_get(struct rtl8111e_stats *out);

#ifdef __cplusplus
}
#endif

#endif
