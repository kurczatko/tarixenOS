#ifndef TLS_CLIENT_H
#define TLS_CLIENT_H

#include "../include/printf/types.h"

int tls13_https_get(const char *host, const char *path, char *response, uint32_t capacity);

#endif
