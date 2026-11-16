/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/proces.h>
#include <kernel/timer.h>
#include <kernel/idt.h>

/* ------------------------------------------------------------------------- */
/* Stale                                                                     */
/* ------------------------------------------------------------------------- */

#define SCHED_CPU_PERF 0U
#define SCHED_CPU_EFF 1U
#define SCHED_CAP_PERF 1024U
#define SCHED_CAP_EFF 512U
#define SCHED_BALANCE_INTERVAL 8U          /* potega dwojki */
#define SCHED_ENERGY_INTERVAL 64U          /* potega dwojki */
#define SCHED_PARK_IDLE_TICKS 200U
#define SCHED_PERF_DEMAND_UTIL 300U
#define SCHED_EFF_TASK_UTIL 160U
#define SCHED_DOWNMIGRATE_UTIL 96U         /* histereza: zejscie na E-core tylko dla brdzo malych zadan */
#define SCHED_NEW_TASK_UTIL 256U
#define SCHED_PULL_SCAN_LIMIT 16U

#define SCHED_MIN_WEIGHT 32U               /* waga zadania o util=0 (zeby liczyly sie tez lekkie) */
#define SCHED_IMBALANCE_MARGIN 256U        /* o ile cudze obciazenie musi byc wieksze, by ciagnac */
#define SCHED_WAKE_PARKED_COST 768U        /* koszt obudzenia zaparkowanego rdzenia w select_cpu */
#define SCHED_IDLE_BONUS 64U
#define SCHED_CACHE_BONUS_HOT 96U
#define SCHED_CACHE_BONUS_COLD 32U
#define SCHED_CACHE_HOT_TICKS 16U

#define NO_CPU 0xFFFFFFFFU
#define PRIO_WORDS ((NR_PRIORITIES + 31) / 32)

static_assert(SMP_MAX_CPUS <= 32, "maski CPU sa 32-bitowe");

void sched_enqueue(struct proces *p);
void sched_dequeue(struct proces *p, uint32_t cpu);
void sched_finish_switch(uint32_t cpu);
extern void smp_send_resched_ipi(uint32_t cpu) __attribute__((weak));

/* ------------------------------------------------------------------------- */
/* Dane globalne                                                             */
/* ------------------------------------------------------------------------- */

struct proces process_table[MAX_PROCESSES];
struct proces *current_cpu[SMP_MAX_CPUS] = {0};
struct proces *current_process = 0;

static uint32_t next_pid = 1;
static uint32_t process_count = 0;
static uint32_t kernel_page_dir_phys = 0;
static volatile uint64_t global_ticks = 0;
uint32_t smp_cpu_count = 1;
uint32_t cpu_online_mask = 1;
uint32_t smp_this_cpu = 0;

static uint32_t driver_kernel_stacks[64][1024] __attribute__((aligned(16)));
static uint32_t driver_stack_count = 0;

/* Ticket lock: uczciwy, bez glodzenia rdzeni przy rywalizacji. */
typedef union {
    uint32_t all;
    struct {
        volatile uint16_t next;
        volatile uint16_t owner;
    } t;
} tlock_t;

struct runqueue {
    uint32_t prio_bitmap[PRIO_WORDS];
    struct proces *prio_head[NR_PRIORITIES];
    struct proces *prio_tail[NR_PRIORITIES];
    uint32_t nr_running;
    uint32_t util_sum;     /* suma util zadan w kolejce (bez biezacego) */
    uint32_t nr_perf;      /* ile zadan w kolejce zada wydajnosci (util >= PERF_DEMAND) */
    tlock_t lock;
} __attribute__((aligned(64)));

struct cpu_meta_t {
    uint32_t type;
    uint32_t capacity;
    uint32_t util;
    uint32_t idle_ticks;
} __attribute__((aligned(64)));

struct proces_ext {
    uint32_t affinity;
    uint32_t util;
    uint64_t util_stamp;
    uint32_t on_cpu;       /* 1 = zadanie jest/bylo jeszcze wykonywane na jakims CPU (przelaczenie niedokonczone) */
    uint32_t rq_cpu;       /* w ktorej kolejce siedzi (NO_CPU = w zadnej); zmieniane pod blokada kolejki */
    uint32_t migrate_to;   /* zadanie aktualnie dzialajace ma sie przeniesc na ten CPU */
    uint32_t qutil;        /* util zapamietane przy wstawieniu do kolejki */
    uint32_t qperf;
    int32_t qprio;
};

static struct runqueue cpu_rq[SMP_MAX_CPUS];
static struct cpu_meta_t cpu_meta[SMP_MAX_CPUS];
static struct proces_ext proc_ext[MAX_PROCESSES];
static uint32_t cpu_parked_mask = 0;
static uint32_t max_cap = SCHED_CAP_PERF;
static struct proces *last_prev[SMP_MAX_CPUS] = {0};

static struct proces *wait_head = 0;
static struct proces *wait_tail = 0;
static struct proces *sleep_head = 0;
static struct proces *sleep_tail = 0;
static tlock_t waitq_lock;
static tlock_t table_lock;
static struct proces *idle_task[SMP_MAX_CPUS] = {0};

uint32_t cpu_id(void) {
    return smp_this_cpu;
}

/* ------------------------------------------------------------------------- */
/* Przerwania i blokady                                                      */
/* ------------------------------------------------------------------------- */

static inline uint32_t irq_save(void) {
    uint32_t f;
    __asm__ volatile("pushf; pop %0; cli" : "=r"(f) :: "memory");
    return f;
}

static inline void irq_restore(uint32_t f) {
    if (f & 0x200U) __asm__ volatile("sti" ::: "memory");
}

static inline void tlock_lock(tlock_t *l) {
    uint16_t me = __atomic_fetch_add(&l->t.next, 1, __ATOMIC_ACQUIRE);
    while (__atomic_load_n(&l->t.owner, __ATOMIC_ACQUIRE) != me) __builtin_ia32_pause();
}

static inline int tlock_trylock(tlock_t *l) {
    uint32_t old = __atomic_load_n(&l->all, __ATOMIC_RELAXED);
    if ((old & 0xFFFFU) != (old >> 16)) return 0;
    uint32_t nw = (old & 0xFFFF0000U) | ((old + 1U) & 0xFFFFU);
    return __atomic_compare_exchange_n(&l->all, &old, nw, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
}

static inline void tlock_unlock(tlock_t *l) {
    __atomic_store_n(&l->t.owner, (uint16_t)(l->t.owner + 1U), __ATOMIC_RELEASE);
}

static inline uint32_t tlock_irqsave(tlock_t *l) {
    uint32_t f = irq_save();
    tlock_lock(l);
    return f;
}

static inline void tlock_irqrestore(tlock_t *l, uint32_t f) {
    tlock_unlock(l);
    irq_restore(f);
}

/* ------------------------------------------------------------------------- */
/* Pomocnicze                                                                */
/* ------------------------------------------------------------------------- */

static inline int prio_clamp(int p) {
    if (p < 0) return 0;
    if (p >= (int)NR_PRIORITIES) return (int)NR_PRIORITIES - 1;
    return p;
}

static inline struct proces_ext *ext_of(struct proces *p) {
    return &proc_ext[p - process_table];
}

static inline uint32_t ncpus(void) {
    return smp_cpu_count < SMP_MAX_CPUS ? smp_cpu_count : SMP_MAX_CPUS;
}

static inline uint32_t mask_cpus(void) {
    uint32_t n = ncpus();
    return n >= 32U ? 0xFFFFFFFFU : ((1U << n) - 1U);
}

static inline uint32_t parked_mask(void) {
    return __atomic_load_n(&cpu_parked_mask, __ATOMIC_ACQUIRE);
}

static inline uint32_t usable_mask(void) {
    uint32_t m = cpu_online_mask & ~parked_mask() & mask_cpus();
    return m ? m : 1U;
}

static inline uint32_t cap_of(uint32_t c) {
    return cpu_meta[c].capacity ? cpu_meta[c].capacity : SCHED_CAP_PERF;
}

static inline int cpu_is_idle(uint32_t c) {
    return idle_task[c] && current_cpu[c] == idle_task[c] && !cpu_rq[c].nr_running;
}

static inline int task_wants_perf(struct proces *p) {
    return p->sched_class == SCHED_REALTIME || p->sched_class == SCHED_CRITICAL || p->sched_class == SCHED_INTERACTIVE;
}

static inline int task_wants_eff(struct proces *p) {
    if (p->sched_class == SCHED_IDLE || p->sched_class == SCHED_BACKGROUND) return 1;
    if (p->sched_class == SCHED_NORMAL && ext_of(p)->util < SCHED_EFF_TASK_UTIL) return 1;
    return 0;
}

/* Rdzen o maksymalnej pojemnosci "pasuje" zawsze - inaczej zadania z util > 80%
 * nigdy nie zostalyby uznane za pasujace do zadnego CPU i migracja by sie zapetlala. */
static inline int fits_cpu(struct proces *p, uint32_t c) {
    uint32_t cap = cap_of(c);
    if (cap >= max_cap) return 1;
    return ext_of(p)->util * 1280U <= cap * 1024U;
}

static void sched_update_maxcap(void) {
    uint32_t m = 0;
    for (uint32_t c = 0; c < ncpus(); c++) {
        uint32_t cap = cap_of(c);
        if (cap > m) m = cap;
    }
    max_cap = m ? m : SCHED_CAP_PERF;
}

/* Obciazenie CPU w jednostkach wagi (util + minimalna waga na zadanie). */
static uint32_t cpu_weight(uint32_t c) {
    struct runqueue *rq = &cpu_rq[c];
    uint32_t w = rq->util_sum + rq->nr_running * SCHED_MIN_WEIGHT;
    struct proces *cur = current_cpu[c];
    if (cur && cur != idle_task[c] && cur->state == PROCES_RUNNING) {
        w += ext_of(cur)->util + SCHED_MIN_WEIGHT;
    }
    return w;
}

static inline uint32_t weight_to_load(uint32_t w, uint32_t c) {
    if (w > (1U << 21)) w = 1U << 21;
    return (w * 1024U) / cap_of(c);
}

static inline uint32_t cpu_load_norm(uint32_t c) {
    return weight_to_load(cpu_weight(c), c);
}

/* ------------------------------------------------------------------------- */
/* Kolejka gotowych (priorytety + lista wg vruntime)                         */
/* ------------------------------------------------------------------------- */

static inline void prio_bitmap_set(struct runqueue *rq, int prio) {
    rq->prio_bitmap[prio >> 5] |= (1U << (prio & 31));
}

static inline void prio_bitmap_clear(struct runqueue *rq, int prio) {
    rq->prio_bitmap[prio >> 5] &= ~(1U << (prio & 31));
}

static inline int prio_find_highest(struct runqueue *rq) {
    for (int i = 0; i < (int)PRIO_WORDS; i++) {
        if (rq->prio_bitmap[i]) return i * 32 + __builtin_ctz(rq->prio_bitmap[i]);
    }
    return -1;
}

/* Wolajacy trzyma blokade cpu_rq[cpu]. */
static void runq_enqueue_locked(struct proces *p, uint32_t cpu) {
    struct proces_ext *e = ext_of(p);
    if (e->rq_cpu != NO_CPU) return;           /* juz w kolejce - ochrona przed podwojnym wstawieniem */
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

    e->rq_cpu = cpu;
    e->qprio = prio;
    e->qutil = e->util;
    e->qperf = (task_wants_perf(p) && e->util >= SCHED_PERF_DEMAND_UTIL) ? 1U : 0U;
    rq->util_sum += e->qutil;
    rq->nr_perf += e->qperf;
}

static void runq_dequeue_locked(struct proces *p, struct runqueue *rq) {
    struct proces_ext *e = ext_of(p);
    int prio = e->qprio;                       /* priorytet z chwili wstawienia, odporny na zmiany */
    if (p->run_prev) p->run_prev->run_next = p->run_next;
    else rq->prio_head[prio] = p->run_next;
    if (p->run_next) p->run_next->run_prev = p->run_prev;
    else rq->prio_tail[prio] = p->run_prev;
    p->run_next = 0;
    p->run_prev = 0;
    if (!rq->prio_head[prio]) prio_bitmap_clear(rq, prio);
    if (rq->nr_running > 0) rq->nr_running--;
    rq->util_sum = rq->util_sum >= e->qutil ? rq->util_sum - e->qutil : 0;
    rq->nr_perf = rq->nr_perf >= e->qperf ? rq->nr_perf - e->qperf : 0;
    e->rq_cpu = NO_CPU;
}

/* Zdejmuje najlepsze zadanie o priorytecie <= limit, pomijajac te, ktore sa
 * jeszcze w trakcie przelaczania na innym CPU (on_cpu). */
static struct proces *rq_pop_best_locked(struct runqueue *rq, int limit) {
    for (int w = 0; w < (int)PRIO_WORDS; w++) {
        uint32_t bits = rq->prio_bitmap[w];
        while (bits) {
            int prio = w * 32 + __builtin_ctz(bits);
            if (prio > limit) return 0;
            bits &= bits - 1U;
            for (struct proces *p = rq->prio_head[prio]; p; p = p->run_next) {
                if (!__atomic_load_n(&ext_of(p)->on_cpu, __ATOMIC_ACQUIRE)) {
                    runq_dequeue_locked(p, rq);
                    return p;
                }
            }
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Kolejki oczekiwania i uspienia                                            */
/* ------------------------------------------------------------------------- */

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
    return rq_pop_best_locked(&cpu_rq[cpu], (int)NR_PRIORITIES - 1);
}

/* ------------------------------------------------------------------------- */
/* Utylizacja zadan                                                          */
/* ------------------------------------------------------------------------- */

/* Zanik geometryczny o polokresie 32 tickow: y^32 = 0.5, wartosci * 1024. */
static const uint16_t util_y32[32] = {
    1024, 1002, 981, 960, 939, 919, 899, 880, 861, 843, 825, 807, 790, 773, 756, 740,
    724, 709, 694, 679, 664, 650, 636, 622, 609, 596, 583, 570, 558, 546, 534, 523
};

/* O(1) zamiast petli do 192 iteracji w przerwaniu. */
static void util_decay(struct proces *p) {
    struct proces_ext *e = ext_of(p);
    uint64_t now = global_ticks;
    uint64_t dt = now > e->util_stamp ? now - e->util_stamp : 0;
    uint32_t v = e->util;
    if (dt >= 352ULL) {
        v = 0;
    } else if (dt) {
        uint32_t n = (uint32_t)dt;
        v = ((v >> (n >> 5)) * (uint32_t)util_y32[n & 31U]) >> 10;
    }
    e->util = v < 16U ? 0U : v;
    e->util_stamp = now;
}

/* ------------------------------------------------------------------------- */
/* Wybor CPU (energy / capacity aware)                                       */
/* ------------------------------------------------------------------------- */

static uint32_t select_cpu_excl(struct proces *p, uint32_t excl) {
    struct proces_ext *e = ext_of(p);
    uint32_t online = cpu_online_mask & mask_cpus();
    uint32_t allowed = e->affinity & online & ~excl;
    if (!allowed) allowed = e->affinity & online;
    if (!allowed) allowed = usable_mask();

    uint32_t parked = parked_mask();
    int want_perf = task_wants_perf(p);
    int want_eff = !want_perf && task_wants_eff(p);
    uint32_t tw = e->util + SCHED_MIN_WEIGHT;
    uint64_t now = global_ticks;
    uint32_t best_cpu = (uint32_t)__builtin_ctz(allowed);
    uint32_t best_cost = 0xFFFFFFFFU;

    for (uint32_t c = 0; c < ncpus(); c++) {
        if (!(allowed & (1U << c))) continue;

        /* koszt = obciazenie CPU PO dolozeniu tego zadania, znormalizowane do pojemnosci */
        uint32_t cost = weight_to_load(cpu_weight(c) + tw, c);

        if (!fits_cpu(p, c)) cost += 1000000U;
        if (cpu_meta[c].type == SCHED_CPU_PERF) {
            if (want_eff) cost += 384U;
            else if (!want_perf) cost += 48U;
        } else if (want_perf) {
            cost += 500000U;
        }
        if (parked & (1U << c)) cost += SCHED_WAKE_PARKED_COST;
        if (cpu_is_idle(c)) cost = cost > SCHED_IDLE_BONUS ? cost - SCHED_IDLE_BONUS : 0U;
        if (c == p->cpu) {
            uint32_t b = (now - p->exec_start) <= SCHED_CACHE_HOT_TICKS ? SCHED_CACHE_BONUS_HOT : SCHED_CACHE_BONUS_COLD;
            cost = cost > b ? cost - b : 0U;
        }
        if (cost < best_cost) {
            best_cost = cost;
            best_cpu = c;
        }
    }
    if (parked & (1U << best_cpu)) {
        __atomic_fetch_and(&cpu_parked_mask, ~(1U << best_cpu), __ATOMIC_SEQ_CST);
    }
    return best_cpu;
}

static inline uint32_t select_cpu(struct proces *p) {
    return select_cpu_excl(p, 0);
}

/* ------------------------------------------------------------------------- */
/* Balansowanie                                                              */
/* ------------------------------------------------------------------------- */

static int pullable(struct proces *p, uint32_t victim, uint32_t dest, int misfit_only) {
    struct proces_ext *e = ext_of(p);
    if (__atomic_load_n(&e->on_cpu, __ATOMIC_ACQUIRE)) return 0;
    if (!((e->affinity >> dest) & 1U)) return 0;
    if (misfit_only) return !fits_cpu(p, victim) && fits_cpu(p, dest);
    if (!fits_cpu(p, dest) && fits_cpu(p, victim)) return 0;
    if (task_wants_perf(p) && cpu_meta[dest].type != SCHED_CPU_PERF) return 0;
    if (cpu_meta[dest].type == SCHED_CPU_PERF && cpu_meta[victim].type == SCHED_CPU_EFF && task_wants_eff(p)) return 0;
    return 1;
}

/* Wolane z wylaczonymi przerwaniami (tick / schedule_cpu). */
static struct proces *pull_from(uint32_t victim, uint32_t dest, int misfit_only) {
    struct runqueue *v = &cpu_rq[victim];
    if (!v->nr_running) return 0;
    if (!tlock_trylock(&v->lock)) return 0;

    struct proces *found = 0;
    uint32_t scanned = 0;
    for (int w = 0; w < (int)PRIO_WORDS && !found && scanned < SCHED_PULL_SCAN_LIMIT; w++) {
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
    tlock_unlock(&v->lock);
    if (found) {
        found->cpu = dest;
        found->nr_migrations++;
    }
    return found;
}

static struct proces *balance_pull(uint32_t cpu) {
    uint32_t usable = usable_mask();
    if (!((usable >> cpu) & 1U)) return 0;

    /* 1) rdzen wydajny sciaga zadania "misfit" z rdzeni energooszczednych */
    if (cpu_meta[cpu].type == SCHED_CPU_PERF) {
        for (uint32_t i = 0; i < ncpus(); i++) {
            if (i == cpu || !((usable >> i) & 1U)) continue;
            if (cpu_meta[i].type != SCHED_CPU_EFF) continue;
            struct proces *m = pull_from(i, cpu, 1);
            if (m) return m;
        }
    }

    /* 2) klasyczne wyrownanie obciazenia znormalizowanego do pojemnosci */
    int idle_now = cpu_is_idle(cpu);
    uint32_t mine = cpu_load_norm(cpu);
    uint32_t best = idle_now ? mine : mine + SCHED_IMBALANCE_MARGIN;
    uint32_t victim = cpu;
    for (uint32_t i = 0; i < ncpus(); i++) {
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

/* Aktywna migracja zadania biezacego: "misfit" w gore, male zadania w dol na E-core.
 * Sama migracja odbywa sie w schedule_cpu (zadanie nie siedzi w kolejce, wiec nie da sie go pociagnac). */
static void misfit_check(uint32_t cpu, struct proces *cur) {
    struct proces_ext *e = ext_of(cur);
    if (e->migrate_to != NO_CPU) return;
    uint32_t cand = e->affinity & cpu_online_mask & mask_cpus() & ~(1U << cpu);
    if (!cand) return;

    int up = !fits_cpu(cur, cpu);
    int down = cpu_meta[cpu].type == SCHED_CPU_PERF && !task_wants_perf(cur) &&
               task_wants_eff(cur) && e->util < SCHED_DOWNMIGRATE_UTIL;
    if (!up && !down) return;

    uint32_t target = NO_CPU;
    uint32_t target_cap = 0;
    for (uint32_t c = 0; c < ncpus(); c++) {
        if (!(cand & (1U << c)) || !cpu_is_idle(c)) continue;
        uint32_t cap = cap_of(c);
        if (up) {
            if (cap <= cap_of(cpu)) continue;
            if (cap > target_cap) { target = c; target_cap = cap; }
        } else {
            if (cpu_meta[c].type != SCHED_CPU_EFF) continue;
            target = c;
            break;
        }
    }
    if (target == NO_CPU) return;
    if (parked_mask() & (1U << target)) {
        __atomic_fetch_and(&cpu_parked_mask, ~(1U << target), __ATOMIC_SEQ_CST);
    }
    e->migrate_to = target;
}

/* ------------------------------------------------------------------------- */
/* Zarzadzanie energia (parkowanie rdzeni)                                   */
/* ------------------------------------------------------------------------- */

static void drain_cpu(uint32_t c) {
    struct runqueue *rq = &cpu_rq[c];
    uint32_t n = rq->nr_running;
    while (n--) {
        uint32_t f = tlock_irqsave(&rq->lock);
        struct proces *p = rq_pop_best_locked(rq, (int)NR_PRIORITIES - 1);
        tlock_irqrestore(&rq->lock, f);
        if (!p) break;
        p->cpu = select_cpu_excl(p, 1U << c);   /* nie wybieraj z powrotem parkowanego rdzenia bo ci krowa nasika! */
        p->nr_migrations++;
        sched_enqueue(p);
    }
}

/* O(liczba CPU) zamiast przegladania calej process_table. */
static void energy_manage(void) {
    uint32_t n = ncpus();
    uint32_t total = 0;
    int perf_demand = 0;
    for (uint32_t c = 0; c < n; c++) {
        struct runqueue *rq = &cpu_rq[c];
        total += rq->util_sum;
        if (rq->nr_perf) perf_demand = 1;
        struct proces *cur = current_cpu[c];
        if (cur && cur != idle_task[c] && cur->state == PROCES_RUNNING) {
            uint32_t u = ext_of(cur)->util;
            total += u;
            if (task_wants_perf(cur) && u >= SCHED_PERF_DEMAND_UTIL) perf_demand = 1;
        }
    }

    uint32_t need = total + (total >> 2);
    uint32_t online = cpu_online_mask & mask_cpus();
    uint32_t desired = 1U;
    uint32_t cap = cap_of(0);

    for (uint32_t pass = 0; pass < 2 && cap < need; pass++) {
        uint32_t want_type = pass == 0 ? SCHED_CPU_EFF : SCHED_CPU_PERF;
        for (uint32_t c = 1; c < n && cap < need; c++) {
            if (!((online >> c) & 1U) || ((desired >> c) & 1U)) continue;
            if (cpu_meta[c].type != want_type) continue;
            desired |= (1U << c);
            cap += cap_of(c);
        }
    }

    if (perf_demand) {
        int have_perf = 0;
        for (uint32_t c = 0; c < n; c++) {
            if (((desired >> c) & 1U) && cpu_meta[c].type == SCHED_CPU_PERF) have_perf = 1;
        }
        if (!have_perf) {
            for (uint32_t c = 1; c < n; c++) {
                if (((online >> c) & 1U) && cpu_meta[c].type == SCHED_CPU_PERF) {
                    desired |= (1U << c);
                    break;
                }
            }
        }
    }

    uint32_t parked = parked_mask();
    for (uint32_t c = 1; c < n; c++) {
        uint32_t bit = 1U << c;
        if (!(online & bit)) continue;
        if (desired & bit) {
            if (parked & bit) __atomic_fetch_and(&cpu_parked_mask, ~bit, __ATOMIC_SEQ_CST);
        } else if (!(parked & bit) && cpu_meta[c].idle_ticks >= SCHED_PARK_IDLE_TICKS &&
                   !cpu_rq[c].nr_running && cpu_is_idle(c)) {
            __atomic_fetch_or(&cpu_parked_mask, bit, __ATOMIC_SEQ_CST);
            drain_cpu(c);
        }
    }
}

/* ------------------------------------------------------------------------- */
/* Topologia CPU                                                             */
/* ------------------------------------------------------------------------- */

static inline void cpuid_raw(uint32_t leaf, uint32_t sub, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    __asm__ volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(leaf), "c"(sub));
}

/* Pozwala kodowi platformy (ACPI/CPPC/kalibracja) nadpisac typ i pojemnosc rdzenia. */
void cpu_set_topology(uint32_t cpu, uint32_t type, uint32_t capacity) {
    if (cpu >= SMP_MAX_CPUS) return;
    cpu_meta[cpu].type = type;
    cpu_meta[cpu].capacity = capacity ? capacity : SCHED_CAP_PERF;
    sched_update_maxcap();
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
    sched_update_maxcap();
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
    return (parked_mask() >> cpu) & 1U;
}

uint32_t sched_idle_hint(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return 0;
    if ((parked_mask() >> cpu) & 1U) return 2U;
    if (cpu_meta[cpu].idle_ticks >= 32U) return 1U;
    return 0U;
}

/* ------------------------------------------------------------------------- */
/* Stos procesu uzytkownika                                                  */
/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */
/* Rdzen schedulera                                                          */
/* ------------------------------------------------------------------------- */

/* Zwalnia zadanie wylaczone z CPU w POPRZEDNIM przelaczeniu. Wolac:
 *  - na poczatku schedule_cpu (fallback), oraz
 *  - najlepiej zaraz po faktycznym przelaczeniu kontekstu (kod asm / powrot z IRQ),
 *    zeby inny CPU mogl od razu przejac zadanie. */
void sched_finish_switch(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) return;
    struct proces *prev = last_prev[cpu];
    if (!prev) return;
    last_prev[cpu] = 0;
    __atomic_store_n(&ext_of(prev)->on_cpu, 0U, __ATOMIC_RELEASE);
}

/* Wolane z wylaczonymi przerwaniami. */
static void schedule_cpu(uint32_t cpu, int yield) {
    struct runqueue *rq = &cpu_rq[cpu];
    struct proces *cur = current_cpu[cpu];
    struct proces *idle = idle_task[cpu];

    sched_finish_switch(cpu);

    int cur_runnable = cur && cur->state == PROCES_RUNNING;
    int cur_is_idle = cur && cur == idle;
    int expired = yield || (cur_runnable && cur->time_slice == 0);

    /* Czy biezace zadanie ma opuscic ten CPU (zadanie migracji lub naruszone affinity)? */
    uint32_t mig = NO_CPU;
    if (cur_runnable && !cur_is_idle) {
        struct proces_ext *ce = ext_of(cur);
        uint32_t req = ce->migrate_to;
        ce->migrate_to = NO_CPU;
        if (!((ce->affinity >> cpu) & 1U)) req = select_cpu_excl(cur, 1U << cpu);
        if (req < ncpus() && req != cpu && ((ce->affinity >> req) & 1U)) mig = req;
    }

    tlock_lock(&rq->lock);

    struct proces *next = 0;
    int limit;
    if (!cur_runnable || cur_is_idle || mig != NO_CPU) {
        limit = (int)NR_PRIORITIES - 1;
    } else {
        int cp = prio_clamp(cur->priority);
        limit = expired ? cp : cp - 1;
    }
    if (limit >= 0) next = rq_pop_best_locked(rq, limit);

    if (cur_runnable && expired && cur->time_slice == 0) cur->time_slice = time_slice_for(cur);

    if (next && cur_runnable && !cur_is_idle && mig == NO_CPU) {
        cur->state = PROCES_READY;
        runq_enqueue_locked(cur, cpu);        /* affinity sprawdzone wyzej, jeszcze pod blokada */
    }
    tlock_unlock(&rq->lock);

    if (!next && (!cur_runnable || cur_is_idle || mig != NO_CPU)) {
        next = balance_pull(cpu);
        if (!next && !cur_is_idle) next = idle;
    }

    if (next && next != cur) {
        if (cur && cur_runnable && !cur_is_idle && mig != NO_CPU) {
            cur->state = PROCES_READY;
            cur->cpu = mig;
            cur->nr_migrations++;
            sched_enqueue(cur);               /* on_cpu=1 chroni je przed przejeciem przed koncem przelaczenia */
        }
        if (cur) {
            cur->ctx_switches++;
            if (!cur_is_idle) last_prev[cpu] = cur;
        }
        __atomic_store_n(&ext_of(next)->on_cpu, 1U, __ATOMIC_RELEASE);
        current_cpu[cpu] = next;
        current_process = next;
        next->cpu = cpu;
        next->exec_start = global_ticks;
    }
    if (next) next->state = PROCES_RUNNING;
}

/* ------------------------------------------------------------------------- */
/* Tablica procesow                                                          */
/* ------------------------------------------------------------------------- */

/* Atomowo rezerwuje wolny slot (nadaje pid), zeby dwa CPU nie dostaly tego samego. */
static struct proces *proces_claim_slot(void) {
    struct proces *p = 0;
    uint32_t f = tlock_irqsave(&table_lock);
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCES_TERMINATED || process_table[i].pid == 0) {
            p = &process_table[i];
            p->pid = next_pid++;
            p->state = PROCES_READY;
            break;
        }
    }
    tlock_irqrestore(&table_lock, f);
    if (p) __atomic_fetch_add(&process_count, 1U, __ATOMIC_SEQ_CST);
    return p;
}

static void proces_release_slot(struct proces *p) {
    p->state = PROCES_TERMINATED;
    p->pid = 0;
    __atomic_fetch_sub(&process_count, 1U, __ATOMIC_SEQ_CST);
}

struct proces *proces_find(uint32_t pid) {
    if (!pid) return 0;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) return &process_table[i];
    }
    return 0;
}

static void proces_copy_name(struct proces *p, const char *name) {
    for (uint32_t i = 0; i < 32; i++) p->name[i] = 0;
    for (uint32_t i = 0; i < 31 && name && name[i]; i++) p->name[i] = name[i];
}

/* Wspolna inicjalizacja pol schedulera dla nowo zarezerwowanego slotu. */
static void proces_init_common(struct proces *p, uint32_t entry_point, uint32_t arg, const char *name) {
    p->entry_point = entry_point;
    p->arg = arg;
    p->window = 0;
    p->run_next = 0;
    p->run_prev = 0;
    p->wait_next = 0;
    p->wait_prev = 0;
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
    p->is_driver = 0;
    p->driver_handle = 0xFFFFFFFFU;
    p->driver_poll_interval = 0;
    p->driver_last_poll = 0;

    struct proces_ext *e = ext_of(p);
    e->affinity = 0xFFFFFFFFU;
    e->util = SCHED_NEW_TASK_UTIL;
    e->util_stamp = global_ticks;
    e->on_cpu = 0;
    e->rq_cpu = NO_CPU;
    e->migrate_to = NO_CPU;
    e->qutil = 0;
    e->qperf = 0;
    e->qprio = 0;

    proces_copy_name(p, name);
    for (uint32_t i = 0; i < MAX_OKNA_PROCESU; i++) p->okna_uzyte[i] = 0;
}

void sched_init_cpu_idle(uint32_t c) {
    if (c >= SMP_MAX_CPUS || idle_task[c]) return;
    struct proces *idle = proces_stworz(0, "idle", 0);
    if (!idle) return;
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

void proces_init(void) {
    for (uint32_t i = 0; i < SMP_MAX_CPUS; i++) {
        for (uint32_t j = 0; j < PRIO_WORDS; j++) cpu_rq[i].prio_bitmap[j] = 0;
        for (uint32_t j = 0; j < NR_PRIORITIES; j++) {
            cpu_rq[i].prio_head[j] = 0;
            cpu_rq[i].prio_tail[j] = 0;
        }
        cpu_rq[i].nr_running = 0;
        cpu_rq[i].util_sum = 0;
        cpu_rq[i].nr_perf = 0;
        cpu_rq[i].lock.all = 0;
        cpu_meta[i].util = 0;
        cpu_meta[i].idle_ticks = 0;
        current_cpu[i] = 0;
        idle_task[i] = 0;
        last_prev[i] = 0;
    }
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        p->pid = 0;
        p->state = PROCES_TERMINATED;
        p->page_directory = 0;
        p->kernel_stack = 0;
        p->user_stack = 0;
        p->run_next = 0;
        p->run_prev = 0;
        p->wait_next = 0;
        p->wait_prev = 0;
        p->window = 0;
        proc_ext[i].affinity = 0xFFFFFFFFU;
        proc_ext[i].util = 0;
        proc_ext[i].util_stamp = 0;
        proc_ext[i].on_cpu = 0;
        proc_ext[i].rq_cpu = NO_CPU;
        proc_ext[i].migrate_to = NO_CPU;
        proc_ext[i].qutil = 0;
        proc_ext[i].qperf = 0;
        proc_ext[i].qprio = 0;
    }
    wait_head = 0;
    wait_tail = 0;
    sleep_head = 0;
    sleep_tail = 0;
    waitq_lock.all = 0;
    table_lock.all = 0;
    cpu_parked_mask = 0;
    current_process = 0;
    next_pid = 1;
    process_count = 0;
    global_ticks = 0;

    asm volatile("mov %%cr3, %0" : "=r"(kernel_page_dir_phys));

    sched_register_this_cpu();

    for (uint32_t c = 0; c < ncpus(); c++) sched_init_cpu_idle(c);
}

struct proces *proces_stworz(uint32_t entry_point, const char *name, uint32_t arg) {
    if (process_count >= MAX_PROCESSES) return 0;
    struct proces *p = proces_claim_slot();
    if (!p) return 0;

    proces_init_common(p, entry_point, arg, name);
    p->page_directory = proces_klonuj_page_directory();
    if (!p->page_directory) {
        proces_release_slot(p);
        return 0;
    }

    proces_ustaw_stos(p, entry_point, arg);
    p->cpu = select_cpu(p);
    sched_enqueue(p);
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
    sched_dequeue(p, p->cpu);                   /* bezpieczne: no-op, jesli nie w kolejce */
    if (p->state == PROCES_BLOCKED || p->state == PROCES_SLEEPING) {
        tlock_lock(&waitq_lock);
        wait_remove_any_locked(p);
        tlock_unlock(&waitq_lock);
    }
    p->state = PROCES_TERMINATED;
    p->pid = 0;
    __atomic_fetch_sub(&process_count, 1U, __ATOMIC_SEQ_CST);

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

    if (cpu == 0) global_ticks += 1ULL;
    uint64_t now = global_ticks;

    if (sleep_head && sleep_head->sleep_until <= now) {
        tlock_lock(&waitq_lock);
        while (sleep_head && sleep_head->sleep_until <= now) {
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
        tlock_unlock(&waitq_lock);
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
            e->util_stamp = now;
        }
    }

    if (cpu == 0 && (((uint32_t)now) & (SCHED_ENERGY_INTERVAL - 1U)) == 0U) energy_manage();

    if (busy && ((((uint32_t)now) + cpu) & (SCHED_BALANCE_INTERVAL - 1U)) == 0U) {
        misfit_check(cpu, cur);                 /* migracja biezacego zadania (wykona schedule_cpu) */
        if (ext_of(cur)->migrate_to == NO_CPU) {
            struct proces *pulled = balance_pull(cpu);
            if (pulled) sched_enqueue(pulled);
        }
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

    tlock_lock(&waitq_lock);
    cur->state = PROCES_SLEEPING;
    cur->sleep_until = ticks;
    cur->sleep_ticks++;
    sleep_insert_locked(cur);
    tlock_unlock(&waitq_lock);

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

    tlock_lock(&waitq_lock);
    cur->state = PROCES_BLOCKED;
    cur->nr_blocked++;
    waitq_enqueue_locked(cur);
    tlock_unlock(&waitq_lock);

    schedule_cpu(cpu, 0);
    irq_restore(f);
}

void proces_wakeup(struct proces *p) {
    if (!p) return;
    uint32_t f = tlock_irqsave(&waitq_lock);
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
    tlock_irqrestore(&waitq_lock, f);
}

int proces_set_priority(uint32_t pid, int32_t priority) {
    struct proces *p = proces_find(pid);
    if (!p) return -1;
    uint32_t f = irq_save();
    int queued = ext_of(p)->rq_cpu != NO_CPU;
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
    int queued = ext_of(p)->rq_cpu != NO_CPU;
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
    /* zadanie w kolejce na zlym CPU przenosimy od razu; dzialajace - schedule_cpu przy najblizszym ticku */
    if (ext_of(p)->rq_cpu != NO_CPU && !((mask >> p->cpu) & 1U)) {
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

    uint32_t stack_idx = __atomic_fetch_add(&driver_stack_count, 1U, __ATOMIC_SEQ_CST);
    if (stack_idx >= 64U) return 0;

    struct proces *p = proces_claim_slot();
    if (!p) return 0;

    proces_init_common(p, entry_point, arg, name);
    p->page_directory = (uint32_t *)kernel_page_dir_phys;
    p->sched_class = SCHED_CRITICAL;
    p->priority = 10;
    p->static_priority = 10;
    p->time_slice = 4U;
    p->original_prio = p->priority;
    p->is_driver = 1;
    p->driver_poll_interval = poll_interval;

    p->kernel_stack = (uint32_t)&driver_kernel_stacks[stack_idx][0] + 4096U - 16U;

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
                proces_wakeup(p);              /* budzi tylko jesli BLOCKED/SLEEPING; nie psuje kolejki */
                return;
            }
        }
    }

    struct proces *p = proces_claim_slot();
    if (!p) return;
    proces_init_common(p, 0, 0, nazwa);
    p->window = okno;
    p->kernel_stack = 0;
    p->user_stack = 0;
    p->page_directory = 0;
    p->cpu = select_cpu(p);
    sched_enqueue(p);
}

/* ------------------------------------------------------------------------- */
/* Interfejs kolejki                                                         */
/* ------------------------------------------------------------------------- */

void sched_enqueue(struct proces *p) {
    uint32_t cpu = p->cpu;
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    if (!((usable_mask() >> cpu) & 1U)) {
        p->cpu = select_cpu(p);                /* odblokuje rdzen, jesli to naprawde potrzebne */
        cpu = p->cpu;
    }
    p->cpu = cpu;
    uint32_t f = tlock_irqsave(&cpu_rq[cpu].lock);
    runq_enqueue_locked(p, cpu);
    tlock_irqrestore(&cpu_rq[cpu].lock, f);
    if (smp_send_resched_ipi && idle_task[cpu] && cpu != cpu_id() && current_cpu[cpu] == idle_task[cpu]) {
        smp_send_resched_ipi(cpu);
    }
}

/* Parametr cpu jest tylko podpowiedzia: faktyczna kolejka wynika z rq_cpu,
 * wiec wyscig z migracja (pull_from / drain_cpu) nie psuje list. */
void sched_dequeue(struct proces *p, uint32_t cpu) {
    (void)cpu;
    struct proces_ext *e = ext_of(p);
    for (;;) {
        uint32_t c = __atomic_load_n(&e->rq_cpu, __ATOMIC_ACQUIRE);
        if (c >= SMP_MAX_CPUS) return;         /* NO_CPU: nie jest w zadnej kolejce */
        uint32_t f = tlock_irqsave(&cpu_rq[c].lock);
        if (e->rq_cpu == c) {
            runq_dequeue_locked(p, &cpu_rq[c]);
            tlock_irqrestore(&cpu_rq[c].lock, f);
            return;
        }
        tlock_irqrestore(&cpu_rq[c].lock, f);  /* zmienila sie w miedzyczasie - sprobuj ponownie */
    }
}

struct proces *sched_pick_next(uint32_t cpu) {
    if (cpu >= SMP_MAX_CPUS) cpu = 0;
    uint32_t f = tlock_irqsave(&cpu_rq[cpu].lock);
    struct proces *next = sched_pick_next_locked(cpu);
    tlock_irqrestore(&cpu_rq[cpu].lock, f);
    return next;
}