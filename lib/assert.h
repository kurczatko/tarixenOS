#ifndef LIBC_ASSERT_H
#define LIBC_ASSERT_H

#include <lib/types.h>

#define assert(expr) ((void)((expr) ? 0 : __assert_fail(__FILE__, __LINE__, #expr)))

void __assert_fail(const char *file, int line, const char *expr);

#endif
