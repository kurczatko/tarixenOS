#include "graficzny.h"

void set_mode_13h() {
    asm volatile(
        "mov $0x13, %%ax\n\t"
        "int $0x10"
        :
        :
        : "ax"
    );
}

void putpixel(int x, int y, unsigned char color) {
    unsigned char *video = (unsigned char *)0xA0000;
    unsigned int offset = y * 320 + x;
    video[offset] = color;
}

void wait_key() {
    asm volatile(
        "xor %%ah, %%ah\n\t"
        "int $0x16"
        :
        :
        : "ax"
    );
}

void graficzny() {
    set_mode_13h();
    putpixel(2, 13, 7); //2137, jp2gmd
    wait_key();
}
