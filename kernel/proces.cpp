/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/proces.h>
#include <kernel/timer.h>
#include <kernel/idt.h>
#include <include/printf/printf.h>

struct proces process_table[MAX_PROCESSES];
struct proces *current_cpu[SMP_MAX_CPUS] = {0};
struct proces *current_process = 0;

static uint32_t next_pid = 1;
static uint32_t process_count = 0;
static uint32_t kernel_page_dir_phys = 0;
static uint64_t global_ticks = 0;
uint32_t smp_cpu_count = 1;
uint32_t cpu_online_mask = 1;
uint32_t smp_this_cpu = 0;

static uint32_t driver_kernel_stacks[64][1024] __attribute__((aligned(16)));
static uint32_t driver_stack_count = 0;

struct runqueue {
    uint32_t prio_bitmap[(NR_PRIORITIES + 31) / 32];
    struct proces *prio_head[NR_PRIORITIES];
    struct proces *prio_tail[NR_PRIORITIES];
    uint64_t nr_running;
    uint64_t load;
    uint32_t lock;
};

static struct runqueue cpu_rq[SMP_MAX_CPUS];

static struct proces *wait_head = 0;
static struct proces *wait_tail = 0;
static struct proces *sleep_head = 0;
static struct proces *sleep_tail = 0;
static uint32_t waitq_lock = 0;
static struct proces *idle_task[SMP_MAX_CPUS] = {0};

uint32_t cpu_id(void) {
    return smp_this_cpu;
}

static inline int prio_clamp(int p) {
    if (p < 0) return 0;
    if (p >= (int)NR_PRIORITIES) return (int)NR_PRIORITIES - 1;
    return p;
}

static inline uint32_t irq_save(void) {
    uint32_t f;
    __asm__ volatile("pushf; pop %0; cli" : "=r"(f) :: "memory");
    return f;
}

static inline void irq_restore(uint32_t f) {
    if (f & 0x200U) __asm__ volatile("sti" ::: "memory");
}

static inline uint32_t lock_irqsave(volatile uint32_t *l) {
    uint32_t f = irq_save();
    while (__atomic_exchange_n(l, 1U, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(l, __ATOMIC_RELAXED)) __builtin_ia32_pause();
    }
    return f;
}

static inline void unlock_raw(volatile uint32_t *l) {
    __atomic_store_n(l, 0U, __ATOMIC_RELEASE);
}

static inline void unlock_irqrestore(volatile uint32_t *l, uint32_t f) {
    unlock_raw(l);
    irq_restore(f);
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

static void runq_enqueue_locked(struct proces *p) {
    uint32_t cpu = p->cpu;
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    struct runqueue *rq = &cpu_rq[cpu];
    int prio = prio_clamp(p->priority);

    struct proces *after = rq->prio_tail[prio];
    if (p->sched_class != SCHED_REALTIME && p->sched_class != SCHED_CRITICAL) {
        while (after && after->vruntime > p->vruntime) after = after->run_prev;
    }
    p->run_prev = after;
    p->run_next = after ? after->run_next : rq->prio_head[prio];
    if (after) after->run_next = p; else rq->prio_head[prio] = p;
    if (p->run_next) p->run_next->run_prev = p; else rq->prio_tail[prio] = p;

    prio_bitmap_set(rq, prio);
    rq->nr_running++;
}

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

static inline struct proces *rq_pop_best_locked(struct runqueue *rq) {
    int prio = prio_find_highest(rq);
    if (prio < 0) return 0;
    struct proces *p = rq->prio_head[prio];
    runq_dequeue_locked(p, rq);
    return p;
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

static void sleep_insert_locked(struct proces *p) {
    struct proces *after = sleep_tail;
    while (after && after->sleep_until > p->sleep_until) after = after->wait_prev;
    p->wait_prev = after;
    p->wait_next = after ? after->wait_next : sleep_head;
    if (after) after->wait_next = p; else sleep_head = p;
    if (p->wait_next) p->wait_next->wait_prev = p; else sleep_tail = p;
}

static void sleep_remove_locked(struct proces *p) {
    if (p->wait_prev) p->wait_prev->wait_next = p->wait_next;
    else sleep_head = p->wait_next;
    if (p->wait_next) p->wait_next->wait_prev = p->wait_prev;
    else sleep_tail = p->wait_prev;
    p->wait_next = 0;
    p->wait_prev = 0;
}

static inline void wait_remove_any_locked(struct proces *p) {
    if (p->state == PROCES_SLEEPING) sleep_remove_locked(p);
    else waitq_dequeue_locked(p);
}

static struct proces *sched_pick_next_locked(uint32_t cpu) {
    return rq_pop_best_locked(&cpu_rq[cpu]);
}

static struct proces *steal_work(uint32_t cpu) {
    uint32_t victim = cpu;
    uint64_t best = 1;
    for (uint32_t i = 0; i < smp_cpu_count; i++) {
        if (i == cpu || !(cpu_online_mask & (1U << i))) continue;
        uint64_t n = cpu_rq[i].nr_running;
        if (n > best) {
            best = n;
            victim = i;
        }
    }
    if (victim == cpu) return 0;
    struct runqueue *v = &cpu_rq[victim];
    if (__atomic_exchange_n(&v->lock, 1U, __ATOMIC_ACQUIRE)) return 0;
    struct proces *p = rq_pop_best_locked(v);
    unlock_raw(&v->lock);
    if (p) {
        p->cpu = cpu;
        p->nr_migrations++;
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

static void schedule_cpu(uint32_t cpu, int yield) {
    struct runqueue *rq = &cpu_rq[cpu];
    struct proces *cur = current_cpu[cpu];
    struct proces *idle = idle_task[cpu];

    int cur_runnable = cur && cur->state == PROCES_RUNNING;
    int cur_is_idle = cur && cur == idle;
    int expired = yield || (cur_runnable && cur->time_slice == 0);

    while (__atomic_exchange_n(&rq->lock, 1U, __ATOMIC_ACQUIRE)) {
        while (__atomic_load_n(&rq->lock, __ATOMIC_RELAXED)) __builtin_ia32_pause();
    }

    int top = prio_find_highest(rq);
    struct proces *next = 0;

    if (top >= 0) {
        if (!cur_runnable || cur_is_idle) {
            next = rq_pop_best_locked(rq);
        } else {
            int cp = prio_clamp(cur->priority);
            if (top < cp || (expired && top == cp)) next = rq_pop_best_locked(rq);
        }
    }

    if (cur_runnable && expired && cur->time_slice == 0) cur->time_slice = time_slice_for(cur);

    if (next && cur_runnable && !cur_is_idle) {
        cur->state = PROCES_READY;
        runq_enqueue_locked(cur);
    }
    unlock_raw(&rq->lock);

    if (!next && (!cur_runnable || cur_is_idle)) {
        next = steal_work(cpu);
        if (!next && !cur_is_idle) next = idle;
    }

    if (next && next != cur) {
        if (cur) cur->ctx_switches++;
        current_cpu[cpu] = next;
        current_process = next;
        next->cpu = cpu;
        next->exec_start = global_ticks;
    }
    if (next) next->state = PROCES_RUNNING;
}

static struct proces *proces_alloc_slot(void) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCES_TERMINATED || process_table[i].pid == 0) {
            return &process_table[i];
        }
    }
    return 0;
}

struct proces *proces_find(uint32_t pid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) return &process_table[i];
    }
    return 0;
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
    sleep_head = 0;
    sleep_tail = 0;
    waitq_lock = 0;
    current_process = 0;
    next_pid = 1;
    process_count = 0;
    global_ticks = 0;

    asm volatile("mov %%cr3, %0" : "=r"(kernel_page_dir_phys));

    for (uint32_t c = 0; c < smp_cpu_count; c++) {
        struct proces *idle = proces_stworz(0, "idle", 0);
        if (idle) {
            sched_dequeue(idle, idle->cpu);
            idle->sched_class = SCHED_IDLE;
            idle->priority = IDLE_PRIORITY;
            idle->static_priority = IDLE_PRIORITY;
            idle->time_slice = 1U;
            idle->state = PROCES_READY;
            idle->cpu = c;
            idle_task[c] = idle;
        }
    }
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
    if (!pid) return;
    uint32_t cpu = cpu_id();
    uint32_t f = irq_save();
    struct proces *p = proces_find(pid);
    if (!p) {
        irq_restore(f);
        return;
    }

    int was_current = (current_cpu[cpu] == p);
    if (p->state == PROCES_READY) {
        sched_dequeue(p, p->cpu);
    } else if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) {
        lock_irqsave(&waitq_lock);
        wait_remove_any_locked(p);
        unlock_raw(&waitq_lock);
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
    if (was_current) schedule_cpu(cpu, 0);
    irq_restore(f);
}

void proces_tick(void) {
    uint32_t cpu = cpu_id();
    uint32_t f = irq_save();

    if (cpu == 0) global_ticks++;

    if (sleep_head && sleep_head->sleep_until <= global_ticks) {
        uint32_t wf = lock_irqsave(&waitq_lock);
        while (sleep_head && sleep_head->sleep_until <= global_ticks) {
            struct proces *w = sleep_head;
            sleep_remove_locked(w);
            w->sleep_until = 0;
            w->state = PROCES_READY;
            w->nr_wakeups++;
            sched_enqueue(w);
        }
        unlock_irqrestore(&waitq_lock, wf);
    }

    struct proces *cur = current_cpu[cpu];
    if (cur && cur->state == PROCES_RUNNING) {
        cur->sum_exec_runtime += TICK_NS;
        cur->vruntime += TICK_NS;
        if (cur->time_slice > 0) cur->time_slice--;
    }

    schedule_cpu(cpu, 0);
    irq_restore(f);
}

struct proces *proces_aktualny(void) {
    return current_cpu[cpu_id()];
}

void proces_yield(void) {
    uint32_t cpu = cpu_id();
    uint32_t f = irq_save();
    if (current_cpu[cpu]) schedule_cpu(cpu, 1);
    irq_restore(f);
}

uint32_t proces_pobierz_pid(void) {
    struct proces *p = current_cpu[cpu_id()];
    if (p) return p->pid;
    return 0;
}

void proces_sleep_until(uint64_t ticks) {
    uint32_t cpu = cpu_id();
    uint32_t f = irq_save();
    struct proces *cur = current_cpu[cpu];
    if (!cur || cur == idle_task[cpu] || ticks <= global_ticks) {
        irq_restore(f);
        return;
    }

    lock_irqsave(&waitq_lock);
    cur->state = PROCES_SLEEPING;
    cur->sleep_until = ticks;
    cur->sleep_ticks++;
    sleep_insert_locked(cur);
    unlock_raw(&waitq_lock);

    schedule_cpu(cpu, 0);
    irq_restore(f);
}

void proces_block(void) {
    uint32_t cpu = cpu_id();
    uint32_t f = irq_save();
    struct proces *cur = current_cpu[cpu];
    if (!cur || cur == idle_task[cpu]) {
        irq_restore(f);
        return;
    }

    lock_irqsave(&waitq_lock);
    cur->state = PROCES_BLOCKED;
    cur->nr_blocked++;
    waitq_enqueue_locked(cur);
    unlock_raw(&waitq_lock);

    schedule_cpu(cpu, 0);
    irq_restore(f);
}

void proces_wakeup(struct proces *p) {
    if (!p) return;
    uint32_t f = lock_irqsave(&waitq_lock);
    if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) {
        wait_remove_any_locked(p);
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
    unlock_irqrestore(&waitq_lock, f);
}

int proces_set_priority(uint32_t pid, int32_t priority) {
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    uint32_t f = irq_save();
    int queued = (p->state == PROCES_READY);
    if (queued) sched_dequeue(p, p->cpu);
    int old = p->priority;
    p->priority = priority;
    p->original_prio = priority;
    p->time_slice = time_slice_for(p);
    if (queued) sched_enqueue(p);
    irq_restore(f);
    return old;
}

int proces_set_sched_class(uint32_t pid, sched_class_t cls) {
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    uint32_t f = irq_save();
    int queued = (p->state == PROCES_READY);
    if (queued) sched_dequeue(p, p->cpu);
    p->sched_class = cls;
    p->time_slice = time_slice_for(p);
    if (queued) sched_enqueue(p);
    irq_restore(f);
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
    uint32_t f = lock_irqsave(&cpu_rq[cpu].lock);
    runq_enqueue_locked(p);
    unlock_irqrestore(&cpu_rq[cpu].lock, f);
}

void sched_dequeue(struct proces *p, uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    uint32_t f = lock_irqsave(&cpu_rq[cpu].lock);
    runq_dequeue_locked(p, &cpu_rq[cpu]);
    unlock_irqrestore(&cpu_rq[cpu].lock, f);
}

struct proces *sched_pick_next(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    uint32_t f = lock_irqsave(&cpu_rq[cpu].lock);
    struct proces *next = sched_pick_next_locked(cpu);
    unlock_irqrestore(&cpu_rq[cpu].lock, f);
    return next;
}