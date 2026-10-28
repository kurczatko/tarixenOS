#ifndef FLOPPY_H
#define FLOPPY_H

#include "../include/printf/types.h"

void floppy_init(void);
int floppy_read_sector(uint32_t sector, char *buffer);
int floppy_write_sector(uint32_t sector, const char *buffer);
int floppy_present(void);

#endif
