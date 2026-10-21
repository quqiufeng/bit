int main(void) {
    long double zero = 0.0L;
    long double nan = zero / zero;
    if (nan == nan) return 1;
    if (!(nan != nan)) return 2;
    if (nan < 1.0L || nan > 1.0L || nan <= 1.0L || nan >= 1.0L) return 3;
    if (!nan) return 4;
    return 0;
}
