double addd(double a, double b) { return a + b; }
float addf(float a, float b) { return a + b; }
double mix(int a, double b, int c, double d) { return a + b + c + d; }
int main(void) {
    if (addd(1.5, 2.5) != 4.0) return 1;
    if (addf(1.25f, 2.75f) != 4.0f) return 2;
    if (mix(1, 2.5, 3, 4.5) != 11.0) return 3;
    return 0;
}
