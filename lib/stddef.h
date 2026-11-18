#ifndef LIBC_STDDEF_H
#define LIBC_STDDEF_H

#include <lib/types.h>

typedef long ptrdiff_t;
typedef int wchar_t;

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif
