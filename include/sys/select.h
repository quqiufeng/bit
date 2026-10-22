#ifndef __C1_SYS_SELECT_H
#define __C1_SYS_SELECT_H
#include <sys/time.h>
typedef struct {
    long __fds_bits[16];
} fd_set;
#define FD_ZERO(set) do { int __i; for (__i = 0; __i < 16; __i++) (set)->__fds_bits[__i] = 0; } while (0)
#define FD_SET(fd, set) ((set)->__fds_bits[(fd) / 64] |= (1L << ((fd) % 64)))
#define FD_CLR(fd, set) ((set)->__fds_bits[(fd) / 64] &= ~(1L << ((fd) % 64)))
#define FD_ISSET(fd, set) (((set)->__fds_bits[(fd) / 64] & (1L << ((fd) % 64))) != 0)
int select(int count, fd_set *read_set, fd_set *write_set, fd_set *except_set,
           struct timeval *timeout);
#endif
