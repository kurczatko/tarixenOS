#include "okno_user.h"
#include "libc/syscall.h"

okno_user_t *obecne_okno = 0;
okno_user_t *aktywne_okno = 0;
okno_user_t *okna_lista[OKNA_MAKS] = {0};
int okna_liczba = 0;

int przeciaganie = 0;
int przesuniecie_x = 0;
int przesuniecie_y = 0;
int ramka_x = 0;
int ramka_y = 0;
int ramka_widoczna = 0;

int zmienianie_rozmiaru = 0;
int kierunek_zmiany = 0;

static char schowek[256] = {0};
static int schowek_dlugosc = 0;

static void okno_dodaj_do_listy(okno_user_t *okno) {
    if (okna_liczba < OKNA_MAKS) {
        okna_lista[okna_liczba++] = okno;
    }
}

static void okno_usun_z_listy(okno_user_t *okno) {
    for (int i = 0; i < okna_liczba; i++) {
        if (okna_lista[i] == okno) {
            for (int j = i; j < okna_liczba - 1; j++) {
                okna_lista[j] = okna_lista[j + 1];
            }
            okna_lista[okna_liczba - 1] = 0;
            okna_liczba--;
            break;
        }
    }
}

static int okno_przycisk_min(int okno_x, int okno_y, int szerokosc) {
    return okno_x + szerokosc - 3 * PRZYCISK_ROZMIAR - 4;
}

static int okno_przycisk_max(int okno_x, int okno_y, int szerokosc) {
    return okno_x + szerokosc - 2 * PRZYCISK_ROZMIAR - 2;
}

static int okno_przycisk_zamknij(int okno_x, int okno_y, int szerokosc) {
    return okno_x + szerokosc - PRZYCISK_ROZMIAR;
}

static int okno_obszar_paska(int x, int y, okno_user_t *okno) {
    return (x >= okno->x && x < okno->x + okno->szerokosc &&
            y >= okno->y && y < okno->y + PASKA_WYSOKOSC);
}

static int okno_obszar_ramka(int x, int y, okno_user_t *okno, int *kierunek) {
    if (okno->zmaksymalizowane) return 0;
    
    int g = RAMKA_GRUBOSC;
    int l = okno->x - g;
    int p = okno->x + okno->szerokosc;
    int gora = okno->y - g;
    int dol = okno->y + okno->wysokosc;

    if (x >= l && x < okno->x && y >= gora && y < okno->y) { *kierunek = 1; return 1; }
    if (x >= p && x < p + g && y >= gora && y < okno->y) { *kierunek = 2; return 1; }
    if (x >= l && x < okno->x && y >= dol && y < dol + g) { *kierunek = 3; return 1; }
    if (x >= p && x < p + g && y >= dol && y < dol + g) { *kierunek = 4; return 1; }
    if (x >= l && x < okno->x && y >= okno->y && y < okno->y + okno->wysokosc) { *kierunek = 5; return 1; }
    if (x >= p && x < p + g && y >= okno->y && y < okno->y + okno->wysokosc) { *kierunek = 6; return 1; }
    if (y >= gora && y < okno->y && x >= okno->x && x < okno->x + okno->szerokosc) { *kierunek = 7; return 1; }
    if (y >= dol && y < dol + g && x >= okno->x && x < okno->x + okno->szerokosc) { *kierunek = 8; return 1; }
    return 0;
}

static void rysuj_ramke_okna(okno_user_t *okno) {
    uint32_t kolor_ramki = KOLOR_RAMKI;
    uint32_t kolor_paska = (okno == aktywne_okno) ? KOLOR_PASKA_AKTYWNE : KOLOR_PASKA;
    uint32_t kolor_tlo = KOLOR_OKNA;

    int x = okno->x;
    int y = okno->y;
    int w = okno->szerokosc;
    int h = okno->wysokosc;
    int g = RAMKA_GRUBOSC;

    rysuj_prostokat_ekran_user(x - g, y - g, w + 2 * g, h + 2 * g, kolor_ramki);
    rysuj_prostokat_ekran_user(x, y, w, h, kolor_tlo);
    rysuj_prostokat_ekran_user(x, y, w, PASKA_WYSOKOSC, kolor_paska);

    if (okno->tytul) {
        rysuj_tekst_ekran_user(x + 8, y + 4, okno->tytul, KOLOR_BIALY);
    }

    int btn_min = okno_przycisk_min(x, y, w);
    int btn_max = okno_przycisk_max(x, y, w);
    int btn_close = okno_przycisk_zamknij(x, y, w);
    int btn_y = y + (PASKA_WYSOKOSC - PRZYCISK_ROZMIAR) / 2;

    rysuj_prostokat_ekran_user(btn_min, btn_y, PRZYCISK_ROZMIAR, PRZYCISK_ROZMIAR, KOLOR_PRZYCISK_MIN);
    rysuj_prostokat_ekran_user(btn_max, btn_y, PRZYCISK_ROZMIAR, PRZYCISK_ROZMIAR, KOLOR_PRZYCISK_MAX);
    rysuj_prostokat_ekran_user(btn_close, btn_y, PRZYCISK_ROZMIAR, PRZYCISK_ROZMIAR, KOLOR_PRZYCISK_ZAMKNIJ);

    rysuj_tekst_ekran_user(btn_min + 4, btn_y + 2, "_", KOLOR_BIALY);
    rysuj_tekst_ekran_user(btn_max + 4, btn_y + 2, okno->zmaksymalizowane ? "#" : "[ ]", KOLOR_BIALY);
    rysuj_tekst_ekran_user(btn_close + 4, btn_y + 2, "X", KOLOR_BIALY);
}

static void rysuj_ramke_przeciaganie(int x, int y, int w, int h) {
    if (ramka_widoczna) {
        rysuj_prostokat_ekran_user(ramka_x, ramka_y, ramka_x + ramka_x + w, ramka_y + ramka_y + h, KOLOR_SZARY);
    }
    ramka_x = x;
    ramka_y = y;
    ramka_widoczna = 1;
    rysuj_prostokat_ekran_user(x, y, w, h, KOLOR_SZARY);
}

okno_handle_t okno_stworz_user(okno_user_t *okno, int x, int y, int w, int h, const char *tytul, void (*rysuj_zawartosc)(int x, int y, int w, int h)) {
    okno_handle_t handle = (okno_handle_t)libc_syscall5(SYSCALL_OKNO_STWORZ, (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h, (uint32_t)tytul);
    
    okno->handle = handle;
    okno->x = x;
    okno->y = y;
    okno->szerokosc = w;
    okno->wysokosc = h;
    okno->x_poprzednie = x;
    okno->y_poprzednie = y;
    okno->szerokosc_poprzednia = w;
    okno->wysokosc_poprzednia = h;
    okno->tytul = tytul;
    okno->rysuj_zawartosc = rysuj_zawartosc;
    okno->obsluz_klik = 0;
    okno->pole_tekstowe_enter = 0;
    okno->pole_tekstowe_wyjdz = 0;
    okno->pole_tekstowe_znak = 0;
    okno->tekst_bufor = 0;
    okno->tekst_dlugosc = 0;
    okno->tekst_maks = 0;
    okno->otwarte = 1;
    okno->zminimalizowane = 0;
    okno->zmaksymalizowane = 0;
    okno->w_polu_tekstowym = 0;
    okno->kursor_pozycja = 0;
    okno->zaznacz_poczatek = 0;
    okno->zaznacz_koniec = 0;

    okno_dodaj_do_listy(okno);
    okno_ustaw_aktywne_user(okno);
    obecne_okno = okno;
    
    return handle;
}

void okno_rysuj_user(okno_user_t *okno) {
    if (!okno->otwarte || okno->zminimalizowane) return;
    
    libc_syscall3(SYSCALL_OKNO_RYSUJ, (uint32_t)okno->handle, 0, 0);
    rysuj_ramke_okna(okno);
    
    if (okno->rysuj_zawartosc) {
        int cx = okno->x + RAMKA_GRUBOSC;
        int cy = okno->y + PASKA_WYSOKOSC + RAMKA_GRUBOSC;
        int cw = okno->szerokosc - 2 * RAMKA_GRUBOSC;
        int ch = okno->wysokosc - PASKA_WYSOKOSC - 2 * RAMKA_GRUBOSC;
        okno->rysuj_zawartosc(cx, cy, cw, ch);
    }
}

void okno_zamknij_user(okno_user_t *okno) {
    if (!okno->otwarte) return;
    libc_syscall3(SYSCALL_OKNO_ZAMKNIJ, (uint32_t)okno->handle, 0, 0);
    okno->otwarte = 0;
    okno_usun_z_listy(okno);
    if (aktywne_okno == okno) {
        aktywne_okno = (okna_liczba > 0) ? okna_lista[okna_liczba - 1] : 0;
    }
    if (obecne_okno == okno) {
        obecne_okno = 0;
    }
}

int okno_czy_otwarte_user(okno_user_t *okno) {
    return okno->otwarte;
}

void okno_ustaw_aktywne_user(okno_user_t *okno) {
    if (aktywne_okno == okno) return;
    aktywne_okno = okno;
    for (int i = 0; i < okna_liczba - 1; i++) {
        if (okna_lista[i] == okno) {
            for (int j = i; j < okna_liczba - 1; j++) {
                okna_lista[j] = okna_lista[j + 1];
            }
            okna_lista[okna_liczba - 1] = okno;
            break;
        }
    }
}

int okno_czy_aktywne_user(okno_user_t *okno) {
    return aktywne_okno == okno;
}

void okno_zminimalizuj_user(okno_user_t *okno) {
    if (!okno->otwarte || okno->zminimalizowane) return;
    okno->zminimalizowane = 1;
    if (aktywne_okno == okno) {
        aktywne_okno = (okna_liczba > 1) ? okna_lista[okna_liczba - 2] : 0;
    }
}

void okno_przywroc_user(okno_user_t *okno) {
    if (!okno->otwarte || !okno->zminimalizowane) return;
    okno->zminimalizowane = 0;
    okno_ustaw_aktywne_user(okno);
}

void okno_maksymalizuj_user(okno_user_t *okno) {
    if (!okno->otwarte || okno->zmaksymalizowane) return;
    okno->x_poprzednie = okno->x;
    okno->y_poprzednie = okno->y;
    okno->szerokosc_poprzednia = okno->szerokosc;
    okno->wysokosc_poprzednia = okno->wysokosc;
    okno->x = RAMKA_GRUBOSC;
    okno->y = RAMKA_GRUBOSC;
    okno->szerokosc = EKRAN_SZEROKOSC - 2 * RAMKA_GRUBOSC;
    okno->wysokosc = EKRAN_WYSOKOSC - 2 * RAMKA_GRUBOSC;
    okno->zmaksymalizowane = 1;
}

void okno_przywroc_rozmiar_user(okno_user_t *okno) {
    if (!okno->otwarte || !okno->zmaksymalizowane) return;
    okno->x = okno->x_poprzednie;
    okno->y = okno->y_poprzednie;
    okno->szerokosc = okno->szerokosc_poprzednia;
    okno->wysokosc = okno->wysokosc_poprzednia;
    okno->zmaksymalizowane = 0;
}

int okno_czy_zminimalizowane_user(okno_user_t *okno) {
    return okno->zminimalizowane;
}

int okno_czy_zmaksymalizowane_user(okno_user_t *okno) {
    return okno->zmaksymalizowane;
}

void okno_przetworz_zdarzenia_user(void) {
    int mx, my;
    mysz_pozycja_user(&mx, &my);
    char znak;
    uint8_t ctrl;
    
    if (klawiatura_getchar_user(&znak, &ctrl)) {
        if (aktywne_okno && aktywne_okno->w_polu_tekstowym && aktywne_okno->tekst_bufor) {
            if (aktywne_okno->pole_tekstowe_znak) {
                aktywne_okno->pole_tekstowe_znak(znak, ctrl);
            }
        } else if (ctrl && znak == 'c') {
            if (aktywne_okno) okno_skopiuj_zaznaczenie_user(aktywne_okno);
        } else if (ctrl && znak == 'v') {
            if (aktywne_okno) okno_wklej_schowek_user(aktywne_okno);
        } else if (ctrl && znak == 'a') {
            if (aktywne_okno && aktywne_okno->tekst_bufor) {
                okno_zaznacz_tekst_user(aktywne_okno, 0, aktywne_okno->tekst_dlugosc);
            }
        }
    }
    
    okno_obsluz_ruch_user(mx, my);
}

int okno_obsluz_klikniecie_user(int x, int y) {
    if (!aktywne_okno) return 0;
    
    okno_user_t *okno = aktywne_okno;
    
    if (okno_obszar_paska(x, y, okno)) {
        int btn_min = okno_przycisk_min(okno->x, okno->y, okno->szerokosc);
        int btn_max = okno_przycisk_max(okno->x, okno->y, okno->szerokosc);
        int btn_close = okno_przycisk_zamknij(okno->x, okno->y, okno->szerokosc);
        int btn_y = okno->y + (PASKA_WYSOKOSC - PRZYCISK_ROZMIAR) / 2;
        
        if (x >= btn_close && x < btn_close + PRZYCISK_ROZMIAR && 
            y >= btn_y && y < btn_y + PRZYCISK_ROZMIAR) {
            okno_zamknij_user(okno);
            return 1;
        }
        if (x >= btn_min && x < btn_min + PRZYCISK_ROZMIAR && 
            y >= btn_y && y < btn_y + PRZYCISK_ROZMIAR) {
            okno_zminimalizuj_user(okno);
            return 1;
        }
        if (x >= btn_max && x < btn_max + PRZYCISK_ROZMIAR && 
            y >= btn_y && y < btn_y + PRZYCISK_ROZMIAR) {
            if (okno->zmaksymalizowane) {
                okno_przywroc_rozmiar_user(okno);
            } else {
                okno_maksymalizuj_user(okno);
            }
            return 1;
        }
        
        przeciaganie = 1;
        przesuniecie_x = x - okno->x;
        przesuniecie_y = y - okno->y;
        return 1;
    }
    
    int kierunek;
    if (okno_obszar_ramka(x, y, okno, &kierunek)) {
        zmienianie_rozmiaru = 1;
        kierunek_zmiany = kierunek;
        return 1;
    }
    
    if (x >= okno->x + RAMKA_GRUBOSC && x < okno->x + okno->szerokosc - RAMKA_GRUBOSC &&
        y >= okno->y + PASKA_WYSOKOSC + RAMKA_GRUBOSC && y < okno->y + okno->wysokosc - RAMKA_GRUBOSC) {
        if (okno->obsluz_klik) {
            okno->obsluz_klik(x - okno->x - RAMKA_GRUBOSC, y - okno->y - PASKA_WYSOKOSC - RAMKA_GRUBOSC);
        }
        if (okno->tekst_bufor) {
            okno_wejdz_do_pola_tekstowego_user(okno);
        }
        return 1;
    }
    
    return 0;
}

void okno_obsluz_ruch_user(int x, int y) {
    if (przeciaganie && aktywne_okno && !aktywne_okno->zmaksymalizowane && !aktywne_okno->zminimalizowane) {
        if (!ramka_widoczna) {
            ramka_x = aktywne_okno->x;
            ramka_y = aktywne_okno->y;
            ramka_widoczna = 1;
        }
        rysuj_ramke_przeciaganie(x - przesuniecie_x, y - przesuniecie_y, aktywne_okno->szerokosc, aktywne_okno->wysokosc);
    } else if (zmienianie_rozmiaru && aktywne_okno && !aktywne_okno->zmaksymalizowane && !aktywne_okno->zminimalizowane) {
        int nx = aktywne_okno->x;
        int ny = aktywne_okno->y;
        int nw = aktywne_okno->szerokosc;
        int nh = aktywne_okno->wysokosc;
        
        switch (kierunek_zmiany) {
            case 1: nw = aktywne_okno->x + aktywne_okno->szerokosc - x; nx = x; nh = aktywne_okno->y + aktywne_okno->wysokosc - y; ny = y; break;
            case 2: nw = x - aktywne_okno->x; nh = aktywne_okno->y + aktywne_okno->wysokosc - y; ny = y; break;
            case 3: nw = aktywne_okno->x + aktywne_okno->szerokosc - x; nx = x; nh = y - aktywne_okno->y; break;
            case 4: nw = x - aktywne_okno->x; nh = y - aktywne_okno->y; break;
            case 5: nw = aktywne_okno->x + aktywne_okno->szerokosc - x; nx = x; break;
            case 6: nw = x - aktywne_okno->x; break;
            case 7: nh = aktywne_okno->y + aktywne_okno->wysokosc - y; ny = y; break;
            case 8: nh = y - aktywne_okno->y; break;
        }
        
        if (nw < 100) nw = 100;
        if (nh < 50) nh = 50;
        
        if (!ramka_widoczna) {
            ramka_x = nx;
            ramka_y = ny;
            ramka_widoczna = 1;
        }
        rysuj_ramke_przeciaganie(nx, ny, nw, nh);
    }
}

void okno_zakoncz_przeciaganie_user(void) {
    if (przeciaganie && aktywne_okno) {
        aktywne_okno->x = ramka_x;
        aktywne_okno->y = ramka_y;
        przeciaganie = 0;
        ramka_widoczna = 0;
        okno_rysuj_user(aktywne_okno);
    } else if (zmienianie_rozmiaru && aktywne_okno) {
        aktywne_okno->x = ramka_x;
        aktywne_okno->y = ramka_y;
        aktywne_okno->szerokosc = ramka_x + aktywne_okno->szerokosc - ramka_x;
        aktywne_okno->wysokosc = ramka_y + aktywne_okno->wysokosc - ramka_y;
        zmienianie_rozmiaru = 0;
        kierunek_zmiany = 0;
        ramka_widoczna = 0;
        okno_rysuj_user(aktywne_okno);
    }
    przeciaganie = 0;
    zmienianie_rozmiaru = 0;
    ramka_widoczna = 0;
}

void okno_ustaw_obsluge_klikniecia_user(okno_user_t *okno, okno_klik_fn fn) {
    okno->obsluz_klik = fn;
}

void okno_ustaw_pole_tekstowe_enter_user(okno_user_t *okno, okno_pole_tekstowe_fn fn) {
    okno->pole_tekstowe_enter = fn;
}

void okno_ustaw_pole_tekstowe_wyjdz_user(okno_user_t *okno, okno_pole_tekstowe_fn fn) {
    okno->pole_tekstowe_wyjdz = fn;
}

void okno_ustaw_pole_tekstowe_znak_user(okno_user_t *okno, okno_pole_tekstowe_znak_fn fn) {
    okno->pole_tekstowe_znak = fn;
}

void okno_ustaw_bufor_tekstu_user(okno_user_t *okno, char *bufor, int dlugosc, int maks) {
    okno->tekst_bufor = bufor;
    okno->tekst_dlugosc = dlugosc;
    okno->tekst_maks = maks;
}

int okno_czy_w_polu_tekstowym_user(okno_user_t *okno) {
    return okno->w_polu_tekstowym;
}

int okno_pobierz_kursor_user(okno_user_t *okno) {
    return okno->kursor_pozycja;
}

void okno_ustaw_kursor_user(okno_user_t *okno, int pozycja) {
    if (pozycja < 0) pozycja = 0;
    if (pozycja > okno->tekst_dlugosc) pozycja = okno->tekst_dlugosc;
    okno->kursor_pozycja = pozycja;
}

void okno_zaznacz_tekst_user(okno_user_t *okno, int poczatek, int koniec) {
    if (poczatek < 0) poczatek = 0;
    if (koniec > okno->tekst_dlugosc) koniec = okno->tekst_dlugosc;
    if (poczatek > koniec) { int t = poczatek; poczatek = koniec; koniec = t; }
    okno->zaznacz_poczatek = poczatek;
    okno->zaznacz_koniec = koniec;
}

void okno_pobierz_zaznaczenie_user(okno_user_t *okno, int *poczatek, int *koniec) {
    *poczatek = okno->zaznacz_poczatek;
    *koniec = okno->zaznacz_koniec;
}

int okno_skopiuj_zaznaczenie_user(okno_user_t *okno) {
    if (!okno->tekst_bufor || okno->zaznacz_poczatek == okno->zaznacz_koniec) return 0;
    int len = okno->zaznacz_koniec - okno->zaznacz_poczatek;
    if (len > 255) len = 255;
    for (int i = 0; i < len; i++) {
        schowek[i] = okno->tekst_bufor[okno->zaznacz_poczatek + i];
    }
    schowek[len] = 0;
    schowek_dlugosc = len;
    return 1;
}

int okno_wklej_schowek_user(okno_user_t *okno) {
    if (!okno->tekst_bufor || schowek_dlugosc == 0) return 0;
    int dostepne = okno->tekst_maks - okno->tekst_dlugosc - 1;
    if (dostepne <= 0) return 0;
    int wklej = schowek_dlugosc;
    if (wklej > dostepne) wklej = dostepne;
    int pos = okno->kursor_pozycja;
    for (int i = okno->tekst_dlugosc; i >= pos; i--) {
        okno->tekst_bufor[i + wklej] = okno->tekst_bufor[i];
    }
    for (int i = 0; i < wklej; i++) {
        okno->tekst_bufor[pos + i] = schowek[i];
    }
    okno->tekst_dlugosc += wklej;
    okno->tekst_bufor[okno->tekst_dlugosc] = 0;
    okno->kursor_pozycja = pos + wklej;
    okno->zaznacz_poczatek = okno->kursor_pozycja;
    okno->zaznacz_koniec = okno->kursor_pozycja;
    return 1;
}

void okno_wejdz_do_pola_tekstowego_user(okno_user_t *okno) {
    okno->w_polu_tekstowym = 1;
    okno->kursor_pozycja = okno->tekst_dlugosc;
    okno->zaznacz_poczatek = okno->kursor_pozycja;
    okno->zaznacz_koniec = okno->kursor_pozycja;
    if (okno->pole_tekstowe_enter) okno->pole_tekstowe_enter();
}

void okno_wyjdz_z_pola_tekstowego_user(okno_user_t *okno) {
    okno->w_polu_tekstowym = 0;
    if (okno->pole_tekstowe_wyjdz) okno->pole_tekstowe_wyjdz();
}

void rysuj_piksel_user(okno_user_t *okno, int x, int y, uint32_t color) {
    if (!okno->otwarte) return;
    libc_syscall5(SYSCALL_OKNO_PIKSEL, (uint32_t)okno->handle, (uint32_t)x, (uint32_t)y, color, 0);
}

void rysuj_prostokat_user(okno_user_t *okno, int x, int y, int w, int h, uint32_t color) {
    if (!okno->otwarte) return;
    libc_syscall5(SYSCALL_OKNO_PROSTOKAT, (uint32_t)okno->handle, (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h);
    (void)color;
}

void rysuj_tekst_user(okno_user_t *okno, int x, int y, const char *text, uint32_t color) {
    if (!okno->otwarte) return;
    libc_syscall5(SYSCALL_OKNO_TEKST, (uint32_t)okno->handle, (uint32_t)x, (uint32_t)y, (uint32_t)text, color);
}

void rysuj_znak_user(okno_user_t *okno, int x, int y, char znak, uint32_t color) {
    char buf[2] = {znak, 0};
    rysuj_tekst_user(okno, x, y, buf, color);
}

void rysuj_piksel_ekran_user(int x, int y, uint32_t color) {
    libc_syscall5(SYSCALL_EKRAN_PIKSEL, (uint32_t)x, (uint32_t)y, color, 0, 0);
}

void rysuj_prostokat_ekran_user(int x, int y, int w, int h, uint32_t color) {
    libc_syscall5(SYSCALL_EKRAN_PROSTOKAT, (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h, color);
}

void rysuj_tekst_ekran_user(int x, int y, const char *text, uint32_t color) {
    libc_syscall5(SYSCALL_TEKST_RYSUJ, (uint32_t)x, (uint32_t)y, (uint32_t)text, color, 0);
}

okno_handle_t okno_stworz(int x, int y, int w, int h, const char *tytul, okno_rysuj_zawartosc_fn rysuj_zawartosc) {
    static okno_user_t okno;
    return okno_stworz_user(&okno, x, y, w, h, tytul, rysuj_zawartosc);
}

void okno_rysuj(void) {
    if (obecne_okno) okno_rysuj_user(obecne_okno);
}

void okno_zamknij(void) {
    if (obecne_okno) okno_zamknij_user(obecne_okno);
}

int okno_czy_otwarte(void) {
    return obecne_okno ? okno_czy_otwarte_user(obecne_okno) : 0;
}

void okno_ustaw_obsluge_klikniecia(okno_klik_fn fn) {
    if (obecne_okno) okno_ustaw_obsluge_klikniecia_user(obecne_okno, fn);
}

void okno_ustaw_pole_tekstowe_enter(okno_pole_tekstowe_fn fn) {
    if (obecne_okno) okno_ustaw_pole_tekstowe_enter_user(obecne_okno, fn);
}

void okno_ustaw_pole_tekstowe_wyjdz(okno_pole_tekstowe_fn fn) {
    if (obecne_okno) okno_ustaw_pole_tekstowe_wyjdz_user(obecne_okno, fn);
}

void okno_ustaw_pole_tekstowe_znak(okno_pole_tekstowe_znak_fn fn) {
    if (obecne_okno) okno_ustaw_pole_tekstowe_znak_user(obecne_okno, fn);
}

void okno_ustaw_bufor_tekstu(char *bufor, int dlugosc, int maks) {
    if (obecne_okno) okno_ustaw_bufor_tekstu_user(obecne_okno, bufor, dlugosc, maks);
}

void rysuj_piksel(int x, int y, uint32_t color) {
    if (obecne_okno) rysuj_piksel_user(obecne_okno, x, y, color);
}

void rysuj_prostokat(int x, int y, int w, int h, uint32_t color) {
    if (obecne_okno) rysuj_prostokat_user(obecne_okno, x, y, w, h, color);
}

void rysuj_tekst(int x, int y, const char *text, uint32_t color) {
    if (obecne_okno) rysuj_tekst_user(obecne_okno, x, y, text, color);
}

void rysuj_znak(int x, int y, char znak, uint32_t color) {
    if (obecne_okno) rysuj_znak_user(obecne_okno, x, y, znak, color);
}

int klawiatura_getchar_user(char *znak, uint8_t *ctrl) {
    uint32_t ret = libc_syscall3(SYSCALL_KLAWIATURA_GETCHAR, 0, 0, 0);
    if (ret == 0xFFFFFFFF) return 0;
    *znak = (char)(ret & 0xFF);
    if (ctrl) *ctrl = (uint8_t)((ret >> 8) & 0xFF);
    return 1;
}

int mysz_pozycja_user(int *x, int *y) {
    uint32_t ret = libc_syscall3(SYSCALL_MYSZ_POZYCJA, 0, 0, 0);
    if (x) *x = (int)(ret & 0xFFFF);
    if (y) *y = (int)(ret >> 16);
    return 0;
}

void proces_yield_user(void) {
    libc_syscall3(SYSCALL_PROCES_YIELD, 0, 0, 0);
}

int okno_liczba_okien_user(void) {
    return okna_liczba;
}

okno_user_t *okno_pobierz_okno_user(int indeks) {
    if (indeks >= 0 && indeks < okna_liczba) {
        return okna_lista[indeks];
    }
    return 0;
}
