#include <math.h>
#include <stdio.h>

int main(void) {
    int exponent;
    double integer;
    if (sqrt(9.0) != 3.0)
        return 1;
    if (pow(2.0, 10.0) != 1024.0)
        return 2;
    if (floor(1.9) != 1.0 || ceil(1.1) != 2.0 || trunc(-1.9) != -1.0)
        return 3;
    if (fmod(7.0, 3.0) != 1.0 || fabs(-2.0) != 2.0)
        return 4;
    if (ldexp(1.0, 3) != 8.0 || frexp(8.0, &exponent) != 0.5 || exponent != 4)
        return 5;
    if (modf(3.5, &integer) != 0.5 || integer != 3.0)
        return 6;
    puts("hosted math ok");
    return 0;
}
