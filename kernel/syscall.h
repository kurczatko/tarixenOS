#ifndef SYSCALL_H
#define SYSCALL_H

#include "../include/printf/types.h"
#include "userspace.h"

#define SYSCALL_TAPETA_ZMIEŃ 0
#define SYSCALL_TAPETA_POBIERZ 1
#define SYSCALL_EKRAN_CZYŚĆ 2
#define SYSCALL_EKRAN_PIKSEL 3
#define SYSCALL_EKRAN_PROSTOKĄT 4
#define SYSCALL_TEKST_RYSUJ 5
#define SYSCALL_OKNO_STWÓRZ 6
#define SYSCALL_OKNO_RYSUJ 7
#define SYSCALL_MYSZ_POZYCJA 8
#define SYSCALL_DRUKUJ 9
#define SYSCALL_WRITE_CONSOLE 20
#define SYSCALL_EXIT 21
#define SYSCALL_BRK 22
#define SYSCALL_OPEN 23
#define SYSCALL_READ 24
#define SYSCALL_CLOSE 25
#define SYSCALL_WRITE_FILE 26

void syscall_install(void);
struct rejestry {
    uint32_t eax, ebx, ecx, edx, esi, edi;
};
extern struct rejestry biezace_rejestry;
extern uint32_t userspace_break;
uint32_t syscall_handler(void);

/* Sprawdza, czy wskaznik lezy w przestrzeni uzytkownika (ponizej granicy jadra). */
static inline int wskaznik_uzytkownika_prawidlowy(const void *ptr)
{
    return (uint32_t)ptr < GRANICA_PAMIEC_UZYTKOWNIKA;
}

#endif
