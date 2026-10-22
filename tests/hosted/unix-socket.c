#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    const char *path = "/tmp/c1-hosted-unix.sock";
    int server, peer, client, status;
    struct sockaddr_un address;
    pid_t child;
    char buffer[4];
    unlink(path);
    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server < 0)
        return 1;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) != 0)
        return 2;
    if (listen(server, 1) != 0)
        return 3;
    child = fork();
    if (child < 0)
        return 4;
    if (child == 0) {
        client = socket(AF_UNIX, SOCK_STREAM, 0);
        if (client < 0)
            _exit(1);
        if (connect(client, (struct sockaddr *)&address, sizeof(address)) != 0)
            _exit(2);
        if (send(client, "unix", 4, 0) != 4)
            _exit(3);
        close(client);
        _exit(0);
    }
    peer = accept(server, 0, 0);
    if (peer < 0)
        return 5;
    if (recv(peer, buffer, 4, 0) != 4)
        return 6;
    close(peer);
    close(server);
    unlink(path);
    if (waitpid(child, &status, 0) != child || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return 7;
    puts("hosted UNIX socket ok");
    return 0;
}
