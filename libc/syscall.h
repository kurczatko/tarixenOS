#ifndef LIBC_SYSCALL_H
#define LIBC_SYSCALL_H
#include "types.h"
#define SYSCALL_WRITE_CONSOLE 20
#define SYSCALL_EXIT 21
#define SYSCALL_BRK 22
#define SYSCALL_OPEN 23
#define SYSCALL_READ 24
#define SYSCALL_CLOSE 25
#define SYSCALL_WRITE_FILE 26
static inline uint32_t libc_syscall(uint32_t n, uint32_t a1, uint32_t a2, uint32_t a3) {
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a1), "c"(a2), "d"(a3) : "memory");
    return ret;
}
#endif
