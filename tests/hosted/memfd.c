#include <stdio.h>
#include <string.h>
#include <sys/memfd.h>
#include <unistd.h>

int main(void) {
    char buffer[8];
    int descriptor = memfd_create("c1-memfd", MFD_CLOEXEC);
    if (descriptor < 0)
        return 1;
    if (write(descriptor, "hello", 5) != 5)
        return 2;
    if (lseek(descriptor, 0, 0) != 0)
        return 3;
    if (read(descriptor, buffer, 5) != 5)
        return 4;
    close(descriptor);
    buffer[5] = 0;
    if (strcmp(buffer, "hello") != 0)
        return 5;
    puts("hosted memfd ok");
    return 0;
}
