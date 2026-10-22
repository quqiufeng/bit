#ifndef __C1_SIGNAL_H
#define __C1_SIGNAL_H
typedef void (*sighandler_t)(int);
typedef struct {
    unsigned long __val[16];
} sigset_t;
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
int sigemptyset(sigset_t *set);
int sigaddset(sigset_t *set, int number);
int sigprocmask(int operation, const sigset_t *set, sigset_t *old_set);
sighandler_t signal(int number, sighandler_t handler);
int raise(int number);
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)
#define SIGINT 2
#define SIGILL 4
#define SIGABRT 6
#define SIGFPE 8
#define SIGSEGV 11
#define SIGTERM 15
#define SIGUSR1 10
#define SIGUSR2 12
#endif
