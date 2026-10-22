#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int sockets[2];
    int epoll;
    struct epoll_event event;
    struct epoll_event result;
    if (sizeof(struct epoll_event) != 12)
        return 1;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
        return 2;
    epoll = epoll_create1(EPOLL_CLOEXEC);
    if (epoll < 0)
        return 3;
    event.events = EPOLLIN;
    event.data = (unsigned long long)sockets[1];
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, sockets[1], &event) != 0)
        return 4;
    if (write(sockets[0], "e", 1) != 1)
        return 5;
    if (epoll_wait(epoll, &result, 1, 1000) != 1)
        return 6;
    if (!(result.events & EPOLLIN) || result.data != (unsigned long long)sockets[1])
        return 7;
    close(epoll);
    close(sockets[0]);
    close(sockets[1]);
    puts("hosted epoll ok");
    return 0;
}
