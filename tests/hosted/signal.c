#include <signal.h>
#include <stdio.h>

static volatile int received;

static void handler(int number) {
    received = number;
}

int main(void) {
    if (signal(SIGUSR1, handler) == SIG_ERR)
        return 1;
    if (raise(SIGUSR1) != 0)
        return 2;
    if (received != SIGUSR1)
        return 3;
    puts("hosted signal callback ok");
    return 0;
}
