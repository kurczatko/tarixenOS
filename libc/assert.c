/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <libc/assert.h>
#include <libc/stdio.h>

void __assert_fail(const char *file, int line, const char *expr) {
    printf("Assertion failed: %s, file %s, line %d\n", expr, file, line);
    for (;;) { __asm__ volatile ("hlt"); }
}
