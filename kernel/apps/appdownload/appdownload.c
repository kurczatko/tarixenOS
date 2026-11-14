/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/apps/appdownload/appdownload.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <drivers/keyboard.h>

void appd() {
    vga_init();
    printf(" lista aplikacji:\n");
}