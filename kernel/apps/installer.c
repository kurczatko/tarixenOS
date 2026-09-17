#include "installer.h"
#include "../../system_plikow/system_plikow.h"
#include "../../include/printf/printf.h"

struct tsi_header {
    char target_name[32];
    uint32_t data_size;
} __attribute__((packed));

void zainstaluj_pakiet_tsi(const char *nazwa_pliku) {
    char bufor[4096];
    int wynik = read_file(nazwa_pliku, bufor, sizeof(bufor));
    
    if (wynik < (int)sizeof(struct tsi_header)) {
        return;
    }
    
    struct tsi_header *header = (struct tsi_header *)bufor;
    char *file_data = bufor + sizeof(struct tsi_header);
    
    if (create_file(header->target_name, header->data_size) < 0) {
        return;
    }
    
    write_file(header->target_name, file_data, header->data_size);
}