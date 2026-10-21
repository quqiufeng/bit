struct wrap { long double value; };
struct wrap make(long double x) {
    struct wrap w;
    w.value = x;
    return w;
}
long double get(struct wrap w) { return w.value; }
int main(void) {
    if (make(42.5L).value != 42.5L)
        return 1;
    if (get(make(42.5L)) != 42.5L)
        return 2;
    return 0;
}
