#ifndef __C1_SYS_EPOLL_H
#define __C1_SYS_EPOLL_H
#include <stdint.h>
#pragma pack(push, 1)
struct epoll_event {
    uint32_t events;
    uint64_t data;
};
#pragma pack(pop)
#define EPOLLIN 0x001
#define EPOLLOUT 0x004
#define EPOLLERR 0x008
#define EPOLLHUP 0x010
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3
#define EPOLL_CLOEXEC 0x80000
int epoll_create1(int flags);
int epoll_ctl(int epoll, int operation, int descriptor, struct epoll_event *event);
int epoll_wait(int epoll, struct epoll_event *events, int maximum, int timeout);
#endif
