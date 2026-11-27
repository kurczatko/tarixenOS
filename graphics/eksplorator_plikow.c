

#include <graphics/eksplorator_plikow.h>
#include <graphics/okno.h>
#include <graphics/rysowanie.h>
#include <graphics/czcionka.h>
#include <fs/system_plikow.h>
#include <kernel/apps/installer.h>
#include <kernel/apps/notatnik.h>
#include <drivers/input/keyboard.h>
#include <tools/cc_os.h>
#include <drivers/storage/floppy.h>
#include <drivers/storage/plyty.h>
#include <drivers/usb/storage/storage.h>
#include <kernel/core/proces.h>
#include <kernel/loader/loader.h>

struct okno pliki;
static int wybrany_element = -1;
static struct fs_entry_info lista_plikow[24];
static int liczba_plikow;
uint8_t tryb_tworz;
static char nazwa_nowego[9];
static int dlugosc_nazwy;
static char status_plikow[24];
static int view_floppy = 0;
static int view_cdrom = 0;
static int view_usb = 0;

static int ma_rozszerzenie(const char *nazwa, const char *rozszerzenie)
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

static void rysuj_zawartosc_plikow(int x, int y, int szerokosc, int wysokosc)
{
    (void)szerokosc;
    liczba_plikow = fs_list_current(lista_plikow, 24);

    rysuj_tekst_chicago(x, y, "PLIKI", 0x00000000);
    rysuj_prostokat(x, y + 14, 78, 20, 0x00334155);
    rysuj_tekst_chicago(x + 8, y + 20, "FOLDER+", 0x00FFFFFF);
    rysuj_prostokat(x + 84, y + 14, 78, 20, 0x00334155);
    rysuj_tekst_chicago(x + 100, y + 20, "TXT+", 0x00FFFFFF);
    rysuj_prostokat(x + 168, y + 14, 56, 20, 0x00334155);
    rysuj_tekst_chicago(x + 184, y + 20, "WROC", 0x00FFFFFF);
    rysuj_prostokat(x + 228, y + 14, 56, 20, 0x00334155);
    rysuj_tekst_chicago(x + 240, y + 20, "USUN", 0x00FFFFFF);

    if (tryb_tworz) {
        rysuj_tekst_chicago(x, y + 40, "NAZWA:", 0x00000000);
        rysuj_tekst_chicago(x + 56, y + 40, nazwa_nowego, 0x00000000);
    } else {
        rysuj_tekst_chicago(x, y + 40, status_plikow, 0x00000000);
    }

    int row_start = y + 58;
    if (!view_floppy && floppy_present()) {
        if (wybrany_element == -2) {
            rysuj_prostokat(x, row_start, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_start, "DYSKIETKA", 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_start, "DYSKIETKA", 0x00000000);
        }
        row_start += 16;
    }
    if (view_floppy) {
        if (wybrany_element == -2) {
            rysuj_prostokat(x, row_start, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_start, "DYSK TWARDY", 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_start, "DYSK TWARDY", 0x00000000);
        }
        row_start += 16;
    }
    if (!view_cdrom && plyty_present()) {
        if (wybrany_element == -3) {
            rysuj_prostokat(x, row_start, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_start, "CD-ROM", 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_start, "CD-ROM", 0x00000000);
        }
        row_start += 16;
    }
    if (view_cdrom) {
        if (wybrany_element == -3) {
            rysuj_prostokat(x, row_start, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_start, "PLYTA OPTYCZNA", 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_start, "PLYTA OPTYCZNA", 0x00000000);
        }
        row_start += 16;
    }
    if (!view_usb && usb_storage_present()) {
        if (wybrany_element == -4) {
            rysuj_prostokat(x, row_start, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_start, "PENDRIVE USB", 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_start, "PENDRIVE USB", 0x00000000);
        }
        row_start += 16;
    }
    if (view_usb) {
        if (wybrany_element == -4) {
            rysuj_prostokat(x, row_start, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_start, "DYSK TWARDY", 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_start, "DYSK TWARDY", 0x00000000);
        }
        row_start += 16;
    }

    for (int i = 0; i < liczba_plikow; i++) {
        int row_y = row_start + i * 16;
        if (row_y + 8 > y + wysokosc) break;
        if (i == wybrany_element) {
            rysuj_prostokat(x, row_y, 250, 14, 0x003B82F6);
            rysuj_tekst_chicago(x, row_y, lista_plikow[i].name, 0x00FFFFFF);
        } else {
            rysuj_tekst_chicago(x, row_y, lista_plikow[i].name, 0x00000000);
        }
        if (lista_plikow[i].is_directory) rysuj_tekst_chicago(x + 176, row_y, "<DIR>", 0x00000000);
    }
}

static void obsluz_klik_plikow(int x, int y)
{
    if (tryb_tworz) return;
    if (y >= 52 && y < 72) {
        if (x >= 16 && x < 94) tryb_tworz = 1;
        else if (x >= 100 && x < 178) tryb_tworz = 2;
        else if (x >= 184 && x < 224) fs_parent_directory();
        else if (x >= 228 && x < 284 && wybrany_element >= 0 && wybrany_element < liczba_plikow) {
            delete_file(lista_plikow[wybrany_element].name);
            wybrany_element = -1;
        }
        nazwa_nowego[0] = 0;
        dlugosc_nazwy = 0;
        wybrany_element = -1;
        okno_narysuj(&pliki);
        return;
    }

    int toggle_row = 88;
    int floppy_visible = (!view_floppy && floppy_present()) || view_floppy;
    int cdrom_visible = (!view_cdrom && plyty_present()) || view_cdrom;
    int usb_visible = (!view_usb && usb_storage_present()) || view_usb;

    int cur_row = toggle_row;
    if (floppy_visible) {
        if (y >= cur_row && y < cur_row + 14) {
            view_floppy = !view_floppy;
            view_cdrom = 0;
            view_usb = 0;
            fs_set_device_cdrom(0);
            fs_set_device_usb(0);
            fs_set_device_floppy(view_floppy);
            fs_reset_directory();
            wybrany_element = -2;
            okno_narysuj(&pliki);
            return;
        }
        cur_row += 16;
    }
    if (cdrom_visible) {
        if (y >= cur_row && y < cur_row + 14) {
            view_cdrom = !view_cdrom;
            view_floppy = 0;
            view_usb = 0;
            fs_set_device_floppy(0);
            fs_set_device_usb(0);
            fs_set_device_cdrom(view_cdrom);
            fs_reset_directory();
            wybrany_element = -3;
            okno_narysuj(&pliki);
            return;
        }
        cur_row += 16;
    }
    if (usb_visible) {
        if (y >= cur_row && y < cur_row + 14) {
            view_usb = !view_usb;
            view_floppy = 0;
            view_cdrom = 0;
            fs_set_device_floppy(0);
            fs_set_device_cdrom(0);
            fs_set_device_usb(view_usb);
            fs_reset_directory();
            wybrany_element = -4;
            okno_narysuj(&pliki);
            return;
        }
        cur_row += 16;
    }

    int toggle_count = floppy_visible + cdrom_visible + usb_visible;
    int file_base = 88 + toggle_count * 16;
    int klikniety_index = (y - file_base) / 16;
    if (klikniety_index < 0) klikniety_index = -1;

    if (klikniety_index >= 0 && klikniety_index < liczba_plikow) {
        if (ma_rozszerzenie(lista_plikow[klikniety_index].name, ".C")) {
            if (wybrany_element == klikniety_index) {
                kompiluj_i_uruchom(lista_plikow[klikniety_index].name);
            } else {
                notatnik_otworz_plik(lista_plikow[klikniety_index].name);
                wybrany_element = klikniety_index;
            }
        } else if (wybrany_element == klikniety_index) {
            if (lista_plikow[klikniety_index].is_directory) {
                if (cd(lista_plikow[klikniety_index].name) == 0) wybrany_element = -1;
            } else if (ma_rozszerzenie(lista_plikow[klikniety_index].name, ".TXT")) {
                notatnik_otworz_plik(lista_plikow[klikniety_index].name);
            } else if (ma_rozszerzenie(lista_plikow[klikniety_index].name, ".BMP")) {
                tapeta_ustaw_bmp(lista_plikow[klikniety_index].name);
            } else if (ma_rozszerzenie(lista_plikow[klikniety_index].name, ".TSI")) {
                zainstaluj_pakiet_tsi(lista_plikow[klikniety_index].name);
            } else if (ma_rozszerzenie(lista_plikow[klikniety_index].name, ".ELF")) {
                proces_zaladuj_elf(lista_plikow[klikniety_index].name);
            }
        } else {
            wybrany_element = klikniety_index;
        }
    } else {
        wybrany_element = -1;
    }
    okno_narysuj(&pliki);
}

void eksplorator_plikow(void)
{
    fs_reset_directory();
    fs_set_device_floppy(0);
    fs_set_device_cdrom(0);
    fs_set_device_usb(0);
    view_floppy = 0;
    view_cdrom = 0;
    view_usb = 0;
    wybrany_element = -1;
    tryb_tworz = 0;
    status_plikow[0] = 0;
    okno_stworz(&pliki, 300, 300, 300, 300, "Pliki", rysuj_zawartosc_plikow);
    proces_rejestruj_okno("PLIKI", &pliki);
    okno_ustaw_obsluge_klikniecia(&pliki, obsluz_klik_plikow);
}

void eksplorator_plikow_obsluz_klawiature_znak(char znak)
{
    if (!okno_czy_aktywne(&pliki)) return;
    if (tryb_tworz) {
        if (znak == '\b') {
            if (dlugosc_nazwy > 0) nazwa_nowego[--dlugosc_nazwy] = 0;
        } else if (znak == '\n' || znak == '\r') {
            if (dlugosc_nazwy > 0) {
                if (tryb_tworz == 1) {
                    const char *status = create_dir(nazwa_nowego) == 0 ? "OK" : "BLAD";
                    int j = 0;
                    while (status[j]) { status_plikow[j] = status[j]; j++; }
                    status_plikow[j] = 0;
                } else if (tryb_tworz == 2) {
                    char nazwa_pliku[13];
                    int i;
                    for (i = 0; i < dlugosc_nazwy; i++) nazwa_pliku[i] = nazwa_nowego[i];
                    nazwa_pliku[i++] = '.';
                    nazwa_pliku[i++] = 'T';
                    nazwa_pliku[i++] = 'X';
                    nazwa_pliku[i++] = 'T';
                    nazwa_pliku[i] = 0;
                    const char *status = create_file(nazwa_pliku, 0) == 0 ? "OK" : "BLAD";
                    int j = 0;
                    while (status[j]) { status_plikow[j] = status[j]; j++; }
                    status_plikow[j] = 0;
                } else if (tryb_tworz == 3) {
                    char nazwa_pliku[13];
                    int i;
                    for (i = 0; i < dlugosc_nazwy; i++) nazwa_pliku[i] = nazwa_nowego[i];
                    nazwa_pliku[i++] = '.';
                    nazwa_pliku[i++] = 'C';
                    nazwa_pliku[i] = 0;
                    const char *status = create_file(nazwa_pliku, 0) == 0 ? "OK" : "BLAD";
                    int j = 0;
                    while (status[j]) { status_plikow[j] = status[j]; j++; }
                    status_plikow[j] = 0;
                }
            }
            tryb_tworz = 0;
            dlugosc_nazwy = 0;
            nazwa_nowego[0] = 0;
        } else {
            if (znak >= 'A' && znak <= 'Z') znak += 'a' - 'A';
            if (((znak >= 'a' && znak <= 'z') || (znak >= '0' && znak <= '9')) && dlugosc_nazwy < 8) {
                nazwa_nowego[dlugosc_nazwy++] = znak;
                nazwa_nowego[dlugosc_nazwy] = 0;
            }
        }
    } else if (znak == 'd' && wybrany_element >= 0 && wybrany_element < liczba_plikow) {
        delete_file(lista_plikow[wybrany_element].name);
        wybrany_element = -1;
    }
    okno_narysuj(&pliki);
}
