/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <graficzny/rysowanie.h>
#include <graficzny/okno.h>
#include <graficzny/czcionka.h>

#define KAMERA_OKNO_SZEROKOSC 320
#define KAMERA_OKNO_WYSOKOSC 240

static struct okno okno_kamera;
static uint8_t okno_kamera_otwarte = 0;

static void okno_kamera_rysuj(int x, int y, int szerokosc, int wysokosc)
{
    extern unsigned char *quickcam_driver_get_frame(void);
    extern unsigned int quickcam_driver_get_width(void);
    extern unsigned int quickcam_driver_get_height(void);
    extern unsigned int quickcam_driver_get_frame_size(void);
    unsigned char *frame = quickcam_driver_get_frame();
    unsigned int w = quickcam_driver_get_width();
    unsigned int h = quickcam_driver_get_height();
    unsigned int fsize = quickcam_driver_get_frame_size();
    rysuj_prostokat(x, y, szerokosc, wysokosc, 0x00000000);
    if (!frame || fsize == 0 || w == 0 || h == 0) {
        rysuj_tekst_chicago(x + 8, y + 8, "BRAK SYGNAŁU", 0x00FFFFFF);
        rysuj_tekst_chicago(x + 8, y + 22, "KAMERY", 0x00FFFFFF);
        return;
    }
    for (int py = 0; py < wysokosc; py++) {
        for (int px = 0; px < szerokosc; px++) {
            unsigned int sx = (unsigned int)px * w / (unsigned int)szerokosc;
            unsigned int sy = (unsigned int)py * h / (unsigned int)wysokosc;
            unsigned int offset = (sy * w + sx) * 2;
            if (offset + 1 >= fsize) continue;
            unsigned char Y = frame[offset];
            unsigned char U = frame[offset + 1];
            int c = (int)Y - 16;
            int d = (int)U - 128;
            int r = (298 * c + 409 * d + 128) >> 8;
            int g = (298 * c - 100 * d - 208 * d + 128) >> 8;
            int b = (298 * c + 516 * d + 128) >> 8;
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);
            unsigned int kolor = ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
            rysuj_piksel(x + px, y + py, kolor);
        }
    }
    rysuj_tekst_chicago(x + 8, y + wysokosc - 18, "QUICKCAM EXPRESS", 0x00FFFFFF);
}

static void okno_kamera_klik(int mx, int my)
{
    (void)mx;
    (void)my;
}

void kamera_ui_init(void)
{
    okno_kamera_otwarte = 0;
}

void kamera_ui_toggle(void)
{
    if (okno_kamera_otwarte && okno_kamera.otwarte) {
        okno_zamknij(&okno_kamera);
        okno_kamera_otwarte = 0;
    } else {
        okno_stworz(&okno_kamera, 100, 100, KAMERA_OKNO_SZEROKOSC, KAMERA_OKNO_WYSOKOSC,
                    "KAMERA - QUICKCAM EXPRESS", okno_kamera_rysuj);
        okno_ustaw_obsluge_klikniecia(&okno_kamera, okno_kamera_klik);
        okno_kamera_otwarte = 1;
    }
}
