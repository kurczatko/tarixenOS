#include "installer.h"
#include "../../system_plikow/system_plikow.h"

#define ELF_MAGIC 0x464C457F

static int jest_elf(const char *nazwa) {
    char bufor[16];
    int n = read_file(nazwa, bufor, sizeof(bufor));
    if (n < 4) return 0;
    return *(uint32_t *)bufor == ELF_MAGIC;
}

void zainstaluj_pakiet_tsi(const char *nazwa_pliku) {
    char bufor[8192];
    int wynik = read_file(nazwa_pliku, bufor, sizeof(bufor));

    if (wynik < (int)sizeof(struct tsi_header)) {
        return;
    }

    struct tsi_header *header = (struct tsi_header *)bufor;

    if (header->magic != TSI_MAGIC) {
        return;
    }

    if (!header->has_icon || header->icon_size == 0) {
        return;
    }

    uint32_t offset = sizeof(struct tsi_header);
    char *file_data = bufor + offset;
    char *icon_data = file_data + header->data_size;

    if ((int)(offset + header->data_size + header->icon_size) > wynik) {
        return;
    }

    if (create_file(header->target_name, header->data_size) < 0) {
        return;
    }

    if (write_file(header->target_name, file_data, header->data_size) < 0) {
        return;
    }

    char ikona_nazwa[64];
    int i = 0;
    while (header->target_name[i] && i < 60) {
        ikona_nazwa[i] = header->target_name[i];
        i++;
    }
    ikona_nazwa[i++] = '.';
    ikona_nazwa[i++] = 'I';
    ikona_nazwa[i++] = 'C';
    ikona_nazwa[i] = 0;

    fs_reset_directory();
    if (cd("pulpit") != 0) {
        create_dir("pulpit");
        cd("pulpit");
    }

    if (create_file(ikona_nazwa, header->icon_size) < 0) {
        cd("..");
        return;
    }

    write_file(ikona_nazwa, icon_data, header->icon_size);

    if (jest_elf(header->target_name)) {
        char desktop_nazwa[64];
        int j = 0;
        while (header->target_name[j] && j < 60) {
            desktop_nazwa[j] = header->target_name[j];
            j++;
        }
        desktop_nazwa[j] = 0;

        if (create_file(desktop_nazwa, header->data_size) >= 0) {
            write_file(desktop_nazwa, file_data, header->data_size);
        }
    }

    cd("..");
}
