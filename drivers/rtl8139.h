#ifndef RTL8139_H
#define RTL8139_H

#include "../include/printf/types.h"

int rtl8139_init(void);
int rtl8139_is_ready(void);
int rtl8139_get_mac(uint8_t mac[6]);
int rtl8139_send(const void *frame, uint16_t length);
int rtl8139_receive(void *frame, uint16_t capacity);

#endif