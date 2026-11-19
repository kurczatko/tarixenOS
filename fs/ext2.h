#ifndef EXT2_H
#define EXT2_H

#include <include/printf/types.h>

#define EXT2_ROOT_INO 2
#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_ERR_OK 0
#define EXT2_ERR_IO -1
#define EXT2_ERR_NOT_FOUND -2
#define EXT2_ERR_NO_SPACE -3
#define EXT2_ERR_NOT_DIR -4
#define EXT2_ERR_NOT_EMPTY -5
#define EXT2_ERR_EXISTS -6
#define EXT2_ERR_NOT_SUPPORTED -7
#define EXT2_ERR_INVALID -8

#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFLNK 0xA000
#define EXT2_S_IFMT 0xF000

#define EXT2_FT_UNKNOWN 0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR 2
#define EXT2_FT_CHRDEV 3
#define EXT2_FT_BLKDEV 4
#define EXT2_FT_FIFO 5
#define EXT2_FT_SOCK 6
#define EXT2_FT_SYMLINK 7

struct ext2_superblock {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint16_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t uuid[16];
    char volume_name[16];
} __attribute__((packed));

struct ext2_group_desc {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t reserved[12];
} __attribute__((packed));

struct ext2_inode {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t osd2[12];
} __attribute__((packed));

struct ext2_dir_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[];
} __attribute__((packed));

struct ext2_stat {
    uint32_t ino;
    uint16_t mode;
    uint32_t size;
    uint32_t mtime;
    uint8_t is_directory;
    uint8_t is_symlink;
    uint8_t is_regular;
};

typedef int (*ext2_dir_cb)(uint32_t ino, uint8_t type, const char *name, void *ctx);

void ext2_init(void);int ext2_mount(void);
int ext2_set_device_floppy(int use_floppy);
int ext2_is_mounted(void);
int ext2_read_block(uint32_t block, char *buffer);
int ext2_write_block(uint32_t block, const char *buffer);

int ext2_read_file(const char *path, char *buffer, uint32_t size);
int ext2_write_file(const char *path, const char *data, uint32_t size);
int ext2_create_file(const char *path, uint32_t size);
int ext2_create_dir(const char *path);
int ext2_delete_file(const char *path);
int ext2_delete_dir(const char *path);
int ext2_ls(const char *path);
int ext2_cd(const char *path);
int ext2_stat(const char *path, struct ext2_stat *st);
int ext2_rename(const char *old_path, const char *new_path);

int ext2_list_current(void);
int ext2_reset_directory(void);
int ext2_parent_directory(void);
int ext2_get_current_dir_name(char *buf, uint32_t size);
uint32_t ext2_get_current_ino(void);

#endif
