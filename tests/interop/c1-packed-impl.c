#pragma pack(push, 1)
struct packed { char a; int b; short c; };
#pragma pack(pop)
int sum_packed(struct packed p) { return p.a + p.b + p.c; }
struct packed make_packed(char a, int b, short c) {
    struct packed p;
    p.a = a; p.b = b; p.c = c;
    return p;
}
