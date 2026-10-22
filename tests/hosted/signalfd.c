#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

int main(void) {
    sigset_t mask;
    struct signalfd_siginfo info;
    int descriptor;
    memset(&info, 0, sizeof(info));
    if (sigemptyset(&mask) != 0)
        return 1;
    if (sigaddset(&mask, SIGUSR1) != 0)
        return 2;
    if (sigprocmask(SIG_BLOCK, &mask, 0) != 0)
        return 3;
    descriptor = signalfd(-1, &mask, SFD_CLOEXEC);
    if (descriptor < 0)
        return 4;
    if (raise(SIGUSR1) != 0)
        return 5;
    if (read(descriptor, &info, sizeof(info)) != (long)sizeof(info))
        return 6;
    close(descriptor);
    if (info.ssi_signo != SIGUSR1)
        return 7;
    puts("hosted signalfd ok");
    return 0;
}
