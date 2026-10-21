#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    const char *path = "/tmp/c1-hosted-stat.txt";
    struct stat value;
    int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0600);
    if (fd < 0)
        return 1;
    if (write(fd, "hello", 5) != 5)
        return 2;
    close(fd);
    if (stat(path, &value) != 0)
        return 3;
    if (!S_ISREG(value.st_mode))
        return 4;
    if (value.st_size != 5)
        return 5;
    if (stat("/tmp", &value) != 0)
        return 6;
    if (!S_ISDIR(value.st_mode))
        return 7;
    unlink(path);
    puts("hosted stat ok");
    return 0;
}
