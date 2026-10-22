#include <stdio.h>
#include <sys/statvfs.h>

int main(void) {
    struct statvfs value;
    if (sizeof(struct statvfs) != 112)
        return 1;
    if (statvfs("/tmp", &value) != 0)
        return 2;
    if (value.f_bsize == 0 || value.f_blocks == 0)
        return 3;
    puts("hosted statvfs ok");
    return 0;
}
