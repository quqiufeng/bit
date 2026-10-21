long double calc(long double a, long double b) { return (a - b) * a / b; }
long double from_int(int x) { return x; }
int to_int(long double x) { return (int)x; }
double to_double(long double x) { return (double)x; }
int main(void) {
    long double a = 7.0L, b = 2.0L;
    if (calc(a, b) != 17.5L)
        return 1;
    if (from_int(42) != 42.0L)
        return 2;
    if (to_int(42.75L) != 42)
        return 3;
    if (to_double(1.5L) != 1.5)
        return 4;
    if (!(a > b) || !(b < a) || !(a >= b) || !(b <= a) || a == b || !(a != b))
        return 5;
    return 0;
}
