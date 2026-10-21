int main(void) {
    _Complex long double z;
    long double *p;
    z = 1.5L;
    p = (long double *)&z;
    if (p[0] != 1.5L || p[1] != 0.0L) return 1;
    z = z + z;
    p = (long double *)&z;
    return (p[0] == 3.0L && p[1] == 0.0L) ? 0 : 2;
}
