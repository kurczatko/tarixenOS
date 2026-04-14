#include "printf.h"
#include "vga.h"
#include "types.h"
#include "stdarg.h"


static void print_int(int value) {
    char buf[12];
    int i = 10;
    buf[11] = 0;

    if (value == 0) {
        vga_putc('0');
        return;
    }

    if (value < 0) {
        vga_putc('-');
        value = -value;
    }

    while (value && i >= 0) {
        buf[i--] = (value % 10) + '0';
        value /= 10;
    }

    i++;
    while (buf[i]) {
        vga_putc(buf[i++]);
    }
}

void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1)) {
            format++;

            switch (*format) {
                case 's': {
                    char* s = va_arg(args, char*);
                    vga_print(s);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    vga_putc(c);
                    break;
                }
                case 'd': {
                    int i = va_arg(args, int);
                    print_int(i);
                    break;
                }
                default:
                    vga_putc('%');
                    vga_putc(*format);
                    break;
            }
        } else {
            vga_putc(*format);
        }
        format++;
    }

    va_end(args);
}