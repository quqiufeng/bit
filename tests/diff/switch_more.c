int f(int x) {
    int r = 0;
    switch (x) {
    case 0: r += 1;
    case 1: r += 2; break;
    case 2: r += 4; break;
    case 3: r += 8; break;
    default: r += 16; break;
    }
    return r;
}
int g(int x) {
    switch (x) {
    case 10: return 10;
    case 11: return 11;
    case 12: return 12;
    case 13: return 13;
    case 14: return 14;
    case 15: return 15;
    case 16: return 16;
    case 17: return 17;
    case 18: return 18;
    case 19: return 19;
    default: return -1;
    }
}
int main(void) {
    if (f(0) != 3) return 1;
    if (f(1) != 2) return 2;
    if (f(2) != 4) return 3;
    if (f(9) != 16) return 4;
    if (g(10) != 10 || g(19) != 19 || g(20) != -1) return 5;
    return 0;
}
