#include <stdarg.h>
long double named(int a, int b, int c, int d, int e, int f, int g, long double x) {
    return x + a + b + c + d + e + f + g;
}
long double variadic(int a, int b, int c, int d, int e, int f, int g, ...) {
    va_list ap;
    long double x;
    va_start(ap, g);
    x = va_arg(ap, long double);
    va_end(ap);
    return x + a + b + c + d + e + f + g;
}
int main(void) {
    if (named(1,2,3,4,5,6,7,14.0L) != 42.0L) return 1;
    if (variadic(1,2,3,4,5,6,7,14.0L) != 42.0L) return 2;
    return 0;
}
