int main(void) {
    _Complex long double z;
    _Complex double d;
    double *dp;
    __real__ z = 1.5L;
    __imag__ z = 2.5L;
    if (__real__ z != 1.5L) return 1;
    if (__imag__ z != 2.5L) return 2;
    __real__ d = 3.0;
    __imag__ d = 4.0;
    dp = (double *)&d;
    if (dp[0] != 3.0 || dp[1] != 4.0) return 3;
    return 0;
}
