#include <kernel/apps/appdownload/appdownload.h>
#include <include/printf/printf.h>
#include <include/printf/vga.h>
#include <drivers/keyboard.h>

void appd() {
    vga_init();
    printf(" lista aplikacji:\n");
}