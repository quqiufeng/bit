int f(int x) {
    return __builtin_expect(x, 1);
}

int main(void) {
    if (__builtin_constant_p(3) != 1)
        return 1;
    if (__builtin_constant_p(f(2)) != 0)
        return 2;
    if (f(7) != 7)
        return 3;
    if (0)
        __builtin_unreachable();
    return 0;
}
