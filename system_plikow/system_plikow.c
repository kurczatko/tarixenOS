#include "system_plikow.h"
#include "../include/printf/printf.h"

#define FAT32_EOC 0x0FFFFFF8
#define FAT32_FREE 0
#define ATA_STATUS 0x1F7

struct fat32_bpb {
    uint8_t jump[3]; char oem[8]; uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster; uint16_t reserved_sectors; uint8_t fat_count;
    uint16_t root_entries; uint16_t small_sectors; uint8_t media;
    uint16_t small_fat_size; uint16_t sectors_per_track; uint16_t heads;
    uint32_t hidden_sectors; uint32_t sectors; uint32_t fat_size;
    uint16_t flags; uint16_t version; uint32_t root_cluster;
    uint16_t fs_info; uint16_t backup_boot; uint8_t reserved[12];
    uint8_t drive; uint8_t reserved2; uint8_t signature; uint32_t serial;
    char label[11]; char type[8];
} __attribute__((packed));

struct fat32_entry {
    char name[11]; uint8_t attributes; uint8_t reserved; uint8_t create_time_tenth;
    uint16_t create_time; uint16_t create_date; uint16_t access_date;
    uint16_t cluster_high; uint16_t modify_time; uint16_t modify_date;
    uint16_t cluster_low; uint32_t size;
} __attribute__((packed));

static struct fat32_bpb bpb;
static uint32_t current_dir = FAT32_ROOT_CLUSTER;
static uint32_t first_data_sector;
static uint32_t fat_start;

static int memory_compare(const void *left, const void *right, uint32_t count) {
    const uint8_t *a = left;
    const uint8_t *b = right;
    while (count--) {
        if (*a != *b) return *a - *b;
        a++;
        b++;
    }
    return 0;
}

static void outb(uint16_t port, uint8_t value) { __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port)); }
static uint8_t inb(uint16_t port) { uint8_t value; __asm__ volatile ("inb %1, %0" : "=a"(value) : "d"(port)); return value; }
static void ata_wait(void) { while (inb(ATA_STATUS) & 0x80) { } }

void read_sector(uint32_t sector, char *buffer) {
    uint32_t i; uint16_t word;
    ata_wait(); outb(0x1F2, 1); outb(0x1F3, sector); outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16); outb(0x1F6, 0xE0 | ((sector >> 24) & 0x0F)); outb(0x1F7, 0x20);
    while (!(inb(ATA_STATUS) & 8)) { }
    for (i = 0; i < 256; i++) { __asm__ volatile ("inw %1, %0" : "=a"(word) : "d"((uint16_t)0x1F0)); buffer[i * 2] = word; buffer[i * 2 + 1] = word >> 8; }
}

void write_sector(uint32_t sector, const char *buffer) {
    uint32_t i; uint16_t word;
    ata_wait(); outb(0x1F2, 1); outb(0x1F3, sector); outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16); outb(0x1F6, 0xE0 | ((sector >> 24) & 0x0F)); outb(0x1F7, 0x30);
    while (!(inb(ATA_STATUS) & 8)) { }
    for (i = 0; i < 256; i++) { word = (uint8_t)buffer[i * 2] | ((uint16_t)(uint8_t)buffer[i * 2 + 1] << 8); __asm__ volatile ("outw %0, %1" : : "a"(word), "d"((uint16_t)0x1F0)); }
    outb(0x1F7, 0xE7); ata_wait();
}

static uint32_t cluster_sector(uint32_t cluster) { return first_data_sector + (cluster - 2) * bpb.sectors_per_cluster; }

static uint32_t fat_entry(uint32_t cluster) {
    char sector[512]; uint32_t value;
    read_sector(fat_start + (cluster * 4) / 512, sector);
    value = *(uint32_t *)(sector + (cluster * 4) % 512);
    return value & 0x0FFFFFFF;
}

static void set_fat_entry(uint32_t cluster, uint32_t value) {
    char sector[512]; uint32_t offset = (cluster * 4) % 512;
    uint32_t sector_number = fat_start + (cluster * 4) / 512; uint8_t copy;
    read_sector(sector_number, sector); *(uint32_t *)(sector + offset) = value; write_sector(sector_number, sector);
    for (copy = 1; copy < bpb.fat_count; copy++) write_sector(sector_number + copy * bpb.fat_size, sector);
}

static uint32_t alloc_cluster(void) {
    uint32_t cluster; char sector[512];
    for (cluster = 2; cluster < bpb.sectors / bpb.sectors_per_cluster + 2; cluster++) {
        if (fat_entry(cluster) == FAT32_FREE) {
            set_fat_entry(cluster, FAT32_EOC);
            for (uint32_t i = 0; i < bpb.sectors_per_cluster; i++) {
                for (uint32_t j = 0; j < 512; j++) sector[j] = 0;
                write_sector(cluster_sector(cluster) + i, sector);
            }
            return cluster;
        }
    }
    return 0;
}

static void make_name(const char *input, char output[11]) {
    uint32_t i = 0; for (i = 0; i < 11; i++) output[i] = ' ';
    i = 0; while (*input && *input != '.' && i < 8) output[i++] = *input++;
    if (*input == '.') { input++; i = 8; while (*input && i < 11) output[i++] = *input++; }
}

static struct fat32_entry *find_entry(const char *name, uint32_t directory, char *sector, uint32_t *sector_number) {
    char wanted[11]; struct fat32_entry *entries = (struct fat32_entry *)sector;
    make_name(name, wanted);
    while (directory >= 2 && directory < FAT32_EOC) {
        *sector_number = cluster_sector(directory); read_sector(*sector_number, sector);
        for (uint32_t i = 0; i < 512 / sizeof(struct fat32_entry); i++) {
            if (entries[i].name[0] == 0) return 0;
            if ((uint8_t)entries[i].name[0] != 0xE5 && entries[i].name[0] != '.' && entries[i].attributes != 0x0F && memory_compare(entries[i].name, wanted, 11) == 0) return &entries[i];
        }
        directory = fat_entry(directory);
    }
    return 0;
}

void init_fs(void) {
    read_sector(0, (char *)&bpb);
    if (bpb.bytes_per_sector != 512 || bpb.root_cluster < 2) { printf("Blad: brak systemu FAT32\n"); return; }
    fat_start = bpb.reserved_sectors; first_data_sector = fat_start + bpb.fat_count * bpb.fat_size; current_dir = bpb.root_cluster;
}

static struct fat32_entry *new_entry(const char *name, uint8_t attributes, char *sector) {
    struct fat32_entry *entry;
    read_sector(cluster_sector(current_dir), sector);
    for (uint32_t i = 0; i < 512 / sizeof(struct fat32_entry); i++) {
        entry = ((struct fat32_entry *)sector) + i;
        if (entry->name[0] == 0 || (uint8_t)entry->name[0] == 0xE5) { make_name(name, entry->name); entry->attributes = attributes; entry->cluster_high = 0; entry->cluster_low = alloc_cluster(); entry->size = 0; return entry; }
    }
    return 0;
}

int create_file(const char *name, uint32_t size) {
    char sector[512]; uint32_t sector_number; struct fat32_entry *entry = find_entry(name, current_dir, sector, &sector_number);
    if (entry) return -1;
    entry = new_entry(name, 0x20, sector); if (!entry || !entry->cluster_low) return -1; entry->size = size; write_sector(cluster_sector(current_dir), sector); return 0;
}

int read_file(const char *name, char *buffer, uint32_t size) {
    char sector[512]; uint32_t sector_number, done = 0, cluster; struct fat32_entry *entry = find_entry(name, current_dir, sector, &sector_number);
    if (!entry || (entry->attributes & 0x10)) return -1;
    cluster = ((uint32_t)entry->cluster_high << 16) | entry->cluster_low;
    while (done < size && done < entry->size && cluster < FAT32_EOC) { read_sector(cluster_sector(cluster), sector); for (uint32_t i = 0; i < 512 && done < size && done < entry->size; i++) buffer[done++] = sector[i]; cluster = fat_entry(cluster); }
    return done;
}

int write_file(const char *name, const char *data, uint32_t size) {
    char directory_sector[512]; char data_sector[512]; uint32_t sector_number, cluster, done = 0; struct fat32_entry *entry = find_entry(name, current_dir, directory_sector, &sector_number);
    if (!entry || (entry->attributes & 0x10)) return -1;
    cluster = ((uint32_t)entry->cluster_high << 16) | entry->cluster_low;
    while (done < size && cluster >= 2 && cluster < FAT32_EOC) { for (uint32_t i = 0; i < 512; i++) data_sector[i] = done + i < size ? data[done + i] : 0; write_sector(cluster_sector(cluster), data_sector); done += 512; if (done < size && fat_entry(cluster) >= FAT32_EOC) set_fat_entry(cluster, alloc_cluster()); cluster = fat_entry(cluster); }
    entry->size = size; write_sector(sector_number, directory_sector); return done >= size ? 0 : -1;
}

int create_dir(const char *name) { char sector[512]; uint32_t sector_number; struct fat32_entry *entry = find_entry(name, current_dir, sector, &sector_number); if (entry) return -1; entry = new_entry(name, 0x10, sector); if (!entry || !entry->cluster_low) return -1; write_sector(cluster_sector(current_dir), sector); return 0; }

int ls(const char *dir) {
    char sector[512]; uint32_t sector_number, directory = current_dir;
    if (dir) { struct fat32_entry *entry = find_entry(dir, current_dir, sector, &sector_number); if (!entry || !(entry->attributes & 0x10)) return -1; directory = ((uint32_t)entry->cluster_high << 16) | entry->cluster_low; }
    read_sector(cluster_sector(directory), sector); for (uint32_t i = 0; i < 512 / sizeof(struct fat32_entry); i++) { struct fat32_entry *entry = ((struct fat32_entry *)sector) + i; if (entry->name[0] == 0) break; if (entry->name[0] != '.' && entry->attributes != 0x0F && (uint8_t)entry->name[0] != 0xE5) printf("%s\n", entry->name); } return 0;
}

int cd(const char *dir) { char sector[512]; uint32_t sector_number; struct fat32_entry *entry; if (dir[0] == '.' && dir[1] == '.') { current_dir = bpb.root_cluster; return 0; } entry = find_entry(dir, current_dir, sector, &sector_number); if (!entry || !(entry->attributes & 0x10)) return -1; current_dir = ((uint32_t)entry->cluster_high << 16) | entry->cluster_low; return 0; }
