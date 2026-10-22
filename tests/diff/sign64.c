int main(void) {
    long long s = -1;
    unsigned long long u = 18446744073709551615ULL;
    unsigned int u32 = 4294967295U;
    signed char c = (signed char)0x80;
    unsigned short us = (unsigned short)0xffff;
    if ((long long)u32 != 4294967295LL) return 1;
    if ((unsigned long long)s != 18446744073709551615ULL) return 2;
    if (c != -128) return 3;
    if ((long long)c != -128LL) return 4;
    if (us != 65535U) return 5;
    if ((long long)us != 65535LL) return 6;
    if ((s >> 63) != -1LL) return 7;
    if ((u >> 63) != 1ULL) return 8;
    if ((long long)(u >> 1) != 9223372036854775807LL) return 9;
    if ((int)0x80000000U != (-2147483647 - 1)) return 10;
    return 0;
}
