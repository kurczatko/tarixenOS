#include "string.h"
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (unsigned char)*a - (unsigned char)*b; }
char *strcpy(char *d, const char *s) { char *o = d; while (*s) { *d++ = *s++; } *d = 0; return o; }
char *strcat(char *d, const char *s) { char *o = d; while (*d) d++; while (*s) { *d++ = *s++; } *d = 0; return o; }
void *memcpy(void *d, const void *s, size_t n) { unsigned char *dd = d; const unsigned char *ss = s; while (n--) *dd++ = *ss++; return d; }
void *memset(void *s, int c, size_t n) { unsigned char *ss = s; while (n--) *ss++ = (unsigned char)c; return s; }
void *memmove(void *d, const void *s, size_t n) { unsigned char *dd = d; const unsigned char *ss = s; if (dd < ss) { while (n--) *dd++ = *ss++; } else if (dd > ss) { dd += n; ss += n; while (n--) *--dd = *--ss; } return d; }
char *strchr(const char *s, int c) { while (*s) { if (*s == (char)c) return (char *)s; s++; } return 0; }
char *strstr(const char *h, const char *n) { size_t nl = strlen(n); if (!nl) return (char *)h; while (*h) { if (!memcmp(h, n, nl)) return (char *)h; h++; } return 0; }
int memcmp(const void *a, const void *b, size_t n) { const unsigned char *aa = a; const unsigned char *bb = b; while (n--) { if (*aa != *bb) return *aa - *bb; aa++; bb++; } return 0; }
