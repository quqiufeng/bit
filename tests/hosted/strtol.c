#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *end;
    long value = strtol("-12345xyz", &end, 10);
    unsigned long hex = strtoul("ff", &end, 16);
    long long wide = strtoll("9223372036854775807", &end, 10);
    if (value != -12345)
        return 1;
    if (hex != 255)
        return 2;
    if (wide != 9223372036854775807LL)
        return 3;
    printf("strtol=%ld hex=%lu\n", value, hex);
    return 0;
}
