#ifndef __C1_SYS_TIMERFD_H
#define __C1_SYS_TIMERFD_H
#include <stdint.h>
#ifndef __C1_TIMESPEC_DEFINED
#define __C1_TIMESPEC_DEFINED
struct timespec {
    long tv_sec;
    long tv_nsec;
};
#endif
struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};
#define TFD_CLOEXEC 0x80000
#define TFD_NONBLOCK 0x800
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
int timerfd_create(int clock, int flags);
int timerfd_settime(int descriptor, int flags, const struct itimerspec *new_value,
                    struct itimerspec *old_value);
int timerfd_gettime(int descriptor, struct itimerspec *value);
#endif
