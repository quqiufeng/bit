#include <stdint.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

int main(void) {
    struct itimerspec timer;
    uint64_t expirations = 0;
    int descriptor;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_nsec = 10000000;
    descriptor = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (descriptor < 0)
        return 1;
    if (timerfd_settime(descriptor, 0, &timer, 0) != 0)
        return 2;
    if (read(descriptor, &expirations, sizeof(expirations)) != (long)sizeof(expirations))
        return 3;
    close(descriptor);
    if (expirations == 0)
        return 4;
    puts("hosted timerfd ok");
    return 0;
}
