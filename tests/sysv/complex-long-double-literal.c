#include <complex.h>
int main(void) {
    _Complex long double z = 1.0L + 2.0L * I;
    long double *p = (long double *)&z;
    if (p[0] != 1.0L || p[1] != 2.0L) return 1;
    _Complex double d = 3.0 + 4.0 * I;
    z = d;
    p = (long double *)&z;
    if (p[0] != 3.0L || p[1] != 4.0L) return 2;
    d = z;
    double *q = (double *)&d;
    if (q[0] != 3.0 || q[1] != 4.0) return 3;
    return 0;
}
