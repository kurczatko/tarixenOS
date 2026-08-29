#include "menu_start.h"
#include "../include/blue.h"
#include "../include/printf/types.h"
#include "../include/printf/vga.h"

static uint8_t menu_open = 0;
static uint16_t saved_screen[2000];
static int saved_cursor_x;
static int saved_cursor_y;

void menu_start() {
	uint16_t* vga_buffer = (uint16_t*)0xB8000;
	uint16_t blue_entry = (uint16_t)' ' | (uint16_t)0x17 << 8;
	uint8_t column;

	vga_mouse_cursor_hide();
	vga_draw_top_bar();
	for (column = 0; column < 6; column++) {
		vga_buffer[column] = blue_entry;
	}
	vga_buffer[1] = (uint16_t)'s' | (uint16_t)0x17 << 8;
	vga_buffer[2] = (uint16_t)'t' | (uint16_t)0x17 << 8;
	vga_buffer[3] = (uint16_t)'a' | (uint16_t)0x17 << 8;
	vga_buffer[4] = (uint16_t)'r' | (uint16_t)0x17 << 8;
	vga_buffer[5] = (uint16_t)'t' | (uint16_t)0x17 << 8;
	vga_mouse_cursor_move(0, 0);
}

void menu_start_click(void) {
	uint16_t* vga_buffer = (uint16_t*)0xB8000;
	uint16_t blue_entry = (uint16_t)' ' | (uint16_t)0x17 << 8;
	int index;

	menu_open = 1;
	vga_mouse_cursor_hide();
	vga_text_cursor_position(&saved_cursor_x, &saved_cursor_y);
	for (index = 0; index < 2000; index++) {
		saved_screen[index] = vga_buffer[index];
		vga_buffer[index] = blue_entry;
	}
	vga_buffer[79] = (uint16_t)'X' | (uint16_t)0x47 << 8;
	vga_mouse_cursor_move(0, 0);
}

void menu_start_mouse_click(void) {
	int mouse_x;
	int mouse_y;
	int index;
	uint16_t* vga_buffer = (uint16_t*)0xB8000;

	vga_mouse_cursor_position(&mouse_x, &mouse_y);
	if (menu_open) {
		if (mouse_x == 79 && mouse_y == 0) {
			menu_open = 0;
			vga_mouse_cursor_hide();
			for (index = 0; index < 2000; index++) {
				vga_buffer[index] = saved_screen[index];
			}
			vga_set_text_cursor_position(saved_cursor_x, saved_cursor_y);
		}
		return;
	}

	if (mouse_x < 6 && mouse_y == 0) {
		menu_start_click();
	}
}