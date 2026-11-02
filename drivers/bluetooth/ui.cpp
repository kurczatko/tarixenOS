#include "bluetooth.h"

extern "C" {
#include "../graficzny/rysowanie.h"
#include "../graficzny/okno.h"
#include "../graficzny/czcionka.h"
}

/*ikona bluetooth na pasku zadan - bitmapa 12x12*/
static const uint8_t ikona_bluetooth_pasek[12][12] = {
    {0,0,0,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,0,0},
    {0,1,1,1,0,0,0,1,1,1,1,0},
    {1,1,1,0,0,0,0,0,1,1,1,1},
    {1,1,1,0,0,1,1,0,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,0,0,1,1,0,1,1,1,1},
    {1,1,1,0,0,0,0,0,1,1,1,1},
    {0,1,1,1,0,0,0,1,1,1,1,0},
    {0,0,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,0,0,0}
};

/*okno parowania bluetooth*/
static struct okno okno_bluetooth;
static uint8_t okno_bluetooth_otwarte = 0;
static uint8_t bluetooth_ikona_aktywna = 0;

/*pozycja ikony bluetooth na pasku zadan*/
#define BLUETOOTH_IKONA_X (EKRAN_SZEROKOSC - 46)
#define BLUETOOTH_IKONA_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 4)
#define BLUETOOTH_IKONA_W 24
#define BLUETOOTH_IKONA_H 22

/*rysuj ikone bluetooth na pasku zadan*/
void bluetooth_ui_draw_icon(int x, int y)
{
    uint32_t kolor = bluetooth_ikona_aktywna ? 0x000000FF : 0x00FFFFFF;
    rysuj_prostokat(x, y, BLUETOOTH_IKONA_W, BLUETOOTH_IKONA_H, 0x003A4B5C);
    int ix = x + (BLUETOOTH_IKONA_W - 12) / 2;
    int iy = y + (BLUETOOTH_IKONA_H - 12) / 2;
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++) {
            if (ikona_bluetooth_pasek[i][j]) {
                rysuj_piksel(ix + j, iy + i, kolor);
            }
        }
    }
    rysuj_prostokat(x, y, BLUETOOTH_IKONA_W, 1, 0x00FFFFFF);
    rysuj_prostokat(x, y, 1, BLUETOOTH_IKONA_H, 0x00FFFFFF);
    rysuj_prostokat(x + BLUETOOTH_IKONA_W - 1, y, 1, BLUETOOTH_IKONA_H, 0x00FFFFFF);
    rysuj_prostokat(x, y + BLUETOOTH_IKONA_H - 1, BLUETOOTH_IKONA_W, 1, 0x00FFFFFF);
}

/*sprawdz czy pozycja jest nad ikona bluetooth*/
int bluetooth_ui_icon_at(int x, int y)
{
    if (y < BLUETOOTH_IKONA_Y || y >= BLUETOOTH_IKONA_Y + BLUETOOTH_IKONA_H) return 0;
    if (x < BLUETOOTH_IKONA_X || x >= BLUETOOTH_IKONA_X + BLUETOOTH_IKONA_W) return 0;
    return 1;
}

/*obsluga klikniecia na ikone bluetooth*/
int bluetooth_ui_mouse_click(int x, int y)
{
    if (!bluetooth_ui_icon_at(x, y)) return 0;
    bluetooth_ui_toggle();
    return 1;
}

/*funkcja rysowania zawartosci okna parowania*/
static void okno_bluetooth_rysuj(int x, int y, int szerokosc, int wysokosc)
{
    (void)x;
    (void)y;
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_chicago(8, 8, "URZADZENIA BLUETOOTH", 0x00000000);
    rysuj_tekst_chicago(8, 20, "SLUCHAWKI:", 0x00000000);

    struct bt_context ctx;
    bluetooth_context_get(&ctx);

    int yy = 34;
    uint8_t count = bluetooth_device_count();
    for (uint8_t i = 0; i < count && yy < wysokosc - 30; i++) {
        const struct bt_device *dev = bluetooth_device_get(i);
        if (!dev) continue;
        rysuj_tekst_chicago(8, yy, dev->name, dev->connected ? 0x0000FF00 : 0x00000000);
        yy += 14;
    }

    if (count == 0) {
        rysuj_tekst_chicago(8, yy, "brak urzadzen", 0x00808080);
        yy += 14;
    }

    rysuj_tekst_chicago(8, yy, "KLIKNIJ URZADZENIE ABY POLACZYC", 0x00808080);
}

/*obsluga klikniecia w oknie parowania*/
static void okno_bluetooth_klik(int mx, int my)
{
    struct bt_context ctx;
    bluetooth_context_get(&ctx);

    int yy = 34;
    uint8_t count = bluetooth_device_count();
    for (uint8_t i = 0; i < count; i++) {
        const struct bt_device *dev = bluetooth_device_get(i);
        if (!dev) continue;
        if (my >= yy && my < yy + 14 && mx >= 8 && mx < 8 + 120) {
            if (!dev->connected) {
                bluetooth_pair((struct bt_device *)dev);
                bluetooth_connect((struct bt_device *)dev);
                if (dev->device_type == BT_DEVICE_HEADPHONES) {
                    bluetooth_a2dp_open((struct bt_device *)dev);
                }
            }
            return;
        }
        yy += 14;
    }
}

/*otworz lub zamknij okno parowania bluetooth*/
void bluetooth_ui_toggle(void)
{
    if (okno_bluetooth_otwarte && okno_bluetooth.otwarte) {
        okno_zamknij(&okno_bluetooth);
        okno_bluetooth_otwarte = 0;
        bluetooth_ikona_aktywna = 0;
    } else {
        okno_stworz(&okno_bluetooth, 300, 100, 300, 220,
                    "BLUETOOTH - PAROWANIE", okno_bluetooth_rysuj);
        okno_ustaw_obsluge_klikniecia(&okno_bluetooth, okno_bluetooth_klik);
        okno_bluetooth_otwarte = 1;
        bluetooth_ikona_aktywna = 1;
    }
}

/*inicjalizacja interfejsu uzytkownika bluetooth*/
void bluetooth_ui_init(void)
{
    okno_bluetooth_otwarte = 0;
    bluetooth_ikona_aktywna = 0;
}

/*rysuj ikone bluetooth na pasku zadan (wywolywane z narysuj_pasek_zadan)*/
void bluetooth_ui_rysuj_pasek(void)
{
    if (!bluetooth_is_ready()) return;
    bluetooth_ui_draw_icon(BLUETOOTH_IKONA_X, BLUETOOTH_IKONA_Y);
}

/*odswiez stan ikony bluetooth (aktywna gdy jest polaczenie lub skanowanie)*/
void bluetooth_ui_update_state(void)
{
    struct bt_context ctx;
    bluetooth_context_get(&ctx);
    bluetooth_ikona_aktywna = (ctx.state == BT_STATE_DISCOVERING) ||
                              (bluetooth_device_count() > 0);
}
