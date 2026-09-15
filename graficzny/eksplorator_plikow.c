#include "eksplorator_plikow.h"
#include "okno.h"
#include "rysowanie.h"
#include "../system_plikow/system_plikow.h"

struct okno pliki;

void rysowanie_nudnych_plikow(int x, int y, int szerokosc, int wysokosc) {
    (void)x;
    (void)y;
    (void)szerokosc;
    (void)wysokosc;
}

void eksplorator_plikow() {
    okno_stworz(&pliki, 300, 300, 300, 300, "Pliki", rysowanie_nudnych_plikow);
}