int mix(char a, short b, unsigned char c, int d, long e, int f, char g, short h) {
    return a + b + c + d + (int)e + f + g + h;
}

int main(void) {
    return mix(1, 2, 3, 4, 5, 6, 7, 8) == 36 ? 0 : 1;
}
