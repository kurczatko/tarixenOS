#include "vga.h"

#define VGA_MEMORY (uint16_t*)0xB8000

static uint16_t* vga_buffer;
static uint8_t row = 0;
static uint8_t col = 0;
uint8_t color = 0x70;

static void vga_update_cursor(void) {
    uint16_t position = row * 80 + col;
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)position), "Nd"((uint16_t)0x3D5));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)(position >> 8)), "Nd"((uint16_t)0x3D5));
}

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

static void vga_mouse_cursor_show(void);

static void vga_scroll(void) {
    uint16_t source_row;
    uint16_t column;

    for (source_row = 2; source_row < 25; source_row++) {
        for (column = 0; column < 80; column++) {
            vga_buffer[(source_row - 1) * 80 + column] =
                vga_buffer[source_row * 80 + column];
        }
    }
    for (column = 0; column < 80; column++) {
        vga_buffer[24 * 80 + column] = vga_entry(' ', color);
    }
    row = 24;
    col = 0;
}

void vga_draw_top_bar(void) {
    uint16_t index;

    vga_mouse_cursor_hide();
    for (index = 0; index < 80; index++) {
        vga_buffer[index] = vga_entry(' ', 0x20);
    }
    vga_mouse_cursor_show();
}

static int mouse_x = 40;
static int mouse_y = 12;
static uint16_t mouse_saved_cell;
static uint8_t mouse_cursor_drawn = 0;

void vga_mouse_cursor_hide(void) {
    if (mouse_cursor_drawn) {
        vga_buffer[mouse_y * 80 + mouse_x] = mouse_saved_cell;
        mouse_cursor_drawn = 0;
    }
}

static void vga_mouse_cursor_show(void) {
    uint16_t cell;
    uint8_t attributes;

    if (mouse_cursor_drawn) {
        vga_mouse_cursor_hide();
    }

    cell = vga_buffer[mouse_y * 80 + mouse_x];
    mouse_saved_cell = cell;
    attributes = (uint8_t)(cell >> 8);
    attributes = (uint8_t)((attributes << 4) | (attributes >> 4));
    vga_buffer[mouse_y * 80 + mouse_x] =
        (cell & 0x00FF) | ((uint16_t)attributes << 8);
    mouse_cursor_drawn = 1;
}

void vga_mouse_cursor_init(void) {
    mouse_x = 40;
    mouse_y = 12;
    vga_mouse_cursor_show();
}

void vga_mouse_cursor_move(int delta_x, int delta_y) {
    vga_mouse_cursor_hide();
    mouse_x += delta_x;
    mouse_y += delta_y;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x > 79) mouse_x = 79;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y > 24) mouse_y = 24;

    vga_mouse_cursor_show();
}

void vga_init() {
    vga_buffer = VGA_MEMORY;
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0A), "Nd"((uint16_t)0x3D4));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D5));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0B), "Nd"((uint16_t)0x3D4));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D5));
    vga_draw_top_bar();
    vga_update_cursor();
}

void vga_reset_text_cursor(void) {
    vga_mouse_cursor_hide();
    row = 1;
    col = 0;
    vga_update_cursor();
    vga_mouse_cursor_show();
}

void vga_putc(char c) {
    vga_mouse_cursor_hide();

    if (c == '\n') {
        row++;
        col = 0;
        if (row >= 25) {
            vga_scroll();
        }
        vga_update_cursor();
        vga_mouse_cursor_show();
        return;
    }

    if (c == '\b') {
        if (col > 0) {
            col--;
            vga_buffer[row * 80 + col] = vga_entry(' ', color);
        }
        vga_update_cursor();
        vga_mouse_cursor_show();
        return;
    }

    vga_buffer[row * 80 + col] = vga_entry(c, color);
    col++;

    if (col >= 80) {
        col = 0;
        row++;
    }
    
    if (row >= 25) {
        vga_scroll();
    }

    vga_update_cursor();
    vga_mouse_cursor_show();
}

void vga_cursor_left(void) {
    if (col > 0) {
        col--;
    } else if (row > 0) {
        row--;
        col = 79;
    }
    vga_update_cursor();
}

void vga_cursor_right(void) {
    if (col < 79) {
        col++;
    } else if (row < 24) {
        row++;
        col = 0;
    }
    vga_update_cursor();
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}