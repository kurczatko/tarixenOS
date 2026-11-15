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

#define SCHED_CPU_PERF 0U
#define SCHED_CPU_EFF 1U
#define SCHED_CAP_PERF 1024U
#define SCHED_CAP_EFF 512U
#define SCHED_BALANCE_INTERVAL 8U
#define SCHED_ENERGY_INTERVAL 64U
#define SCHED_PARK_IDLE_TICKS 200U
#define SCHED_PERF_DEMAND_UTIL 300U
#define SCHED_EFF_TASK_UTIL 160U
#define SCHED_NEW_TASK_UTIL 256U
#define SCHED_PULL_SCAN_LIMIT 16U

void sched_enqueue(struct proces *p);
void sched_dequeue(struct proces *p, uint32_t cpu);
extern void smp_send_resched_ipi(uint32_t cpu) __attribute__((weak));

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

struct cpu_meta_t {
    uint32_t type;
    uint32_t capacity;
    uint32_t util;
    uint32_t idle_ticks;
};

struct proces_ext {
    uint32_t affinity;
    uint32_t util;
    uint64_t util_stamp;
};

static struct runqueue cpu_rq[SMP_MAX_CPUS];
static struct cpu_meta_t cpu_meta[SMP_MAX_CPUS];
static struct proces_ext proc_ext[MAX_PROCESSES];
static uint32_t cpu_parked_mask = 0;

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

static inline struct proces_ext *ext_of(struct proces *p) {
    return &proc_ext[p - process_table];
}

static inline uint32_t cap_of(uint32_t c) {
    return cpu_meta[c].capacity ? cpu_meta[c].capacity : SCHED_CAP_PERF;
}

static inline uint32_t mask_cpus(void) {
    return smp_cpu_count >= 32U ? 0xFFFFFFFFU : ((1U << smp_cpu_count) - 1U);
}

static inline uint32_t usable_mask(void) {
    uint32_t m = cpu_online_mask & ~cpu_parked_mask & mask_cpus();
    return m ? m : 1U;
}

static inline int task_wants_perf(struct proces *p) {
    return p->sched_class == SCHED_REALTIME || p->sched_class == SCHED_CRITICAL || p->sched_class == SCHED_INTERACTIVE;
}

static inline int task_wants_eff(struct proces *p) {
    if (p->sched_class == SCHED_IDLE || p->sched_class == SCHED_BACKGROUND) return 1;
    if (p->sched_class == SCHED_NORMAL && ext_of(p)->util < SCHED_EFF_TASK_UTIL) return 1;
    return 0;
}

static inline int fits_cpu(struct proces *p, uint32_t c) {
    return ext_of(p)->util * 1280U <= cap_of(c) * 1024U;
}

static uint32_t cpu_load_norm(uint32_t c) {
    uint32_t n = (uint32_t)cpu_rq[c].nr_running;
    if (n > 1023U) n = 1023U;
    struct proces *cur = current_cpu[c];
    if (cur && cur != idle_task[c] && cur->state == PROCES_RUNNING) n++;
    return (n * 65536U) / cap_of(c);
}

static void util_decay(struct proces *p) {
    struct proces_ext *e = ext_of(p);
    uint64_t dt64 = global_ticks - e->util_stamp;
    uint32_t dt = dt64 > 192ULL ? 192U : (uint32_t)dt64;
    while (dt && e->util) {
        e->util -= e->util >> 5;
        dt--;
    }
    if (e->util < 16U) e->util = 0;
    e->util_stamp = global_ticks;
}

static uint32_t select_cpu(struct proces *p) {
    struct proces_ext *e = ext_of(p);
    uint32_t usable = usable_mask();
    uint32_t allowed = e->affinity & usable;
    if (!allowed) allowed = e->affinity & cpu_online_mask & mask_cpus();
    if (!allowed) allowed = usable;

    int want_perf = task_wants_perf(p);
    int want_eff = !want_perf && task_wants_eff(p);
    uint32_t best_cpu = 0;
    uint32_t best_cost = 0xFFFFFFFFU;

    for (uint32_t c = 0; c < smp_cpu_count && c < SMP_MAX_CPUS; c++) {
        if (!(allowed & (1U << c))) continue;
        uint32_t cost = cpu_load_norm(c);
        if (!fits_cpu(p, c)) cost += 1000000U;
        if (cpu_meta[c].type == SCHED_CPU_PERF) {
            if (want_eff) cost += 300U;
            else if (!want_perf) cost += 20U;
        } else if (want_perf) {
            cost += 500000U;
        }
        if (c == p->cpu && cost >= 30U) cost -= 30U;
        if (cost < best_cost) {
            best_cost = cost;
            best_cpu = c;
        }
    }
    if (cpu_parked_mask & (1U << best_cpu)) {
        __atomic_fetch_and(&cpu_parked_mask, ~(1U << best_cpu), __ATOMIC_SEQ_CST);
    }
    return best_cpu;
}

static int pullable(struct proces *p, uint32_t victim, uint32_t dest, int misfit_only) {
    struct proces_ext *e = ext_of(p);
    if (!((e->affinity >> dest) & 1U)) return 0;
    if (misfit_only) return !fits_cpu(p, victim) && fits_cpu(p, dest);
    if (!fits_cpu(p, dest) && fits_cpu(p, victim)) return 0;
    if (task_wants_perf(p) && cpu_meta[dest].type != SCHED_CPU_PERF) return 0;
    if (cpu_meta[dest].type == SCHED_CPU_PERF && cpu_meta[victim].type == SCHED_CPU_EFF && task_wants_eff(p)) return 0;
    return 1;
}

static struct proces *pull_from(uint32_t victim, uint32_t dest, int misfit_only) {
    struct runqueue *v = &cpu_rq[victim];
    if (!v->nr_running) return 0;
    if (__atomic_exchange_n(&v->lock, 1U, __ATOMIC_ACQUIRE)) return 0;

    struct proces *found = 0;
    uint32_t scanned = 0;
    int words = ((int)NR_PRIORITIES + 31) / 32;
    for (int w = 0; w < words && !found && scanned < SCHED_PULL_SCAN_LIMIT; w++) {
        uint32_t bits = v->prio_bitmap[w];
        while (bits && !found && scanned < SCHED_PULL_SCAN_LIMIT) {
            int b = __builtin_ctz(bits);
            bits &= bits - 1U;
            struct proces *p = v->prio_head[w * 32 + b];
            while (p && scanned < SCHED_PULL_SCAN_LIMIT) {
                scanned++;
                if (pullable(p, victim, dest, misfit_only)) {
                    found = p;
                    break;
                }
                p = p->run_next;
            }
        }
    }
    if (found) runq_dequeue_locked(found, v);
    unlock_raw(&v->lock);
    if (found) {
        found->cpu = dest;
        found->nr_migrations++;
    }
    return found;
}

static struct proces *balance_pull(uint32_t cpu) {
    uint32_t usable = usable_mask();
    if (!((usable >> cpu) & 1U)) return 0;

    if (cpu_meta[cpu].type == SCHED_CPU_PERF) {
        for (uint32_t i = 0; i < smp_cpu_count && i < SMP_MAX_CPUS; i++) {
            if (i == cpu || !((usable >> i) & 1U)) continue;
            if (cpu_meta[i].type != SCHED_CPU_EFF) continue;
            struct proces *m = pull_from(i, cpu, 1);
            if (m) return m;
        }
    }

    uint32_t mine = cpu_load_norm(cpu);
    uint32_t unit = 65536U / cap_of(cpu);
    uint32_t best = mine + unit;
    uint32_t victim = cpu;
    for (uint32_t i = 0; i < smp_cpu_count && i < SMP_MAX_CPUS; i++) {
        if (i == cpu || !((usable >> i) & 1U)) continue;
        if (!cpu_rq[i].nr_running) continue;
        uint32_t l = cpu_load_norm(i);
        if (l > best) {
            best = l;
            victim = i;
        }
    }
    if (victim == cpu) return 0;
    return pull_from(victim, cpu, 0);
}

static void drain_cpu(uint32_t c) {
    struct runqueue *rq = &cpu_rq[c];
    uint32_t n = (uint32_t)rq->nr_running;
    while (n--) {
        uint32_t f = lock_irqsave(&rq->lock);
        struct proces *p = rq_pop_best_locked(rq);
        unlock_irqrestore(&rq->lock, f);
        if (!p) break;
        p->cpu = select_cpu(p);
        p->nr_migrations++;
        sched_enqueue(p);
    }
}

static void energy_manage(void) {
    uint32_t total = 0;
    int perf_demand = 0;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        if (!p->pid || p->sched_class == SCHED_IDLE) continue;
        if (p->state != PROCES_READY && p->state != PROCES_RUNNING) continue;
        uint32_t u = proc_ext[i].util;
        total += u;
        if (task_wants_perf(p) && u >= SCHED_PERF_DEMAND_UTIL) perf_demand = 1;
    }
    uint32_t need = total + (total >> 2);
    uint32_t online = cpu_online_mask & mask_cpus();
    uint32_t desired = 1U;
    uint32_t cap = cap_of(0);

    for (uint32_t pass = 0; pass < 2 && cap < need; pass++) {
        uint32_t want_type = pass == 0 ? SCHED_CPU_EFF : SCHED_CPU_PERF;
        for (uint32_t c = 1; c < smp_cpu_count && c < SMP_MAX_CPUS && cap < need; c++) {
            if (!((online >> c) & 1U) || ((desired >> c) & 1U)) continue;
            if (cpu_meta[c].type != want_type) continue;
            desired |= (1U << c);
            cap += cap_of(c);
        }
    }

    if (perf_demand) {
        int have_perf = 0;
        for (uint32_t c = 0; c < smp_cpu_count && c < SMP_MAX_CPUS; c++) {
            if (((desired >> c) & 1U) && cpu_meta[c].type == SCHED_CPU_PERF) have_perf = 1;
        }
        if (!have_perf) {
            for (uint32_t c = 1; c < smp_cpu_count && c < SMP_MAX_CPUS; c++) {
                if (((online >> c) & 1U) && cpu_meta[c].type == SCHED_CPU_PERF) {
                    desired |= (1U << c);
                    break;
                }
            }
        }
    }

    for (uint32_t c = 1; c < smp_cpu_count && c < SMP_MAX_CPUS; c++) {
        uint32_t bit = 1U << c;
        if (!(online & bit)) continue;
        if (desired & bit) {
            if (cpu_parked_mask & bit) __atomic_fetch_and(&cpu_parked_mask, ~bit, __ATOMIC_SEQ_CST);
        } else if (!(cpu_parked_mask & bit) && cpu_meta[c].idle_ticks >= SCHED_PARK_IDLE_TICKS && !cpu_rq[c].nr_running) {
            __atomic_fetch_or(&cpu_parked_mask, bit, __ATOMIC_SEQ_CST);
            drain_cpu(c);
        }
    }
}

static inline void cpuid_raw(uint32_t leaf, uint32_t sub, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    __asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(leaf), "c"(sub));
}

void cpu_set_topology(uint32_t cpu, uint32_t type, uint32_t capacity) {
    if (cpu >= SMP_MAX_CPUS) return;
    cpu_meta[cpu].type = type;
    cpu_meta[cpu].capacity = capacity ? capacity : SCHED_CAP_PERF;
}

void sched_register_this_cpu(void) {
    uint32_t cpu = cpu_id();
    if (cpu >= SMP_MAX_CPUS) return;
    uint32_t a, b, c, d;
    uint32_t type = SCHED_CPU_PERF;
    uint32_t cap = SCHED_CAP_PERF;
    cpuid_raw(0, 0, &a, &b, &c, &d);
    uint32_t maxleaf = a;
    if (maxleaf >= 7U) {
        cpuid_raw(7, 0, &a, &b, &c, &d);
        if (((d >> 15) & 1U) && maxleaf >= 0x1AU) {
            cpuid_raw(0x1A, 0, &a, &b, &c, &d);
            if ((a >> 24) == 0x20U) {
                type = SCHED_CPU_EFF;
                cap = SCHED_CAP_EFF;
            }
        }
    }
    cpu_meta[cpu].type = type;
    cpu_meta[cpu].capacity = cap;
}

uint32_t sched_cpu_util(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return 0;
    return cpu_meta[cpu].util;
}

uint32_t sched_cpu_capacity(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return 0;
    return cap_of(cpu);
}

uint32_t sched_cpu_type(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return SCHED_CPU_PERF;
    return cpu_meta[cpu].type;
}

int sched_cpu_parked(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return 0;
    return (cpu_parked_mask >> cpu) & 1U;
}

uint32_t sched_idle_hint(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return 0;
    if ((cpu_parked_mask >> cpu) & 1U) return 2U;
    if (cpu_meta[cpu].idle_ticks >= 32U) return 1U;
    return 0U;
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

    int requeued = 0;
    if (next && cur_runnable && !cur_is_idle) {
        cur->state = PROCES_READY;
        runq_enqueue_locked(cur);
        requeued = 1;
    }
    unlock_raw(&rq->lock);

    if (requeued && !(((ext_of(cur)->affinity & usable_mask()) >> cpu) & 1U)) {
        sched_dequeue(cur, cpu);
        cur->cpu = select_cpu(cur);
        sched_enqueue(cur);
    }

    if (!next && (!cur_runnable || cur_is_idle)) {
        next = balance_pull(cpu);
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
        cpu_meta[i].util = 0;
        cpu_meta[i].idle_ticks = 0;
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
        proc_ext[i].affinity = 0xFFFFFFFFU;
        proc_ext[i].util = 0;
        proc_ext[i].util_stamp = 0;
    }
    wait_head = 0;
    wait_tail = 0;
    sleep_head = 0;
    sleep_tail = 0;
    waitq_lock = 0;
    cpu_parked_mask = 0;
    current_process = 0;
    next_pid = 1;
    process_count = 0;
    global_ticks = 0;

    asm volatile("mov %%cr3, %0" : "=r"(kernel_page_dir_phys));

    sched_register_this_cpu();

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
            ext_of(idle)->affinity = 1U << c;
            idle_task[c] = idle;
        }
    }
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
    p->cpu = cpu_id();
    p->preempt_count = 0;
    p->prio_inherited = 0;
    p->original_prio = p->priority;
    p->policy = 0;
    p->nr_wakeups = 0;
    p->nr_migrations = 0;
    p->nr_blocked = 0;
    p->sleep_ticks = 0;
    p->ctx_switches = 0;

    struct proces_ext *e = ext_of(p);
    e->affinity = 0xFFFFFFFFU;
    e->util = SCHED_NEW_TASK_UTIL;
    e->util_stamp = global_ticks;

    for (uint32_t i = 0; i < 31 && name && name[i]; i++) {
        p->name[i] = name[i];
    }
    p->name[31] = 0;
    if (!name) p->name[0] = 0;

    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }

    proces_ustaw_stos(p, entry_point, arg);
    p->cpu = select_cpu(p);
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
            util_decay(w);
            uint32_t target = select_cpu(w);
            if (target != w->cpu) w->nr_migrations++;
            w->cpu = target;
            sched_enqueue(w);
        }
        unlock_irqrestore(&waitq_lock, wf);
    }

    struct proces *cur = current_cpu[cpu];
    int busy = cur && cur != idle_task[cpu] && cur->state == PROCES_RUNNING;
    struct cpu_meta_t *m = &cpu_meta[cpu];
    if (busy) {
        m->util += (1024U - m->util) >> 4;
        m->idle_ticks = 0;
    } else {
        m->util -= m->util >> 4;
        m->idle_ticks++;
    }

    if (cur && cur->state == PROCES_RUNNING) {
        cur->sum_exec_runtime += TICK_NS;
        cur->vruntime += TICK_NS;
        if (cur->time_slice > 0) cur->time_slice--;
        if (busy) {
            struct proces_ext *e = ext_of(cur);
            e->util += (1024U - e->util) >> 5;
            e->util_stamp = global_ticks;
        }
    }

    if (cpu == 0 && (((uint32_t)global_ticks) & (SCHED_ENERGY_INTERVAL - 1U)) == 0U) energy_manage();

    if (busy && ((((uint32_t)global_ticks) + cpu) & (SCHED_BALANCE_INTERVAL - 1U)) == 0U) {
        struct proces *pulled = balance_pull(cpu);
        if (pulled) sched_enqueue(pulled);
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
        util_decay(p);
        uint32_t target = select_cpu(p);
        if (target != p->cpu) p->nr_migrations++;
        p->cpu = target;
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
    if (queued) {
        p->cpu = select_cpu(p);
        sched_enqueue(p);
    }
    irq_restore(f);
    return 0;
}

int proces_set_affinity(uint32_t pid, uint32_t mask) {
    mask &= mask_cpus();
    if (!mask) return -1;
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    uint32_t f = irq_save();
    ext_of(p)->affinity = mask;
    if (p->state == PROCES_READY && !((mask >> p->cpu) & 1U)) {
        sched_dequeue(p, p->cpu);
        p->cpu = select_cpu(p);
        p->nr_migrations++;
        sched_enqueue(p);
    }
    irq_restore(f);
    return 0;
}

uint32_t proces_get_affinity(uint32_t pid) {
    struct proces *p = proces_find(pid);
    if (!p) return 0;
    return ext_of(p)->affinity;
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
    p->cpu = cpu_id();
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

    struct proces_ext *e = ext_of(p);
    e->affinity = 0xFFFFFFFFU;
    e->util = SCHED_NEW_TASK_UTIL;
    e->util_stamp = global_ticks;

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

    p->cpu = select_cpu(p);
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
    p->pid = next_pid++;
    p->state = PROCES_READY;
    p->window = okno;
    p->entry_point = 0;
    p->arg = 0;
    p->kernel_stack = 0;
    p->user_stack = 0;
    p->page_directory = 0;
    p->cpu = cpu_id();
    if (nazwa) {
        for (uint32_t i = 0; i < 31 && nazwa[i]; i++) p->name[i] = nazwa[i];
    }
    p->name[31] = 0;
    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) {
        p->okna_uzyte[i] = 0;
    }
    struct proces_ext *e = ext_of(p);
    e->affinity = 0xFFFFFFFFU;
    e->util = SCHED_NEW_TASK_UTIL;
    e->util_stamp = global_ticks;
    p->cpu = select_cpu(p);
    process_count++;
    sched_enqueue(p);
}

void sched_enqueue(struct proces *p) {
    uint32_t cpu = p->cpu;
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    if (!((usable_mask() >> cpu) & 1U)) {
        p->cpu = select_cpu(p);
        cpu = p->cpu;
    }
    uint32_t f = lock_irqsave(&cpu_rq[cpu].lock);
    runq_enqueue_locked(p);
    unlock_irqrestore(&cpu_rq[cpu].lock, f);
    if (smp_send_resched_ipi && idle_task[cpu] && cpu != cpu_id() && current_cpu[cpu] == idle_task[cpu]) {
        smp_send_resched_ipi(cpu);
    }
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