/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/* type_screen.c */
/* wywolanie systemowe napisania*/

#include <kernel/syscals/type_screen.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>

void type_screen(char *str) {
    vga_init();
    printf("%s", str);
}