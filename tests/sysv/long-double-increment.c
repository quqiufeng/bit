int main(void) {
    long double x = 1.5L;
    long double a = x++;
    long double b = ++x;
    long double c = x--;
    long double d = --x;
    if (a != 1.5L) return 1;
    if (b != 3.5L) return 2;
    if (c != 3.5L) return 3;
    if (d != 1.5L) return 4;
    if (x != 1.5L) return 5;
    x += 2.5L;
    if (x != 4.0L) return 6;
    return 0;
}
