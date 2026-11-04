#ifndef PROCES_H
#define PROCES_H

#include "../include/printf/types.h"
#include "paging.h"
#include "gdt.h"
#include "graficzny/okno.h"

#define MAX_PROCESSES 32
#define MAX_OKNA_PROCESU 8

typedef enum {
    PROCES_READY = 0,
    PROCES_RUNNING = 1,
    PROCES_BLOCKED = 2,
    PROCES_TERMINATED = 3
} proces_state_t;

struct proces_rejestry {
    uint32_t esp;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
    uint32_t eip;
};

struct proces {
    struct proces_rejestry regs;
    uint32_t pid;
    proces_state_t state;
    uint32_t *page_directory;
    uint32_t kernel_stack;
    uint32_t user_stack;
    uint32_t entry_point;
    char name[32];
    struct okno *window;
    uint32_t arg;
    struct okno okna[MAX_OKNA_PROCESU];
    uint8_t okna_uzyte[MAX_OKNA_PROCESU];
};

extern struct proces *current_process;
extern struct proces process_table[MAX_PROCESSES];

void proces_init(void);
struct proces *proces_stworz(uint32_t entry_point, const char *name, uint32_t arg);
void proces_zakoncz(uint32_t pid);
void proces_tick(void);
struct proces *proces_aktualny(void);
void proces_yield(void);
uint32_t proces_pobierz_pid(void);

uint32_t *proces_klonuj_page_directory(void);

void proces_rejestruj_okno(const char *nazwa, struct okno *okno);

#endif
