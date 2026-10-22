#ifndef __C1_SYS_SIGNALFD_H
#define __C1_SYS_SIGNALFD_H
#include <signal.h>
#include <stdint.h>
#define SFD_CLOEXEC 0x80000
#define SFD_NONBLOCK 0x800
struct signalfd_siginfo {
    uint32_t ssi_signo;
    unsigned char __data[124];
};
int signalfd(int descriptor, const sigset_t *mask, int flags);
#endif
