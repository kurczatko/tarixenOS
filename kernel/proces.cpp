#include "proces.h"
#include "timer.h"
#include "idt.h"
#include "../include/printf/printf.h"

struct proces process_table[MAX_PROCESSES];
struct proces *current_process = 0;
static uint32_t next_pid = 1;
static uint32_t process_count = 0;
static uint32_t kernel_page_dir_phys = 0;
static uint64_t global_ticks = 0;

static uint32_t prio_bitmap[(NR_PRIORITIES + 31) / 32] = {0};
static struct proces *prio_head[NR_PRIORITIES] = {0};
static struct proces *prio_tail[NR_PRIORITIES] = {0};

static struct proces *wait_head = 0;
static struct proces *wait_tail = 0;
static struct proces *idle_task = 0;

static inline int prio_clamp(int p) {
    if (p < 0) return 0;
    if (p >= (int)NR_PRIORITIES) return (int)NR_PRIORITIES - 1;
    return p;
}

static inline void prio_bitmap_set(int prio) {
    prio_bitmap[prio >> 5] |= (1U << (prio & 31));
}

static inline void prio_bitmap_clear(int prio) {
    prio_bitmap[prio >> 5] &= ~(1U << (prio & 31));
}

static inline int prio_find_highest(void) {
    for (int i = ((int)NR_PRIORITIES + 31) / 32 - 1; i >= 0; i--) {
        if (prio_bitmap[i]) {
            return i * 32 + (31 - __builtin_clz(prio_bitmap[i]));
        }
    }
    return -1;
}

static void runq_enqueue(struct proces *p) {
    int prio = prio_clamp(p->priority);
    if (p->sched_class == SCHED_REALTIME || p->sched_class == SCHED_CRITICAL) {
        p->run_next = 0;
        p->run_prev = prio_tail[prio];
        if (prio_tail[prio]) prio_tail[prio]->run_next = p;
        else prio_head[prio] = p;
        prio_tail[prio] = p;
    } else {
        struct proces *cur = prio_head[prio];
        struct proces *prev = 0;
        while (cur && cur->vruntime <= p->vruntime) {
            prev = cur;
            cur = cur->run_next;
        }
        p->run_next = cur;
        p->run_prev = prev;
        if (prev) prev->run_next = p;
        else prio_head[prio] = p;
        if (cur) cur->run_prev = p;
        else prio_tail[prio] = p;
    }
    prio_bitmap_set(prio);
}

static void runq_dequeue(struct proces *p) {
    int prio = prio_clamp(p->priority);
    if (p->run_prev) p->run_prev->run_next = p->run_next;
    else prio_head[prio] = p->run_next;
    if (p->run_next) p->run_next->run_prev = p->run_prev;
    else prio_tail[prio] = p->run_prev;
    p->run_next = 0;
    p->run_prev = 0;
    if (!prio_head[prio]) prio_bitmap_clear(prio);
}

static void waitq_enqueue(struct proces *p) {
    p->wait_next = 0;
    p->wait_prev = wait_tail;
    if (wait_tail) wait_tail->wait_next = p;
    else wait_head = p;
    wait_tail = p;
}

static void waitq_dequeue(struct proces *p) {
    if (p->wait_prev) p->wait_prev->wait_next = p->wait_next;
    else wait_head = p->wait_next;
    if (p->wait_next) p->wait_next->wait_prev = p->wait_prev;
    else wait_tail = p->wait_prev;
    p->wait_next = 0;
    p->wait_prev = 0;
}

static struct proces *sched_pick_next(void) {
    int prio = prio_find_highest();
    if (prio < 0) return idle_task;
    struct proces *p = prio_head[prio];
    while (p && (p->state != PROCES_READY && p->state != PROCES_RUNNING)) {
        p = p->run_next;
    }
    return p;
}

static void proces_ustaw_stos(struct proces *p, uint32_t entry_point, uint32_t arg) {
    p->user_stack = 0x003FF000;
    p->kernel_stack = (uint32_t)gdt_kernel_stack_top();

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

static uint32_t time_slice_for(struct proces *p) {
    if (p->sched_class == SCHED_REALTIME || p->sched_class == SCHED_CRITICAL) return 2U;
    if (p->sched_class == SCHED_IDLE) return 1U;
    if (p->sched_class == SCHED_INTERACTIVE) return 6U;
    if (p->sched_class == SCHED_BACKGROUND) return 8U;
    return DEFAULT_TIME_SLICE;
}

void proces_init(void) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        p->pid = 0;
        p->state = PROCES_TERMINATED;
        p->page_directory = 0;
        p->kernel_stack = 0;
        p->user_stack = 0;
        p->entry_point = 0;
        p->window = 0;
        p->arg = 0;
        p->run_next = 0;
        p->run_prev = 0;
        p->wait_next = 0;
        p->wait_prev = 0;
        p->vruntime = 0;
        p->priority = 120;
        p->static_priority = 120;
        p->time_slice = 0;
        p->sched_class = SCHED_NORMAL;
        p->ctx_switches = 0;
        p->nr_wakeups = 0;
        p->nr_migrations = 0;
        p->nr_blocked = 0;
        p->sleep_ticks = 0;
        p->sleep_until = 0;
        p->wait_start = 0;
        p->sum_exec_runtime = 0;
        p->exec_start = 0;
        p->period = 0;
        p->deadline = 0;
        p->cpu = 0;
        p->preempt_count = 0;
        p->prio_inherited = 0;
        p->original_prio = 0;
        p->policy = 0;
        for (uint32_t k = 0; k < MAX_OKNA_PROCESU; k++) {
            p->okna_uzyte[k] = 0;
        }
    }
    for (uint32_t i = 0; i < ((NR_PRIORITIES + 31) / 32); i++) prio_bitmap[i] = 0;
    for (uint32_t i = 0; i < NR_PRIORITIES; i++) {
        prio_head[i] = 0;
        prio_tail[i] = 0;
    }
    wait_head = 0;
    wait_tail = 0;
    current_process = 0;
    next_pid = 1;
    process_count = 0;
    global_ticks = 0;
    idle_task = 0;

    asm volatile("mov %%cr3, %0" : "=r"(kernel_page_dir_phys));

    idle_task = proces_stworz(0, "idle", 0);
    if (idle_task) {
        idle_task->sched_class = SCHED_IDLE;
        idle_task->priority = IDLE_PRIORITY;
        idle_task->static_priority = IDLE_PRIORITY;
        idle_task->time_slice = 1U;
        idle_task->state = PROCES_READY;
        runq_enqueue(idle_task);
    }
}

static struct proces *proces_alloc_slot(void) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCES_TERMINATED || process_table[i].pid == 0) {
            return &process_table[i];
        }
    }
    return 0;
}

struct proces *proces_stworz(uint32_t entry_point, const char *name, uint32_t arg) {
    if (process_count >= MAX_PROCESSES) return 0;
    struct proces *p = proces_alloc_slot();
    if (!p) return 0;

    p->pid = next_pid++;
    p->state = PROCES_READY;
    p->entry_point = entry_point;
    p->arg = arg;
    p->window = 0;
    p->page_directory = proces_klonuj_page_directory();
    p->sched_class = SCHED_NORMAL;
    p->priority = 120;
    p->static_priority = 120;
    p->time_slice = time_slice_for(p);
    p->vruntime = 0;
    p->sum_exec_runtime = 0;
    p->exec_start = global_ticks;
    p->wait_start = 0;
    p->sleep_until = 0;
    p->period = 0;
    p->deadline = 0;
    p->cpu = 0;
    p->preempt_count = 0;
    p->prio_inherited = 0;
    p->original_prio = p->priority;
    p->policy = 0;
    p->nr_wakeups = 0;
    p->nr_migrations = 0;
    p->nr_blocked = 0;
    p->sleep_ticks = 0;
    p->ctx_switches = 0;

    for (uint32_t i = 0; i < 31 && name && name[i]; i++) {
        p->name[i] = name[i];
    }
    p->name[31] = 0;
    if (!name) p->name[0] = 0;

    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }

    proces_ustaw_stos(p, entry_point, arg);
    runq_enqueue(p);
    process_count++;
    return p;
}

void proces_zakoncz(uint32_t pid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        if (p->pid == pid) {
            if (p->state == PROCES_RUNNING && current_process == p) {
                p->state = PROCES_TERMINATED;
                p->pid = 0;
                process_count--;
                current_process = 0;
                return;
            }
            if (p->state == PROCES_READY) runq_dequeue(p);
            else if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) waitq_dequeue(p);
            p->state = PROCES_TERMINATED;
            p->pid = 0;
            process_count--;
            return;
        }
    }
}

void proces_tick(void) {
    global_ticks++;

    struct proces *w = wait_head;
    while (w) {
        struct proces *next = w->wait_next;
        if (w->sleep_until && global_ticks >= w->sleep_until) {
            w->sleep_until = 0;
            w->state = PROCES_READY;
            waitq_dequeue(w);
            runq_enqueue(w);
            w->nr_wakeups++;
        }
        w = next;
    }

    if (current_process && current_process->state == PROCES_RUNNING) {
        current_process->sum_exec_runtime += TICK_NS;
        current_process->vruntime += TICK_NS;
        if (current_process->time_slice > 0) current_process->time_slice--;

        if (current_process->time_slice == 0 && current_process != idle_task) {
            current_process->state = PROCES_READY;
            runq_enqueue(current_process);
            current_process->time_slice = time_slice_for(current_process);
        }
    }

    struct proces *next = sched_pick_next();
    if (next && next != current_process) {
        if (current_process && current_process->state == PROCES_RUNNING) {
            current_process->ctx_switches++;
        }
        current_process = next;
        current_process->state = PROCES_RUNNING;
        if (current_process != idle_task && current_process->state == PROCES_READY) {
            runq_dequeue(current_process);
        }
    } else if (!next || next == idle_task) {
        if (current_process && current_process->state == PROCES_RUNNING && current_process != idle_task) {
            current_process->ctx_switches++;
        }
        current_process = idle_task ? idle_task : 0;
        if (idle_task) idle_task->state = PROCES_RUNNING;
    }
}

struct proces *proces_aktualny(void) {
    return current_process;
}

void proces_yield(void) {
    if (!current_process) return;
    if (current_process->state == PROCES_RUNNING) {
        current_process->state = PROCES_READY;
        runq_enqueue(current_process);
        current_process->time_slice = time_slice_for(current_process);
    }
    struct proces *next = sched_pick_next();
    if (next && next != current_process) {
        current_process->ctx_switches++;
        current_process = next;
        current_process->state = PROCES_RUNNING;
        if (current_process != idle_task) runq_dequeue(current_process);
    }
}

uint32_t proces_pobierz_pid(void) {
    if (current_process) return current_process->pid;
    return 0;
}

void proces_sleep_until(uint64_t ticks) {
    if (!current_process) return;
    current_process->state = PROCES_SLEEPING;
    current_process->sleep_until = ticks;
    current_process->sleep_ticks++;
    waitq_enqueue(current_process);
    struct proces *next = sched_pick_next();
    if (next && next != current_process) {
        current_process->ctx_switches++;
        current_process = next;
        current_process->state = PROCES_RUNNING;
        if (current_process != idle_task) runq_dequeue(current_process);
    } else {
        current_process = idle_task ? idle_task : 0;
        if (idle_task) idle_task->state = PROCES_RUNNING;
    }
}

void proces_block(void) {
    if (!current_process) return;
    current_process->state = PROCES_BLOCKED;
    current_process->nr_blocked++;
    waitq_enqueue(current_process);
    struct proces *next = sched_pick_next();
    if (next && next != current_process) {
        current_process->ctx_switches++;
        current_process = next;
        current_process->state = PROCES_RUNNING;
        if (current_process != idle_task) runq_dequeue(current_process);
    } else {
        current_process = idle_task ? idle_task : 0;
        if (idle_task) idle_task->state = PROCES_RUNNING;
    }
}

void proces_wakeup(struct proces *p) {
    if (!p) return;
    if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) {
        waitq_dequeue(p);
        p->sleep_until = 0;
        p->state = PROCES_READY;
        p->nr_wakeups++;
        runq_enqueue(p);
    }
}

struct proces *proces_find(uint32_t pid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) return &process_table[i];
    }
    return 0;
}

int proces_set_priority(uint32_t pid, int32_t priority) {
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    int old = p->priority;
    p->priority = priority;
    p->original_prio = priority;
    p->time_slice = time_slice_for(p);
    if (p->state == PROCES_READY) {
        runq_dequeue(p);
        runq_enqueue(p);
    }
    return old;
}

int proces_set_sched_class(uint32_t pid, sched_class_t cls) {
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    p->sched_class = cls;
    p->time_slice = time_slice_for(p);
    if (p->state == PROCES_READY) {
        runq_dequeue(p);
        runq_enqueue(p);
    }
    return 0;
}

int proces_get_stats(uint32_t pid, struct proces *out) {
    struct proces *p = proces_find(pid);
    if (!p || !out) return -1;
    *out = *p;
    return 0;
}

uint32_t *proces_klonuj_page_directory(void) {
    uint32_t *new_dir = (uint32_t *)0x00800000;
    uint32_t *kernel_dir = (uint32_t *)kernel_page_dir_phys;

    for (int i = 0; i < 1024; i++) new_dir[i] = 0;

    for (int i = 0; i < 256; i++) new_dir[i] = kernel_dir[i];

    uint32_t *user_pt = (uint32_t *)0x00801000;
    for (int i = 0; i < 1024; i++) user_pt[i] = 0;

    for (uint32_t addr = USER_MEM_START; addr < USER_MEM_END; addr += PAGE_SIZE) {
        map_page(user_pt, addr, addr, PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);
    }

    new_dir[1] = ((uint32_t)user_pt | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);

    uint32_t *fb_pt = (uint32_t *)0x00802000;
    for (int i = 0; i < 1024; i++) fb_pt[i] = 0;
    for (int i = 0; i < 1024; i++) {
        map_page(fb_pt, 0xFD000000 + i * PAGE_SIZE, 0xFD000000 + i * PAGE_SIZE, PAGE_PRESENT | PAGE_READWRITE);
    }
    new_dir[0xFD000000 >> 22] = ((uint32_t)fb_pt | PAGE_PRESENT | PAGE_READWRITE);

    return new_dir;
}

void proces_rejestruj_okno(const char *nazwa, struct okno *okno) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        if (p->pid != 0 && p->state != PROCES_TERMINATED) {
            int rowna = 1;
            for (uint32_t k = 0; k < 32; k++) {
                if (p->name[k] != (nazwa ? nazwa[k] : 0)) { rowna = 0; break; }
                if (p->name[k] == 0) break;
            }
            if (rowna) {
                p->window = okno;
                p->state = PROCES_READY;
                return;
            }
        }
    }
    int slot = -1;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == 0 || process_table[i].state == PROCES_TERMINATED) {
            slot = (int)i;
            break;
        }
    }
    if (slot == -1) return;
    struct proces *p = &process_table[slot];
    p->pid = next_pid++;
    p->state = PROCES_READY;
    p->window = okno;
    p->entry_point = 0;
    p->arg = 0;
    p->kernel_stack = 0;
    p->user_stack = 0;
    p->page_directory = 0;
    if (nazwa) {
        for (uint32_t i = 0; i < 31 && nazwa[i]; i++) p->name[i] = nazwa[i];
    }
    p->name[31] = 0;
    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }
    process_count++;
    runq_enqueue(p);
}
