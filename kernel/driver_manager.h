#ifndef DRIVER_MANAGER_H
#define DRIVER_MANAGER_H

#include "../kernel/proces.h"

#define MAX_DRIVERS 64

typedef int (*driver_init_fn)(void);
typedef void (*driver_shutdown_fn)(void);
typedef void (*driver_poll_fn)(void);

struct driver_desc {
    char name[64];
    driver_init_fn init;
    driver_shutdown_fn shutdown;
    driver_poll_fn poll;
    uint32_t poll_interval;
    uint32_t pid;
    uint8_t active;
    uint8_t hardware_detected;
    uint8_t shutdown_requested;
};

#ifdef __cplusplus
extern "C" {
#endif

void driver_manager_register(const char *name, driver_init_fn init, driver_shutdown_fn shutdown, driver_poll_fn poll, uint32_t poll_interval);
int driver_manager_init_all(void);
void driver_manager_poll_all(void);
void driver_manager_shutdown_by_pid(uint32_t pid);
int driver_manager_is_driver(uint32_t pid);
struct driver_desc *driver_manager_find_by_pid(uint32_t pid);
void driver_manager_shutdown_all(void);
int driver_manager_get_driver_list(char names[][64], int max_count);

#ifdef __cplusplus
}
#endif

#endif
