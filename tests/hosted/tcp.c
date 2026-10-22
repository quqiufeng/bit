#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int server, peer, client, status;
    struct sockaddr_in address;
    socklen_t length;
    char buffer[8];
    pid_t child;
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
        return 1;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) != 0)
        return 2;
    if (listen(server, 1) != 0)
        return 3;
    length = sizeof(address);
    if (getsockname(server, (struct sockaddr *)&address, &length) != 0)
        return 4;
    child = fork();
    if (child < 0)
        return 5;
    if (child == 0) {
        client = socket(AF_INET, SOCK_STREAM, 0);
        if (client < 0)
            _exit(1);
        if (connect(client, (struct sockaddr *)&address, sizeof(address)) != 0)
            _exit(2);
        if (send(client, "hello", 5, 0) != 5)
            _exit(3);
        close(client);
        _exit(0);
    }
    peer = accept(server, 0, 0);
    if (peer < 0)
        return 6;
    if (recv(peer, buffer, 5, 0) != 5)
        return 7;
    buffer[5] = 0;
    close(peer);
    close(server);
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return 8;
    if (strcmp(buffer, "hello") != 0)
        return 9;
    puts("hosted TCP loopback ok");
    return 0;
}
