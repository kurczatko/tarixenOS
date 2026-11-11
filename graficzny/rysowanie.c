#include "rysowanie.h"
#include "czcionka.h"
#include "okno.h"
#include "../system_plikow/system_plikow.h"
#include "../../drivers/bluetooth/bluetooth.h"
#include "menu_start.h"

#define CURSOR_W 12
#define CURSOR_H 18
#define KOLOR_TLA 0x0098C74C
#define KOLOR_PASKA_ZADAN 0x002D3E50

static inline uint8_t vbe_read8(volatile uint8_t *base, uint32_t off) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    volatile char *p = (volatile char *)base;
    return *(volatile uint8_t *)(p + off);
#pragma GCC diagnostic pop
}
#define KOLOR_TEKST_BIALY 0x00FFFFFF

#define NAPIS_SZEROKOSC (27 * 8)
#define NAPIS_X (EKRAN_SZEROKOSC - NAPIS_SZEROKOSC - 16)
#define NAPIS_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 16)
#define NAPIS_WYSOKOSC 8

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
#define TAPETA_BITMAPA_ADRES 0x02000000u
#define TAPETA_BITMAPA_BAJTY (EKRAN_SZEROKOSC * EKRAN_WYSOKOSC * 4u)
#define PLIK_TAPETA_CIEMNA "TAPETA.RAW"
#define PLIK_TAPETA_JASNA "TAPETAJ.RAW"
static uint32_t *tapeta_bitmapa = (uint32_t *)TAPETA_BITMAPA_ADRES;
static int tapeta_bitmapa_wczytana;
static int tapeta_jasna;
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

#define PASEK_ICON_W 24
#define PASEK_ICON_H 24
#define PASEK_ICON_SPACJE 4
#define PASEK_ICON_START_X 80

static int tekst_rowne(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a++ != *b++) return 0;
    }
    return *a == *b;
}

static const uint8_t ikona_notatnik_pasek[12][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,0,0,1,1,0},
    {0,0,0,0,0,0,0,0,1,1,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,0,1,1,0,0,0,0,0,0},
    {0,0,0,1,1,0,0,0,0,0,0,0},
    {0,0,1,1,0,0,0,0,0,0,0,0},
    {0,1,1,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0}
};

static const uint8_t ikona_pliki_pasek[12][12] = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

static const uint8_t ikona_www_pasek[12][12] = {
    {0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,1,1,1,2,2,1,1,1,0,0},
    {0,1,1,2,1,2,2,1,2,1,1,0},
    {1,1,2,1,1,2,2,1,1,2,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,2,2,2,2,2,2,2,2,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,2,1,1,2,2,1,1,2,1,1},
    {0,1,1,2,1,2,2,1,2,1,1,0},
    {0,0,1,1,1,2,2,1,1,1,0,0},
    {0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0}
};

void pasek_zadan_rysuj_ikone(int x, int y, const char *tytul)
{
    rysuj_prostokat(x, y, PASEK_ICON_W, PASEK_ICON_H, 0x003A4B5C);
    int ix = x + (PASEK_ICON_W - 12) / 2;
    int iy = y + (PASEK_ICON_H - 12) / 2;
    if (tekst_rowne(tytul, "NOTATNIK")) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                if (ikona_notatnik_pasek[i][j])
                    rysuj_piksel(ix + j, iy + i, KOLOR_ZOLTY);
            }
        }
    } else if (tekst_rowne(tytul, "Pliki")) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                if (ikona_pliki_pasek[i][j])
                    rysuj_piksel(ix + j, iy + i, KOLOR_ZOLTY);
            }
        }
    } else if (tekst_rowne(tytul, "PRZEGLADARKA HTTP/HTTPS")) {
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 12; j++) {
                uint8_t p = ikona_www_pasek[i][j];
                if (p)
                    rysuj_piksel(ix + j, iy + i, p == 1 ? KOLOR_NIEBIESKI : 0x0068B8FF);
            }
        }
    } else {
        rysuj_prostokat(ix + 2, iy + 2, 8, 8, KOLOR_BIALY);
        rysuj_prostokat(ix + 4, iy + 4, 4, 4, KOLOR_CZARNY);
    }
    rysuj_prostokat(x, y, PASEK_ICON_W, 1, KOLOR_BIALY);
    rysuj_prostokat(x, y, 1, PASEK_ICON_H, KOLOR_BIALY);
    rysuj_prostokat(x + PASEK_ICON_W - 1, y, 1, PASEK_ICON_H, KOLOR_BIALY);
    rysuj_prostokat(x, y + PASEK_ICON_H - 1, PASEK_ICON_W, 1, KOLOR_BIALY);
}

void pasek_zadan_rysuj_ikony_minimalizowanych(void)
{
    int x = PASEK_ICON_START_X;
    int y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 3;
    x = PASEK_ICON_START_X;
    for (int i = 0; i < okno_liczba_okien(); i++) {
        struct okno *okno = okno_pobierz_okno(i);
        if (!okno || !okno->otwarte || !okno->zminimalizowane) continue;
        pasek_zadan_rysuj_ikone(x, y, okno->tytul);
        x += PASEK_ICON_W + PASEK_ICON_SPACJE;
    }
}

struct okno *pasek_zadan_ikona_na_pozycji(int x, int y)
{
    int start_y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 3;
    if (y < start_y || y > start_y + PASEK_ICON_H) return 0;
    int ix = PASEK_ICON_START_X;
    for (int i = 0; i < okno_liczba_okien(); i++) {
        struct okno *okno = okno_pobierz_okno(i);
        if (!okno || !okno->otwarte || !okno->zminimalizowane) continue;
        if (x >= ix && x < ix + PASEK_ICON_W) return okno;
        ix += PASEK_ICON_W + PASEK_ICON_SPACJE;
    }
    return 0;
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

static void kopiuj_pamiec(volatile uint8_t *dst, const uint32_t *src, uint32_t bajty)
{
    volatile uint32_t *d = (volatile uint32_t *)dst;
    uint32_t dwords = bajty / 4;
    for (uint32_t i = 0; i < dwords; i++) {
        d[i] = src[i];
    }
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
            kopiuj_pamiec(&pamiec_graficzna[offset * 4], &obraz[offset], szerokosc * 4);
        } else {
            for (int kolumna = 0; kolumna < szerokosc; kolumna++)
                zapisz_piksel_framebuffera(x + kolumna, y + wiersz, obraz[offset + kolumna]);
        }
    }
    narysuj_kursor_bitmapa(kursor_x, kursor_y);
}

static void narysuj_napis_systemowy(void)
{
    int tekst_x = NAPIS_X;
    int tekst_y = NAPIS_Y;
    rysuj_tekst_chicago(tekst_x, tekst_y, "TarixenOS Tryb Graficzny", KOLOR_TEKST_BIALY);
}

void odswiez_widok(void)
{
    if (framebuffer_ma_szybki_format()) {
        kopiuj_pamiec(pamiec_graficzna, obraz, EKRAN_SZEROKOSC * EKRAN_WYSOKOSC * 4);
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
                continue;
            } else if (tapeta_bitmapa_wczytana) {
                obraz[offset + px] = tapeta_bitmapa[offset + px];
            } else {
                obraz[offset + px] = tapeta_gradientowa ? kolor_tla_dla_x(x + px) : kolor_tapety;
            }
        }
    }

    if (x < NAPIS_X + NAPIS_SZEROKOSC && x + szerokosc > NAPIS_X &&
        y < NAPIS_Y + NAPIS_WYSOKOSC && y + wysokosc > NAPIS_Y) {
        narysuj_napis_systemowy();
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

#define BLUR_RADIUS 4
static const int gauss_kernel[9] = {1, 4, 9, 14, 16, 14, 9, 4, 1};
#define GAUSS_SUM 72
static uint32_t blur_temp[EKRAN_SZEROKOSC * PASEK_ZADAN_WYSOKOSC];

static uint32_t tapeta_piksel(int x, int y)
{
    if (x < 0) x = 0;
    if (x >= EKRAN_SZEROKOSC) x = EKRAN_SZEROKOSC - 1;
    if (y < 0) y = 0;
    if (y >= EKRAN_WYSOKOSC) y = EKRAN_WYSOKOSC - 1;
    if (tapeta_bitmapa_wczytana) {
        return tapeta_bitmapa[(uint32_t)y * EKRAN_SZEROKOSC + (uint32_t)x];
    }
    if (tapeta_gradientowa) {
        return kolor_tla_dla_x(x);
    }
    return kolor_tapety;
}

static void rozmyj_pasek_zadan(void)
{
    int paske_y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC;

    for (int py = 0; py < PASEK_ZADAN_WYSOKOSC; py++) {
        int y = paske_y + py;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            uint32_t r = 0, g = 0, b = 0;
            for (int k = 0; k < 9; k++) {
                int sx = x + (k - 4);
                uint32_t px = tapeta_piksel(sx, y);
                int w = gauss_kernel[k];
                r += ((px >> 16) & 0xFF) * w;
                g += ((px >> 8) & 0xFF) * w;
                b += (px & 0xFF) * w;
            }
            blur_temp[(uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)x] =
                (r / GAUSS_SUM << 16) | (g / GAUSS_SUM << 8) | (b / GAUSS_SUM);
        }
    }

    for (int py = 0; py < PASEK_ZADAN_WYSOKOSC; py++) {
        int y = paske_y + py;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            uint32_t r = 0, g = 0, b = 0;
            for (int k = 0; k < 9; k++) {
                int sy = py + (k - 4);
                if (sy < 0) sy = 0;
                if (sy >= PASEK_ZADAN_WYSOKOSC) sy = PASEK_ZADAN_WYSOKOSC - 1;
                uint32_t px = blur_temp[(uint32_t)sy * EKRAN_SZEROKOSC + (uint32_t)x];
                int w = gauss_kernel[k];
                r += ((px >> 16) & 0xFF) * w;
                g += ((px >> 8) & 0xFF) * w;
                b += (px & 0xFF) * w;
            }
            uint32_t blurred = (r / GAUSS_SUM << 16) | (g / GAUSS_SUM << 8) | (b / GAUSS_SUM);

            uint32_t taskbar = KOLOR_PASKA_ZADAN;
            int krawedz = py < 2 ? py : (py > PASEK_ZADAN_WYSOKOSC - 3 ? PASEK_ZADAN_WYSOKOSC - 1 - py : 2);
            uint32_t alpha = 60 + krawedz * 15;
            r = ((blurred >> 16) & 0xFF) * (255 - alpha) + ((taskbar >> 16) & 0xFF) * alpha;
            g = ((blurred >> 8) & 0xFF) * (255 - alpha) + ((taskbar >> 8) & 0xFF) * alpha;
            b = (blurred & 0xFF) * (255 - alpha) + (taskbar & 0xFF) * alpha;

            obraz[(uint32_t)y * EKRAN_SZEROKOSC + (uint32_t)x] = (r / 255 << 16) | (g / 255 << 8) | (b / 255);
        }
    }
}

void narysuj_pasek_zadan(void)
{
    rozmyj_pasek_zadan();
    menu_start_rysuj_przycisk();
    pasek_zadan_rysuj_ikony_minimalizowanych();
    narysuj_czas_paska();
    narysuj_ikone_internetu();
    bluetooth_ui_rysuj_pasek();
    quickcam_ui_rysuj_pasek();
    odswiez_fragment_widoku(0, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC, EKRAN_SZEROKOSC, PASEK_ZADAN_WYSOKOSC);
}

static const int gauss_kernel_5[5] = {1, 4, 6, 4, 1};
#define GAUSS_SUM_5 16
static uint32_t blur_row_temp[EKRAN_SZEROKOSC];

void rozmyj_prostokat(int x, int y, int szerokosc, int wysokosc)
{
    if (x < 0) { szerokosc += x; x = 0; }
    if (y < 0) { wysokosc += y; y = 0; }
    if (x + szerokosc > EKRAN_SZEROKOSC) szerokosc = EKRAN_SZEROKOSC - x;
    if (y + wysokosc > EKRAN_WYSOKOSC) wysokosc = EKRAN_WYSOKOSC - y;
    if (szerokosc <= 0 || wysokosc <= 0) return;

    for (int py = y; py < y + wysokosc; py++) {
        for (int px = x; px < x + szerokosc; px++) {
            uint32_t r = 0, g = 0, b = 0;
            for (int k = 0; k < 5; k++) {
                int sx = px + (k - 2);
                if (sx < 0) sx = 0;
                if (sx >= EKRAN_SZEROKOSC) sx = EKRAN_SZEROKOSC - 1;
                uint32_t p = obraz[(uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)sx];
                int w = gauss_kernel_5[k];
                r += ((p >> 16) & 0xFF) * w;
                g += ((p >> 8) & 0xFF) * w;
                b += (p & 0xFF) * w;
            }
            blur_row_temp[px] = (r / GAUSS_SUM_5 << 16) | (g / GAUSS_SUM_5 << 8) | (b / GAUSS_SUM_5);
        }
        for (int px = x; px < x + szerokosc; px++) {
            obraz[(uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)px] = blur_row_temp[px];
        }
    }

    for (int px = x; px < x + szerokosc; px++) {
        for (int py = y; py < y + wysokosc; py++) {
            uint32_t r = 0, g = 0, b = 0;
            for (int k = 0; k < 5; k++) {
                int sy = py + (k - 2);
                if (sy < 0) sy = 0;
                if (sy >= EKRAN_WYSOKOSC) sy = EKRAN_WYSOKOSC - 1;
                uint32_t p = obraz[(uint32_t)sy * EKRAN_SZEROKOSC + (uint32_t)px];
                int w = gauss_kernel_5[k];
                r += ((p >> 16) & 0xFF) * w;
                g += ((p >> 8) & 0xFF) * w;
                b += (p & 0xFF) * w;
            }
            uint32_t blurred = (r / GAUSS_SUM_5 << 16) | (g / GAUSS_SUM_5 << 8) | (b / GAUSS_SUM_5);

            uint32_t taskbar = KOLOR_PASKA_ZADAN;
            uint32_t alpha = 70;
            r = ((blurred >> 16) & 0xFF) * (255 - alpha) + ((taskbar >> 16) & 0xFF) * alpha;
            g = ((blurred >> 8) & 0xFF) * (255 - alpha) + ((taskbar >> 8) & 0xFF) * alpha;
            b = (blurred & 0xFF) * (255 - alpha) + (taskbar & 0xFF) * alpha;

            obraz[(uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)px] = (r / 255 << 16) | (g / 255 << 8) | (b / 255);
        }
    }
}

void rysuj_prostokat_polprzezroczysty(int x, int y, int szerokosc, int wysokosc, uint32_t kolor, uint32_t alpha)
{
    if (x < 0) { szerokosc += x; x = 0; }
    if (y < 0) { wysokosc += y; y = 0; }
    if (x + szerokosc > EKRAN_SZEROKOSC) szerokosc = EKRAN_SZEROKOSC - x;
    if (y + wysokosc > EKRAN_WYSOKOSC) wysokosc = EKRAN_WYSOKOSC - y;
    if (szerokosc <= 0 || wysokosc <= 0) return;

    for (int py = y; py < y + wysokosc; py++) {
        uint32_t offset = (uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)x;
        for (int px = 0; px < szerokosc; px++) {
            uint32_t dest = obraz[offset + px];
            uint32_t r = ((dest >> 16) & 0xFF) * (255 - alpha) + ((kolor >> 16) & 0xFF) * alpha;
            uint32_t g = ((dest >> 8) & 0xFF) * (255 - alpha) + ((kolor >> 8) & 0xFF) * alpha;
            uint32_t b = (dest & 0xFF) * (255 - alpha) + (kolor & 0xFF) * alpha;
            obraz[offset + px] = (r / 255 << 16) | (g / 255 << 8) | (b / 255);
        }
    }
}

void wypelnij_ekran(uint32_t kolor)
{
    kolor_tapety = kolor;
    tapeta_gradientowa = 0;
    tapeta_bitmapa_wczytana = 0;
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
    tapeta_bitmapa_wczytana = 0;
    for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
        uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
        for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
            obraz[offset + x] = kolor_tapety;
        }
    }
    narysuj_napis_systemowy();
    if (okno_liczba_okien() > 0) {
        okno_odrysuj_wszystkie();
    } else {
        odswiez_fragment_widoku(0, 0, EKRAN_SZEROKOSC,
                                EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC);
    }
}

uint32_t pobierz_kolor_tapety(void)
{
    return kolor_tapety;
}

#define PLIK_TAPETA "TAPETA.CFG"

static void narysuj_tapete_tlo(void);

int tapeta_czy_jasna(void)
{
    return tapeta_jasna;
}

int tapeta_ustaw_obraz(int jasna)
{
    const char *plik = jasna ? PLIK_TAPETA_JASNA : PLIK_TAPETA_CIEMNA;
    fs_reset_directory();
    int bajty = read_file(plik, (char *)tapeta_bitmapa, TAPETA_BITMAPA_BAJTY);
    if (bajty != (int)TAPETA_BITMAPA_BAJTY) return 0;
    tapeta_jasna = jasna != 0;
    tapeta_bitmapa_wczytana = 1;
    tapeta_gradientowa = 0;
    narysuj_tapete_tlo();
    return 1;
}

static uint32_t parsuj_hex(char **p)
{
    uint32_t v = 0;
    if ((*p)[0] == '0' && ((*p)[1] == 'x' || (*p)[1] == 'X')) {
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
    if (tapeta_bitmapa_wczytana) {
        for (int y = 0; y < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC; y++) {
            uint32_t offset = (uint32_t)y * EKRAN_SZEROKOSC;
            for (int x = 0; x < EKRAN_SZEROKOSC; x++) {
                obraz[offset + x] = tapeta_bitmapa[offset + x];
            }
        }
    } else if (tapeta_gradientowa) {
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
    if (okno_liczba_okien() > 0) {
        okno_odrysuj_wszystkie();
    } else {
        odswiez_fragment_widoku(0, 0, EKRAN_SZEROKOSC,
                                EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC);
    }
}

int tapeta_wczytaj(void)
{
    char buf[64];
    char tryb = '2';
    fs_reset_directory();
    int n = read_file(PLIK_TAPETA, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0;
        char *p = buf;
        while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
        kolor_tapety = parsuj_hex(&p);
        while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
        if (*p) tryb = *p;
    }
    tapeta_jasna = tryb == '3';
    tapeta_gradientowa = tryb == '1';
    tapeta_bitmapa_wczytana = 0;
    if (tryb == '2' || tryb == '3') {
        const char *plik = tapeta_jasna ? PLIK_TAPETA_JASNA : PLIK_TAPETA_CIEMNA;
        fs_reset_directory();
        int bitmapa_bajty = read_file(plik, (char *)tapeta_bitmapa,
                                      TAPETA_BITMAPA_BAJTY);
        tapeta_bitmapa_wczytana = bitmapa_bajty == (int)TAPETA_BITMAPA_BAJTY;
        if (!tapeta_bitmapa_wczytana && tapeta_jasna) {
            tapeta_jasna = 0;
            fs_reset_directory();
            bitmapa_bajty = read_file(PLIK_TAPETA_CIEMNA, (char *)tapeta_bitmapa,
                                      TAPETA_BITMAPA_BAJTY);
            tapeta_bitmapa_wczytana = bitmapa_bajty == (int)TAPETA_BITMAPA_BAJTY;
        }
        if (!tapeta_bitmapa_wczytana && n <= 0) return 0;
        tapeta_gradientowa = 0;
    }
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
    buf[pos++] = tapeta_bitmapa_wczytana ? (tapeta_jasna ? '3' : '2') : (tapeta_gradientowa ? '1' : '0');
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
    volatile uint8_t *mode_info = (volatile uint8_t *)0x0500;
    uint32_t lfb_address = (uint32_t)vbe_read8(mode_info, 0x28)
                         | ((uint32_t)vbe_read8(mode_info, 0x29) << 8)
                         | ((uint32_t)vbe_read8(mode_info, 0x2A) << 16)
                         | ((uint32_t)vbe_read8(mode_info, 0x2B) << 24);

    if (lfb_address != 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
        uint8_t wersja_vbe = *(volatile uint8_t *)0x05FF;
        uint16_t liniowy_pitch = 0;
        pamiec_graficzna = (volatile uint8_t *)lfb_address;
        framebuffer_pitch = (uint16_t)(vbe_read8(mode_info, 0x10) | (vbe_read8(mode_info, 0x11) << 8));
        framebuffer_bytes_per_pixel = (vbe_read8(mode_info, 0x19) + 7) / 8;
        red_mask_size = vbe_read8(mode_info, 0x1F);
        red_field_position = vbe_read8(mode_info, 0x20);
        green_mask_size = vbe_read8(mode_info, 0x21);
        green_field_position = vbe_read8(mode_info, 0x22);
        blue_mask_size = vbe_read8(mode_info, 0x23);
        blue_field_position = vbe_read8(mode_info, 0x24);
        if (wersja_vbe >= 3) {
            liniowy_pitch = (uint16_t)(vbe_read8(mode_info, 0x32) | (vbe_read8(mode_info, 0x33) << 8));
            if (liniowy_pitch) framebuffer_pitch = liniowy_pitch;
            if (vbe_read8(mode_info, 0x36)) {
                red_mask_size = vbe_read8(mode_info, 0x36);
                red_field_position = vbe_read8(mode_info, 0x37);
            }
            if (vbe_read8(mode_info, 0x38)) {
                green_mask_size = vbe_read8(mode_info, 0x38);
                green_field_position = vbe_read8(mode_info, 0x39);
            }
            if (vbe_read8(mode_info, 0x3A)) {
                blue_mask_size = vbe_read8(mode_info, 0x3A);
                blue_field_position = vbe_read8(mode_info, 0x3B);
            }
        }
#pragma GCC diagnostic pop
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

static uint8_t kursor_tekstowy_stan = 0;
static int kursor_tekstowy_licznik = 0;

void kursor_tekstowy_tick(void)
{
    kursor_tekstowy_licznik++;
    if (kursor_tekstowy_licznik >= 8) {
        kursor_tekstowy_licznik = 0;
        kursor_tekstowy_stan = !kursor_tekstowy_stan;
    }
}

uint8_t kursor_tekstowy_widoczny(void)
{
    return kursor_tekstowy_stan;
}

void kursor_tekstowy_rysuj(int x, int y)
{
    if (!kursor_tekstowy_stan) return;
    rysuj_prostokat(x, y, 2, 16, 0x00000000);
}

void quickcam_ui_init(void)
{
}

void quickcam_ui_rysuj_pasek(void)
{
    extern unsigned char quickcam_driver_is_detected(void);
    extern unsigned char quickcam_driver_is_streaming(void);
    if (!quickcam_driver_is_detected()) return;
    uint8_t streaming = quickcam_driver_is_streaming();
    uint32_t kolor = streaming ? KOLOR_CZERWONY : KOLOR_BIALY;
    rysuj_prostokat(KAMERA_IKONA_X, KAMERA_IKONA_Y, KAMERA_IKONA_W, KAMERA_IKONA_H, 0x003A4B5C);
    rysuj_prostokat(KAMERA_IKONA_X + 6, KAMERA_IKONA_Y + 6, 12, 10, kolor);
    rysuj_kwadrat(KAMERA_IKONA_X + 10, KAMERA_IKONA_Y + 10, 4, 0x00000000);
    if (streaming) {
        rysuj_prostokat(KAMERA_IKONA_X + 20, KAMERA_IKONA_Y + 2, 4, 4, KOLOR_CZERWONY);
    }
    rysuj_prostokat(KAMERA_IKONA_X, KAMERA_IKONA_Y, KAMERA_IKONA_W, 1, 0x00FFFFFF);
    rysuj_prostokat(KAMERA_IKONA_X, KAMERA_IKONA_Y, 1, KAMERA_IKONA_H, 0x00FFFFFF);
    rysuj_prostokat(KAMERA_IKONA_X + KAMERA_IKONA_W - 1, KAMERA_IKONA_Y, 1, KAMERA_IKONA_H, 0x00FFFFFF);
    rysuj_prostokat(KAMERA_IKONA_X, KAMERA_IKONA_Y + KAMERA_IKONA_H - 1, KAMERA_IKONA_W, 1, 0x00FFFFFF);
}

int quickcam_ui_icon_at(int x, int y)
{
    if (y < KAMERA_IKONA_Y || y >= KAMERA_IKONA_Y + KAMERA_IKONA_H) return 0;
    if (x < KAMERA_IKONA_X || x >= KAMERA_IKONA_X + KAMERA_IKONA_W) return 0;
    return 1;
}

int quickcam_ui_mouse_click(int x, int y)
{
    if (!quickcam_ui_icon_at(x, y)) return 0;
    extern void kamera_ui_toggle(void);
    kamera_ui_toggle();
    return 1;
}

void quickcam_ui_toggle(void)
{
    extern void kamera_ui_toggle(void);
    kamera_ui_toggle();
}