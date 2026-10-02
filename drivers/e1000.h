#ifndef E1000_H
#define E1000_H

#include "../include/printf/types.h"

int e1000_init(void);
int e1000_is_ready(void);
int e1000_get_mac(uint8_t mac[6]);
int e1000_send(const void *frame, uint16_t length);
int e1000_receive(void *frame, uint16_t capacity);

#endif
