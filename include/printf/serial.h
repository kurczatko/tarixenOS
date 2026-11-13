#ifndef SERIAL_H
#define SERIAL_H

#include <include/printf/types.h>

void serial_init(void);
void serial_putc(char c);
void serial_print(const char* str);

#endif
