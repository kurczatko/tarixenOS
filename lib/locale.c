/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <lib/locale.h>

static struct lconv default_lconv = {
    ".", "", "", "", "",
    ".", "", "", "", "",
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};

struct lconv *localeconv(void) { return &default_lconv; }

char *setlocale(int category, const char *locale) {
    (void)category;
    (void)locale;
    return (char *)"C";
}
