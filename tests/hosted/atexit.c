#include <stdio.h>
#include <stdlib.h>

static void finished(void) {
    puts("hosted atexit callback ok");
}

int main(void) {
    if (atexit(finished) != 0)
        return 1;
    if (!getenv("PATH"))
        return 2;
    return 0;
}
