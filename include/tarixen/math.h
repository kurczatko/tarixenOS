#ifndef TARIXEN_MATH_H
#define TARIXEN_MATH_H

#include <include/printf/types.h>

static inline int32_t math_abs(int32_t x)
{
    return x < 0 ? -x : x;
}

static inline uint32_t math_min_u32(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

static inline uint32_t math_max_u32(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

static inline int32_t math_min_i32(int32_t a, int32_t b)
{
    return a < b ? a : b;
}

static inline int32_t math_max_i32(int32_t a, int32_t b)
{
    return a > b ? a : b;
}

static inline uint32_t math_isqrt(uint32_t n)
{
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

static inline double math_sin(double x)
{
    while (x > 3.14159265358979323846) x -= 6.28318530717958647692;
    while (x < -3.14159265358979323846) x += 6.28318530717958647692;
    double x2 = x * x;
    double result = x * (1.0 - x2 * (1.0 / 6.0 - x2 * (1.0 / 120.0 - x2 / 5040.0)));
    return result;
}

static inline double math_cos(double x)
{
    return math_sin(x + 3.14159265358979323846 / 2.0);
}

#endif
