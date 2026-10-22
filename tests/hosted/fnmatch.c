#include <fnmatch.h>
#include <stdio.h>

int main(void) {
    if (fnmatch("*.c", "hello.c", 0) != 0)
        return 1;
    if (fnmatch("*.c", "hello.txt", 0) != FNM_NOMATCH)
        return 2;
    if (fnmatch("src/*.c", "src/main.c", FNM_PATHNAME) != 0)
        return 3;
    if (fnmatch("src/*.c", "src/lib/main.c", FNM_PATHNAME) != FNM_NOMATCH)
        return 4;
    puts("hosted fnmatch ok");
    return 0;
}
