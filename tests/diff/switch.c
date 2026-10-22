int classify(int x) {
    switch (x) {
    case -7: return 1;
    case -1: return 2;
    case 0: return 3;
    case 1: return 4;
    case 7: return 5;
    case 100: return 6;
    default: return 9;
    }
}
int main(void) {
    int i, s = 0;
    for (i = -20; i <= 120; i++) s += classify(i);
    switch (s) {
    case 7: return 0;
    case 8: return 1;
    default: return 2;
    }
}
