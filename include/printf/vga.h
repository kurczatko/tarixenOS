#ifndef VGA_H
#define VGA_H

#include "types.h"

void vga_init();
void vga_draw_top_bar(void);
void vga_reset_text_cursor(void);
void vga_text_cursor_position(int* x, int* y);
void vga_set_text_cursor_position(int x, int y);
void vga_text_cursor_tick(void);
void vga_putc(char c);
void vga_print(const char* str);
void vga_cursor_left(void);
void vga_cursor_right(void);
void vga_mouse_cursor_init(void);
void vga_mouse_cursor_move(int delta_x, int delta_y);
void vga_mouse_cursor_hide(void);
void vga_mouse_cursor_position(int* x, int* y);

#endif