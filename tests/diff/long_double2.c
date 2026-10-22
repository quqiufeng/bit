long double add(long double a, long double b) { return a + b; }
long double mul(long double a, long double b) { return a * b; }
long double divi(long double a, long double b) { return a / b; }
int main(void) {
    long double a = 1.234567890123456789L;
    long double b = 0.000000000000000001L;
    if (add(a, b) <= a) return 1;
    if (mul(a, 2.0L) != 2.0L * a) return 2;
    if (divi(a, 2.0L) * 2.0L != a) return 3;
    if ((long double)(long long)a != 1.0L) return 4;
    return 0;
}
