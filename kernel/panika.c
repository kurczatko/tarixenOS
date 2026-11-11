#include "panika.h"
#include "../include/printf/printf.h"

void panika(const char *powod)
{
    printf(" !!!!!!!!!!!!!!!!!!!\n");
    printf(" system sie popsul\n");
    printf(" %s\n", powod ? powod : "this is the end");
    printf(" ==================\n");
    printf(" jesli system sie popsul bo uzyles do tego komendy to ok\n");
    printf(" jesli samo to wyslij to (skarge) na kurczatkojestslodkie@gmail.com\n");
    for(;;) { /*200 IQ pętla ktorej nie powstydzilby sie nawet einstein*/
        __asm__ __volatile__ ("hlt");
    }
}