# tekst

gdy chcesz wypisac tekst w naglowku dodaj 
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"

a nastepnie aby wypisac tekst napisz printf("tekst\n");

WAŻNE: napisz vga_init(); 

# uruchamianie

aby uruchomic napisz w głównym katalogu komende:

make

a nastpenie wejdz w folder build uzywajac:

cd build

a nastepnie komenda:

qemu-system-x86_64 -hda tarixenios.img

uwaga musisz mieć qemu!

prosimy o usuwanie folderu build do produkcji, po tym jak się przetestowało i wszytsko działa.

# pisanie klawiaturą

aby uzytkownik systemu mogl uzyc klawiatury, do kodu nalezy dodac:

#include "../../drivers/keyboard.h"

a nastepnie:

    while(1) {
        char c = keyboard_getchar();
        printf("%c", c);
    }

chyba ze nie chcesz aby tekst uzytkownika byl wyswietlany, wtedy mozesz wyrzucic printf-a.

WAZNE: dodaj keyboard_init();

# iso 

aby zrobic iso uruchom komende w glownym folderze:

cd build/iso && xorriso -as mkisofs -o ../tarixeni.iso -b bootx64.efi -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot bootx64.efi .

UWAGA: NARAZIE WERSJA ISO JEST EKSPERYMENTALNA I NIE OBIECUJEMY ZE ZADZIALA NA PRAWDZIWYM SPRZECIE
