#include "panika.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"

void panika(const char *powod)
{
    __asm__ volatile ("movw $0x0003, %%ax; int $0x10" ::: "eax");
    vga_init();
    printf(" !!!!!!!!!!!!!!!!!!!\n");
    printf(" system sie popsul\n");
    printf(" %s\n", powod ? powod : "this is the end");
    printf(" ==================\n");
    printf(" jesli system sie popsul bo uzyles do tego komendy to ok\n");
    printf(" jesli samo to wyslij to (skarge) na kurczatkojestslodkie@gmail.com\n");
    __asm__ __volatile__("cli; hlt");
}