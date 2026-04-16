#ifndef SYSTEM_PLIKOW_H
#define SYSTEM_PLIKOW_H

// Wlasne definicje typow bo tak
typedef unsigned int uint32_t;

#define BLOCK_SIZE 512
#define TOTAL_BLOCKS 1024
#define INODE_COUNT 256
#define DATA_BLOCKS (TOTAL_BLOCKS - 1 - (INODE_COUNT * sizeof(struct inode) / BLOCK_SIZE))

struct superblock {
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t inode_table_start;
};

struct inode {
    uint32_t size;
    uint32_t type; 
    uint32_t data_blocks[12]; 
};

void init_fs();
int create_file(const char *name, uint32_t size);
int read_file(const char *name, char *buffer, uint32_t size);
int write_file(const char *name, const char *data, uint32_t size);
int create_dir(const char *name);
int ls(const char *dir);
int cd(const char *dir);

#endif