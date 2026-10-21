int f(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8;
}

int main(void) {
    return f(1, 1, 1, 1, 1, 1, 1, 1) == 36 ? 0 : 1;
}
