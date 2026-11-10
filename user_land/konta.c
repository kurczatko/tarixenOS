#include "../graficzny/rysowanie.h"
#include "../graficzny/czcionka.h"
#include "../graficzny/okno.h"
#include "../graficzny/pulpit.h"
#include "../drivers/keyboard.h"
#include "../system_plikow/system_plikow.h"
#include "../kernel/tls_crypto.h"
#include "konta.h"

static int konto_strlen(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int konto_strcmp(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a++ != *b++) return 0;
    }
    return *a == *b;
}

static void konto_strcpy(char *dst, const char *src)
{
    int i = 0;
    while (src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

#define KONTA_TLO 0x001A1A2E
#define KONTA_KARTA 0x002D2D44
#define KONTA_KARTA_AKTYWNA 0x003B3B5C
#define KONTA_TEKST 0x00FFFFFF
#define KONTA_TEKST_DIM 0x00A0A0B0
#define KONTA_PRZYCISK 0x003B82F6
#define KONTA_PRZYCISK_HOVER 0x004A90E6
#define KONTA_BLED 0x00E74C3C
#define KONTA_SUKCES 0x0034C759

#define KONTA_STAN_WYBOR 0
#define KONTA_STAN_HASLO 1
#define KONTA_STAN_BLED 2
#define KONTA_STAN_NOWE 3
#define KONTA_STAN_ZARZADZANIE 4

static struct okno okno_konta;
static struct konto_uzytkownika uzytkownicy[KONTA_MAKS_UZYTKOWNIKOW];
static int liczba_uzytkownikow;
static int wybrany_uzytkownik;
static char haslo_wpisane[KONTA_MAX_HASLO];
static int haslo_dlugosc;
static char nowa_nazwa[KONTA_MAX_NAZWA];
static int nowa_nazwa_dlugosc;
static char nowe_haslo[KONTA_MAX_HASLO];
static int nowe_haslo_dlugosc;
static char powtorz_haslo[KONTA_MAX_HASLO];
static int powtorz_haslo_dlugosc;
static uint8_t zalogowano;
static const char *uzytkownik_aktywny_nazwa;
static const char *katalog_domowy_aktywny;
static int stan_konta;
static int przewijanie_uzytkownikow;
static char komunikat_bled[128];
static int komunikat_bled_dlugosc;
static uint8_t w_polu_hasla;
static uint8_t w_polu_nowej_nazwy;
static uint8_t w_polu_nowego_hasla;
static uint8_t w_polu_powtorzenia;
static uint32_t kolory_avatarow[] = {
    0x00FF6B6B, 0x004ECDC4, 0x0045B7D1, 0x00FFA07A,
    0x0098D8C8, 0x00F06292, 0x00AED581, 0x007986CB,
    0x00FF8A80, 0x0064B5F6, 0x00BA68C8, 0x004DB6AC,
    0x00FFD54F, 0x00FF7043, 0x008D6E63, 0x0078909C
};

void konta_narysuj(int x, int y, int szerokosc, int wysokosc);

static void konto_haszuj_haslo(const char *haslo, uint8_t wyjscie[32])
{
    struct sha256_context ctx;
    const char *salt = "TARIXENOS_SALT_2026";
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)salt, 19);
    sha256_update(&ctx, (const uint8_t *)haslo, (uint32_t)konto_strlen(haslo));
    sha256_final(&ctx, wyjscie);
}

static int konto_porownaj_hasz(const uint8_t a[32], const uint8_t b[32])
{
    for (int i = 0; i < 32; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static char konto_buf_zapisz[4128];

static void konto_zapisz_wszystkich(void)
{
    char *buf = konto_buf_zapisz;
    for (int i = 0; i < 4128; i++) buf[i] = 0;
    const char *magic = "TARIXENUSR";
    for (int i = 0; i < 10; i++) buf[i] = magic[i];
    uint32_t version = 1;
    uint32_t count = (uint32_t)liczba_uzytkownikow;
    buf[10] = (uint8_t)(version & 0xFF);
    buf[11] = (uint8_t)((version >> 8) & 0xFF);
    buf[12] = (uint8_t)((version >> 16) & 0xFF);
    buf[13] = (uint8_t)((version >> 24) & 0xFF);
    buf[14] = (uint8_t)(count & 0xFF);
    buf[15] = (uint8_t)((count >> 8) & 0xFF);
    buf[16] = (uint8_t)((count >> 16) & 0xFF);
    buf[17] = (uint8_t)((count >> 24) & 0xFF);
    int off = 32;
    for (int i = 0; i < liczba_uzytkownikow; i++) {
        struct konto_uzytkownika *u = &uzytkownicy[i];
        for (int j = 0; j < 32 && u->nazwa[j]; j++) buf[off + j] = u->nazwa[j];
        off += 32;
        for (int j = 0; j < 32; j++) buf[off + j] = u->hash_hasla[j];
        off += 32;
        for (int j = 0; j < 64 && u->katalog_domowy[j]; j++) buf[off + j] = u->katalog_domowy[j];
        off += 64;
        uint32_t kolor = u->kolor_tla;
        buf[off++] = (uint8_t)(kolor & 0xFF);
        buf[off++] = (uint8_t)((kolor >> 8) & 0xFF);
        buf[off++] = (uint8_t)((kolor >> 16) & 0xFF);
        buf[off++] = (uint8_t)((kolor >> 24) & 0xFF);
        buf[off++] = u->aktywny;
        off += 31;
    }
    fs_reset_directory();
    if (write_file(KONTA_PLIK_DANYCH, buf, 4128) != 0) {
        if (create_file(KONTA_PLIK_DANYCH, 4128) != 0 ||
            write_file(KONTA_PLIK_DANYCH, buf, 4128) != 0) {
            return;
        }
    }
}

static char konto_buf_wczytaj[4128];

static void konto_wczytaj_wszystkich(void)
{
    char *buf = konto_buf_wczytaj;
    fs_reset_directory();
    int n = read_file(KONTA_PLIK_DANYCH, buf, 4128);
    if (n <= 0) {
        liczba_uzytkownikow = 0;
        return;
    }
    const char *magic = "TARIXENUSR";
    int magiczny = 1;
    for (int i = 0; i < 10; i++) {
        if (buf[i] != magic[i]) { magiczny = 0; break; }
    }
    if (!magiczny) {
        liczba_uzytkownikow = 0;
        return;
    }
    uint32_t count = (uint32_t)buf[14] | ((uint32_t)buf[15] << 8) |
                     ((uint32_t)buf[16] << 16) | ((uint32_t)buf[17] << 24);
    if (count > KONTA_MAKS_UZYTKOWNIKOW) count = KONTA_MAKS_UZYTKOWNIKOW;
    liczba_uzytkownikow = (int)count;
    int off = 32;
    for (int i = 0; i < liczba_uzytkownikow; i++) {
        struct konto_uzytkownika *u = &uzytkownicy[i];
        for (int j = 0; j < 32; j++) u->nazwa[j] = buf[off + j];
        u->nazwa[31] = 0;
        off += 32;
        for (int j = 0; j < 32; j++) u->hash_hasla[j] = buf[off + j];
        off += 32;
        for (int j = 0; j < 64; j++) u->katalog_domowy[j] = buf[off + j];
        u->katalog_domowy[63] = 0;
        off += 64;
        u->kolor_tla = (uint32_t)buf[off] | ((uint32_t)buf[off + 1] << 8) |
                       ((uint32_t)buf[off + 2] << 16) | ((uint32_t)buf[off + 3] << 24);
        off += 4;
        u->aktywny = buf[off];
        off += 31;
    }
}

static void konto_utworz_katalog_domowy(const char *nazwa)
{
    char sciezka[128];
    fs_reset_directory();
    create_dir("uzytkownicy");
    cd("uzytkownicy");
    konto_strcpy(sciezka, nazwa);
    create_dir(sciezka);
    cd(sciezka);
}

static int konto_znajdz_po_nazwie(const char *nazwa)
{
    for (int i = 0; i < liczba_uzytkownikow; i++) {
        if (konto_strcmp(uzytkownicy[i].nazwa, nazwa)) return i;
    }
    return -1;
}

static char znak_tytulu_uzytkownika(const char *nazwa)
{
    if (!nazwa[0]) return 'U';
    if (nazwa[0] >= 'a' && nazwa[0] <= 'z') return (char)(nazwa[0] - 'a' + 'A');
    if (nazwa[0] >= 'A' && nazwa[0] <= 'Z') return nazwa[0];
    if (nazwa[0] >= '0' && nazwa[0] <= '9') return nazwa[0];
    return 'U';
}

static void konto_rysuj_tekst_centrowany(int y, const char *tekst, uint32_t kolor)
{
    int dlugosc = konto_strlen(tekst);
    int x = (EKRAN_SZEROKOSC - dlugosc * 8) / 2;
    rysuj_tekst_chicago(x, y, tekst, kolor);
}

static void konto_rysuj_avatar(int x, int y, int rozmiar, const char *nazwa, uint32_t kolor, uint8_t aktywne)
{
    int cx = x + rozmiar / 2;
    int cy = y + rozmiar / 2;
    int promien = rozmiar / 2;
    rysuj_prostokat(x, y, rozmiar, rozmiar, aktywne ? KONTA_KARTA_AKTYWNA : KONTA_KARTA);
    for (int dy = -promien; dy <= promien; dy++) {
        for (int dx = -promien; dx <= promien; dx++) {
            if (dx * dx + dy * dy <= promien * promien) {
                rysuj_piksel(cx + dx, cy + dy, kolor);
            }
        }
    }
    char inicjal = znak_tytulu_uzytkownika(nazwa);
    rysuj_znak_chicago(cx - 4, cy - 4, inicjal, 0x00000000);
}

static void konto_rysuj_przycisk(int x, int y, int szerokosc, int wysokosc, const char *tekst, uint8_t hover)
{
    rysuj_prostokat(x, y, szerokosc, wysokosc, hover ? KONTA_PRZYCISK_HOVER : KONTA_PRZYCISK);
    rysuj_prostokat(x, y, szerokosc, 1, 0x00555555);
    rysuj_prostokat(x, y + wysokosc - 1, szerokosc, 1, 0x00222222);
    int dlugosc = konto_strlen(tekst);
    int tx = x + (szerokosc - dlugosc * 8) / 2;
    int ty = y + (wysokosc - 8) / 2;
    rysuj_tekst_chicago(tx, ty, tekst, KONTA_TEKST);
}

static void konto_narysuj_logowanie(int x, int y, int szerokosc, int wysokosc)
{
    (void)x; (void)y; (void)szerokosc; (void)wysokosc;
    wypelnij_ekran(KONTA_TLO);
    konto_rysuj_tekst_centrowany(60, "TARIXENOS", 0x00FFFFFF);
    konto_rysuj_tekst_centrowany(90, "WYBIERZ KONTO", KONTA_TEKST_DIM);

    int start_y = 180;
    int card_w = 260;
    int card_x = (EKRAN_SZEROKOSC - card_w) / 2;

    if (liczba_uzytkownikow == 0) {
        konto_rysuj_tekst_centrowany(start_y, "Brak kont. Utworz pierwsze konto.", KONTA_TEKST_DIM);
    } else {
        int pokazanych = 0;
        int wyswietlanych = 0;
        for (int i = przewijanie_uzytkownikow; i < liczba_uzytkownikow && wyswietlanych < 4; i++) {
            struct konto_uzytkownika *u = &uzytkownicy[i];
            if (!u->aktywny) continue;
            int cy = start_y + pokazanych * 160;
            uint8_t aktywne = (i == wybrany_uzytkownik);
            konto_rysuj_avatar(card_x + 30, cy + 20, 100, u->nazwa, u->kolor_tla, aktywne);
            rysuj_tekst_chicago(card_x + 150, cy + 35, u->nazwa, aktywne ? KONTA_TEKST : KONTA_TEKST_DIM);
            if (aktywne) {
                rysuj_prostokat(card_x + 20, cy + 130, card_w - 40, 30, 0x003A3A50);
                rysuj_prostokat(card_x + 22, cy + 132, card_w - 44, 26, 0x002A2A3A);
                for (int j = 0; j < haslo_dlugosc; j++) {
                    rysuj_znak_chicago(card_x + 26 + j * 10, cy + 138, '*', KONTA_TEKST);
                }
                if (w_polu_hasla && kursor_tekstowy_widoczny()) {
                    rysuj_prostokat(card_x + 26 + haslo_dlugosc * 10, cy + 136, 2, 22, KONTA_TEKST);
                }
                rysuj_tekst_chicago(card_x + 20, cy + 170, "Wpisz haslo i nacisnij Enter", KONTA_TEKST_DIM);
            }
            pokazanych++;
            wyswietlanych++;
        }
    }

    konto_rysuj_przycisk(40, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80, 160, 40, "DODAJ KONTO", 0);
    konto_rysuj_przycisk(EKRAN_SZEROKOSC - 200, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80, 160, 40, "WYLACZ", 0);

    if (komunikat_bled_dlugosc > 0) {
        rysuj_prostokat((EKRAN_SZEROKOSC - 400) / 2, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 180, 400, 30, KONTA_BLED);
        rysuj_tekst_chicago((EKRAN_SZEROKOSC - 400) / 2 + 10, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 172, komunikat_bled, KONTA_TEKST);
    }
}

static void konto_narysuj_zarzadzanie(int x, int y, int szerokosc, int wysokosc)
{
    (void)x; (void)y; (void)szerokosc; (void)wysokosc;
    wypelnij_ekran(KONTA_TLO);
    konto_rysuj_tekst_centrowany(60, "ZARZADZANIE UZYTKOWNIKAMI", 0x00FFFFFF);

    int start_y = 140;
    int row_h = 50;
    for (int i = 0; i < liczba_uzytkownikow; i++) {
        struct konto_uzytkownika *u = &uzytkownicy[i];
        if (!u->aktywny) continue;
        int cy = start_y + i * row_h;
        rysuj_prostokat(80, cy, EKRAN_SZEROKOSC - 160, row_h - 8, i == wybrany_uzytkownik ? KONTA_KARTA_AKTYWNA : KONTA_KARTA);
        konto_rysuj_avatar(100, cy + 10, 30, u->nazwa, u->kolor_tla, 0);
        rysuj_tekst_chicago(150, cy + 20, u->nazwa, KONTA_TEKST);
        rysuj_tekst_chicago(150, cy + 36, u->katalog_domowy, KONTA_TEKST_DIM);
    }

    konto_rysuj_przycisk(40, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80, 160, 40, "DODAJ", 0);
    konto_rysuj_przycisk(220, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80, 160, 40, "USUN", 0);
    konto_rysuj_przycisk(EKRAN_SZEROKOSC - 200, EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80, 160, 40, "ZAMKNIJ", 0);
}

static void konto_narysuj_nowe_konto(int x, int y, int szerokosc, int wysokosc)
{
    (void)x; (void)y; (void)szerokosc; (void)wysokosc;
    wypelnij_ekran(KONTA_TLO);
    konto_rysuj_tekst_centrowany(80, "TWORZENIE NOWEGO KONTA", 0x00FFFFFF);

    int cx = EKRAN_SZEROKOSC / 2 - 200;
    int cy = 200;

    rysuj_tekst_chicago(cx, cy, "Nazwa uzytkownika:", KONTA_TEKST);
    rysuj_prostokat(cx, cy + 22, 400, 30, 0x003A3A50);
    rysuj_prostokat(cx + 2, cy + 24, 396, 26, 0x002A2A3A);
    rysuj_tekst_chicago(cx + 6, cy + 30, nowa_nazwa, KONTA_TEKST);
    if (w_polu_nowej_nazwy && kursor_tekstowy_widoczny()) {
        rysuj_prostokat(cx + 6 + nowa_nazwa_dlugosc * 8, cy + 28, 2, 20, KONTA_TEKST);
    }

    cy += 80;
    rysuj_tekst_chicago(cx, cy, "Haslo:", KONTA_TEKST);
    rysuj_prostokat(cx, cy + 22, 400, 30, 0x003A3A50);
    rysuj_prostokat(cx + 2, cy + 24, 396, 26, 0x002A2A3A);
    for (int j = 0; j < nowe_haslo_dlugosc; j++) {
        rysuj_znak_chicago(cx + 6 + j * 10, cy + 30, '*', KONTA_TEKST);
    }
    if (w_polu_nowego_hasla && kursor_tekstowy_widoczny()) {
        rysuj_prostokat(cx + 6 + nowe_haslo_dlugosc * 10, cy + 28, 2, 20, KONTA_TEKST);
    }

    cy += 80;
    rysuj_tekst_chicago(cx, cy, "Powtorz haslo:", KONTA_TEKST);
    rysuj_prostokat(cx, cy + 22, 400, 30, 0x003A3A50);
    rysuj_prostokat(cx + 2, cy + 24, 396, 26, 0x002A2A3A);
    for (int j = 0; j < powtorz_haslo_dlugosc; j++) {
        rysuj_znak_chicago(cx + 6 + j * 10, cy + 30, '*', KONTA_TEKST);
    }
    if (w_polu_powtorzenia && kursor_tekstowy_widoczny()) {
        rysuj_prostokat(cx + 6 + powtorz_haslo_dlugosc * 10, cy + 28, 2, 20, KONTA_TEKST);
    }

    konto_rysuj_przycisk(cx, cy + 80, 180, 40, "UTWORZ", 0);
    konto_rysuj_przycisk(cx + 220, cy + 80, 180, 40, "ANULUJ", 0);
}

static void konto_bled(const char *msg)
{
    konto_strcpy(komunikat_bled, msg);
    komunikat_bled_dlugosc = konto_strlen(msg);
    stan_konta = KONTA_STAN_BLED;
}

static void konto_czysc_bled(void)
{
    komunikat_bled_dlugosc = 0;
    komunikat_bled[0] = 0;
    if (stan_konta == KONTA_STAN_BLED) stan_konta = KONTA_STAN_WYBOR;
}

static void konto_zaloguj_wybranego(void)
{
    if (wybrany_uzytkownik < 0 || wybrany_uzytkownik >= liczba_uzytkownikow) return;
    struct konto_uzytkownika *u = &uzytkownicy[wybrany_uzytkownik];
    uint8_t hash[32];
    konto_haszuj_haslo(haslo_wpisane, hash);
    if (!konto_porownaj_hasz(hash, u->hash_hasla)) {
        konto_bled("Nieprawidlowe haslo");
        haslo_dlugosc = 0;
        haslo_wpisane[0] = 0;
        return;
    }
    zalogowano = 1;
    uzytkownik_aktywny_nazwa = u->nazwa;
    katalog_domowy_aktywny = u->katalog_domowy;
    okno_zamknij(&okno_konta);
    tapeta_wczytaj();
}

static void konto_utworz_nowe(void)
{
    if (nowa_nazwa_dlugosc == 0 || nowe_haslo_dlugosc == 0) {
        konto_bled("Wypelnij wszystkie pola");
        return;
    }
    if (nowe_haslo_dlugosc != powtorz_haslo_dlugosc ||
        !konto_strcmp(nowe_haslo, powtorz_haslo)) {
        konto_bled("Hasla nie sa identyczne");
        return;
    }
    if (konto_znajdz_po_nazwie(nowa_nazwa) >= 0) {
        konto_bled("Uzytkownik juz istnieje");
        return;
    }
    if (liczba_uzytkownikow >= KONTA_MAKS_UZYTKOWNIKOW) {
        konto_bled("Osiagnieto limit uzytkownikow");
        return;
    }
    struct konto_uzytkownika *u = &uzytkownicy[liczba_uzytkownikow];
    konto_strcpy(u->nazwa, nowa_nazwa);
    konto_haszuj_haslo(nowe_haslo, u->hash_hasla);
    konto_strcpy(u->katalog_domowy, nowa_nazwa);
    u->kolor_tla = kolory_avatarow[liczba_uzytkownikow % (sizeof(kolory_avatarow) / sizeof(kolory_avatarow[0]))];
    u->aktywny = 1;
    liczba_uzytkownikow++;
    nowa_nazwa_dlugosc = 0;
    nowa_nazwa[0] = 0;
    nowe_haslo_dlugosc = 0;
    nowe_haslo[0] = 0;
    powtorz_haslo_dlugosc = 0;
    powtorz_haslo[0] = 0;
    w_polu_nowej_nazwy = 0;
    w_polu_nowego_hasla = 0;
    w_polu_powtorzenia = 0;
    stan_konta = KONTA_STAN_WYBOR;
    wybrany_uzytkownik = liczba_uzytkownikow - 1;
    okno_narysuj(&okno_konta);
}

static void konto_klik(int mx, int my)
{
    if (stan_konta == KONTA_STAN_ZARZADZANIE) {
        if (my >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80 && my < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 40) {
            if (mx >= 40 && mx < 200) {
                stan_konta = KONTA_STAN_NOWE;
                nowa_nazwa_dlugosc = 0;
                nowa_nazwa[0] = 0;
                nowe_haslo_dlugosc = 0;
                nowe_haslo[0] = 0;
                powtorz_haslo_dlugosc = 0;
                powtorz_haslo[0] = 0;
                w_polu_nowej_nazwy = 1;
                w_polu_nowego_hasla = 0;
                w_polu_powtorzenia = 0;
                okno_narysuj(&okno_konta);
                return;
            }
            if (mx >= 220 && mx < 380) {
                if (wybrany_uzytkownik >= 0 && wybrany_uzytkownik < liczba_uzytkownikow) {
                    uzytkownicy[wybrany_uzytkownik].aktywny = 0;
                    if (wybrany_uzytkownik >= liczba_uzytkownikow) wybrany_uzytkownik = liczba_uzytkownikow - 1;
                    okno_narysuj(&okno_konta);
                }
                return;
            }
            if (mx >= EKRAN_SZEROKOSC - 200 && mx < EKRAN_SZEROKOSC - 40) {
                okno_zamknij(&okno_konta);
                tapeta_wczytaj();
                return;
            }
        }
        int start_y = 140;
        int row_h = 50;
        int idx = 0;
        for (int i = 0; i < liczba_uzytkownikow; i++) {
            if (!uzytkownicy[i].aktywny) continue;
            int cy = start_y + idx * row_h;
            if (mx >= 80 && mx < EKRAN_SZEROKOSC - 80 && my >= cy && my < cy + row_h - 8) {
                wybrany_uzytkownik = i;
                okno_narysuj(&okno_konta);
                return;
            }
            idx++;
        }
        return;
    }

    if (stan_konta == KONTA_STAN_NOWE) {
        int cx = EKRAN_SZEROKOSC / 2 - 200;
        int cy = 200;
        if (mx >= cx && mx < cx + 400 && my >= cy + 22 && my < cy + 52) {
            w_polu_nowej_nazwy = 1;
            w_polu_nowego_hasla = 0;
            w_polu_powtorzenia = 0;
            okno_wejdz_do_pola_tekstowego(&okno_konta);
            return;
        }
        cy += 80;
        if (mx >= cx && mx < cx + 400 && my >= cy + 22 && my < cy + 52) {
            w_polu_nowej_nazwy = 0;
            w_polu_nowego_hasla = 1;
            w_polu_powtorzenia = 0;
            okno_wejdz_do_pola_tekstowego(&okno_konta);
            return;
        }
        cy += 80;
        if (mx >= cx && mx < cx + 400 && my >= cy + 22 && my < cy + 52) {
            w_polu_nowej_nazwy = 0;
            w_polu_nowego_hasla = 0;
            w_polu_powtorzenia = 1;
            okno_wejdz_do_pola_tekstowego(&okno_konta);
            return;
        }
        cy += 80;
        if (mx >= cx && mx < cx + 180 && my >= cy && my < cy + 40) {
            konto_utworz_nowe();
            return;
        }
        if (mx >= cx + 220 && mx < cx + 400 && my >= cy && my < cy + 40) {
            stan_konta = KONTA_STAN_WYBOR;
            nowa_nazwa_dlugosc = 0;
            nowa_nazwa[0] = 0;
            nowe_haslo_dlugosc = 0;
            nowe_haslo[0] = 0;
            powtorz_haslo_dlugosc = 0;
            powtorz_haslo[0] = 0;
            w_polu_nowej_nazwy = 0;
            w_polu_nowego_hasla = 0;
            w_polu_powtorzenia = 0;
            okno_narysuj(&okno_konta);
            return;
        }
        return;
    }

    if (my >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 80 && my < EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - 40) {
        if (mx >= 40 && mx < 200) {
            stan_konta = KONTA_STAN_NOWE;
            nowa_nazwa_dlugosc = 0;
            nowa_nazwa[0] = 0;
            nowe_haslo_dlugosc = 0;
            nowe_haslo[0] = 0;
            powtorz_haslo_dlugosc = 0;
            powtorz_haslo[0] = 0;
            w_polu_nowej_nazwy = 1;
            w_polu_nowego_hasla = 0;
            w_polu_powtorzenia = 0;
            okno_wejdz_do_pola_tekstowego(&okno_konta);
            okno_narysuj(&okno_konta);
            return;
        }
        if (mx >= EKRAN_SZEROKOSC - 200 && mx < EKRAN_SZEROKOSC - 40) {
            extern void shutdown(void);
            shutdown();
            return;
        }
    }

    if (liczba_uzytkownikow == 0) return;
    int start_y = 180;
    int card_w = 260;
    int card_x = (EKRAN_SZEROKOSC - card_w) / 2;
    int pokazanych = 0;
    for (int i = przewijanie_uzytkownikow; i < liczba_uzytkownikow; i++) {
        if (!uzytkownicy[i].aktywny) continue;
        int cy = start_y + pokazanych * 160;
        if (mx >= card_x && mx < card_x + card_w && my >= cy && my < cy + 140) {
            wybrany_uzytkownik = i;
            stan_konta = KONTA_STAN_HASLO;
            haslo_dlugosc = 0;
            haslo_wpisane[0] = 0;
            w_polu_hasla = 1;
            konto_czysc_bled();
            okno_wejdz_do_pola_tekstowego(&okno_konta);
            okno_narysuj(&okno_konta);
            return;
        }
        pokazanych++;
    }
}

static int konto_znak(struct okno *okno, char znak, uint8_t ctrl)
{
    (void)okno;
    (void)ctrl;
    if (stan_konta == KONTA_STAN_NOWE) {
        if (w_polu_nowej_nazwy) {
            if (znak == '\b') {
                if (nowa_nazwa_dlugosc > 0) {
                    nowa_nazwa_dlugosc--;
                    nowa_nazwa[nowa_nazwa_dlugosc] = 0;
                    okno_narysuj(&okno_konta);
                }
            } else if (znak == '\n' || znak == '\r') {
                w_polu_nowej_nazwy = 0;
                w_polu_nowego_hasla = 1;
            } else if (znak >= 32 && znak <= 126 && nowa_nazwa_dlugosc < KONTA_MAX_NAZWA - 1) {
                nowa_nazwa[nowa_nazwa_dlugosc++] = znak;
                nowa_nazwa[nowa_nazwa_dlugosc] = 0;
                okno_narysuj(&okno_konta);
            }
        } else if (w_polu_nowego_hasla) {
            if (znak == '\b') {
                if (nowe_haslo_dlugosc > 0) {
                    nowe_haslo_dlugosc--;
                    nowe_haslo[nowe_haslo_dlugosc] = 0;
                    okno_narysuj(&okno_konta);
                }
            } else if (znak == '\n' || znak == '\r') {
                w_polu_nowego_hasla = 0;
                w_polu_powtorzenia = 1;
            } else if (znak >= 32 && znak <= 126 && nowe_haslo_dlugosc < KONTA_MAX_HASLO - 1) {
                nowe_haslo[nowe_haslo_dlugosc++] = znak;
                nowe_haslo[nowe_haslo_dlugosc] = 0;
                okno_narysuj(&okno_konta);
            }
        } else if (w_polu_powtorzenia) {
            if (znak == '\b') {
                if (powtorz_haslo_dlugosc > 0) {
                    powtorz_haslo_dlugosc--;
                    powtorz_haslo[powtorz_haslo_dlugosc] = 0;
                    okno_narysuj(&okno_konta);
                }
            } else if (znak == '\n' || znak == '\r') {
                konto_utworz_nowe();
            } else if (znak >= 32 && znak <= 126 && powtorz_haslo_dlugosc < KONTA_MAX_HASLO - 1) {
                powtorz_haslo[powtorz_haslo_dlugosc++] = znak;
                powtorz_haslo[powtorz_haslo_dlugosc] = 0;
                okno_narysuj(&okno_konta);
            }
        }
        return 1;
    }

    if (w_polu_hasla) {
        if (znak == '\b') {
            if (haslo_dlugosc > 0) {
                haslo_dlugosc--;
                haslo_wpisane[haslo_dlugosc] = 0;
                okno_narysuj(&okno_konta);
            }
        } else if (znak == '\n' || znak == '\r') {
            konto_zaloguj_wybranego();
        } else if (znak >= 32 && znak <= 126 && haslo_dlugosc < KONTA_MAX_HASLO - 1) {
            haslo_wpisane[haslo_dlugosc++] = znak;
            haslo_wpisane[haslo_dlugosc] = 0;
            okno_narysuj(&okno_konta);
        }
    }
    return 1;
}

void konta_start(void)
{
    zalogowano = 0;
    uzytkownik_aktywny_nazwa = 0;
    katalog_domowy_aktywny = 0;
    stan_konta = KONTA_STAN_WYBOR;
    wybrany_uzytkownik = -1;
    haslo_dlugosc = 0;
    haslo_wpisane[0] = 0;
    w_polu_hasla = 0;
    komunikat_bled_dlugosc = 0;
    komunikat_bled[0] = 0;
    przewijanie_uzytkownikow = 0;
    nowa_nazwa_dlugosc = 0;
    nowa_nazwa[0] = 0;
    nowe_haslo_dlugosc = 0;
    nowe_haslo[0] = 0;
    powtorz_haslo_dlugosc = 0;
    powtorz_haslo[0] = 0;
    w_polu_nowej_nazwy = 0;
    w_polu_nowego_hasla = 0;
    w_polu_powtorzenia = 0;

    if (liczba_uzytkownikow == 0) {
        struct konto_uzytkownika *u = &uzytkownicy[0];
        konto_strcpy(u->nazwa, "Administrator");
        konto_haszuj_haslo("admin", u->hash_hasla);
        konto_strcpy(u->katalog_domowy, "Administrator");
        u->kolor_tla = kolory_avatarow[0];
        u->aktywny = 1;
        liczba_uzytkownikow = 1;
        wybrany_uzytkownik = 0;
        stan_konta = KONTA_STAN_HASLO;
        w_polu_hasla = 1;
    } else {
        wybrany_uzytkownik = 0;
    }

    okno_stworz_pelnoekranowy(&okno_konta, "Logowanie", konta_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_konta, konto_klik);
    okno_ustaw_pole_tekstowe_znak(&okno_konta, konto_znak);
    okno_wejdz_do_pola_tekstowego(&okno_konta);
}

void konta_zarzadzaj(void)
{
    konto_wczytaj_wszystkich();
    stan_konta = KONTA_STAN_ZARZADZANIE;
    wybrany_uzytkownik = liczba_uzytkownikow > 0 ? 0 : -1;
    komunikat_bled_dlugosc = 0;
    komunikat_bled[0] = 0;
    okno_stworz_pelnoekranowy(&okno_konta, "Zarzadzanie uzytkownikami", konta_narysuj);
    okno_ustaw_obsluge_klikniecia(&okno_konta, konto_klik);
    okno_ustaw_pole_tekstowe_znak(&okno_konta, konto_znak);
    okno_wejdz_do_pola_tekstowego(&okno_konta);
}

uint8_t konta_czy_zalogowano(void)
{
    return zalogowano;
}

const char *konta_pobierz_nazwa_uzytkownika(void)
{
    return uzytkownik_aktywny_nazwa;
}

const char *konta_pobierz_katalog_domowy(void)
{
    return katalog_domowy_aktywny;
}

uint8_t konta_jest_administratorem(void)
{
    return uzytkownik_aktywny_nazwa != 0 && konto_strcmp(uzytkownik_aktywny_nazwa, "Administrator");
}

void konta_obsluz(void)
{
    if (!zalogowano && !okno_konta.otwarte) {
        konta_start();
    }
    char znak;
    while (keyboard_try_getchar(&znak)) {
        uint8_t ctrl = keyboard_ctrl_pressed();
        if (okno_konta.pole_tekstowe_znak) {
            if (!okno_konta.pole_tekstowe_znak(&okno_konta, znak, ctrl)) break;
        }
    }
}

void konta_narysuj(int x, int y, int szerokosc, int wysokosc)
{
    if (stan_konta == KONTA_STAN_ZARZADZANIE) {
        konto_narysuj_zarzadzanie(x, y, szerokosc, wysokosc);
        return;
    }
    if (stan_konta == KONTA_STAN_NOWE) {
        konto_narysuj_nowe_konto(x, y, szerokosc, wysokosc);
        return;
    }
    konto_narysuj_logowanie(x, y, szerokosc, wysokosc);
}
