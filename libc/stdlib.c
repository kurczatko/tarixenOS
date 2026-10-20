#include "stdlib.h"
#include "syscall.h"
static uint32_t heap_ptr = 0;
static uint32_t rand_state = 1;
void *malloc(size_t size) {
    if (size == 0) size = 1;
    if (!heap_ptr) heap_ptr = libc_syscall(SYSCALL_BRK, 0, 0, 0);
    uint32_t old = heap_ptr;
    uint32_t np = old + size;
    if (libc_syscall(SYSCALL_BRK, np, 0, 0) == 0) return 0;
    heap_ptr = np;
    return (void *)old;
}
void free(void *p) { (void)p; }
void exit(int code) { libc_syscall(SYSCALL_EXIT, (uint32_t)code, 0, 0); for (;;) { __asm__ volatile ("hlt"); } }
int atoi(const char *s) { int v = 0, neg = 0; while (*s == ' ' || *s == '\t') s++; if (*s == '-') { neg = 1; s++; } while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; } return neg ? -v : v; }
unsigned int rand(void) { rand_state = rand_state * 1103515245u + 12345u; return (rand_state >> 16) & 0x7FFF; }
void srand(unsigned int seed) { rand_state = seed; }
