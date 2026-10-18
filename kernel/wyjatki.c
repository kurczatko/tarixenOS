#include "wyjatki.h"
#include "panika.h"
#include "../include/printf/printf.h"

static const char *wyjatek_nazwa(uint32_t numer)
{
    switch (numer) {
        case 0:  return "Blad dzielenia przez zero";
        case 1:  return "Debug";
        case 3:  return "Punkt przerwania";
        case 4:  return "Przepełnienie";
        case 5:  return "Zakres indeksu przekroczony";
        case 6:  return "Nieprawidlowy kod operacji";
        case 7:  return "Urządzenie nie jest dostępne";
        case 8:  return "Blad podwojny";
        case 10: return "Nieprawidlowy TSS";
        case 11: return "Segment nie jest obecny";
        case 12: return "Blad stosu segmentu";
        case 13: return "Ogólna ochrona";
        case 14: return "Blad strony";
        case 16: return "Blad FPU";
        case 17: return "Sprawdzenie wyrównania";
        case 18: return "Sprawdzenie maszyny";
        case 19: return "Blad zmiennoprzecinkowy SIMD";
        default: return "Nieznany wyjątek";
    }
}

void wyjatek_obsluz(uint32_t numer, uint32_t kod_bledu)
{
    (void)kod_bledu;
    const char *nazwa = wyjatek_nazwa(numer);
    panika(nazwa);
}
