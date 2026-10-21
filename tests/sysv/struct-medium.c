struct triple { int a; int b; int c; };
struct pair2 { long a; long b; };
int f(int a, int b, int c, int d, int e, struct triple p) {
    return a + b + c + d + e + p.a + p.b + p.c;
}
struct triple make(void) {
    struct triple p;
    p.a = 10;
    p.b = 20;
    p.c = 12;
    return p;
}
struct pair2 makepair(long a, long b) {
    struct pair2 p;
    p.a = a;
    p.b = b;
    return p;
}
long sumpair(struct pair2 p) { return p.a + p.b; }
int main(void) {
    struct triple t = make();
    struct pair2 p = makepair(20, 22);
    if (f(1, 2, 3, 4, 5, t) != 57)
        return 1;
    if (sumpair(p) != 42)
        return 2;
    return 0;
}
