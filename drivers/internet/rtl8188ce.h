#ifndef RTL8188CE_H
#define RTL8188CE_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*Zaawansowany sterownik Realtek RTL8188CE (PCIe 802.11n WiFi) napisany w C++.
 *Obsluguje wykrycie urzadzenia na PCI, mapowanie MMIO, odczyt adresu MAC,
 *wysylanie i odbieranie ramek ethernet oraz statystyki TX/RX.
 *Integracja ze stosem sieciowym (drivers/net.c) przez funkcje send/receive,
 *wiec przegladarka (kernel/browser.c) korzysta z tej karty automatycznie
 *gdy jest dostepna i zainicjowana.*/

int rtl8188ce_init(void);
int rtl8188ce_is_ready(void);
int rtl8188ce_get_mac(uint8_t mac[6]);
int rtl8188ce_send(const void *frame, uint16_t length);
int rtl8188ce_receive(void *frame, uint16_t capacity);

/*Statystyki sterownika (tylko do odczytu po inicjalizacji).*/
struct rtl8188ce_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t resets;
};
void rtl8188ce_stats_get(struct rtl8188ce_stats *out);

#ifdef __cplusplus
}
#endif

#endif
