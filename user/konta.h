#ifndef KONTA_H
#define KONTA_H

#include <include/printf/types.h>

#define KONTA_MAKS_UZYTKOWNIKOW 32
#define KONTA_MAX_NAZWA 32
#define KONTA_MAX_HASLO 64
#define KONTA_MAX_SCIEZKA 64
#define KONTA_PLIK_DANYCH "USERS.DAT"

struct konto_uzytkownika {
    char nazwa[KONTA_MAX_NAZWA];
    uint8_t hash_hasla[32];
    char katalog_domowy[KONTA_MAX_SCIEZKA];
    uint32_t kolor_tla;
    uint8_t aktywny;
    uint8_t padding[31];
};

void konta_start(void);
uint8_t konta_czy_zalogowano(void);
void konta_obsluz(void);
const char *konta_pobierz_nazwa_uzytkownika(void);
const char *konta_pobierz_katalog_domowy(void);
void konta_zarzadzaj(void);
uint8_t konta_jest_administratorem(void);

#endif
