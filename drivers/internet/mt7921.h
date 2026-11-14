#ifndef MT7921_H
#define MT7921_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int mt7921_init(void);
int mt7921_is_ready(void);
int mt7921_get_mac(uint8_t mac[6]);
int mt7921_send(const void *frame, uint16_t length);
int mt7921_receive(void *frame, uint16_t capacity);
int mt7921_link_up(void);
int mt7921_speed_mbps(void);
int mt7921_rssi(void);
int mt7921_channel_get(void);
int mt7921_channel_set(uint8_t channel);
int mt7921_ssid_scan(void);
int mt7921_autonegotiate(void);
int mt7921_reset_link(void);
int mt7921_loopback_test(const void *frame, uint16_t length);
void mt7921_set_multicast(const uint8_t *addresses, uint32_t count);
struct mt7921_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t resets;
    uint32_t link_changes;
    uint32_t auth_failures;
    uint32_t scan_results;
};
void mt7921_stats_get(struct mt7921_stats *out);
void mt7921_stats_reset(void);
void mt7921_shutdown(void);
void mt7921_poll(void);

#ifdef __cplusplus
}
#endif

#endif
