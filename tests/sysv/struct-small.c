struct pair { int a; int b; };
struct pair make(int a, int b) {
    struct pair p;
    p.a = a;
    p.b = b;
    return p;
}
int sum(struct pair p) { return p.a + p.b; }
int main(void) {
    struct pair p = make(20, 22);
    return sum(p) == 42 ? 0 : 1;
}
