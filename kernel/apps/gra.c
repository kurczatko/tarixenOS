#include "gra.h"
#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/okno.h"
#include "../graficzny/czcionka.h"
#include "../include/printf/types.h"

extern uint32_t *obraz;

#define GRA_MAP_W 16
#define GRA_MAP_H 16

static const uint8_t gra_mapa[GRA_MAP_H][GRA_MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,1,1,1,0,1,1,1,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,1,1,1,1,1,1,1,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,0,1,1,1,0,1,1,0,1,1,1,0,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static double gra_pos_x = 1.5;
static double gra_pos_y = 1.5;
static double gra_dir_x = 1.0;
static double gra_dir_y = 0.0;
static double gra_plane_x = 0.0;
static double gra_plane_y = 0.66;

static struct okno okno_gra;

static double gra_fabs(double v)
{
    return v < 0 ? -v : v;
}

static void gra_obrot(double kat)
{
    double s;
    double c;
    if (kat > 0) {
        s = 0.04998;
        c = 0.99875;
    } else {
        s = -0.04998;
        c = 0.99875;
    }
    double stara_dir_x = gra_dir_x;
    gra_dir_x = gra_dir_x * c - gra_dir_y * s;
    gra_dir_y = stara_dir_x * s + gra_dir_y * c;
    double stara_plane_x = gra_plane_x;
    gra_plane_x = gra_plane_x * c - gra_plane_y * s;
    gra_plane_y = stara_plane_x * s + gra_plane_y * c;
}

static void gra_rysuj_zawartosc(int x, int y, int szerokosc, int wysokosc)
{
    for (int rx = 0; rx < szerokosc; rx++) {
        for (int ry = 0; ry < wysokosc / 2; ry++) {
            obraz[(y + ry) * EKRAN_SZEROKOSC + (x + rx)] = 0x00404040;
        }
        for (int ry = wysokosc / 2; ry < wysokosc; ry++) {
            obraz[(y + ry) * EKRAN_SZEROKOSC + (x + rx)] = 0x00101010;
        }
    }

    for (int rx = 0; rx < szerokosc; rx++) {
        double kamera_x = 2 * rx / (double)szerokosc - 1;
        double promien_x = gra_dir_x + gra_plane_x * kamera_x;
        double promien_y = gra_dir_y + gra_plane_y * kamera_x;
        int map_x = (int)gra_pos_x;
        int map_y = (int)gra_pos_y;
        double delta_dist_x = promien_x == 0 ? 1e30 : gra_fabs(1 / promien_x);
        double delta_dist_y = promien_y == 0 ? 1e30 : gra_fabs(1 / promien_y);
        double dystans_boku_x;
        double dystans_boku_y;
        int kroku_x;
        int kroku_y;
        if (promien_x < 0) {
            kroku_x = -1;
            dystans_boku_x = (gra_pos_x - map_x) * delta_dist_x;
        } else {
            kroku_x = 1;
            dystans_boku_x = (map_x + 1.0 - gra_pos_x) * delta_dist_x;
        }
        if (promien_y < 0) {
            kroku_y = -1;
            dystans_boku_y = (gra_pos_y - map_y) * delta_dist_y;
        } else {
            kroku_y = 1;
            dystans_boku_y = (map_y + 1.0 - gra_pos_y) * delta_dist_y;
        }
        int uderzenie = 0;
        int strona = 0;
        while (uderzenie == 0) {
            if (dystans_boku_x < dystans_boku_y) {
                dystans_boku_x += delta_dist_x;
                map_x += kroku_x;
                strona = 0;
            } else {
                dystans_boku_y += delta_dist_y;
                map_y += kroku_y;
                strona = 1;
            }
            if (map_x < 0 || map_x >= GRA_MAP_W || map_y < 0 || map_y >= GRA_MAP_H) break;
            if (gra_mapa[map_y][map_x] == 1) uderzenie = 1;
        }
        if (uderzenie == 0) continue;
        double dystans_proj;
        if (strona == 0) dystans_proj = (map_x - gra_pos_x + (1 - kroku_x) / 2) / promien_x;
        else dystans_proj = (map_y - gra_pos_y + (1 - kroku_y) / 2) / promien_y;
        int wysokosc_linii = (int)(wysokosc / dystans_proj);
        int poczatek = -wysokosc_linii / 2 + wysokosc / 2;
        if (poczatek < 0) poczatek = 0;
        int koniec = wysokosc_linii / 2 + wysokosc / 2;
        if (koniec >= wysokosc) koniec = wysokosc - 1;
        uint32_t kolor = strona == 0 ? 0x00FF0000 : 0x00800000;
        double cien = 1.0 / (1.0 + dystans_proj * 0.2);
        uint32_t r = (uint32_t)(((kolor >> 16) & 0xFF) * cien);
        uint32_t g = (uint32_t)(((kolor >> 8) & 0xFF) * cien);
        uint32_t b = (uint32_t)((kolor & 0xFF) * cien);
        kolor = (r << 16) | (g << 8) | b;
        for (int ry = poczatek; ry <= koniec; ry++) {
            obraz[(y + ry) * EKRAN_SZEROKOSC + (x + rx)] = kolor;
        }
    }

    int cx = x + szerokosc / 2;
    int cy = y + wysokosc / 2;
    obraz[cy * EKRAN_SZEROKOSC + cx] = 0x00FFFFFF;
    obraz[(cy - 1) * EKRAN_SZEROKOSC + cx] = 0x00FFFFFF;
    obraz[(cy + 1) * EKRAN_SZEROKOSC + cx] = 0x00FFFFFF;
    obraz[cy * EKRAN_SZEROKOSC + cx - 1] = 0x00FFFFFF;
    obraz[cy * EKRAN_SZEROKOSC + cx + 1] = 0x00FFFFFF;
}

void gra(void)
{
    keyboard_init();
    okno_stworz(&okno_gra, 100, 100, 640, 480, "GRA", gra_rysuj_zawartosc);
    while (okno_gra.otwarte) {
        char znak = 0;
        while (keyboard_try_getchar(&znak)) {
            if (znak == 27) {
                okno_zamknij(&okno_gra);
                return;
            }
            double predkosc = 0.07;
            if (znak == 'w') {
                double nx = gra_pos_x + gra_dir_x * predkosc;
                double ny = gra_pos_y + gra_dir_y * predkosc;
                if (nx >= 0 && nx < GRA_MAP_W && ny >= 0 && ny < GRA_MAP_H &&
                    gra_mapa[(int)ny][(int)nx] == 0) {
                    gra_pos_x = nx;
                    gra_pos_y = ny;
                }
            }
            if (znak == 's') {
                double nx = gra_pos_x - gra_dir_x * predkosc;
                double ny = gra_pos_y - gra_dir_y * predkosc;
                if (nx >= 0 && nx < GRA_MAP_W && ny >= 0 && ny < GRA_MAP_H &&
                    gra_mapa[(int)ny][(int)nx] == 0) {
                    gra_pos_x = nx;
                    gra_pos_y = ny;
                }
            }
            if (znak == 'a') gra_obrot(-0.05);
            if (znak == 'd') gra_obrot(0.05);
        }
        mouse_poll();
        if (!okno_gra.otwarte) break;
        int mx;
        int my;
        mouse_pozycja(&mx, &my);
        static int ostatni_x;
        static int ostatni_y;
        int dx = mx - ostatni_x;
        int dy = my - ostatni_y;
        if (dx != 0) {
            gra_obrot(dx * 0.0015);
        }
        ostatni_x = mx;
        ostatni_y = my;
        gra_rysuj_zawartosc(okno_gra.x + 8, okno_gra.y + 30, okno_gra.szerokosc - 16, okno_gra.wysokosc - 38);
        odswiez_fragment_widoku(okno_gra.x + 8, okno_gra.y + 30, okno_gra.szerokosc - 16, okno_gra.wysokosc - 38);
        for (volatile int i = 0; i < 300000; i++) {
            __asm__ volatile("nop");
        }
    }
}
