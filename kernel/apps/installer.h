#ifndef INSTALLER_H
#define INSTALLER_H

#include <include/printf/types.h>

#define TSI_MAGIC 0x49534954

struct tsi_header {
    uint32_t magic;
    char target_name[32];
    char display_name[32];
    uint32_t data_size;
    uint32_t icon_size;
    uint8_t has_icon;
    uint8_t reserved[3];
} __attribute__((packed));

void zainstaluj_pakiet_tsi(const char *nazwa_pliku);

#endif
