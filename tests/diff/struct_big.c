struct big { long a; long b; long c; };
struct ld { long double x; long double y; };
struct big make_big(long a, long b, long c) { struct big x; x.a=a; x.b=b; x.c=c; return x; }
long sum_big(struct big x) { return x.a + x.b + x.c; }
struct ld make_ld(long double x, long double y) { struct ld r; r.x=x; r.y=y; return r; }
long double sum_ld(struct ld x) { return x.x + x.y; }
int main(void) {
    struct big b = make_big(1000000000L, 2000000000L, 3000000000L);
    struct ld d = make_ld(1.5L, 2.25L);
    if (sum_big(b) != 6000000000L) return 1;
    if (sum_ld(d) != 3.75L) return 2;
    return 0;
}
