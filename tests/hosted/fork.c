#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int status;
    pid_t child = fork();
    if (child < 0)
        return 1;
    if (child == 0)
        _exit(42);
    if (waitpid(child, &status, 0) != child)
        return 2;
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 42)
        return 3;
    puts("hosted fork/waitpid ok");
    return 0;
}
