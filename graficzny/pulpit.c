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

struct pulp_ikona {
    char nazwa[13];
    uint8_t jest_folder;
    int x;
    int y;
};

static struct pulp_ikona ikony[PULPIT_MAX_ICONI];
static int liczba_ikon;

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
    int kolumna = 0;
    int wiersz = 0;
    for (int i = 0; i < liczba_ikon; i++) {
        ikony[i].x = 20 + kolumna * PULPIT_ICON_W;
        ikony[i].y = 20 + wiersz * PULPIT_ICON_H;
        for (int j = 0; j < 13; j++) ikony[i].nazwa[j] = lista[i].name[j];
        ikony[i].nazwa[12] = 0;
        ikony[i].jest_folder = lista[i].is_directory;
        kolumna++;
        if (kolumna >= 14) {
            kolumna = 0;
            wiersz++;
        }
    }
    fs_set_current_dir(stary_dir);
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
    pulp_laduj_ikony();
    pulp_rysuj_wszystkie_ikony();
}

void pulp_obsluz_klik(int x, int y)
{
    for (int i = 0; i < liczba_ikon; i++) {
        if (x >= ikony[i].x && x < ikony[i].x + PULPIT_ICON_W &&
            y >= ikony[i].y && y < ikony[i].y + PULPIT_ICON_H) {
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
            return;
        }
    }
}

void pulp_odswiez(void)
{
    pulp_rysuj_ikony();
}
