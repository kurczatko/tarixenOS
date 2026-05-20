
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "shell.h"
#include "../include/blue.h"
#include "konfiguracja.h"
#include "../scripts/ekran_bootowania.h"
#include "../../drivers/keyboard.h"
#include "../drivers/sound/sound.h"

void kernel_main(void)
{
    // hda_init();
    // hda_play_sound();
    keyboard_init();
    vga_init();
    blue();

    /*
    * mozliwe problemy z blue() poniewaz zmienilem to co robi
    * nazwa moze mylic
    */
    konfiguracja();
    printf("system jest prosty w uzyciu, napisz 'h' aby wejsc w menu pomocy.\n");
    /*
    * inicjacja klawiatury
    * wyczyszczenie ekranu bo uzytkownik go nie wymyl (chyba)
    * konfiguracja
    * inicjacja vga
    * wypisanie tekstu
    * i nareszcie shell
    */
    shell();
}
