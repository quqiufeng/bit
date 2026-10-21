#ifndef __C1_MATH_H
#define __C1_MATH_H
#define M_PI 3.14159265358979323846
double acos(double value);
double asin(double value);
double atan(double value);
double atan2(double y, double x);
double ceil(double value);
double cos(double value);
double cosh(double value);
double exp(double value);
double fabs(double value);
double floor(double value);
double fmod(double value, double divisor);
double frexp(double value, int *exponent);
double hypot(double x, double y);
double ldexp(double value, int exponent);
double log(double value);
double log10(double value);
double modf(double value, double *integer);
double pow(double base, double exponent);
double round(double value);
double sin(double value);
double sinh(double value);
double sqrt(double value);
double tan(double value);
double tanh(double value);
double trunc(double value);
#endif
