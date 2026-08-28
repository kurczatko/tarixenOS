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
#include "../include/blue.h"
#include "../user_land/konta.h"
#include "../system_plikow/system_plikow.h"
#include "syscals/random.h"
#include "shutdown.h"

void shell() {
    vga_init();
    keyboard_init();

    konta();

    printf(" &#| ");
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
                printf(" b - czyszczenie ekranu\n");
                printf(" z - losowa liczba\n");
                printf(" ^ - wszytsko co zwiazane z plikami\n");
            }

            if (buf[0] == 'v') {
                printf(" wersja: 3.0.0\n");
                printf("\n");
            }



            if (buf[0] == '^') {
                printf(" czy chcesz:\n");
                printf(" a. stworzyc folder\n b. wyswietlic zawartosc tego gdzie jestes\n c. stworzyc plik\n UWAGA! jak wpisujesz nazwe to tylko jedna litera.");
                char odp = keyboard_getchar();
                if (odp == 'a') {
                    printf(" nazwa:");
                    char nazwa[2] = { keyboard_getchar(), 0 };
                    create_dir(nazwa);
                }
                if (odp == 'b') {
                    printf(" w jakim folderze?:");
                    char folder[2] = { keyboard_getchar(), 0 };
                    ls(folder);
                }
                if (odp == 'c') {
                    printf(" nazwa:");
                    char plik[2] = { keyboard_getchar(), 0 };
                    create_file(plik, 32);
                }
            }

            if (buf[0] == 'b') {
                blue();
            }

            if (buf[0] == 'z') {
                printf(" losowa liczba:\n");
                random();
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
                shutdown();
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

            printf(" &#| ");
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

