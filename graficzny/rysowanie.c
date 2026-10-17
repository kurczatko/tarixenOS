#include "rysowanie.h"
#include "czcionka.h"
#include "../system_plikow/system_plikow.h"

#define CURSOR_W 12
#define CURSOR_H 18
#define KOLOR_TLA 0x0098C74C
#define KOLOR_PASKA_ZADAN 0x002D3E50
#define KOLOR_TEKST_BIALY 0x00FFFFFF

static volatile uint8_t *pamiec_graficzna = (volatile uint8_t *)0xFD000000;
static uint32_t framebuffer_pitch = EKRAN_SZEROKOSC * 4;
static uint8_t framebuffer_bytes_per_pixel = 4;
static uint8_t red_mask_size = 8;
static uint8_t red_field_position = 16;
static uint8_t green_mask_size = 8;
static uint8_t green_field_position = 8;
static uint8_t blue_mask_size = 8;
static uint8_t blue_field_position;
static uint8_t internet_dostepny;
static char czas_paska[9] = "--:--:--";

static const uint8_t kursor_bitmapa[CURSOR_H][CURSOR_W] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 1, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0},
    {1, 0, 0, 1, 1, 2, 2, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}
};

static int kursor_x;
static int kursor_y;

static void narysuj_kursor_bitmapa(int x, int y);

uint32_t *obraz = (uint32_t *)0x00400000;
static uint32_t kolor_tapety = KOLOR_TLA;
static int tapeta_gradientowa = 1;

static uint32_t kolor_tla_dla_x(int x)
{
    uint32_t niebieski = (uint32_t)x * 255 / (EKRAN_SZEROKOSC - 1);
    uint32_t zielony = 255 - niebieski;
    return (zielony << 8) | niebieski;
}

static void narysuj_ikone_internetu(void)
{
    static const uint16_t bitmapa[14] = {
        0x03F0, 0x0C0C, 0x1002, 0x23C1, 0x2C31, 0x3019, 0x3019,
        0x3019, 0x2C31, 0x23C1, 0x1002, 0x0C0C, 0x03F0, 0x0000
    };
    uint32_t kolor = internet_dostepny ? KOLOR_BIALY : KOLOR_SZARY;
    int x = EKRAN_SZEROKOSC - 22;
    int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 8;
    for (int pion = 0; pion < 14; pion++) {
        for (int poziom = 0; poziom < 12; poziom++) {
            if (bitmapa[pion] & (0x0800 >> poziom))
                rysuj_piksel(x + poziom, y + pion, kolor);
        }
    }
}

static void narysuj_czas_paska(void)
{
    int x = EKRAN_SZEROKOSC - 108;
    int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 11;
    rysuj_prostokat(x - 4, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC,
                    84, PASEK_ZADAN_WYSOKOSC, KOLOR_PASKA_ZADAN);
    rysuj_tekst_chicago(x, y, czas_paska, KOLOR_BIALY);
}

void ustaw_czas_paska(const char *czas)
{
    int i = 0;
    while (czas[i] && i < 8) { czas_paska[i] = czas[i]; i++; }
    czas_paska[i] = 0;
    while (i < 8) czas_paska[i++] = ' ';
    narysuj_czas_paska();
    odswiez_fragment_widoku(EKRAN_SZEROKOSC - 112,
                            EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC,
                            88, PASEK_ZADAN_WYSOKOSC);
}

void ustaw_status_internetu(uint8_t dostepny)
{
    internet_dostepny = dostepny != 0;
    narysuj_ikone_internetu();
    odswiez_fragment_widoku(EKRAN_SZEROKOSC - 24,
                            EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC,
                            24, PASEK_ZADAN_WYSOKOSC);
}

static void kopiuj_pamiec(void *dst, const void *src, uint32_t bajty)
{
    uint32_t *d = (uint32_t *)dst;
    const uint32_t *s = (const uint32_t *)src;
    uint32_t dwords = bajty / 4;
    for (uint32_t i = 0; i < dwords; i++) {
        d[i] = s[i];
    }
}

static void zapisz_piksel_framebuffera(int x, int y, uint32_t kolor)
{
    uint32_t czerwony = (kolor >> 16) & 0xFF;
    uint32_t zielony = (kolor >> 8) & 0xFF;
    uint32_t niebieski = kolor & 0xFF;
    uint32_t wartosc = 0;
    volatile uint8_t *piksel = pamiec_graficzna + (uint32_t)y * framebuffer_pitch +
                               (uint32_t)x * framebuffer_bytes_per_pixel;

    if (red_mask_size) wartosc |= (czerwony >> (8 - red_mask_size)) << red_field_position;
    if (green_mask_size) wartosc |= (zielony >> (8 - green_mask_size)) << green_field_position;
    if (blue_mask_size) wartosc |= (niebieski >> (8 - blue_mask_size)) << blue_field_position;
    for (uint8_t bajt = 0; bajt < framebuffer_bytes_per_pixel; bajt++)
        piksel[bajt] = (uint8_t)(wartosc >> (bajt * 8));
}

    static int framebuffer_ma_szybki_format(void)
    {
        return framebuffer_bytes_per_pixel == 4 &&
           framebuffer_pitch == EKRAN_SZEROKOSC * 4 &&
           red_mask_size == 8 && red_field_position == 16 &&
           green_mask_size == 8 && green_field_position == 8 &&
           blue_mask_size == 8 && blue_field_position == 0;
    }

void odswiez_fragment_widoku(int x, int y, int szerokosc, int wysokosc)
{
    if (x < 0) {
        szerokosc += x;
        x = 0;
    }
    if (y < 0) {
        wysokosc += y;
        y = 0;
    }
    if (x + szerokosc > EKRAN_SZEROKOSC) szerokosc = EKRAN_SZEROKOSC - x;
    if (y + wysokosc > EKRAN_WYSOKOSC) wysokosc = EKRAN_WYSOKOSC - y;
    if (szerokosc <= 0 || wysokosc <= 0) return;

    for (int wiersz = 0; wiersz < wysokosc; wiersz++) {
        uint32_t offset = (uint32_t)(y + wiersz) * EKRAN_SZEROKOSC + (uint32_t)x;
        if (framebuffer_ma_szybki_format()) {
            kopiuj_pamiec((void *)&pamiec_graficzna[offset * 4], &obraz[offset], szerokosc * 4);
        } else {
            for (int kolumna = 0; kolumna < szerokosc; kolumna++)
                zapisz_piksel_framebuffera(x + kolumna, y + wiersz, obraz[offset + kolumna]);
        }
    }
    narysuj_kursor_bitmapa(kursor_x, kursor_y);
}

static void narysuj_napis_systemowy(void)
{
    int szerokosc_napisu = 27 * 8;
    int tekst_x = EKRAN_SZEROKOSC - szerokosc_napisu - 16;
    int tekst_y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 16;
    rysuj_tekst_chicago(tekst_x, tekst_y, "TarixenOS Tryb Graficzny", KOLOR_TEKST_BIALY);
}

void odswiez_widok(void)
{
    if (framebuffer_ma_szybki_format()) {
        kopiuj_pamiec((void *)pamiec_graficzna, obraz, EKRAN_SZEROKOSC * EKRAN_WYSOKOSC * 4);
    } else {
        for (int y = 0; y < EKRAN_WYSOKOSC; y++) {
            uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
            for (int x = 0; x < EKRAN_SZEROKOSC; x++)
                zapisz_piksel_framebuffera(x, y, obraz[offset + x]);
        }
    }
    narysuj_kursor_bitmapa(kursor_x, kursor_y);
}

void odtworz_fragment_tla_bufor(int x, int y, int szerokosc, int wysokosc)
{
    if (x < 0) {
        szerokosc += x;
        x = 0;
    }
    if (y < 0) {
        wysokosc += y;
        y = 0;
    }
    if (x + szerokosc > EKRAN_SZEROKOSC) szerokosc = EKRAN_SZEROKOSC - x;
    if (y + wysokosc > EKRAN_WYSOKOSC) wysokosc = EKRAN_WYSOKOSC - y;
    if (szerokosc <= 0 || wysokosc <= 0) return;

    for (int py = y; py < y + wysokosc; py++) {
        uint32_t offset = (uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)x;
        for (int px = 0; px < szerokosc; px++) {
            if (py >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) {
                obraz[offset + px] = KOLOR_PASKA_ZADAN;
            } else {
                obraz[offset + px] = tapeta_gradientowa ? kolor_tla_dla_x(x + px) : kolor_tapety;
            }
        }
    }
}

void odtworz_fragment_tla(int x, int y, int szerokosc, int wysokosc)
{
    odtworz_fragment_tla_bufor(x, y, szerokosc, wysokosc);
    odswiez_fragment_widoku(x, y, szerokosc, wysokosc);
}

void rysuj_piksel(int x, int y, uint32_t kolor)
{
    if (x < 0 || x >= EKRAN_SZEROKOSC || y < 0 || y >= EKRAN_WYSOKOSC) return;
    obraz[y * EKRAN_SZEROKOSC + x] = kolor;
}

void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint32_t kolor)
{
    for (int pion = 0; pion < wysokosc; pion++) {
        int py = y + pion;
        if (py < 0 || py >= EKRAN_WYSOKOSC) continue;
        uint32_t offset = (uint32_t)py * EKRAN_SZEROKOSC;
        for (int poziom = 0; poziom < szerokosc; poziom++) {
            int px = x + poziom;
            if (px >= 0 && px < EKRAN_SZEROKOSC) {
                obraz[offset + px] = kolor;
            }
        }
    }
}

void rysuj_kwadrat(int x, int y, int bok, uint32_t kolor)
{
    rysuj_prostokat(x, y, bok, bok, kolor);
}

static void narysuj_kursor_bitmapa(int x, int y)
{
    for (int cy = 0; cy < CURSOR_H; cy++) {
        int py = y + cy;
        if (py < 0 || py >= EKRAN_WYSOKOSC) continue;

        for (int cx = 0; cx < CURSOR_W; cx++) {
            uint8_t typ = kursor_bitmapa[cy][cx];
            if (typ == 0) continue;

            int px = x + cx;
            if (px < 0 || px >= EKRAN_SZEROKOSC) continue;

            uint32_t kolor = (typ == 1) ? 0x00000000 : 0x00FFFFFF;
            zapisz_piksel_framebuffera(px, py, kolor);
        }
    }
}

void narysuj_pasek_zadan(void)
{
    uint32_t start = (uint32_t)(EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) * EKRAN_SZEROKOSC;
    uint32_t size = (uint32_t)PASEK_ZADAN_WYSOKOSC * EKRAN_SZEROKOSC;
    for (uint32_t i = 0; i < size; i++) {
        obraz[start + i] = KOLOR_PASKA_ZADAN;
    }
    narysuj_czas_paska();
    narysuj_ikone_internetu();
    odswiez_fragment_widoku(0, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC, EKRAN_SZEROKOSC, PASEK_ZADAN_WYSOKOSC);
}

void wypelnij_ekran(uint32_t kolor)
{
    kolor_tapety = kolor;
    tapeta_gradientowa = 0;
    for (uint32_t i = 0; i < EKRAN_SZEROKOSC * EKRAN_WYSOKOSC; i++) {
        obraz[i] = kolor_tapety;
    }
    narysuj_pasek_zadan();
    narysuj_napis_systemowy();
    odswiez_widok();
}

void ustaw_kolor_tapety(uint32_t kolor)
{
    kolor_tapety = kolor;
    tapeta_gradientowa = 0;
    for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
        uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            obraz[offset + x] = kolor_tapety;
        }
    }
    narysuj_napis_systemowy();
    odswiez_fragment_widoku(0, 0, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC);
}

#define PLIK_TAPETA "TAPETA.CFG"

static uint32_t parsuj_hex(char **p)
{
    uint32_t v = 0;
    if (*p[0] == '0' && (*p[1] == 'x' || *p[1] == 'X')) {
        *p += 2;
    }
    while (**p) {
        char c = **p;
        uint32_t cyfra;
        if (c >= '0' && c <= '9') cyfra = (uint32_t)(c - '0');
        else if (c >= 'a' && c <= 'f') cyfra = (uint32_t)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') cyfra = (uint32_t)(c - 'A' + 10);
        else break;
        v = (v << 4) | cyfra;
        (*p)++;
    }
    return v;
}

static void narysuj_tapete_tlo(void)
{
    if (tapeta_gradientowa) {
        for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
            uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
            for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
                obraz[offset + x] = kolor_tla_dla_x(x);
            }
        }
    } else {
        for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
            uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
            for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
                obraz[offset + x] = kolor_tapety;
            }
        }
    }
    narysuj_napis_systemowy();
    odswiez_fragment_widoku(0, 0, EKRAN_SZEROKOSC, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC);
}

int tapeta_wczytaj(void)
{
    fs_reset_directory();
    char buf[64];
    int n = read_file(PLIK_TAPETA, buf, sizeof(buf) - 1);
    if (n <= 0) return 0;
    buf[n] = 0;
    char *p = buf;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    kolor_tapety = parsuj_hex(&p);
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    tapeta_gradientowa = (*p == '1') ? 1u : 0u;
    narysuj_tapete_tlo();
    return 1;
}

void tapeta_zapisz(void)
{
    char buf[32];
    int pos = 0;
    uint32_t v = kolor_tapety;
    buf[pos++] = '0';
    buf[pos++] = 'x';
    for (int i = 0; i < 8; i++) {
        buf[pos++] = "0123456789ABCDEF"[(v >> (28 - i * 4)) & 0xF];
    }
    buf[pos++] = ' ';
    buf[pos++] = tapeta_gradientowa ? '1' : '0';
    buf[pos++] = '\n';
    buf[pos] = 0;
    fs_reset_directory();
    if (write_file(PLIK_TAPETA, buf, (uint32_t)pos) != 0) {
        if (create_file(PLIK_TAPETA, (uint32_t)pos) != 0 ||
            write_file(PLIK_TAPETA, buf, (uint32_t)pos) != 0) {
            return;
        }
    }
}

void kursor_graficzny_init(void)
{
    kursor_x = EKRAN_SZEROKOSC / 2;
    kursor_y = EKRAN_WYSOKOSC / 2;
    odswiez_widok();
}

void kursor_graficzny_przesun(int delta_x, int delta_y)
{
    if (delta_x == 0 && delta_y == 0) return;

    int stary_x = kursor_x;
    int stary_y = kursor_y;

    kursor_x += delta_x;
    kursor_y += delta_y;
    if (kursor_x < 0) kursor_x = 0;
    if (kursor_y < 0) kursor_y = 0;
    if (kursor_x > EKRAN_SZEROKOSC - CURSOR_W) kursor_x = EKRAN_SZEROKOSC - CURSOR_W;
    if (kursor_y > EKRAN_WYSOKOSC - CURSOR_H) kursor_y = EKRAN_WYSOKOSC - CURSOR_H;

    if (kursor_x == stary_x && kursor_y == stary_y) return;

    odswiez_fragment_widoku(stary_x, stary_y, CURSOR_W, CURSOR_H);
}

void kursor_graficzny_pozycja(int *x, int *y)
{
    *x = kursor_x;
    *y = kursor_y;
}

void rysowanie_init(void)
{
    volatile uint8_t *mode_info = (volatile uint8_t *)0x50000;
    uint32_t lfb_address = *(volatile uint32_t *)(mode_info + 0x28);

    if (lfb_address != 0) {
        uint8_t wersja_vbe = *(volatile uint8_t *)0x500FF;
        uint16_t liniowy_pitch = 0;
        pamiec_graficzna = (volatile uint8_t *)lfb_address;
        framebuffer_pitch = *(volatile uint16_t *)(mode_info + 0x10);
        framebuffer_bytes_per_pixel = (mode_info[0x19] + 7) / 8;
        red_mask_size = mode_info[0x1F];
        red_field_position = mode_info[0x20];
        green_mask_size = mode_info[0x21];
        green_field_position = mode_info[0x22];
        blue_mask_size = mode_info[0x23];
        blue_field_position = mode_info[0x24];
        if (wersja_vbe >= 3) {
            liniowy_pitch = *(volatile uint16_t *)(mode_info + 0x32);
            if (liniowy_pitch) framebuffer_pitch = liniowy_pitch;
            if (mode_info[0x36]) {
                red_mask_size = mode_info[0x36];
                red_field_position = mode_info[0x37];
            }
            if (mode_info[0x38]) {
                green_mask_size = mode_info[0x38];
                green_field_position = mode_info[0x39];
            }
            if (mode_info[0x3A]) {
                blue_mask_size = mode_info[0x3A];
                blue_field_position = mode_info[0x3B];
            }
        }
    }

    tapeta_gradientowa = 1;
    for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
        uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            obraz[offset + x] = kolor_tla_dla_x(x);
        }
    }

    for (int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y < EKRAN_WYSOKOSC; y++) {
        uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            obraz[offset + x] = KOLOR_PASKA_ZADAN;
        }
    }

    internet_dostepny = 0;
    narysuj_czas_paska();
    narysuj_ikone_internetu();

    narysuj_napis_systemowy();

    odswiez_widok();
    kursor_graficzny_init();
}