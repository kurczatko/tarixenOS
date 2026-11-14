/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <include/blue.h>
#include <include/printf/vga.h>
#include <include/printf/types.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>

// funkcja na poczatku miala robic niebieski ekran ale tera zluzy jako czyszczenie ekranu
void blue(void) {
    uint16_t* vga_buffer = (uint16_t*)0xB8000;
    uint16_t white_entry = (uint16_t)' ' | (uint16_t)0x70 << 8;

    vga_mouse_cursor_hide();
    
    // Wypełnij cały ekran (2000 pozycji)
    for (int i = 0; i < 2000; i++) {
        vga_buffer[i] = white_entry;
    }
    vga_draw_top_bar();
    vga_reset_text_cursor();
}
