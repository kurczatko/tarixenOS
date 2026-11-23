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
#define DOCK_PAS_Y          (EKRAN_WYSOKOSC - DOCK_MARGINES_DOLNY - DOCK_WYSOKOSC_PX)
#define OKNA_DOL_MAKS       (DOCK_PAS_Y - 6)
void rysuj_szklo(int x, int y, int w, int h, int promien,
                 uint32_t tint, uint32_t alfa, int z_obrazu);
int menu_start_czy_w_docku(int x, int y);
void menu_start_rysuj_dock(void);

/* ---------------- menu w lewym gornym rogu (jak jablko w macOS) ---------------- */
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
#define DOCK_Y       DOCK_PAS_Y
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
    return n;
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

static void dock_geom(int *x, int *w, int *nmin)
{
    int n = dock_licz_min();
    int szer = 2 * DOCK_PAD + DOCK_APEK * DOCK_SLOT;
    if (n > 0) szer += DOCK_ROZDZ + n * DOCK_SLOT;
    if (szer > EKRAN_SZEROKOSC) szer = EKRAN_SZEROKOSC;
    *x = (EKRAN_SZEROKOSC - szer) / 2;
    *w = szer;
    *nmin = n;
}

static int dock_czy_uruchomiona(int indeks)
{
    for (int i = 0; i < okno_liczba_okien(); i++) {
        struct okno *o = okno_pobierz_okno(i);
        if (o && o->otwarte && dock_indeks_dla_tytulu(o->tytul) == indeks) return 1;
    }
    return 0;
}

/* ikona zminimalizowanego okna w docku */
static void dock_rysuj_min(int x, int y, const char *tytul)
{
    int idx = dock_indeks_dla_tytulu(tytul);
    if (idx >= 0) {
        ikony_dock[idx](x, y);
    } else {
        char lit[2];
        lit[0] = (tytul && tytul[0]) ? male_na_duze(tytul[0]) : '?';
        lit[1] = 0;
        ikona_cien(x + 1, y + 1, 30, 30, 7);
        ikona_rect(x + 1, y + 1, 30, 30, 7, 0x006FB1FF, 0x001E4FA8);
        rysuj_tekst_chicago(x + 12, y + 12, lit, 0x00FFFFFF);
    }
}

void pasek_zadan_rysuj_ikone(int x, int y, const char *tytul)
{
    dock_rysuj_min(x, y, tytul);
}

struct okno *pasek_zadan_ikona_na_pozycji(int x, int y)
{
    int dx, dw, n;
    dock_geom(&dx, &dw, &n);
    if (n == 0 || y < DOCK_Y || y >= DOCK_Y + DOCK_WYS) return 0;
    int rel = x - (dx + DOCK_PAD + DOCK_APEK * DOCK_SLOT + DOCK_ROZDZ);
    if (rel < 0) return 0;
    int j = rel / DOCK_SLOT;
    if (j >= n) return 0;
    return dock_okno_min(j);
}

int menu_start_czy_w_docku(int x, int y)
{
    int dx, dw, n;
    dock_geom(&dx, &dw, &n);
    return x >= dx && x < dx + dw && y >= DOCK_Y && y < DOCK_Y + DOCK_WYS;
}

void menu_start_rysuj_dock(void)
{
    int dx, dw, n;
    dock_geom(&dx, &dw, &n);

    /* rozmyte, zaokraglone szklo - rozmywa to, co jest pod dockiem (tapeta i okna) */
    rysuj_szklo(dx, DOCK_Y, dw, DOCK_WYS, DOCK_PROMIEN, 0x00FFFFFF, 64, 1);

    for (int i = 0; i < DOCK_APEK; i++) {
        int ix = dx + DOCK_PAD + i * DOCK_SLOT + (DOCK_SLOT - DOCK_IKONA) / 2;
        ikony_dock[i](ix, DOCK_IKONA_Y);
        if (dock_czy_uruchomiona(i))   /* kropka pod uruchomiona aplikacja */
            ikona_kolo(ix + DOCK_IKONA / 2, DOCK_Y + DOCK_WYS - 7, 2, 0x00606878, 0x00101820);
    }

    if (n > 0) {
        int sx = dx + DOCK_PAD + DOCK_APEK * DOCK_SLOT + DOCK_ROZDZ / 2;
        rysuj_prostokat_polprzezroczysty(sx, DOCK_Y + 10, 1, DOCK_WYS - 20, 0x00FFFFFF, 140);
        rysuj_prostokat_polprzezroczysty(sx + 1, DOCK_Y + 10, 1, DOCK_WYS - 20, 0x00000000, 40);
        for (int j = 0; j < n; j++) {
            struct okno *o = dock_okno_min(j);
            int ix = dx + DOCK_PAD + DOCK_APEK * DOCK_SLOT + DOCK_ROZDZ + j * DOCK_SLOT +
                     (DOCK_SLOT - DOCK_IKONA) / 2;
            if (o) dock_rysuj_min(ix, DOCK_IKONA_Y, o->tytul);
        }
    }
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
    int dx, dw, n;
    dock_geom(&dx, &dw, &n);
    if (y < DOCK_Y || y >= DOCK_Y + DOCK_WYS || x < dx || x >= dx + dw) return;

    int rel = x - (dx + DOCK_PAD);
    if (rel < 0) return;
    int slot = rel / DOCK_SLOT;
    if (slot < DOCK_APEK) {
        dock_uruchom(slot);
        return;
    }
    struct okno *min = pasek_zadan_ikona_na_pozycji(x, y);
    if (min) okno_przywroc(min);
}