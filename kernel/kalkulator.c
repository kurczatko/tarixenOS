#include "kalkulator.h"
#include "../drivers/keyboard.h"
#include "../graficzny/czcionka.h"
#include "../graficzny/okno.h"
#include "../graficzny/rysowanie.h"

#define KALKULATOR_SZEROKOSC 320
#define KALKULATOR_WYSOKOSC 420
#define KALKULATOR_PRZYCISK_W 60
#define KALKULATOR_PRZYCISK_H 44
#define KALKULATOR_PRZYCISK_X0 16
#define KALKULATOR_PRZYCISK_Y0 70

#define KOLOR_TEKST_KALKULATORA 0x00334155
#define KOLOR_PRZYCISKU_KALKULATORA 0x004A5D6E
#define KOLOR_TEKST_PRZYCISKU_KALKULATORA 0x00FFFFFF
#define KOLOR_WYNIKU 0x000000FF

static struct okno okno_kalkulator;
static uint8_t kalkulator_otwarty;
static char kalkulator_wyswietlacz[32];
static int kalkulator_dlugosc;
static double kalkulator_operand1;
static char kalkulator_operacja;
static uint8_t kalkulator_nowe_wejscie;

static const uint8_t ikona_kalkulatora[12][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,1,1,0,0,0,0},
    {0,1,1,0,0,0,0,1,1,0,0,0},
    {1,1,0,0,0,0,0,0,1,1,0,0},
    {1,0,0,0,0,0,0,0,0,1,1,0},
    {1,0,0,0,0,1,1,0,0,0,1,1},
    {1,0,0,0,1,1,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,1},
    {0,1,1,0,0,0,0,0,0,0,1,0},
    {0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0}
};

static void kalkulator_wyswietl(void)
{
    kalkulator_wyswietlacz[kalkulator_dlugosc] = 0;
}

static void kalkulator_dodaj_znak(char znak)
{
    if (kalkulator_nowe_wejscie) {
        kalkulator_dlugosc = 0;
        kalkulator_nowe_wejscie = 0;
    }
    if (kalkulator_dlugosc < 28) {
        kalkulator_wyswietlacz[kalkulator_dlugosc++] = znak;
    }
    kalkulator_wyswietl();
}

static void kalkulator_czyść(void)
{
    kalkulator_dlugosc = 0;
    kalkulator_operand1 = 0;
    kalkulator_operacja = 0;
    kalkulator_nowe_wejscie = 0;
    kalkulator_wyswietl();
}

static double kalkator_pobierz_liczbe(void)
{
    double v = 0;
    int kropka = 0;
    double mnoznik = 0.1;
    for (int i = 0; i < kalkulator_dlugosc; i++) {
        char c = kalkulator_wyswietlacz[i];
        if (c == '.') { kropka = 1; continue; }
        if (c < '0' || c > '9') continue;
        if (!kropka) v = v * 10 + (c - '0');
        else { v += (c - '0') * mnoznik; mnoznik *= 0.1; }
    }
    return v;
}

static void kalkulator_wynik(void)
{
    double operand2 = kalkator_pobierz_liczbe();
    double wynik = 0;
    if (kalkulator_operacja == '+') wynik = kalkulator_operand1 + operand2;
    else if (kalkulator_operacja == '-') wynik = kalkulator_operand1 - operand2;
    else if (kalkulator_operacja == '*') wynik = kalkulator_operand1 * operand2;
    else if (kalkulator_operacja == '/') {
        if (operand2 == 0) wynik = 0;
        else wynik = kalkulator_operand1 / operand2;
    } else wynik = operand2;

    char buf[32];
    int i = 0;
    double v = wynik;
    if (v < 0) { buf[i++] = '-'; v = -v; }
    v = v * 10000.0 + 0.5;

    double p[20];
    int n = 0;
    p[0] = 1;
    while (n < 19 && p[n] <= v) { n++; p[n] = p[n - 1] * 10; }

    int started = 0;
    int total = 0;
    int dig_start = i;
    for (int k = n - 1; k >= 0; k--) {
        int cyfra = 0;
        while (v >= p[k]) { v -= p[k]; cyfra++; }
        if (cyfra != 0 || started || k == 0) {
            buf[i++] = '0' + cyfra;
            started = 1;
            total++;
        }
    }

    if (total <= 4) {
        char tmp[32];
        int ti = 0;
        tmp[ti++] = '0';
        tmp[ti++] = '.';
        int need = 4 - total;
        for (int z = 0; z < need; z++) tmp[ti++] = '0';
        for (int z = dig_start; z < i; z++) tmp[ti++] = buf[z];
        tmp[ti] = 0;
        for (int z = 0; z < ti; z++) buf[z] = tmp[z];
        i = ti;
        while (i > 2 && buf[i - 1] == '0') i--;
        if (i > 2 && buf[i - 1] == '.') i--;
        buf[i] = 0;
    } else {
        int point_pos = i - 4;
        for (int z = i; z > point_pos; z--) buf[z] = buf[z - 1];
        buf[point_pos] = '.';
        i++;
        while (i > 1 && buf[i - 1] == '0') i--;
        if (i > 1 && buf[i - 1] == '.') i--;
        buf[i] = 0;
    }

    kalkulator_dlugosc = 0;
    while (buf[kalkulator_dlugosc] && kalkulator_dlugosc < 28) {
        kalkulator_wyswietlacz[kalkulator_dlugosc] = buf[kalkulator_dlugosc];
        kalkulator_dlugosc++;
    }
    kalkulator_nowe_wejscie = 1;
    kalkulator_wyswietl();
}

static void kalkulator_ustaw_operacja(char op)
{
    kalkulator_operand1 = kalkator_pobierz_liczbe();
    kalkulator_operacja = op;
    kalkulator_nowe_wejscie = 1;
}

static const char *przyciski[16] = {
    "7","8","9","/",
    "4","5","6","*",
    "1","2","3","-",
    "0","C","=","+"
};

static void kalkulator_narysuj(int x, int y, int szerokosc, int wysokosc)
{
    (void)szerokosc;
    (void)wysokosc;
    rysuj_prostokat(x + 8, y + 30, szerokosc - 16, 34, KOLOR_BIALY);
    rysuj_prostokat(x + 8, y + 30, szerokosc - 16, 34, KOLOR_WYNIKU);
    rysuj_tekst_chicago(x + 12, y + 44, kalkulator_wyswietlacz, KOLOR_TEKST_KALKULATORA);

    int wx = KALKULATOR_PRZYCISK_X0;
    int wy = KALKULATOR_PRZYCISK_Y0;
    for (int i = 0; i < 16; i++) {
        rysuj_prostokat(x + wx, y + wy, KALKULATOR_PRZYCISK_W, KALKULATOR_PRZYCISK_H, KOLOR_PRZYCISKU_KALKULATORA);
        int tx = x + wx + (KALKULATOR_PRZYCISK_W - 8) / 2;
        int ty = y + wy + (KALKULATOR_PRZYCISK_H - 16) / 2;
        rysuj_tekst_chicago(tx, ty, przyciski[i], KOLOR_TEKST_PRZYCISKU_KALKULATORA);
        wx += KALKULATOR_PRZYCISK_W + 8;
        if ((i + 1) % 4 == 0) {
            wx = KALKULATOR_PRZYCISK_X0;
            wy += KALKULATOR_PRZYCISK_H + 8;
        }
    }

    for (int iy = 0; iy < 12; iy++) {
        for (int ix = 0; ix < 12; ix++) {
            if (ikona_kalkulatora[iy][ix]) {
                rysuj_piksel(x + 8 + ix, y + 8 + iy, KOLOR_ZOLTY);
            }
        }
    }

    if (okno_kalkulator.w_polu_tekstowym && kursor_tekstowy_widoczny()) {
        kursor_tekstowy_rysuj(x + 12 + kalkulator_dlugosc * 8, y + 44);
    }
}

static void kalkulator_klik(int x, int y)
{
    int bx = KALKULATOR_PRZYCISK_X0;
    int by = KALKULATOR_PRZYCISK_Y0;
    for (int i = 0; i < 16; i++) {
        if (x >= bx && x < bx + KALKULATOR_PRZYCISK_W &&
            y >= by && y < by + KALKULATOR_PRZYCISK_H) {
            const char *etykieta = przyciski[i];
            if (etykieta[0] == 'C') {
                kalkulator_czyść();
            } else if (etykieta[0] == '=') {
                kalkulator_wynik();
            } else if (etykieta[0] == '+' || etykieta[0] == '-' ||
                       etykieta[0] == '*' || etykieta[0] == '/') {
                kalkulator_ustaw_operacja(etykieta[0]);
            } else {
                kalkulator_dodaj_znak(etykieta[0]);
            }
            okno_narysuj(&okno_kalkulator);
            return;
        }
        bx += KALKULATOR_PRZYCISK_W + 8;
        if ((i + 1) % 4 == 0) {
            bx = KALKULATOR_PRZYCISK_X0;
            by += KALKULATOR_PRZYCISK_H + 8;
        }
    }
}

static void kalkulator_pole_tekstowe_enter(struct okno *okno)
{
    (void)okno;
}

static void kalkulator_pole_tekstowe_wyjdz(struct okno *okno)
{
    (void)okno;
}

static int kalkulator_pole_tekstowe_znak(struct okno *okno, char znak, uint8_t ctrl)
{
    (void)okno;
    (void)ctrl;
    if (znak == '\b') {
        if (kalkulator_dlugosc > 0) kalkulator_dlugosc--;
    } else if ((znak == '\n' || znak == '\r')) {
        kalkulator_wynik();
    } else if (znak == '+' || znak == '-' || znak == '*' || znak == '/') {
        kalkulator_ustaw_operacja(znak);
    } else if (znak >= '0' && znak <= '9' || znak == '.') {
        kalkulator_dodaj_znak(znak);
    } else {
        return 1;
    }
    kalkulator_wyswietl();
    okno_narysuj(&okno_kalkulator);
    return 1;
}

void kalkulator_otworz(void)
{
    kalkulator_dlugosc = 0;
    kalkulator_wyswietlacz[0] = 0;
    kalkulator_operand1 = 0;
    kalkulator_operacja = 0;
    kalkulator_nowe_wejscie = 0;
    okno_stworz(&okno_kalkulator, 56, 50, KALKULATOR_SZEROKOSC,
                KALKULATOR_WYSOKOSC, "KALKULATOR", kalkulator_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_kalkulator, kalkulator_klik);
    okno_ustaw_pole_tekstowe_enter(&okno_kalkulator, (okno_pole_tekstowe)kalkulator_pole_tekstowe_enter);
    okno_ustaw_pole_tekstowe_wyjdz(&okno_kalkulator, (okno_pole_tekstowe)kalkulator_pole_tekstowe_wyjdz);
    okno_ustaw_pole_tekstowe_znak(&okno_kalkulator, (okno_pole_tekstowe_znak)kalkulator_pole_tekstowe_znak);
    okno_ustaw_bufor_tekstu(&okno_kalkulator, kalkulator_wyswietlacz, kalkulator_dlugosc, 32);
    kalkulator_otwarty = 1;
}

void kalkulator_obsluz_klawiature(void)
{
    char znak;
    if (!kalkulator_otwarty) return;
    if (!okno_kalkulator.otwarte) {
        kalkulator_otwarty = 0;
        return;
    }
    if (!okno_kalkulator.w_polu_tekstowym) return;
    if (!okno_czy_aktywne(&okno_kalkulator)) return;
    if (!keyboard_try_getchar(&znak)) return;
    kalkulator_pole_tekstowe_znak(&okno_kalkulator, znak, 0);
}
