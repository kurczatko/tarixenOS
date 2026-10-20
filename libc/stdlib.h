#ifndef LIBC_STDLIB_H
#define LIBC_STDLIB_H
#include "types.h"
void *malloc(size_t size);
void free(void *p);
void exit(int code);
int atoi(const char *s);
unsigned int rand(void);
void srand(unsigned int seed);
#endif
