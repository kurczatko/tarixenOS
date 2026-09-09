#ifndef SYSTEM_PLIKOW_H
#define SYSTEM_PLIKOW_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define FAT32_SECTOR_SIZE 512
#define FAT32_ROOT_CLUSTER 2

void init_fs(void);
int create_file(const char *name, uint32_t size);
int read_file(const char *name, char *buffer, uint32_t size);
int write_file(const char *name, const char *data, uint32_t size);
int create_dir(const char *name);
int ls(const char *dir);
int cd(const char *dir);

#endif
