#ifndef PAGING_H
#define PAGING_H

#include "../include/printf/types.h"

#define PAGE_SIZE       4096
#define PAGE_PRESENT    0x00000001
#define PAGE_READWRITE  0x00000002
#define PAGE_USER       0x00000004

#define USER_MEM_START  0x00100000
#define USER_MEM_END    0x00400000

void paging_init(void);

#endif
