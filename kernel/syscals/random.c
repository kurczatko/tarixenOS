#include "random.h"

static uint64_t rng_state = 0xACE123456789ULL;
static int rdrand_supported = -1;

static int check_rdrand_support(void) {
    uint32_t eax, ebx, ecx, edx;
    eax = 1;

    asm volatile (
        "cpuid" // sprawdza wsparcie do rdrand (kto kurwa wymyslil te zjebana nazwe)
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
    );

    return (ecx & (1 << 30)) != 0;
}

uint32_t get_hardware_random(void) { // funkcja do generowania losowych liczb przy pomocy rdrand
    uint32_t val;

    if (rdrand_supported == -1) {
        rdrand_supported = check_rdrand_support();
    }

    if (rdrand_supported) {
        asm volatile ("rdrand %0" : "=r" (val));
        return val;
    }

    return 0;
}

uint64_t rdtsc(void) {
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static uint32_t xorshift32_next(void) {
    uint64_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    rng_state = x;
    return (uint32_t)x;
}

uint32_t random(void) { //glowny syscal
    uint32_t hw_rand = get_hardware_random();
    if (hw_rand != 0) {
        return hw_rand;
    }

    if (rng_state == 0xACE123456789ULL || rng_state == 0) {
        rng_state ^= rdtsc();
    }

    rng_state ^= rdtsc();

    return xorshift32_next();
}