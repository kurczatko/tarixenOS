#include "plyty.h"
#include "../include/printf/printf.h"
#include "../system_plikow/system_plikow.h"

static inline void ply_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t ply_inb(uint16_t port) {
    uint8_t value; __asm__ volatile ("inb %1, %0" : "=a"(value) : "d"(port)); return value;
}
static inline void ply_outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint16_t ply_inw(uint16_t port) {
    uint16_t value; __asm__ volatile ("inw %1, %0" : "=a"(value) : "d"(port)); return value;
}
static inline void ply_outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint32_t ply_inl(uint16_t port) {
    uint32_t value; __asm__ volatile ("inl %1, %0" : "=a"(value) : "d"(port)); return value;
}

#define PLITY_PRIMARY_DATA     0x1F0
#define PLITY_PRIMARY_ERROR    0x1F1
#define PLITY_PRIMARY_FEATURES 0x1F1
#define PLITY_PRIMARY_SECCNT   0x1F2
#define PLITY_PRIMARY_SECNUM   0x1F3
#define PLITY_PRIMARY_CYLLOW   0x1F4
#define PLITY_PRIMARY_CYLHIGH  0x1F5
#define PLITY_PRIMARY_DH       0x1F6
#define PLITY_PRIMARY_CMD      0x1F7
#define PLITY_PRIMARY_STATUS   0x1F7

#define PLITY_SECONDARY_DATA     0x170
#define PLITY_SECONDARY_ERROR    0x171
#define PLITY_SECONDARY_FEATURES 0x171
#define PLITY_SECONDARY_SECCNT   0x172
#define PLITY_SECONDARY_SECNUM   0x173
#define PLITY_SECONDARY_CYLLOW   0x174
#define PLITY_SECONDARY_CYLHIGH  0x175
#define PLITY_SECONDARY_DH       0x176
#define PLITY_SECONDARY_CMD      0x177
#define PLITY_SECONDARY_STATUS   0x177

#define PLITY_CMD_ATAPI_IDENTIFY   0xA1
#define PLITY_CMD_ATAPI_PACKET     0xA0
#define PLITY_CMD_ATAPI_READ       0xA8
#define PLITY_CMD_ATAPI_READ_CD    0xBE
#define PLITY_CMD_ATAPI_READ_TOC   0x43
#define PLITY_CMD_ATAPI_MODE_SENSE 0x5A
#define PLITY_CMD_ATAPI_GET_CONFIGURATION 0x46
#define PLITY_CMD_ATAPI_START_STOP 0x1B
#define PLITY_CMD_ATAPI_PREVENT_ALLOW 0x1E
#define PLITY_CMD_ATAPI_LOAD_UNLOAD 0xA6

#define PLITY_ATAPI_DEV_CDROM  0x05
#define PLITY_ATAPI_DEV_DVDROM 0x0A

static struct plyty_disc g_disc;
static uint16_t g_base_port = 0x1F0;
static uint8_t g_drive_select = 0xA0;

static void ply_delay(void) {
    ply_inb(0x80);
    ply_inb(0x80);
    ply_inb(0x80);
    ply_inb(0x80);
}

static int ply_wait_ready(uint32_t tries) {
    for (uint32_t i = 0; i < tries; i++) {
        uint8_t st = ply_inb(g_base_port + 7);
        if ((st & 0x80) == 0 && (st & 0x40) != 0) return 1;
    }
    return 0;
}

static int ply_wait_not_busy(uint32_t tries) {
    for (uint32_t i = 0; i < tries; i++) {
        uint8_t st = ply_inb(g_base_port + 7);
        if ((st & 0x80) == 0) return 1;
    }
    return 0;
}

static int ply_atapi_packet(uint8_t *cdb, uint8_t cdb_len, uint8_t *data, uint32_t data_len, int read) {
    if (!ply_wait_not_busy(1000000)) return 0;
    ply_outb(g_base_port + 6, g_drive_select);
    ply_delay();
    ply_outb(g_base_port + 3, 0);
    ply_outb(g_base_port + 2, (uint8_t)((data_len + 1) & 0xFF));
    ply_outb(g_base_port + 1, 0);
    ply_outb(g_base_port, 0);
    ply_outb(g_base_port + 7, PLITY_CMD_ATAPI_PACKET);
    if (!ply_wait_ready(1000000)) return 0;
    for (uint8_t i = 0; i < cdb_len; i++) {
        ply_outb(g_base_port, cdb[i]);
    }
    if (data_len > 0 && data) {
        uint32_t words = data_len / 2;
        if (read) {
            for (uint32_t i = 0; i < words; i++) {
                uint16_t w = ply_inw(g_base_port);
                data[i * 2] = (uint8_t)w;
                data[i * 2 + 1] = (uint8_t)(w >> 8);
            }
        } else {
            for (uint32_t i = 0; i < words; i++) {
                uint16_t w = (uint16_t)((uint16_t)data[i * 2] | ((uint16_t)data[i * 2 + 1] << 8));
                ply_outw(g_base_port, w);
            }
        }
        uint32_t remaining = data_len % 2;
        if (remaining && read) {
            uint16_t w = ply_inw(g_base_port);
            data[data_len - 1] = (uint8_t)w;
        }
    }
    if (!ply_wait_not_busy(1000000)) return 0;
    uint8_t st = ply_inb(g_base_port + 7);
    if (st & 0x02) return 0;
    return 1;
}

static int ply_identify(void) {
    uint8_t buf[512];
    for (uint32_t i = 0; i < 512; i++) buf[i] = 0;
    if (!ply_wait_not_busy(1000000)) return 0;
    ply_outb(g_base_port + 6, g_drive_select);
    ply_delay();
    ply_outb(g_base_port + 3, 0);
    ply_outb(g_base_port + 2, 0);
    ply_outb(g_base_port + 1, 0);
    ply_outb(g_base_port, 0);
    ply_outb(g_base_port + 7, PLITY_CMD_ATAPI_IDENTIFY);
    if (!ply_wait_ready(1000000)) return 0;
    for (uint32_t i = 0; i < 256; i++) {
        uint16_t w = ply_inw(g_base_port);
        buf[i * 2] = (uint8_t)w;
        buf[i * 2 + 1] = (uint8_t)(w >> 8);
    }
    if (!ply_wait_not_busy(1000000)) return 0;
    uint8_t st = ply_inb(g_base_port + 7);
    if (st & 0x02) return 0;
    uint8_t dev_type = buf[0] & 0x1F;
    if (dev_type == PLITY_ATAPI_DEV_CDROM || dev_type == PLITY_ATAPI_DEV_DVDROM) return 1;
    return 1;
}

static int ply_get_config(uint8_t feature, uint8_t *buf, uint32_t len) {
    uint8_t cdb[12] = {0x46, 0, 0, 0, (uint8_t)(feature & 0xFF), (uint8_t)(feature >> 8), 0, 0, (uint8_t)(len & 0xFF), (uint8_t)(len >> 8), 0, 0};
    return ply_atapi_packet(cdb, 12, buf, len, 1);
}

static uint32_t ply_read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int ply_test_unit_ready(void) {
    uint8_t cdb[12] = {0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    return ply_atapi_packet(cdb, 12, 0, 0, 0);
}

static int ply_read_capacity(void) {
    uint8_t cdb[12] = {0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buf[8];
    for (uint32_t i = 0; i < 8; i++) buf[i] = 0;
    if (!ply_atapi_packet(cdb, 12, buf, 8, 1)) return 0;
    g_disc.sector_count = ply_read_be32(buf);
    g_disc.sector_size = ply_read_be32(buf + 4);
    if (g_disc.sector_size == 0) g_disc.sector_size = 2048;
    return 1;
}

static int ply_detect_type(void) {
    uint8_t buf[4096];
    for (uint32_t i = 0; i < 4096; i++) buf[i] = 0;
    if (!ply_get_config((uint8_t)0x00, buf, 4096)) {
        g_disc.type = PLITY_CDROM;
        return 1;
    }
    uint32_t count = ply_read_be32(buf + 2);
    uint32_t offset = 8;
    uint32_t detected = 0;
    for (uint32_t i = 0; i < count && offset + 4 <= 4096; i++) {
        uint16_t feat = (uint16_t)((uint16_t)buf[offset] << 8 | buf[offset + 1]);
        uint8_t flags = buf[offset + 3];
        uint8_t current = flags & 1;
        if (current) {
            switch (feat) {
                case 0x00: detected |= PLITY_CDROM; break;
                case 0x10: detected |= PLITY_DVDROM; break;
                case 0x1B: detected |= PLITY_DVDR; break;
                case 0x1D: detected |= PLITY_DVDRW; break;
                case 0x40: detected |= PLITY_BDROM; break;
                case 0x41: detected |= PLITY_BDR; break;
                case 0x42: detected |= PLITY_BDRE; break;
            }
        }
        offset += 4;
    }
    if (detected == 0) detected = PLITY_CDROM;
    g_disc.type = detected;
    return 1;
}

static int ply_detect_filesystem(void) {
    char buf[2048];
    plyty_read_sector(16, buf);
    uint8_t type = buf[0];
    if (type == 0x00 || type == 0x01) {
        g_disc.fs_type |= PLITY_FS_ISO9660;
        char *id = (char *)(buf + 1);
        if (id[0] == 'C' && id[1] == 'D' && id[2] == '0' && id[3] == '0' && id[4] == '1') {
            g_disc.fs_type |= PLITY_FS_ISO9660;
        }
        for (uint32_t i = 0; i < 32 && i < 127; i++) {
            char c = buf[77 + i];
            if (c == 0 || c == ' ') break;
            g_disc.volume_label[i] = c;
        }
        g_disc.volume_label[32] = 0;
        return 1;
    }
    plyty_read_sector(16, buf);
    if (buf[0] == 'B' && buf[1] == 'A' && buf[2] == 'A' && buf[3] == ' ') {
        g_disc.fs_type |= PLITY_FS_UDF;
    }
    plyty_read_sector(16, buf);
    if (buf[0] == '%' && buf[1] == '@') {
        g_disc.fs_type |= PLITY_FS_JOLIET;
    }
    if (g_disc.fs_type == 0) g_disc.fs_type = PLITY_FS_ISO9660;
    return 1;
}

int plyty_init(void) {
    g_disc.present = 0;
    g_disc.ready = 0;
    g_disc.type = 0;
    g_disc.fs_type = 0;
    g_disc.sector_count = 0;
    g_disc.sector_size = 2048;
    g_disc.volume_label[0] = 0;

    ply_outb(g_base_port + 6, g_drive_select);
    ply_delay();
    uint8_t status = ply_inb(g_base_port + 7);
    if (status == 0xFF) return 0;

    if (!ply_identify()) return 0;
    if (!ply_test_unit_ready()) return 0;
    if (!ply_detect_type()) return 0;
    if (!ply_read_capacity()) return 0;
    if (!ply_detect_filesystem()) return 0;

    g_disc.present = 1;
    g_disc.ready = 1;
    return 1;
}

int plyty_present(void) {
    return g_disc.present ? 1 : 0;
}

int plyty_ready(void) {
    return g_disc.ready ? 1 : 0;
}

uint32_t plyty_type(void) {
    return g_disc.type;
}

uint64_t plyty_sector_count(void) {
    return g_disc.sector_count;
}

uint32_t plyty_sector_size(void) {
    return g_disc.sector_size;
}

int plyty_read_sector(uint64_t sector, char *buffer) {
    if (!g_disc.ready) return 0;
    uint8_t cdb[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    cdb[1] = 0;
    cdb[2] = (uint8_t)((sector >> 24) & 0xFF);
    cdb[3] = (uint8_t)((sector >> 16) & 0xFF);
    cdb[4] = (uint8_t)((sector >> 8) & 0xFF);
    cdb[5] = (uint8_t)(sector & 0xFF);
    cdb[7] = 0;
    cdb[8] = 1;
    return ply_atapi_packet(cdb, 12, (uint8_t *)buffer, g_disc.sector_size, 1);
}

int plyty_read_sectors(uint64_t sector, uint32_t count, char *buffer) {
    if (!g_disc.ready) return 0;
    for (uint32_t i = 0; i < count; i++) {
        if (!plyty_read_sector(sector + i, buffer + i * g_disc.sector_size)) return 0;
    }
    return 1;
}

const char *plyty_volume_label(void) {
    return g_disc.volume_label;
}

uint32_t plyty_fs_type(void) {
    return g_disc.fs_type;
}

static int ply_iso9660_read_dir(uint64_t sector, struct plyty_file_entry *entries, int capacity, int *count) {
    char buf[2048];
    if (!plyty_read_sector(sector, buf)) return 0;
    uint8_t type = buf[0];
    if (type == 0x00 || type == 0x01) {
        uint32_t extent = *(uint32_t *)(buf + 2);
        uint32_t root_sector = extent;
        return ply_iso9660_read_dir(root_sector, entries, capacity, count);
    }
    uint32_t offset = 0;
    while (offset < 2048 && *count < capacity) {
        uint8_t dr_len = buf[offset];
        if (dr_len == 0) break;
        uint32_t extent = *(uint32_t *)(buf + offset + 2);
        uint32_t data_len = *(uint32_t *)(buf + offset + 10);
        uint8_t flags = buf[offset + 25];
        uint8_t name_len = buf[offset + 32];
        if (name_len > 0 && offset + 33 + name_len <= 2048) {
            char name[256];
            uint32_t nlen = (uint32_t)name_len;
            if (nlen > 255) nlen = 255;
            for (uint32_t i = 0; i < nlen; i++) name[i] = buf[offset + 33 + i];
            name[nlen] = 0;
            if (name[0] != 0 && name[0] != 1 && name[0] != '.') {
                struct plyty_file_entry *e = &entries[*count];
                for (uint32_t i = 0; i < 255 && name[i]; i++) e->name[i] = name[i];
                e->name[255] = 0;
                e->is_directory = (flags & 0x02) != 0;
                e->size = data_len;
                e->sector = extent;
                (*count)++;
            }
        }
        offset += dr_len;
    }
    return 1;
}

int plyty_list_root(struct plyty_file_entry *entries, int capacity) {
    if (!g_disc.ready || !(g_disc.fs_type & PLITY_FS_ISO9660)) return 0;
    int count = 0;
    ply_iso9660_read_dir(16, entries, capacity, &count);
    return count;
}

int plyty_find_file(const char *name, struct plyty_file_entry *entry) {
    if (!g_disc.ready) return 0;
    struct plyty_file_entry entries[64];
    int count = plyty_list_root(entries, 64);
    for (int i = 0; i < count; i++) {
        uint32_t j = 0;
        while (name[j] && entries[i].name[j] && name[j] == entries[i].name[j]) j++;
        if (name[j] == 0 && entries[i].name[j] == 0) {
            *entry = entries[i];
            return 1;
        }
    }
    return 0;
}

int plyty_read_file(const char *name, char *buffer, uint64_t size) {
    if (!g_disc.ready) return 0;
    struct plyty_file_entry entry;
    if (!plyty_find_file(name, &entry)) return 0;
    uint64_t total = entry.size < size ? entry.size : size;
    uint64_t sectors = (total + g_disc.sector_size - 1) / g_disc.sector_size;
    if (sectors == 0) return 0;
    return plyty_read_sectors(entry.sector, (uint32_t)sectors, buffer);
}

int plyty_disc_changed(void) {
    if (!g_disc.present) return 0;
    uint8_t cdb[12] = {0x1E, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t buf[8];
    for (uint32_t i = 0; i < 8; i++) buf[i] = 0;
    if (!ply_atapi_packet(cdb, 12, buf, 8, 1)) return 0;
    return (buf[0] & 0x01) ? 1 : 0;
}

void plyty_eject(void) {
    uint8_t cdb[12] = {0xA6, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ply_atapi_packet(cdb, 12, 0, 0, 0);
}

void plyty_close_tray(void) {
    uint8_t cdb[12] = {0xA6, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    ply_atapi_packet(cdb, 12, 0, 0, 0);
}

extern "C" uint64_t __udivdi3(uint64_t a, uint64_t b) {
    if (b == 0) return 0xFFFFFFFFFFFFFFFFULL;
    uint64_t q = 0;
    uint64_t r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((a >> i) & 1);
        if (r >= b) {
            r -= b;
            q |= (1ULL << i);
        }
    }
    return q;
}
