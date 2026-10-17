#include "mouse.h"
#include "../include/printf/types.h"
#include "../include/printf/vga.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/okno.h"
#include "../graficzny/menu_start.h"
#include "../graficzny/pulpit.h"
#include "../kernel/browser.h"

#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64

static uint8_t packet[4];
static uint8_t packet_index = 0;
static uint8_t packet_size = 3;
static int accumulated_x = 0;
static int accumulated_y = 0;
static uint8_t left_button_pressed = 0;

void mouse_pozycja(int *x, int *y) {
    kursor_graficzny_pozycja(x, y);
}

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

static uint8_t mouse_write_and_read_ack(uint8_t value)
{
    mouse_write(value);
    mouse_wait_output();
    return mouse_inb(MOUSE_DATA_PORT);
}

void mouse_init(void) {
    uint8_t response;

    mouse_wait_input();
    mouse_outb(MOUSE_COMMAND_PORT, 0xA8);
    mouse_write_and_read_ack(0xF5);
    mouse_write_and_read_ack(0xF3);
    mouse_write_and_read_ack(200);
    mouse_write_and_read_ack(0xF3);
    mouse_write_and_read_ack(100);
    mouse_write_and_read_ack(0xF3);
    mouse_write_and_read_ack(80);
    mouse_write(0xF2);
    mouse_wait_output();
    response = mouse_inb(MOUSE_DATA_PORT);
    if (response == 0xFA) {
        mouse_wait_output();
        response = mouse_inb(MOUSE_DATA_PORT);
        if (response == 0x03 || response == 0x04) packet_size = 4;
    }
    mouse_write_and_read_ack(0xF4);
    (void)response;
    kursor_graficzny_init();
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
    if (packet_index < packet_size) {
        return;
    }
    packet_index = 0;

    if (!(packet[0] & 0x08)) {
        return;
    }

    if (packet_size == 4) {
        int wheel = packet[3] & 0x0F;
        if (wheel & 0x08) wheel -= 16;
        if (wheel) browser_scroll_wheel(wheel);
    }

    if ((packet[0] & 1) && !left_button_pressed) {
        left_button_pressed = 1;
        {
            int x;
            int y;
            mouse_pozycja(&x, &y);
            if (!okno_obsluz_klikniecie(x, y)) {
                if (!pulp_mouse_press(x, y)) {
                    if (y >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC && x >= PASEK_ICON_START_X) {
                        struct okno *ikona_okno = pasek_zadan_ikona_na_pozycji(x, y);
                        if (ikona_okno) {
                            okno_przywroc(ikona_okno);
                        } else {
                            menu_start_mouse_click();
                        }
                    } else {
                        menu_start_mouse_click();
                    }
                }
            }
        }
    }
    if (!(packet[0] & 1)) {
        left_button_pressed = 0;
        okno_zakoncz_przeciaganie();
        {
            int x;
            int y;
            mouse_pozycja(&x, &y);
            pulp_mouse_release(x, y);
        }
    }

    delta_x = (int)packet[1];
    delta_y = (int)packet[2];
    if (packet[0] & 0x10) delta_x -= 256;
    if (packet[0] & 0x20) delta_y -= 256;

    accumulated_x += delta_x;
    accumulated_y += delta_y;
    delta_x = accumulated_x / 2;
    delta_y = accumulated_y / 2;
    accumulated_x %= 2;
    accumulated_y %= 2;

    if (delta_x != 0 || delta_y != 0) {
        kursor_graficzny_przesun(delta_x, -delta_y);
        {
            int x;
            int y;
            mouse_pozycja(&x, &y);
            if (left_button_pressed) {
                okno_obsluz_ruch(x, y);
                pulp_mouse_ruch(x, y);
            }
        }
    }
}