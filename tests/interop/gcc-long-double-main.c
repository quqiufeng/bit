#include <stdio.h>
struct ldwrap { long double value; };
long double add_ld(long double, long double);
struct ldwrap wrap_ld(long double);
long double unwrap_ld(struct ldwrap);
long double precise_ld(void);
long double stack_named(int, int, int, int, int, int, int, long double);
long double stack_var(int, int, int, int, int, int, int, ...);
int main(void) {
    struct ldwrap w = wrap_ld(42.5L);
    printf("%Lf %Lf\n", add_ld(1.5L, 2.5L), unwrap_ld(w));
    return add_ld(1.5L, 2.5L) == 4.0L && w.value == 42.5L && unwrap_ld(w) == 42.5L &&
                   precise_ld() == 1.234567890123456789L &&
                   stack_named(1, 2, 3, 4, 5, 6, 7, 14.0L) == 42.0L &&
                   stack_var(1, 2, 3, 4, 5, 6, 7, 14.0L) == 42.0L
               ? 0
               : 1;
}
