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

#define SYSCALL_PROCES_STWORZ 27
#define SYSCALL_PROCES_ZAKONCZ 28
#define SYSCALL_PROCES_YIELD 29
#define SYSCALL_PROCES_POBIERZ_PID 30

#define SYSCALL_FS_LIST 31
#define SYSCALL_FS_CD 32
#define SYSCALL_FS_PWD 33
#define SYSCALL_FS_RESET_DIR 34

// New window syscalls
#define SYSCALL_OKNO_ZAMKNIJ 35
#define SYSCALL_OKNO_PIKSEL 36
#define SYSCALL_OKNO_PROSTOKAT 37
#define SYSCALL_OKNO_TEKST 38
#define SYSCALL_OKNO_KLIK 39
#define SYSCALL_OKNO_RUCH 40

// Keyboard syscall
#define SYSCALL_KLAWIATURA_GETCHAR 41

// Load and run ELF from filesystem
#define SYSCALL_PROCES_URUCHOM_ELF 42

void syscall_install(void);
struct rejestry {
    uint32_t eax, ebx, ecx, edx, esi, edi;
};
extern struct rejestry biezace_rejestry;
extern uint32_t userspace_break;
uint32_t syscall_handler(void);

static inline int wskaznik_uzytkownika_prawidlowy(const void *ptr)
{
    return (uint32_t)ptr < GRANICA_PAMIEC_UZYTKOWNIKA;
}

#endif
