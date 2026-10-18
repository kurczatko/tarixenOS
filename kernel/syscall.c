#include "syscall.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/okno.h"
#include "../graficzny/czcionka.h"
#include "../include/printf/printf.h"

struct rejestry biezace_rejestry;

static struct okno okno_tapeta_user;

static void rysuj_wybor_tapety_user(int x, int y, int szerokosc, int wysokosc)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_chicago(x, y, "WYBIERZ TAPETE", 0x00000000);
    for (int indeks = 0; indeks < 8; indeks++)
        rysuj_prostokat(x + 8 + (indeks % 4) * 32, y + 18 + (indeks / 4) * 32, 24, 24, kolory[indeks]);
}

static void obsluz_wybor_tapety_user(int x, int y)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    int kolumna;
    int wiersz;
    if (x < 16 || x >= 144 || y < 48 || y >= 112) return;
    kolumna = (x - 16) / 32;
    wiersz = (y - 48) / 32;
    if (kolumna < 0 || kolumna >= 4 || wiersz < 0 || wiersz >= 2) return;
    ustaw_kolor_tapety(kolory[wiersz * 4 + kolumna]);
    tapeta_zapisz();
}

struct rejestry biezace_rejestry;

uint32_t syscall_handler(void)
{
    uint32_t eax = biezace_rejestry.eax;
    uint32_t ebx = biezace_rejestry.ebx;
    uint32_t ecx = biezace_rejestry.ecx;
    uint32_t edx = biezace_rejestry.edx;
    uint32_t esi = biezace_rejestry.esi;
    uint32_t edi = biezace_rejestry.edi;

    switch (eax) {
        case SYSCALL_TAPETA_ZMIEŃ:
            ustaw_kolor_tapety((uint32_t)ebx);
            tapeta_zapisz();
            return 0;
        case SYSCALL_TAPETA_POBIERZ:
            return 0;
        case SYSCALL_EKRAN_CZYŚĆ:
            wypelnij_ekran((uint32_t)ebx);
            return 0;
        case SYSCALL_EKRAN_PIKSEL:
            rysuj_piksel((int)ebx, (int)ecx, (uint32_t)edx);
            return 0;
        case SYSCALL_EKRAN_PROSTOKĄT:
            rysuj_prostokat((int)ebx, (int)ecx, (int)edx, (int)esi, (uint32_t)edi);
            return 0;
        case SYSCALL_TEKST_RYSUJ:
            rysuj_tekst_chicago((int)ebx, (int)ecx, (const char *)edx, (uint32_t)esi);
            return 0;
        case SYSCALL_OKNO_STWÓRZ:
            okno_stworz(&okno_tapeta_user, (int)ebx, (int)ecx, (int)edx, (int)esi, "TAPETA", rysuj_wybor_tapety_user);
            okno_ustaw_obsluge_klikniecia(&okno_tapeta_user, obsluz_wybor_tapety_user);
            return 0;
        case SYSCALL_OKNO_RYSUJ:
            odswiez_widok();
            return 0;
        case SYSCALL_MYSZ_POZYCJA:
            return 0;
        case SYSCALL_DRUKUJ:
            printf((const char *)ebx);
            return 0;
        default:
            return 0xFFFFFFFF;
    }
}

void syscall_install(void)
{
}

void syscall_zapisz_rejestry(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi)
{
    biezace_rejestry.eax = eax;
    biezace_rejestry.ebx = ebx;
    biezace_rejestry.ecx = ecx;
    biezace_rejestry.edx = edx;
    biezace_rejestry.esi = esi;
    biezace_rejestry.edi = edi;
}
