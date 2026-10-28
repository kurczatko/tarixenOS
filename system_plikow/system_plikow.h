#ifndef SYSTEM_PLIKOW_H
#define SYSTEM_PLIKOW_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define FAT32_SECTOR_SIZE 512
#define FAT32_ROOT_CLUSTER 2

struct fs_entry_info {
	char name[13];
	uint8_t is_directory;
};

#define FAT32_EOC 0x0FFFFFF8
struct fat32_entry {
    char name[11]; uint8_t attributes; uint8_t reserved; uint8_t create_time_tenth;
    uint16_t create_time; uint16_t create_date; uint16_t access_date;
    uint16_t cluster_high; uint16_t modify_time; uint16_t modify_date;
    uint16_t cluster_low; uint32_t size;
} __attribute__((packed));

void init_fs(void);
int create_file(const char *name, uint32_t size);
int read_file(const char *name, char *buffer, uint32_t size);
int write_file(const char *name, const char *data, uint32_t size);
int create_dir(const char *name);
int ls(const char *dir);
int cd(const char *dir);
int fs_list_current(struct fs_entry_info *entries, int capacity);
void fs_reset_directory(void);
int fs_parent_directory(void);
uint32_t fs_get_current_dir(void);
void fs_set_current_dir(uint32_t dir);
uint32_t fs_fat_next(uint32_t cluster);
void fs_read_sector(uint32_t sector, char *buffer);
void fs_write_sector(uint32_t sector, const char *buffer);
uint32_t fs_cluster_sector(uint32_t cluster);
struct fat32_entry *fs_find_entry(const char *name, char *sector, uint32_t *sector_number);
uint32_t fs_alloc_cluster(void);

void fs_set_device_floppy(int use_floppy);
int fs_is_floppy(void);

#endif
