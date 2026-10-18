#ifndef USERSPACE_H
#define USERSPACE_H

#include "../include/printf/types.h"

/* Przejdz do userspace (ring 3) i wykonaj kod pod `wejscie` na stosie `stos`.
 * Nie wraca do wywolujacego. */
void userspace_uruchom(uint32_t wejscie, uint32_t stos);

/* Gora dostepnego adresowego prostoru uzytkownika (ponizej ladowania jadra). */
#define GRANICA_PAMIEC_UZYTKOWNIKA 0x00100000

#endif
