/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <user_land/okno_user.h>
#include <kernel/syscall.h>

#define KURSOR_ROZMIAR 16

static int kursor_x = 512;
static int kursor_y = 384;
static int ostatni_x = 512;
static int ostatni_y = 384;

static struct okno_user okno_kursor;

static void kursor_narysuj(int x, int y, int szerokosc, int wysokosc) {
    (void)x; (void)y; (void)szerokosc; (void)wysokosc;
}

static void kursor_klik(int mx, int my) {
    (void)mx; (void)my;
}

static void kursor_pole_tekstowe_enter(void) {
}

static void kursor_pole_tekstowe_wyjdz(void) {
}

static int kursor_pole_tekstowe_znak(char znak, uint8_t ctrl) {
    (void)znak; (void)ctrl;
    return 1;
}

static void rysuj_kursor(int x, int y) {
    for (int i = 0; i < KURSOR_ROZMIAR; i++) {
        for (int j = 0; j < KURSOR_ROZMIAR - i; j++) {
            if (x + i < 1024 && y + j < 768)
                rysuj_piksel_user(x + i, y + j, KOLOR_BIALY);
        }
    }
    for (int i = 0; i < KURSOR_ROZMIAR; i++) {
        for (int j = 0; j < KURSOR_ROZMIAR - i; j++) {
            if (x + i < 1024 && y + j < 768)
                rysuj_piksel_user(x + i, y + j, KOLOR_CZARNY);
        }
    }
}

int main(void) {
    okno_stworz_user(&okno_kursor, 0, 0, 1, 1, "KURSOR", kursor_narysuj);
    okno_ustaw_obsluge_klikniecia_user(&okno_kursor, kursor_klik);
    okno_ustaw_pole_tekstowe_enter_user(&okno_kursor, kursor_pole_tekstowe_enter);
    okno_ustaw_pole_tekstowe_wyjdz_user(&okno_kursor, kursor_pole_tekstowe_wyjdz);
    okno_ustaw_pole_tekstowe_znak_user(&okno_kursor, kursor_pole_tekstowe_znak);

    while (okno_czy_otwarte_user(&okno_kursor)) {
        int mx, my;
        mysz_pozycja_user(&mx, &my);

        if (mx != ostatni_x || my != ostatni_y) {
            rysuj_kursor(mx, my);
            ostatni_x = mx;
            ostatni_y = my;
        }

        for (int i = 0; i < 50000; i++) {
            __asm__ volatile("nop");
        }

        proces_yield_user();
    }

    return 0;
}
