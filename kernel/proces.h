#ifndef PROCES_H
#define PROCES_H

#include "../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "paging.h"
#include "gdt.h"
#include "graficzny/okno.h"

#define MAX_PROCESSES 64
#define MAX_OKNA_PROCESU 8

#define NR_PRIORITIES 140
#define MAX_WAIT_QUEUES 32
#define IDLE_PRIORITY (NR_PRIORITIES - 1)
#define DEFAULT_TIME_SLICE 4
#define TICK_NS 10000000ULL

typedef enum {
    PROCES_TERMINATED = 0,
    PROCES_READY = 1,
    PROCES_RUNNING = 2,
    PROCES_BLOCKED = 3,
    PROCES_SLEEPING = 4,
    PROCES_ZOMBIE = 5
} proces_state_t;

typedef enum {
    SCHED_IDLE = 0,
    SCHED_BACKGROUND = 1,
    SCHED_NORMAL = 2,
    SCHED_INTERACTIVE = 3,
    SCHED_REALTIME = 4,
    SCHED_CRITICAL = 5
} sched_class_t;

#define SCHED_NORMAL_NAME    "NORMAL"
#define SCHED_INTERACTIVE_NAME "INTERACTIVE"
#define SCHED_REALTIME_NAME   "REALTIME"
#define SCHED_BACKGROUND_NAME "BACKGROUND"
#define SCHED_IDLE_NAME       "IDLE"
#define SCHED_CRITICAL_NAME   "CRITICAL"

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

    sched_class_t sched_class;
    int32_t priority;
    int32_t static_priority;
    uint32_t time_slice;
    uint32_t exec_start;
    uint64_t vruntime;
    uint64_t sum_exec_runtime;
    uint64_t wait_start;
    uint64_t sleep_until;
    uint32_t period;
    uint32_t deadline;
    uint32_t cpu;
    uint8_t preempt_count;
    uint8_t prio_inherited;
    int32_t original_prio;
    uint32_t policy;
    struct proces *wait_next;
    struct proces *wait_prev;
    struct proces *run_next;
    struct proces *run_prev;
    uint64_t ctx_switches;
    uint64_t nr_wakeups;
    uint64_t nr_migrations;
    uint64_t nr_blocked;
    uint32_t sleep_ticks;
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

void proces_sleep_until(uint64_t ticks);
int proces_set_priority(uint32_t pid, int32_t priority);
int proces_set_sched_class(uint32_t pid, sched_class_t cls);
void proces_block(void);
void proces_wakeup(struct proces *p);
struct proces *proces_find(uint32_t pid);
int proces_get_stats(uint32_t pid, struct proces *out);

#ifdef __cplusplus
}
#endif

#endif
