#include <stdarg.h>
long double sumld(int count, ...) {
    va_list ap;
    int i;
    long double total = 0.0L;
    va_start(ap, count);
    for (i = 0; i < count; i = i + 1)
        total = total + va_arg(ap, long double);
    va_end(ap);
    return total;
}
int main(void) {
    return sumld(3, 10.5L, 20.25L, 11.25L) == 42.0L ? 0 : 1;
}
