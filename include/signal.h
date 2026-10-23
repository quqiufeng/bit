#ifndef __C1_SIGNAL_H
#define __C1_SIGNAL_H
typedef void (*sighandler_t)(int);
typedef struct {
    unsigned long __val[16];
} sigset_t;
typedef struct {
    int si_signo;
    int si_errno;
    int si_code;
    int __pad;
} siginfo_t;
struct sigaction {
    void (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
int sigemptyset(sigset_t *set);
int sigaddset(sigset_t *set, int number);
int sigprocmask(int operation, const sigset_t *set, sigset_t *old_set);
sighandler_t signal(int number, sighandler_t handler);
int raise(int number);
int sigaction(int number, const struct sigaction *action, struct sigaction *old_action);
#define SA_SIGINFO 4
#define SA_RESETHAND 0x80000000
#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)
#define SIGINT 2
#define SIGILL 4
#define SIGABRT 6
#define SIGFPE 8
#define SIGBUS 7
#define FPE_INTDIV 1
#define FPE_INTOVF 2
#define FPE_FLTDIV 3
#define FPE_FLTOVF 4
#define FPE_FLTUND 5
#define FPE_FLTRES 6
#define FPE_FLTINV 7
#define FPE_FLTSUB 8
#define SIGSEGV 11
#define SIGTERM 15
#define SIGUSR1 10
#define SIGUSR2 12
#endif
