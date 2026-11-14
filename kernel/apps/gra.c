/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/apps/gra.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <graficzny/rysowanie.h>
#include <graficzny/okno.h>
#include <graficzny/czcionka.h>
#include <include/printf/types.h>
#include <kernel/proces.h>

extern uint32_t *obraz;

#define GRA_MAP_W 16
#define GRA_MAP_H 16

#define MAX_PILKI 20
#define PILKA_ROZMIAR 0.3
#define PILKA_PREDKOSC 0.015
#define SPAWN_INTERVAL_MIN 300
#define SPAWN_INTERVAL_MAX 500

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

typedef struct {
    double x;
    double y;
    int aktywna;
} pilka_t;

static pilka_t pilki[MAX_PILKI];
static uint32_t spawn_timer = 0;
static int gra_koniec = 0;
static uint32_t rand_seed = 123456789;

static struct okno okno_gra;

static uint32_t gra_rand(void)
{
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed >> 16) & 0x7FFF;
}

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

static int czy_wolne_miejsce(double x, double y)
{
    int mx = (int)x;
    int my = (int)y;
    if (mx < 0 || mx >= GRA_MAP_W || my < 0 || my >= GRA_MAP_H) return 0;
    if (gra_mapa[my][mx] != 0) return 0;
    return 1;
}

static void spawn_pilki(void)
{
    int spawn_count = 1 + (gra_rand() % 3);
    int spawned = 0;
    
    for (int i = 0; i < MAX_PILKI && spawned < spawn_count; i++) {
        if (!pilki[i].aktywna) {
            int proby = 0;
            do {
                pilki[i].x = 1.5 + (double)(gra_rand() % (GRA_MAP_W - 3));
                pilki[i].y = 1.5 + (double)(gra_rand() % (GRA_MAP_H - 3));
                proby++;
            } while (!czy_wolne_miejsce(pilki[i].x, pilki[i].y) && proby < 100);
            
            if (czy_wolne_miejsce(pilki[i].x, pilki[i].y)) {
                pilki[i].aktywna = 1;
                spawned++;
            }
        }
    }
}

static void aktualizuj_pilki(void)
{
    for (int i = 0; i < MAX_PILKI; i++) {
        if (!pilki[i].aktywna) continue;
        
        double dx = gra_pos_x - pilki[i].x;
        double dy = gra_pos_y - pilki[i].y;
        double dist = gra_fabs(dx) + gra_fabs(dy);
        
        if (dist > 0) {
            double nx = pilki[i].x + (dx / dist) * PILKA_PREDKOSC;
            double ny = pilki[i].y + (dy / dist) * PILKA_PREDKOSC;
            
            if (czy_wolne_miejsce(nx, ny)) {
                pilki[i].x = nx;
                pilki[i].y = ny;
            } else {
                if (czy_wolne_miejsce(nx, pilki[i].y)) {
                    pilki[i].x = nx;
                } else if (czy_wolne_miejsce(pilki[i].x, ny)) {
                    pilki[i].y = ny;
                }
            }
        }
        
        double pdx = gra_pos_x - pilki[i].x;
        double pdy = gra_pos_y - pilki[i].y;
        double pdist = gra_fabs(pdx) + gra_fabs(pdy);
        if (pdist < 0.5) {
            gra_koniec = 1;
        }
    }
}

static void rysuj_pilke_3d(int x, int y, int szerokosc, int wysokosc)
{
    for (int i = 0; i < MAX_PILKI; i++) {
        if (!pilki[i].aktywna) continue;
        
        double sprite_x = pilki[i].x - gra_pos_x;
        double sprite_y = pilki[i].y - gra_pos_y;
        
        double inv_det = 1.0 / (gra_plane_x * gra_dir_y - gra_dir_x * gra_plane_y);
        double transform_x = inv_det * (gra_dir_y * sprite_x - gra_dir_x * sprite_y);
        double transform_y = inv_det * (-gra_plane_y * sprite_x + gra_plane_x * sprite_y);
        
        if (transform_y <= 0) continue;
        
        int sprite_screen_x = (int)((szerokosc / 2) * (1 + transform_x / transform_y));
        int sprite_height = (int)(wysokosc / transform_y);
        int sprite_width = sprite_height;
        
        int draw_start_y = -sprite_height / 2 + wysokosc / 2 + y;
        if (draw_start_y < y) draw_start_y = y;
        int draw_end_y = sprite_height / 2 + wysokosc / 2 + y;
        if (draw_end_y >= y + wysokosc) draw_end_y = y + wysokosc - 1;
        
        int draw_start_x = -sprite_width / 2 + sprite_screen_x + x;
        if (draw_start_x < x) draw_start_x = x;
        int draw_end_x = sprite_width / 2 + sprite_screen_x + x;
        if (draw_end_x >= x + szerokosc) draw_end_x = x + szerokosc - 1;
        
        for (int stripe = draw_start_x; stripe < draw_end_x; stripe++) {
            int tex_x = (int)(256 * (stripe - (-sprite_width / 2 + sprite_screen_x)) * 32 / sprite_width) / 256;
            if (transform_y > 0 && stripe > x && stripe < x + szerokosc) {
                for (int py = draw_start_y; py < draw_end_y; py++) {
                    int tex_y = (int)((py - (-sprite_height / 2 + wysokosc / 2 + y)) * 32 / sprite_height);
                    
                    int cx = 16;
                    int cy = 16;
                    int dx_tex = tex_x - cx;
                    int dy_tex = tex_y - cy;
                    int dist_sq = dx_tex * dx_tex + dy_tex * dy_tex;
                    int radius = 16;
                    
                    if (dist_sq <= radius * radius) {
                        uint32_t kolor = KOLOR_ZOLTY;
                        
                        int eye_y = 8;
                        int eye_radius = 3;
                        int left_eye_x = 8;
                        int right_eye_x = 24;
                        
                        int dx_left = tex_x - left_eye_x;
                        int dy_left = tex_y - eye_y;
                        if (dx_left * dx_left + dy_left * dy_left <= eye_radius * eye_radius) {
                            kolor = KOLOR_CZARNY;
                        }
                        
                        int dx_right = tex_x - right_eye_x;
                        int dy_right = tex_y - eye_y;
                        if (dx_right * dx_right + dy_right * dy_right <= eye_radius * eye_radius) {
                            kolor = KOLOR_CZARNY;
                        }
                        
                        int smile_y = 22;
                        if (tex_y == smile_y && tex_x >= 10 && tex_x <= 22) {
                            kolor = KOLOR_CZARNY;
                        }
                        if (tex_y == smile_y + 1 && tex_x >= 9 && tex_x <= 23) {
                            kolor = KOLOR_CZARNY;
                        }
                        
                        obraz[py * EKRAN_SZEROKOSC + stripe] = kolor;
                    }
                }
            }
        }
    }
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

    rysuj_pilke_3d(x, y, szerokosc, wysokosc);

    if (gra_koniec) {
        const char *msg = "przegrales";
        int msg_len = 9;
        int char_w = 8;
        int char_h = 8;
        int start_x = x + (szerokosc - msg_len * char_w) / 2;
        int start_y = y + (wysokosc - char_h) / 2;
        rysuj_tekst_8x8(start_x, start_y, msg, KOLOR_CZERWONY);
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
    proces_rejestruj_okno("GRA", &okno_gra);
    
    for (int i = 0; i < MAX_PILKI; i++) {
        pilki[i].aktywna = 0;
    }
    spawn_timer = 0;
    gra_koniec = 0;
    gra_pos_x = 1.5;
    gra_pos_y = 1.5;
    gra_dir_x = 1.0;
    gra_dir_y = 0.0;
    gra_plane_x = 0.0;
    gra_plane_y = 0.66;
    
    while (okno_gra.otwarte) {
        char znak = 0;
        while (keyboard_try_getchar(&znak)) {
            if (znak == 27) {
                okno_zamknij(&okno_gra);
                return;
            }
            if (gra_koniec) {
                if (znak == 'r' || znak == ' ') {
                    for (int i = 0; i < MAX_PILKI; i++) {
                        pilki[i].aktywna = 0;
                    }
                    spawn_timer = 0;
                    gra_koniec = 0;
                    gra_pos_x = 1.5;
                    gra_pos_y = 1.5;
                    gra_dir_x = 1.0;
                    gra_dir_y = 0.0;
                    gra_plane_x = 0.0;
                    gra_plane_y = 0.66;
                }
                continue;
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
        (void)dy;
        if (dx != 0) {
            gra_obrot(dx * 0.0015);
        }
        ostatni_x = mx;
        ostatni_y = my;
        
        if (!gra_koniec) {
            spawn_timer++;
            if (spawn_timer >= SPAWN_INTERVAL_MIN + (gra_rand() % (SPAWN_INTERVAL_MAX - SPAWN_INTERVAL_MIN))) {
                spawn_pilki();
                spawn_timer = 0;
            }
            aktualizuj_pilki();
        }
        
        gra_rysuj_zawartosc(okno_gra.x + 8, okno_gra.y + 30, okno_gra.szerokosc - 16, okno_gra.wysokosc - 38);
        odswiez_fragment_widoku(okno_gra.x + 8, okno_gra.y + 30, okno_gra.szerokosc - 16, okno_gra.wysokosc - 38);
        for (int i = 0; i < 300000; i++) {
            __asm__ volatile("nop");
        }
    }
}
