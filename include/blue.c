#include "blue.h"
#include "printf/vga.h"
#include "printf/types.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"

// funkcja na poczatku miala robic niebieski ekran ale tera zluzy jako czyszczenie ekranu
void blue(void) {
        uint16_t* vga_buffer = (uint16_t*)0xB8000;
    uint16_t blue_entry = (uint16_t)' ' | (uint16_t)0x01 << 8;
    
    // Wypełnij cały ekran (2000 pozycji)
    for (int i = 0; i < 2000; i++) {
        vga_buffer[i] = blue_entry;
    }
}
