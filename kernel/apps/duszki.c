/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include "duszki.h"
#include <drivers/input/keyboard.h>
#include <drivers/input/mouse.h>
#include <graphics/rysowanie.h>
#include <graphics/okno.h>
#include <graphics/czcionka.h>
#include <include/printf/types.h>
#include <kernel/proces.h>

extern uint32_t *obraz;

#define DUSZKI_MAP_W 20
#define DUSZKI_MAP_H 18
#define DUSZKI_CELL 24
#define DUSZKI_OFFSET_X 100
#define DUSZKI_OFFSET_Y 80

#define MAX_DUSZKI 400
#define MAX_WROGOW 4

#define PUSTE 0
#define SCIANA 1
#define KROPLKA 2
#define AMULET 3

typedef struct {
    int x;
    int y;
    int dir;
    int aktywny;
    int strach;
    int strach_timer;
} wróg_t;

static uint8_t duszki_mapa[DUSZKI_MAP_H][DUSZKI_MAP_W];

static const uint8_t duszki_mapa_wzorzec[DUSZKI_MAP_H][DUSZKI_MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,2,1},
    {1,3,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,3,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,2,2,2,2,2,1,2,2,2,1},
    {1,1,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,1,1},
    {0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0},
    {1,1,1,1,2,1,2,1,1,3,1,1,2,1,2,1,2,1,1,1},
    {0,0,0,0,2,0,0,0,1,1,1,0,0,0,2,0,0,0,0,0},
    {1,1,1,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,1,2,1},
    {1,3,2,2,2,1,2,2,2,3,2,2,2,3,2,2,2,3,2,1},
    {1,1,1,1,2,1,2,1,2,1,2,1,2,1,2,1,2,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static int duszki_gracz_x = 5;
static int duszki_gracz_y = 4;
static int duszki_gracz_dir = 0;
static int duszki_gracz_next_dir = 0;
static int duszki_zycia = 3;
static int duszki_punkty = 0;
static int duszki_poziom = 1;
static int duszki_koniec = 0;
static int duszki_pozostalo_kropek = 0;

static wróg_t wrogowie[MAX_WROGOW];

static uint32_t duszki_rand_seed = 192837452;

static uint32_t duszki_rand(void)
{
    duszki_rand_seed = duszki_rand_seed * 1103515245 + 12345;
    return (duszki_rand_seed >> 16) & 0x7FFF;
}

static struct okno okno_duszki;

static void duszki_init(void)
{
    duszki_gracz_x = 5;
    duszki_gracz_y = 4;
    duszki_gracz_dir = 0;
    duszki_gracz_next_dir = 0;
    duszki_zycia = 3;
    duszki_punkty = 0;
    duszki_poziom = 1;
    duszki_koniec = 0;

    wrogowie[0].x = 1; wrogowie[0].y = 1; wrogowie[0].dir = 1; wrogowie[0].aktywny = 1; wrogowie[0].strach = 0; wrogowie[0].strach_timer = 0;
    wrogowie[1].x = 18; wrogowie[1].y = 1; wrogowie[1].dir = 3; wrogowie[1].aktywny = 1; wrogowie[1].strach = 0; wrogowie[1].strach_timer = 0;
    wrogowie[2].x = 1; wrogowie[2].y = 16; wrogowie[2].dir = 2; wrogowie[2].aktywny = 1; wrogowie[2].strach = 0; wrogowie[2].strach_timer = 0;
    wrogowie[3].x = 18; wrogowie[3].y = 16; wrogowie[3].dir = 0; wrogowie[3].aktywny = 1; wrogowie[3].strach = 0; wrogowie[3].strach_timer = 0;

    for (int y = 0; y < DUSZKI_MAP_H; y++) {
        for (int x = 0; x < DUSZKI_MAP_W; x++) {
            duszki_mapa[y][x] = duszki_mapa_wzorzec[y][x];
        }
    }

    duszki_pozostalo_kropek = 0;
    for (int y = 0; y < DUSZKI_MAP_H; y++) {
        for (int x = 0; x < DUSZKI_MAP_W; x++) {
            if (duszki_mapa[y][x] == KROPLKA || duszki_mapa[y][x] == AMULET) {
                duszki_pozostalo_kropek++;
            }
        }
    }
}

static int duszki_czy_wolne(int mx, int my)
{
    if (mx < 0 || mx >= DUSZKI_MAP_W || my < 0 || my >= DUSZKI_MAP_H) return 0;
    return duszki_mapa[my][mx] != SCIANA;
}

static void duszki_rusz_gracza(int dir)
{
    int nx = duszki_gracz_x;
    int ny = duszki_gracz_y;
    if (dir == 0) nx++;
    else if (dir == 1) nx--;
    else if (dir == 2) ny--;
    else if (dir == 3) ny++;

    if (duszki_czy_wolne(nx, ny)) {
        duszki_gracz_x = nx;
        duszki_gracz_y = ny;
        duszki_gracz_dir = dir;

        uint8_t komorka = duszki_mapa[duszki_gracz_y][duszki_gracz_x];
        if (komorka == KROPLKA) {
            duszki_mapa[duszki_gracz_y][duszki_gracz_x] = PUSTE;
            duszki_punkty += 10;
            duszki_pozostalo_kropek--;
        } else if (komorka == AMULET) {
            duszki_mapa[duszki_gracz_y][duszki_gracz_x] = PUSTE;
            duszki_punkty += 50;
            duszki_pozostalo_kropek--;
            for (int i = 0; i < MAX_WROGOW; i++) {
                wrogowie[i].strach = 1;
                wrogowie[i].strach_timer = 400;
            }
        }
    }
}

static void duszki_rusz_wroga(int i)
{
    if (!wrogowie[i].aktywny) return;

    if (wrogowie[i].strach) {
        wrogowie[i].strach_timer--;
        if (wrogowie[i].strach_timer <= 0) {
            wrogowie[i].strach = 0;
        }
    }

    int nx = wrogowie[i].x;
    int ny = wrogowie[i].y;
    int dir = wrogowie[i].dir;

    if (dir == 0) nx++;
    else if (dir == 1) nx--;
    else if (dir == 2) ny--;
    else if (dir == 3) ny++;

    if (!duszki_czy_wolne(nx, ny)) {
        int nowe_diry[4] = {0, 1, 2, 3};
        for (int a = 0; a < 4; a++) {
            int b = a + (duszki_rand() % (4 - a));
            int tmp = nowe_diry[a]; nowe_diry[a] = nowe_diry[b]; nowe_diry[b] = tmp;
        }
        for (int a = 0; a < 4; a++) {
            int tnx = wrogowie[i].x;
            int tny = wrogowie[i].y;
            if (nowe_diry[a] == 0) tnx++;
            else if (nowe_diry[a] == 1) tnx--;
            else if (nowe_diry[a] == 2) tny--;
            else if (nowe_diry[a] == 3) tny++;
            if (duszki_czy_wolne(tnx, tny)) {
                wrogowie[i].dir = nowe_diry[a];
                return;
            }
        }
        return;
    }

    wrogowie[i].x = nx;
    wrogowie[i].y = ny;
}

static void duszki_sprawdź_kolizje(void)
{
    for (int i = 0; i < MAX_WROGOW; i++) {
        if (!wrogowie[i].aktywny) continue;
        if (wrogowie[i].x == duszki_gracz_x && wrogowie[i].y == duszki_gracz_y) {
            if (wrogowie[i].strach) {
                wrogowie[i].aktywny = 0;
                duszki_punkty += 200;
            } else {
                duszki_zycia--;
                if (duszki_zycia <= 0) {
                    duszki_koniec = 1;
                } else {
                    duszki_gracz_x = 5;
                    duszki_gracz_y = 4;
                    wrogowie[0].x = 1; wrogowie[0].y = 1; wrogowie[0].dir = 1;
                    wrogowie[1].x = 18; wrogowie[1].y = 1; wrogowie[1].dir = 3;
                    wrogowie[2].x = 1; wrogowie[2].y = 16; wrogowie[2].dir = 2;
                    wrogowie[3].x = 18; wrogowie[3].y = 16; wrogowie[3].dir = 0;
                    for (int j = 0; j < MAX_WROGOW; j++) {
                        wrogowie[j].strach = 0;
                        wrogowie[j].strach_timer = 0;
                    }
                }
            }
        }
    }
}

static void duszki_rysuj_kratke(int ox, int oy)
{
    for (int y = 0; y < DUSZKI_MAP_H; y++) {
        for (int x = 0; x < DUSZKI_MAP_W; x++) {
            int px = ox + x * DUSZKI_CELL;
            int py = oy + y * DUSZKI_CELL;
            uint8_t komorka = duszki_mapa[y][x];

            if (komorka == SCIANA) {
                rysuj_prostokat(px, py, DUSZKI_CELL, DUSZKI_CELL, 0x004B0082);
                rysuj_prostokat(px + 2, py + 2, DUSZKI_CELL - 4, DUSZKI_CELL - 4, 0x008A2BE2);
            } else if (komorka == KROPLKA) {
                wypelnij_okrag(px + DUSZKI_CELL / 2, py + DUSZKI_CELL / 2, 3, KOLOR_ZOLTY);
            } else if (komorka == AMULET) {
                wypelnij_okrag(px + DUSZKI_CELL / 2, py + DUSZKI_CELL / 2, 6, 0x00FFD700);
                rysuj_okrag(px + DUSZKI_CELL / 2, py + DUSZKI_CELL / 2, 8, 0x00FFD700);
            }
        }
    }
}

static void duszki_rysuj_gracza(int ox, int oy)
{
    int cx = ox + duszki_gracz_x * DUSZKI_CELL + DUSZKI_CELL / 2;
    int cy = oy + duszki_gracz_y * DUSZKI_CELL + DUSZKI_CELL / 2;
    int promien = DUSZKI_CELL / 2 - 2;

    wypelnij_okrag(cx, cy, promien, 0x00FF00FF);
    wypelnij_okrag(cx - 4, cy - 4, 3, KOLOR_BIALY);
    wypelnij_okrag(cx + 4, cy - 4, 3, KOLOR_BIALY);
}

static void duszki_rysuj_wroga(int ox, int oy, int i)
{
    if (!wrogowie[i].aktywny) return;

    int cx = ox + wrogowie[i].x * DUSZKI_CELL + DUSZKI_CELL / 2;
    int cy = oy + wrogowie[i].y * DUSZKI_CELL + DUSZKI_CELL / 2;
    int promien = DUSZKI_CELL / 2 - 2;

    uint32_t kolor = wrogowie[i].strach ? 0x000000FF : 0x00FF4500;
    wypelnij_okrag(cx, cy, promien, kolor);
    wypelnij_okrag(cx - 4, cy - 4, 3, KOLOR_BIALY);
    wypelnij_okrag(cx + 4, cy - 4, 3, KOLOR_BIALY);
}

static void duszki_rysuj_ui(int gx, int gy, int gw, int gh)
{
    char txt[64];
    int len = 0;

    rysuj_prostokat(gx, gy, gw, 20, 0x001E293B);

    txt[0] = 'P'; txt[1] = 'U'; txt[2] = 'N'; txt[3] = 'K'; txt[4] = 'T'; txt[5] = 'Y'; txt[6] = ':'; txt[7] = ' ';
    len = 8;
    int tmp = duszki_punkty;
    char rev[16]; int rlen = 0;
    if (tmp == 0) rev[rlen++] = '0';
    while (tmp > 0) { rev[rlen++] = '0' + (tmp % 10); tmp /= 10; }
    while (rlen > 0) txt[len++] = rev[--rlen];
    txt[len] = 0;
    rysuj_tekst_chicago(gx + 4, gy + 2, txt, KOLOR_BIALY);

    txt[0] = 'Z'; txt[1] = 'Y'; txt[2] = 'C'; txt[3] = 'I'; txt[4] = ':'; txt[5] = ' ';
    len = 6;
    tmp = duszki_zycia;
    rlen = 0;
    if (tmp == 0) rev[rlen++] = '0';
    while (tmp > 0) { rev[rlen++] = '0' + (tmp % 10); tmp /= 10; }
    while (rlen > 0) txt[len++] = rev[--rlen];
    txt[len] = 0;
    rysuj_tekst_chicago(gx + gw - 56, gy + 2, txt, KOLOR_BIALY);
}

static void duszki_rysuj_koniec(int gx, int gy, int gw, int gh)
{
    const char *msg = "KONIEC GRY";
    int msg_len = 10;
    int char_w = 8;
    int start_x = gx + (gw - msg_len * char_w) / 2;
    int start_y = gy + gh / 2 - 8;
    rysuj_tekst_chicago(start_x, start_y, msg, KOLOR_CZERWONY);

    const char *msg2 = "Nacisnij R";
    int msg2_len = 10;
    int start_x2 = gx + (gw - msg2_len * char_w) / 2;
    int start_y2 = gy + gh / 2 + 8;
    rysuj_tekst_chicago(start_x2, start_y2, msg2, KOLOR_BIALY);
}

static void duszki_rysuj(int gx, int gy, int gw, int gh)
{
    rysuj_prostokat(gx, gy, gw, gh, 0x00000011);

    duszki_rysuj_ui(gx, gy, gw, gh);

    duszki_rysuj_kratke(gx, gy + 24);
    duszki_rysuj_gracza(gx, gy + 24);
    for (int i = 0; i < MAX_WROGOW; i++) {
        duszki_rysuj_wroga(gx, gy + 24, i);
    }

    if (duszki_koniec) {
        duszki_rysuj_koniec(gx, gy, gw, gh);
    }

    odswiez_fragment_widoku(gx, gy, gw, gh);
}

void duszki(void)
{
    keyboard_init();
    okno_stworz(&okno_duszki, 100, 100, 600, 500, "DUSZKI", duszki_rysuj);
    proces_rejestruj_okno("DUSZKI", &okno_duszki);

    duszki_init();

    int klatka = 0;
    int ruch_co = 12;
    int gracz_ruch_co = 8;

    while (okno_duszki.otwarte) {
        char znak = 0;
        while (keyboard_try_getchar(&znak)) {
            if (znak == 27) {
                okno_zamknij(&okno_duszki);
                return;
            }
            if (duszki_koniec) {
                if (znak == 'r' || znak == 'R') {
                    duszki_init();
                }
                continue;
            }
            if (znak == 'w' || znak == 'W' || znak == KEYBOARD_UP) duszki_gracz_next_dir = 2;
            if (znak == 's' || znak == 'S' || znak == KEYBOARD_DOWN) duszki_gracz_next_dir = 3;
            if (znak == 'a' || znak == 'A' || znak == KEYBOARD_LEFT) duszki_gracz_next_dir = 1;
            if (znak == 'd' || znak == 'D' || znak == KEYBOARD_RIGHT) duszki_gracz_next_dir = 0;
        }

        mouse_poll();

        if (!duszki_koniec) {
            if (klatka % gracz_ruch_co == 0) {
                int nx = duszki_gracz_x;
                int ny = duszki_gracz_y;
                if (duszki_gracz_next_dir == 0) nx++;
                else if (duszki_gracz_next_dir == 1) nx--;
                else if (duszki_gracz_next_dir == 2) ny--;
                else if (duszki_gracz_next_dir == 3) ny++;
                if (duszki_czy_wolne(nx, ny)) {
                    duszki_gracz_dir = duszki_gracz_next_dir;
                }
                duszki_rusz_gracza(duszki_gracz_dir);
            }

            if (klatka % ruch_co == 0) {
                for (int i = 0; i < MAX_WROGOW; i++) {
                    duszki_rusz_wroga(i);
                }
            }

            duszki_sprawdź_kolizje();

            if (duszki_pozostalo_kropek == 0) {
                duszki_poziom++;
                duszki_init();
            }
        }

        klatka++;

        int gx = okno_duszki.x + 8;
        int gy = okno_duszki.y + 30;
        int gw = okno_duszki.szerokosc - 16;
        int gh = okno_duszki.wysokosc - 38;
        duszki_rysuj(gx, gy, gw, gh);

        for (int i = 0; i < 50000; i++) {
            __asm__ volatile("nop");
        }
    }
}
