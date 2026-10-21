#include <float.h>

int main(void) {
    if (sizeof(float) != 4 || sizeof(double) != 8)
        return 1;
    if (FLT_MANT_DIG != 24 || DBL_MANT_DIG != 53)
        return 2;
    if (FLT_RADIX != 2)
        return 3;
    if (!(FLT_MAX > 1.0e38F) || !(DBL_MAX > 1.0e308))
        return 4;
    if (!(FLT_MIN < 1.0e-37F) || !(DBL_MIN < 1.0e-307))
        return 5;
    if (!(FLT_EPSILON < 1.0e-6F) || !(DBL_EPSILON < 1.0e-15))
        return 6;
    return 0;
}
