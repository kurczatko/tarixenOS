#include "proces.h"
#include "paging.h"
#include "gdt.h"
#include "syscall.h"
#include "../include/printf/printf.h"
#include "../include/printf/types.h"
#include "idt.h"
#include "timer.h"

struct proces process_table[MAX_PROCESSES];
struct proces *current_process = 0;
static uint32_t next_pid = 1;
static uint32_t process_count = 0;

static uint32_t kernel_page_dir_phys = 0;

static void proces_ustaw_stos(struct proces *p, uint32_t entry_point, uint32_t arg) {
    p->user_stack = 0x003FF000;
    p->kernel_stack = gdt_kernel_stack_top();

    uint32_t *stack = (uint32_t *)p->kernel_stack;
    stack--;
    *stack = GDT_SEL_RING3_DATA;
    stack--;
    *stack = p->user_stack;
    stack--;
    *stack = 0x202;
    stack--;
    *stack = GDT_SEL_RING3_CODE;
    stack--;
    *stack = entry_point;

    p->regs.esp = (uint32_t)stack;
    p->regs.ebp = (uint32_t)stack;
    p->regs.eax = arg;
    p->regs.ebx = 0;
    p->regs.ecx = 0;
    p->regs.edx = 0;
    p->regs.esi = 0;
    p->regs.edi = 0;
    p->regs.eflags = 0x202;
    p->regs.cs = GDT_SEL_RING3_CODE;
    p->regs.ds = GDT_SEL_RING3_DATA;
    p->regs.es = GDT_SEL_RING3_DATA;
    p->regs.fs = GDT_SEL_RING3_DATA;
    p->regs.gs = GDT_SEL_RING3_DATA;
    p->regs.ss = GDT_SEL_RING3_DATA;
    p->regs.eip = entry_point;
}

void proces_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCES_TERMINATED;
    }
    current_process = 0;
    next_pid = 1;
    process_count = 0;

    asm volatile("mov %%cr3, %0" : "=r"(kernel_page_dir_phys));
}

uint32_t *proces_klonuj_page_directory(void) {
    uint32_t *new_dir = (uint32_t *)0x00800000;
    uint32_t *kernel_dir = (uint32_t *)kernel_page_dir_phys;

    for (int i = 0; i < 1024; i++) {
        new_dir[i] = 0;
    }

    for (int i = 0; i < 256; i++) {
        new_dir[i] = kernel_dir[i];
    }

    uint32_t *user_pt = (uint32_t *)0x00801000;
    for (int i = 0; i < 1024; i++) {
        user_pt[i] = 0;
    }

    for (uint32_t addr = USER_MEM_START; addr < USER_MEM_END; addr += PAGE_SIZE) {
        map_page(user_pt, addr, addr, PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);
    }

    new_dir[1] = ((uint32_t)user_pt | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);

    uint32_t *fb_pt = (uint32_t *)0x00802000;
    for (int i = 0; i < 1024; i++) {
        fb_pt[i] = 0;
    }
    for (int i = 0; i < 1024; i++) {
        map_page(fb_pt, 0xFD000000 + i * PAGE_SIZE, 0xFD000000 + i * PAGE_SIZE, PAGE_PRESENT | PAGE_READWRITE);
    }
    new_dir[0xFD000000 >> 22] = ((uint32_t)fb_pt | PAGE_PRESENT | PAGE_READWRITE);

    return new_dir;
}

struct proces *proces_stworz(uint32_t entry_point, const char *name, uint32_t arg) {
    if (process_count >= MAX_PROCESSES) {
        return 0;
    }

    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCES_TERMINATED || process_table[i].pid == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return 0;
    }

    struct proces *p = &process_table[slot];
    p->pid = next_pid++;
    p->state = PROCES_READY;
    p->entry_point = entry_point;
    p->arg = arg;
    p->window = 0;

    for (int i = 0; i < 31 && name[i]; i++) {
        p->name[i] = name[i];
    }
    p->name[31] = 0;

    for (int i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }

    p->page_directory = proces_klonuj_page_directory();
    proces_ustaw_stos(p, entry_point, arg);

    process_count++;
    return p;
}

void proces_zakoncz(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            process_table[i].state = PROCES_TERMINATED;
            process_table[i].pid = 0;
            process_count--;
            break;
        }
    }
}

void proces_tick(void) {
    if (!current_process) return;

    struct proces *next = 0;
    int start = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == current_process->pid) {
            start = i + 1;
            break;
        }
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        int idx = (start + i) % MAX_PROCESSES;
        if (process_table[idx].state == PROCES_READY) {
            next = &process_table[idx];
            break;
        }
    }

    if (!next && current_process->state == PROCES_READY) {
        next = current_process;
    }

    if (!next) return;

    if (current_process->state == PROCES_RUNNING) {
        current_process->state = PROCES_READY;
    }

    next->state = PROCES_RUNNING;
    current_process = next;
}

struct proces *proces_aktualny(void) {
    return current_process;
}

void proces_yield(void) {
    asm volatile("int $0x80" :: "a"(29));
}

uint32_t proces_pobierz_pid(void) {
    if (current_process) return current_process->pid;
    return 0;
}
