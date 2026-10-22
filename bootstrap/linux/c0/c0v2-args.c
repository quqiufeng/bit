int mix(int a, int b, int c, int d, int e, int f,
        int g, int h, int i, int j, int k) {
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f +
           7 * g + 8 * h + 9 * i + 10 * j + 11 * k;
}

int main(void) {
    return mix(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) == 506 ? 42 : 1;
}
