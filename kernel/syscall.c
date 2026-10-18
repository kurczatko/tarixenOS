#include "syscall.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/okno.h"
#include "../graficzny/czcionka.h"
#include "../include/printf/printf.h"

struct rejestry biezace_rejestry;

static struct okno okno_tapeta_user;
#define KOLO_SRODEK_X 250
#define KOLO_SRODEK_Y 80
#define KOLO_PROMIEN 40

static uint32_t pierwiastkuj(uint32_t n)
{
    if (n == 0) return 0;
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

static int kat_xy(int dx, int dy)
{
    if (dx == 0 && dy == 0) return 0;
    int ax = dx < 0 ? -dx : dx;
    int ay = dy < 0 ? -dy : dy;
    int kat;
    if (ax >= ay) {
        int baza = (ay * 45) / ax;
        if (dx >= 0 && dy >= 0) kat = baza;
        else if (dx < 0 && dy >= 0) kat = 180 - baza;
        else if (dx < 0 && dy < 0) kat = 180 + baza;
        else kat = 360 - baza;
    } else {
        int baza = 90 - (ax * 45) / ay;
        if (dx >= 0 && dy >= 0) kat = baza;
        else if (dx < 0 && dy >= 0) kat = 180 - baza;
        else if (dx < 0 && dy < 0) kat = 180 + baza;
        else kat = 360 - baza;
    }
    if (kat == 360) kat = 0;
    return kat;
}

static uint32_t hsv_na_xrgb(int h, uint32_t s, uint32_t v)
{
    uint32_t c = (v * s) / 255;
    uint32_t sektor = (uint32_t)h / 60;
    uint32_t f = (uint32_t)h % 60;
    uint32_t r, g, b;
    switch (sektor) {
    case 0: r = c; g = (c * f) / 60; b = 0; break;
    case 1: r = (c * (60 - f)) / 60; g = c; b = 0; break;
    case 2: r = 0; g = c; b = (c * f) / 60; break;
    case 3: r = 0; g = (c * (60 - f)) / 60; b = c; break;
    case 4: r = (c * f) / 60; g = 0; b = c; break;
    default: r = c; g = 0; b = (c * (60 - f)) / 60; break;
    }
    uint32_t m = v - c;
    r += m;
    g += m;
    b += m;
    return (r << 16) | (g << 8) | b;
}

static void rysuj_wybor_tapety_user(int x, int y, int szerokosc, int wysokosc)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    int srodek_x = x + KOLO_SRODEK_X - 8;
    int srodek_y = y + KOLO_SRODEK_Y - 30;
    int granica = (KOLO_PROMIEN - 1) * (KOLO_PROMIEN - 1);
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_chicago(x, y, "WYBIERZ TAPETE", 0x00000000);
    for (int indeks = 0; indeks < 8; indeks++)
        rysuj_prostokat(x + 8 + (indeks % 4) * 32, y + 18 + (indeks / 4) * 32, 24, 24, kolory[indeks]);
    for (int dy = -KOLO_PROMIEN; dy <= KOLO_PROMIEN; dy++) {
        for (int dx = -KOLO_PROMIEN; dx <= KOLO_PROMIEN; dx++) {
            uint32_t r2 = (uint32_t)(dx * dx + dy * dy);
            if (r2 > (uint32_t)(KOLO_PROMIEN * KOLO_PROMIEN)) continue;
            if (r2 > (uint32_t)granica) {
                rysuj_piksel(srodek_x + dx, srodek_y + dy, 0x00000000);
                continue;
            }
            uint32_t r = pierwiastkuj(r2);
            int h = kat_xy(dx, dy);
            uint32_t nasycenie = r * 255 / KOLO_PROMIEN;
            rysuj_piksel(srodek_x + dx, srodek_y + dy, hsv_na_xrgb(h, nasycenie, 255));
        }
    }
}

static void obsluz_wybor_tapety_user(int x, int y)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    int kolumna;
    int wiersz;
    if (x >= 16 && x < 144 && y >= 48 && y < 112) {
        kolumna = (x - 16) / 32;
        wiersz = (y - 48) / 32;
        if (kolumna >= 0 && kolumna < 4 && wiersz >= 0 && wiersz < 2) {
            ustaw_kolor_tapety(kolory[wiersz * 4 + kolumna]);
            tapeta_zapisz();
        }
        return;
    }
    int dx = x - KOLO_SRODEK_X;
    int dy = y - KOLO_SRODEK_Y;
    uint32_t r2 = (uint32_t)(dx * dx + dy * dy);
    if (r2 > (uint32_t)(KOLO_PROMIEN * KOLO_PROMIEN)) return;
    uint32_t r = pierwiastkuj(r2);
    int h = kat_xy(dx, dy);
    uint32_t nasycenie = r * 255 / KOLO_PROMIEN;
    ustaw_kolor_tapety(hsv_na_xrgb(h, nasycenie, 255));
    tapeta_zapisz();
}

uint32_t syscall_handler(void)
{
    uint32_t eax = biezace_rejestry.eax;
    uint32_t ebx = biezace_rejestry.ebx;
    uint32_t ecx = biezace_rejestry.ecx;
    uint32_t edx = biezace_rejestry.edx;
    uint32_t esi = biezace_rejestry.esi;
    uint32_t edi = biezace_rejestry.edi;

    switch (eax) {
        case SYSCALL_TAPETA_ZMIEŃ:
            ustaw_kolor_tapety((uint32_t)ebx);
            tapeta_zapisz();
            return 0;
        case SYSCALL_TAPETA_POBIERZ:
            return 0;
        case SYSCALL_EKRAN_CZYŚĆ:
            wypelnij_ekran((uint32_t)ebx);
            return 0;
        case SYSCALL_EKRAN_PIKSEL:
            rysuj_piksel((int)ebx, (int)ecx, (uint32_t)edx);
            return 0;
        case SYSCALL_EKRAN_PROSTOKĄT:
            rysuj_prostokat((int)ebx, (int)ecx, (int)edx, (int)esi, (uint32_t)edi);
            return 0;
        case SYSCALL_TEKST_RYSUJ:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)edx))
                return 0xFFFFFFFF;
            rysuj_tekst_chicago((int)ebx, (int)ecx, (const char *)edx, (uint32_t)esi);
            return 0;
        case SYSCALL_OKNO_STWÓRZ:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)edi))
                return 0xFFFFFFFF;
            okno_stworz(&okno_tapeta_user, (int)ebx, (int)ecx, (int)edx, (int)esi, (const char *)edi, rysuj_wybor_tapety_user);
            okno_ustaw_obsluge_klikniecia(&okno_tapeta_user, obsluz_wybor_tapety_user);
            return 0;
        case SYSCALL_OKNO_RYSUJ:
            odswiez_widok();
            return 0;
        case SYSCALL_MYSZ_POZYCJA:
            return 0;
        case SYSCALL_DRUKUJ:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx))
                return 0xFFFFFFFF;
            printf((const char *)ebx);
            return 0;
        default:
            return 0xFFFFFFFF;
    }
}

void syscall_install(void)
{
}

void syscall_zapisz_rejestry(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi)
{
    biezace_rejestry.eax = eax;
    biezace_rejestry.ebx = ebx;
    biezace_rejestry.ecx = ecx;
    biezace_rejestry.edx = edx;
    biezace_rejestry.esi = esi;
    biezace_rejestry.edi = edi;
}
