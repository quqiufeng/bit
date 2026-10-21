struct triple { int a; int b; int c; };
struct pair2 { long a; long b; };
struct big { int a[8]; };

struct triple make_triple(int a, int b, int c) {
    struct triple p;
    p.a = a; p.b = b; p.c = c;
    return p;
}
int sum_triple(struct triple p) { return p.a + p.b + p.c; }

struct pair2 make_pair2(long a, long b) {
    struct pair2 p;
    p.a = a; p.b = b;
    return p;
}
long sum_pair2(struct pair2 p) { return p.a + p.b; }

struct big make_big(int seed) {
    struct big p;
    int i;
    for (i = 0; i < 8; i = i + 1)
        p.a[i] = seed + i;
    return p;
}
int sum_big(struct big p) {
    int i, total = 0;
    for (i = 0; i < 8; i = i + 1)
        total = total + p.a[i];
    return total;
}
