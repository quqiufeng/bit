#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

int main(void) {
    const char *path = "/tmp/c1-hosted-inotify.txt";
    char buffer[4096];
    int descriptor, file, watch;
    long got;
    unlink(path);
    descriptor = inotify_init1(IN_CLOEXEC);
    if (descriptor < 0)
        return 1;
    watch = inotify_add_watch(descriptor, "/tmp", IN_MODIFY | IN_CLOSE_WRITE | IN_CREATE);
    if (watch < 0)
        return 2;
    file = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (file < 0)
        return 3;
    if (write(file, "x", 1) != 1)
        return 4;
    close(file);
    got = read(descriptor, buffer, sizeof(buffer));
    if (got <= 0)
        return 5;
    inotify_rm_watch(descriptor, watch);
    close(descriptor);
    unlink(path);
    puts("hosted inotify ok");
    return 0;
}
