double sum10(double a, double b, double c, double d, double e, double f, double g, double h, double i, double j) {
    return a + b + c + d + e + f + g + h + i + j;
}

double mix10(int a, double b, int c, double d, int e, double f, int g, double h, int i, double j) {
    return a + b + c + d + e + f + g + h + i + j;
}

int main(void) {
    if (sum10(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0) != 55.0)
        return 1;
    if (mix10(1, 2.0, 3, 4.0, 5, 6.0, 7, 8.0, 9, 10.0) != 55.0)
        return 2;
    return 0;
}
