#include <stdio.h>
#include <sys/resource.h>

int main(void) {
    struct rlimit value;
    if (getrlimit(RLIMIT_NOFILE, &value) != 0)
        return 1;
    if (value.rlim_cur == 0)
        return 2;
    puts("hosted getrlimit ok");
    return 0;
}
