struct big { int a[8]; };
struct big make(int seed) {
    struct big p;
    int i;
    for (i = 0; i < 8; i = i + 1)
        p.a[i] = seed + i;
    return p;
}
int sum(struct big p) {
    int i, total = 0;
    for (i = 0; i < 8; i = i + 1)
        total = total + p.a[i];
    return total;
}
int main(void) {
    struct big p = make(1);
    return sum(p) == 36 ? 0 : 1;
}
