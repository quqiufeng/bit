#pragma pack(push, 1)
struct packed { char a; int b; short c; };
#pragma pack(pop)
struct normal { char a; int b; short c; };
int sum(struct packed p) { return p.a + p.b + p.c; }
struct packed make(char a, int b, short c) {
    struct packed p;
    p.a = a; p.b = b; p.c = c;
    return p;
}
int main(void) {
    struct packed p = make(1, 2, 3);
    struct normal n;
    if (sizeof(struct packed) != 7)
        return 1;
    if (sizeof(struct normal) != 12)
        return 2;
    if (sum(p) != 6)
        return 3;
    n.a = 1; n.b = 2; n.c = 3;
    return n.a + n.b + n.c == 6 ? 0 : 4;
}
