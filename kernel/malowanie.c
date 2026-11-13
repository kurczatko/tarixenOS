#include <kernel/malowanie.h>
#include <graficzny/czcionka.h>
#include <graficzny/okno.h>
#include <graficzny/rysowanie.h>
#include <system_plikow/system_plikow.h>
#include <drivers/keyboard.h>
#include <include/printf/types.h>
#include <kernel/proces.h>

extern uint32_t *obraz;

#define MAL_PLIK "OBRAZ.MAL"
#define WIN_W 450
#define WIN_H 400
#define CANVAS_W 320
#define CANVAS_H 240
#define CANVAS_LEFT 8
#define CANVAS_TOP 30
#define BRUSH_R 3
#define PAL_Y 280
#define PAL_X0 16
#define PAL_SZ 32
#define PAL_SPACJA 6
#define BTN_Y 330
#define BTN_SAVE_X 260
#define BTN_LOAD_X 320
#define BTN_CLEAR_X 380
#define BTN_W 50
#define BTN_H 24
#define STATUS_Y 365

#define KOLOR_PLAX 0x004A5D6E
#define KOLOR_TEKST_PRZYCISKU 0x00FFFFFF

static const uint32_t PALETA[6] = {
    KOLOR_CZERWONY, KOLOR_ZIELONY, KOLOR_NIEBIESKI, KOLOR_ZOLTY, KOLOR_CZARNY, KOLOR_BIALY
};
static const char *NAZWA_PAL[6] = {
    "CZERW", "ZIEL", "NIEB", "ZOLTY", "CZARNY", "GUMKA"
};

struct okno okno_malowanie;
uint8_t malowanie_otwarty = 0;
static int malowanie_kolor = 0;
static char malowanie_status[32];
static char mal_buf[7 + CANVAS_W * CANVAS_H * 4];
static int mal_ostatni_x = -1;
static int mal_ostatni_y = -1;
static uint8_t mal_rysowanie = 0;

static void malowanie_ustaw_status(const char *tekst)
{
    int i = 0;
    while (tekst[i] && i < 31) {
        malowanie_status[i] = tekst[i];
        i++;
    }
    malowanie_status[i] = 0;
}

static uint32_t mal_piksel(int indeks)
{
    return (uint32_t)mal_buf[indeks] |
           ((uint32_t)mal_buf[indeks + 1] << 8) |
           ((uint32_t)mal_buf[indeks + 2] << 16) |
           ((uint32_t)mal_buf[indeks + 3] << 24);
}

static int mal_piksel_offset(int px, int py)
{
    return 7 + (py * CANVAS_W + px) * 4;
}

static void malowanie_wyczysc(void)
{
    for (int i = 0; i < CANVAS_W * CANVAS_H; i++) {
        int off = 7 + i * 4;
        mal_buf[off + 0] = 0xFF;
        mal_buf[off + 1] = 0xFF;
        mal_buf[off + 2] = 0xFF;
        mal_buf[off + 3] = 0x00;
    }
    malowanie_ustaw_status("wyczyszczono");
    mal_ostatni_x = -1;
    mal_ostatni_y = -1;
    mal_rysowanie = 0;
    okno_narysuj(&okno_malowanie);
}

static int malowanie_zapisz(void)
{
    int total;
    mal_buf[0] = 'M';
    mal_buf[1] = 'A';
    mal_buf[2] = 'L';
    mal_buf[3] = (unsigned char)(CANVAS_W & 0xFF);
    mal_buf[4] = (unsigned char)(CANVAS_W >> 8);
    mal_buf[5] = (unsigned char)(CANVAS_H & 0xFF);
    mal_buf[6] = (unsigned char)(CANVAS_H >> 8);
    total = 7 + CANVAS_W * CANVAS_H * 4;
    fs_reset_directory();
    if (create_file(MAL_PLIK, (uint32_t)total) != 0) {
        /* plik juz istnieje - nadpiszemy go write_file */
    }
    if (write_file(MAL_PLIK, mal_buf, (uint32_t)total) != 0) {
        malowanie_ustaw_status("blad zapisu");
        return -1;
    }
    malowanie_ustaw_status("zapisano MAL");
    return 0;
}

static void malowanie_wczytaj(void)
{
    int odczyt;
    int w;
    int h;
    fs_reset_directory();
    odczyt = read_file(MAL_PLIK, mal_buf, (uint32_t)sizeof(mal_buf));
    if (odczyt < 7 || mal_buf[0] != 'M' || mal_buf[1] != 'A' || mal_buf[2] != 'L') {
        malowanie_ustaw_status("zly format");
        return;
    }
    w = mal_buf[3] | (mal_buf[4] << 8);
    h = mal_buf[5] | (mal_buf[6] << 8);
    if (w != CANVAS_W || h != CANVAS_H) {
        malowanie_ustaw_status("zly rozmiar");
        return;
    }
    malowanie_ustaw_status("wczytano MAL");
    mal_ostatni_x = -1;
    mal_ostatni_y = -1;
    mal_rysowanie = 0;
    okno_narysuj(&okno_malowanie);
}

static void malowanie_rysuj_piksel(int px, int py, uint32_t kolor)
{
    if (px < 0 || px >= CANVAS_W || py < 0 || py >= CANVAS_H) return;
    int off = mal_piksel_offset(px, py);
    mal_buf[off + 0] = (unsigned char)(kolor & 0xFF);
    mal_buf[off + 1] = (unsigned char)((kolor >> 8) & 0xFF);
    mal_buf[off + 2] = (unsigned char)((kolor >> 16) & 0xFF);
    mal_buf[off + 3] = (unsigned char)((kolor >> 24) & 0xFF);
}

static void malowanie_rysuj_liniedo(int x0, int y0, int x1, int y1, uint32_t kolor)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = dx < 0 ? -1 : 1;
    int sy = dy < 0 ? -1 : 1;
    dx = dx < 0 ? -dx : dx;
    dy = dy < 0 ? -dy : dy;
    int err = (dx > dy ? dx : -dy) / 2;
    int cx = x0;
    int cy = y0;
    for (;;) {
        malowanie_rysuj_piksel(cx, cy, kolor);
        if (cx == x1 && cy == y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; cx += sx; }
        if (e2 < dy) { err += dx; cy += sy; }
    }
}

static void malowanie_rysuj_petelek(int cx, int cy, uint32_t kolor)
{
    for (int i = -BRUSH_R; i <= BRUSH_R; i++) {
        int py = cy + i;
        if (py < 0 || py >= CANVAS_H) continue;
        for (int j = -BRUSH_R; j <= BRUSH_R; j++) {
            int px = cx + j;
            if (px < 0 || px >= CANVAS_W) continue;
            malowanie_rysuj_piksel(px, py, kolor);
        }
    }
}

static void malowanie_rysuj_zawartosc(int x, int y, int szerokosc, int wysokosc)
{
    int px;
    int py;
    int i;
    (void)szerokosc;
    (void)wysokosc;

    rysuj_prostokat(x, y, CANVAS_W, CANVAS_H, KOLOR_SZARY);
    for (py = 0; py < CANVAS_H; py++) {
        for (px = 0; px < CANVAS_W; px++) {
            uint32_t c = mal_piksel(mal_piksel_offset(px, py));
            if (c != KOLOR_BIALY) {
                obraz[(y + py) * EKRAN_SZEROKOSC + (x + px)] = c;
            }
        }
    }

    for (i = 0; i < 6; i++) {
        int bx = PAL_X0 + i * (PAL_SZ + PAL_SPACJA);
        rysuj_prostokat(x + bx, y + PAL_Y, PAL_SZ, PAL_SZ, KOLOR_PLAX);
        rysuj_prostokat(x + bx + 2, y + PAL_Y + 2, PAL_SZ - 4, PAL_SZ - 4, PALETA[i]);
        if (i == malowanie_kolor) {
            rysuj_prostokat(x + bx, y + PAL_Y, PAL_SZ, 3, KOLOR_BIALY);
            rysuj_prostokat(x + bx, y + PAL_Y + PAL_SZ - 3, PAL_SZ, 3, KOLOR_BIALY);
            rysuj_prostokat(x + bx, y + PAL_Y, 3, PAL_SZ, KOLOR_BIALY);
            rysuj_prostokat(x + bx + PAL_SZ - 3, y + PAL_Y, 3, PAL_SZ, KOLOR_BIALY);
        }
        rysuj_tekst_chicago(x + bx + 2, y + PAL_Y + PAL_SZ + 2, NAZWA_PAL[i], KOLOR_TEKST_PRZYCISKU);
    }

    rysuj_prostokat(x + BTN_SAVE_X, y + BTN_Y, BTN_W, BTN_H, KOLOR_PLAX);
    rysuj_tekst_chicago(x + BTN_SAVE_X + 4, y + BTN_Y + 4, "ZAPISZ", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + BTN_LOAD_X, y + BTN_Y, BTN_W, BTN_H, KOLOR_PLAX);
    rysuj_tekst_chicago(x + BTN_LOAD_X + 4, y + BTN_Y + 4, "WCZYTAJ", KOLOR_TEKST_PRZYCISKU);
    rysuj_prostokat(x + BTN_CLEAR_X, y + BTN_Y, BTN_W, BTN_H, KOLOR_PLAX);
    rysuj_tekst_chicago(x + BTN_CLEAR_X + 4, y + BTN_Y + 4, "CZYSCZ", KOLOR_TEKST_PRZYCISKU);

    rysuj_tekst_chicago(x + 8, y + STATUS_Y, malowanie_status, KOLOR_TEKST_PRZYCISKU);
}

static void malowanie_klik(int x, int y)
{
    int cx = x - CANVAS_LEFT;
    int cy = y - CANVAS_TOP;
    int kolor;
    int i;
    int bx;

    if (cx >= 0 && cx < CANVAS_W && cy >= 0 && cy < CANVAS_H) {
        mal_rysowanie = 1;
        mal_ostatni_x = cx;
        mal_ostatni_y = cy;
        kolor = PALETA[malowanie_kolor];
        malowanie_rysuj_petelek(cx, cy, kolor);
        okno_narysuj(&okno_malowanie);
        return;
    }

    if (cy >= PAL_Y && cy < PAL_Y + PAL_SZ) {
        for (i = 0; i < 6; i++) {
            bx = PAL_X0 + i * (PAL_SZ + PAL_SPACJA);
            if (cx >= bx && cx < bx + PAL_SZ) {
                malowanie_kolor = i;
                mal_rysowanie = 0;
                mal_ostatni_x = -1;
                mal_ostatni_y = -1;
                okno_narysuj(&okno_malowanie);
                return;
            }
        }
    }

    if (cy >= BTN_Y && cy < BTN_Y + BTN_H) {
        if (cx >= BTN_SAVE_X && cx < BTN_SAVE_X + BTN_W) {
            if (malowanie_zapisz() == 0) {
                okno_narysuj(&okno_malowanie);
            }
            return;
        }
        if (cx >= BTN_LOAD_X && cx < BTN_LOAD_X + BTN_W) {
            malowanie_wczytaj();
            okno_narysuj(&okno_malowanie);
            return;
        }
        if (cx >= BTN_CLEAR_X && cx < BTN_CLEAR_X + BTN_W) {
            malowanie_wyczysc();
            return;
        }
    }
}

static void malowanie_ruch(struct okno *okno, int x, int y)
{
    (void)okno;
    int cx = x - CANVAS_LEFT;
    int cy = y - CANVAS_TOP;

    if (!mal_rysowanie) return;
    if (cx < 0 || cx >= CANVAS_W || cy < 0 || cy >= CANVAS_H) {
        mal_rysowanie = 0;
        mal_ostatni_x = -1;
        mal_ostatni_y = -1;
        return;
    }

    if (mal_ostatni_x >= 0 && mal_ostatni_y >= 0) {
        uint32_t kolor = PALETA[malowanie_kolor];
        malowanie_rysuj_liniedo(mal_ostatni_x, mal_ostatni_y, cx, cy, kolor);
    }
    {
        uint32_t kolor = PALETA[malowanie_kolor];
        malowanie_rysuj_petelek(cx, cy, kolor);
    }

    mal_ostatni_x = cx;
    mal_ostatni_y = cy;
    okno_narysuj(&okno_malowanie);
}

void malowanie_otworz(void)
{
    malowanie_otwarty = 1;
    malowanie_wyczysc();
    malowanie_kolor = 0;
    malowanie_ustaw_status("gotowy");
    okno_stworz(&okno_malowanie, 100, 60, WIN_W, WIN_H, "MALOWANIE", malowanie_rysuj_zawartosc);
    proces_rejestruj_okno("MALOWANIE", &okno_malowanie);
    okno_ustaw_obsluge_klikniecia(&okno_malowanie, malowanie_klik);
    okno_ustaw_obsluge_ruchu(&okno_malowanie, malowanie_ruch);
}

void malowanie_obsluz_klawiature_znak(char znak)
{
    if (!malowanie_otwarty) return;
    if (!okno_malowanie.otwarte) {
        malowanie_otwarty = 0;
        return;
    }
    if (!okno_czy_aktywne(&okno_malowanie)) return;
    if (znak == 27) {
        okno_zamknij(&okno_malowanie);
        malowanie_otwarty = 0;
        return;
    }
    if (znak == 's') {
        if (malowanie_zapisz() == 0) {
            okno_narysuj(&okno_malowanie);
        }
    } else if (znak == 'l') malowanie_wczytaj();
    else if (znak == 'c') malowanie_wyczysc();
    else return;
    okno_narysuj(&okno_malowanie);
}
