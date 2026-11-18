/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <graphics/menu_start.h>
#include <graphics/czcionka.h>
#include <graphics/rysowanie.h>
#include <graphics/okno.h>
#include <drivers/input/mouse.h>
#include <kernel/sys/shutdown.h>
#include <kernel/apps/notatnik.h>
#include <graphics/eksplorator_plikow.h>
#include <kernel/apps/browser.h>
#include <kernel/apps/kalkulator.h>
#include <kernel/apps/gra.h>
#include <kernel/apps/duszki.h>
#include <kernel/apps/malowanie.h>
#include <kernel/core/zadania.h>
#include <graphics/terminal.h>

#define START_X 0
#define START_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC)
#define START_SZEROKOSC 72
#define START_WYSOKOSC PASEK_ZADAN_WYSOKOSC

#define LICZBA_POZYCJI 11
#define POZYCJA_WYSOKOSC 38
#define POZYCJA_ODSTEP 42
#define MENU_MARGINES 12

#define MENU_X 0
#define MENU_SZEROKOSC 170
#define MENU_WYSOKOSC (MENU_MARGINES + (LICZBA_POZYCJI - 1) * POZYCJA_ODSTEP + POZYCJA_WYSOKOSC + 10)
#define MENU_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - MENU_WYSOKOSC)

#define KOLOR_PRZYCISKU_START 0x004A5D6E
#define KOLOR_RAMKA_MENU      0x003A4B5C
#define KOLOR_TLO_MENU        0x001E293B
#define KOLOR_ELEMENT_MENU    0x00334155
#define KOLOR_TEKST_BIALY     0x00FFFFFF

static uint8_t menu_otwarte;

extern struct okno *aktywne_okno;

static int ikona_abs(int a)
{
    return a < 0 ? -a : a;
}

static int ikona_pierw(int n)
{
    int r = 0;
    if (n <= 0)
        return 0;
    while ((r + 1) * (r + 1) <= n)
        r++;
    return r;
}

static uint32_t kolor_mieszaj(uint32_t a, uint32_t b, int t, int m)
{
    if (m <= 0)
        return a;
    if (t < 0)
        t = 0;
    if (t > m)
        t = m;
    int r = ((int)((a >> 16) & 255) * (m - t) + (int)((b >> 16) & 255) * t) / m;
    int g = ((int)((a >> 8) & 255) * (m - t) + (int)((b >> 8) & 255) * t) / m;
    int bl = ((int)(a & 255) * (m - t) + (int)(b & 255) * t) / m;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)bl;
}

static uint32_t kolor_jasniej(uint32_t c, int d)
{
    int r = (int)((c >> 16) & 255) + d;
    int g = (int)((c >> 8) & 255) + d;
    int b = (int)(c & 255) + d;
    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static void ikona_wypelnij(int x, int y, int w, int h, uint32_t c)
{
    for (int iy = 0; iy < h; iy++)
        for (int ix = 0; ix < w; ix++)
            rysuj_piksel(x + ix, y + iy, c);
}

static int ikona_wciecie(int i, int h, int r)
{
    int k;
    int dy2;
    int s;
    if (r <= 0)
        return 0;
    if (i < r)
        k = r - i;
    else if (i >= h - r)
        k = i - (h - r) + 1;
    else
        return 0;
    dy2 = 2 * k - 1;
    s = ikona_pierw(4 * r * r - dy2 * dy2);
    return r - (s + 1) / 2;
}

static void ikona_cien(int x, int y, int w, int h, int r)
{
    for (int g = 2; g >= 0; g--) {
        int sx = x + 1 - g;
        int sy = y + 2 - g;
        int sw = w + 2 * g;
        int sh = h + 2 * g;
        int sr = r + g;
        for (int i = 0; i < sh; i++) {
            int in = ikona_wciecie(i, sh, sr);
            rysuj_prostokat_polprzezroczysty(sx + in, sy + i, sw - 2 * in, 1, 0x00000000, 14);
        }
    }
}

static void ikona_cien_elipsa(int cx, int cy, int rx, int ry)
{
    for (int g = 0; g < 3; g++) {
        int a = rx + 4 - 2 * g;
        int b = ry + 2 - g;
        for (int i = -b; i <= b; i++) {
            int half = a * ikona_pierw(b * b - i * i) / (b > 0 ? b : 1);
            rysuj_prostokat_polprzezroczysty(cx - half, cy + i, 2 * half, 1, 0x00000000, 14);
        }
    }
}

static void ikona_rect(int x, int y, int w, int h, int r, uint32_t gora, uint32_t dol)
{
    uint32_t krawedz = kolor_jasniej(dol, -60);
    for (int i = 0; i < h; i++) {
        int in = ikona_wciecie(i, h, r);
        uint32_t c = kolor_mieszaj(gora, dol, i, h - 1);
        for (int j = in; j < w - in; j++) {
            uint32_t p = c;
            if (i == 0 || i == h - 1 || j == in || j == w - in - 1)
                p = krawedz;
            else if (i == 1)
                p = kolor_jasniej(c, 45);
            rysuj_piksel(x + j, y + i, p);
        }
    }
}

static void ikona_kolo(int cx, int cy, int r, uint32_t jasny, uint32_t ciemny)
{
    int hx = cx - r / 3;
    int hy = cy - r / 3;
    int md2 = 9 * r * r / 4;
    if (md2 < 1)
        md2 = 1;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            int d2 = dx * dx + dy * dy;
            if (d2 <= r * r) {
                int px = cx + dx;
                int py = cy + dy;
                int ex = px - hx;
                int ey = py - hy;
                uint32_t c = kolor_mieszaj(jasny, ciemny, ex * ex + ey * ey, md2);
                if (r > 1 && d2 > (r - 1) * (r - 1))
                    c = kolor_jasniej(c, -45);
                rysuj_piksel(px, py, c);
            }
        }
    }
}

static void ikona_linia(int x0, int y0, int x1, int y1, int grubosc, uint32_t c)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int n = ikona_abs(dx) > ikona_abs(dy) ? ikona_abs(dx) : ikona_abs(dy);
    if (n == 0)
        n = 1;
    for (int s = 0; s <= n; s++) {
        int px = x0 + dx * s / n;
        int py = y0 + dy * s / n;
        ikona_wypelnij(px - grubosc / 2, py - grubosc / 2, grubosc, grubosc, c);
    }
}

static void ikona_notatnik(int x, int y)
{
    ikona_cien(x + 4, y + 3, 24, 27, 3);
    ikona_rect(x + 4, y + 3, 24, 27, 3, 0x00FFFFFF, 0x00D9DEE8);
    ikona_rect(x + 4, y + 3, 24, 7, 3, 0x005B9BEA, 0x002457B5);
    for (int k = 0; k < 4; k++)
        ikona_kolo(x + 9 + k * 5, y + 6, 2, 0x00E5E7EB, 0x006B7280);
    for (int k = 0; k < 4; k++) {
        ikona_wypelnij(x + 8, y + 14 + k * 4, 16, 1, 0x00A9B6CE);
        ikona_wypelnij(x + 8, y + 15 + k * 4, 16, 1, 0x00EEF1F7);
    }
    ikona_linia(x + 14, y + 27, x + 27, y + 14, 3, 0x00F5B820);
    ikona_linia(x + 15, y + 28, x + 28, y + 15, 1, 0x00C98A0A);
    ikona_linia(x + 12, y + 29, x + 14, y + 27, 2, 0x00E9C99B);
    ikona_wypelnij(x + 11, y + 29, 2, 2, 0x003A2A1A);
    ikona_kolo(x + 28, y + 13, 2, 0x00FF9AAE, 0x00D44A6A);
}

static void ikona_konta(int x, int y)
{
    ikona_cien(x + 1, y + 1, 30, 30, 7);
    ikona_rect(x + 1, y + 1, 30, 30, 7, 0x006FB1FF, 0x001E4FA8);
    ikona_rect(x + 7, y + 19, 18, 10, 5, 0x00F4F7FB, 0x00B4C2D8);
    ikona_kolo(x + 16, y + 12, 6, 0x00FFE0C2, 0x00E0A472);
}

static void ikona_wylacz(int x, int y)
{
    int cx = x + 16;
    int cy = y + 17;
    ikona_cien(x + 1, y + 1, 30, 30, 7);
    ikona_rect(x + 1, y + 1, 30, 30, 7, 0x00FF7A6B, 0x00B3151B);
    for (int dy = -9; dy <= 9; dy++) {
        for (int dx = -9; dx <= 9; dx++) {
            int d2 = dx * dx + dy * dy;
            if (d2 <= 81 && d2 >= 49 && !(dy < 0 && ikona_abs(dx) <= 3))
                rysuj_piksel(cx + dx, cy + dy, kolor_mieszaj(0x00FFFFFF, 0x00FFC9C9, dy + 9, 18));
        }
    }
    ikona_rect(x + 14, y + 7, 4, 11, 2, 0x00FFFFFF, 0x00FFD6D6);
}

static void ikona_pliki(int x, int y)
{
    ikona_cien(x + 1, y + 6, 30, 22, 3);
    ikona_rect(x + 2, y + 5, 13, 7, 2, 0x00E0A22C, 0x00B87612);
    ikona_rect(x + 2, y + 8, 28, 19, 2, 0x00E8AE35, 0x00B87612);
    ikona_rect(x + 5, y + 9, 22, 12, 1, 0x00FFFFFF, 0x00DDE3EE);
    ikona_wypelnij(x + 8, y + 12, 14, 1, 0x00A9B6CE);
    ikona_wypelnij(x + 8, y + 15, 10, 1, 0x00A9B6CE);
    ikona_rect(x + 1, y + 14, 30, 15, 3, 0x00FFD667, 0x00F0A21C);
}

static void ikona_www(int x, int y)
{
    int cx = x + 16;
    int cy = y + 16;
    ikona_cien_elipsa(cx, y + 30, 10, 2);
    ikona_kolo(cx, cy, 14, 0x007CC4FF, 0x000F3F9C);
    for (int dy = -12; dy <= 12; dy++) {
        for (int dx = -12; dx <= 12; dx++) {
            if (dx * dx + dy * dy > 144)
                continue;
            if ((dx + 5) * (dx + 5) + (dy + 4) * (dy + 4) <= 20 ||
                (dx - 5) * (dx - 5) + (dy + 6) * (dy + 6) <= 9 ||
                (dx + 3) * (dx + 3) + (dy - 6) * (dy - 6) <= 16 ||
                (dx - 7) * (dx - 7) + (dy - 4) * (dy - 4) <= 9)
                rysuj_piksel(cx + dx, cy + dy, kolor_mieszaj(0x007BD36B, 0x001F7A2E, dy + 13, 26));
        }
    }
    ikona_kolo(cx - 6, cy - 8, 2, 0x00FFFFFF, 0x00CFE8FF);
}

static void ikona_kalkulator(int x, int y)
{
    ikona_cien(x + 4, y + 1, 24, 30, 4);
    ikona_rect(x + 4, y + 1, 24, 30, 4, 0x006B7587, 0x00262C38);
    ikona_rect(x + 7, y + 4, 18, 7, 2, 0x00B7F0A5, 0x005FA25A);
    ikona_wypelnij(x + 14, y + 6, 2, 3, 0x001D4B1F);
    ikona_wypelnij(x + 17, y + 6, 2, 3, 0x001D4B1F);
    ikona_wypelnij(x + 20, y + 6, 3, 3, 0x001D4B1F);
    for (int r = 0; r < 3; r++) {
        for (int k = 0; k < 3; k++) {
            if (k == 2)
                ikona_rect(x + 7 + k * 6, y + 13 + r * 5, 5, 4, 1, 0x00FFB347, 0x00E0700E);
            else
                ikona_rect(x + 7 + k * 6, y + 13 + r * 5, 5, 4, 1, 0x00F2F4F8, 0x00B6BECE);
        }
    }
}

static void ikona_gra(int x, int y)
{
    ikona_cien(x + 1, y + 8, 30, 18, 8);
    ikona_kolo(x + 6, y + 22, 5, 0x006F788D, 0x00232834);
    ikona_kolo(x + 26, y + 22, 5, 0x006F788D, 0x00232834);
    ikona_rect(x + 1, y + 8, 30, 16, 7, 0x008791A6, 0x002C3240);
    ikona_rect(x + 6, y + 14, 9, 3, 1, 0x00596074, 0x00131720);
    ikona_rect(x + 9, y + 11, 3, 9, 1, 0x00596074, 0x00131720);
    ikona_kolo(x + 24, y + 13, 2, 0x00FF8A80, 0x00C62828);
    ikona_kolo(x + 21, y + 16, 2, 0x009BE58F, 0x002E8B2E);
    ikona_kolo(x + 27, y + 16, 2, 0x008FB8FF, 0x001E50C8);
    ikona_kolo(x + 24, y + 19, 2, 0x00FFF08A, 0x00D79A10);
    ikona_wypelnij(x + 14, y + 19, 4, 1, 0x00171B24);
}

static void ikona_malowanie(int x, int y)
{
    ikona_cien_elipsa(x + 16, y + 28, 12, 2);
    ikona_rect(x + 1, y + 5, 30, 22, 10, 0x00E9C48B, 0x00A9733A);
    ikona_kolo(x + 24, y + 21, 3, 0x001E2A3C, 0x000F1724);
    ikona_kolo(x + 7, y + 13, 3, 0x00FF8D8D, 0x00C91F1F);
    ikona_kolo(x + 13, y + 9, 3, 0x00FFF38A, 0x00D4A10C);
    ikona_kolo(x + 20, y + 9, 3, 0x008FD0FF, 0x001B5FC1);
    ikona_kolo(x + 26, y + 14, 3, 0x009BEA9B, 0x002C9A3A);
    ikona_kolo(x + 9, y + 20, 3, 0x00D8A4FF, 0x007A2BC2);
}

static void ikona_zadania(int x, int y)
{
    ikona_cien(x + 4, y + 3, 24, 28, 3);
    ikona_rect(x + 4, y + 3, 24, 28, 3, 0x00C98F54, 0x007A4A22);
    ikona_rect(x + 7, y + 7, 18, 22, 1, 0x00FFFFFF, 0x00E1E7F2);
    ikona_rect(x + 11, y + 1, 10, 6, 2, 0x00E5E9F0, 0x007C8798);
    ikona_wypelnij(x + 15, y + 3, 2, 2, 0x004B5565);
    for (int k = 0; k < 3; k++) {
        ikona_rect(x + 9, y + 10 + k * 6, 4, 4, 1, 0x00FFFFFF, 0x00D3DBE8);
        if (k < 2) {
            ikona_linia(x + 9, y + 12 + k * 6, x + 10, y + 13 + k * 6, 1, 0x001FA34A);
            ikona_linia(x + 10, y + 13 + k * 6, x + 13, y + 10 + k * 6, 1, 0x001FA34A);
        }
        ikona_wypelnij(x + 15, y + 11 + k * 6, 8, 2, 0x00A9B6CE);
    }
}

static int duch_wewn(int xx, int yy)
{
    int m;
    int dol;
    if (xx < -11 || xx > 11 || yy < 0)
        return 0;
    if (yy < 11 && xx * xx + (11 - yy) * (11 - yy) > 121)
        return 0;
    m = (xx + 11) % 8;
    dol = 22 + (m < 4 ? m : 8 - m);
    return yy <= dol;
}

static void ikona_duszki(int x, int y)
{
    int cx = x + 16;
    int oy = y + 2;
    ikona_cien_elipsa(cx, y + 30, 10, 2);
    for (int yy = 0; yy < 28; yy++) {
        for (int xx = -11; xx <= 11; xx++) {
            if (!duch_wewn(xx, yy))
                continue;
            uint32_t c = kolor_mieszaj(0x00FFFFFF, 0x00A9B7F2, yy, 26);
            if (!duch_wewn(xx - 1, yy) || !duch_wewn(xx + 1, yy) ||
                !duch_wewn(xx, yy - 1) || !duch_wewn(xx, yy + 1))
                c = 0x005C6BB5;
            rysuj_piksel(cx + xx, oy + yy, c);
        }
    }
    ikona_kolo(x + 12, y + 13, 2, 0x00555B80, 0x00000000);
    ikona_kolo(x + 20, y + 13, 2, 0x00555B80, 0x00000000);
    ikona_wypelnij(x + 11, y + 12, 1, 1, 0x00FFFFFF);
    ikona_wypelnij(x + 19, y + 12, 1, 1, 0x00FFFFFF);
    ikona_kolo(x + 16, y + 18, 2, 0x005A4A8A, 0x002B2F4A);
    ikona_kolo(x + 8, y + 17, 2, 0x00FFB3C8, 0x00FF8FB0);
    ikona_kolo(x + 24, y + 17, 2, 0x00FFB3C8, 0x00FF8FB0);
}

static void ikona_terminal(int x, int y)
{
    ikona_cien(x + 1, y + 1, 30, 30, 3);
    ikona_rect(x + 1, y + 1, 30, 30, 3, 0x00222222, 0x00111111);
    rysuj_tekst_chicago(x + 6, y + 6, ">", 0x0000FF00);
    rysuj_tekst_chicago(x + 14, y + 6, "_", 0x00FFFFFF);
    rysuj_prostokat(x + 4, y + 24, 22, 4, 0x00333333);
}

static void (*const ikony_menu[LICZBA_POZYCJI])(int, int) = {
    ikona_notatnik,
    ikona_konta,
    ikona_wylacz,
    ikona_pliki,
    ikona_www,
    ikona_kalkulator,
    ikona_gra,
    ikona_malowanie,
    ikona_zadania,
    ikona_duszki,
    ikona_terminal
};

static const char *const nazwy_menu[LICZBA_POZYCJI] = {
    "NOTATNIK",
    "KONTA",
    "WYLACZ",
    "PLIKI",
    "WWW",
    "KALKULATOR",
    "GRA",
    "MALOWANIE",
    "ZADANIA",
    "DUSZKI",
    "TERMINAL"
};

void menu_start_rysuj_przycisk(void)
{
    static const uint8_t ikona_menu_start[16][16] = {
        {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0},
        {0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0},
        {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0}
    };
    rozmyj_prostokat(START_X, START_Y, START_SZEROKOSC, START_WYSOKOSC);
    rysuj_prostokat_polprzezroczysty(START_X, START_Y, START_SZEROKOSC, START_WYSOKOSC, KOLOR_PRZYCISKU_START, 70);
    
    for (int iy = 0; iy < 16; iy++) {
        for (int ix = 0; ix < 16; ix++) {
            if (ikona_menu_start[iy][ix]) { 
                rysuj_piksel(START_X + 6 + ix, START_Y + (START_WYSOKOSC / 2) - 8 + iy, KOLOR_ZOLTY);
            }
        }
    }
    
    rysuj_tekst_chicago(START_X + 26, START_Y + (START_WYSOKOSC / 2) - 4, "start", KOLOR_TEKST_BIALY);
}

void menu_start(void)
{
    menu_otwarte = 0;
    menu_start_rysuj_przycisk();
    odswiez_widok();
}

static void menu_start_narysuj(void)
{
    rozmyj_prostokat(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC);
    rysuj_prostokat_polprzezroczysty(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC, KOLOR_RAMKA_MENU, 80);
    rysuj_prostokat_polprzezroczysty(MENU_X + 2, MENU_Y + 2, MENU_SZEROKOSC - 4, MENU_WYSOKOSC - 4, KOLOR_TLO_MENU, 75);

    for (int i = 0; i < LICZBA_POZYCJI; i++) {
        int gora = MENU_Y + MENU_MARGINES + i * POZYCJA_ODSTEP;
        rysuj_prostokat_polprzezroczysty(MENU_X + 6, gora, MENU_SZEROKOSC - 12, POZYCJA_WYSOKOSC, KOLOR_ELEMENT_MENU, 85);
        ikony_menu[i](MENU_X + 10, gora + 3);
        rysuj_tekst_chicago(MENU_X + 50, gora + (POZYCJA_WYSOKOSC / 2) - 4, nazwy_menu[i], KOLOR_TEKST_BIALY);
    }

    odswiez_widok();
}

static void menu_start_wyczysc(void)
{
    odtworz_fragment_tla(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC);
    
    if (aktywne_okno && aktywne_okno->otwarte) {
        int ox = aktywne_okno->x;
        int oy = aktywne_okno->y;
        int ow = aktywne_okno->szerokosc;
        int oh = aktywne_okno->wysokosc;

        if (MENU_X < ox + ow && MENU_X + MENU_SZEROKOSC > ox &&
            MENU_Y < oy + oh && MENU_Y + MENU_WYSOKOSC > oy) {
            okno_narysuj(aktywne_okno);
        }
    }
    
    odswiez_widok();
}

void menu_start_przelacz(void)
{
    if (menu_otwarte) {
        menu_otwarte = 0;
        menu_start_wyczysc();
    } else {
        menu_otwarte = 1;
        menu_start_narysuj();
    }
}

void menu_start_mouse_click(void)
{
    int x;
    int y;
    int wzgledne;
    int indeks;

    mouse_pozycja(&x, &y);

    if (!menu_otwarte) {
        if (x >= START_X && x < START_X + START_SZEROKOSC &&
            y >= START_Y && y < START_Y + START_WYSOKOSC) {
            menu_otwarte = 1;
            menu_start_narysuj();
        }
        return;
    }

    if (x < MENU_X || x >= MENU_X + MENU_SZEROKOSC ||
        y < MENU_Y || y >= MENU_Y + MENU_WYSOKOSC) {
        menu_otwarte = 0;
        menu_start_wyczysc();
        return;
    }

    if (x < MENU_X + 6 || x >= MENU_X + MENU_SZEROKOSC - 6)
        return;

    wzgledne = y - MENU_Y - MENU_MARGINES;
    if (wzgledne < 0)
        return;

    indeks = wzgledne / POZYCJA_ODSTEP;
    if (indeks >= LICZBA_POZYCJI || wzgledne % POZYCJA_ODSTEP >= POZYCJA_WYSOKOSC)
        return;

    if (indeks == 2) {
        shutdown();
        return;
    }

    menu_otwarte = 0;
    menu_start_wyczysc();

    switch (indeks) {
    case 0:
        notatnik_otworz();
        break;
    case 1:
        konta_zarzadzaj();
        break;
    case 3:
        eksplorator_plikow();
        break;
    case 4:
        browser_open();
        break;
    case 5:
        kalkulator_otworz();
        break;
    case 6:
        gra();
        break;
    case 7:
        malowanie_otworz();
        break;
    case 8:
        zadania_otworz();
        break;
    case 9:
        duszki();
        break;
    case 10:
        terminal_otworz();
        break;
    }
}