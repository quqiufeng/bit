#ifndef __C1_SYS_RESOURCE_H
#define __C1_SYS_RESOURCE_H
typedef unsigned long rlim_t;
struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};
#define RLIMIT_CPU 0
#define RLIMIT_FSIZE 1
#define RLIMIT_DATA 2
#define RLIMIT_STACK 3
#define RLIMIT_CORE 4
#define RLIMIT_NOFILE 7
#define RLIM_INFINITY (~0UL)
int getrlimit(int resource, struct rlimit *value);
int setrlimit(int resource, const struct rlimit *value);
#endif
