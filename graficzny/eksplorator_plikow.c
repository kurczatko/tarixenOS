#include "eksplorator_plikow.h"
#include "okno.h"
#include "rysowanie.h"
#include "czcionka.h"
#include "../system_plikow/system_plikow.h"
#include "../kernel/apps/installer.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct okno pliki;
static int wybrany_element = -1;
static char lista_plikow[16][13];
static int liczba_plikow = 0;

extern void read_sector(uint32_t sector, char *buffer);

struct fat32_bpb_local {
    uint8_t jump[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t small_sectors;
    uint8_t media;
    uint16_t small_fat_size;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t sectors;
    uint32_t fat_size;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
} __attribute__((packed));

struct fat32_entry_local {
    char name[11]; 
    uint8_t attributes; 
    uint8_t reserved; 
    uint8_t create_time_tenth;
    uint16_t create_time; 
    uint16_t create_date; 
    uint16_t access_date;
    uint16_t cluster_high; 
    uint16_t modify_time; 
    uint16_t modify_date;
    uint16_t cluster_low; 
    uint32_t size;
} __attribute__((packed));

static uint32_t pobierz_sektor_klastra(uint32_t cluster) {
    char bpb_sector[512];
    read_sector(0, bpb_sector);
    struct fat32_bpb_local *bpb = (struct fat32_bpb_local *)bpb_sector;
    uint32_t first_data_sector = bpb->reserved_sectors + (bpb->fat_count * bpb->fat_size);
    return first_data_sector + (cluster - 2) * bpb->sectors_per_cluster;
}

static int pobierz_pliki_lokalnie(char nazwy[][13], int max_plikow) {
    char sector[512];
    struct fat32_entry_local *entries;
    int licznik = 0;
    
    read_sector(pobierz_sektor_klastra(2), sector);
    entries = (struct fat32_entry_local *)sector;
    
    for (uint32_t i = 0; i < 512 / sizeof(struct fat32_entry_local); i++) {
        if (entries[i].name[0] == 0) break;
        if (entries[i].name[0] != '.' && entries[i].attributes != 0x0F && (uint8_t)entries[i].name[0] != 0xE5) {
            if (licznik < max_plikow) {
                int idx = 0;
                for (int j = 0; j < 11; j++) {
                    if (entries[i].name[j] != ' ') {
                        nazwy[licznik][idx++] = entries[i].name[j];
                    }
                }
                nazwy[licznik][idx] = '\0';
                licznik++;
            }
        }
    }
    return licznik;
}

static int czy_to_tsi(const char *nazwa) {
    int len = 0;
    while (nazwa[len] != '\0') {
        len++;
    }
    if (len > 4 && nazwa[len - 4] == '.' && 
        (nazwa[len - 3] == 't' || nazwa[len - 3] == 'T') &&
        (nazwa[len - 2] == 's' || nazwa[len - 2] == 'S') &&
        (nazwa[len - 1] == 'i' || nazwa[len - 1] == 'I')) {
        return 1;
    }
    return 0;
}

static void rysuj_zawartosc_plikow(int x, int y, int szerokosc, int wysokosc) {
    (void)szerokosc;
    (void)wysokosc;
    liczba_plikow = pobierz_pliki_lokalnie(lista_plikow, 16);
    
    rysuj_tekst_8x8(x, y, "Pliki na dysku:", 0x00000000);
    
    for (int i = 0; i < liczba_plikow; i++) {
        if (i == wybrany_element) {
            rysuj_prostokat(x, y + 20 + (i * 16), 250, 14, 0x003B82F6);
            rysuj_tekst_8x8(x, y + 20 + (i * 16), lista_plikow[i], 0x00FFFFFF);
        } else {
            rysuj_tekst_8x8(x, y + 20 + (i * 16), lista_plikow[i], 0x00000000);
        }
    }
}

static void obsluz_klik_plikow(int x, int y) {
    (void)x;
    int klikniety_index = (y - 20) / 16;
    if (klikniety_index >= 0 && klikniety_index < liczba_plikow) {
        if (wybrany_element == klikniety_index) {
            if (czy_to_tsi(lista_plikow[klikniety_index])) {
                zainstaluj_pakiet_tsi(lista_plikow[klikniety_index]);
            }
        } else {
            wybrany_element = klikniety_index;
            okno_narysuj(&pliki);
        }
    } else {
        wybrany_element = -1;
        okno_narysuj(&pliki);
    }
}

void eksplorator_plikow() {
    okno_stworz(&pliki, 300, 300, 300, 300, "Pliki", rysuj_zawartosc_plikow);
    okno_ustaw_obsluge_klikniecia(&pliki, obsluz_klik_plikow);
}