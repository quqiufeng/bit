#include <errno.h>
#include <stdio.h>

int main(void) {
    errno = 0;
    if (errno != 0)
        return 1;
    errno = EINVAL;
    if (errno != EINVAL)
        return 2;
    puts("hosted errno/TLS ok");
    return 0;
}
