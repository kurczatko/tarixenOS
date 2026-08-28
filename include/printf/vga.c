#include "vga.h"

#define VGA_MEMORY (uint16_t*)0xB8000

static uint16_t* vga_buffer;
static uint8_t row = 0;
static uint8_t col = 0;
uint8_t color = 0x0E;

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

void vga_init() {
    vga_buffer = VGA_MEMORY;
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0A), "Nd"((uint16_t)0x3D4));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D5));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0B), "Nd"((uint16_t)0x3D4));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D5));
    vga_update_cursor();
}

void vga_putc(char c) {
    if (c == '\n') {
        row++;
        col = 0;
        vga_update_cursor();
        return;
    }

    if (c == '\b') {
        if (col > 0) {
            col--;
            vga_buffer[row * 80 + col] = vga_entry(' ', color);
        }
        vga_update_cursor();
        return;
    }

    vga_buffer[row * 80 + col] = vga_entry(c, color);
    col++;

    if (col >= 80) {
        col = 0;
        row++;
    }
    
    if (row >= 25) {
        //przesuwanie ekranu
        for (uint16_t i = 0; i < 1920; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        for (uint16_t i = 1920; i < 2000; i++) {
            vga_buffer[i] = vga_entry(' ', color);
        }
        row = 24;
        col = 0;
    }

    vga_update_cursor();
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