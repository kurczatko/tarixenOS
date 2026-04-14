#include "blue.h"
#include "printf/vga.h"
#include "printf/types.h"

// Funkcja zmieniająca cały ekran na niebieski (niebieskie tło, czarny tekst)
// Używa kolor 0x01 (niebieskie tło w trybie VGA text)
// Wypełnia cały bufor VGA spacjami - 80 kolumn x 25 wierszy = 2000 komórek
void blue(void) {
    uint16_t* vga_buffer = (uint16_t*)0xB8000;
    uint16_t blue_entry = (uint16_t)' ' | (uint16_t)0x01 << 8;  // Spacja z niebieskim tłem
    
    // Wypełnij cały ekran (2000 pozycji)
    for (int i = 0; i < 2000; i++) {
        vga_buffer[i] = blue_entry;
    }
}

