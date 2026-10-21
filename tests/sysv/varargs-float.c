#include <stdarg.h>

double sum(double first, int count, ...) {
    va_list ap;
    int i;
    double total = first;
    va_start(ap, count);
    for (i = 0; i < count; i = i + 1)
        total = total + va_arg(ap, double);
    va_end(ap);
    return total;
}

int main(void) {
    if (sum(0.0, 10, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0) != 55.0)
        return 1;
    return 0;
}
