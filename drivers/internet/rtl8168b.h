#ifndef RTL8168B_H
#define RTL8168B_H

#include "../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Realtek RTL8168B / RTL8111B family PCIe Gigabit Ethernet driver.
 * Provides the same send/receive interface used by the network stack
 * (drivers/net.c) so the browser (kernel/browser.c) can use it via
 * net_ready() / net_http_get(). */

int rtl8168b_init(void);
int rtl8168b_is_ready(void);
int rtl8168b_get_mac(uint8_t mac[6]);
int rtl8168b_send(const void *frame, uint16_t length);
int rtl8168b_receive(void *frame, uint16_t capacity);

/* Driver statistics (read-only after init). */
struct rtl8168b_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t resets;
};
void rtl8168b_stats_get(struct rtl8168b_stats *out);

#ifdef __cplusplus
}
#endif

#endif
