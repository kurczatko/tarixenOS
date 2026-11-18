#ifndef TARIXEN_MSTUFF_H
#define TARIXEN_MSTUFF_H

#include <include/printf/types.h>

#define M_ALIGN 16
#define M_ALIGN_MASK (M_ALIGN - 1)

static inline uint32_t m_align_up(uint32_t v)
{
    return (v + M_ALIGN_MASK) & ~M_ALIGN_MASK;
}

static inline void *m_memcpy(void *dst, const void *src, uint32_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    uint32_t i = 0;

    if (((uint32_t)d | (uint32_t)s | n) & 3) {
        while (i < n) { d[i] = s[i]; i++; }
        return dst;
    }

    uint32_t *dd = (uint32_t *)d;
    const uint32_t *ss = (const uint32_t *)s;
    while (i + 3 < n) { dd[i / 4] = ss[i / 4]; i += 4; }
    while (i < n) { d[i] = s[i]; i++; }
    return dst;
}

static inline void *m_memset(void *s, int c, uint32_t n)
{
    unsigned char *p = (unsigned char *)s;
    unsigned char uc = (unsigned char)c;
    uint32_t i = 0;

    if (((uint32_t)p | n) & 3) {
        while (i < n) { p[i] = uc; i++; }
        return s;
    }

    uint32_t fill = (uint32_t)uc | ((uint32_t)uc << 8) |
                    ((uint32_t)uc << 16) | ((uint32_t)uc << 24);
    uint32_t *pp = (uint32_t *)p;
    while (i + 3 < n) { pp[i / 4] = fill; i += 4; }
    while (i < n) { p[i] = uc; i++; }
    return s;
}

static inline void *m_memmove(void *dst, const void *src, uint32_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        return m_memcpy(d, s, n);
    } else if (d > s) {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

static inline int m_memcmp(const void *a, const void *b, uint32_t n)
{
    const unsigned char *aa = (const unsigned char *)a;
    const unsigned char *bb = (const unsigned char *)b;
    while (n--) {
        if (*aa != *bb) return (int)*aa - (int)*bb;
        aa++; bb++;
    }
    return 0;
}

void *m_malloc(uint32_t size);
void m_free(void *ptr);
void *m_realloc(void *ptr, uint32_t size);
void *m_calloc(uint32_t count, uint32_t size);

#endif
