#ifndef __C1_SYS_TIME_H
#define __C1_SYS_TIME_H
#include <sys/types.h>
struct timeval {
    time_t tv_sec;
    long tv_usec;
};
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
int gettimeofday(struct timeval *value, void *timezone);
int settimeofday(const struct timeval *value, const struct timezone *timezone);
#endif
