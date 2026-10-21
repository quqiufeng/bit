struct ff { float a; float b; };
struct dd { double a; double b; };
struct di { double a; int b; };
struct id { int a; double b; };
struct mixed { int a; float b; };
struct ff makeff(float a, float b) { struct ff p; p.a=a; p.b=b; return p; }
float sumff(struct ff p) { return p.a + p.b; }
struct dd makedd(double a, double b) { struct dd p; p.a=a; p.b=b; return p; }
double sumdd(struct dd p) { return p.a + p.b; }
struct di makedi(double a, int b) { struct di p; p.a=a; p.b=b; return p; }
double sumdi(struct di p) { return p.a + p.b; }
struct id makeid(int a, double b) { struct id p; p.a=a; p.b=b; return p; }
double sumid(struct id p) { return p.a + p.b; }
struct mixed makem(int a, float b) { struct mixed p; p.a=a; p.b=b; return p; }
float summ(struct mixed p) { return p.a + p.b; }
int main(void) {
    if (sumff(makeff(1.25f, 2.75f)) != 4.0f) return 1;
    if (sumdd(makedd(1.5, 2.5)) != 4.0) return 2;
    if (sumdi(makedi(1.5, 2)) != 3.5) return 3;
    if (sumid(makeid(2, 1.5)) != 3.5) return 4;
    if (summ(makem(2, 1.5f)) != 3.5f) return 5;
    return 0;
}
