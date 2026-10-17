#include "pulpit.h"
#include "okno.h"
#include "rysowanie.h"
#include "czcionka.h"
#include "../system_plikow/system_plikow.h"
#include "../drivers/keyboard.h"
#include "../kernel/notatnik.h"

#define PULPIT_ICON_W 64
#define PULPIT_ICON_H 80
#define PULPIT_MAX_ICONI 32

#define PLIK_UKLAD "UKLAD.CFG"

struct pulp_ikona {
    char nazwa[13];
    uint8_t jest_folder;
    int x;
    int y;
};

static struct pulp_ikona ikony[PULPIT_MAX_ICONI];
static int liczba_ikon;

static int przeciąganie_ikona = -1;
static int przeciąganie_offset_x;
static int przeciąganie_offset_y;
static int przeciąganie_ruch;

struct saved_pos {
    char nazwa[13];
    int x;
    int y;
    uint8_t jest_folder;
};
static struct saved_pos saved[PULPIT_MAX_ICONI];
static int saved_count;

static int nazwy_rowne(const char *a, const char *b)
{
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

static int parsuj_uint(char **p)
{
    int v = 0;
    while (**p >= '0' && **p <= '9') {
        v = v * 10 + (**p - '0');
        (*p)++;
    }
    return v;
}

static void zapisz_uint(char *buf, int *pos, int v)
{
    if (v == 0) {
        buf[(*pos)++] = '0';
        return;
    }
    char tmp[8];
    int t = 0;
    while (v > 0) {
        tmp[t++] = (char)('0' + v % 10);
        v /= 10;
    }
    while (t > 0) buf[(*pos)++] = tmp[--t];
}

static void pulp_laduj_ikony(void)
{
    static struct fs_entry_info lista[PULPIT_MAX_ICONI];
    uint32_t stary_dir = fs_get_current_dir();
    fs_reset_directory();
    if (cd("pulpit") != 0) {
        create_dir("pulpit");
        cd("pulpit");
    }
    liczba_ikon = fs_list_current(lista, PULPIT_MAX_ICONI);

    ikony_wczytaj_uklad();

    int kolumna = 0;
    int wiersz = 0;
    for (int i = 0; i < liczba_ikon; i++) {
        for (int j = 0; j < 13; j++) ikony[i].nazwa[j] = lista[i].name[j];
        ikony[i].nazwa[12] = 0;
        ikony[i].jest_folder = lista[i].is_directory;
        ikony[i].x = 20 + kolumna * PULPIT_ICON_W;
        ikony[i].y = 20 + wiersz * PULPIT_ICON_H;
        kolumna++;
        if (kolumna >= 14) {
            kolumna = 0;
            wiersz++;
        }
        for (int s = 0; s < saved_count; s++) {
            if (nazwy_rowne(ikony[i].nazwa, saved[s].nazwa)) {
                ikony[i].x = saved[s].x;
                ikony[i].y = saved[s].y;
                break;
            }
        }
    }
    fs_set_current_dir(stary_dir);
}

int ikony_wczytaj_uklad(void)
{
    fs_reset_directory();
    char buf[1024];
    int n = read_file(PLIK_UKLAD, buf, sizeof(buf) - 1);
    if (n <= 0) return 0;
    buf[n] = 0;
    char *p = buf;
    saved_count = 0;
    while (*p && saved_count < PULPIT_MAX_ICONI) {
        while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
        if (!*p) break;
        int j = 0;
        while (*p && *p != ' ' && *p != '\n' && *p != '\r' && *p != '\t' && j < 12) {
            saved[saved_count].nazwa[j++] = *p++;
        }
        saved[saved_count].nazwa[j] = 0;
        while (*p == ' ') p++;
        saved[saved_count].x = parsuj_uint(&p);
        while (*p == ' ') p++;
        saved[saved_count].y = parsuj_uint(&p);
        while (*p == ' ') p++;
        saved[saved_count].jest_folder = (uint8_t)(parsuj_uint(&p) != 0 ? 1 : 0);
        saved_count++;
        while (*p && *p != '\n' && *p != '\r') p++;
    }
    return saved_count > 0;
}

void ikony_zapisz_uklad(void)
{
    char buf[1024];
    int pos = 0;
    for (int i = 0; i < liczba_ikon; i++) {
        int j = 0;
        while (ikony[i].nazwa[j]) {
            buf[pos++] = ikony[i].nazwa[j];
            j++;
        }
        buf[pos++] = ' ';
        zapisz_uint(buf, &pos, ikony[i].x);
        buf[pos++] = ' ';
        zapisz_uint(buf, &pos, ikony[i].y);
        buf[pos++] = ' ';
        buf[pos++] = ikony[i].jest_folder ? '1' : '0';
        buf[pos++] = '\n';
    }
    buf[pos] = 0;
    fs_reset_directory();
    if (write_file(PLIK_UKLAD, buf, (uint32_t)pos) != 0) {
        if (create_file(PLIK_UKLAD, (uint32_t)pos) != 0 ||
            write_file(PLIK_UKLAD, buf, (uint32_t)pos) != 0) {
            return;
        }
    }
}

static void pulp_rysuj_ikone(int x, int y, const char *nazwa, uint8_t jest_folder)
{
    uint32_t kolor = jest_folder ? 0x004A5D6E : 0x00334155;
    rysuj_prostokat(x, y, 48, 48, kolor);
    rysuj_prostokat(x + 4, y + 4, 40, 40, 0x00FFFFFF);
    if (jest_folder) {
        rysuj_prostokat(x + 12, y + 12, 24, 12, 0x00FBBF24);
    } else {
        rysuj_prostokat(x + 12, y + 16, 24, 8, 0x00FBBF24);
        rysuj_prostokat(x + 12, y + 28, 24, 8, 0x00FBBF24);
    }
    rysuj_tekst_chicago(x, y + 52, nazwa, 0x00000000);
}

static void pulp_rysuj_wszystkie_ikony(void)
{
    for (int i = 0; i < liczba_ikon; i++) {
        pulp_rysuj_ikone(ikony[i].x, ikony[i].y, ikony[i].nazwa, ikony[i].jest_folder);
    }
    odswiez_widok();
}

void pulp_init(void)
{
    fs_reset_directory();
    if (cd("pulpit") != 0) {
        create_dir("pulpit");
        cd("pulpit");
    }
    pulp_laduj_ikony();
    cd("..");
    pulp_rysuj_wszystkie_ikony();
}

void pulp_rysuj_ikony(void)
{
    pulp_rysuj_wszystkie_ikony();
}

void pulp_odswiez(void)
{
    pulp_rysuj_wszystkie_ikony();
}

int pulp_mouse_press(int x, int y)
{
    for (int i = 0; i < liczba_ikon; i++) {
        if (x >= ikony[i].x && x < ikony[i].x + PULPIT_ICON_W &&
            y >= ikony[i].y && y < ikony[i].y + PULPIT_ICON_H) {
            przeciąganie_ikona = i;
            przeciąganie_offset_x = x - ikony[i].x;
            przeciąganie_offset_y = y - ikony[i].y;
            przeciąganie_ruch = 0;
            return 1;
        }
    }
    przeciąganie_ikona = -1;
    przeciąganie_ruch = 0;
    return 0;
}

void pulp_mouse_ruch(int x, int y)
{
    if (przeciąganie_ikona < 0) return;
    int i = przeciąganie_ikona;
    int nx = x - przeciąganie_offset_x;
    int ny = y - przeciąganie_offset_y;
    if (nx < 0) nx = 0;
    if (ny < 0) ny = 0;
    if (nx + PULPIT_ICON_W > EKRAN_SZEROKOSC) nx = EKRAN_SZEROKOSC - PULPIT_ICON_W;
    if (ny + PULPIT_ICON_H > EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC)
        ny = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC - PULPIT_ICON_H;
    if (nx == ikony[i].x && ny == ikony[i].y) return;

    odtworz_fragment_tla(ikony[i].x, ikony[i].y, PULPIT_ICON_W, PULPIT_ICON_H);
    ikony[i].x = nx;
    ikony[i].y = ny;
    przeciąganie_ruch = 1;
    pulp_rysuj_ikone(nx, ny, ikony[i].nazwa, ikony[i].jest_folder);
    odswiez_fragment_widoku(nx, ny, PULPIT_ICON_W, PULPIT_ICON_H);
}

void pulp_mouse_release(int x, int y)
{
    (void)x;
    (void)y;
    if (przeciąganie_ikona >= 0) {
        int i = przeciąganie_ikona;
        przeciąganie_ikona = -1;
        if (przeciąganie_ruch) {
            ikony_zapisz_uklad();
        } else {
            uint32_t stary_dir = fs_get_current_dir();
            fs_reset_directory();
            cd("pulpit");
            if (ikony[i].jest_folder) {
                cd(ikony[i].nazwa);
            } else {
                notatnik_otworz_plik(ikony[i].nazwa);
            }
            cd("..");
            fs_set_current_dir(stary_dir);
        }
        przeciąganie_ruch = 0;
        return;
    }
}
