/*
* okej na pocztku ta funkcja sluzyla do wypisywania tekstu w vga
* ale teraz sluzy do wypisywania rzeczy na porcie szeregowym
* bo system jest juz graficzny i nikt nie odczyta
* tekstu vga.
*/

#include "printf.h"
#include "serial.h"
#include "types.h"
#include "stdarg.h"


static void print_int(int value) {
    char buf[12];
    int i = 10;
    buf[11] = 0;

    if (value == 0) {
        serial_putc('0');
        return;
    }

    if (value < 0) {
        serial_putc('-');
        value = -value;
    }

    while (value && i >= 0) {
        buf[i--] = (value % 10) + '0';
        value /= 10;
    }

    i++;
    while (buf[i]) {
        serial_putc(buf[i++]);
    }
}

void printf(const char* format, ...) { // deklaracja funkcji printf
    /*TODO: zmienic nazwe na printk (nie chce mi sie)*/
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1)) {
            format++;

            switch (*format) {
                case 's': {
                    char* s = va_arg(args, char*);
                    serial_print(s);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    serial_putc(c);
                    break;
                }
                case 'd': {
                    int i = va_arg(args, int);
                    print_int(i);
                    break;
                }
                default:
                    serial_putc('%');
                    serial_putc(*format);
                    break;
            }
        } else {
            serial_putc(*format);
        }
        format++;
    }

    va_end(args);
}