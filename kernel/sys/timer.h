#ifndef TIMER_H
#define TIMER_H

#include <include/printf/types.h>

void timer_init(uint32_t frequency);
void timer_handler(void);
uint32_t timer_ticks_get(void);

#endif
