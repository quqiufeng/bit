#include <stdio.h>
#pragma pack(push, 1)
struct packed { char a; int b; short c; };
#pragma pack(pop)
int sum_packed(struct packed);
struct packed make_packed(char, int, short);
int main(void) {
    struct packed p = make_packed(1, 2, 3);
    printf("%d %d\n", (int)sizeof(struct packed), sum_packed(p));
    return sizeof(struct packed) == 7 && p.a == 1 && p.b == 2 && p.c == 3 && sum_packed(p) == 6 ? 0 : 1;
}
