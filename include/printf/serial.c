#include "serial.h"

#define SERIAL_COM1 0x3F8

static void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init(void) {
    uint16_t port = SERIAL_COM1;

    outb(port + 1, 0x00);         
    outb(port + 3, 0x80);        
    outb(port + 0, 0x03);        
    outb(port + 1, 0x00);         
    outb(port + 3, 0x03);        
    outb(port + 2, 0xC7);       
    outb(port + 4, 0x0B);         
    outb(port + 4, 0x1E);        
    outb(port + 0, 0xAE);         
    if (inb(port) != 0xAE) {
        return;                   
    }
    outb(port + 4, 0x0F);         
}

void serial_putc(char c) {
    uint16_t port = SERIAL_COM1;
    while (!(inb(port + 5) & 0x20)) {
    }
    outb(port, c);
}

void serial_print(const char* str) {
    while (*str) {
        serial_putc(*str++);
    }
}
