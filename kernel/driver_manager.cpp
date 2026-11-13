#include <kernel/driver_manager.h>
#include <kernel/proces.h>
#include <include/printf/printf.h>

static void driver_idle_poll(void)
{
    while (1) {
        proces_yield();
    }
}

static struct driver_desc g_drivers[MAX_DRIVERS];
static uint32_t g_driver_count = 0;
static uint32_t g_global_ticks = 0;

void driver_manager_register(const char *name, driver_init_fn init, driver_shutdown_fn shutdown, driver_poll_fn poll, uint32_t poll_interval)
{
    if (g_driver_count >= MAX_DRIVERS) return;
    if (!init) return;

    struct driver_desc *d = &g_drivers[g_driver_count];
    uint32_t i = 0;
    while (name && name[i] && i < 63) {
        d->name[i] = name[i];
        i++;
    }
    d->name[i] = 0;
    d->init = init;
    d->shutdown = shutdown;
    d->poll = poll;
    d->poll_interval = poll_interval;
    d->pid = 0;
    d->active = 0;
    d->hardware_detected = 0;
    d->shutdown_requested = 0;
    g_driver_count++;
}

int driver_manager_init_all(void)
{
    int started = 0;
    for (uint32_t i = 0; i < g_driver_count; i++) {
        struct driver_desc *d = &g_drivers[i];
        if (d->active) continue;

        int rc = d->init();
        if (rc <= 0) {
            d->hardware_detected = 0;
            continue;
        }

        d->hardware_detected = 1;

        struct proces *p = proces_stworz_sterownik(
            (uint32_t)(d->poll ? d->poll : driver_idle_poll),
            d->name,
            (uint32_t)d,
            d->poll_interval
        );

        if (!p) {
            d->hardware_detected = 0;
            continue;
        }

        d->pid = p->pid;
        d->active = 1;
        p->driver_handle = i;
        started++;
    }
    return started;
}

void driver_manager_poll_all(void)
{
    g_global_ticks++;

    for (uint32_t i = 0; i < g_driver_count; i++) {
        struct driver_desc *d = &g_drivers[i];
        if (!d->active || !d->hardware_detected) continue;

        struct proces *p = proces_find(d->pid);
        if (!p || p->state == PROCES_TERMINATED) {
            if (d->shutdown) d->shutdown();
            d->active = 0;
            d->hardware_detected = 0;
            d->pid = 0;
            continue;
        }

        if (d->poll_interval > 0) {
            if (g_global_ticks - p->driver_last_poll < d->poll_interval) continue;
            p->driver_last_poll = g_global_ticks;
        }
    }
}

void driver_manager_shutdown_by_pid(uint32_t pid)
{
    for (uint32_t i = 0; i < g_driver_count; i++) {
        struct driver_desc *d = &g_drivers[i];
        if (d->active && d->pid == pid && d->shutdown) {
            d->shutdown();
            d->active = 0;
            d->hardware_detected = 0;
            d->pid = 0;
            return;
        }
    }
}

int driver_manager_is_driver(uint32_t pid)
{
    for (uint32_t i = 0; i < g_driver_count; i++) {
        if (g_drivers[i].active && g_drivers[i].pid == pid) return 1;
    }
    return 0;
}

struct driver_desc *driver_manager_find_by_pid(uint32_t pid)
{
    for (uint32_t i = 0; i < g_driver_count; i++) {
        if (g_drivers[i].active && g_drivers[i].pid == pid) return &g_drivers[i];
    }
    return 0;
}

void driver_manager_shutdown_all(void)
{
    for (uint32_t i = 0; i < g_driver_count; i++) {
        struct driver_desc *d = &g_drivers[i];
        if (d->active && d->shutdown) {
            d->shutdown();
            d->active = 0;
            d->hardware_detected = 0;
            d->pid = 0;
        }
    }
}

int driver_manager_get_driver_list(char names[][64], int max_count)
{
    int count = 0;
    for (uint32_t i = 0; i < g_driver_count && count < max_count; i++) {
        struct driver_desc *d = &g_drivers[i];
        if (d->active && d->hardware_detected) {
            uint32_t j = 0;
            while (d->name[j] && j < 63) {
                names[count][j] = d->name[j];
                j++;
            }
            names[count][j] = 0;
            count++;
        }
    }
    return count;
}
