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

    outb(port + 1, 0x00);         // Disable all interrupts
    outb(port + 3, 0x80);         // Enable DLAB (set baud rate divisor)
    outb(port + 0, 0x03);         // Divisor 38400 baud (low byte)
    outb(port + 1, 0x00);         // (high byte)
    outb(port + 3, 0x03);         // 8 bits, no parity, one stop bit
    outb(port + 2, 0xC7);         // Enable FIFO, clear them, with 14-byte threshold
    outb(port + 4, 0x0B);         // IRQs enabled, RTS/DSR set
    outb(port + 4, 0x1E);         // Set in loopback mode, test the serial chip
    outb(port + 0, 0xAE);         // Test serial chip (send byte 0xAE and check if serial returns same byte)
    if (inb(port) != 0xAE) {
        return;                   // Serial chip not found
    }
    outb(port + 4, 0x0F);         // Normal operation
}

void serial_putc(char c) {
    uint16_t port = SERIAL_COM1;
    while (!(inb(port + 5) & 0x20)) {
        ; // wait for Transmitter Holding Register Empty
    }
    outb(port, c);
}

void serial_print(const char* str) {
    while (*str) {
        serial_putc(*str++);
    }
}
