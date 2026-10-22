int main(void) {
    signed char c = (signed char)0xff;
    unsigned char uc = (unsigned char)0xff;
    short s = (short)0x8000;
    unsigned short us = (unsigned short)0x8000;
    int i = -1;
    unsigned int u = 0xffffffffU;
    long l;
    if (c != -1) return 1;
    if (uc != 255) return 2;
    if (s != -32768) return 3;
    if (us != 32768) return 4;
    if ((long)c != -1L) return 5;
    if ((long)uc != 255L) return 6;
    if ((long)i != -1L) return 7;
    if ((long)u != 4294967295L) return 8;
    if ((i >> 1) != -1) return 9;
    if ((u >> 1) != 2147483647U) return 10;
    l = (long)i;
    if (l != -1L) return 11;
    if ((int)0x80000000U != (-2147483647 - 1)) return 12;
    return 0;
}
