#include <stdarg.h>

double mixed(int a, double b, int c, double d, int count, ...) {
    va_list ap;
    int i;
    double total = 0.0;
    if (a + c != 4 || b + d != 7.0)
        return -1.0;
    va_start(ap, count);
    for (i = 0; i < count; i = i + 1)
        total = total + va_arg(ap, double);
    va_end(ap);
    return total;
}

int many(int a, int b, int c, int d, int e, int f, int count, ...) {
    va_list ap;
    int i, total = a + b + c + d + e + f;
    va_start(ap, count);
    for (i = 0; i < count; i = i + 1)
        total = total + va_arg(ap, int);
    va_end(ap);
    return total;
}

int main(void) {
    if (mixed(1, 2.5, 3, 4.5, 2, 10.0, 20.0) != 30.0)
        return 1;
    if (many(1, 2, 3, 4, 5, 6, 3, 10, 20, 12) != 63)
        return 2;
    return 0;
}
