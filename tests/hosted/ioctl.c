#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int sockets[2];
    int available = 0;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
        return 1;
    if (write(sockets[0], "hello", 5) != 5)
        return 2;
    if (ioctl(sockets[1], FIONREAD, &available) != 0)
        return 3;
    if (available != 5)
        return 4;
    close(sockets[0]);
    close(sockets[1]);
    puts("hosted ioctl ok");
    return 0;
}
