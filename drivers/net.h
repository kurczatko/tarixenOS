#ifndef NET_H
#define NET_H

#include "../include/printf/types.h"

int net_init(void);
int net_ready(void);
int net_http_get(const char *host, const char *path, char *response, uint32_t capacity);

struct net_tcp_connection {
    uint8_t remote_ip[4];
    uint8_t remote_mac[6];
    uint16_t local_port;
    uint16_t remote_port;
    uint32_t sequence;
    uint32_t acknowledgement;
    uint8_t connected;
};

int net_tcp_connect(struct net_tcp_connection *connection, const char *host, uint16_t port);
int net_tcp_send(struct net_tcp_connection *connection, const uint8_t *data, uint16_t length);
int net_tcp_receive(struct net_tcp_connection *connection, uint8_t *data, uint16_t capacity,
                    uint32_t attempts);
void net_tcp_close(struct net_tcp_connection *connection);

#endif
