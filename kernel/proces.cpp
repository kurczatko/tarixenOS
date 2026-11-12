#include "proces.h"
#include "timer.h"
#include "idt.h"
#include "../include/printf/printf.h"

struct proces process_table[MAX_PROCESSES];
struct proces *current_cpu[SMP_MAX_CPUS] = {0};
struct proces *current_process = 0;

static uint32_t next_pid = 1;
static uint32_t process_count = 0;
static uint32_t kernel_page_dir_phys = 0;
static uint64_t global_ticks = 0;
uint32_t smp_cpu_count = 1;
uint32_t cpu_online_mask = 1;
volatile uint32_t smp_this_cpu = 0;

static uint32_t driver_kernel_stacks[64][1024] __attribute__((aligned(16)));
static uint32_t driver_stack_count = 0;

struct runqueue {
    uint32_t prio_bitmap[(NR_PRIORITIES + 31) / 32];
    struct proces *prio_head[NR_PRIORITIES];
    struct proces *prio_tail[NR_PRIORITIES];
    uint64_t nr_running;
    uint64_t load;
    volatile uint32_t lock;
};

static struct runqueue cpu_rq[SMP_MAX_CPUS];

static struct proces *wait_head = 0;
static struct proces *wait_tail = 0;
static volatile uint32_t waitq_lock = 0;
static struct proces *idle_task[SMP_MAX_CPUS] = {0};

uint32_t cpu_id(void) {
    return smp_this_cpu;
}

static inline int prio_clamp(int p) {
    if (p < 0) return 0;
    if (p >= (int)NR_PRIORITIES) return (int)NR_PRIORITIES - 1;
    return p;
}

static inline void smp_spin_lock(volatile uint32_t *lock) {
    uint32_t tmp = 1;
    __asm__ volatile (
        "1:\n"
        "xchg %0, %1\n"
        "test %0, %0\n"
        "jnz 1b\n"
        : "+r"(tmp), "+m"(*lock)
        :
        : "memory"
    );
}

static inline void smp_spin_unlock(volatile uint32_t *lock) {
    __asm__ volatile (
        "movl $0, %0\n"
        : "=m"(*lock)
        :
        : "memory"
    );
}

static inline void prio_bitmap_set(struct runqueue *rq, int prio) {
    rq->prio_bitmap[prio >> 5] |= (1U << (prio & 31));
}

static inline void prio_bitmap_clear(struct runqueue *rq, int prio) {
    rq->prio_bitmap[prio >> 5] &= ~(1U << (prio & 31));
}

static inline int prio_find_highest(struct runqueue *rq) {
    for (int i = 0; i < ((int)NR_PRIORITIES + 31) / 32; i++) {
        if (rq->prio_bitmap[i]) {
            return i * 32 + __builtin_ctz(rq->prio_bitmap[i]);
        }
    }
    return -1;
}

/* Caller MUST hold cpu_rq[cpu].lock. */
static void runq_enqueue_locked(struct proces *p) {
    uint32_t cpu = p->cpu;
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    struct runqueue *rq = &cpu_rq[cpu];
    int prio = prio_clamp(p->priority);
    if (p->sched_class == SCHED_REALTIME || p->sched_class == SCHED_CRITICAL) {
        p->run_next = 0;
        p->run_prev = rq->prio_tail[prio];
        if (rq->prio_tail[prio]) rq->prio_tail[prio]->run_next = p;
        else rq->prio_head[prio] = p;
        rq->prio_tail[prio] = p;
    } else {
        struct proces *cur = rq->prio_head[prio];
        struct proces *prev = 0;
        while (cur && cur->vruntime <= p->vruntime) {
            prev = cur;
            cur = cur->run_next;
        }
        p->run_next = cur;
        p->run_prev = prev;
        if (prev) prev->run_next = p;
        else rq->prio_head[prio] = p;
        if (cur) cur->run_prev = p;
        else rq->prio_tail[prio] = p;
    }
    prio_bitmap_set(rq, prio);
    rq->nr_running++;
}

/* Caller MUST hold cpu_rq[cpu].lock. */
static void runq_dequeue_locked(struct proces *p, struct runqueue *rq) {
    int prio = prio_clamp(p->priority);
    if (p->run_prev) p->run_prev->run_next = p->run_next;
    else rq->prio_head[prio] = p->run_next;
    if (p->run_next) p->run_next->run_prev = p->run_prev;
    else rq->prio_tail[prio] = p->run_prev;
    p->run_next = 0;
    p->run_prev = 0;
    if (!rq->prio_head[prio]) prio_bitmap_clear(rq, prio);
    if (rq->nr_running > 0) rq->nr_running--;
}

static void waitq_enqueue_locked(struct proces *p) {
    p->wait_next = 0;
    p->wait_prev = wait_tail;
    if (wait_tail) wait_tail->wait_next = p;
    else wait_head = p;
    wait_tail = p;
}

static void waitq_dequeue_locked(struct proces *p) {
    if (p->wait_prev) p->wait_prev->wait_next = p->wait_next;
    else wait_head = p->wait_next;
    if (p->wait_next) p->wait_next->wait_prev = p->wait_prev;
    else wait_tail = p->wait_prev;
    p->wait_next = 0;
    p->wait_prev = 0;
}

/* Caller MUST hold cpu_rq[cpu].lock. Returns next task for this cpu. */
static struct proces *sched_pick_next_locked(uint32_t cpu) {
    struct runqueue *rq = &cpu_rq[cpu];
    int prio = prio_find_highest(rq);
    if (prio >= 0) {
        struct proces *p = rq->prio_head[prio];
        while (p) {
            if ((p->state == PROCES_READY || p->state == PROCES_RUNNING) && p != current_cpu[cpu]) {
                runq_dequeue_locked(p, rq);
                return p;
            }
            p = p->run_next;
        }
    }

    /* SMP load balancing: steal from the busiest other CPU. */
    uint64_t best = 0;
    uint32_t victim = cpu;
    for (uint32_t i = 0; i < smp_cpu_count; i++) {
        if (i == cpu || !(cpu_online_mask & (1U << i))) continue;
        if (cpu_rq[i].nr_running > best) {
            best = cpu_rq[i].nr_running;
            victim = i;
        }
    }
    if (victim != cpu && best > 1) {
        struct runqueue *vrq = &cpu_rq[victim];
        int vprio = prio_find_highest(vrq);
        if (vprio >= 0) {
            struct proces *p = vrq->prio_head[vprio];
            while (p) {
                if ((p->state == PROCES_READY || p->state == PROCES_RUNNING) && p != idle_task[victim]) {
                    runq_dequeue_locked(p, vrq);
                    p->cpu = cpu;
                    p->nr_migrations++;
                    return p;
                }
                p = p->run_next;
            }
        }
    }
    return idle_task[cpu] ? idle_task[cpu] : 0;
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
    for (uint32_t i = 0; i < SMP_MAX_CPUS; i++) {
        for (uint32_t j = 0; j < ((NR_PRIORITIES + 31) / 32); j++) cpu_rq[i].prio_bitmap[j] = 0;
        for (uint32_t j = 0; j < NR_PRIORITIES; j++) {
            cpu_rq[i].prio_head[j] = 0;
            cpu_rq[i].prio_tail[j] = 0;
        }
        cpu_rq[i].nr_running = 0;
        cpu_rq[i].load = 0;
        cpu_rq[i].lock = 0;
        current_cpu[i] = 0;
        idle_task[i] = 0;
    }
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
        p->is_driver = 0;
        p->driver_handle = 0xFFFFFFFFU;
        p->driver_poll_interval = 0;
        p->driver_last_poll = 0;
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
    wait_head = 0;
    wait_tail = 0;
    waitq_lock = 0;
    current_process = 0;
    next_pid = 1;
    process_count = 0;
    global_ticks = 0;

    asm volatile("mov %%cr3, %0" : "=r"(kernel_page_dir_phys));

    for (uint32_t c = 0; c < smp_cpu_count; c++) {
        char name[16];
        int n = 0;
        while (name[n]) n++;
        (void)name;
        struct proces *idle = proces_stworz(0, "idle", 0);
        if (idle) {
            idle->sched_class = SCHED_IDLE;
            idle->priority = IDLE_PRIORITY;
            idle->static_priority = IDLE_PRIORITY;
            idle->time_slice = 1U;
            idle->state = PROCES_READY;
            idle->cpu = c;
            sched_enqueue(idle);
            idle_task[c] = idle;
        }
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

    uint32_t cpu = cpu_id();

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
    p->cpu = cpu;
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
    sched_enqueue(p);
    process_count++;
    return p;
}

void proces_zakoncz(uint32_t pid) {
    uint32_t cpu = cpu_id();
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        if (p->pid == pid) {
            int was_current = (current_cpu[cpu] == p);
            if (p->state == PROCES_READY) sched_dequeue(p, p->cpu);
            else if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) {
                smp_spin_lock(&waitq_lock);
                waitq_dequeue_locked(p);
                smp_spin_unlock(&waitq_lock);
            }
            p->state = PROCES_TERMINATED;
            p->pid = 0;
            process_count--;
            if (p->page_directory && p->page_directory != (uint32_t *)kernel_page_dir_phys) {
                uint32_t *user_pt = (uint32_t *)(p->page_directory[1] & ~0xFFF);
                uint32_t *fb_pt = (uint32_t *)(p->page_directory[0xFD000000 >> 22] & ~0xFFF);
                paging_free_frame(fb_pt);
                paging_free_frame(user_pt);
                paging_free_frame(p->page_directory);
                p->page_directory = 0;
            }
            if (was_current) {
                current_cpu[cpu] = 0;
                current_process = 0;
                struct proces *next = sched_pick_next(cpu);
                if (next) {
                    current_cpu[cpu] = next;
                    current_process = next;
                    next->state = PROCES_RUNNING;
                    next->cpu = cpu;
                    next->exec_start = global_ticks;
                } else if (idle_task[cpu]) {
                    current_cpu[cpu] = idle_task[cpu];
                    current_process = idle_task[cpu];
                    idle_task[cpu]->state = PROCES_RUNNING;
                }
            }
            return;
        }
    }
}

void proces_tick(void) {
    uint32_t cpu = cpu_id();
    global_ticks++;

    smp_spin_lock(&waitq_lock);
    struct proces *w = wait_head;
    while (w) {
        struct proces *next = w->wait_next;
        if (w->sleep_until && global_ticks >= w->sleep_until) {
            w->sleep_until = 0;
            w->state = PROCES_READY;
            waitq_dequeue_locked(w);
            w->cpu = cpu;
            sched_enqueue(w);
            w->nr_wakeups++;
        }
        w = next;
    }
    smp_spin_unlock(&waitq_lock);

    smp_spin_lock(&cpu_rq[cpu].lock);
    if (current_cpu[cpu] && current_cpu[cpu]->state == PROCES_RUNNING) {
        struct proces *cur = current_cpu[cpu];
        cur->sum_exec_runtime += TICK_NS;
        cur->vruntime += TICK_NS;
        if (cur->time_slice > 0) cur->time_slice--;

        if (cur->time_slice == 0 && cur != idle_task[cpu]) {
            cur->state = PROCES_READY;
            runq_enqueue_locked(cur);
            cur->time_slice = time_slice_for(cur);
        }
    }

    struct proces *next = sched_pick_next_locked(cpu);
    smp_spin_unlock(&cpu_rq[cpu].lock);

    if (next && next != current_cpu[cpu]) {
        if (current_cpu[cpu] && current_cpu[cpu]->state == PROCES_RUNNING) {
            current_cpu[cpu]->ctx_switches++;
        }
        current_cpu[cpu] = next;
        current_process = next;
        next->state = PROCES_RUNNING;
        next->cpu = cpu;
        next->exec_start = global_ticks;
    } else if (!next || next == idle_task[cpu]) {
        if (current_cpu[cpu] && current_cpu[cpu]->state == PROCES_RUNNING && current_cpu[cpu] != idle_task[cpu]) {
            current_cpu[cpu]->ctx_switches++;
        }
        current_cpu[cpu] = idle_task[cpu] ? idle_task[cpu] : 0;
        current_process = current_cpu[cpu];
        if (idle_task[cpu]) idle_task[cpu]->state = PROCES_RUNNING;
    }
}

struct proces *proces_aktualny(void) {
    return current_cpu[cpu_id()];
}

void proces_yield(void) {
    uint32_t cpu = cpu_id();
    if (!current_cpu[cpu]) return;
    smp_spin_lock(&cpu_rq[cpu].lock);
    if (current_cpu[cpu]->state == PROCES_RUNNING) {
        current_cpu[cpu]->state = PROCES_READY;
        runq_enqueue_locked(current_cpu[cpu]);
        current_cpu[cpu]->time_slice = time_slice_for(current_cpu[cpu]);
    }
    struct proces *next = sched_pick_next_locked(cpu);
    smp_spin_unlock(&cpu_rq[cpu].lock);

    if (next && next != current_cpu[cpu]) {
        current_cpu[cpu]->ctx_switches++;
        current_cpu[cpu] = next;
        current_process = next;
        next->state = PROCES_RUNNING;
        next->cpu = cpu;
        next->exec_start = global_ticks;
    }
}

uint32_t proces_pobierz_pid(void) {
    struct proces *p = current_cpu[cpu_id()];
    if (p) return p->pid;
    return 0;
}

void proces_sleep_until(uint64_t ticks) {
    uint32_t cpu = cpu_id();
    if (!current_cpu[cpu]) return;
    smp_spin_lock(&cpu_rq[cpu].lock);
    current_cpu[cpu]->state = PROCES_SLEEPING;
    current_cpu[cpu]->sleep_until = ticks;
    current_cpu[cpu]->sleep_ticks++;
    smp_spin_unlock(&cpu_rq[cpu].lock);

    smp_spin_lock(&waitq_lock);
    waitq_enqueue_locked(current_cpu[cpu]);
    smp_spin_unlock(&waitq_lock);

    struct proces *next = sched_pick_next(cpu);
    if (next && next != current_cpu[cpu]) {
        current_cpu[cpu]->ctx_switches++;
        current_cpu[cpu] = next;
        current_process = next;
        next->state = PROCES_RUNNING;
        next->cpu = cpu;
        next->exec_start = global_ticks;
    } else {
        current_cpu[cpu] = idle_task[cpu] ? idle_task[cpu] : 0;
        current_process = current_cpu[cpu];
        if (idle_task[cpu]) idle_task[cpu]->state = PROCES_RUNNING;
    }
}

void proces_block(void) {
    uint32_t cpu = cpu_id();
    if (!current_cpu[cpu]) return;
    smp_spin_lock(&cpu_rq[cpu].lock);
    current_cpu[cpu]->state = PROCES_BLOCKED;
    current_cpu[cpu]->nr_blocked++;
    smp_spin_unlock(&cpu_rq[cpu].lock);

    smp_spin_lock(&waitq_lock);
    waitq_enqueue_locked(current_cpu[cpu]);
    smp_spin_unlock(&waitq_lock);

    struct proces *next = sched_pick_next(cpu);
    if (next && next != current_cpu[cpu]) {
        current_cpu[cpu]->ctx_switches++;
        current_cpu[cpu] = next;
        current_process = next;
        next->state = PROCES_RUNNING;
        next->cpu = cpu;
        next->exec_start = global_ticks;
    } else {
        current_cpu[cpu] = idle_task[cpu] ? idle_task[cpu] : 0;
        current_process = current_cpu[cpu];
        if (idle_task[cpu]) idle_task[cpu]->state = PROCES_RUNNING;
    }
}

void proces_wakeup(struct proces *p) {
    if (!p) return;
    smp_spin_lock(&waitq_lock);
    if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) {
        waitq_dequeue_locked(p);
        p->sleep_until = 0;
        p->state = PROCES_READY;
        p->nr_wakeups++;
        uint32_t cpu = cpu_id();
        if (p->cpu != cpu && current_cpu[cpu] == idle_task[cpu]) {
            p->cpu = cpu;
            p->nr_migrations++;
        }
        sched_enqueue(p);
    }
    smp_spin_unlock(&waitq_lock);
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
        sched_dequeue(p, p->cpu);
        sched_enqueue(p);
    }
    return old;
}

int proces_set_sched_class(uint32_t pid, sched_class_t cls) {
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    p->sched_class = cls;
    p->time_slice = time_slice_for(p);
    if (p->state == PROCES_READY) {
        sched_dequeue(p, p->cpu);
        sched_enqueue(p);
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
    uint32_t *new_dir = (uint32_t *)paging_alloc_frame();
    if (!new_dir) return 0;
    uint32_t *kernel_dir = (uint32_t *)kernel_page_dir_phys;

    for (int i = 0; i < 1024; i++) new_dir[i] = 0;

    for (int i = 0; i < 256; i++) new_dir[i] = kernel_dir[i];

    uint32_t *user_pt = (uint32_t *)paging_alloc_frame();
    if (!user_pt) { paging_free_frame(new_dir); return 0; }
    for (int i = 0; i < 1024; i++) user_pt[i] = 0;

    for (uint32_t addr = USER_MEM_START; addr < USER_MEM_END; addr += PAGE_SIZE) {
        map_page(user_pt, addr, addr, PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);
    }

    new_dir[1] = ((uint32_t)user_pt | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER);

    uint32_t *fb_pt = (uint32_t *)paging_alloc_frame();
    if (!fb_pt) { paging_free_frame(user_pt); paging_free_frame(new_dir); return 0; }
    for (int i = 0; i < 1024; i++) fb_pt[i] = 0;
    for (int i = 0; i < 1024; i++) {
        map_page(fb_pt, 0xFD000000 + i * PAGE_SIZE, 0xFD000000 + i * PAGE_SIZE, PAGE_PRESENT | PAGE_READWRITE);
    }
    new_dir[0xFD000000 >> 22] = ((uint32_t)fb_pt | PAGE_PRESENT | PAGE_READWRITE);

    return new_dir;
}

uint32_t proces_get_kernel_page_dir(void)
{
    return kernel_page_dir_phys;
}

struct proces *proces_stworz_sterownik(uint32_t entry_point, const char *name, uint32_t arg, uint32_t poll_interval)
{
    if (process_count >= MAX_PROCESSES) return 0;
    if (driver_stack_count >= 64) return 0;

    struct proces *p = proces_alloc_slot();
    if (!p) return 0;

    uint32_t cpu = cpu_id();

    p->pid = next_pid++;
    p->state = PROCES_READY;
    p->entry_point = entry_point;
    p->arg = arg;
    p->window = 0;
    p->page_directory = (uint32_t *)kernel_page_dir_phys;
    p->sched_class = SCHED_CRITICAL;
    p->priority = 10;
    p->static_priority = 10;
    p->time_slice = 4U;
    p->vruntime = 0;
    p->sum_exec_runtime = 0;
    p->exec_start = global_ticks;
    p->wait_start = 0;
    p->sleep_until = 0;
    p->period = 0;
    p->deadline = 0;
    p->cpu = cpu;
    p->preempt_count = 0;
    p->prio_inherited = 0;
    p->original_prio = p->priority;
    p->policy = 0;
    p->nr_wakeups = 0;
    p->nr_migrations = 0;
    p->nr_blocked = 0;
    p->sleep_ticks = 0;
    p->ctx_switches = 0;
    p->is_driver = 1;
    p->driver_handle = 0xFFFFFFFFU;
    p->driver_poll_interval = poll_interval;
    p->driver_last_poll = 0;

    for (uint32_t i = 0; i < 31 && name && name[i]; i++) {
        p->name[i] = name[i];
    }
    p->name[31] = 0;
    if (!name) p->name[0] = 0;

    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }

    p->kernel_stack = (uint32_t)&driver_kernel_stacks[driver_stack_count][0] + 4096U - 16U;
    driver_stack_count++;

    uint32_t *stack = (uint32_t *)p->kernel_stack;
    stack--;
    *stack = GDT_RING0_DATA;
    stack--;
    *stack = 0U;
    stack--;
    *stack = 0x202U;
    stack--;
    *stack = GDT_RING0_CODE;
    stack--;
    *stack = entry_point;

    p->regs.esp = (uint32_t)(stack + 4);
    p->regs.ebp = (uint32_t)(stack + 4);
    p->regs.eax = arg;
    p->regs.ebx = 0;
    p->regs.ecx = 0;
    p->regs.edx = 0;
    p->regs.esi = 0;
    p->regs.edi = 0;
    p->regs.eflags = 0x202U;
    p->regs.cs = GDT_RING0_CODE;
    p->regs.ds = GDT_RING0_DATA;
    p->regs.es = GDT_RING0_DATA;
    p->regs.fs = GDT_RING0_DATA;
    p->regs.gs = GDT_RING0_DATA;
    p->regs.ss = GDT_RING0_DATA;
    p->regs.eip = entry_point;

    sched_enqueue(p);
    process_count++;
    return p;
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
                sched_enqueue(p);
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
    uint32_t cpu = cpu_id();
    p->pid = next_pid++;
    p->state = PROCES_READY;
    p->window = okno;
    p->entry_point = 0;
    p->arg = 0;
    p->kernel_stack = 0;
    p->user_stack = 0;
    p->page_directory = 0;
    p->cpu = cpu;
    if (nazwa) {
        for (uint32_t i = 0; i < 31 && nazwa[i]; i++) p->name[i] = nazwa[i];
    }
    p->name[31] = 0;
    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }
    process_count++;
    sched_enqueue(p);
}

void sched_enqueue(struct proces *p) {
    uint32_t cpu = p->cpu;
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    smp_spin_lock(&cpu_rq[cpu].lock);
    runq_enqueue_locked(p);
    smp_spin_unlock(&cpu_rq[cpu].lock);
}

void sched_dequeue(struct proces *p, uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    smp_spin_lock(&cpu_rq[cpu].lock);
    runq_dequeue_locked(p, &cpu_rq[cpu]);
    smp_spin_unlock(&cpu_rq[cpu].lock);
}

struct proces *sched_pick_next(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    smp_spin_lock(&cpu_rq[cpu].lock);
    struct proces *next = sched_pick_next_locked(cpu);
    smp_spin_unlock(&cpu_rq[cpu].lock);
    return next;
}
