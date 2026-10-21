#include <stdarg.h>

int consume(int count, va_list ap) {
    int i, total = 0;
    for (i = 0; i < count; i = i + 1)
        total = total + va_arg(ap, int);
    return total;
}

int sum(int count, ...) {
    va_list ap;
    int total;
    va_start(ap, count);
    total = consume(count, ap);
    va_end(ap);
    return total;
}

int main(void) {
    return sum(4, 10, 20, 5, 7) == 42 ? 0 : 1;
}
