int main(void) {
    float zero = 0.0f;
    float nan = zero / zero;
    if (nan == nan) return 1;
    if (!(nan != nan)) return 2;
    if (nan < 1.0f || nan > 1.0f || nan <= 1.0f || nan >= 1.0f) return 3;
    return 0;
}
