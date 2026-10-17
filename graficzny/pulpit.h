#ifndef PULPIT_H
#define PULPIT_H

#include "../include/printf/types.h"

void pulp_init(void);
void pulp_rysuj_ikony(void);
void pulp_odswiez(void);
/* przeciąganie ikon na pulpicie */
int pulp_mouse_press(int x, int y);
void pulp_mouse_ruch(int x, int y);
void pulp_mouse_release(int x, int y);
/* zapis i odczyt układu ikon */
int ikony_wczytaj_uklad(void);
void ikony_zapisz_uklad(void);

#endif
