#ifndef MOUSE_H
#define MOUSE_H

#include <include/printf/types.h>

void mouse_init(void);
void mouse_poll(void);
void mouse_process_event(int delta_x, int delta_y, int wheel, uint8_t buttons);
void mouse_pozycja(int *x, int *y);
uint8_t mouse_lewy_przycisk(void);

#endif
