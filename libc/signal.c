/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <libc/signal.h>

int signal(int signum, sighandler_t handler) {
    (void)signum;
    (void)handler;
    return 0;
}

int raise(int signum) {
    (void)signum;
    return 0;
}
