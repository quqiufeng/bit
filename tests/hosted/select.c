#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int sockets[2];
    fd_set readable;
    char buffer[4];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
        return 1;
    if (write(sockets[0], "hi", 2) != 2)
        return 2;
    FD_ZERO(&readable);
    FD_SET(sockets[1], &readable);
    if (select(sockets[1] + 1, &readable, 0, 0, 0) != 1)
        return 3;
    if (!FD_ISSET(sockets[1], &readable))
        return 4;
    if (read(sockets[1], buffer, 2) != 2)
        return 5;
    close(sockets[0]);
    close(sockets[1]);
    puts("hosted select ok");
    return 0;
}
