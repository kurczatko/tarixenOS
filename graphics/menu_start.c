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

/* --- uklad gornego paska i docka --- */
#ifndef PASEK_GORNY_WYSOKOSC
#define PASEK_GORNY_WYSOKOSC 24
#endif
#define DOCK_WYSOKOSC_PX    60
#define DOCK_MARGINES_DOLNY 10
#define DOCK_MAG_GORA       28    /* miejsce nad dockiem na powiekszone ikony */
/* gorna krawedz pasa, ktory przerysowujemy razem z dockiem */
#define DOCK_PAS_Y          (EKRAN_WYSOKOSC - DOCK_MARGINES_DOLNY - DOCK_WYSOKOSC_PX - DOCK_MAG_GORA)
/* gorna krawedz samego szkla docka */
#define DOCK_SZKLO_Y        (DOCK_PAS_Y + DOCK_MAG_GORA)
#define OKNA_DOL_MAKS       (DOCK_SZKLO_Y - 8)
void rysuj_szklo(int x, int y, int w, int h, int promien,
                 uint32_t tint, uint32_t alfa, int z_obrazu);
int menu_start_czy_w_docku(int x, int y);
void menu_start_rysuj_dock(void);
void menu_start_dock_hover(int x, int y);
void okno_odrysuj_dolny_pas(void);
extern uint32_t *obraz;

/* ---------------- menu w lewym gornym rogu ---------------- */
#define PRZYCISK_SZEROKOSC 112   /* logo + napis "TarixenOS" */

#define MENU_X 4
#define MENU_Y (PASEK_GORNY_WYSOKOSC + 2)
#define MENU_SZEROKOSC 170
#define MENU_MARGINES 6
#define POZYCJA_WYSOKOSC 38
#define MENU_WYSOKOSC (2 * MENU_MARGINES + POZYCJA_WYSOKOSC)   /* jedna pozycja: WYLACZ */

#define KOLOR_TLO_MENU     0x001E293B
#define KOLOR_ELEMENT_MENU 0x00334155
#define KOLOR_TEKST_BIALY  0x00FFFFFF

/* ---------------- dock ---------------- */
#define DOCK_IKONA   32
#define DOCK_SLOT    44
#define DOCK_PAD     8
#define DOCK_WYS     DOCK_WYSOKOSC_PX
#define DOCK_DOL     DOCK_MARGINES_DOLNY
#define DOCK_PROMIEN 16
#define DOCK_APEK    10
#define DOCK_ROZDZ   16
#define DOCK_Y       DOCK_SZKLO_Y
#define DOCK_IKONA_Y (DOCK_Y + 9)

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

/* ====================================================================
 * DOCK: wszystkie aplikacje, ktore byly w menu start (bez WYLACZ).
 * Po prawej, za separatorem, sa zminimalizowane okna.
 * ==================================================================== */
static void (*const ikony_dock[DOCK_APEK])(int, int) = {
    ikona_notatnik,
    ikona_konta,
    ikona_pliki,
    ikona_www,
    ikona_kalkulator,
    ikona_gra,
    ikona_malowanie,
    ikona_zadania,
    ikona_duszki,
    ikona_terminal
};

static const char *const nazwy_dock[DOCK_APEK] = {
    "NOTATNIK", "KONTA", "PLIKI", "WWW", "KALKULATOR",
    "GRA", "MALOWANIE", "ZADANIA", "DUSZKI", "TERMINAL"
};

static char male_na_duze(char c)
{
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

static int tekst_rowny_ci(const char *a, const char *b)
{
    while (*a && *b) {
        if (male_na_duze(*a++) != male_na_duze(*b++)) return 0;
    }
    return *a == *b;
}

/* ktory przycisk docka odpowiada tytulowi okna (-1 = nie wiadomo) */
static int dock_indeks_dla_tytulu(const char *tytul)
{
    if (!tytul) return -1;
    for (int i = 0; i < DOCK_APEK; i++)
        if (tekst_rowny_ci(tytul, nazwy_dock[i])) return i;
    if (tekst_rowny_ci(tytul, "PRZEGLADARKA HTTP/HTTPS")) return 3;
    return -1;
}

static int dock_licz_min(void)
{
    int n = 0;
    for (int i = 0; i < okno_liczba_okien(); i++) {
        struct okno *o = okno_pobierz_okno(i);
        if (o && o->otwarte && o->zminimalizowane) n++;
    }
    return n > 16 ? 16 : n;
}

static struct okno *dock_okno_min(int j)
{
    for (int i = 0; i < okno_liczba_okien(); i++) {
        struct okno *o = okno_pobierz_okno(i);
        if (!o || !o->otwarte || !o->zminimalizowane) continue;
        if (j-- == 0) return o;
    }
    return 0;
}

/* ---- uklad docka z powiekszaniem ---- */
#define DOCK_MAG_PROMIEN   100     /* zasieg powiekszenia wokol kursora (px) */
#define DOCK_MAG_MAX       1741    /* maksymalna skala: 1.7x (1024 = 1.0) */
#define DOCK_MAG_PODEJSCIE 36      /* ile px nad dockiem zaczyna sie powiekszanie */
#define DOCK_MAX_POZ       (DOCK_APEK + 16)
#define DOCK_BLOK          44      /* bufor ikony: 32 px + zapas na cien */
#define DOCK_BLOK_X        6
#define DOCK_BLOK_Y        4
#define DOCK_HOVER_MARGINES 6      /* zapas po bokach odswiezanego obszaru (cienie, krawedz szkla) */

struct dock_poz {
    int x;      /* lewa krawedz slotu */
    int w;      /* szerokosc slotu */
    int s;      /* skala ikony (1024 = 1.0) */
};

static struct dock_poz dock_p[DOCK_MAX_POZ];
static int dock_mx = -1000;
static int dock_my = -1000;
static int dock_hover_aktywny;

/* ikony z kanalem alfa (premultiplied ARGB), potrzebne do skalowania */
static uint32_t dock_cache[DOCK_APEK + 1][DOCK_BLOK * DOCK_BLOK];
static int dock_cache_proba;
static int dock_cache_ok;

static void ikona_generyczna(int x, int y)
{
    ikona_cien(x + 1, y + 1, 30, 30, 7);
    ikona_rect(x + 1, y + 1, 30, 30, 7, 0x006FB1FF, 0x001E4FA8);
}

static void dock_ikona_bezposrednio(int idx, int x, int y)
{
    if (idx >= 0 && idx < DOCK_APEK) ikony_dock[idx](x, y);
    else ikona_generyczna(x, y);
}

/* Kazda ikone rysujemy raz na czarnym i raz na bialym tle i z roznicy
 * wyliczamy przezroczystosc. Gdy rysowanie nie trafia do bufora obraz
 * (tryb ze sterownikiem hal_gfx), cache jest wylaczony i dock dziala bez
 * powiekszania. */
static void dock_zbuduj_cache(void)
{
    static uint32_t zapas[DOCK_BLOK * DOCK_BLOK];
    static uint32_t czarne[DOCK_BLOK * DOCK_BLOK];
    static uint32_t biale[DOCK_BLOK * DOCK_BLOK];
    int bx = 4;
    int by = DOCK_PAS_Y + 20;
    int ok = 1;

    dock_cache_proba = 1;
    for (int j = 0; j < DOCK_BLOK; j++)
        for (int i = 0; i < DOCK_BLOK; i++)
            zapas[j * DOCK_BLOK + i] = obraz[(by + j) * EKRAN_SZEROKOSC + bx + i];

    for (int idx = 0; idx <= DOCK_APEK && ok; idx++) {
        for (int przebieg = 0; przebieg < 2 && ok; przebieg++) {
            uint32_t tlo = przebieg ? 0x00FFFFFF : 0x00000000;
            uint32_t *cel = przebieg ? biale : czarne;
            rysuj_prostokat(bx, by, DOCK_BLOK, DOCK_BLOK, tlo);
            if ((obraz[by * EKRAN_SZEROKOSC + bx] & 0x00FFFFFF) != tlo) {
                ok = 0;
                break;
            }
            dock_ikona_bezposrednio(idx, bx + DOCK_BLOK_X, by + DOCK_BLOK_Y);
            for (int j = 0; j < DOCK_BLOK; j++)
                for (int i = 0; i < DOCK_BLOK; i++)
                    cel[j * DOCK_BLOK + i] = obraz[(by + j) * EKRAN_SZEROKOSC + bx + i] & 0x00FFFFFF;
        }
        if (!ok) break;
        for (int k = 0; k < DOCK_BLOK * DOCK_BLOK; k++) {
            int cb = (int)czarne[k];
            int cw = (int)biale[k];
            int d = (((cw >> 16) & 255) - ((cb >> 16) & 255) +
                     ((cw >> 8) & 255) - ((cb >> 8) & 255) +
                     (cw & 255) - (cb & 255)) / 3;
            int al = 255 - d;
            int pr = (cb >> 16) & 255;
            int pg = (cb >> 8) & 255;
            int pb = cb & 255;
            if (al < 0) al = 0;
            if (al > 255) al = 255;
            if (pr > al) pr = al;
            if (pg > al) pg = al;
            if (pb > al) pb = al;
            dock_cache[idx][k] = ((uint32_t)al << 24) | ((uint32_t)pr << 16) |
                                 ((uint32_t)pg << 8) | (uint32_t)pb;
        }
    }

    for (int j = 0; j < DOCK_BLOK; j++)
        for (int i = 0; i < DOCK_BLOK; i++)
            obraz[(by + j) * EKRAN_SZEROKOSC + bx + i] = zapas[j * DOCK_BLOK + i];
    dock_cache_ok = ok;
}

/* ikona skalowana dwuliniowo z cache, narysowana na buforze obraz */
static void dock_blit(int idx, int ikona_x, int ikona_y, int s)
{
    const uint32_t *src = dock_cache[idx];
    int bw = DOCK_BLOK * s / 1024;
    int bl = ikona_x - DOCK_BLOK_X * s / 1024;
    int bt = ikona_y - DOCK_BLOK_Y * s / 1024;
    if (bw < 1 || s < 1) return;

    for (int dy = 0; dy < bw; dy++) {
        int py = bt + dy;
        if (py < 0 || py >= EKRAN_WYSOKOSC) continue;
        int sv = ((2 * dy + 1) * 131072) / s - 128;
        int v0 = sv >> 8;
        int fv = sv & 255;
        int v1 = v0 + 1;
        if (v0 < 0) v0 = 0;
        if (v0 > DOCK_BLOK - 1) v0 = DOCK_BLOK - 1;
        if (v1 < 0) v1 = 0;
        if (v1 > DOCK_BLOK - 1) v1 = DOCK_BLOK - 1;
        for (int dx = 0; dx < bw; dx++) {
            int px = bl + dx;
            if (px < 0 || px >= EKRAN_SZEROKOSC) continue;
            int su = ((2 * dx + 1) * 131072) / s - 128;
            int u0 = su >> 8;
            int fu = su & 255;
            int u1 = u0 + 1;
            if (u0 < 0) u0 = 0;
            if (u0 > DOCK_BLOK - 1) u0 = DOCK_BLOK - 1;
            if (u1 < 0) u1 = 0;
            if (u1 > DOCK_BLOK - 1) u1 = DOCK_BLOK - 1;

            uint32_t p00 = src[v0 * DOCK_BLOK + u0];
            uint32_t p10 = src[v0 * DOCK_BLOK + u1];
            uint32_t p01 = src[v1 * DOCK_BLOK + u0];
            uint32_t p11 = src[v1 * DOCK_BLOK + u1];
            uint32_t w00 = (uint32_t)((256 - fu) * (256 - fv));
            uint32_t w10 = (uint32_t)(fu * (256 - fv));
            uint32_t w01 = (uint32_t)((256 - fu) * fv);
            uint32_t w11 = (uint32_t)(fu * fv);
            uint32_t al = ((p00 >> 24) * w00 + (p10 >> 24) * w10 +
                           (p01 >> 24) * w01 + (p11 >> 24) * w11) >> 16;
            if (al == 0) continue;
            uint32_t r = (((p00 >> 16) & 255) * w00 + ((p10 >> 16) & 255) * w10 +
                          ((p01 >> 16) & 255) * w01 + ((p11 >> 16) & 255) * w11) >> 16;
            uint32_t g = (((p00 >> 8) & 255) * w00 + ((p10 >> 8) & 255) * w10 +
                          ((p01 >> 8) & 255) * w01 + ((p11 >> 8) & 255) * w11) >> 16;
            uint32_t b = ((p00 & 255) * w00 + (p10 & 255) * w10 +
                          (p01 & 255) * w01 + (p11 & 255) * w11) >> 16;
            uint32_t d = obraz[(uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)px];
            r += ((d >> 16) & 255) * (255 - al) / 255;
            g += ((d >> 8) & 255) * (255 - al) / 255;
            b += (d & 255) * (255 - al) / 255;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            obraz[(uint32_t)py * EKRAN_SZEROKOSC + (uint32_t)px] = (r << 16) | (g << 8) | b;
        }
    }
}

/* waga powiekszenia zalezna od wysokosci kursora (0..1024) */
static int dock_waga(int y)
{
    int gora = DOCK_Y - DOCK_MAG_PODEJSCIE;
    if (y < gora) return 0;
    if (y >= DOCK_Y) return 1024;
    return (y - gora) * 1024 / DOCK_MAG_PODEJSCIE;
}

static int dock_gladko(int t)
{
    if (t < 0) t = 0;
    if (t > 1024) t = 1024;
    return t * t / 1024 * (3 * 1024 - 2 * t) / 1024;
}

/* Liczy pozycje wszystkich ikon dla kursora w (hx, hy). Dock jest wysrodkowany;
 * po powiekszeniu ikon robi sie szerszy na obie strony. */
static void dock_uloz(int hx, int hy, int n, int *lewy, int *szer)
{
    int a = dock_cache_ok ? dock_waga(hy) : 0;
    int liczba;
    int suma = 0;
    int bsz;
    int bl;
    int x;
    if (n > 16) n = 16;
    liczba = DOCK_APEK + n;
    bsz = 2 * DOCK_PAD + DOCK_APEK * DOCK_SLOT + (n > 0 ? DOCK_ROZDZ + n * DOCK_SLOT : 0);
    bl = (EKRAN_SZEROKOSC - bsz) / 2;

    for (int i = 0; i < liczba; i++) {
        int cb = bl + DOCK_PAD + i * DOCK_SLOT + DOCK_SLOT / 2 + (i >= DOCK_APEK ? DOCK_ROZDZ : 0);
        int s = 1024;
        if (a > 0) {
            int d = hx > cb ? hx - cb : cb - hx;
            if (d < DOCK_MAG_PROMIEN) {
                int f = 1024 - dock_gladko(d * 1024 / DOCK_MAG_PROMIEN);
                s = 1024 + (DOCK_MAG_MAX - 1024) * f / 1024 * a / 1024;
            }
        }
        dock_p[i].s = s;
        dock_p[i].w = DOCK_SLOT * s / 1024;
        suma += dock_p[i].w;
    }
    *szer = 2 * DOCK_PAD + suma + (n > 0 ? DOCK_ROZDZ : 0);
    *lewy = (EKRAN_SZEROKOSC - *szer) / 2;
    x = *lewy + DOCK_PAD;
    for (int i = 0; i < liczba; i++) {
        if (i == DOCK_APEK) x += DOCK_ROZDZ;
        dock_p[i].x = x;
        x += dock_p[i].w;
    }
}

static int dock_czy_uruchomiona(int indeks)
{
    for (int i = 0; i < okno_liczba_okien(); i++) {
        struct okno *o = okno_pobierz_okno(i);
        if (o && o->otwarte && dock_indeks_dla_tytulu(o->tytul) == indeks) return 1;
    }
    return 0;
}

/* gorna krawedz ikony o danej skali (dolna jest zawsze w tym samym miejscu) */
static int dock_ikona_gora(const struct dock_poz *p)
{
    return DOCK_Y + 9 + DOCK_IKONA - DOCK_IKONA * p->s / 1024;
}

/* ktora pozycja docka jest pod punktem (x, y); kursor = (x, y) */
static int dock_trafienie(int x, int y, int *indeks)
{
    int lewy, szer;
    int n = dock_licz_min();
    if (n > 16) n = 16;
    dock_uloz(x, y, n, &lewy, &szer);
    for (int i = 0; i < DOCK_APEK + n; i++) {
        int gora = dock_ikona_gora(&dock_p[i]);
        if (gora > DOCK_Y) gora = DOCK_Y;
        if (x >= dock_p[i].x && x < dock_p[i].x + dock_p[i].w &&
            y >= gora && y < DOCK_Y + DOCK_WYS) {
            *indeks = i;
            return 1;
        }
    }
    return 0;
}

static void dock_rysuj_pozycje(int cache_idx, const char *tytul, const struct dock_poz *p)
{
    int d = DOCK_IKONA * p->s / 1024;
    int lewo = p->x + (p->w - d) / 2;
    int gora = dock_ikona_gora(p);
    if (dock_cache_ok) dock_blit(cache_idx, lewo, gora, p->s);
    else dock_ikona_bezposrednio(cache_idx, lewo, gora);
    if (cache_idx >= DOCK_APEK) {     /* okno bez rozpoznanej aplikacji: pierwsza litera */
        char lit[2];
        lit[0] = (tytul && tytul[0]) ? male_na_duze(tytul[0]) : '?';
        lit[1] = 0;
        rysuj_tekst_chicago(lewo + d / 2 - 4, gora + d / 2 - 4, lit, 0x00FFFFFF);
    }
}

void pasek_zadan_rysuj_ikone(int x, int y, const char *tytul)
{
    int idx = dock_indeks_dla_tytulu(tytul);
    dock_ikona_bezposrednio(idx, x, y);
}

struct okno *pasek_zadan_ikona_na_pozycji(int x, int y)
{
    int idx;
    if (!dock_trafienie(x, y, &idx) || idx < DOCK_APEK) return 0;
    return dock_okno_min(idx - DOCK_APEK);
}

int menu_start_czy_w_docku(int x, int y)
{
    int idx;
    int lewy, szer;
    int n = dock_licz_min();
    if (n > 16) n = 16;
    if (dock_trafienie(x, y, &idx)) return 1;
    dock_uloz(x, y, n, &lewy, &szer);
    return x >= lewy && x < lewy + szer && y >= DOCK_Y && y < DOCK_Y + DOCK_WYS;
}

void menu_start_rysuj_dock(void)
{
    int lewy, szer;
    int n = dock_licz_min();
    if (n > 16) n = 16;
    if (!dock_cache_proba) dock_zbuduj_cache();
    dock_uloz(dock_mx, dock_my, n, &lewy, &szer);

    /* rozmyte, zaokraglone szklo - rozmywa to, co jest pod dockiem (tapeta i okna) */
    rysuj_szklo(lewy, DOCK_Y, szer, DOCK_WYS, DOCK_PROMIEN, 0x00FFFFFF, 64, 1);

    for (int i = 0; i < DOCK_APEK; i++) {
        dock_rysuj_pozycje(i, 0, &dock_p[i]);
        if (dock_czy_uruchomiona(i)) {   /* biala kropka pod uruchomiona aplikacja */
            int kx = dock_p[i].x + dock_p[i].w / 2 - 2;
            int ky = DOCK_Y + DOCK_WYS - 9;
            rysuj_prostokat(kx + 1, ky, 2, 1, 0x00FFFFFF);
            rysuj_prostokat(kx, ky + 1, 4, 2, 0x00FFFFFF);
            rysuj_prostokat(kx + 1, ky + 3, 2, 1, 0x00FFFFFF);
        }
    }

    if (n > 0) {
        int sx = dock_p[DOCK_APEK - 1].x + dock_p[DOCK_APEK - 1].w + DOCK_ROZDZ / 2;
        rysuj_prostokat_polprzezroczysty(sx, DOCK_Y + 10, 1, DOCK_WYS - 20, 0x00FFFFFF, 140);
        rysuj_prostokat_polprzezroczysty(sx + 1, DOCK_Y + 10, 1, DOCK_WYS - 20, 0x00000000, 40);
        for (int j = 0; j < n; j++) {
            struct okno *o = dock_okno_min(j);
            if (!o) continue;
            int idx = dock_indeks_dla_tytulu(o->tytul);
            dock_rysuj_pozycje(idx >= 0 ? idx : DOCK_APEK, o->tytul, &dock_p[DOCK_APEK + j]);
        }
    }
}

/* Wywolywane przy kazdym ruchu kursora: przerysowuje dock z powiekszeniem.
 *
 * Cala klatka jest skladana w buforze obraz (czysta scena + dock), a na ekran
 * trafia dopiero gotowy wynik, jednym odswiezeniem. Wczesniej tapeta byla
 * wysylana na ekran jeszcze przed narysowaniem docka (odtworz_fragment_tla),
 * przez co dock znikal na ulamek sekundy przy kazdym ruchu myszy. */
void menu_start_dock_hover(int x, int y)
{
    int aktywny = 0;
    int stary_x = dock_mx;
    int stary_y = dock_my;
    if (!dock_cache_ok) {
        dock_mx = x;
        dock_my = y;
        return;
    }
    if (dock_waga(y) > 0) {
        int n = dock_licz_min();
        int bsz;
        int bl;
        if (n > 16) n = 16;
        bsz = 2 * DOCK_PAD + DOCK_APEK * DOCK_SLOT + (n > 0 ? DOCK_ROZDZ + n * DOCK_SLOT : 0);
        bl = (EKRAN_SZEROKOSC - bsz) / 2;
        if (x >= bl - DOCK_MAG_PROMIEN && x < bl + bsz + DOCK_MAG_PROMIEN) aktywny = 1;
    }
    if (!aktywny && !dock_hover_aktywny) {
        dock_mx = x;
        dock_my = y;
        return;
    }
    if (aktywny && x == dock_mx && y == dock_my) return;
    dock_mx = x;
    dock_my = y;
    dock_hover_aktywny = aktywny;

    /* Odswiezamy tylko obszar starego i nowego polozenia docka. */
    int n = dock_licz_min();
    int stary_lewy, stara_szer, nowy_lewy, nowa_szer;
    int lewy, prawy;
    if (n > 16) n = 16;
    dock_uloz(stary_x, stary_y, n, &stary_lewy, &stara_szer);
    dock_uloz(x, y, n, &nowy_lewy, &nowa_szer);

    lewy = stary_lewy < nowy_lewy ? stary_lewy : nowy_lewy;
    prawy = stary_lewy + stara_szer > nowy_lewy + nowa_szer ?
             stary_lewy + stara_szer : nowy_lewy + nowa_szer;
    lewy -= DOCK_HOVER_MARGINES;
    prawy += DOCK_HOVER_MARGINES;
    if (lewy < 0) lewy = 0;
    if (prawy > EKRAN_SZEROKOSC) prawy = EKRAN_SZEROKOSC;
    if (prawy <= lewy) return;

    /* 1. czysta scena (tapeta + okna) w buforze - nic nie idzie na ekran */
    okno_odrysuj_dolny_pas();
    /* 2. dock na swiezej scenie, tez tylko w buforze */
    menu_start_rysuj_dock();
    /* 3. jedno odswiezenie gotowej klatki */
    odswiez_fragment_widoku(lewy, DOCK_PAS_Y, prawy - lewy,
                            EKRAN_WYSOKOSC - DOCK_PAS_Y);
}

/* ====================================================================
 * Lewy gorny rog: logo + nazwa aktywnego okna
 * ==================================================================== */
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

    /* podswietlenie, gdy menu jest otwarte */
    if (menu_otwarte)
        rysuj_prostokat_polprzezroczysty(0, 0, PRZYCISK_SZEROKOSC, PASEK_GORNY_WYSOKOSC - 1,
                                         0x00FFFFFF, 55);

    for (int iy = 0; iy < 16; iy++) {
        for (int ix = 0; ix < 16; ix++) {
            if (ikona_menu_start[iy][ix])
                rysuj_piksel(8 + ix, (PASEK_GORNY_WYSOKOSC - 16) / 2 + iy, KOLOR_ZOLTY);
        }
    }
    rysuj_tekst_chicago(30, PASEK_GORNY_WYSOKOSC / 2 - 4, "TarixenOS", KOLOR_TEKST_BIALY);

    /* nazwa aktywnego okna, jak nazwa aplikacji w macOS */
    if (aktywne_okno && aktywne_okno->otwarte && !aktywne_okno->zminimalizowane && aktywne_okno->tytul)
        rysuj_tekst_chicago(PRZYCISK_SZEROKOSC + 12, PASEK_GORNY_WYSOKOSC / 2 - 4,
                            aktywne_okno->tytul, KOLOR_TEKST_BIALY);
}

void menu_start(void)
{
    menu_otwarte = 0;
    narysuj_pasek_zadan();
    odswiez_widok();
}

/* ====================================================================
 * Menu rozwijane (tylko WYLACZ)
 * ==================================================================== */
static void polprzezroczysty_zaokr(int x, int y, int w, int h, int r, uint32_t kolor, uint32_t alfa)
{
    for (int i = 0; i < h; i++) {
        int in = ikona_wciecie(i, h, r);
        rysuj_prostokat_polprzezroczysty(x + in, y + i, w - 2 * in, 1, kolor, alfa);
    }
}

static void menu_start_narysuj(void)
{
    rysuj_szklo(MENU_X, MENU_Y, MENU_SZEROKOSC, MENU_WYSOKOSC, 10, KOLOR_TLO_MENU, 150, 1);

    int gora = MENU_Y + MENU_MARGINES;
    polprzezroczysty_zaokr(MENU_X + 6, gora, MENU_SZEROKOSC - 12, POZYCJA_WYSOKOSC, 6,
                           KOLOR_ELEMENT_MENU, 120);
    ikona_wylacz(MENU_X + 10, gora + 3);
    rysuj_tekst_chicago(MENU_X + 50, gora + (POZYCJA_WYSOKOSC / 2) - 4, "WYLACZ", KOLOR_TEKST_BIALY);

    odswiez_widok();
}

static void menu_start_wyczysc(void)
{
    odtworz_fragment_tla(MENU_X - 2, MENU_Y - 2, MENU_SZEROKOSC + 4, MENU_WYSOKOSC + 4);
    okno_odrysuj_wszystkie();
}

void menu_start_przelacz(void)
{
    if (menu_otwarte) {
        menu_otwarte = 0;
        menu_start_wyczysc();
    } else {
        menu_otwarte = 1;
        narysuj_pasek_zadan();    /* podswietlenie przycisku */
        menu_start_narysuj();
    }
}

static void dock_uruchom(int indeks)
{
    switch (indeks) {
    case 0: notatnik_otworz(); break;
    case 1: konta_zarzadzaj(); break;
    case 2: eksplorator_plikow(); break;
    case 3: browser_open(); break;
    case 4: kalkulator_otworz(); break;
    case 5: gra(); break;
    case 6: malowanie_otworz(); break;
    case 7: zadania_otworz(); break;
    case 8: duszki(); break;
    case 9: terminal_otworz(); break;
    }
}

/* Klik w ikone aplikacji w docku:
 *  - brak otwartego okna          -> uruchom aplikacje
 *  - okno jest, ale bez fokusu    -> ustaw na nim fokus (na wierzch)
 *  - okno ma fokus                -> zminimalizuj je
 *  - okno jest tylko zminimalizowane -> przywroc je */
static void dock_kliknij_aplikacje(int indeks)
{
    struct okno *cel = 0;        /* najwyzsze widoczne okno tej aplikacji */
    struct okno *zmin = 0;       /* zminimalizowane okno tej aplikacji */

    for (int i = okno_liczba_okien() - 1; i >= 0; i--) {
        struct okno *o = okno_pobierz_okno(i);
        if (!o || !o->otwarte || dock_indeks_dla_tytulu(o->tytul) != indeks) continue;
        if (o->zminimalizowane) {
            if (!zmin) zmin = o;
        } else if (!cel) {
            cel = o;
        }
    }

    if (cel) {
        struct okno *aktywne_tej_aplikacji = 0;
        if (aktywne_okno && aktywne_okno->otwarte && !aktywne_okno->zminimalizowane &&
            dock_indeks_dla_tytulu(aktywne_okno->tytul) == indeks)
            aktywne_tej_aplikacji = aktywne_okno;

        if (aktywne_tej_aplikacji) {
            okno_zminimalizuj(aktywne_tej_aplikacji);
        } else {
            okno_ustaw_aktywne(cel);
            okno_odrysuj_wszystkie();
        }
        return;
    }
    if (zmin) {
        okno_przywroc(zmin);
        return;
    }
    dock_uruchom(indeks);
}

void menu_start_mouse_click(void)
{
    int x;
    int y;

    mouse_pozycja(&x, &y);

    int na_przycisku = (x >= 0 && x < PRZYCISK_SZEROKOSC && y >= 0 && y < PASEK_GORNY_WYSOKOSC);

    if (menu_otwarte) {
        if (x >= MENU_X && x < MENU_X + MENU_SZEROKOSC &&
            y >= MENU_Y && y < MENU_Y + MENU_WYSOKOSC) {
            int gora = MENU_Y + MENU_MARGINES;
            if (x >= MENU_X + 6 && x < MENU_X + MENU_SZEROKOSC - 6 &&
                y >= gora && y < gora + POZYCJA_WYSOKOSC) {
                shutdown();
            }
            return;
        }
        menu_otwarte = 0;
        menu_start_wyczysc();
        if (na_przycisku) return;     /* klik w przycisk tylko zamyka menu */
    }

    if (na_przycisku) {
        menu_otwarte = 1;
        narysuj_pasek_zadan();
        menu_start_narysuj();
        return;
    }

    /* dock */
    int idx;
    if (!dock_trafienie(x, y, &idx)) return;
    if (idx < DOCK_APEK) {
        dock_kliknij_aplikacje(idx);
        return;
    }
    struct okno *min = dock_okno_min(idx - DOCK_APEK);
    if (min) okno_przywroc(min);
}