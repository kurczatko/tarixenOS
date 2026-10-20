#ifndef LIBC_STRING_H
#define LIBC_STRING_H
#include "types.h"
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
char *strcpy(char *d, const char *s);
char *strcat(char *d, const char *s);
void *memcpy(void *d, const void *s, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *d, const void *s, size_t n);
char *strchr(const char *s, int c);
char *strstr(const char *h, const char *n);
int memcmp(const void *a, const void *b, size_t n);
#endif
