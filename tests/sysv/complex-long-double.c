_Complex long double sum(_Complex long double a, _Complex long double b) {
    return a + b;
}
int main(void) {
    _Complex long double a, b, c, d;
    long double *p;
    if (sizeof(_Complex long double) != 32) return 1;
    p = (long double *)&a; p[0] = 1.0L; p[1] = 2.0L;
    p = (long double *)&b; p[0] = 3.0L; p[1] = 4.0L;
    c = sum(a, b);
    p = (long double *)&c;
    if (p[0] != 4.0L || p[1] != 6.0L) return 2;
    c = a - b;
    p = (long double *)&c;
    if (p[0] != -2.0L || p[1] != -2.0L) return 3;
    c = a * b;
    p = (long double *)&c;
    if (p[0] != -5.0L || p[1] != 10.0L) return 4;
    c = a / b;
    p = (long double *)&c;
    if (p[0] != 0.44L || p[1] != 0.08L) return 5;
    c = -a;
    p = (long double *)&c;
    if (p[0] != -1.0L || p[1] != -2.0L) return 6;
    if (!(a == a) || a != a) return 7;
    if (!a) return 8;
    d = a;
    d = d + b;
    p = (long double *)&d;
    if (p[0] != 4.0L || p[1] != 6.0L) return 9;
    return 0;
}
