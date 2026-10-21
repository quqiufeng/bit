#include <stdarg.h>

int sum(int count, ...) {
    va_list ap;
    int i, total = 0;
    va_start(ap, count);
    for (i = 0; i < count; i = i + 1)
        total = total + va_arg(ap, int);
    va_end(ap);
    return total;
}

int main(void) {
    if (sum(3, 10, 20, 12) != 42)
        return 1;
    if (sum(8, 1, 2, 3, 4, 5, 6, 7, 8) != 36)
        return 2;
    return 0;
}
