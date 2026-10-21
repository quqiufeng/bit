struct ldwrap { long double value; };
long double add_ld(long double a, long double b) { return a + b; }
struct ldwrap wrap_ld(long double value) {
    struct ldwrap w;
    w.value = value;
    return w;
}
long double unwrap_ld(struct ldwrap w) { return w.value; }
long double precise_ld(void) { return 1.234567890123456789L; }
