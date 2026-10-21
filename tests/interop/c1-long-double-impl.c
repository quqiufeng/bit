#include <stdarg.h>
struct ldwrap { long double value; };
long double add_ld(long double a, long double b) { return a + b; }
struct ldwrap wrap_ld(long double value) {
    struct ldwrap w;
    w.value = value;
    return w;
}
long double unwrap_ld(struct ldwrap w) { return w.value; }
long double precise_ld(void) { return 1.234567890123456789L; }
long double stack_named(int a, int b, int c, int d, int e, int f, int g, long double x) {
    return x + a + b + c + d + e + f + g;
}
long double stack_var(int a, int b, int c, int d, int e, int f, int g, ...) {
    va_list ap;
    long double x;
    va_start(ap, g);
    x = va_arg(ap, long double);
    va_end(ap);
    return x + a + b + c + d + e + f + g;
}
