int main(void) {
    double zero = 0.0;
    double nan = zero / zero;
    if (nan == nan) return 1;
    if (!(nan != nan)) return 2;
    if (nan < 1.0 || nan > 1.0 || nan <= 1.0 || nan >= 1.0) return 3;
    return 0;
}
