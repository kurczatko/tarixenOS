#include "zegar.h"

#define CMOS_INDEX 0x70
#define CMOS_DATA 0x71
#define CMOS_SECONDS 0x00
#define CMOS_MINUTES 0x02
#define CMOS_HOURS 0x04
#define CMOS_STATUS_A 0x0A
#define CMOS_STATUS_B 0x0B

static uint32_t sekundy_doby;
static uint8_t ostatnia_sekunda_rtc;
static uint8_t zegar_zainicjalizowany;
static uint8_t zegar_sieciowy;
static uint8_t ostatnia_wyswietlona_sekunda = 255;

static uint8_t port_in(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void port_out(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t cmos_read(uint8_t reg)
{
    port_out(CMOS_INDEX, reg);
    return port_in(CMOS_DATA);
}

static uint8_t bcd_to_bin(uint8_t value)
{
    return (uint8_t)((value & 0x0F) + ((value >> 4) * 10));
}

static uint8_t rtc_read_stable(uint8_t reg)
{
    uint8_t first;
    uint8_t second;
    for (uint32_t retry = 0; retry < 100000; retry++) {
        while (cmos_read(CMOS_STATUS_A) & 0x80) { }
        first = cmos_read(reg);
        second = cmos_read(reg);
        if (first == second) return first;
    }
    return cmos_read(reg);
}

static int rtc_get(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    uint8_t status = cmos_read(CMOS_STATUS_B);
    uint8_t hour_value = rtc_read_stable(CMOS_HOURS);
    uint8_t minute_value = rtc_read_stable(CMOS_MINUTES);
    uint8_t second_value = rtc_read_stable(CMOS_SECONDS);
    uint8_t pm = hour_value & 0x80;
    hour_value &= 0x7F;

    if (!(status & 0x04)) {
        hour_value = bcd_to_bin(hour_value);
        minute_value = bcd_to_bin(minute_value);
        second_value = bcd_to_bin(second_value);
    }
    if (!(status & 0x02)) {
        if (hour_value == 12) hour_value = 0;
        if (pm) hour_value = (uint8_t)(hour_value + 12);
    }
    if (hour_value > 23 || minute_value > 59 || second_value > 59) return 0;
    *hour = hour_value;
    *minute = minute_value;
    *second = second_value;
    return 1;
}

void zegar_init(void)
{
    uint8_t hour = 0, minute = 0, second = 0;
    if (rtc_get(&hour, &minute, &second)) {
        sekundy_doby = (uint32_t)hour * 3600 + (uint32_t)minute * 60 + second;
        ostatnia_sekunda_rtc = second;
    } else {
        sekundy_doby = 0;
        ostatnia_sekunda_rtc = 0;
    }
    zegar_sieciowy = 0;
    zegar_zainicjalizowany = 1;
}

static int month_from_name(const char *text)
{
    static const char *months[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    for (int month = 0; month < 12; month++) {
        int match = 1;
        for (int i = 0; i < 3; i++) {
            char c = text[i];
            if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
            char expected = months[month][i];
            if (expected >= 'a' && expected <= 'z') expected -= 'a' - 'A';
            if (c != expected) match = 0;
        }
        if (match) return month + 1;
    }
    return 0;
}

static int parse_two_digits(const char *text)
{
    if (text[0] < '0' || text[0] > '9' || text[1] < '0' || text[1] > '9') return -1;
    return (text[0] - '0') * 10 + text[1] - '0';
}

int zegar_synchronizuj_http(const char *response, uint32_t length)
{
    if (!response) return 0;
    for (uint32_t i = 0; i + 35 < length; i++) {
        if ((response[i] != 'D' && response[i] != 'd') ||
            (response[i + 1] != 'a' && response[i + 1] != 'A') ||
            (response[i + 2] != 't' && response[i + 2] != 'T') ||
            (response[i + 3] != 'e' && response[i + 3] != 'E') || response[i + 4] != ':') continue;

        const char *date = response + i + 5;
        while (*date == ' ') date++;
        if (date[3] != ',' || date[4] != ' ') continue;
        int day = parse_two_digits(date + 5);
        int month = month_from_name(date + 8);
        int year = 0;
        for (int digit = 12; digit < 16; digit++) {
            if (date[digit] < '0' || date[digit] > '9') { year = 0; break; }
            year = year * 10 + date[digit] - '0';
        }
        int hour = parse_two_digits(date + 17);
        int minute = parse_two_digits(date + 20);
        int second = parse_two_digits(date + 23);
        if (day < 1 || day > 31 || !month || year < 2020 || year > 2099 ||
            hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60) continue;

        sekundy_doby = (uint32_t)hour * 3600 + (uint32_t)minute * 60 + (uint32_t)(second % 60);
        ostatnia_sekunda_rtc = rtc_read_stable(CMOS_SECONDS);
        if (!(cmos_read(CMOS_STATUS_B) & 0x04)) ostatnia_sekunda_rtc = bcd_to_bin(ostatnia_sekunda_rtc);
        zegar_sieciowy = 1;
        zegar_zainicjalizowany = 1;
        ostatnia_wyswietlona_sekunda = 255;
        return 1;
    }
    return 0;
}

int zegar_pobierz(char tekst[9])
{
    uint8_t hour = (uint8_t)(sekundy_doby / 3600);
    uint8_t minute = (uint8_t)((sekundy_doby / 60) % 60);
    uint8_t second = (uint8_t)(sekundy_doby % 60);
    uint32_t new_seconds;
    if (!zegar_zainicjalizowany) zegar_init();

    if (zegar_sieciowy) {
        if (rtc_get(&hour, &minute, &second)) {
            uint8_t elapsed = (uint8_t)((second + 60 - ostatnia_sekunda_rtc) % 60);
            if (elapsed) {
                sekundy_doby = (sekundy_doby + elapsed) % 86400;
                ostatnia_sekunda_rtc = second;
            }
        }
        new_seconds = sekundy_doby;
        hour = (uint8_t)(new_seconds / 3600);
        minute = (uint8_t)((new_seconds / 60) % 60);
        second = (uint8_t)(new_seconds % 60);
    } else if (rtc_get(&hour, &minute, &second)) {
        sekundy_doby = (uint32_t)hour * 3600 + (uint32_t)minute * 60 + second;
        ostatnia_sekunda_rtc = second;
    }

    tekst[0] = (char)('0' + hour / 10);
    tekst[1] = (char)('0' + hour % 10);
    tekst[2] = ':';
    tekst[3] = (char)('0' + minute / 10);
    tekst[4] = (char)('0' + minute % 10);
    tekst[5] = ':';
    tekst[6] = (char)('0' + second / 10);
    tekst[7] = (char)('0' + second % 10);
    tekst[8] = 0;

    if (second != ostatnia_wyswietlona_sekunda) {
        ostatnia_wyswietlona_sekunda = second;
        return 1;
    }
    return 0;
}
