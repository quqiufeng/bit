#include <stdio.h>
long double add(long double a, long double b) { return a + b; }
int main(void) {
    long double value = add(1.5L, 2.5L);
    if (value != 4.0L)
        return 1;
    printf("long double=%Lf\n", value);
    return 0;
}
