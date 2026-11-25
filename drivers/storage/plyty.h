/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#ifndef PLYTY_H
#define PLYTY_H

#ifdef __cplusplus
extern "C" {
#endif

enum plyty_typ
{
    PLY_TYP_BRAK = 0,
    PLY_TYP_CD_ROM,
    PLY_TYP_CD_R,
    PLY_TYP_CD_RW,
    PLY_TYP_DVD_ROM,
    PLY_TYP_DVD_R,
    PLY_TYP_DVD_RW,
    PLY_TYP_DVD_RAM,
    PLY_TYP_DVD_PLUS_R,
    PLY_TYP_DVD_PLUS_RW,
    PLY_TYP_DVD_DL,
    PLY_TYP_BD_ROM,
    PLY_TYP_BD_R,
    PLY_TYP_BD_RE,
    PLY_TYP_INNY
};

struct plyty_wpis
{
    char nazwa[256];
    unsigned long long rozmiar;
    unsigned long long lba;
    unsigned int atrybuty;
    unsigned char katalog;
};

void plyty_init(void);
bool plyty_present(void);
bool plyty_ready(void);
enum plyty_typ plyty_type(void);
unsigned long long plyty_sector_count(void);
unsigned int plyty_sector_size(void);
bool plyty_read_sector(unsigned long long lba, void* bufor);
bool plyty_read_sectors(unsigned long long lba, unsigned int liczba, void* bufor);
const char* plyty_volume_label(void);
const char* plyty_fs_type(void);
bool plyty_list_root(struct plyty_wpis* wpisy, unsigned int maks, unsigned int* liczba);
bool plyty_find_file(const char* sciezka, struct plyty_wpis* wynik);
bool plyty_read_file(const char* sciezka, void* bufor, unsigned long long pojemnosc, unsigned long long* odczytano);
bool plyty_disc_changed(void);
bool plyty_eject(void);
bool plyty_close_tray(void);

#ifdef __cplusplus
}
#endif

#endif
