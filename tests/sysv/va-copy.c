#include <stdarg.h>

int pair_sum(int count, ...) {
    va_list first, second;
    int a, b, c;
    va_start(first, count);
    va_copy(second, first);
    a = va_arg(first, int);
    b = va_arg(second, int);
    c = va_arg(second, int);
    va_end(first);
    va_end(second);
    return a == 20 && b == 20 && c == 22 ? 0 : 1;
}

int main(void) {
    return pair_sum(2, 20, 22);
}
