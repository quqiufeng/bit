#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    const char *path = "/tmp/c1-hosted-posix-io.txt";
    char buffer[8];
    int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0600);
    if (fd < 0)
        return 1;
    if (write(fd, "hello", 5) != 5)
        return 2;
    if (lseek(fd, 0, SEEK_SET) != 0)
        return 3;
    if (read(fd, buffer, 5) != 5)
        return 4;
    buffer[5] = 0;
    close(fd);
    unlink(path);
    if (strcmp(buffer, "hello") != 0)
        return 5;
    puts("hosted POSIX I/O ok");
    return 0;
}
