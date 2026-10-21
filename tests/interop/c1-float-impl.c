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
