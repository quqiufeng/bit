struct small { int a; char b; short c; };
struct pair { long a; long b; };
struct mixed { int a; double b; };
struct small make_small(int a, char b, short c) { struct small s; s.a=a; s.b=b; s.c=c; return s; }
struct pair make_pair(long a, long b) { struct pair p; p.a=a; p.b=b; return p; }
struct mixed make_mixed(int a, double b) { struct mixed m; m.a=a; m.b=b; return m; }
int sum_small(struct small s) { return s.a + s.b + s.c; }
long sum_pair(struct pair p) { return p.a + p.b; }
double sum_mixed(struct mixed m) { return m.a + m.b; }
int main(void) {
    struct small s = make_small(10, 20, 30);
    struct pair p = make_pair(100, 200);
    struct mixed m = make_mixed(3, 4.5);
    if (sum_small(s) != 60) return 1;
    if (sum_pair(p) != 300) return 2;
    if (sum_mixed(m) != 7.5) return 3;
    return 0;
}
