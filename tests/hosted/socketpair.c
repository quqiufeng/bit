#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    int sockets[2];
    char buffer[8];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
        return 1;
    if (write(sockets[0], "hello", 5) != 5)
        return 2;
    if (read(sockets[1], buffer, 5) != 5)
        return 3;
    buffer[5] = 0;
    close(sockets[0]);
    close(sockets[1]);
    if (strcmp(buffer, "hello") != 0)
        return 4;
    puts("hosted socketpair ok");
    return 0;
}
