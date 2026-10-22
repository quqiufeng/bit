#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int sockets[2];
    struct pollfd descriptor;
    char buffer[4];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
        return 1;
    if (write(sockets[0], "ok", 2) != 2)
        return 2;
    descriptor.fd = sockets[1];
    descriptor.events = POLLIN;
    descriptor.revents = 0;
    if (poll(&descriptor, 1, 0) != 1)
        return 3;
    if (!(descriptor.revents & POLLIN))
        return 4;
    if (read(sockets[1], buffer, 2) != 2)
        return 5;
    close(sockets[0]);
    close(sockets[1]);
    puts("hosted poll ok");
    return 0;
}
