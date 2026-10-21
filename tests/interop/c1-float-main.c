struct ff { float a; float b; };
struct dd { double a; double b; };
struct di { double a; int b; };
struct id { int a; double b; };
struct mixed { int a; float b; };
struct ff makeff(float, float); float sumff(struct ff);
struct dd makedd(double, double); double sumdd(struct dd);
struct di makedi(double, int); double sumdi(struct di);
struct id makeid(int, double); double sumid(struct id);
struct mixed makem(int, float); float summ(struct mixed);
int main(void) {
    if (sumff(makeff(1.25f, 2.75f)) != 4.0f) return 1;
    if (sumdd(makedd(1.5, 2.5)) != 4.0) return 2;
    if (sumdi(makedi(1.5, 2)) != 3.5) return 3;
    if (sumid(makeid(2, 1.5)) != 3.5) return 4;
    if (summ(makem(2, 1.5f)) != 3.5f) return 5;
    return 0;
}
