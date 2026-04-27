#include "shell.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../kernel/apps/kaka.h"
#include "../kernel/apps/pakiet.h"
#include "../kernel/apps/calc.h"
#include "../kernel/apps/appdownload/appdownload.h"
#include "../kernel/apps/wyp.hpp"
#include "../kernel/apps/kalendarz.h"
#include "panika.h"
#include "../scripts/mem.h"

void shell() {
    vga_init();
    keyboard_init();

    printf(" > ");
    char buf[256] = {0};
    int i = 0;
    while (1) {
        char com = keyboard_getchar();
        if (com == '\n' || com == '\r') {
            printf("\n");
            buf[i] = 0;

            if (buf[0] == 'h') {
                printf(" h - pomoc\n");
                printf(" v - wersja\n");
                printf(" l - logo\n");
                printf(" i - info o systemie\n");
                printf(" p - pisanie\n");
                printf(" k - gra pt. kaka\n");
                printf(" { - pakiet biurowy\n");
                printf(" c - kalkulator\n");
                printf(" a - sklep z aplikacjami\n");
                printf(" r - wylaczenie\n");
                printf(" w - wypisanie tekstu\n");
                printf(" g - kalendarz\n");
                printf(" u - panika jadra\n");
                printf(" m - ilosc dostepnej pamieci\n");
                printf(" cs - sprawdz sam\n");
            }

            if (buf[0] == 'v') {
                printf(" wersja: 2.0.0\n");
                printf("\n");
            }

            if(buf[0] == 'c' && buf[1] == 's') {
                printf(" UwU\n");
            }

            if(buf[0]=='m'){
                mem();
            }

            if (buf[0] == 'u') {
                panika();
            }

            if (buf[0] == 'g'){
                kalendarz();
            }

            if (buf[0] == 'w') {
                wyp();
            }

            if (buf[0] == 'r') {
                asm volatile("cli; hlt");
            }

            if (buf[0] == 'a') {
                appd();
            }

            if (buf[0] == '{') {
                pakiet();
            }

            if (buf[0] == 'c') {
                calc();
            }

            if (buf[0] == 'l') {
                printf(" ---------------------\n");
                printf("                     -\n");
                printf("                     -\n");
                printf("                     -\n");
                printf(" ---------------------\n");
                printf(" -\n");
                printf(" -\n");
                printf(" -\n");
                printf( "---------------------\n");
            }

            if(buf[0] == 'i') {
                printf(" system name: tarixenOS\n");
                printf(" 2026 version\n");
            }

            if (buf[0] == 'k') {
                kaka();
            }

            if (buf[0] == 'p') {
                printf(" napisz cos:\n");

                while(1) {
                    char c = keyboard_getchar();
                    printf("%c", c);
                }
            }

            printf(" > ");
            i = 0;
        } else if (com == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < 255) {
            buf[i++] = com;
            printf("%c", com);
        }
    }
}

