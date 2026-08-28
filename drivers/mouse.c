#include "mouse.h"
#include "../include/printf/types.h"
#include "../include/printf/vga.h"

#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64

static uint8_t packet[3];
static uint8_t packet_index = 0;
static int accumulated_x = 0;
static int accumulated_y = 0;

static uint8_t mouse_inb(uint16_t port) {
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void mouse_outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void mouse_wait_input(void) {
    while (mouse_inb(MOUSE_STATUS_PORT) & 2) {
    }
}

static void mouse_wait_output(void) {
    while (!(mouse_inb(MOUSE_STATUS_PORT) & 1)) {
    }
}

static void mouse_write(uint8_t value) {
    mouse_wait_input();
    mouse_outb(MOUSE_COMMAND_PORT, 0xD4);
    mouse_wait_input();
    mouse_outb(MOUSE_DATA_PORT, value);
}

void mouse_init(void) {
    uint8_t response;

    mouse_wait_input();
    mouse_outb(MOUSE_COMMAND_PORT, 0xA8);
    mouse_write(0xF4);
    mouse_wait_output();
    response = mouse_inb(MOUSE_DATA_PORT);
    (void)response;
    vga_mouse_cursor_init();
}

void mouse_poll(void) {
    uint8_t status;
    uint8_t packet_byte;
    int delta_x;
    int delta_y;

    status = mouse_inb(MOUSE_STATUS_PORT);
    if (!(status & 1) || !(status & 0x20)) {
        return;
    }

    packet_byte = mouse_inb(MOUSE_DATA_PORT);
    if (packet_index == 0 && !(packet_byte & 0x08)) {
        return;
    }
    packet[packet_index++] = packet_byte;
    if (packet_index < 3) {
        return;
    }
    packet_index = 0;

    if (!(packet[0] & 0x08)) {
        return;
    }

    delta_x = (int)packet[1];
    delta_y = (int)packet[2];
    if (packet[0] & 0x10) delta_x -= 256;
    if (packet[0] & 0x20) delta_y -= 256;

    accumulated_x += delta_x;
    accumulated_y += delta_y;
    delta_x = accumulated_x / 8;
    delta_y = accumulated_y / 16;
    accumulated_x %= 8;
    accumulated_y %= 16;

    if (delta_x != 0 || delta_y != 0) {
        vga_mouse_cursor_move(delta_x, -delta_y);
    }
}
