#include "konta.h"
#include "../include/printf/printf.h"
#include "../include/printf/vga.h"
#include "../../drivers/keyboard.h"
#include "../include/blue.h"
#include "../kernel/shell.h"
#include "../kernel/panika.h"

void konta() {
    vga_init();
    keyboard_init();
    printf(" prosimy o utworzenie konta.\n");
    printf(" jak chcesz sie nazywac? (jedna litera)\n");
    char uzytkownik = keyboard_getchar();
    
    int uzytkownik_upr = 0;
    /*
    * domyslne uprawnienia czyli zwykly uztkownik
    * jesli jest ustawione na 1 to jest administrator
    * eksperymentalne
    */

    if (uzytkownik_upr == 0) {
        printf("masz status zwyklego uzytkownika.\n");
    }
    else if (uzytkownik_upr == 1) {
        printf("masz status administratora.\n");
    } else {
        printf("system uprawnien zostal uszkodzony.\n");
        panika();
    }

    //wybor obrazka (testowe)

    printf(" wybierz obrazek na konto\n");
    printf(" 1. UwU\n");
    printf(" 2. -_-\n");
    printf(" 3. :3\n");
    char obrazek = keyboard_getchar();

    printf(" jakie haslo? (jeden znak) \n");
    char haslo = keyboard_getchar();
    blue();
    if (obrazek == '1') {
        printf(" UwU\n");
    } else if (obrazek == '2') {
        printf(" -_-\n");
    } else if (obrazek == '3') {
        printf(" :3\n");
    }
    printf(" %c\n", uzytkownik);
    printf(" wpisz haslo:");
    char zgdh = keyboard_getchar();
    if (zgdh == haslo) {
        printf(" dobrze!\n");
    } else {
        printf(" zle! wywalaj wlamywaczu\n");
        while(1) {
            keyboard_getchar();
        }
    }
    // zaawansowane opcje logowania
    if(zgdh == '^') {
        printf(" zaawansowane opcje logowania\n");
        printf(" 1. przejdz do powloki bez logowania bo zapomniales hasla\n");
        printf(" wtedy kliknij 1\n");
        char opcja = keyboard_getchar();
        if (opcja == '1') {
            blue();
            printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        } else {
            blue();
            printf(" system zostaje zatrzymany\n");
            asm volatile("hlt");
        }
    }
}