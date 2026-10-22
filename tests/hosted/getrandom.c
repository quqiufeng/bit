#include <stdio.h>
#include <string.h>
#include <sys/random.h>

int main(void) {
    unsigned char buffer[16];
    long got;
    memset(buffer, 0, sizeof(buffer));
    got = getrandom(buffer, sizeof(buffer), 0);
    if (got != (long)sizeof(buffer))
        return 1;
    puts("hosted getrandom ok");
    return 0;
}
