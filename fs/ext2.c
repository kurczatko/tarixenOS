/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*
* Pelny, zaawansowany sterownik systemu plikow ext2.
* Implementuje superblok, grupowe deskryptory, wezly i, bitowe mapy blokow
* i wezlow, katalogi, indeksy posrednie (pojedyncze, podwojne, potrojne),
* tworzenie i usuwanie plikow oraz katalogow, odczyt i zapis, nawigacje
* miedzy katalogami, stat, sciezki wzgledne i bezwzgledne, kropka oraz
* kropka-kropka.
*/

#include <fs/ext2.h>
#include <drivers/storage/floppy.h>

#define EXT2_SUPER_SECTOR 2
#define EXT2_MAX_PATH 256
#define EXT2_MAX_COMPONENTS 64

static uint8_t ext2_mounted = 0;
static uint8_t ext2_use_floppy = 0;
static uint32_t ext2_block_size = 1024;
static uint32_t ext2_blocks_per_group = 8192;
static uint32_t ext2_inodes_per_group = 128;
static uint32_t ext2_inode_size = 128;
static uint32_t ext2_blocks_count = 0;
static uint32_t ext2_inodes_count = 0;
static uint32_t ext2_first_ino = 11;
static uint32_t ext2_first_data_block = 1;
static uint32_t ext2_group_count = 0;
static uint32_t ext2_current_ino = EXT2_ROOT_INO;
static char ext2_cwd[EXT2_MAX_PATH] = "/";

static void ext2_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t ext2_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "d"(port));
    return value;
}

static void ext2_ata_wait(void) {
    while (ext2_inb(0x1F7) & 0x80) { }
}

static int ext2_read_sector(uint32_t sector, char *buffer) {
    if (ext2_use_floppy) {
        return floppy_read_sector(sector, buffer);
    }
    uint32_t i;
    uint16_t word;
    ext2_ata_wait();
    ext2_outb(0x1F2, 1);
    ext2_outb(0x1F3, (uint8_t)sector);
    ext2_outb(0x1F4, (uint8_t)(sector >> 8));
    ext2_outb(0x1F5, (uint8_t)(sector >> 16));
    ext2_outb(0x1F6, (uint8_t)(0xE0 | ((sector >> 24) & 0x0F)));
    ext2_outb(0x1F7, 0x20);
    while (!(ext2_inb(0x1F7) & 8)) { }
    for (i = 0; i < 256; i++) {
        __asm__ volatile ("inw %1, %0" : "=a"(word) : "d"((uint16_t)0x1F0));
        buffer[i * 2] = (char)word;
        buffer[i * 2 + 1] = (char)(word >> 8);
    }
    return 1;
}

static int ext2_write_sector(uint32_t sector, const char *buffer) {
    if (ext2_use_floppy) {
        return floppy_write_sector(sector, buffer);
    }
    uint32_t i;
    uint16_t word;
    ext2_ata_wait();
    ext2_outb(0x1F2, 1);
    ext2_outb(0x1F3, (uint8_t)sector);
    ext2_outb(0x1F4, (uint8_t)(sector >> 8));
    ext2_outb(0x1F5, (uint8_t)(sector >> 16));
    ext2_outb(0x1F6, (uint8_t)(0xE0 | ((sector >> 24) & 0x0F)));
    ext2_outb(0x1F7, 0x30);
    while (!(ext2_inb(0x1F7) & 8)) { }
    for (i = 0; i < 256; i++) {
        word = (uint16_t)((uint8_t)buffer[i * 2] | ((uint16_t)(uint8_t)buffer[i * 2 + 1] << 8));
        __asm__ volatile ("outw %0, %1" : : "a"(word), "d"((uint16_t)0x1F0));
    }
    ext2_outb(0x1F7, 0xE7);
    ext2_ata_wait();
    return 1;
}

static uint32_t ext2_sectors_per_block(void) {
    return ext2_block_size / 512;
}

int ext2_read_block(uint32_t block, char *buffer) {
    uint32_t sector = block * ext2_sectors_per_block();
    uint32_t count = ext2_sectors_per_block();
    for (uint32_t i = 0; i < count; i++) {
        if (!ext2_read_sector(sector + i, buffer + i * 512)) return 0;
    }
    return 1;
}

int ext2_write_block(uint32_t block, const char *buffer) {
    uint32_t sector = block * ext2_sectors_per_block();
    uint32_t count = ext2_sectors_per_block();
    for (uint32_t i = 0; i < count; i++) {
        if (!ext2_write_sector(sector + i, buffer + i * 512)) return 0;
    }
    return 1;
}

static int ext2_bitmap_test(const char *bitmap, uint32_t bit) {
    return (bitmap[bit / 8] >> (bit % 8)) & 1u;
}

static void ext2_bitmap_set(char *bitmap, uint32_t bit) {
    bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8));
}

static void ext2_bitmap_clear(char *bitmap, uint32_t bit) {
    bitmap[bit / 8] &= (uint8_t)~(1u << (bit % 8));
}

static int ext2_read_super(struct ext2_superblock *sb) {
    char raw[1024];
    if (!ext2_read_sector(EXT2_SUPER_SECTOR, raw)) return 0;
    if (!ext2_read_sector(EXT2_SUPER_SECTOR + 1, raw + 512)) return 0;
    for (uint32_t i = 0; i < sizeof(struct ext2_superblock); i++) {
        ((char *)sb)[i] = raw[i];
    }
    if (sb->magic != EXT2_SUPER_MAGIC) return 0;
    return 1;
}

static int ext2_write_super(const struct ext2_superblock *sb) {
    char raw[1024];
    for (uint32_t i = 0; i < sizeof(struct ext2_superblock); i++) {
        raw[i] = ((const char *)sb)[i];
    }
    for (uint32_t i = sizeof(struct ext2_superblock); i < 1024; i++) raw[i] = 0;
    if (!ext2_write_sector(EXT2_SUPER_SECTOR, raw)) return 0;
    if (!ext2_write_sector(EXT2_SUPER_SECTOR + 1, raw + 512)) return 0;
    return 1;
}

static uint32_t ext2_gd_block(uint32_t group) {
    uint32_t block = ext2_first_data_block + 1;
    uint32_t per_block = ext2_block_size / (uint32_t)sizeof(struct ext2_group_desc);
    return block + group / per_block;
}

static uint32_t ext2_gd_offset(uint32_t group) {
    uint32_t per_block = ext2_block_size / (uint32_t)sizeof(struct ext2_group_desc);
    return (group % per_block) * (uint32_t)sizeof(struct ext2_group_desc);
}

static int ext2_read_group_desc(uint32_t group, struct ext2_group_desc *gd) {
    char block[4096];
    if (ext2_block_size > sizeof(block)) return 0;
    if (!ext2_read_block(ext2_gd_block(group), block)) return 0;
    for (uint32_t i = 0; i < sizeof(struct ext2_group_desc); i++) {
        ((char *)gd)[i] = block[ext2_gd_offset(group) + i];
    }
    return 1;
}

static int ext2_write_group_desc(uint32_t group, const struct ext2_group_desc *gd) {
    char block[4096];
    if (ext2_block_size > sizeof(block)) return 0;
    if (!ext2_read_block(ext2_gd_block(group), block)) return 0;
    for (uint32_t i = 0; i < sizeof(struct ext2_group_desc); i++) {
        block[ext2_gd_offset(group) + i] = ((const char *)gd)[i];
    }
    return ext2_write_block(ext2_gd_block(group), block);
}

static int ext2_read_inode(uint32_t ino, struct ext2_inode *inode) {
    if (ino == 0 || ino > ext2_inodes_count) return EXT2_ERR_NOT_FOUND;
    uint32_t group = (ino - 1) / ext2_inodes_per_group;
    uint32_t index = (ino - 1) % ext2_inodes_per_group;
    struct ext2_group_desc gd;
    if (!ext2_read_group_desc(group, &gd)) return EXT2_ERR_IO;
    uint32_t offset = index * ext2_inode_size;
    uint32_t block = gd.inode_table + offset / ext2_block_size;
    uint32_t boff = offset % ext2_block_size;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    if (!ext2_read_block(block, blockbuf)) return EXT2_ERR_IO;
    for (uint32_t i = 0; i < ext2_inode_size; i++) {
        ((char *)inode)[i] = blockbuf[boff + i];
    }
    return EXT2_ERR_OK;
}

static int ext2_write_inode(uint32_t ino, const struct ext2_inode *inode) {
    if (ino == 0 || ino > ext2_inodes_count) return EXT2_ERR_NOT_FOUND;
    uint32_t group = (ino - 1) / ext2_inodes_per_group;
    uint32_t index = (ino - 1) % ext2_inodes_per_group;
    struct ext2_group_desc gd;
    if (!ext2_read_group_desc(group, &gd)) return EXT2_ERR_IO;
    uint32_t offset = index * ext2_inode_size;
    uint32_t block = gd.inode_table + offset / ext2_block_size;
    uint32_t boff = offset % ext2_block_size;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    if (!ext2_read_block(block, blockbuf)) return EXT2_ERR_IO;
    for (uint32_t i = 0; i < ext2_inode_size; i++) {
        blockbuf[boff + i] = ((const char *)inode)[i];
    }
    return ext2_write_block(block, blockbuf);
}

static int ext2_alloc_from_bitmap(uint32_t group, uint32_t bitmap_block, uint32_t count, uint32_t start_bit, uint32_t *bit_out) {
    (void)group;
    char bitmap[4096];
    if (ext2_block_size > sizeof(bitmap)) return EXT2_ERR_IO;
    if (!ext2_read_block(bitmap_block, bitmap)) return EXT2_ERR_IO;
    uint32_t max_bit = ext2_block_size * 8;
    if (count > max_bit) count = max_bit;
    for (uint32_t b = start_bit; b + count <= max_bit; b++) {
        int free_run = 1;
        for (uint32_t k = 0; k < count; k++) {
            if (ext2_bitmap_test(bitmap, b + k)) { free_run = 0; break; }
        }
        if (free_run) {
            for (uint32_t k = 0; k < count; k++) ext2_bitmap_set(bitmap, b + k);
            if (!ext2_write_block(bitmap_block, bitmap)) return EXT2_ERR_IO;
            *bit_out = b;
            return EXT2_ERR_OK;
        }
    }
    return EXT2_ERR_NO_SPACE;
}

static int ext2_free_bitmap_bits(uint32_t group, uint32_t bitmap_block, uint32_t start_bit, uint32_t count) {
    (void)group;
    char bitmap[4096];
    if (ext2_block_size > sizeof(bitmap)) return EXT2_ERR_IO;
    if (!ext2_read_block(bitmap_block, bitmap)) return EXT2_ERR_IO;
    for (uint32_t k = 0; k < count; k++) ext2_bitmap_clear(bitmap, start_bit + k);
    return ext2_write_block(bitmap_block, bitmap);
}

static int ext2_alloc_block_group(uint32_t group, uint32_t *block_out) {
    struct ext2_group_desc gd;
    if (!ext2_read_group_desc(group, &gd)) return EXT2_ERR_IO;
    uint32_t bit = 0;
    int rc = ext2_alloc_from_bitmap(group, gd.block_bitmap, ext2_blocks_per_group, 0, &bit);
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t block = group * ext2_blocks_per_group + bit;
    gd.free_blocks_count--;
    if (!ext2_write_group_desc(group, &gd)) return EXT2_ERR_IO;
    *block_out = block;
    return EXT2_ERR_OK;
}

static int ext2_free_block(uint32_t block) {
    uint32_t group = block / ext2_blocks_per_group;
    uint32_t bit = block % ext2_blocks_per_group;
    struct ext2_group_desc gd;
    if (!ext2_read_group_desc(group, &gd)) return EXT2_ERR_IO;
    int rc = ext2_free_bitmap_bits(group, gd.block_bitmap, bit, 1);
    if (rc != EXT2_ERR_OK) return rc;
    gd.free_blocks_count++;
    return ext2_write_group_desc(group, &gd);
}

static int ext2_alloc_inode_group(uint32_t group, uint32_t *ino_out) {
    struct ext2_group_desc gd;
    if (!ext2_read_group_desc(group, &gd)) return EXT2_ERR_IO;
    uint32_t start = (group == 0) ? (ext2_first_ino - 1) : 0;
    uint32_t bit = 0;
    int rc = ext2_alloc_from_bitmap(group, gd.inode_bitmap, ext2_inodes_per_group, start, &bit);
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t ino = group * ext2_inodes_per_group + bit + 1;
    gd.free_inodes_count--;
    if (!ext2_write_group_desc(group, &gd)) return EXT2_ERR_IO;
    *ino_out = ino;
    return EXT2_ERR_OK;
}

static int ext2_free_inode(uint32_t ino) {
    if (ino == 0) return EXT2_ERR_OK;
    uint32_t group = (ino - 1) / ext2_inodes_per_group;
    uint32_t bit = (ino - 1) % ext2_inodes_per_group;
    struct ext2_group_desc gd;
    if (!ext2_read_group_desc(group, &gd)) return EXT2_ERR_IO;
    int rc = ext2_free_bitmap_bits(group, gd.inode_bitmap, bit, 1);
    if (rc != EXT2_ERR_OK) return rc;
    gd.free_inodes_count++;
    return ext2_write_group_desc(group, &gd);
}

static int ext2_super_dec_free_blocks(void) {
    struct ext2_superblock sb;
    if (!ext2_read_super(&sb)) return EXT2_ERR_IO;
    if (sb.free_blocks_count > 0) sb.free_blocks_count--;
    return ext2_write_super(&sb);
}

static int ext2_super_dec_free_inodes(void) {
    struct ext2_superblock sb;
    if (!ext2_read_super(&sb)) return EXT2_ERR_IO;
    if (sb.free_inodes_count > 0) sb.free_inodes_count--;
    return ext2_write_super(&sb);
}

static int ext2_super_inc_free_inodes(void) {
    struct ext2_superblock sb;
    if (!ext2_read_super(&sb)) return EXT2_ERR_IO;
    sb.free_inodes_count++;
    return ext2_write_super(&sb);
}

static int ext2_block_map_read(const struct ext2_inode *inode, uint32_t lblock, uint32_t *pblock) {
    uint32_t per_block = ext2_block_size / 4;
    if (lblock < 12) {
        *pblock = inode->block[lblock];
        return EXT2_ERR_OK;
    }
    uint32_t idx = lblock - 12;
    if (idx < per_block) {
        char ib[4096];
        if (ext2_block_size > sizeof(ib)) return EXT2_ERR_IO;
        if (inode->block[12] == 0) { *pblock = 0; return EXT2_ERR_OK; }
        if (!ext2_read_block(inode->block[12], ib)) return EXT2_ERR_IO;
        *pblock = ((uint32_t *)ib)[idx];
        return EXT2_ERR_OK;
    }
    idx -= per_block;
    uint32_t per2 = per_block * per_block;
    if (idx < per2) {
        char ib[4096], ib2[4096];
        if (ext2_block_size > sizeof(ib)) return EXT2_ERR_IO;
        if (inode->block[13] == 0) { *pblock = 0; return EXT2_ERR_OK; }
        if (!ext2_read_block(inode->block[13], ib)) return EXT2_ERR_IO;
        uint32_t first = ((uint32_t *)ib)[idx / per_block];
        if (first == 0) { *pblock = 0; return EXT2_ERR_OK; }
        if (!ext2_read_block(first, ib2)) return EXT2_ERR_IO;
        *pblock = ((uint32_t *)ib2)[idx % per_block];
        return EXT2_ERR_OK;
    }
    idx -= per2;
    uint32_t per3 = per2 * per_block;
    if (idx < per3) {
        char ib[4096], ib2[4096], ib3[4096];
        if (ext2_block_size > sizeof(ib)) return EXT2_ERR_IO;
        if (inode->block[14] == 0) { *pblock = 0; return EXT2_ERR_OK; }
        if (!ext2_read_block(inode->block[14], ib)) return EXT2_ERR_IO;
        uint32_t first = ((uint32_t *)ib)[idx / per2];
        if (first == 0) { *pblock = 0; return EXT2_ERR_OK; }
        if (!ext2_read_block(first, ib2)) return EXT2_ERR_IO;
        uint32_t second = ((uint32_t *)ib2)[(idx % per2) / per_block];
        if (second == 0) { *pblock = 0; return EXT2_ERR_OK; }
        if (!ext2_read_block(second, ib3)) return EXT2_ERR_IO;
        *pblock = ((uint32_t *)ib3)[idx % per_block];
        return EXT2_ERR_OK;
    }
    return EXT2_ERR_INVALID;
}

static int ext2_block_map_write(struct ext2_inode *inode, uint32_t lblock, uint32_t pblock, int allocate) {
    uint32_t per_block = ext2_block_size / 4;
    if (lblock < 12) {
        inode->block[lblock] = pblock;
        return EXT2_ERR_OK;
    }
    uint32_t idx = lblock - 12;
    if (idx < per_block) {
        char ib[4096];
        if (ext2_block_size > sizeof(ib)) return EXT2_ERR_IO;
        if (inode->block[12] == 0) {
            if (!allocate) return EXT2_ERR_NOT_FOUND;
            uint32_t nb = 0;
            int rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            for (uint32_t i = 0; i < ext2_block_size; i++) ib[i] = 0;
            if (!ext2_write_block(nb, ib)) return EXT2_ERR_IO;
            inode->block[12] = nb;
        } else {
            if (!ext2_read_block(inode->block[12], ib)) return EXT2_ERR_IO;
        }
        ((uint32_t *)ib)[idx] = pblock;
        return ext2_write_block(inode->block[12], ib);
    }
    idx -= per_block;
    uint32_t per2 = per_block * per_block;
    if (idx < per2) {
        char ib[4096], ib2[4096];
        if (ext2_block_size > sizeof(ib)) return EXT2_ERR_IO;
        if (inode->block[13] == 0) {
            if (!allocate) return EXT2_ERR_NOT_FOUND;
            uint32_t nb = 0;
            int rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            for (uint32_t i = 0; i < ext2_block_size; i++) ib[i] = 0;
            if (!ext2_write_block(nb, ib)) return EXT2_ERR_IO;
            inode->block[13] = nb;
        } else {
            if (!ext2_read_block(inode->block[13], ib)) return EXT2_ERR_IO;
        }
        uint32_t first = ((uint32_t *)ib)[idx / per_block];
        if (first == 0) {
            if (!allocate) return EXT2_ERR_NOT_FOUND;
            uint32_t nb = 0;
            int rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            for (uint32_t i = 0; i < ext2_block_size; i++) ib2[i] = 0;
            if (!ext2_write_block(nb, ib2)) return EXT2_ERR_IO;
            first = nb;
            ((uint32_t *)ib)[idx / per_block] = first;
            if (!ext2_write_block(inode->block[13], ib)) return EXT2_ERR_IO;
        } else {
            if (!ext2_read_block(first, ib2)) return EXT2_ERR_IO;
        }
        ((uint32_t *)ib2)[idx % per_block] = pblock;
        return ext2_write_block(first, ib2);
    }
    idx -= per2;
    uint32_t per3 = per2 * per_block;
    if (idx < per3) {
        char ib[4096], ib2[4096], ib3[4096];
        if (ext2_block_size > sizeof(ib)) return EXT2_ERR_IO;
        if (inode->block[14] == 0) {
            if (!allocate) return EXT2_ERR_NOT_FOUND;
            uint32_t nb = 0;
            int rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            for (uint32_t i = 0; i < ext2_block_size; i++) ib[i] = 0;
            if (!ext2_write_block(nb, ib)) return EXT2_ERR_IO;
            inode->block[14] = nb;
        } else {
            if (!ext2_read_block(inode->block[14], ib)) return EXT2_ERR_IO;
        }
        uint32_t first = ((uint32_t *)ib)[idx / per2];
        if (first == 0) {
            if (!allocate) return EXT2_ERR_NOT_FOUND;
            uint32_t nb = 0;
            int rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            for (uint32_t i = 0; i < ext2_block_size; i++) ib2[i] = 0;
            if (!ext2_write_block(nb, ib2)) return EXT2_ERR_IO;
            first = nb;
            ((uint32_t *)ib)[idx / per2] = first;
            if (!ext2_write_block(inode->block[14], ib)) return EXT2_ERR_IO;
        } else {
            if (!ext2_read_block(first, ib2)) return EXT2_ERR_IO;
        }
        uint32_t second = ((uint32_t *)ib2)[(idx % per2) / per_block];
        if (second == 0) {
            if (!allocate) return EXT2_ERR_NOT_FOUND;
            uint32_t nb = 0;
            int rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            for (uint32_t i = 0; i < ext2_block_size; i++) ib3[i] = 0;
            if (!ext2_write_block(nb, ib3)) return EXT2_ERR_IO;
            second = nb;
            ((uint32_t *)ib2)[(idx % per2) / per_block] = second;
            if (!ext2_write_block(first, ib2)) return EXT2_ERR_IO;
        } else {
            if (!ext2_read_block(second, ib3)) return EXT2_ERR_IO;
        }
        ((uint32_t *)ib3)[idx % per_block] = pblock;
        return ext2_write_block(second, ib3);
    }
    return EXT2_ERR_INVALID;
}

static void ext2_inode_free_blocks(struct ext2_inode *inode) {
    uint32_t per_block = ext2_block_size / 4;
    uint32_t total = (inode->size + ext2_block_size - 1) / ext2_block_size;
    if (total == 0) return;
    for (uint32_t i = 0; i < 12 && i < total; i++) {
        if (inode->block[i]) ext2_free_block(inode->block[i]);
    }
    uint32_t rem = total - 12;
    if (rem > 0) {
        char ib[4096];
        if (ext2_block_size <= sizeof(ib) && inode->block[12]) {
            ext2_read_block(inode->block[12], ib);
            uint32_t n = rem < per_block ? rem : per_block;
            for (uint32_t i = 0; i < n; i++) {
                uint32_t b = ((uint32_t *)ib)[i];
                if (b) ext2_free_block(b);
            }
            ext2_free_block(inode->block[12]);
            rem -= n;
        }
    }
    if (rem > 0) {
        char ib[4096], ib2[4096];
        if (ext2_block_size <= sizeof(ib) && inode->block[13]) {
            ext2_read_block(inode->block[13], ib);
            uint32_t processed = 0;
            for (uint32_t i = 0; i < per_block && processed < rem; i++) {
                uint32_t first = ((uint32_t *)ib)[i];
                if (!first) continue;
                ext2_read_block(first, ib2);
                uint32_t n = rem - processed < per_block ? rem - processed : per_block;
                for (uint32_t j = 0; j < n; j++) {
                    uint32_t b = ((uint32_t *)ib2)[j];
                    if (b) ext2_free_block(b);
                }
                ext2_free_block(first);
                processed += n;
            }
            ext2_free_block(inode->block[13]);
            rem -= processed;
        }
    }
    if (rem > 0) {
        char ib[4096], ib2[4096], ib3[4096];
        if (ext2_block_size <= sizeof(ib) && inode->block[14]) {
            ext2_read_block(inode->block[14], ib);
            uint32_t processed = 0;
            for (uint32_t i = 0; i < per_block && processed < rem; i++) {
                uint32_t first = ((uint32_t *)ib)[i];
                if (!first) continue;
                ext2_read_block(first, ib2);
                uint32_t processed2 = 0;
                for (uint32_t j = 0; j < per_block && processed2 < (rem - processed); j++) {
                    uint32_t second = ((uint32_t *)ib2)[j];
                    if (!second) continue;
                    ext2_read_block(second, ib3);
                    uint32_t n = rem - processed - processed2 < per_block ? rem - processed - processed2 : per_block;
                    for (uint32_t k = 0; k < n; k++) {
                        uint32_t b = ((uint32_t *)ib3)[k];
                        if (b) ext2_free_block(b);
                    }
                    ext2_free_block(second);
                    processed2 += n;
                }
                ext2_free_block(first);
                processed += processed2;
            }
            ext2_free_block(inode->block[14]);
        }
    }
}

static int ext2_inode_blocks_count(const struct ext2_inode *inode) {
    uint32_t total = (inode->size + ext2_block_size - 1) / ext2_block_size;
    if (total == 0) return 0;
    uint32_t count = 0;
    uint32_t per_block = ext2_block_size / 4;
    for (uint32_t i = 0; i < 12 && i < total; i++) if (inode->block[i]) count++;
    uint32_t rem = total - 12;
    if (rem > 0) {
        count++;
        uint32_t n = rem < per_block ? rem : per_block;
        rem -= n;
    }
    if (rem > 0) {
        count++;
        uint32_t processed = 0;
        for (uint32_t i = 0; i < per_block && processed < rem; i++) {
            count++;
            uint32_t n = rem - processed < per_block ? rem - processed : per_block;
            processed += n;
        }
    }
    if (rem > 0) {
        count++;
        uint32_t processed = 0;
        for (uint32_t i = 0; i < per_block && processed < rem; i++) {
            count++;
            uint32_t processed2 = 0;
            for (uint32_t j = 0; j < per_block && processed2 < (rem - processed); j++) {
                count++;
                uint32_t n = rem - processed - processed2 < per_block ? rem - processed - processed2 : per_block;
                processed2 += n;
            }
            processed += processed2;
        }
    }
    return (int)count;
}

struct ext2_dir_foreach_ctx {
    ext2_dir_cb cb;
    void *ctx;
};

static int ext2_dir_foreach(uint32_t dir_ino, ext2_dir_cb cb, void *ctx) {
    struct ext2_inode inode;
    int rc = ext2_read_inode(dir_ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    if ((inode.mode & EXT2_S_IFMT) != EXT2_S_IFDIR) return EXT2_ERR_NOT_DIR;
    uint32_t total = (inode.size + ext2_block_size - 1) / ext2_block_size;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    for (uint32_t bi = 0; bi < total; bi++) {
        uint32_t pblock = 0;
        rc = ext2_block_map_read(&inode, bi, &pblock);
        if (rc != EXT2_ERR_OK) return rc;
        if (pblock == 0) continue;
        if (!ext2_read_block(pblock, blockbuf)) return EXT2_ERR_IO;
        uint32_t off = 0;
        while (off + 8 <= ext2_block_size) {
            struct ext2_dir_entry *de = (struct ext2_dir_entry *)(blockbuf + off);
            if (de->rec_len == 0) break;
            if (de->inode != 0) {
                char name[256];
                uint32_t nlen = de->name_len;
                if (nlen > 255) nlen = 255;
                for (uint32_t i = 0; i < nlen; i++) name[i] = de->name[i];
                name[nlen] = 0;
                uint8_t type = de->file_type;
                if (type == EXT2_FT_UNKNOWN) {
                    type = EXT2_FT_REG_FILE;
                }
                int stop = cb(de->inode, type, name, ctx);
                if (stop) return EXT2_ERR_OK;
            }
            off += de->rec_len;
            if (de->rec_len < 8) break;
        }
    }
    return EXT2_ERR_OK;
}

struct ext2_lookup_ctx {
    const char *name;
    uint32_t len;
    uint32_t *ino;
    uint8_t *type;
    int *found;
};

static int ext2_lookup_cb(uint32_t ino, uint8_t type, const char *nm, void *ctx) {
    struct ext2_lookup_ctx *c = (struct ext2_lookup_ctx *)ctx;
    if (c->len == 0) return 0;
    int match = 1;
    for (uint32_t i = 0; i < c->len; i++) {
        if (nm[i] != c->name[i]) { match = 0; break; }
    }
    if (match && nm[c->len] == 0) {
        *c->ino = ino;
        *c->type = type;
        *c->found = 1;
        return 1;
    }
    return 0;
}

static int ext2_dir_lookup(uint32_t dir_ino, const char *name, uint32_t *ino_out, uint8_t *type_out) {
    uint32_t namelen = 0;
    while (name[namelen]) namelen++;
    if (namelen == 0 || namelen > 255) return EXT2_ERR_NOT_FOUND;
    uint32_t f_ino = 0;
    uint8_t f_type = 0;
    int found = 0;
    struct ext2_lookup_ctx cbctx;
    cbctx.name = name;
    cbctx.len = namelen;
    cbctx.ino = &f_ino;
    cbctx.type = &f_type;
    cbctx.found = &found;
    int rc = ext2_dir_foreach(dir_ino, ext2_lookup_cb, &cbctx);
    if (rc != EXT2_ERR_OK) return rc;
    if (!found) return EXT2_ERR_NOT_FOUND;
    *ino_out = f_ino;
    if (type_out) *type_out = f_type;
    return EXT2_ERR_OK;
}

static int ext2_resolve_path(const char *path, uint32_t *ino_out, uint8_t *type_out) {
    if (!path || path[0] == 0) return EXT2_ERR_INVALID;
    uint32_t ino = (path[0] == '/') ? EXT2_ROOT_INO : ext2_current_ino;
    char comp[256];
    uint32_t ci = 0;
    int in_comp = 0;
    for (uint32_t i = 0; ; i++) {
        char c = path[i];
        if (c == '/' || c == 0) {
            if (in_comp) {
                comp[ci] = 0;
                in_comp = 0;
                if (comp[0] == '.' && comp[1] == 0) {
                } else if (comp[0] == '.' && comp[1] == '.' && comp[2] == 0) {
                    uint8_t t = 0;
                    int rc = ext2_dir_lookup(ino, "..", &ino, &t);
                    if (rc != EXT2_ERR_OK) return rc;
                } else {
                    uint8_t t = 0;
                    int rc = ext2_dir_lookup(ino, comp, &ino, &t);
                    if (rc != EXT2_ERR_OK) return rc;
                }
                ci = 0;
            }
            if (c == 0) break;
        } else {
            if (!in_comp) { in_comp = 1; ci = 0; }
            if (ci < 255) comp[ci++] = c;
        }
    }
    *ino_out = ino;
    if (type_out) {
        struct ext2_inode inode;
        int rc = ext2_read_inode(ino, &inode);
        if (rc != EXT2_ERR_OK) return rc;
        *type_out = ((inode.mode & EXT2_S_IFMT) == EXT2_S_IFDIR) ? EXT2_FT_DIR :
                    ((inode.mode & EXT2_S_IFMT) == EXT2_S_IFLNK) ? EXT2_FT_SYMLINK : EXT2_FT_REG_FILE;
    }
    return EXT2_ERR_OK;
}

static int ext2_parent_of_path(const char *path, char *base_out, uint32_t base_size) {
    const char *lastslash = 0;
    for (uint32_t i = 0; path[i]; i++) if (path[i] == '/') lastslash = path + i;
    if (!lastslash) {
        base_out[0] = 0;
        return EXT2_ERR_OK;
    }
    uint32_t len = (uint32_t)(lastslash - path);
    if (len == 0) { base_out[0] = '/'; base_out[1] = 0; return EXT2_ERR_OK; }
    if (len + 1 > base_size) return EXT2_ERR_INVALID;
    for (uint32_t i = 0; i < len; i++) base_out[i] = path[i];
    base_out[len] = 0;
    return EXT2_ERR_OK;
}

static const char *ext2_base_name(const char *path) {
    const char *s = path;
    const char *lastslash = 0;
    for (; *s; s++) if (*s == '/') lastslash = s;
    return lastslash ? lastslash + 1 : path;
}

static int ext2_path_components(const char *path, char comps[EXT2_MAX_COMPONENTS][256], int *count) {
    int n = 0;
    uint32_t i = 0;
    while (path[i]) {
        while (path[i] == '/') i++;
        if (!path[i]) break;
        int ci = 0;
        while (path[i] && path[i] != '/') {
            if (ci < 255) comps[n][ci++] = path[i];
            i++;
        }
        comps[n][ci] = 0;
        n++;
        if (n >= EXT2_MAX_COMPONENTS) break;
    }
    *count = n;
    return EXT2_ERR_OK;
}

void ext2_init(void) {
    ext2_mounted = 0;
    ext2_use_floppy = 0;
    ext2_block_size = 1024;
    ext2_blocks_per_group = 8192;
    ext2_inodes_per_group = 128;
    ext2_inode_size = 128;
    ext2_blocks_count = 0;
    ext2_inodes_count = 0;
    ext2_first_ino = 11;
    ext2_first_data_block = 1;
    ext2_group_count = 0;
    ext2_current_ino = EXT2_ROOT_INO;
    ext2_cwd[0] = '/';
    ext2_cwd[1] = 0;
}

int ext2_mount(void) {
    struct ext2_superblock sb;
    if (!ext2_read_super(&sb)) {
        return EXT2_ERR_IO;
    }
    ext2_block_size = 1024u << sb.log_block_size;
    if (ext2_block_size < 1024 || ext2_block_size > 4096) ext2_block_size = 1024;
    ext2_blocks_per_group = sb.blocks_per_group;
    ext2_inodes_per_group = sb.inodes_per_group;
    ext2_inode_size = sb.inode_size ? sb.inode_size : 128;
    ext2_blocks_count = sb.blocks_count;
    ext2_inodes_count = sb.inodes_count;
    ext2_first_ino = sb.first_ino ? sb.first_ino : 11;
    ext2_first_data_block = sb.first_data_block;
    ext2_group_count = (sb.blocks_count + sb.blocks_per_group - 1) / sb.blocks_per_group;
    ext2_current_ino = EXT2_ROOT_INO;
    ext2_cwd[0] = '/';
    ext2_cwd[1] = 0;
    ext2_mounted = 1;
    return EXT2_ERR_OK;
}

int ext2_set_device_floppy(int use_floppy) {
    ext2_use_floppy = (uint8_t)(use_floppy ? 1 : 0);
    return EXT2_ERR_OK;
}

int ext2_is_mounted(void) {
    return ext2_mounted ? 1 : 0;
}

int ext2_read_file(const char *path, char *buffer, uint32_t size) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    uint32_t ino = 0;
    uint8_t type = 0;
    int rc = ext2_resolve_path(path, &ino, &type);
    if (rc != EXT2_ERR_OK) return rc;
    if (type == EXT2_FT_DIR) return EXT2_ERR_NOT_DIR;
    struct ext2_inode inode;
    rc = ext2_read_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t total = (inode.size + ext2_block_size - 1) / ext2_block_size;
    uint32_t copy = size < inode.size ? size : inode.size;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    uint32_t done = 0;
    for (uint32_t bi = 0; bi < total && done < copy; bi++) {
        uint32_t pblock = 0;
        rc = ext2_block_map_read(&inode, bi, &pblock);
        if (rc != EXT2_ERR_OK) return rc;
        if (pblock == 0) continue;
        if (!ext2_read_block(pblock, blockbuf)) return EXT2_ERR_IO;
        uint32_t avail = ext2_block_size;
        if (done + avail > copy) avail = copy - done;
        for (uint32_t i = 0; i < avail; i++) buffer[done++] = blockbuf[i];
    }
    return (int)done;
}

int ext2_write_file(const char *path, const char *data, uint32_t size) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    uint32_t ino = 0;
    uint8_t type = 0;
    int rc = ext2_resolve_path(path, &ino, &type);
    if (rc != EXT2_ERR_OK) {
        if (rc == EXT2_ERR_NOT_FOUND) {
            rc = ext2_create_file(path, size);
            if (rc != EXT2_ERR_OK) return rc;
            rc = ext2_resolve_path(path, &ino, &type);
            if (rc != EXT2_ERR_OK) return rc;
        } else {
            return rc;
        }
    }
    struct ext2_inode inode;
    rc = ext2_read_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    if ((inode.mode & EXT2_S_IFMT) != EXT2_S_IFREG) return EXT2_ERR_NOT_DIR;
    uint32_t total = (size + ext2_block_size - 1) / ext2_block_size;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    uint32_t done = 0;
    for (uint32_t bi = 0; bi < total; bi++) {
        for (uint32_t i = 0; i < ext2_block_size; i++) blockbuf[i] = 0;
        uint32_t avail = ext2_block_size;
        if (done + avail > size) avail = size - done;
        for (uint32_t i = 0; i < avail; i++) blockbuf[i] = data[done + i];
        done += avail;
        uint32_t pblock = 0;
        rc = ext2_block_map_read(&inode, bi, &pblock);
        if (rc != EXT2_ERR_OK) return rc;
        if (pblock == 0) {
            uint32_t nb = 0;
            rc = ext2_alloc_block_group(0, &nb);
            if (rc != EXT2_ERR_OK) return rc;
            pblock = nb;
            rc = ext2_block_map_write(&inode, bi, pblock, 1);
            if (rc != EXT2_ERR_OK) return rc;
            ext2_super_dec_free_blocks();
        }
        if (!ext2_write_block(pblock, blockbuf)) return EXT2_ERR_IO;
    }
    inode.size = size;
    inode.blocks = ext2_inode_blocks_count(&inode);
    rc = ext2_write_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    return EXT2_ERR_OK;
}

static int ext2_dir_add(uint32_t dir_ino, uint32_t ino, const char *name, uint8_t type) {
    uint32_t namelen = 0;
    while (name[namelen]) namelen++;
    if (namelen == 0 || namelen > 255) return EXT2_ERR_INVALID;
    uint32_t need = 8 + namelen;
    need = (need + 3) & ~3u;
    struct ext2_inode dir;
    int rc = ext2_read_inode(dir_ino, &dir);
    if (rc != EXT2_ERR_OK) return rc;
    if ((dir.mode & EXT2_S_IFMT) != EXT2_S_IFDIR) return EXT2_ERR_NOT_DIR;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    uint32_t total = (dir.size + ext2_block_size - 1) / ext2_block_size;
    int placed = 0;
    for (uint32_t bi = 0; bi < total && !placed; bi++) {
        uint32_t pblock = 0;
        rc = ext2_block_map_read(&dir, bi, &pblock);
        if (rc != EXT2_ERR_OK) return rc;
        if (pblock == 0) continue;
        if (!ext2_read_block(pblock, blockbuf)) return EXT2_ERR_IO;
        uint32_t off = 0;
        uint32_t last_off = 0;
        uint16_t last_rec = 0;
        while (off + 8 <= ext2_block_size) {
            struct ext2_dir_entry *de = (struct ext2_dir_entry *)(blockbuf + off);
            if (de->rec_len == 0) break;
            last_off = off;
            last_rec = de->rec_len;
            off += de->rec_len;
            if (de->rec_len < 8) break;
            if (de->inode == 0) {
                uint32_t free_space = de->rec_len;
                if (free_space >= need) {
                    de->rec_len = (uint16_t)need;
                    struct ext2_dir_entry *nde = (struct ext2_dir_entry *)(blockbuf + off + need);
                    nde->inode = ino;
                    nde->rec_len = (uint16_t)(free_space - need);
                    nde->name_len = (uint8_t)namelen;
                    nde->file_type = type;
                    for (uint32_t i = 0; i < namelen; i++) nde->name[i] = name[i];
                    if (!ext2_write_block(pblock, blockbuf)) return EXT2_ERR_IO;
                    placed = 1;
                    break;
                }
            }
        }
        if (!placed) {
            uint32_t tail = ext2_block_size - last_off;
            if (tail >= need) {
                struct ext2_dir_entry *lde = (struct ext2_dir_entry *)(blockbuf + last_off);
                lde->rec_len = (uint16_t)last_rec;
                struct ext2_dir_entry *nde = (struct ext2_dir_entry *)(blockbuf + last_off + last_rec);
                nde->inode = ino;
                nde->rec_len = (uint16_t)(tail - last_rec);
                nde->name_len = (uint8_t)namelen;
                nde->file_type = type;
                for (uint32_t i = 0; i < namelen; i++) nde->name[i] = name[i];
                if (!ext2_write_block(pblock, blockbuf)) return EXT2_ERR_IO;
                placed = 1;
                break;
            }
        }
    }
    if (!placed) {
        uint32_t nb = 0;
        rc = ext2_alloc_block_group(0, &nb);
        if (rc != EXT2_ERR_OK) return rc;
        for (uint32_t i = 0; i < ext2_block_size; i++) blockbuf[i] = 0;
        struct ext2_dir_entry *nde = (struct ext2_dir_entry *)blockbuf;
        nde->inode = ino;
        nde->rec_len = (uint16_t)ext2_block_size;
        nde->name_len = (uint8_t)namelen;
        nde->file_type = type;
        for (uint32_t i = 0; i < namelen; i++) nde->name[i] = name[i];
        if (!ext2_write_block(nb, blockbuf)) return EXT2_ERR_IO;
        rc = ext2_block_map_write(&dir, total, nb, 1);
        if (rc != EXT2_ERR_OK) return rc;
        dir.size += ext2_block_size;
        dir.blocks = ext2_inode_blocks_count(&dir);
        rc = ext2_write_inode(dir_ino, &dir);
        if (rc != EXT2_ERR_OK) return rc;
    }
    return EXT2_ERR_OK;
}

static int ext2_dir_remove(uint32_t dir_ino, const char *name, uint32_t *removed_ino) {
    uint32_t namelen = 0;
    while (name[namelen]) namelen++;
    struct ext2_inode dir;
    int rc = ext2_read_inode(dir_ino, &dir);
    if (rc != EXT2_ERR_OK) return rc;
    if ((dir.mode & EXT2_S_IFMT) != EXT2_S_IFDIR) return EXT2_ERR_NOT_DIR;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    uint32_t total = (dir.size + ext2_block_size - 1) / ext2_block_size;
    for (uint32_t bi = 0; bi < total; bi++) {
        uint32_t pblock = 0;
        rc = ext2_block_map_read(&dir, bi, &pblock);
        if (rc != EXT2_ERR_OK) return rc;
        if (pblock == 0) continue;
        if (!ext2_read_block(pblock, blockbuf)) return EXT2_ERR_IO;
        uint32_t off = 0;
        while (off + 8 <= ext2_block_size) {
            struct ext2_dir_entry *de = (struct ext2_dir_entry *)(blockbuf + off);
            if (de->rec_len == 0) break;
            if (de->inode != 0 && de->name_len == namelen) {
                int match = 1;
                for (uint32_t i = 0; i < namelen; i++) if (de->name[i] != name[i]) { match = 0; break; }
                if (match) {
                    if (removed_ino) *removed_ino = de->inode;
                    de->inode = 0;
                    if (!ext2_write_block(pblock, blockbuf)) return EXT2_ERR_IO;
                    return EXT2_ERR_OK;
                }
            }
            off += de->rec_len;
            if (de->rec_len < 8) break;
        }
    }
    return EXT2_ERR_NOT_FOUND;
}

int ext2_create_file(const char *path, uint32_t size) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    char parent[EXT2_MAX_PATH];
    const char *base = ext2_base_name(path);
    int rc = ext2_parent_of_path(path, parent, sizeof(parent));
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t pino = (parent[0] == 0) ? ext2_current_ino : EXT2_ROOT_INO;
    if (parent[0] != 0) {
        uint8_t t = 0;
        rc = ext2_resolve_path(parent, &pino, &t);
        if (rc != EXT2_ERR_OK) return rc;
    }
    uint32_t exists_ino = 0;
    uint8_t exists_type = 0;
    rc = ext2_dir_lookup(pino, base, &exists_ino, &exists_type);
    if (rc == EXT2_ERR_OK) return EXT2_ERR_EXISTS;
    uint32_t ino = 0;
    rc = ext2_alloc_inode_group(0, &ino);
    if (rc != EXT2_ERR_OK) return rc;
    struct ext2_inode inode;
    for (uint32_t i = 0; i < 15; i++) inode.block[i] = 0;
    inode.mode = EXT2_S_IFREG | 0644;
    inode.uid = 0;
    inode.gid = 0;
    inode.size = 0;
    inode.atime = 0;
    inode.ctime = 0;
    inode.mtime = 0;
    inode.dtime = 0;
    inode.links_count = 1;
    inode.blocks = 0;
    inode.flags = 0;
    inode.generation = 0;
    inode.file_acl = 0;
    inode.dir_acl = 0;
    inode.faddr = 0;
    for (uint32_t i = 0; i < 12; i++) inode.osd2[i] = 0;
    rc = ext2_write_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_super_dec_free_inodes();
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_dir_add(pino, ino, base, EXT2_FT_REG_FILE);
    if (rc != EXT2_ERR_OK) return rc;
    if (size > 0) {
        rc = ext2_write_file(path, "", 0);
        if (rc != EXT2_ERR_OK) return rc;
    }
    return EXT2_ERR_OK;
}

int ext2_create_dir(const char *path) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    char parent[EXT2_MAX_PATH];
    const char *base = ext2_base_name(path);
    int rc = ext2_parent_of_path(path, parent, sizeof(parent));
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t pino = (parent[0] == 0) ? ext2_current_ino : EXT2_ROOT_INO;
    if (parent[0] != 0) {
        uint8_t t = 0;
        rc = ext2_resolve_path(parent, &pino, &t);
        if (rc != EXT2_ERR_OK) return rc;
    }
    uint32_t exists_ino = 0;
    uint8_t exists_type = 0;
    rc = ext2_dir_lookup(pino, base, &exists_ino, &exists_type);
    if (rc == EXT2_ERR_OK) return EXT2_ERR_EXISTS;
    uint32_t ino = 0;
    rc = ext2_alloc_inode_group(0, &ino);
    if (rc != EXT2_ERR_OK) return rc;
    struct ext2_inode inode;
    for (uint32_t i = 0; i < 15; i++) inode.block[i] = 0;
    inode.mode = EXT2_S_IFDIR | 0755;
    inode.uid = 0;
    inode.gid = 0;
    inode.size = ext2_block_size;
    inode.atime = 0;
    inode.ctime = 0;
    inode.mtime = 0;
    inode.dtime = 0;
    inode.links_count = 2;
    inode.blocks = 1;
    inode.flags = 0;
    inode.generation = 0;
    inode.file_acl = 0;
    inode.dir_acl = 0;
    inode.faddr = 0;
    for (uint32_t i = 0; i < 12; i++) inode.osd2[i] = 0;
    rc = ext2_write_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_super_dec_free_inodes();
    if (rc != EXT2_ERR_OK) return rc;
    char blockbuf[4096];
    if (ext2_block_size > sizeof(blockbuf)) return EXT2_ERR_IO;
    for (uint32_t i = 0; i < ext2_block_size; i++) blockbuf[i] = 0;
    struct ext2_dir_entry *de = (struct ext2_dir_entry *)blockbuf;
    de->inode = ino;
    de->rec_len = 12;
    de->name_len = 1;
    de->file_type = EXT2_FT_DIR;
    de->name[0] = '.';
    struct ext2_dir_entry *de2 = (struct ext2_dir_entry *)(blockbuf + 12);
    de2->inode = pino;
    de2->rec_len = (uint16_t)(ext2_block_size - 12);
    de2->name_len = 2;
    de2->file_type = EXT2_FT_DIR;
    de2->name[0] = '.';
    de2->name[1] = '.';
    uint32_t nb = 0;
    rc = ext2_alloc_block_group(0, &nb);
    if (rc != EXT2_ERR_OK) return rc;
    if (!ext2_write_block(nb, blockbuf)) return EXT2_ERR_IO;
    rc = ext2_block_map_write(&inode, 0, nb, 1);
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_write_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_dir_add(pino, ino, base, EXT2_FT_DIR);
    if (rc != EXT2_ERR_OK) return rc;
    struct ext2_inode pdir;
    rc = ext2_read_inode(pino, &pdir);
    if (rc == EXT2_ERR_OK) {
        pdir.links_count++;
        ext2_write_inode(pino, &pdir);
    }
    return EXT2_ERR_OK;
}

int ext2_delete_file(const char *path) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    char parent[EXT2_MAX_PATH];
    const char *base = ext2_base_name(path);
    int rc = ext2_parent_of_path(path, parent, sizeof(parent));
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t pino = (parent[0] == 0) ? ext2_current_ino : EXT2_ROOT_INO;
    if (parent[0] != 0) {
        uint8_t t = 0;
        rc = ext2_resolve_path(parent, &pino, &t);
        if (rc != EXT2_ERR_OK) return rc;
    }
    uint32_t ino = 0;
    uint8_t type = 0;
    rc = ext2_dir_lookup(pino, base, &ino, &type);
    if (rc != EXT2_ERR_OK) return rc;
    if (type == EXT2_FT_DIR) return EXT2_ERR_NOT_DIR;
    struct ext2_inode inode;
    rc = ext2_read_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    ext2_inode_free_blocks(&inode);
    rc = ext2_free_inode(ino);
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_super_inc_free_inodes();
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_dir_remove(pino, base, 0);
    if (rc != EXT2_ERR_OK) return rc;
    struct ext2_inode pdir;
    rc = ext2_read_inode(pino, &pdir);
    if (rc == EXT2_ERR_OK && pdir.links_count > 0) {
        pdir.links_count--;
        ext2_write_inode(pino, &pdir);
    }
    return EXT2_ERR_OK;
}

struct ext2_empty_ctx {
    int empty;
};

static int ext2_empty_cb(uint32_t cino, uint8_t ctype, const char *cname, void *ctx) {
    (void)cino;
    (void)ctype;
    struct ext2_empty_ctx *e = (struct ext2_empty_ctx *)ctx;
    if (cname[0] == '.' && cname[1] == 0) return 0;
    if (cname[0] == '.' && cname[1] == '.' && cname[2] == 0) return 0;
    e->empty = 0;
    return 1;
}

int ext2_delete_dir(const char *path) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    uint32_t ino = 0;
    uint8_t type = 0;
    int rc = ext2_resolve_path(path, &ino, &type);
    if (rc != EXT2_ERR_OK) return rc;
    if (type != EXT2_FT_DIR) return EXT2_ERR_NOT_DIR;
    if (ino == EXT2_ROOT_INO) return EXT2_ERR_NOT_EMPTY;
    int empty = 1;
    struct ext2_empty_ctx ectx;
    ectx.empty = empty;
    rc = ext2_dir_foreach(ino, ext2_empty_cb, &ectx);
    if (rc != EXT2_ERR_OK) return rc;
    if (!ectx.empty) return EXT2_ERR_NOT_EMPTY;
    char parent[EXT2_MAX_PATH];
    const char *base = ext2_base_name(path);
    rc = ext2_parent_of_path(path, parent, sizeof(parent));
    if (rc != EXT2_ERR_OK) return rc;
    uint32_t pino = (parent[0] == 0) ? ext2_current_ino : EXT2_ROOT_INO;
    if (parent[0] != 0) {
        uint8_t t = 0;
        rc = ext2_resolve_path(parent, &pino, &t);
        if (rc != EXT2_ERR_OK) return rc;
    }
    struct ext2_inode inode;
    rc = ext2_read_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    ext2_inode_free_blocks(&inode);
    rc = ext2_free_inode(ino);
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_super_inc_free_inodes();
    if (rc != EXT2_ERR_OK) return rc;
    rc = ext2_dir_remove(pino, base, 0);
    if (rc != EXT2_ERR_OK) return rc;
    struct ext2_inode pdir;
    rc = ext2_read_inode(pino, &pdir);
    if (rc == EXT2_ERR_OK && pdir.links_count > 0) {
        pdir.links_count--;
        ext2_write_inode(pino, &pdir);
    }
    return EXT2_ERR_OK;
}

int ext2_ls(const char *path) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    uint32_t ino = ext2_current_ino;
    uint8_t type = 0;
    int rc = EXT2_ERR_OK;
    if (path && path[0]) {
        rc = ext2_resolve_path(path, &ino, &type);
        if (rc != EXT2_ERR_OK) return rc;
        if (type != EXT2_FT_DIR) return EXT2_ERR_NOT_DIR;
    }
    return ext2_dir_foreach(ino, 0, 0);
}

int ext2_cd(const char *path) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    if (!path || path[0] == 0) return EXT2_ERR_INVALID;
    uint32_t ino = 0;
    uint8_t type = 0;
    int rc = ext2_resolve_path(path, &ino, &type);
    if (rc != EXT2_ERR_OK) return rc;
    if (type != EXT2_FT_DIR) return EXT2_ERR_NOT_DIR;
    ext2_current_ino = ino;
    if (path[0] == '/') {
        uint32_t len = 0;
        while (path[len] && len < EXT2_MAX_PATH - 1) { ext2_cwd[len] = path[len]; len++; }
        ext2_cwd[len] = 0;
    } else {
        if (ext2_cwd[0] == '/' && ext2_cwd[1] == 0) {
            char tmp[EXT2_MAX_PATH];
            tmp[0] = '/'; tmp[1] = 0;
            uint32_t n = 0;
            while (path[n]) { if (n + 2 < EXT2_MAX_PATH) tmp[n + 1] = path[n]; n++; }
            tmp[n + 1] = 0;
            for (uint32_t i = 0; i < EXT2_MAX_PATH; i++) ext2_cwd[i] = tmp[i];
        } else {
            uint32_t n = 0;
            while (ext2_cwd[n]) n++;
            if (n > 0 && ext2_cwd[n - 1] != '/') {
                if (n + 1 < EXT2_MAX_PATH) ext2_cwd[n++] = '/';
            }
            uint32_t m = 0;
            while (path[m]) { if (n + m < EXT2_MAX_PATH - 1) ext2_cwd[n + m] = path[m]; m++; }
            ext2_cwd[n + m] = 0;
        }
    }
    return EXT2_ERR_OK;
}

int ext2_stat(const char *path, struct ext2_stat *st) {
    if (!ext2_mounted || !st) return EXT2_ERR_IO;
    uint32_t ino = 0;
    uint8_t type = 0;
    int rc = ext2_resolve_path(path, &ino, &type);
    if (rc != EXT2_ERR_OK) return rc;
    struct ext2_inode inode;
    rc = ext2_read_inode(ino, &inode);
    if (rc != EXT2_ERR_OK) return rc;
    st->ino = ino;
    st->mode = inode.mode;
    st->size = inode.size;
    st->mtime = inode.mtime;
    st->is_directory = ((inode.mode & EXT2_S_IFMT) == EXT2_S_IFDIR) ? 1 : 0;
    st->is_symlink = ((inode.mode & EXT2_S_IFMT) == EXT2_S_IFLNK) ? 1 : 0;
    st->is_regular = ((inode.mode & EXT2_S_IFMT) == EXT2_S_IFREG) ? 1 : 0;
    return EXT2_ERR_OK;
}

int ext2_rename(const char *old_path, const char *new_path) {
    (void)old_path; (void)new_path;
    return EXT2_ERR_NOT_SUPPORTED;
}

int ext2_list_current(void) {
    return ext2_ls(ext2_cwd);
}

int ext2_reset_directory(void) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    ext2_current_ino = EXT2_ROOT_INO;
    ext2_cwd[0] = '/';
    ext2_cwd[1] = 0;
    return EXT2_ERR_OK;
}

int ext2_parent_directory(void) {
    if (!ext2_mounted) return EXT2_ERR_IO;
    uint32_t ino = 0;
    uint8_t type = 0;
    int rc = ext2_dir_lookup(ext2_current_ino, "..", &ino, &type);
    if (rc != EXT2_ERR_OK) return rc;
    ext2_current_ino = ino;
    if (ext2_cwd[0] == '/' && ext2_cwd[1] == 0) return EXT2_ERR_OK;
    uint32_t n = 0;
    while (ext2_cwd[n]) n++;
    while (n > 1 && ext2_cwd[n - 1] == '/') n--;
    while (n > 1 && ext2_cwd[n - 1] != '/') n--;
    if (n == 0) n = 1;
    ext2_cwd[n] = 0;
    if (ext2_cwd[0] == 0) { ext2_cwd[0] = '/'; ext2_cwd[1] = 0; }
    return EXT2_ERR_OK;
}

int ext2_get_current_dir_name(char *buf, uint32_t size) {
    if (!buf || size == 0) return EXT2_ERR_INVALID;
    uint32_t n = 0;
    while (ext2_cwd[n] && n < size - 1) { buf[n] = ext2_cwd[n]; n++; }
    buf[n] = 0;
    return EXT2_ERR_OK;
}

uint32_t ext2_get_current_ino(void) {
    return ext2_current_ino;
}
