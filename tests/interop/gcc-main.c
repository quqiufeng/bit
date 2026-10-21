#include <stdio.h>
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
    printf("%d %ld %d\n", sum_triple(t), sum_pair2(p), sum_big(b));
    return sum_triple(t) == 42 && sum_pair2(p) == 42 && sum_big(b) == 36 ? 0 : 1;
}
