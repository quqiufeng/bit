#include <inttypes.h>
#include <stdio.h>

int main(void) {
    int64_t value = -42;
    uint64_t other = 42;
    intmax_t maximum = 7;
    printf("%" PRId64 " %" PRIu64 " %" PRIdPTR " %ld\n", value, other, maximum, (long)sizeof(intmax_t));
    return value == -42 && other == 42 && sizeof(intmax_t) == 8 ? 0 : 1;
}
