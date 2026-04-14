#include "vga.h"

#define VGA_MEMORY (uint16_t*)0xB8000

static uint16_t* vga_buffer;
static uint8_t row = 0;
static uint8_t col = 0;
uint8_t color = 0x01; //niebieski na czarnym

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

void vga_init() {
    vga_buffer = VGA_MEMORY;
}

void vga_putc(char c) {
    if (c == '\n') {
        row++;
        col = 0;
        return;
    }

    vga_buffer[row * 80 + col] = vga_entry(c, color);
    col++;

    if (col >= 80) {
        col = 0;
        row++;
    }
    
    if (row >= 25) {
        // Scroll up
        for (uint16_t i = 0; i < 1920; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        for (uint16_t i = 1920; i < 2000; i++) {
            vga_buffer[i] = vga_entry(' ', color);
        }
        row = 24;
        col = 0;
    }
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}