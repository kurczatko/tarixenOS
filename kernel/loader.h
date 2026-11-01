#ifndef LOADER_H
#define LOADER_H

#include "../include/printf/types.h"

int userspace_load_elf(const char *path, uint32_t *entry_out);
int userspace_load_and_run(const char *path);
int proces_zaladuj_elf(const char *path);

#endif
