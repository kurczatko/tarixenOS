#include <../graficzny/rysowanie.h>
#include <../graficzny/czcionka.h>
#include <../drivers/delay.h>

void konta() {
    wypelnij_ekran(0xFF7D7A72);
    rysuj_kwadrat(900, 900, 200, 0xFF464D47);
    rysuj_tekst_chicago(900, 900, "Prosze o utworzenie konta.", 0xFF000000);
}