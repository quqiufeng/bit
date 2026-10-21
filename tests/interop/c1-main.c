struct triple { int a; int b; int c; };
struct pair2 { long a; long b; };
struct big { int a[8]; };
struct triple make_triple(int, int, int);
int sum_triple(struct triple);
struct pair2 make_pair2(long, long);
long sum_pair2(struct pair2);
struct big make_big(int);
int sum_big(struct big);
int main(void) {
    struct triple t = make_triple(10, 20, 12);
    struct pair2 p = make_pair2(20, 22);
    struct big b = make_big(1);
    if (sum_triple(t) != 42)
        return 1;
    if (sum_pair2(p) != 42)
        return 2;
    if (sum_big(b) != 36)
        return 3;
    return 0;
}
