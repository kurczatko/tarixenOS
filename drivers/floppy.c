#include "floppy.h"

#define FLOPPY_DOR  0x3F2
#define FLOPPY_MSR  0x3F4
#define FLOPPY_DATA 0x3F5

#define CMD_RECALIBRATE 0x07
#define CMD_SENSE_INT   0x08
#define CMD_SPECIFY     0x03
#define CMD_READ        0xE6
#define CMD_WRITE       0xC5

static int g_floppy_present = 0;

static void outb(uint16_t port, uint8_t v) { __asm__ volatile ("outb %0, %1" : : "a"(v), "Nd"(port)); }
static uint8_t inb(uint16_t port) { uint8_t v; __asm__ volatile ("inb %1, %0" : "=a"(v) : "d"(port)); return v; }

static int msr_wait(int dir_to_cpu, int timeout) {
    while (timeout-- > 0) {
        uint8_t msr = inb(FLOPPY_MSR);
        if ((msr & 0x10) && (((msr & 0x20) != 0) == dir_to_cpu)) return 1;
    }
    return 0;
}

static int cmd_byte(uint8_t b) {
    if (!msr_wait(0, 200000)) return 0;
    outb(FLOPPY_DATA, b);
    return 1;
}

static int result_byte(uint8_t *b) {
    if (!msr_wait(1, 200000)) return 0;
    *b = inb(FLOPPY_DATA);
    return 1;
}

static void motor_on(int drive) {
    outb(FLOPPY_DOR, 0x0C | (drive & 3));
    for (volatile int i = 0; i < 200000; i++);
}

static void motor_off(void) {
    outb(FLOPPY_DOR, 0x0C);
}

static int floppy_reset(void) {
    outb(FLOPPY_DOR, 0x00);
    for (volatile int i = 0; i < 10000; i++);
    outb(FLOPPY_DOR, 0x0C);
    for (volatile int i = 0; i < 100000; i++);
    uint8_t st0, st1;
    cmd_byte(CMD_SENSE_INT);
    result_byte(&st0);
    result_byte(&st1);
    return 1;
}

static int floppy_recalibrate(void) {
    motor_on(0);
    if (!cmd_byte(CMD_RECALIBRATE)) return 0;
    if (!cmd_byte(0x00)) return 0;
    for (volatile int i = 0; i < 500000; i++);
    uint8_t st0, st1;
    cmd_byte(CMD_SENSE_INT);
    result_byte(&st0);
    result_byte(&st1);
    return 1;
}

static int floppy_specify(void) {
    if (!cmd_byte(CMD_SPECIFY)) return 0;
    if (!cmd_byte(0xDF)) return 0;
    if (!cmd_byte(0x02)) return 0;
    return 1;
}

static void lba_to_chs(uint32_t lba, uint8_t *c, uint8_t *h, uint8_t *s) {
    *s = (uint8_t)((lba % 18) + 1);
    *h = (uint8_t)((lba / 18) % 2);
    *c = (uint8_t)(lba / 36);
}

static int floppy_readwrite(uint8_t cmd, uint32_t sector, char *buffer) {
    motor_on(0);
    uint8_t c, h, s;
    lba_to_chs(sector, &c, &h, &s);
    uint8_t drive_head = (uint8_t)(h << 2);
    if (!cmd_byte(cmd)) return 0;
    if (!cmd_byte(drive_head)) return 0;
    if (!cmd_byte(c)) return 0;
    if (!cmd_byte(h)) return 0;
    if (!cmd_byte(s)) return 0;
    if (!cmd_byte(2)) return 0;
    if (!cmd_byte(18)) return 0;
    if (!cmd_byte(0x1B)) return 0;
    if (!cmd_byte(0xFF)) return 0;

    int data_phase = (cmd == CMD_READ) ? 1 : 0;
    for (uint32_t i = 0; i < 512; i++) {
        if (data_phase) {
            uint8_t byte;
            if (!result_byte(&byte)) return 0;
            buffer[i] = (char)byte;
        } else {
            if (!cmd_byte((uint8_t)buffer[i])) return 0;
        }
    }

    uint8_t r[7];
    for (int i = 0; i < 7; i++) {
        if (!result_byte(&r[i])) return 0;
    }
    motor_off();
    return 1;
}

int floppy_read_sector(uint32_t sector, char *buffer) {
    return floppy_readwrite(CMD_READ, sector, buffer);
}

int floppy_write_sector(uint32_t sector, const char *buffer) {
    char tmp[512];
    for (int i = 0; i < 512; i++) tmp[i] = buffer[i];
    return floppy_readwrite(CMD_WRITE, sector, tmp);
}

int floppy_present(void) {
    return g_floppy_present;
}

void floppy_init(void) {
    g_floppy_present = 0;
    if (!floppy_reset()) return;
    if (!floppy_recalibrate()) return;
    if (!floppy_specify()) return;

    char buf[512];
    if (floppy_read_sector(0, buf)) {
        uint16_t bps = (uint16_t)((uint8_t)buf[11] | ((uint16_t)(uint8_t)buf[12] << 8));
        if (bps == 512) g_floppy_present = 1;
    }
}
