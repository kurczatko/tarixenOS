/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/* type_add.c */
/* wywolanie systemowe dodawania */

#include <kernel/syscals/type_add.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>

void type_add(int liczba1, int liczba2) {
    int wynik = liczba1 + liczba2;
    vga_init();
    printf("%d + %d = %d", liczba1, liczba2, wynik);
}