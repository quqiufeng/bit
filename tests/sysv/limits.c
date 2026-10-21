#include <limits.h>
#include <stddef.h>
#include <stdint.h>

int main(void) {
    if (sizeof(int8_t) != 1 || sizeof(int16_t) != 2)
        return 1;
    if (sizeof(int32_t) != 4 || sizeof(int64_t) != 8)
        return 2;
    if (sizeof(long) != 8 || sizeof(void *) != 8)
        return 3;
    if (sizeof(size_t) != 8)
        return 4;
    if (INT_MAX != 2147483647)
        return 5;
    if (UINT_MAX != 4294967295U)
        return 6;
    if (LONG_MAX != 9223372036854775807L)
        return 7;
    if (sizeof(uintptr_t) != 8)
        return 8;
    return 0;
}
