#ifndef VGA_H
#define VGA_H

#include "types.h"

void vga_init();
void vga_putc(char c);
void vga_print(const char* str);

#endif