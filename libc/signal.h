#ifndef LIBC_SIGNAL_H
#define LIBC_SIGNAL_H

#include <libc/types.h>

typedef void (*sighandler_t)(int);

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGFPE 8
#define SIGKILL 9
#define SIGSEGV 11
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)

int signal(int signum, sighandler_t handler);
int raise(int signum);

#endif
