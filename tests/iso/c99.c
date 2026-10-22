struct P { int x; int y; };
static int sum(const int *p, int n) {
    int i, s = 0;
    for (i = 0; i < n; i++) s += p[i];
    return s;
}
int main(void) {
    long long big = 9000000000LL;
    _Bool flag = 1;
    int a[4] = { [2] = 7, [0] = 3 };
    struct P p = { .y = 5, .x = 2 };
    int *q = (int[]){ 1, 2, 3 };
    // C99 line comment
    if (big != 9000000000LL) return 1;
    if (!flag) return 2;
    if (a[0] != 3 || a[2] != 7 || a[3] != 0) return 3;
    if (p.x != 2 || p.y != 5) return 4;
    if (sum(q, 3) != 6) return 5;
    return 0;
}
