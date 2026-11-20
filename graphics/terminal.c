/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <graphics/terminal.h>
#include <graphics/czcionka.h>
#include <graphics/okno.h>
#include <graphics/rysowanie.h>
#include <fs/system_plikow.h>
#include <kernel/apps/notatnik.h>
#include <graphics/eksplorator_plikow.h>
#include <kernel/apps/kalkulator.h>
#include <kernel/apps/browser.h>
#include <kernel/apps/gra.h>
#include <kernel/apps/malowanie.h>
#include <kernel/core/zadania.h>
#include <kernel/apps/duszki.h>
#include <kernel/core/proces.h>
#include <kernel/loader/loader.h>
#include <tools/cc_os.h>

#define TERMINAL_SZEROKOSC 700
#define TERMINAL_WYSOKOSC 500
#define TERMINAL_LINII_MAKS 28
#define TERMINAL_LINIA_DLUGOSC 95
#define TERMINAL_CZIONKA_WYSOKOSC 16
#define TERMINAL_PADDING_X 8
#define TERMINAL_PADDING_Y 8
#define TERMINAL_PROMPT "> "

struct okno okno_terminal;
static uint8_t terminal_otwarty = 0;
static uint8_t terminal_tryb_edycji = 0;
static char terminal_nazwa_pliku[32];
static char terminal_edytor_linie[64][256];
static int terminal_edytor_liczba_linii = 0;
static int terminal_edytor_biezaca_linia = 0;
static int terminal_edytor_zmieniony = 0;
static char linie_wyjscia[TERMINAL_LINII_MAKS][TERMINAL_LINIA_DLUGOSC + 1];
static int liczba_linii = 0;
static char linia_wejscia[TERMINAL_LINIA_DLUGOSC + 1];
static int dlugosc_wejscia = 0;

static void terminal_dodaj_linie(const char *tekst)
{
    if (liczba_linii >= TERMINAL_LINII_MAKS) {
        for (int i = 0; i < TERMINAL_LINII_MAKS - 1; i++) {
            for (int j = 0; j < TERMINAL_LINIA_DLUGOSC; j++) {
                linie_wyjscia[i][j] = linie_wyjscia[i + 1][j];
            }
            linie_wyjscia[i][TERMINAL_LINIA_DLUGOSC] = 0;
        }
        liczba_linii = TERMINAL_LINII_MAKS - 1;
    }
    int dlugosc = 0;
    while (tekst[dlugosc] && dlugosc < TERMINAL_LINIA_DLUGOSC) {
        linie_wyjscia[liczba_linii][dlugosc] = tekst[dlugosc];
        dlugosc++;
    }
    linie_wyjscia[liczba_linii][dlugosc] = 0;
    liczba_linii++;
}

static int terminal_starts_with(const char *a, const char *b)
{
    while (*b) {
        if (*a++ != *b++) return 0;
    }
    return 1;
}

static uint32_t terminal_parsuj_hex(const char *p)
{
    uint32_t v = 0;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
    while (*p) {
        char c = *p;
        uint32_t cyfra;
        if (c >= '0' && c <= '9') cyfra = (uint32_t)(c - '0');
        else if (c >= 'a' && c <= 'f') cyfra = (uint32_t)(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') cyfra = (uint32_t)(c - 'A' + 10);
        else break;
        v = (v << 4) | cyfra;
        p++;
    }
    return v;
}

static const char *terminal_pobierz_argument(const char *komenda)
{
    while (*komenda && *komenda != ' ') komenda++;
    while (*komenda == ' ') komenda++;
    return *komenda ? komenda : 0;
}

static int terminal_ma_rozszerzenie(const char *nazwa, const char *rozszerzenie)
{
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

static void terminal_edytor_wczytaj_plik(const char *nazwa)
{
    char buf[4096];
    fs_reset_directory();
    int bajty = read_file(nazwa, buf, sizeof(buf) - 1);
    if (bajty < 0) {
        terminal_dodaj_linie("Blad odczytu pliku");
        return;
    }
    buf[bajty] = 0;

    terminal_edytor_liczba_linii = 0;
    terminal_edytor_biezaca_linia = 0;
    terminal_edytor_zmieniony = 0;

    int linia = 0;
    int pos = 0;
    int start = 0;
    while (buf[pos] && linia < 64) {
        if (buf[pos] == '\n' || buf[pos] == '\r') {
            int len = pos - start;
            if (len > 255) len = 255;
            for (int i = 0; i < len; i++) terminal_edytor_linie[linia][i] = buf[start + i];
            terminal_edytor_linie[linia][len] = 0;
            linia++;
            if (buf[pos] == '\r' && buf[pos + 1] == '\n') pos++;
            pos++;
            start = pos;
            continue;
        }
        pos++;
    }
    if (start < pos && linia < 64) {
        int len = pos - start;
        if (len > 255) len = 255;
        for (int i = 0; i < len; i++) terminal_edytor_linie[linia][i] = buf[start + i];
        terminal_edytor_linie[linia][len] = 0;
        linia++;
    }
    if (linia == 0) {
        terminal_edytor_linie[0][0] = 0;
        linia = 1;
    }
    terminal_edytor_liczba_linii = linia;

    int d = 0;
    while (nazwa[d]) { terminal_nazwa_pliku[d] = nazwa[d]; d++; }
    terminal_nazwa_pliku[d] = 0;
}

static void terminal_edytor_zapisz_plik(void)
{
    char buf[4096];
    int pos = 0;
    for (int i = 0; i < terminal_edytor_liczba_linii; i++) {
        int len = 0;
        while (terminal_edytor_linie[i][len]) len++;
        for (int j = 0; j < len; j++) buf[pos++] = terminal_edytor_linie[i][j];
        if (i < terminal_edytor_liczba_linii - 1) buf[pos++] = '\n';
        if (pos > 4000) break;
    }
    buf[pos] = 0;
    fs_reset_directory();
    create_file(terminal_nazwa_pliku, (uint32_t)pos);
    if (write_file(terminal_nazwa_pliku, buf, (uint32_t)pos) == 0) {
        terminal_dodaj_linie("Plik zapisany");
        terminal_edytor_zmieniony = 0;
    } else {
        terminal_dodaj_linie("Blad zapisu pliku");
    }
}

static void terminal_edytor_dodaj_znak(char znak)
{
    if (terminal_edytor_biezaca_linia >= terminal_edytor_liczba_linii) return;
    char *linia = terminal_edytor_linie[terminal_edytor_biezaca_linia];
    int len = 0;
    while (linia[len]) len++;
    if (len >= 255) return;
    linia[len] = znak;
    linia[len + 1] = 0;
    terminal_edytor_zmieniony = 1;
}

static void terminal_edytor_usun_znak(void)
{
    if (terminal_edytor_biezaca_linia >= terminal_edytor_liczba_linii) return;
    char *linia = terminal_edytor_linie[terminal_edytor_biezaca_linia];
    int len = 0;
    while (linia[len]) len++;
    if (len == 0) return;
    linia[len - 1] = 0;
    terminal_edytor_zmieniony = 1;
}

static void terminal_edytor_nowa_linia(void)
{
    if (terminal_edytor_liczba_linii >= 64) return;
    for (int i = terminal_edytor_liczba_linii; i > terminal_edytor_biezaca_linia + 1; i--) {
        for (int j = 0; j < 256; j++) terminal_edytor_linie[i][j] = terminal_edytor_linie[i - 1][j];
    }
    terminal_edytor_linie[terminal_edytor_biezaca_linia + 1][0] = 0;
    terminal_edytor_liczba_linii++;
    terminal_edytor_biezaca_linia++;
    terminal_edytor_zmieniony = 1;
}

static void terminal_wykonaj_komende(const char *komenda)
{
    terminal_dodaj_linie(komenda);

    if (terminal_starts_with(komenda, "help") || terminal_starts_with(komenda, "pomoc")) {
        terminal_dodaj_linie("Dostepne komendy:");
        terminal_dodaj_linie("  help               - ta pomoc");
        terminal_dodaj_linie("  clear              - wyczysc ekran terminala");
        terminal_dodaj_linie("  ls                 - lista plikow");
        terminal_dodaj_linie("  cd [dir]           - zmiana katalogu");
        terminal_dodaj_linie("  pwd                - sciezka bieżąca");
        terminal_dodaj_linie("  rm [plik]          - usun plik");
        terminal_dodaj_linie("  touch [plik]       - utworz plik");
        terminal_dodaj_linie("  mkdir [dir]        - utworz katalog");
        terminal_dodaj_linie("  compile [plik.c]   - skompiluj plik .c");
        terminal_dodaj_linie("  run [plik]         - uruchom plik");
        terminal_dodaj_linie("  tapeta_zmien XXXX  - zmiana tapety (kolor hex np. FF0000)");
        terminal_dodaj_linie("  notatnik           - otworz notatnik");
        terminal_dodaj_linie("  pliki              - otworz eksplorator plikow");
        terminal_dodaj_linie("  kalkulator         - otworz kalkulator");
        terminal_dodaj_linie("  browser            - otworz przegladarke");
        terminal_dodaj_linie("  gra                - otworz gre");
        terminal_dodaj_linie("  malowanie          - otworz malowanie");
        terminal_dodaj_linie("  zadania            - otworz zarzadzanie zadaniami");
        terminal_dodaj_linie("  duszki             - otworz duszki");
        terminal_dodaj_linie("  edit [plik]        - edytuj plik .c/.txt w terminalu");
        terminal_dodaj_linie("  exit / quit        - zamknij terminal");
    } else if (terminal_starts_with(komenda, "clear")) {
        liczba_linii = 0;
    } else if (terminal_starts_with(komenda, "exit") || terminal_starts_with(komenda, "quit")) {
        okno_zamknij(&okno_terminal);
        terminal_otwarty = 0;
        return;
    } else if (terminal_starts_with(komenda, "ls")) {
        struct fs_entry_info entries[24];
        int count = fs_list_current(entries, 24);
        for (int i = 0; i < count; i++) {
            terminal_dodaj_linie(entries[i].name);
        }
        if (count == 0) terminal_dodaj_linie("(pusto)");
    } else if (terminal_starts_with(komenda, "cd")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            if (cd(arg) == 0) terminal_dodaj_linie("OK");
            else terminal_dodaj_linie("BLAD");
        } else {
            terminal_dodaj_linie("Uzycie: cd [katalog]");
        }
    } else if (terminal_starts_with(komenda, "pwd")) {
        char buf[128];
        fs_get_current_dir_name(buf);
        terminal_dodaj_linie(buf);
    } else if (terminal_starts_with(komenda, "rm")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            if (delete_file(arg) == 0) terminal_dodaj_linie("usunieto");
            else terminal_dodaj_linie("BLAD");
        } else {
            terminal_dodaj_linie("Uzycie: rm [plik]");
        }
    } else if (terminal_starts_with(komenda, "touch")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            if (create_file(arg, 0) == 0) terminal_dodaj_linie("utworzono");
            else terminal_dodaj_linie("BLAD");
        } else {
            terminal_dodaj_linie("Uzycie: touch [plik]");
        }
    } else if (terminal_starts_with(komenda, "mkdir")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            if (create_dir(arg) == 0) terminal_dodaj_linie("utworzono");
            else terminal_dodaj_linie("BLAD");
        } else {
            terminal_dodaj_linie("Uzycie: mkdir [katalog]");
        }
    } else if (terminal_starts_with(komenda, "compile")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            int res = kompiluj_plik(arg, arg);
            if (res == 0) terminal_dodaj_linie("skompilowano");
            else terminal_dodaj_linie("blad kompilacji");
        } else {
            terminal_dodaj_linie("Uzycie: compile [plik.c]");
        }
    } else if (terminal_starts_with(komenda, "run")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            proces_zaladuj_elf(arg);
            terminal_dodaj_linie("uruchomiono");
        } else {
            terminal_dodaj_linie("Uzycie: run [plik]");
        }
    } else if (terminal_starts_with(komenda, "notatnik")) {
        notatnik_otworz();
    } else if (terminal_starts_with(komenda, "pliki") || terminal_starts_with(komenda, "eksplorator")) {
        eksplorator_plikow();
    } else if (terminal_starts_with(komenda, "kalkulator")) {
        kalkulator_otworz();
    } else if (terminal_starts_with(komenda, "browser") || terminal_starts_with(komenda, "www")) {
        browser_open();
    } else if (terminal_starts_with(komenda, "gra")) {
        gra();
    } else if (terminal_starts_with(komenda, "malowanie")) {
        malowanie_otworz();
    } else if (terminal_starts_with(komenda, "zadania")) {
        zadania_otworz();
    } else if (terminal_starts_with(komenda, "duszki")) {
        duszki();
    } else if (terminal_starts_with(komenda, "edit ")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            if (terminal_ma_rozszerzenie(arg, ".C") || terminal_ma_rozszerzenie(arg, ".TXT")) {
                terminal_edytor_wczytaj_plik(arg);
                terminal_tryb_edycji = 1;
                okno_narysuj(&okno_terminal);
            } else {
                terminal_dodaj_linie("Tylko pliki .c i .txt");
            }
        } else {
            terminal_dodaj_linie("Uzycie: edit [plik.c / plik.txt]");
        }
    } else if (terminal_starts_with(komenda, "notatnik")) {
        const char *arg = terminal_pobierz_argument(komenda);
        if (arg) {
            notatnik_otworz_plik(arg);
        } else {
            notatnik_otworz();
        }
    } else if (terminal_starts_with(komenda, "tapeta_zmien")) {
        const char *p = komenda;
        while (*p && *p != ' ') p++;
        while (*p == ' ') p++;
        if (*p) {
            uint32_t kolor = terminal_parsuj_hex(p);
            ustaw_kolor_tapety(kolor);
            terminal_dodaj_linie("Tapeta zmieniona");
        } else {
            terminal_dodaj_linie("Uzycie: tapeta_zmien [kolor hex]");
        }
    } else {
        terminal_dodaj_linie("Nieznana komenda. Wpisz 'help'.");
    }

    dlugosc_wejscia = 0;
    linia_wejscia[0] = 0;
    okno_narysuj(&okno_terminal);
}

static void terminal_narysuj(int x, int y, int szerokosc, int wysokosc)
{
    rysuj_prostokat(x, y, szerokosc, wysokosc, 0x00000000);

    if (terminal_tryb_edycji) {
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X, y + TERMINAL_PADDING_Y,
            "EDYCJA: ", 0x0000FF00);
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X + 64, y + TERMINAL_PADDING_Y,
            terminal_nazwa_pliku, 0x00FFFFFF);
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X, y + TERMINAL_PADDING_Y + 16,
            terminal_edytor_zmieniony ? " [zmieniono]" : "", 0x00FF0000);
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X, y + TERMINAL_PADDING_Y + 32,
            "Strzalki gora/dol | Ctrl+X zapisz | Ctrl+Q wyjdz", 0x00888888);

        int linia_y = y + TERMINAL_PADDING_Y + 48;
        int wysok_linii = TERMINAL_CZIONKA_WYSOKOSC + 2;
        int limit_dol = y + wysokosc - 24;
        int poczatek = terminal_edytor_biezaca_linia > 10 ? terminal_edytor_biezaca_linia - 10 : 0;
        int koniec = poczatek + 20;
        if (koniec > terminal_edytor_liczba_linii) koniec = terminal_edytor_liczba_linii;

        for (int i = poczatek; i < koniec; i++) {
            if (linia_y + TERMINAL_CZIONKA_WYSOKOSC > limit_dol) break;
            char numer[8];
            int n = i + 1;
            numer[0] = '0' + n / 100 % 10;
            numer[1] = '0' + n / 10 % 10;
            numer[2] = '0' + n % 10;
            numer[3] = ':';
            numer[4] = ' ';
            numer[5] = 0;
            rysuj_tekst_chicago(x + TERMINAL_PADDING_X, linia_y, numer, 0x0000FF00);
            uint32_t kolor = (i == terminal_edytor_biezaca_linia) ? 0x00FFFFFF : 0x00888888;
            rysuj_tekst_chicago(x + TERMINAL_PADDING_X + 40, linia_y, terminal_edytor_linie[i], kolor);
            linia_y += wysok_linii;
        }

        int input_y = y + wysokosc - 24;
        rysuj_prostokat(x, input_y, szerokosc, 24, 0x00111111);
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X, input_y + 4, "> ", 0x0000FF00);
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X + 16, input_y + 4,
            terminal_edytor_linie[terminal_edytor_biezaca_linia], 0x00FFFFFF);

        if (okno_terminal.w_polu_tekstowym && kursor_tekstowy_widoczny()) {
            int len = 0;
            while (terminal_edytor_linie[terminal_edytor_biezaca_linia][len]) len++;
            int cursor_x = x + TERMINAL_PADDING_X + 16 + len * 8;
            rysuj_prostokat(cursor_x, input_y + 4, 2, 12, 0x00FFFFFF);
        }
        return;
    }

    int linia_y = y + TERMINAL_PADDING_Y;
    int wysok_linii = TERMINAL_CZIONKA_WYSOKOSC + 2;

    int limit_dol = y + wysokosc - 24;
    for (int i = 0; i < liczba_linii; i++) {
        if (linia_y + TERMINAL_CZIONKA_WYSOKOSC > limit_dol) break;
        rysuj_tekst_chicago(x + TERMINAL_PADDING_X, linia_y, linie_wyjscia[i], 0x00FFFFFF);
        linia_y += wysok_linii;
    }

    int input_y = y + wysokosc - 24;
    rysuj_prostokat(x, input_y, szerokosc, 24, 0x00111111);
    rysuj_tekst_chicago(x + TERMINAL_PADDING_X, input_y + 4, TERMINAL_PROMPT, 0x0000FF00);
    rysuj_tekst_chicago(x + TERMINAL_PADDING_X + 40, input_y + 4, linia_wejscia, 0x00FFFFFF);

    if (okno_terminal.w_polu_tekstowym && kursor_tekstowy_widoczny()) {
        int cursor_x = x + TERMINAL_PADDING_X + 40 + dlugosc_wejscia * 8;
        rysuj_prostokat(cursor_x, input_y + 4, 2, 12, 0x00FFFFFF);
    }
}

static void terminal_pole_tekstowe_enter(struct okno *okno)
{
    (void)okno;
    if (terminal_tryb_edycji) {
        terminal_edytor_nowa_linia();
        okno_narysuj(&okno_terminal);
        return;
    }
    terminal_wykonaj_komende(linia_wejscia);
}

static int terminal_pole_tekstowe_znak(struct okno *okno, char znak, uint8_t ctrl)
{
    (void)okno;
    if (terminal_tryb_edycji) {
        if (ctrl) {
            if (znak == 'x') {
                terminal_edytor_zapisz_plik();
                terminal_tryb_edycji = 0;
                okno_narysuj(&okno_terminal);
                return 1;
            }
            if (znak == 'q') {
                terminal_tryb_edycji = 0;
                okno_narysuj(&okno_terminal);
                return 1;
            }
            return 1;
        }
        if (znak == '\b') {
            terminal_edytor_usun_znak();
        } else if (znak == '\n' || znak == '\r') {
            terminal_edytor_nowa_linia();
        } else if (znak >= 32 && znak <= 126) {
            terminal_edytor_dodaj_znak(znak);
        } else if (znak == 'w' && terminal_edytor_biezaca_linia > 0) {
            terminal_edytor_biezaca_linia--;
        } else if (znak == 's' && terminal_edytor_biezaca_linia < terminal_edytor_liczba_linii - 1) {
            terminal_edytor_biezaca_linia++;
        } else {
            return 1;
        }
        okno_narysuj(&okno_terminal);
        return 1;
    }
    (void)ctrl;
    if (znak == '\b') {
        if (dlugosc_wejscia > 0) {
            dlugosc_wejscia--;
            linia_wejscia[dlugosc_wejscia] = 0;
        }
    } else if (znak == '\n' || znak == '\r') {
        terminal_wykonaj_komende(linia_wejscia);
        return 1;
    } else if (znak >= 32 && znak <= 126 && dlugosc_wejscia < TERMINAL_LINIA_DLUGOSC) {
        linia_wejscia[dlugosc_wejscia] = znak;
        dlugosc_wejscia++;
        linia_wejscia[dlugosc_wejscia] = 0;
    } else {
        return 1;
    }
    okno_narysuj(&okno_terminal);
    return 1;
}

static void terminal_klik(int x, int y)
{
    (void)x;
    (void)y;
    okno_wejdz_do_pola_tekstowego(&okno_terminal);
}

void terminal_otworz_biezacy(void)
{
    if (terminal_otwarty && okno_terminal.otwarte) {
        okno_narysuj(&okno_terminal);
        return;
    }
    terminal_otwarty = 1;
    liczba_linii = 0;
    dlugosc_wejscia = 0;
    linia_wejscia[0] = 0;
    terminal_dodaj_linie("TarixenOS Terminal v1.0");
    terminal_dodaj_linie("Wpisz 'help' aby zobaczyc dostepne komendy");
    terminal_dodaj_linie("");

    int x = (EKRAN_SZEROKOSC - TERMINAL_SZEROKOSC) / 2;
    int y = (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - TERMINAL_WYSOKOSC) / 2;

    okno_stworz(&okno_terminal, x, y, TERMINAL_SZEROKOSC, TERMINAL_WYSOKOSC,
                "TERMINAL", terminal_narysuj);
    proces_rejestruj_okno("TERMINAL", &okno_terminal);
    okno_ustaw_obsluge_klikniecia(&okno_terminal, terminal_klik);
    okno_ustaw_pole_tekstowe_enter(&okno_terminal, (okno_pole_tekstowe)terminal_pole_tekstowe_enter);
    okno_ustaw_pole_tekstowe_znak(&okno_terminal, (okno_pole_tekstowe_znak)terminal_pole_tekstowe_znak);
    okno_ustaw_bufor_tekstu(&okno_terminal, linia_wejscia, 0, TERMINAL_LINIA_DLUGOSC);
}

void terminal_otworz(void)
{
    terminal_otworz_biezacy();
}

void terminal_obsluz_klawiature_znak(char znak)
{
    if (!terminal_otwarty) return;
    if (!okno_terminal.otwarte) {
        terminal_otwarty = 0;
        return;
    }
    if (terminal_tryb_edycji) {
        if (!okno_czy_aktywne(&okno_terminal)) return;
        if (znak == '\n' || znak == '\r') {
            okno_wejdz_do_pola_tekstowego(&okno_terminal);
        }
        if (znak == 'w' && terminal_edytor_biezaca_linia > 0) {
            terminal_edytor_biezaca_linia--;
            okno_narysuj(&okno_terminal);
        }
        if (znak == 's' && terminal_edytor_biezaca_linia < terminal_edytor_liczba_linii - 1) {
            terminal_edytor_biezaca_linia++;
            okno_narysuj(&okno_terminal);
        }
        return;
    }
    if (okno_terminal.w_polu_tekstowym) return;
    if (!okno_czy_aktywne(&okno_terminal)) return;
    if (znak == '\n' || znak == '\r') {
        okno_wejdz_do_pola_tekstowego(&okno_terminal);
    }
}
