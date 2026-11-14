/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <libc/math.h>

static double abs_d(double x) { return x < 0 ? -x : x; }

double sin(double x) {
    while (x > M_PI) x -= 2 * M_PI;
    while (x < -M_PI) x += 2 * M_PI;
    double r = x;
    double term = x;
    double x2 = x * x;
    for (int i = 1; i < 12; i++) {
        term *= -x2 / ((2 * i) * (2 * i + 1));
        r += term;
    }
    return r;
}

double cos(double x) {
    while (x > M_PI) x -= 2 * M_PI;
    while (x < -M_PI) x += 2 * M_PI;
    double r = 1.0;
    double term = 1.0;
    double x2 = x * x;
    for (int i = 1; i < 12; i++) {
        term *= -x2 / ((2 * i - 1) * (2 * i));
        r += term;
    }
    return r;
}

double tan(double x) {
    double s = sin(x);
    double c = cos(x);
    if (c == 0.0) return 0.0;
    return s / c;
}

double sqrt(double x) {
    if (x < 0) return 0.0;
    if (x == 0) return 0.0;
    double g = x;
    for (int i = 0; i < 20; i++) {
        g = 0.5 * (g + x / g);
    }
    return g;
}

double fabs(double x) { return x < 0 ? -x : x; }

double floor(double x) {
    double i = (double)(long)x;
    if (i > x) i -= 1.0;
    return i;
}

double ceil(double x) {
    double i = (double)(long)x;
    if (i < x) i += 1.0;
    return i;
}

double fmod(double x, double y) {
    if (y == 0) return 0.0;
    return x - y * floor(x / y);
}

double log(double x) {
    if (x <= 0) return 0.0;
    double y = (x - 1.0) / (x + 1.0);
    double y2 = y * y;
    double r = y;
    double term = y;
    for (int i = 1; i < 20; i++) {
        term *= y2;
        r += term / (2 * i + 1);
    }
    return 2 * r;
}

double log10(double x) { return log(x) / log(10.0); }

double exp(double x) {
    double r = 1.0;
    double term = 1.0;
    for (int i = 1; i < 30; i++) {
        term *= x / i;
        r += term;
    }
    return r;
}

double pow(double base, double e) {
    if (base == 0) return 0.0;
    if (e == 0) return 1.0;
    return exp(e * log(abs_d(base)));
}

double atan(double x) {
    if (x > 1.0) return M_PI / 2 - atan(1.0 / x);
    if (x < -1.0) return -M_PI / 2 - atan(1.0 / x);
    double r = 0.0;
    double term = x;
    double x2 = x * x;
    for (int i = 0; i < 30; i++) {
        r += term;
        term *= -x2 * (2 * i + 1) / (2 * i + 3);
    }
    return r;
}

double atan2(double y, double x) {
    if (x > 0) return atan(y / x);
    if (x < 0 && y >= 0) return atan(y / x) + M_PI;
    if (x < 0 && y < 0) return atan(y / x) - M_PI;
    if (x == 0 && y > 0) return M_PI / 2;
    if (x == 0 && y < 0) return -M_PI / 2;
    return 0.0;
}

double asin(double x) { return atan2(x, sqrt(1 - x * x)); }
double acos(double x) { return M_PI / 2 - asin(x); }
