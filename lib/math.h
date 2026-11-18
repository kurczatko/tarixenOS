#ifndef LIBC_MATH_H
#define LIBC_MATH_H

#include <lib/types.h>

double sin(double x);
double cos(double x);
double tan(double x);
double sqrt(double x);
double pow(double base, double exp);
double fabs(double x);
double floor(double x);
double ceil(double x);
double fmod(double x, double y);
double log(double x);
double log10(double x);
double exp(double x);
double atan(double x);
double atan2(double y, double x);
double asin(double x);
double acos(double x);

#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536

#endif
