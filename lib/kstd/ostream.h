#ifndef KSTD_OSTREAM_H
#define KSTD_OSTREAM_H

#include <lib/kstd/type_traits.h>
#include <lib/kstd/utility.h>
#include <include/printf/serial.h>

namespace kstd {

class ostream {
public:
    KSTD_CONSTEXPR ostream() noexcept {}

    ostream& put(char c) {
        serial_putc(c);
        return *this;
    }

    ostream& write(const char* s, size_t n) {
        for (size_t i = 0; i < n; ++i) serial_putc(s[i]);
        return *this;
    }

    ostream& write(const char* s) {
        while (*s) serial_putc(*s++);
        return *this;
    }

    ostream& flush() { return *this; }

    ostream& operator<<(char c) { return put(c); }
    ostream& operator<<(signed char c) { return put(static_cast<char>(c)); }
    ostream& operator<<(unsigned char c) { return put(static_cast<char>(c)); }

    ostream& operator<<(const char* s) { return write(s); }
    ostream& operator<<(char* s) { return write(static_cast<const char*>(s)); }

    ostream& operator<<(bool b) { return write(b ? "true" : "false"); }

    ostream& operator<<(short value)   { return write_int(static_cast<int>(value), 10, false); }
    ostream& operator<<(int value)     { return write_int(value, 10, false); }
    ostream& operator<<(long value)    { return write_long(value, 10, false); }
    ostream& operator<<(long long value){ return write_llong(value, 10, false); }

    ostream& operator<<(unsigned short value)   { return write_uint(static_cast<unsigned int>(value), 10, false); }
    ostream& operator<<(unsigned int value)     { return write_uint(value, 10, false); }
    ostream& operator<<(unsigned long value)    { return write_ulong(value, 10, false); }
    ostream& operator<<(unsigned long long value){ return write_ullong(value, 10, false); }

    ostream& operator<<(float value)  { return write_float(value); }
    ostream& operator<<(double value) { return write_double(value); }

    ostream& operator<<(void* ptr) {
        return write_ptr(reinterpret_cast<uint32_t>(ptr));
    }
    ostream& operator<<(const void* ptr) {
        return write_ptr(reinterpret_cast<uint32_t>(ptr));
    }

    ostream& operator<<(ostream& (*manip)(ostream&)) {
        return manip(*this);
    }

private:
    ostream& write_int(int value, int base, bool caps) {
        char buf[32];
        int i = 31;
        buf[i] = 0;
        bool neg = false;
        unsigned int uval;
        if (value < 0) { neg = true; uval = static_cast<unsigned int>(-(value + 1)) + 1u; }
        else { uval = static_cast<unsigned int>(value); }
        do {
            unsigned int d = uval % static_cast<unsigned int>(base);
            buf[--i] = (d < 10) ? static_cast<char>('0' + d)
                                : static_cast<char>((caps ? 'A' : 'a') + d - 10);
            uval /= static_cast<unsigned int>(base);
        } while (uval);
        if (neg) buf[--i] = '-';
        return write(&buf[i]);
    }

    ostream& write_long(long value, int base, bool caps) {
        char buf[32];
        int i = 31;
        buf[i] = 0;
        bool neg = false;
        unsigned long uval;
        if (value < 0) { neg = true; uval = static_cast<unsigned long>(-(value + 1)) + 1ul; }
        else { uval = static_cast<unsigned long>(value); }
        do {
            unsigned long d = uval % static_cast<unsigned long>(base);
            buf[--i] = (d < 10) ? static_cast<char>('0' + d)
                                : static_cast<char>((caps ? 'A' : 'a') + d - 10);
            uval /= static_cast<unsigned long>(base);
        } while (uval);
        if (neg) buf[--i] = '-';
        return write(&buf[i]);
    }

    ostream& write_llong(long long value, int base, bool caps) {
        char buf[32];
        int i = 31;
        buf[i] = 0;
        bool neg = false;
        unsigned long long uval;
        if (value < 0) { neg = true; uval = static_cast<unsigned long long>(-(value + 1)) + 1ull; }
        else { uval = static_cast<unsigned long long>(value); }
        do {
            unsigned long long d = uval % static_cast<unsigned long long>(base);
            buf[--i] = (d < 10) ? static_cast<char>('0' + d)
                                : static_cast<char>((caps ? 'A' : 'a') + d - 10);
            uval /= static_cast<unsigned long long>(base);
        } while (uval);
        if (neg) buf[--i] = '-';
        return write(&buf[i]);
    }

    ostream& write_uint(unsigned int value, int base, bool caps) {
        char buf[32];
        int i = 31;
        buf[i] = 0;
        if (value == 0) { return put('0'); }
        while (value) {
            unsigned int d = value % static_cast<unsigned int>(base);
            buf[--i] = (d < 10) ? static_cast<char>('0' + d)
                                : static_cast<char>((caps ? 'A' : 'a') + d - 10);
            value /= static_cast<unsigned int>(base);
        }
        return write(&buf[i]);
    }

    ostream& write_ulong(unsigned long value, int base, bool caps) {
        char buf[32];
        int i = 31;
        buf[i] = 0;
        if (value == 0) { return put('0'); }
        while (value) {
            unsigned long d = value % static_cast<unsigned long>(base);
            buf[--i] = (d < 10) ? static_cast<char>('0' + d)
                                : static_cast<char>((caps ? 'A' : 'a') + d - 10);
            value /= static_cast<unsigned long>(base);
        }
        return write(&buf[i]);
    }

    ostream& write_ullong(unsigned long long value, int base, bool caps) {
        char buf[32];
        int i = 31;
        buf[i] = 0;
        if (value == 0) { return put('0'); }
        while (value) {
            unsigned long long d = value % static_cast<unsigned long long>(base);
            buf[--i] = (d < 10) ? static_cast<char>('0' + d)
                                : static_cast<char>((caps ? 'A' : 'a') + d - 10);
            value /= static_cast<unsigned long long>(base);
        }
        return write(&buf[i]);
    }

    ostream& write_ptr(uint32_t ptr) {
        serial_print("0x");
        char buf[8];
        int i = 7;
        buf[i] = 0;
        if (ptr == 0) { serial_putc('0'); return *this; }
        while (ptr) {
            unsigned int d = ptr % 16u;
            buf[--i] = (d < 10) ? static_cast<char>('0' + d) : static_cast<char>('A' + d - 10);
            ptr /= 16u;
        }
        write(&buf[i]);
        return *this;
    }

    ostream& write_float(float value) {
        int whole = static_cast<int>(value);
        float frac = value - static_cast<float>(whole);
        if (frac < 0.0f) frac = -frac;
        *this << whole;
        put('.');
        for (int i = 0; i < 6; ++i) {
            frac *= 10.0f;
            int digit = static_cast<int>(frac);
            put(static_cast<char>('0' + digit));
            frac -= static_cast<float>(digit);
        }
        return *this;
    }

    ostream& write_double(double value) {
        int whole = static_cast<int>(value);
        double frac = value - static_cast<double>(whole);
        if (frac < 0.0) frac = -frac;
        *this << whole;
        put('.');
        for (int i = 0; i < 6; ++i) {
            frac *= 10.0;
            int digit = static_cast<int>(frac);
            put(static_cast<char>('0' + digit));
            frac -= static_cast<double>(digit);
        }
        return *this;
    }
};

extern ostream cout;

inline ostream& endl(ostream& os) {
    os.put('\n');
    os.flush();
    return os;
}

inline ostream& flush(ostream& os) {
    os.flush();
    return os;
}

}

#endif
