#include "zadania.h"
#include "../graficzny/czcionka.h"
#include "../graficzny/okno.h"
#include "../graficzny/rysowanie.h"
#include "../kernel/proces.h"

#define ZADANIA_SZEROKOSC 560
#define ZADANIA_WYSOKOSC 420
#define ZADANIA_WIERSZ_WYS 20
#define ZADANIA_WIERSZ_POCZ 24
#define ZADANIA_PRZYCISK_W 60
#define ZADANIA_PRZYCISK_H 18
#define ZADANIA_NAZWA_MAKS 14

#define KOLOR_TEKST_ZADANIA 0x00334155
#define KOLOR_PRZYCISKU_ZABIJ 0x00C0392B
#define KOLOR_TEKST_PRZYCISKU 0x00FFFFFF
#define KOLOR_WIERSZ_PARZYSTY 0x00EEF2F6

static struct okno okno_zadania;

static const char *zadania_stan_nazwa(proces_state_t stan)
{
    if (stan == PROCES_READY) return "GOTOWY";
    if (stan == PROCES_RUNNING) return "URUCHOMIONY";
    if (stan == PROCES_BLOCKED) return "ZABLOKOWANY";
    return "ZAKONCZONY";
}

static int zadania_przycisk_x(int szerokosc)
{
    return (szerokosc - 16) - ZADANIA_PRZYCISK_W - 4;
}

static void zadania_u32_do_tekstu(uint32_t v, char *buf)
{
    char tmp[12];
    int i = 0;
    int j = 0;
    if (v == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return;
    }
    while (v > 0) {
        tmp[i++] = (char)('0' + (v % 10));
        v /= 10;
    }
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = 0;
}

static void zadania_narysuj(int x, int y, int szerokosc, int wysokosc)
{
    (void)wysokosc;
    int bx = zadania_przycisk_x(szerokosc);
    char buf[16];
    char nazwa[ZADANIA_NAZWA_MAKS + 1];
    int n;

    rysuj_tekst_chicago(x + 4, y, "PID", KOLOR_TEKST_ZADANIA);
    rysuj_tekst_chicago(x + 30, y, "NAZWA", KOLOR_TEKST_ZADANIA);
    rysuj_tekst_chicago(x + 150, y, "STAN", KOLOR_TEKST_ZADANIA);

    int wy = y + ZADANIA_WIERSZ_POCZ;
    int numer = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        if (p->pid == 0 || p->state == PROCES_TERMINATED) continue;
        if (numer % 2 == 0) {
            rysuj_prostokat(x, wy, szerokosc, ZADANIA_WIERSZ_WYS, KOLOR_WIERSZ_PARZYSTY);
        }
        zadania_u32_do_tekstu(p->pid, buf);
        rysuj_tekst_chicago(x + 4, wy + 2, buf, KOLOR_TEKST_ZADANIA);
        n = 0;
        while (p->name[n] && n < ZADANIA_NAZWA_MAKS) {
            nazwa[n] = p->name[n];
            n++;
        }
        nazwa[n] = 0;
        rysuj_tekst_chicago(x + 30, wy + 2, nazwa, KOLOR_TEKST_ZADANIA);
        rysuj_tekst_chicago(x + 150, wy + 2, zadania_stan_nazwa(p->state), KOLOR_TEKST_ZADANIA);
        rysuj_prostokat(bx, wy + 1, ZADANIA_PRZYCISK_W, ZADANIA_PRZYCISK_H, KOLOR_PRZYCISKU_ZABIJ);
        rysuj_tekst_chicago(bx + 8, wy + 4, "ZABIJ", KOLOR_TEKST_PRZYCISKU);
        wy += ZADANIA_WIERSZ_WYS;
        numer++;
    }
}

static void zadania_klik(int x, int y)
{
    int bx = zadania_przycisk_x(okno_zadania.szerokosc);
    int cy = y - 30;
    int wy = ZADANIA_WIERSZ_POCZ;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        struct proces *p = &process_table[i];
        if (p->pid == 0 || p->state == PROCES_TERMINATED) continue;
        if (cy >= wy && cy < wy + ZADANIA_WIERSZ_WYS &&
            x >= bx && x < bx + ZADANIA_PRZYCISK_W) {
            if (p->window) okno_zamknij(p->window);
            proces_zakoncz(p->pid);
            okno_narysuj(&okno_zadania);
            return;
        }
        wy += ZADANIA_WIERSZ_WYS;
    }
}

void zadania_otworz(void)
{
    okno_stworz(&okno_zadania, 180, 120, ZADANIA_SZEROKOSC, ZADANIA_WYSOKOSC,
                "ZARZADZANIE ZADANIAMI", zadania_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_zadania, zadania_klik);
}
