#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname value;
    if (uname(&value) != 0)
        return 1;
    if (strcmp(value.sysname, "Linux") != 0)
        return 2;
    if (!strstr(value.machine, "x86_64"))
        return 3;
    puts("hosted uname ok");
    return 0;
}
