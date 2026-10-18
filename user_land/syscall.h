#ifndef SYSCALL_USER_H
#define SYSCALL_USER_H

#include "../include/printf/types.h"

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

static inline uint32_t syscall_wywolaj(uint32_t numer, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    uint32_t wynik;
    asm volatile ("int $0x80"
        : "=a"(wynik)
        : "a"(numer), "b"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5)
        : "memory");
    return wynik;
}

static inline void tapeta_zmien(uint32_t kolor)
{
    syscall_wywolaj(SYSCALL_TAPETA_ZMIEŃ, kolor, 0, 0, 0, 0);
}

static inline void ekran_czysc(uint32_t kolor)
{
    syscall_wywolaj(SYSCALL_EKRAN_CZYŚĆ, kolor, 0, 0, 0, 0);
}

static inline void ekran_piksel(int x, int y, uint32_t kolor)
{
    syscall_wywolaj(SYSCALL_EKRAN_PIKSEL, (uint32_t)x, (uint32_t)y, kolor, 0, 0);
}

static inline void ekran_prostokat(int x, int y, int sx, int sy, uint32_t kolor)
{
    syscall_wywolaj(SYSCALL_EKRAN_PROSTOKĄT, (uint32_t)x, (uint32_t)y, (uint32_t)sx, (uint32_t)sy, kolor);
}

static inline void tekst_rysuj(int x, int y, const char *tekst, uint32_t kolor)
{
    syscall_wywolaj(SYSCALL_TEKST_RYSUJ, (uint32_t)x, (uint32_t)y, (uint32_t)tekst, kolor, 0);
}

static inline void okno_stworz(int x, int y, int sx, int sy, const char *tytul)
{
    syscall_wywolaj(SYSCALL_OKNO_STWÓRZ, (uint32_t)x, (uint32_t)y, (uint32_t)sx, (uint32_t)sy, (uint32_t)tytul);
}

static inline void druk(const char *tekst)
{
    syscall_wywolaj(SYSCALL_DRUKUJ, (uint32_t)tekst, 0, 0, 0, 0);
}

#endif
