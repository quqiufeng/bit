#include <stdio.h>
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
    printf("%f %f %f %f %f\n", sumff(makeff(1.25f,2.75f)), sumdd(makedd(1.5,2.5)), sumdi(makedi(1.5,2)), sumid(makeid(2,1.5)), summ(makem(2,1.5f)));
    return sumff(makeff(1.25f,2.75f)) == 4.0f && sumdd(makedd(1.5,2.5)) == 4.0 && sumdi(makedi(1.5,2)) == 3.5 && sumid(makeid(2,1.5)) == 3.5 && summ(makem(2,1.5f)) == 3.5f ? 0 : 1;
}
