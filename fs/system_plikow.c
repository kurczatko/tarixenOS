/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/*
* NIE RUSZAC TEGO ten plik to absolutny smietnik ale jesli cos zmienisz przestanie dzialac
*/

#include <fs/system_plikow.h>
#include <include/printf/kprint.h>
#include <drivers/storage/floppy.h>
#include <drivers/storage/plyty.h>

#define FAT32_EOC      0x0FFFFFF8   /* >= tego = koniec lancucha */
#define FAT32_EOC_MARK 0x0FFFFFFF   /* to zapisujemy do FAT jako koniec */
#define FAT32_FREE     0
#define SECTOR_SIZE    512
#define ENTRIES_PER_SECTOR (SECTOR_SIZE / sizeof(struct fat32_entry))
#define INVALID_LBA    0xFFFFFFFFu

#define ATA_DATA    0x1F0
#define ATA_STATUS  0x1F7
#define ATA_ALT     0x3F6
#define ATA_MAX_CHUNK 128           /* sektorow na jedna komende ATA */

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

static struct fat32_bpb bpb;
static uint32_t current_dir = FAT32_ROOT_CLUSTER;
static uint32_t directory_stack[16];
static uint8_t directory_depth;
static uint32_t first_data_sector;
static uint32_t fat_start;
static uint32_t total_clusters;      /* 0 = system plikow niezamontowany/niepoprawny */
static uint32_t next_free_hint = 2;
static uint8_t fs_use_floppy = 0;
static uint8_t fs_use_cdrom = 0;
static uint8_t ata_dirty = 0;

#define FS_READY() (total_clusters != 0)

/* ------------------------------------------------------------------ */
/*  Niski poziom: porty i ATA PIO                                      */
/* ------------------------------------------------------------------ */

static void outb(uint16_t port, uint8_t value) { __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port)); }
static uint8_t inb(uint16_t port) { uint8_t value; __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port)); return value; }

/* jedna instrukcja rep insw/outsw zamiast 256 osobnych inw/outw */
static inline void insw(uint16_t port, void *addr, uint32_t count) {
    __asm__ volatile ("cld; rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}
static inline void outsw(uint16_t port, const void *addr, uint32_t count) {
    __asm__ volatile ("cld; rep outsw" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

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

static void ata_delay(void) { inb(ATA_ALT); inb(ATA_ALT); inb(ATA_ALT); inb(ATA_ALT); }

/* z timeoutem - poprzednio brak dysku = zawieszenie na zawsze */
static int ata_wait_bsy(void) {
    for (uint32_t i = 0; i < 1000000; i++) {
        uint8_t s = inb(ATA_STATUS);
        if (s == 0xFF) return -1;           /* brak urzadzenia */
        if (!(s & 0x80)) return 0;
    }
    return -1;
}

static int ata_wait_drq(void) {
    for (uint32_t i = 0; i < 1000000; i++) {
        uint8_t s = inb(ATA_STATUS);
        if (s & 0x80) continue;
        if (s & 0x21) return -1;            /* ERR lub DF */
        if (s & 0x08) return 0;             /* DRQ */
    }
    return -1;
}

static int ata_command(uint32_t lba, uint32_t count, uint8_t cmd) {
    if (ata_wait_bsy()) return -1;
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, (uint8_t)count);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(ATA_STATUS, cmd);
    ata_delay();
    return 0;
}

static int ata_read(uint32_t lba, uint32_t count, void *buf) {
    uint8_t *p = (uint8_t *)buf;
    while (count) {
        uint32_t n = count > ATA_MAX_CHUNK ? ATA_MAX_CHUNK : count;
        if (ata_command(lba, n, 0x20)) return -1;
        for (uint32_t i = 0; i < n; i++) {
            if (ata_wait_drq()) return -1;
            insw(ATA_DATA, p, 256);
            p += SECTOR_SIZE;
        }
        lba += n; count -= n;
    }
    return 0;
}

static int ata_write(uint32_t lba, uint32_t count, const void *buf) {
    const uint8_t *p = (const uint8_t *)buf;
    while (count) {
        uint32_t n = count > ATA_MAX_CHUNK ? ATA_MAX_CHUNK : count;
        if (ata_command(lba, n, 0x30)) return -1;
        for (uint32_t i = 0; i < n; i++) {
            if (ata_wait_drq()) return -1;
            outsw(ATA_DATA, p, 256);
            p += SECTOR_SIZE;
        }
        if (ata_wait_bsy()) return -1;
        lba += n; count -= n;
    }
    ata_dirty = 1;
    return 0;
}

/* flush cache dysku robimy RAZ na koniec operacji, nie po kazdym sektorze */
static void ata_flush(void) {
    if (ata_wait_bsy()) return;
    outb(0x1F6, 0xE0);
    outb(ATA_STATUS, 0xE7);
    ata_wait_bsy();
    ata_dirty = 0;
}

/* ------------------------------------------------------------------ */
/*  Warstwa urzadzenia: ATA / dyskietka / CD                           */
/* ------------------------------------------------------------------ */

static int dev_read(uint32_t lba, uint32_t count, void *buf) {
    uint8_t *p = (uint8_t *)buf;
    if (fs_use_cdrom) {
        static char temp[2048];
        for (uint32_t i = 0; i < count; i++) {
            if (!plyty_read_sector(lba + i, temp)) return -1;
            for (uint32_t j = 0; j < SECTOR_SIZE; j++) p[i * SECTOR_SIZE + j] = temp[j];
        }
        return 0;
    }
    if (fs_use_floppy) {
        for (uint32_t i = 0; i < count; i++) floppy_read_sector(lba + i, (char *)(p + i * SECTOR_SIZE));
        return 0;
    }
    return ata_read(lba, count, buf);
}

static int dev_write(uint32_t lba, uint32_t count, const void *buf) {
    const uint8_t *p = (const uint8_t *)buf;
    if (fs_use_cdrom) return -1;
    if (fs_use_floppy) {
        for (uint32_t i = 0; i < count; i++) floppy_write_sector(lba + i, (const char *)(p + i * SECTOR_SIZE));
        return 0;
    }
    return ata_write(lba, count, buf);
}

/* ------------------------------------------------------------------ */
/*  Cache sektora FAT (to byla glowna przyczyna wolnego zapisu)        */
/* ------------------------------------------------------------------ */

static char fat_cache[SECTOR_SIZE];
static uint32_t fat_cache_lba = INVALID_LBA;
static uint8_t fat_cache_dirty = 0;

static void fat_flush_cache(void) {
    if (!fat_cache_dirty || fat_cache_lba == INVALID_LBA) return;
    for (uint8_t copy = 0; copy < bpb.fat_count; copy++)
        dev_write(fat_cache_lba + copy * bpb.fat_size, 1, fat_cache);
    fat_cache_dirty = 0;
}

static void fat_cache_invalidate(void) {
    fat_flush_cache();
    fat_cache_lba = INVALID_LBA;
}

void fs_sync(void) {
    fat_flush_cache();
    if (ata_dirty && !fs_use_floppy && !fs_use_cdrom) ata_flush();
}

static void fat_load(uint32_t lba) {
    if (fat_cache_lba == lba) return;
    fat_flush_cache();
    dev_read(lba, 1, fat_cache);
    fat_cache_lba = lba;
}

static int is_valid_cluster(uint32_t c) {
    return c >= 2 && c < total_clusters + 2;
}

static uint32_t fat_entry(uint32_t cluster) {
    if (!is_valid_cluster(cluster)) return FAT32_EOC_MARK;
    fat_load(fat_start + cluster / 128);
    const uint8_t *p = (const uint8_t *)fat_cache + (cluster % 128) * 4;
    uint32_t value = p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    return value & 0x0FFFFFFF;
}

static void set_fat_entry(uint32_t cluster, uint32_t value) {
    if (!is_valid_cluster(cluster)) return;
    fat_load(fat_start + cluster / 128);
    uint8_t *p = (uint8_t *)fat_cache + (cluster % 128) * 4;
    value = (value & 0x0FFFFFFF) | ((uint32_t)p[3] & 0xF0) << 24;   /* zachowaj gorne 4 bity */
    p[0] = (uint8_t)value; p[1] = (uint8_t)(value >> 8);
    p[2] = (uint8_t)(value >> 16); p[3] = (uint8_t)(value >> 24);
    fat_cache_dirty = 1;
}

static uint32_t cluster_sector(uint32_t cluster) { return first_data_sector + (cluster - 2) * bpb.sectors_per_cluster; }

/* ------------------------------------------------------------------ */
/*  Alokacja klastrow                                                  */
/* ------------------------------------------------------------------ */

static void zero_cluster(uint32_t cluster) {
    static const char zero[SECTOR_SIZE] = {0};
    uint32_t lba = cluster_sector(cluster);
    for (uint32_t i = 0; i < bpb.sectors_per_cluster; i++) dev_write(lba + i, 1, zero);
}

/* szuka wolnego klastra od "hinta" (nie od 2) i NIE zeruje danych */
static uint32_t alloc_cluster_raw(void) {
    uint32_t last = total_clusters + 2;
    uint32_t c = next_free_hint;
    if (c < 2 || c >= last) c = 2;
    for (uint32_t n = 0; n < total_clusters; n++) {
        if (fat_entry(c) == FAT32_FREE) {
            set_fat_entry(c, FAT32_EOC_MARK);
            next_free_hint = c + 1;
            return c;
        }
        if (++c >= last) c = 2;
    }
    return 0;
}

/* wersja z zerowaniem - dla katalogow i dla fs_alloc_cluster() */
static uint32_t alloc_cluster(void) {
    uint32_t c = alloc_cluster_raw();
    if (c) zero_cluster(c);
    return c;
}

static void free_chain(uint32_t cluster) {
    while (is_valid_cluster(cluster)) {
        uint32_t next = fat_entry(cluster);
        set_fat_entry(cluster, FAT32_FREE);
        if (cluster < next_free_hint) next_free_hint = cluster;
        cluster = next;
    }
}

/* dopasowuje lancuch do dokladnie `needed` klastrow (przedluza lub skraca) */
static int resize_chain(uint32_t *first_io, uint32_t needed) {
    uint32_t first = *first_io, cluster, next;
    int created = 0;

    if (needed == 0) { free_chain(first); *first_io = 0; return 0; }
    if (!is_valid_cluster(first)) {
        first = alloc_cluster_raw();
        if (!first) return -1;
        created = 1;
    }
    cluster = first;
    for (uint32_t n = 1; n < needed; n++) {
        next = fat_entry(cluster);
        if (!is_valid_cluster(next)) {
            next = alloc_cluster_raw();
            if (!next) { if (created) free_chain(first); return -1; }
            set_fat_entry(cluster, next);
        }
        cluster = next;
    }
    next = fat_entry(cluster);
    if (is_valid_cluster(next)) free_chain(next);    /* skracanie pliku - stare klastry nie wyciekaja */
    set_fat_entry(cluster, FAT32_EOC_MARK);
    *first_io = first;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Wpisy katalogu                                                     */
/* ------------------------------------------------------------------ */

static uint32_t entry_cluster(const struct fat32_entry *e) {
    return ((uint32_t)e->cluster_high << 16) | e->cluster_low;
}

static void set_entry_cluster(struct fat32_entry *e, uint32_t c) {
    e->cluster_high = (uint16_t)(c >> 16);
    e->cluster_low = (uint16_t)(c & 0xFFFF);
}

static int is_listable(const struct fat32_entry *e) {
    return (uint8_t)e->name[0] != 0xE5 && e->name[0] != '.' &&
           (e->attributes & 0x3F) != 0x0F && !(e->attributes & 0x08);
}

static void make_name(const char *input, char output[11]) {
    uint32_t i;
    for (i = 0; i < 11; i++) output[i] = ' ';
    i = 0;
    while (*input && *input != '.' && i < 8) {
        char c = *input++;
        output[i++] = (c >= 'a' && c <= 'z') ? c - 32 : c;   /* FAT: wielkie litery */
    }
    while (*input && *input != '.') input++;                 /* za dluga nazwa - pomin reszte */
    if (*input == '.') {
        input++; i = 8;
        while (*input && i < 11) {
            char c = *input++;
            output[i++] = (c >= 'a' && c <= 'z') ? c - 32 : c;
        }
    }
}

static void format_short_name(const char short_name[11], char name[13]) {
    uint32_t base_length = 8;
    uint32_t extension_length = 3;
    uint32_t position = 0;

    while (base_length > 0 && short_name[base_length - 1] == ' ') base_length--;
    while (extension_length > 0 && short_name[8 + extension_length - 1] == ' ') extension_length--;

    for (uint32_t i = 0; i < base_length; i++) name[position++] = short_name[i];
    if (extension_length > 0) {
        name[position++] = '.';
        for (uint32_t i = 0; i < extension_length; i++) name[position++] = short_name[8 + i];
    }
    name[position] = '\0';
}

/* iterator po WSZYSTKICH sektorach WSZYSTKICH klastrow katalogu */
struct dir_iter {
    uint32_t cluster, sec, lba, idx, hops;
    uint8_t loaded;
    char buf[SECTOR_SIZE];
};

static void dir_begin(struct dir_iter *it, uint32_t cluster) {
    it->cluster = cluster; it->sec = 0; it->lba = 0; it->hops = 0;
    it->idx = ENTRIES_PER_SECTOR; it->loaded = 0;
}

static struct fat32_entry *dir_next(struct dir_iter *it) {
    if (it->idx >= ENTRIES_PER_SECTOR) {
        if (it->loaded) {
            if (++it->sec >= bpb.sectors_per_cluster) {
                it->sec = 0;
                it->cluster = fat_entry(it->cluster);
                if (++it->hops > total_clusters) return 0;   /* ochrona przed petla w FAT */
            }
        }
        if (!is_valid_cluster(it->cluster)) return 0;
        it->lba = cluster_sector(it->cluster) + it->sec;
        if (dev_read(it->lba, 1, it->buf)) return 0;
        it->loaded = 1;
        it->idx = 0;
    }
    return ((struct fat32_entry *)it->buf) + it->idx++;
}

/* zwraca wpis w buforze `sector` oraz LBA sektora w *sector_number */
static struct fat32_entry *find_entry(const char *name, uint32_t directory, char *sector, uint32_t *sector_number) {
    char wanted[11];
    struct fat32_entry *entries = (struct fat32_entry *)sector;
    uint32_t hops = 0;
    make_name(name, wanted);
    while (is_valid_cluster(directory) && hops++ <= total_clusters) {
        for (uint32_t s = 0; s < bpb.sectors_per_cluster; s++) {
            uint32_t lba = cluster_sector(directory) + s;
            if (dev_read(lba, 1, sector)) return 0;
            for (uint32_t i = 0; i < ENTRIES_PER_SECTOR; i++) {
                if (entries[i].name[0] == 0) return 0;
                if (!is_listable(&entries[i])) continue;
                if (memory_compare(entries[i].name, wanted, 11) == 0) { *sector_number = lba; return &entries[i]; }
            }
        }
        directory = fat_entry(directory);
    }
    return 0;
}

/* wolny slot w current_dir; jesli brak - rozszerza katalog o nowy klaster */
static struct fat32_entry *new_entry(const char *name, uint8_t attributes, char *sector, uint32_t *sector_number) {
    struct fat32_entry *entries = (struct fat32_entry *)sector;
    uint32_t cluster = current_dir, last = 0, hops = 0, lba;
    struct fat32_entry *e;

    while (is_valid_cluster(cluster) && hops++ <= total_clusters) {
        for (uint32_t s = 0; s < bpb.sectors_per_cluster; s++) {
            lba = cluster_sector(cluster) + s;
            if (dev_read(lba, 1, sector)) return 0;
            for (uint32_t i = 0; i < ENTRIES_PER_SECTOR; i++) {
                e = &entries[i];
                if (e->name[0] == 0 || (uint8_t)e->name[0] == 0xE5) {
                    uint8_t *p = (uint8_t *)e;
                    for (uint32_t k = 0; k < sizeof(*e); k++) p[k] = 0;
                    make_name(name, e->name);
                    e->attributes = attributes;
                    *sector_number = lba;
                    return e;
                }
            }
        }
        last = cluster;
        cluster = fat_entry(cluster);
    }
    if (!last) return 0;
    cluster = alloc_cluster();                      /* katalog pelny - dolacz nowy klaster */
    if (!cluster) return 0;
    set_fat_entry(last, cluster);
    lba = cluster_sector(cluster);
    if (dev_read(lba, 1, sector)) return 0;
    e = &entries[0];
    make_name(name, e->name);
    e->attributes = attributes;
    *sector_number = lba;
    return e;
}

static int dir_is_empty(uint32_t cluster) {
    struct dir_iter it; struct fat32_entry *e;
    dir_begin(&it, cluster);
    while ((e = dir_next(&it))) {
        if (e->name[0] == 0) break;
        if (is_listable(e)) return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Montowanie                                                         */
/* ------------------------------------------------------------------ */

/* parsuje do bufora tymczasowego i dopiero po walidacji nadpisuje stan */
static int mount_fs(void) {
    char boot[SECTOR_SIZE];
    struct fat32_bpb tmp;

    fat_cache_invalidate();
    total_clusters = 0;
    if (dev_read(0, 1, boot)) return 0;
    for (uint32_t i = 0; i < sizeof(tmp); i++) ((char *)&tmp)[i] = boot[i];

    if (tmp.bytes_per_sector != 512 || tmp.sectors_per_cluster == 0 ||
        (tmp.sectors_per_cluster & (tmp.sectors_per_cluster - 1)) != 0 ||
        tmp.reserved_sectors == 0 || tmp.fat_count == 0 || tmp.fat_size == 0 ||
        tmp.root_cluster < 2 || tmp.sectors == 0) return 0;

    uint32_t data_start = tmp.reserved_sectors + (uint32_t)tmp.fat_count * tmp.fat_size;
    if (tmp.sectors <= data_start) return 0;

    uint32_t clusters = (tmp.sectors - data_start) / tmp.sectors_per_cluster;
    if (clusters + 2 > tmp.fat_size * 128) clusters = tmp.fat_size * 128 - 2;
    if (clusters == 0) return 0;

    bpb = tmp;
    fat_start = bpb.reserved_sectors;
    first_data_sector = data_start;
    total_clusters = clusters;
    next_free_hint = 2;
    current_dir = bpb.root_cluster;
    directory_depth = 0;
    return 1;
}

void init_fs(void) {
    if (!mount_fs()) kprint("Blad: brak systemu FAT32\n");
}

void fs_set_device_floppy(int use_floppy) {
    fs_sync();
    fs_use_floppy = (uint8_t)(use_floppy ? 1 : 0);
    mount_fs();
}

int fs_is_floppy(void) {
    return fs_use_floppy;
}

void fs_set_device_cdrom(int use_cdrom) {
    fs_sync();
    fs_use_cdrom = (uint8_t)(use_cdrom ? 1 : 0);
    fat_cache_lba = INVALID_LBA;
    directory_depth = 0;
    /* wyjscie z trybu CD: wroc do katalogu glownego FAT (wczesniej zostawalo 0) */
    current_dir = use_cdrom ? 0 : bpb.root_cluster;
}

int fs_is_cdrom(void) {
    return fs_use_cdrom;
}

/* ------------------------------------------------------------------ */
/*  Surowe sektory (API publiczne)                                     */
/* ------------------------------------------------------------------ */

void read_sector(uint32_t sector, char *buffer) {
    fat_flush_cache();
    dev_read(sector, 1, buffer);
}

void write_sector(uint32_t sector, const char *buffer) {
    if (fs_use_cdrom) return;
    fat_cache_invalidate();
    dev_write(sector, 1, buffer);
    if (!fs_use_floppy) ata_flush();
}

/* ------------------------------------------------------------------ */
/*  Operacje na plikach                                                */
/* ------------------------------------------------------------------ */

int create_file(const char *name, uint32_t size) {
    char sector[SECTOR_SIZE]; uint32_t sector_number, first = 0;
    struct fat32_entry *entry;
    if (fs_use_cdrom || !FS_READY()) return -1;
    if (find_entry(name, current_dir, sector, &sector_number)) return -1;

    uint32_t cb = (uint32_t)bpb.sectors_per_cluster * SECTOR_SIZE;
    uint32_t needed = size / cb + (size % cb != 0);
    if (needed && resize_chain(&first, needed) != 0) { fs_sync(); return -1; }

    entry = new_entry(name, 0x20, sector, &sector_number);
    if (!entry) { free_chain(first); fs_sync(); return -1; }
    set_entry_cluster(entry, first);
    entry->size = size;
    dev_write(sector_number, 1, sector);

    /* plik o zadanym rozmiarze ma czytac zera, nie smieci z dysku */
    uint32_t c = first;
    for (uint32_t n = 0; n < needed && is_valid_cluster(c); n++) { zero_cluster(c); c = fat_entry(c); }
    fs_sync();
    return 0;
}

int read_file(const char *name, char *buffer, uint32_t size) {
    if (fs_use_cdrom) {
        unsigned long long odczytano = 0;
        if (!plyty_read_file(name, buffer, size, &odczytano)) return -1;
        return (int)odczytano;
    }
    char sector[SECTOR_SIZE]; uint32_t sector_number, done = 0, cluster, file_size;
    struct fat32_entry *entry;
    if (!FS_READY()) return -1;
    entry = find_entry(name, current_dir, sector, &sector_number);
    if (!entry || (entry->attributes & 0x10)) return -1;
    file_size = entry->size;
    cluster = entry_cluster(entry);
    if (size > file_size) size = file_size;

    uint32_t cb = (uint32_t)bpb.sectors_per_cluster * SECTOR_SIZE;
    while (done < size && is_valid_cluster(cluster)) {
        /* sklej sasiadujace klastry w jeden duzy odczyt */
        uint32_t remaining = size - done, covered = cb, last = cluster, next;
        while (covered < remaining) {
            next = fat_entry(last);
            if (next != last + 1 || !is_valid_cluster(next)) break;
            last = next; covered += cb;
        }
        uint32_t bytes = covered < remaining ? covered : remaining;
        uint32_t full = bytes / SECTOR_SIZE, tail = bytes % SECTOR_SIZE;
        uint32_t lba = cluster_sector(cluster);
        if (full && dev_read(lba, full, buffer + done)) break;
        if (tail) {
            if (dev_read(lba + full, 1, sector)) break;
            for (uint32_t i = 0; i < tail; i++) buffer[done + full * SECTOR_SIZE + i] = sector[i];
        }
        done += bytes;
        cluster = fat_entry(last);
    }
    return done;
}

int write_file(const char *name, const char *data, uint32_t size) {
    char directory_sector[SECTOR_SIZE]; char tail_sector[SECTOR_SIZE];
    uint32_t sector_number, cluster, first, done = 0;
    struct fat32_entry *entry;
    if (fs_use_cdrom || !FS_READY()) return -1;
    entry = find_entry(name, current_dir, directory_sector, &sector_number);
    if (!entry || (entry->attributes & 0x10)) return -1;

    uint32_t cb = (uint32_t)bpb.sectors_per_cluster * SECTOR_SIZE;
    uint32_t needed = size / cb + (size % cb != 0);

    /* 1) lancuch klastrow: przedluz LUB skroc (poprzednio stare klastry wyciekaly) */
    first = entry_cluster(entry);
    if (resize_chain(&first, needed) != 0) { fs_sync(); return -1; }

    /* 2) dane: cale sektory prosto z bufora, sasiadujace klastry jedna komenda */
    cluster = first;
    while (done < size) {
        if (!is_valid_cluster(cluster)) { fs_sync(); return -1; }
        uint32_t remaining = size - done, covered = cb, last = cluster, next;
        while (covered < remaining) {
            next = fat_entry(last);
            if (next != last + 1 || !is_valid_cluster(next)) break;
            last = next; covered += cb;
        }
        uint32_t bytes = covered < remaining ? covered : remaining;
        uint32_t full = bytes / SECTOR_SIZE, tail = bytes % SECTOR_SIZE;
        uint32_t lba = cluster_sector(cluster);
        if (full && dev_write(lba, full, data + done)) { fs_sync(); return -1; }
        if (tail) {
            for (uint32_t i = 0; i < SECTOR_SIZE; i++)
                tail_sector[i] = i < tail ? data[done + full * SECTOR_SIZE + i] : 0;
            if (dev_write(lba + full, 1, tail_sector)) { fs_sync(); return -1; }
        }
        done += bytes;
        cluster = fat_entry(last);
    }

    /* 3) wpis katalogu - zapis do wlasciwego sektora (find_entry zwraca jego LBA) */
    set_entry_cluster(entry, first);
    entry->size = size;
    dev_write(sector_number, 1, directory_sector);
    fs_sync();
    return 0;
}

int create_dir(const char *name) {
    char sector[SECTOR_SIZE], buf[SECTOR_SIZE]; uint32_t sector_number, cluster;
    struct fat32_entry *entry;
    if (fs_use_cdrom || !FS_READY()) return -1;
    if (find_entry(name, current_dir, sector, &sector_number)) return -1;

    cluster = alloc_cluster();
    if (!cluster) { fs_sync(); return -1; }
    entry = new_entry(name, 0x10, sector, &sector_number);
    if (!entry) { free_chain(cluster); fs_sync(); return -1; }
    set_entry_cluster(entry, cluster);
    dev_write(sector_number, 1, sector);

    /* wpisy "." i ".." w nowym katalogu (zgodnosc z innymi systemami) */
    for (uint32_t i = 0; i < SECTOR_SIZE; i++) buf[i] = 0;
    struct fat32_entry *d = (struct fat32_entry *)buf;
    for (uint32_t i = 0; i < 11; i++) { d[0].name[i] = ' '; d[1].name[i] = ' '; }
    d[0].name[0] = '.';
    d[1].name[0] = '.'; d[1].name[1] = '.';
    d[0].attributes = d[1].attributes = 0x10;
    set_entry_cluster(&d[0], cluster);
    set_entry_cluster(&d[1], current_dir == bpb.root_cluster ? 0 : current_dir);
    dev_write(cluster_sector(cluster), 1, buf);
    fs_sync();
    return 0;
}

int delete_file(const char *name) {
    char sector[SECTOR_SIZE];
    uint32_t sector_number;
    if (fs_use_cdrom || !FS_READY()) return -1;
    struct fat32_entry *entry = find_entry(name, current_dir, sector, &sector_number);
    if (!entry) return -1;
    uint32_t cluster = entry_cluster(entry);
    if ((entry->attributes & 0x10) && !dir_is_empty(cluster)) return -1;
    free_chain(cluster);                       /* poprzednio zwalniane tylko dla katalogow - pliki wyciekaly */
    entry->name[0] = (char)0xE5;
    dev_write(sector_number, 1, sector);       /* wlasciwy sektor, nie zawsze pierwszy katalogu */
    fs_sync();
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Listowanie / nawigacja                                             */
/* ------------------------------------------------------------------ */

int fs_list_current(struct fs_entry_info *entries, int capacity) {
    if (fs_use_cdrom) {
        struct plyty_wpis pentries[24];
        unsigned int liczba = 0;
        if (!plyty_list_root(pentries, capacity > 24 ? 24 : capacity, &liczba)) return -1;
        int count = (int)liczba;
        for (int i = 0; i < count; i++) {
            uint32_t j = 0;
            while (pentries[i].nazwa[j] && j < 12) {
                entries[i].name[j] = pentries[i].nazwa[j];
                j++;
            }
            entries[i].name[j] = 0;
            entries[i].is_directory = pentries[i].katalog;
        }
        return count;
    }
    struct dir_iter it; struct fat32_entry *e; int count = 0;
    if (!FS_READY()) return 0;
    dir_begin(&it, current_dir);
    while ((e = dir_next(&it))) {
        if (e->name[0] == 0) break;
        if (!is_listable(e)) continue;
        if (count >= capacity) break;
        format_short_name(e->name, entries[count].name);
        entries[count].is_directory = (e->attributes & 0x10) != 0;
        count++;
    }
    return count;
}

int ls(const char *dir) {
    if (fs_use_cdrom) {
        struct fs_entry_info list[24];
        int n = fs_list_current(list, 24);
        for (int i = 0; i < n; i++) kprint("%s\n", list[i].name);
        return 0;
    }
    char sector[SECTOR_SIZE]; uint32_t sector_number, directory = current_dir;
    struct dir_iter it; struct fat32_entry *e; char name[13];
    if (!FS_READY()) return -1;
    if (dir && *dir) {
        e = find_entry(dir, current_dir, sector, &sector_number);
        if (!e || !(e->attributes & 0x10)) return -1;
        directory = entry_cluster(e);
        if (directory < 2) directory = bpb.root_cluster;
    }
    dir_begin(&it, directory);
    while ((e = dir_next(&it))) {
        if (e->name[0] == 0) break;
        if (!is_listable(e)) continue;
        format_short_name(e->name, name);       /* name[11] nie ma '\0' - wczesniej %s czytalo smieci */
        kprint("%s\n", name);
    }
    return 0;
}

int cd(const char *dir) {
    if (fs_use_cdrom) {
        return 0;
    }
    char sector[SECTOR_SIZE];
    uint32_t sector_number;
    struct fat32_entry *entry;
    if (!FS_READY()) return -1;

    if (dir[0] == '/') {
        current_dir = bpb.root_cluster;
        directory_depth = 0;
        return 0;
    }
    if (dir[0] == '.' && dir[1] == 0) return 0;
    if (dir[0] == '.' && dir[1] == '.' && dir[2] == 0) {
        if (directory_depth == 0) current_dir = bpb.root_cluster;
        else current_dir = directory_stack[--directory_depth];
        return 0;
    }

    entry = find_entry(dir, current_dir, sector, &sector_number);
    if (!entry || !(entry->attributes & 0x10) || directory_depth >= 16) return -1;
    directory_stack[directory_depth++] = current_dir;
    current_dir = entry_cluster(entry);
    if (current_dir < 2) current_dir = bpb.root_cluster;
    return 0;
}

void fs_reset_directory(void) {
    current_dir = bpb.root_cluster;
    directory_depth = 0;
}

int fs_parent_directory(void) {
    return cd("..");
}

uint32_t fs_get_current_dir(void) {
    return current_dir;
}

void fs_set_current_dir(uint32_t dir) {
    current_dir = dir;
    directory_depth = 0;
}

uint32_t fs_fat_next(uint32_t cluster) {
    return fat_entry(cluster);
}

void fs_read_sector(uint32_t sector, char *buffer) {
    read_sector(sector, buffer);
}

void fs_write_sector(uint32_t sector, const char *buffer) {
    write_sector(sector, buffer);
}

uint32_t fs_alloc_cluster(void) {
    uint32_t c = alloc_cluster();
    fs_sync();
    return c;
}

uint32_t fs_cluster_sector(uint32_t cluster) {
    return cluster_sector(cluster);
}

struct fat32_entry *fs_find_entry(const char *name, char *sector, uint32_t *sector_number) {
    return find_entry(name, current_dir, sector, sector_number);
}

/* szuka w katalogu `parent` podkatalogu o klastrze `child` i zwraca jego nazwe */
static int name_of_child(uint32_t parent, uint32_t child, char *out) {
    struct dir_iter it; struct fat32_entry *e;
    dir_begin(&it, parent);
    while ((e = dir_next(&it))) {
        if (e->name[0] == 0) break;
        if (!is_listable(e) || !(e->attributes & 0x10)) continue;
        if (entry_cluster(e) == child) { format_short_name(e->name, out); return 1; }
    }
    return 0;
}

/* buf musi miec >= 128 bajtow. Poprzednia wersja psula stan katalogu i zwracala zla nazwe */
void fs_get_current_dir_name(char *buf) {
    uint32_t chain[17], pos = 0;
    buf[0] = '/'; buf[1] = 0;
    if (fs_use_cdrom || !FS_READY() || directory_depth == 0) return;

    for (uint32_t i = 0; i < directory_depth; i++) chain[i] = directory_stack[i];
    chain[directory_depth] = current_dir;

    for (uint32_t i = 1; i <= directory_depth; i++) {
        char name[13];
        if (!name_of_child(chain[i - 1], chain[i], name)) { name[0] = '?'; name[1] = 0; }
        uint32_t len = 0;
        while (name[len]) len++;
        if (pos + 1 + len >= 127) break;
        buf[pos++] = '/';
        for (uint32_t k = 0; k < len; k++) buf[pos++] = name[k];
    }
    buf[pos] = 0;
}