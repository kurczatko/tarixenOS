#include "system_plikow.h"
#include "../include/printf/printf.h"

typedef unsigned int uint32_t;

static void *memcpy(void *dest, const void *src, uint32_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

static void *memset(void *s, int c, uint32_t n) {
    char *p = s;
    while (n--) *p++ = c;
    return s;
}

static int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

static char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

static uint32_t strlen(const char *s) {
    uint32_t len = 0;
    while (*s++) len++;
    return len;
}

extern void read_sector(uint32_t sector, char *buffer);
extern void write_sector(uint32_t sector, const char *buffer);

static void read_block(uint32_t block, char *buffer) { read_sector(block, buffer); }
static void write_block(uint32_t block, const char *buffer) { write_sector(block, buffer); }

static struct superblock sb_cache;
static struct inode inode_cache[INODE_COUNT];
static uint32_t free_blocks[DATA_BLOCKS];
static uint32_t free_block_count = DATA_BLOCKS;
static uint32_t current_dir = 0;

static uint32_t alloc_block() { return free_block_count ? free_blocks[--free_block_count] : 0; }
static void free_block(uint32_t block) { free_blocks[free_block_count++] = block; }

static uint32_t find_inode(const char *name) {
    struct inode *dir_inode = &inode_cache[current_dir];
    if (dir_inode->type != 1) return 0;
    uint32_t block = dir_inode->data_blocks[0];
    if (block == 0) return 0;
    char data[BLOCK_SIZE];
    read_block(block, data);
    char *ptr = data;
    while (*ptr) {
        if (strcmp(ptr, name) == 0) {
            ptr += strlen(ptr) + 1;
            return *(uint32_t *)ptr;
        }
        ptr += strlen(ptr) + 1 + sizeof(uint32_t);
    }
    return 0;
}

static uint32_t alloc_inode() {
    for (uint32_t i = 0; i < INODE_COUNT; i++) if (inode_cache[i].size == 0) return i;
    return 0;
}

void init_fs() {
    read_block(0, (char *)&sb_cache);
    if (sb_cache.magic != 0xDEADBEEF) {
        sb_cache.magic = 0xDEADBEEF;
        sb_cache.total_blocks = TOTAL_BLOCKS;
        sb_cache.free_blocks = DATA_BLOCKS;
        sb_cache.inode_table_start = 1;
        write_block(0, (char *)&sb_cache);
        for (uint32_t i = 0; i < DATA_BLOCKS; i++) free_blocks[i] = i + 1 + (INODE_COUNT * sizeof(struct inode) / BLOCK_SIZE);
        free_block_count = DATA_BLOCKS;
        memset(inode_cache, 0, sizeof(inode_cache));
        write_block(1, (char *)inode_cache);
        struct inode *root = &inode_cache[0];
        root->size = 0;
        root->type = 1;
        write_block(1, (char *)inode_cache);
    } else {
        read_block(1, (char *)inode_cache);
        for (uint32_t i = 0; i < DATA_BLOCKS; i++) free_blocks[i] = i + 1 + (INODE_COUNT * sizeof(struct inode) / BLOCK_SIZE);
        free_block_count = DATA_BLOCKS;
    }
}

int create_file(const char *name, uint32_t size) {
    uint32_t inode_id = alloc_inode();
    if (inode_id == 0) return -1;
    struct inode *inode = &inode_cache[inode_id];
    inode->size = size;
    inode->type = 0;
    uint32_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (blocks_needed > 12) return -1;
    for (uint32_t i = 0; i < blocks_needed; i++) {
        inode->data_blocks[i] = alloc_block();
        if (inode->data_blocks[i] == 0) return -1;
    }
    struct inode *dir_inode = &inode_cache[current_dir];
    uint32_t dir_block = dir_inode->data_blocks[0];
    if (dir_block == 0) {
        dir_block = alloc_block();
        dir_inode->data_blocks[0] = dir_block;
    }
    char data[BLOCK_SIZE];
    read_block(dir_block, data);
    char *ptr = data;
    while (*ptr) ptr += strlen(ptr) + 1 + sizeof(uint32_t);
    strcpy(ptr, name);
    ptr += strlen(name) + 1;
    *(uint32_t *)ptr = inode_id;
    write_block(dir_block, data);
    write_block(1, (char *)inode_cache);
    return 0;
}

int read_file(const char *name, char *buffer, uint32_t size) {
    uint32_t inode_id = find_inode(name);
    if (inode_id == 0) return -1;
    struct inode *inode = &inode_cache[inode_id];
    if (inode->type != 0) return -1;
    uint32_t to_read = size < inode->size ? size : inode->size;
    uint32_t bytes_read = 0;
    for (uint32_t i = 0; i < 12 && bytes_read < to_read; i++) {
        uint32_t block = inode->data_blocks[i];
        if (block == 0) break;
        char data[BLOCK_SIZE];
        read_block(block, data);
        uint32_t block_size = BLOCK_SIZE;
        if (bytes_read + block_size > to_read) block_size = to_read - bytes_read;
        memcpy(buffer + bytes_read, data, block_size);
        bytes_read += block_size;
    }
    return bytes_read;
}

int write_file(const char *name, const char *data, uint32_t size) {
    uint32_t inode_id = find_inode(name);
    if (inode_id == 0) return -1;
    struct inode *inode = &inode_cache[inode_id];
    if (inode->type != 0) return -1;
    inode->size = size;
    uint32_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (blocks_needed > 12) return -1;
    for (uint32_t i = 0; i < 12; i++) if (inode->data_blocks[i]) free_block(inode->data_blocks[i]);
    for (uint32_t i = 0; i < blocks_needed; i++) {
        inode->data_blocks[i] = alloc_block();
        if (inode->data_blocks[i] == 0) return -1;
    }
    uint32_t bytes_written = 0;
    for (uint32_t i = 0; i < blocks_needed; i++) {
        uint32_t block = inode->data_blocks[i];
        char block_data[BLOCK_SIZE];
        uint32_t block_size = BLOCK_SIZE;
        if (bytes_written + block_size > size) block_size = size - bytes_written;
        memcpy(block_data, data + bytes_written, block_size);
        write_block(block, block_data);
        bytes_written += block_size;
    }
    write_block(1, (char *)inode_cache);
    return 0;
}

int create_dir(const char *name) {
    uint32_t inode_id = alloc_inode();
    if (inode_id == 0) return -1;
    struct inode *inode = &inode_cache[inode_id];
    inode->size = 0;
    inode->type = 1;
    struct inode *dir_inode = &inode_cache[current_dir];
    uint32_t dir_block = dir_inode->data_blocks[0];
    if (dir_block == 0) {
        dir_block = alloc_block();
        dir_inode->data_blocks[0] = dir_block;
    }
    char data[BLOCK_SIZE];
    read_block(dir_block, data);
    char *ptr = data;
    while (*ptr) ptr += strlen(ptr) + 1 + sizeof(uint32_t);
    strcpy(ptr, name);
    ptr += strlen(name) + 1;
    *(uint32_t *)ptr = inode_id;
    write_block(dir_block, data);
    write_block(1, (char *)inode_cache);
    return 0;
}

int ls(const char *dir) {
    uint32_t dir_inode_id = current_dir;
    if (dir) {
        dir_inode_id = find_inode(dir);
        if (dir_inode_id == 0) return -1;
    }
    struct inode *dir_inode = &inode_cache[dir_inode_id];
    if (dir_inode->type != 1) return -1;
    uint32_t block = dir_inode->data_blocks[0];
    if (block == 0) return 0;
    char data[BLOCK_SIZE];
    read_block(block, data);
    char *ptr = data;
    while (*ptr) {
        printf("%s\n", ptr);
        ptr += strlen(ptr) + 1 + sizeof(uint32_t);
    }
    return 0;
}

int cd(const char *dir) {
    if (strcmp(dir, "..") == 0) {
        current_dir = 0;
        return 0;
    }
    uint32_t inode_id = find_inode(dir);
    if (inode_id == 0) return -1;
    struct inode *inode = &inode_cache[inode_id];
    if (inode->type != 1) return -1;
    current_dir = inode_id;
    return 0;
}
