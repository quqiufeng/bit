struct bigd { double a[4]; };
struct bigd make(void) {
    struct bigd p;
    p.a[0] = 1.5;
    p.a[1] = 2.5;
    p.a[2] = 3.5;
    p.a[3] = 4.5;
    return p;
}
double sum(struct bigd p) {
    return p.a[0] + p.a[1] + p.a[2] + p.a[3];
}
int main(void) {
    struct bigd p = make();
    return sum(p) == 12.0 ? 0 : 1;
}
