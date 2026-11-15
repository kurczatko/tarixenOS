#ifndef MOUSE_H
#define MOUSE_H

#include <include/printf/types.h>

void mouse_init(void);
void mouse_poll(void);
void mouse_pozycja(int *x, int *y);
uint8_t mouse_lewy_przycisk(void);

#endif
