long double f(long double a, long double b) { return (a - b) * a / b; }
int main(void) {
    long double a = 7.0L, b = 2.0L;
    if (f(a, b) != 17.5L) return 1;
    if ((int)a != 7) return 2;
    if ((double)b != 2.0) return 3;
    if (!(a > b) || !(b < a) || !(a >= b) || !(b <= a)) return 4;
    if (a == b || !(a != b)) return 5;
    return 0;
}
