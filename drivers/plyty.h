#ifndef PLITY_H
#define PLITY_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PLITY_CDROM     0x01
#define PLITY_DVDROM    0x02
#define PLITY_BDROM     0x04
#define PLITY_HDDVD     0x08
#define PLITY_CDI       0x10
#define PLITY_PHOTOCD   0x20
#define PLITY_CDR       0x40
#define PLITY_CDRW      0x80
#define PLITY_DVDR      0x100
#define PLITY_DVDRW     0x200
#define PLITY_BDR       0x400
#define PLITY_BDRE      0x800
#define PLITY_MO        0x1000
#define PLITY_WO        0x2000

#define PLITY_FS_ISO9660   0x01
#define PLITY_FS_UDF       0x02
#define PLITY_FS_JOLIET    0x04
#define PLITY_FS_ROCKRIDGE 0x08
#define PLITY_FS_HFS       0x10
#define PLITY_FS_CDFS      0x20

struct plyty_disc {
    uint32_t type;
    uint32_t fs_type;
    uint64_t sector_count;
    uint32_t sector_size;
    uint8_t present;
    uint8_t ready;
    char volume_label[128];
};

struct plyty_file_entry {
    char name[256];
    uint8_t is_directory;
    uint64_t size;
    uint64_t sector;
};

int plyty_init(void);
int plyty_present(void);
int plyty_ready(void);
uint32_t plyty_type(void);
uint64_t plyty_sector_count(void);
uint32_t plyty_sector_size(void);
int plyty_read_sector(uint64_t sector, char *buffer);
int plyty_read_sectors(uint64_t sector, uint32_t count, char *buffer);
const char *plyty_volume_label(void);
uint32_t plyty_fs_type(void);
int plyty_list_root(struct plyty_file_entry *entries, int capacity);
int plyty_find_file(const char *name, struct plyty_file_entry *entry);
int plyty_read_file(const char *name, char *buffer, uint64_t size);
int plyty_disc_changed(void);
void plyty_eject(void);
void plyty_close_tray(void);

#ifdef __cplusplus
}
#endif

#endif
