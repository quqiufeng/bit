struct ldwrap { long double value; };
long double add_ld(long double, long double);
struct ldwrap wrap_ld(long double);
long double unwrap_ld(struct ldwrap);
long double precise_ld(void);
int main(void) {
    struct ldwrap w = wrap_ld(42.5L);
    if (add_ld(1.5L, 2.5L) != 4.0L)
        return 1;
    if (w.value != 42.5L)
        return 2;
    if (unwrap_ld(w) != 42.5L)
        return 3;
    if (precise_ld() != 1.234567890123456789L)
        return 4;
    return 0;
}
