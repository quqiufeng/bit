enum E { E_A = 3, E_B, E_C };
union U { int i; unsigned char c[4]; };
struct S { unsigned a : 3; signed b : 5; int c; };
typedef int (*Fn)(int);
static int add1(int x) { return x + 1; }
static int f(int x) {
    int y = 0;
    do { y++; } while (y < x);
    switch (x) {
    case 1: y += 2;
    case 2: y += 3; break;
    default: y += 4; break;
    }
    goto done;
done:
    return y;
}
int main(void) {
    enum E e = E_B;
    union U u;
    struct S s;
    Fn fn = add1;
    int i, a[4];
    if (e != 4) return 1;
    u.i = 0x41424344;
    if (u.c[0] != 0x44) return 2;
    s.a = 5; s.b = -3; s.c = 7;
    if (s.a != 5 || s.b != -3 || s.c != 7) return 3;
    if (sizeof(struct S) != 8) return 4;
    if (fn(41) != 42) return 5;
    for (i = 0; i < 4; i++) a[i] = i * i;
    if (a[3] != 9) return 6;
    if (f(2) != 6) return 7;
    return 0;
}
