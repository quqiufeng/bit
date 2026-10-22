#include <stdarg.h>
int sum(int n, ...) {
    va_list ap;
    int i, s = 0;
    va_start(ap, n);
    for (i = 0; i < n; i++) s += va_arg(ap, int);
    va_end(ap);
    return s;
}
int main(void) {
    if (sum(0) != 0) return 1;
    if (sum(1, 7) != 7) return 2;
    if (sum(6, 1, 2, 3, 4, 5, 6) != 21) return 3;
    return 0;
}
