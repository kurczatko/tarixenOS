#ifndef USERLAND_SYSCALL_H
#define USERLAND_SYSCALL_H

#include "../include/printf/types.h"

#define SYSCALL_TAPETA_ZMIEN 0
#define SYSCALL_TAPETA_POBIERZ 1
#define SYSCALL_EKRAN_CZYSC 2
#define SYSCALL_EKRAN_PIKSEL 3
#define SYSCALL_EKRAN_PROSTOKAT 4
#define SYSCALL_TEKST_RYSUJ 5
#define SYSCALL_OKNO_STWORZ 6
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
#define SYSCALL_OKNO_ZAMKNIJ 35
#define SYSCALL_OKNO_PIKSEL 36
#define SYSCALL_OKNO_PROSTOKAT 37
#define SYSCALL_OKNO_TEKST 38
#define SYSCALL_OKNO_KLIK 39
#define SYSCALL_OKNO_RUCH 40
#define SYSCALL_KLAWIATURA_GETCHAR 41
#define SYSCALL_PROCES_URUCHOM_ELF 42

static inline uint32_t syscall0(uint32_t num)
{
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(num) : "memory");
    return ret;
}

static inline uint32_t syscall1(uint32_t num, uint32_t arg1)
{
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1) : "memory");
    return ret;
}

static inline uint32_t syscall2(uint32_t num, uint32_t arg1, uint32_t arg2)
{
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2) : "memory");
    return ret;
}

static inline uint32_t syscall3(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3) : "memory");
    return ret;
}

static inline uint32_t syscall4(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4) : "memory");
    return ret;
}

static inline uint32_t syscall5(uint32_t num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5) : "memory");
    return ret;
}

#endif
