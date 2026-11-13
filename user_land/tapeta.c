#include <graficzny/okno.h>
#include <graficzny/rysowanie.h>
#include <user_land/tapeta.h>

static struct okno okno;

static void tapeta_app_rysuj(int x, int y, int szerokosc, int wysokosc) {
    tapeta_picker_rysuj(x, y, szerokosc, wysokosc);
}

static void tapeta_app_klik(int x, int y)
{
    tapeta_picker_klik(x, y);
    okno_narysuj(&okno);
}

void tapeta_app(void)
{
    okno_stworz(&okno, 248, 80, 360, 170, "TAPETA", tapeta_app_rysuj);
    okno_ustaw_obsluge_klikniecia(&okno, tapeta_app_klik);
}
