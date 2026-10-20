#include "stdio.h"
#include "syscall.h"
#include "stdarg.h"
static void write_char(char c) {
    libc_syscall(SYSCALL_WRITE_CONSOLE, (uint32_t)&c, 1, 0);
}
static void write_str(const char *s) {
    while (*s) write_char(*s++);
}
static void write_uint(unsigned int v, int base, int caps) {
    char buf[32]; int i = 0; if (v == 0) { write_char('0'); return; }
    while (v) { unsigned int d = v % base; buf[i++] = (d < 10) ? (char)('0' + d) : (char)((caps ? 'A' : 'a') + d - 10); v /= base; }
    while (i--) write_char(buf[i]);
}
static void write_int(int v) { if (v < 0) { write_char('-'); v = -v; } write_uint((unsigned)v, 10, 0); }
int printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == '%') { write_char('%'); }
            else if (*fmt == 'c') { write_char((char)va_arg(ap, int)); }
            else if (*fmt == 's') { write_str(va_arg(ap, char *)); }
            else if (*fmt == 'd') { write_int(va_arg(ap, int)); }
            else if (*fmt == 'u') { write_uint(va_arg(ap, unsigned), 10, 0); }
            else if (*fmt == 'x') { write_uint(va_arg(ap, unsigned), 16, 0); }
            else if (*fmt == 'X') { write_uint(va_arg(ap, unsigned), 16, 1); }
        } else { write_char(*fmt); }
        fmt++;
    }
    va_end(ap);
    return 0;
}
int putc(int c) { write_char((char)c); return c; }
int puts(const char *s) { write_str(s); write_char('\n'); return 0; }
