#ifndef ZEGAR_H
#define ZEGAR_H

#include "../include/printf/types.h"

void zegar_init(void);
int zegar_synchronizuj_http(const char *response, uint32_t length);
int zegar_pobierz(char tekst[9]);

#endif
