#include "../okno_user.h"
#include "syscall.h"
#include "syscall.h"

#define NOTATNIK_PLIK "NOTATKA.NTF"
#define NOTATNIK_MAKS 512
#define NOTATNIK_SZEROKOSC 640
#define NOTATNIK_WYSOKOSC 440
#define NOTATNIK_PRZYCISK_Y 370

#define KOLOR_TEKST_NOTATKI   0x00334155
#define KOLOR_PRZYCISKU       0x004A5D6E
#define KOLOR_TEKST_PRZYCISKU 0x00FFFFFF

static struct okno_user okno_notatnik;
static char notatka[NOTATNIK_MAKS + 1];
static uint8_t notatka_rozmiar[NOTATNIK_MAKS + 1];
static uint8_t notatka_styl[NOTATNIK_MAKS + 1];
static uint8_t notatnik_biezacy_rozmiar = 1;
static uint8_t notatnik_biezacy_styl = 0;
static char notatnik_plik[16] = NOTATNIK_PLIK;
static int notatka_dlugosc = 0;
static char notatnik_status[32] = "gotowy";
static uint8_t notatnik_tryb_tekstowy = 0;

static int notatnik_ma_rozszerzenie(const char *nazwa, const char *rozszerzenie) {
    int dlugosc = 0;
    int rozszerzenie_dlugosc = 0;
    while (nazwa[dlugosc]) dlugosc++;
    while (rozszerzenie[rozszerzenie_dlugosc]) rozszerzenie_dlugosc++;
    if (dlugosc < rozszerzenie_dlugosc) return 0;
    for (int i = 0; i < rozszerzenie_dlugosc; i++) {
        char znak = nazwa[dlugosc - rozszerzenie_dlugosc + i];
        char oczekiwany = rozszerzenie[i];
        if (znak >= 'a' && znak <= 'z') znak -= 'a' - 'A';
        if (oczekiwany >= 'a' && oczekiwany <= 'z') oczekiwany -= 'a' - 'A';
        if (znak != oczekiwany) return 0;
    }
    return 1;
}

static void notatnik_ustaw_tryb_tekstowy(void) {
    notatnik_tryb_tekstowy = notatnik_ma_rozszerzenie(notatnik_plik, ".TXT") ? 1 : 0;
}

static void notatnik_ustaw_status(const char *tekst) {
    int indeks = 0;
    while (tekst[indeks] && indeks < 31) {
        notatnik_status[indeks] = tekst[indeks];
        indeks++;
    }
    notatnik_status[indeks] = 0;
}

static void notatnik_narysuj(int x, int y, int szerokosc, int wysokosc) {
    int indeks;
    int linia = 0;
    int line_x = x;
    int cena;
    (void)wysokosc;
    for (indeks = 0; indeks < notatka_dlugosc; indeks++) {
        if (notatka[indeks] == '\n') {
            linia++;
            line_x = x;
            continue;
        }
        cena = (notatka_rozmiar[indeks] == 0) ? 4 : (notatka_rozmiar[indeks] == 1 ? 8 : 12);
        if (notatka_styl[indeks] & 4) cena += 2;
        if (line_x + cena > x + szerokosc - 16) {
            linia++;
            line_x = x;
        }
        if (y + linia * 16 + 16 > y + NOTATNIK_PRZYCISK_Y) continue;
        rysuj_tekst(line_x, y + linia * 16, &notatka[indeks], KOLOR_TEKST_NOTATKI);
        line_x += cena;
    }
    if (notatnik_tryb_tekstowy) {
        rysuj_tekst(x + 490, y + NOTATNIK_PRZYCISK_Y + 4, notatnik_status, KOLOR_TEKST_PRZYCISKU);
    } else {
        rysuj_prostokat(x + 8, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 12, y + NOTATNIK_PRZYCISK_Y + 4, "KURSA", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 60, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 64, y + NOTATNIK_PRZYCISK_Y + 4, "POGR", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 112, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 116, y + NOTATNIK_PRZYCISK_Y + 4, "CIEN", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 164, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 168, y + NOTATNIK_PRZYCISK_Y + 4, "MALY", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 216, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 220, y + NOTATNIK_PRZYCISK_Y + 4, "NOR", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 268, y + NOTATNIK_PRZYCISK_Y, 44, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 272, y + NOTATNIK_PRZYCISK_Y + 4, "DUZY", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 320, y + NOTATNIK_PRZYCISK_Y, 80, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 324, y + NOTATNIK_PRZYCISK_Y + 4, "ODCZYTAJ", KOLOR_TEKST_PRZYCISKU);
        rysuj_prostokat(x + 410, y + NOTATNIK_PRZYCISK_Y, 70, 20, KOLOR_PRZYCISKU);
        rysuj_tekst(x + 414, y + NOTATNIK_PRZYCISK_Y + 4, "ZAPISZ", KOLOR_TEKST_PRZYCISKU);
        rysuj_tekst(x + 490, y + NOTATNIK_PRZYCISK_Y + 4, notatnik_status, KOLOR_TEKST_PRZYCISKU);
    }
}

static void notatnik_odczytaj(void) {
    int i;
    int odczyt;
    char buf[NOTATNIK_MAKS + 8];
    for (i = 0; i <= NOTATNIK_MAKS; i++) {
        notatka[i] = 0;
        notatka_rozmiar[i] = 1;
        notatka_styl[i] = 0;
    }
    notatka_dlugosc = 0;
    int fd = syscall1(SYSCALL_OPEN, (uint32_t)notatnik_plik);
    if (fd < 0) {
        notatka_dlugosc = 0;
        notatka[0] = 0;
        notatnik_ustaw_status("brak pliku");
        return;
    }
    odczyt = syscall3(SYSCALL_READ, fd, (uint32_t)buf, sizeof(buf));
    syscall1(SYSCALL_CLOSE, fd);
    if (odczyt < 0) {
        notatnik_ustaw_status("blad odczytu");
        return;
    }
    if (notatnik_tryb_tekstowy) {
        int count = odczyt;
        if (count > NOTATNIK_MAKS) count = NOTATNIK_MAKS;
        for (i = 0; i < count; i++) {
            notatka[i] = buf[i];
            notatka_rozmiar[i] = 1;
            notatka_styl[i] = 0;
        }
        notatka_dlugosc = count;
        notatka[notatka_dlugosc] = 0;
        notatnik_ustaw_status("odczytano txt");
        return;
    }
    if (odczyt < 6 || buf[0] != 'N' || buf[1] != 'T' || buf[2] != 'F') {
        notatnik_ustaw_status("zly format");
        return;
    }
    int count = (unsigned char)buf[4] | ((unsigned char)buf[5] << 8);
    if (count > NOTATNIK_MAKS) count = NOTATNIK_MAKS;
    if (6 + count * 3 > odczyt) count = (odczyt - 6) / 3;
    for (i = 0; i < count; i++) {
        notatka[i] = buf[6 + i * 3];
        notatka_rozmiar[i] = buf[6 + i * 3 + 1];
        notatka_styl[i] = buf[6 + i * 3 + 2];
    }
    notatka_dlugosc = count;
    notatka[notatka_dlugosc] = 0;
    notatnik_ustaw_status("odczytano");
}

static void notatnik_zapisz(void) {
    if (notatnik_tryb_tekstowy) {
        int fd = syscall1(SYSCALL_OPEN, (uint32_t)notatnik_plik);
        if (fd < 0) {
            fd = syscall3(SYSCALL_OPEN, (uint32_t)notatnik_plik, 0, 0);
        }
        if (fd >= 0) {
            syscall3(SYSCALL_WRITE_FILE, fd, (uint32_t)notatka, notatka_dlugosc);
            syscall1(SYSCALL_CLOSE, fd);
        }
        notatnik_ustaw_status("zapisano txt");
        return;
    }
    char buf[NOTATNIK_MAKS * 3 + 8];
    int i;
    int pos = 0;
    buf[pos++] = 'N';
    buf[pos++] = 'T';
    buf[pos++] = 'F';
    buf[pos++] = 0;
    buf[pos++] = (unsigned char)(notatka_dlugosc & 0xFF);
    buf[pos++] = (unsigned char)(notatka_dlugosc >> 8);
    for (i = 0; i < notatka_dlugosc; i++) {
        buf[pos++] = notatka[i];
        buf[pos++] = notatka_rozmiar[i];
        buf[pos++] = notatka_styl[i];
    }
    int fd = syscall1(SYSCALL_OPEN, (uint32_t)notatnik_plik);
    if (fd < 0) {
        fd = syscall3(SYSCALL_OPEN, (uint32_t)notatnik_plik, 0, 0);
    }
    if (fd >= 0) {
        syscall3(SYSCALL_WRITE_FILE, fd, (uint32_t)buf, pos);
        syscall1(SYSCALL_CLOSE, fd);
    }
    notatnik_ustaw_status("zapisano");
}

static void notatnik_klik(int mx, int my) {
    int cx = mx;
    int cy = my;
    if (!notatnik_tryb_tekstowy && cy >= NOTATNIK_PRZYCISK_Y && cy < NOTATNIK_PRZYCISK_Y + 20) {
        if (cx >= 8 && cx < 52) notatnik_biezacy_styl ^= 2;
        if (cx >= 60 && cx < 104) notatnik_biezacy_styl ^= 1;
        if (cx >= 112 && cx < 156) notatnik_biezacy_styl ^= 4;
        if (cx >= 164 && cx < 208) notatnik_biezacy_rozmiar = 0;
        if (cx >= 216 && cx < 260) notatnik_biezacy_rozmiar = 1;
        if (cx >= 268 && cx < 312) notatnik_biezacy_rozmiar = 2;
        if (cx >= 320 && cx < 400) notatnik_odczytaj();
        if (cx >= 410 && cx < 480) notatnik_zapisz();
        okno_rysuj();
        return;
    }
    okno_ustaw_bufor_tekstu_user(&okno_notatnik, notatka, notatka_dlugosc, NOTATNIK_MAKS);
    okno_wejdz_do_pola_tekstowego_user(&okno_notatnik);
    okno_notatnik.kursor_pozycja = notatka_dlugosc;
}

static void notatnik_pole_tekstowe_enter(void) {
}

static void notatnik_pole_tekstowe_wyjdz(void) {
    notatnik_ustaw_status("edytor zakonczony");
}

static int notatnik_pole_tekstowe_znak(char znak, uint8_t ctrl) {
    if (ctrl) {
        if (znak == 'c') {
            return 1;
        }
        if (znak == 'v') {
            notatka[notatka_dlugosc] = 0;
            okno_rysuj();
            return 1;
        }
        return 1;
    }
    if (znak == '\b') {
        if (notatka_dlugosc > 0 && okno_notatnik.kursor_pozycja > 0) {
            for (int i = okno_notatnik.kursor_pozycja - 1; i < notatka_dlugosc - 1; i++) {
                notatka[i] = notatka[i + 1];
                notatka_rozmiar[i] = notatka_rozmiar[i + 1];
                notatka_styl[i] = notatka_styl[i + 1];
            }
            notatka_dlugosc--;
            okno_notatnik.kursor_pozycja--;
        }
    } else if ((znak == '\n' || znak == '\r') && notatka_dlugosc < NOTATNIK_MAKS) {
        for (int i = notatka_dlugosc; i >= okno_notatnik.kursor_pozycja; i--) {
            notatka[i + 1] = notatka[i];
            notatka_rozmiar[i + 1] = notatka_rozmiar[i];
            notatka_styl[i + 1] = notatka_styl[i];
        }
        notatka[okno_notatnik.kursor_pozycja] = '\n';
        notatka_rozmiar[okno_notatnik.kursor_pozycja] = notatnik_biezacy_rozmiar;
        notatka_styl[okno_notatnik.kursor_pozycja] = notatnik_biezacy_styl;
        notatka_dlugosc++;
        okno_notatnik.kursor_pozycja++;
    } else if (znak >= 32 && znak <= 126 && notatka_dlugosc < NOTATNIK_MAKS) {
        for (int i = notatka_dlugosc; i >= okno_notatnik.kursor_pozycja; i--) {
            notatka[i + 1] = notatka[i];
            notatka_rozmiar[i + 1] = notatka_rozmiar[i];
            notatka_styl[i + 1] = notatka_styl[i];
        }
        notatka[okno_notatnik.kursor_pozycja] = znak;
        notatka_rozmiar[okno_notatnik.kursor_pozycja] = notatnik_biezacy_rozmiar;
        notatka_styl[okno_notatnik.kursor_pozycja] = notatnik_biezacy_styl;
        notatka_dlugosc++;
        okno_notatnik.kursor_pozycja++;
    } else {
        return 1;
    }
    notatka[notatka_dlugosc] = 0;
    okno_rysuj();
    return 1;
}

int main(void) {
    notatka_dlugosc = 0;
    notatka[0] = 0;
    notatka_rozmiar[0] = 1;
    notatka_styl[0] = 0;
    notatnik_ustaw_tryb_tekstowy();
    notatnik_ustaw_status("gotowy");
    notatnik_odczytaj();

    okno_stworz_user(&okno_notatnik, 56, 50, NOTATNIK_SZEROKOSC,
                     NOTATNIK_WYSOKOSC, "NOTATNIK", notatnik_narysuj);
    okno_ustaw_obsluge_klikniecia_user(&okno_notatnik, notatnik_klik);
    okno_ustaw_pole_tekstowe_enter_user(&okno_notatnik, notatnik_pole_tekstowe_enter);
    okno_ustaw_pole_tekstowe_wyjdz_user(&okno_notatnik, notatnik_pole_tekstowe_wyjdz);
    okno_ustaw_pole_tekstowe_znak_user(&okno_notatnik, notatnik_pole_tekstowe_znak);
    okno_ustaw_bufor_tekstu_user(&okno_notatnik, notatka, notatka_dlugosc, NOTATNIK_MAKS);

    while (okno_czy_otwarte_user(&okno_notatnik)) {
        okno_rysuj();
        proces_yield_user();
    }

    return 0;
}
