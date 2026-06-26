
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

    /*
    * mozliwe problemy z blue() poniewaz zmienilem to co robi
    * nazwa moze mylic
    */
    konfiguracja();
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

// ten plik ma byc najmniejszy jak sie da zeby latwo sie lapalo bledy, a wszystko co bardziej zlozone lepiej dokumentowane (dzieki bogu narazie jest krotki)
