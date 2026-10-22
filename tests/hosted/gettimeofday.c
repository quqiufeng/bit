#include <stdio.h>
#include <sys/time.h>

int main(void) {
    struct timeval value;
    if (gettimeofday(&value, 0) != 0)
        return 1;
    if (value.tv_sec <= 0)
        return 2;
    if (value.tv_usec < 0 || value.tv_usec >= 1000000)
        return 3;
    puts("hosted gettimeofday ok");
    return 0;
}
