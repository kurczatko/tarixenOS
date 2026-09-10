#include "rysowanie.h"

#define PAMIEC_GRAFICZNA ((uint8_t *)0xA0000)
#define PORT_SEKWENCERA 0x3C4
#define PORT_DANYCH_SEKWENCERA 0x3C5
#define PORT_KONTROLERA_GRAFIKI 0x3CE
#define PORT_DANYCH_GRAFIKI 0x3CF
#define KURSOR_BOK 5
#define KOLOR_TLA 1
#define KOLOR_KURSORA 0
#define KOLOR_PASKA_ZADAN 8

static int kursor_x;
static int kursor_y;
static uint8_t obraz[EKRAN_SZEROKOSC * EKRAN_WYSOKOSC];
static uint8_t kolor_tapety = KOLOR_TLA;

static void wybierz_plane(uint8_t numer);

static void wypelnij_obraz_planami(uint8_t kolor)
{
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (uint32_t bajt = 0; bajt < 80 * EKRAN_WYSOKOSC; bajt++) {
            PAMIEC_GRAFICZNA[bajt] = (kolor & (1 << plane)) ? 0xFF : 0x00;
        }
    }
}

static void wypelnij_pasek_planami(uint8_t kolor)
{
    uint32_t poczatek = (uint32_t)(EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) * 80;
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (uint32_t bajt = 0; bajt < 80 * PASEK_ZADAN_WYSOKOSC; bajt++) {
            PAMIEC_GRAFICZNA[poczatek + bajt] = (kolor & (1 << plane)) ? 0xFF : 0x00;
        }
    }
}

static void ustaw_kontroler_grafiki(uint8_t rejestr, uint8_t wartosc)
{
    __asm__ volatile ("outb %0, %1" : : "a"(rejestr), "Nd"((uint16_t)PORT_KONTROLERA_GRAFIKI));
    __asm__ volatile ("outb %0, %1" : : "a"(wartosc), "Nd"((uint16_t)PORT_DANYCH_GRAFIKI));
}

static void wybierz_plane(uint8_t numer)
{
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)2), "Nd"((uint16_t)PORT_SEKWENCERA));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)(1 << numer)), "Nd"((uint16_t)PORT_DANYCH_SEKWENCERA));
}

static void odswiez_ekran(void)
{
    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(1, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (uint32_t indeks = 0; indeks < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC / 8; indeks++) {
            uint8_t bajt = 0;
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint32_t piksel = indeks * 8 + bit;
                if (obraz[piksel] & (1 << plane)) bajt |= (uint8_t)(0x80 >> bit);
            }
            PAMIEC_GRAFICZNA[indeks] = bajt;
        }
    }
}

void odswiez_widok(void)
{
    odswiez_ekran();
}

static void odswiez_fragment_z_bufora(int x, int y, int szerokosc, int wysokosc)
{
    int pierwszy_bajt = x >> 3;
    int ostatni_bajt = (x + szerokosc - 1) >> 3;

    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(1, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
    for (uint8_t plane = 0; plane < 4; plane++) {
        wybierz_plane(plane);
        for (int wiersz = 0; wiersz < wysokosc; wiersz++) {
            for (int bajt = pierwszy_bajt; bajt <= ostatni_bajt; bajt++) {
                uint8_t wartosc = 0;
                int pierwszy_piksel = bajt * 8;
                for (int bit = 0; bit < 8; bit++) {
                    int piksel_x = pierwszy_piksel + bit;
                    if (obraz[(y + wiersz) * EKRAN_SZEROKOSC + piksel_x] & (1 << plane))
                        wartosc |= (uint8_t)(0x80 >> bit);
                }
                PAMIEC_GRAFICZNA[(uint32_t)(y + wiersz) * 80 + bajt] = wartosc;
            }
        }
    }
}

void rysuj_piksel(int x, int y, uint8_t kolor)
{
    if (x < 0 || x >= EKRAN_SZEROKOSC || y < 0 || y >= EKRAN_WYSOKOSC) return;
    obraz[y * EKRAN_SZEROKOSC + x] = kolor & 0x0F;
}

void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint8_t kolor)
{
    for (int pion = 0; pion < wysokosc; pion++)
        for (int poziom = 0; poziom < szerokosc; poziom++)
            rysuj_piksel(x + poziom, y + pion, kolor);
    odswiez_ekran();
}

void rysuj_kwadrat(int x, int y, int bok, uint8_t kolor)
{
    rysuj_prostokat(x, y, bok, bok, kolor);
}

static void rysuj_fragment(int x, int y, int szerokosc, int wysokosc, uint8_t kolor)
{
    int wiersz;
    int pierwszy_bajt;
    int ostatni_bajt;

    if (x < 0 || y < 0 || x + szerokosc > EKRAN_SZEROKOSC || y + wysokosc > EKRAN_WYSOKOSC) return;
    pierwszy_bajt = x >> 3;
    ostatni_bajt = (x + szerokosc - 1) >> 3;
    ustaw_kontroler_grafiki(0, kolor & 0x0F);
    ustaw_kontroler_grafiki(1, 0x0F);
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)2), "Nd"((uint16_t)PORT_SEKWENCERA));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)PORT_DANYCH_SEKWENCERA));
    for (wiersz = 0; wiersz < wysokosc; wiersz++) {
        int bajt;
        for (bajt = pierwszy_bajt; bajt <= ostatni_bajt; bajt++) {
            uint8_t maska = 0xFF;
            if (bajt == pierwszy_bajt) maska &= (uint8_t)(0xFF >> (x & 7));
            if (bajt == ostatni_bajt) maska &= (uint8_t)(0xFF << (7 - ((x + szerokosc - 1) & 7)));
            ustaw_kontroler_grafiki(8, maska);
            PAMIEC_GRAFICZNA[(uint32_t)(y + wiersz) * 80 + bajt] = 0;
        }
    }
    ustaw_kontroler_grafiki(1, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
}

void narysuj_pasek_zadan(void)
{
    wypelnij_pasek_planami(KOLOR_PASKA_ZADAN);
}

void wypelnij_ekran(uint8_t kolor)
{
    kolor_tapety = kolor;
    wypelnij_obraz_planami(kolor);
}

void ustaw_kolor_tapety(uint8_t kolor)
{
    kolor_tapety = kolor & 0x0F;
    for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++)
        for (int x = 0; x < EKRAN_SZEROKOSC; x++)
            obraz[y * EKRAN_SZEROKOSC + x] = kolor_tapety;
    odswiez_ekran();
}

static void przywroc_tlo_kursora(void)
{
    odswiez_fragment_z_bufora(kursor_x, kursor_y, KURSOR_BOK, KURSOR_BOK);
}

void kursor_graficzny_init(void)
{
    kursor_x = EKRAN_SZEROKOSC / 2;
    kursor_y = EKRAN_WYSOKOSC / 2;
    rysuj_fragment(kursor_x, kursor_y, KURSOR_BOK, KURSOR_BOK, KOLOR_KURSORA);
}

void kursor_graficzny_przesun(int delta_x, int delta_y)
{
    przywroc_tlo_kursora();
    kursor_x += delta_x;
    kursor_y += delta_y;
    if (kursor_x < 0) kursor_x = 0;
    if (kursor_y < 0) kursor_y = 0;
    if (kursor_x > EKRAN_SZEROKOSC - KURSOR_BOK) kursor_x = EKRAN_SZEROKOSC - KURSOR_BOK;
    if (kursor_y > EKRAN_WYSOKOSC - KURSOR_BOK) kursor_y = EKRAN_WYSOKOSC - KURSOR_BOK;
    rysuj_fragment(kursor_x, kursor_y, KURSOR_BOK, KURSOR_BOK, KOLOR_KURSORA);
}

void kursor_graficzny_pozycja(int *x, int *y)
{
    *x = kursor_x;
    *y = kursor_y;
}

void rysowanie_init(void)
{
    ustaw_kontroler_grafiki(5, 0);
    ustaw_kontroler_grafiki(8, 0xFF);
    for (uint32_t indeks = 0; indeks < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC; indeks++) obraz[indeks] = KOLOR_TLA;
    for (int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y < EKRAN_WYSOKOSC; y++)
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) obraz[y * EKRAN_SZEROKOSC + x] = KOLOR_PASKA_ZADAN;
    odswiez_ekran();
    kursor_graficzny_init();
}
