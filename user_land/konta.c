#include "../graficzny/rysowanie.h"
#include "../graficzny/czcionka.h"
#include "../graficzny/okno.h"
#include "../graficzny/pulpit.h"
#include "../drivers/keyboard.h"
#include "../system_plikow/system_plikow.h"

#define PLIK_HASLA "HASLO.TXT"
#define SEKTOR_HASLA 10000
#define KONTO_SZEROKOSC 420
#define KONTO_WYSOKOSC 190

static struct okno okno_konta;
static char haslo[64];
static int haslo_dlugosc;
static char zapamietane_haslo[64];
static int zapamietane_dlugosc;
static uint8_t zalogowano = 0;
static uint8_t zle_haslo = 0;
static uint8_t ustawianie = 0;

static int tekst_rowne(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a++ != *b++) return 0;
    }
    return *a == *b;
}

static void konta_czytaj_zapamietane(void)
{
    char buf[512];
    fs_read_sector(SEKTOR_HASLA, buf);
    uint8_t dlugosc = (uint8_t)buf[0];
    if (dlugosc > 0 && dlugosc < 64) {
        for (int i = 0; i < dlugosc; i++) zapamietane_haslo[i] = (char)buf[1 + i];
        zapamietane_haslo[dlugosc] = 0;
        zapamietane_dlugosc = dlugosc;
        return;
    }
    fs_reset_directory();
    int n = read_file(PLIK_HASLA, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = 0;
        int i = 0;
        while (buf[i] && buf[i] != '\n' && buf[i] != '\r' && buf[i] != ' ') i++;
        int limit = i < 63 ? i : 63;
        for (int j = 0; j < limit; j++) zapamietane_haslo[j] = buf[j];
        zapamietane_haslo[limit] = 0;
        zapamietane_dlugosc = limit;
    } else {
        zapamietane_dlugosc = 0;
        zapamietane_haslo[0] = 0;
    }
}

static void konta_zapisz_haslo(void)
{
    char buf[512];
    for (int i = 0; i < 512; i++) buf[i] = 0;
    buf[0] = (char)haslo_dlugosc;
    for (int i = 0; i < haslo_dlugosc; i++) buf[1 + i] = haslo[i];
    fs_write_sector(SEKTOR_HASLA, buf);

    fs_reset_directory();
    if (write_file(PLIK_HASLA, haslo, (uint32_t)haslo_dlugosc) != 0) {
        if (create_file(PLIK_HASLA, (uint32_t)haslo_dlugosc) != 0 ||
            write_file(PLIK_HASLA, haslo, (uint32_t)haslo_dlugosc) != 0) {
            return;
        }
    }
    int limit = haslo_dlugosc < 63 ? haslo_dlugosc : 63;
    for (int i = 0; i < limit; i++) zapamietane_haslo[i] = haslo[i];
    zapamietane_haslo[limit] = 0;
    zapamietane_dlugosc = limit;
}

static void konta_dzialaj(void)
{
    if (ustawianie) {
        if (haslo_dlugosc == 0) return;
        konta_zapisz_haslo();
        zalogowano = 1;
        ustawianie = 0;
        okno_zamknij(&okno_konta);
        return;
    }
    if (tekst_rowne(haslo, zapamietane_haslo)) {
        zalogowano = 1;
        okno_zamknij(&okno_konta);
    } else {
        zle_haslo = 1;
        haslo_dlugosc = 0;
        haslo[0] = 0;
        okno_narysuj(&okno_konta);
    }
}

static void konta_narysuj(int x, int y, int szerokosc, int wysokosc)
{
    (void)szerokosc;
    (void)wysokosc;
    if (ustawianie) {
        rysuj_tekst_chicago(x, y, "Ustaw haslo konta:", 0x00000000);
    } else if (zle_haslo) {
        rysuj_tekst_chicago(x, y, "Zle haslo. Sprobuj ponownie:", 0x00FF0000);
    } else {
        rysuj_tekst_chicago(x, y, "Podaj haslo:", 0x00000000);
    }
    rysuj_prostokat(x, y + 22, 220, 26, 0x00FFFFFF);
    rysuj_prostokat(x, y + 22, 220, 1, 0x00000000);
    rysuj_prostokat(x, y + 47, 220, 1, 0x00000000);
    rysuj_prostokat(x, y + 22, 1, 26, 0x00000000);
    rysuj_prostokat(x + 219, y + 22, 1, 26, 0x00000000);
    for (int i = 0; i < haslo_dlugosc; i++) {
        rysuj_znak_chicago(x + 4 + i * 8, y + 26, '*', 0x00000000);
    }
    if (okno_konta.w_polu_tekstowym && kursor_tekstowy_widoczny()) {
        rysuj_prostokat(x + 4 + haslo_dlugosc * 8, y + 24, 2, 20, 0x00000000);
    }
    rysuj_prostokat(x + 232, y + 22, 80, 26, 0x004A5D6E);
    rysuj_tekst_chicago(x + 258, y + 28, "OK", 0x00FFFFFF);
}

static void konta_klik(int mx, int my)
{
    if (mx >= 232 && mx < 232 + 80 && my >= 22 && my < 22 + 26) {
        konta_dzialaj();
        return;
    }
    okno_wejdz_do_pola_tekstowego(&okno_konta);
}

static int konta_znak(struct okno *okno, char znak, uint8_t ctrl)
{
    (void)okno;
    (void)ctrl;
    if (znak == '\b') {
        if (haslo_dlugosc > 0) {
            haslo_dlugosc--;
            haslo[haslo_dlugosc] = 0;
            okno_narysuj(&okno_konta);
        }
        return 1;
    }
    if (znak == '\n' || znak == '\r') {
        konta_dzialaj();
        return 1;
    }
    if (znak >= 32 && znak <= 126 && haslo_dlugosc < 63) {
        haslo[haslo_dlugosc++] = znak;
        haslo[haslo_dlugosc] = 0;
        okno_narysuj(&okno_konta);
        return 1;
    }
    return 1;
}

void konta_start(void)
{
    fs_reset_directory();
    konta_czytaj_zapamietane();
    ustawianie = (zapamietane_dlugosc <= 0);
    zalogowano = 0;
    zle_haslo = 0;
    haslo_dlugosc = 0;
    haslo[0] = 0;
    int x = (EKRAN_SZEROKOSC - KONTO_SZEROKOSC) / 2;
    int y = (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - KONTO_WYSOKOSC) / 2;
    if (y < 20) y = 20;
    okno_stworz(&okno_konta, x, y, KONTO_SZEROKOSC, KONTO_WYSOKOSC,
                ustawianie ? "Ustaw haslo" : "Logowanie", konta_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_konta, konta_klik);
    okno_ustaw_pole_tekstowe_znak(&okno_konta, konta_znak);
    okno_ustaw_bufor_tekstu(&okno_konta, haslo, haslo_dlugosc, 63);
    okno_wejdz_do_pola_tekstowego(&okno_konta);
}

uint8_t konta_czy_zalogowano(void)
{
    return zalogowano;
}

void konta_obsluz(void)
{
    if (!zalogowano && !okno_konta.otwarte) {
        konta_start();
    }
    if (!zalogowano) {
        okno_narysuj(&okno_konta);
    }
    char znak;
    while (keyboard_try_getchar(&znak)) {
        uint8_t ctrl = keyboard_ctrl_pressed();
        if (okno_konta.w_polu_tekstowym && okno_konta.pole_tekstowe_znak) {
            if (!okno_konta.pole_tekstowe_znak(&okno_konta, znak, ctrl)) break;
        }
    }
}
