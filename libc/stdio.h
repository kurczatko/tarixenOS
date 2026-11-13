#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H
#include <libc/types.h>
int printf(const char *fmt, ...);
int putc(int c);
int puts(const char *s);
#endif
