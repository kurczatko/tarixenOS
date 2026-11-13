# dokumentacja techniczna tarixenOS

to jest aktualna dokumentacja funkcji systemu. stara dokumentacja byla z poczatku projektu i nie pasuje do obecnego kodu.

wszystkie nazwy sa po polsku, poza nazewnictwem technicznym i plikami.

## uruchomienie systemu

system uruchamia sie z boot.asm. boot.asm laduje jadro z dysku, przechodzi w tryb chroniony i skacze do `kernel_main` w `kernel/kernel.c`.

kompilacja:

```
make
cd build
qemu-system-x86_64 -m 2G -boot c -drive file=tarixenOS.img,format=raw -machine pc -display sdl
```

musisz miec qemu.

## jadro - kernel/kernel.c

`kernel_main` to punkt wejscia jadra. robi nastepujace rzeczy:

1. wyzerowanie segmentu BSS
2. inicjalizacja rysowania (`rysowanie_init`)
3. inicjalizacja zegara (`zegar_init`)
4. inicjalizacja systemu plikow (`init_fs`)
5. inicjalizacja stacji dyskow (`floppy_init`, `nvme_init`, `intel_uhd_init`)
6. ladunek tapety (`tapeta_wczytaj`)
7. instalacja GDT (`gdt_install`), IDT (`idt_install`), syscall (`syscall_init`)
8. inicjalizacja sieci (`net_init`)
9. inicjalizacja pulpitu (`pulp_init`, `tapeta_app`)
10. wyswietlenie menu start (`menu_start`)
11. inicjalizacja myszy i klawiatury (`mouse_init`, `keyboard_init`, `usb_init`)
12. dzwiek (`init_intel_hda`, `alc650_init`)
13. konta uzytkownikow (`konta_start`)
14. PIT i timer (`pik_init`, `timer_init`)

po inicjalizacji `kernel_main` wchodzi w nieskonczona petle ktora obsluguje zdarzenia: czas, mysz, klawiatura, okna, menu start.

### proces_uruchom_pierwszy

uruchamia pierwszy proces uzytkownika. ustawia TSS (esp0, cr3), pobiera rejestry eip i esp z struktury procesu i wywoluje `userspace_uruchom`.

```
void proces_uruchom_pierwszy(struct proces *p)
```

## zaawansowany harmonogram procesow - kernel/proces.cpp

sterownik harmonogramu procesow (scheduler) napisany w C++ zastepuje uproszczony round-robin z kernel/proces.c. implementuje wielopoziomowa kolejke z opinia zwrotna (MLFQ), sprawiedliwe dzielenie CPU w stylu CFS (Completely Fair Scheduler), stale priorytety dla procesow czasu rzeczywistego, kolejki oczekiwania, dziedziczenie priorytetu, obudzenie po czasie, statystyki per-proces oraz dynamiczne dostosowanie kwantumu czasu. harmonogram dziala w trybie wywlaszczajacym (preempcyjnym) wywolywany z obslugi przerwania timera (timer_handler -> proces_tick). kontekst sprzetowy (rejestry, CR3, TSS esp0) jest przelaczany przez `kernel/timer_entry.S` na podstawie wskaznika `current_process` wybranego przez `proces_tick`.

klasy planowania:
- SCHED_CRITICAL - najwyzszy priorytet, procesy krytyczne systemu, kwantum 2 tiki, FIFO w ramach priorytetu.
- SCHED_REALTIME - procesy czasu rzeczywistego, stale priorytety, kwantum 2 tiki, FIFO w ramach priorytetu.
- SCHED_INTERACTIVE - procesy interaktywne, wiekszy kwantum (6 tikow), kolejka CFS posortowana po vruntime.
- SCHED_NORMAL - procesy zwykle, domyslny kwantum 4 tiki, kolejka CFS posortowana po vruntime.
- SCHED_BACKGROUND - procesy w tle, kwantum 8 tikow, nizszy priorytet.
- SCHED_IDLE - proces bezczynny (idle), najnizszy priorytet, uruchamiany gdy brak gotowych procesow.

zakres priorytetow: 0 (najwyzszy) do 139 (najnizszy). idle ma priorytet 139.

### proces_init

inicjalizuje harmonogram: zeruje tablice procesow, czysci kolejki gotowosci i oczekiwania, odczytuje page directory jadra z CR3, tworzy proces bezczynny (idle).

```
void proces_init(void)
```

### proces_stworz

tworzy nowy proces. przydziela slot, nadaje pid, kopiuje page directory jadra, ustawia ramke rejestrow (esp, ebp, eip, cs, ds, es, fs, gs, ss, eflags) dla powrotu do userspace przez iret, przydziela domyslna klase SCHED_NORMAL i priorytet 120, dodaje do kolejki gotowosci. zwraca wskaznik na proces lub 0 przy brak miejsca.

```
struct proces *proces_stworz(uint32_t entry_point, const char *name, uint32_t arg)
```

### proces_zakoncz

oznacza proces o danym pid jako zakonczony (TERMINATED), usuwa z kolejki gotowosci lub oczekiwania, zwalnia slot. jesli proces byl biezacy, wylacza current_process.

```
void proces_zakoncz(uint32_t pid)
```

### proces_tick

glowna petla harmonogramu wywolywana co tik timera (100 Hz). zwieksza licznik tikow, obudza procesy ktorych czas snju nastapil, aktualizuje vruntime i suma_exec_runtime procesu biezacego, wyczerpuje kwantum czasu i nastepnym tiku przenosi proces do kolejki gotowosci, wybiera nastepny proces do uruchomienia (najwyzszy priorytet, w ramach klasy NORMAL/INTERACTIVE proces z najmniejszym vruntime) i ustawia current_process. timer_entry.S wykonuje rzeczywiste przelaczenie kontekstu (rejestry, CR3, TSS).

```
void proces_tick(void)
```

### proces_aktualny

zwraca wskaznik na aktualnie wykonywany proces.

```
struct proces *proces_aktualny(void)
```

### proces_yield

wywolanie systemowe (int 0x80, eax=29). przenosi proces biezacy do kolejki gotowosci, resetuje jego kwantum i wybiera nastepny proces.

```
void proces_yield(void)
```

### proces_pobierz_pid

zwraca pid aktualnego procesu lub 0.

```
uint32_t proces_pobierz_pid(void)
```

### proces_sleep_until

wstawia proces biezacy w stan SLEEPING na az do podanego tikta, dodaje do kolejki oczekiwania i przelacza na nastepny proces.

```
void proces_sleep_until(uint64_t ticks)
```

### proces_set_priority

zmienia priorytet procesu o danym pid i przepisuje go w kolejce gotowosci. zwraca poprzedni priorytet lub -1 przy bledzie.

```
int proces_set_priority(uint32_t pid, int32_t priority)
```

### proces_set_sched_class

zmienia klase planowania procesu (CRITICAL/REALTIME/INTERACTIVE/NORMAL/BACKGROUND/IDLE) i dopasowuje kwantum czasu.

```
int proces_set_sched_class(uint32_t pid, sched_class_t cls)
```

### proces_block

wstawia proces biezacy w stan BLOCKED (oczekiwanie na zasob), dodaje do kolejki oczekiwania i przelacza na nastepny proces.

```
void proces_block(void)
```

### proces_wakeup

obudza proces z kolejki oczekiwania (BLOCKED lub SLEEPING), ustawia stan READY i dodaje do kolejki gotowosci.

```
void proces_wakeup(struct proces *p)
```

### proces_find

szuka procesu po pid. zwraca wskaznik lub 0.

```
struct proces *proces_find(uint32_t pid)
```

### proces_get_stats

kopiuje pelna strukture procesu (statystyki, priorytet, vruntime, czas wykonania) do bufora wyjsciowego. zwraca 0 lub -1.

```
int proces_get_stats(uint32_t pid, struct proces *out)
```

### proces_klonuj_page_directory

klonuje page directory jadra dla nowego procesu: kopiuje wpisy jadra, mapuje pamiec userspace (1MB-4MB) oraz ramke graficzna (0xFD000000). zwraca nowy page directory.

```
uint32_t *proces_klonuj_page_directory(void)
```

### proces_rejestruj_okno

rejestruje okno dla procesu o podanej nazwie lub tworzy nowy proces okna. ustawia wskaznik okna i dodaje proces do kolejki gotowosci.

```
void proces_rejestruj_okno(const char *nazwa, struct okno *okno)
```

### struktura proces

| pole | typ | opis |
|------|-----|------|
| pid | uint32_t | unikalny identyfikator procesu |
| state | proces_state_t | stan (TERMINATED/READY/RUNNING/BLOCKED/SLEEPING/ZOMBIE) |
| sched_class | sched_class_t | klasa planowania (CRITICAL/REALTIME/INTERACTIVE/NORMAL/BACKGROUND/IDLE) |
| priority | int32_t | priorytet 0-139 (0 najwyzszy) |
| static_priority | int32_t | bazowy priorytet bez dziedziczenia |
| time_slice | uint32_t | pozostaly kwantum czasu w tikach |
| vruntime | uint64_t | wirtualny czas wykonania (CFS, sprawiedliwe dzielenie) |
| sum_exec_runtime | uint64_t | calkowity czas wykonania w ns |
| sleep_until | uint64_t | tik obudzenia dla procesu spiacego |
| page_directory | uint32_t * | adres page directory procesu |
| kernel_stack | uint32_t | wierzcholek stosu jadra (TSS esp0) |
| user_stack | uint32_t | wierzcholek stosu userspace |
| entry_point | uint32_t | adres wejscia procesu |
| ctx_switches | uint64_t | liczba przelaczen kontekstu |
| nr_wakeups | uint64_t | liczba obudzen |
| nr_blocked | uint64_t | liczba zablokowan |
| run_next / run_prev | struct proces * | lacza kolejki gotowosci |
| wait_next / wait_prev | struct proces * | lacza kolejke oczekiwania |

## stronicowanie - kernel/paging.c

### paging_init

inicjalizuje tablice stron i page directory. mapuje pamiec od 0 do 4MB (userspace od 1MB do 4MB jako user), od 4MB do 8MB oraz ramke graficzna pod 0xFD000000. wlacza stronicowanie w rejestrze CR0.

```
void paging_init(void)
```

### map_page

wpisuje mapowanie strony w podanej tablicy stron. vaddr na paddr z podanymi flagami.

```
void map_page(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
```

flagi: PAGE_PRESENT (0x1), PAGE_READWRITE (0x2), PAGE_USER (0x4).

## przerwania i syscall - kernel/syscall.c

### syscall_handler

glowna obsluga wywolan systemowych. czyta eax jako numer funkcji i reszte rejestrow jako argumenty. zwraca 0xFFFFFFFF przy bledzie.

dostepne syscally:

| numer | nazwa | opis |
|-------|-------|------|
| 0 | SYSCALL_TAPETA_ZMIEN | zmienia kolor tapety z ebx |
| 1 | SYSCALL_TAPETA_POBIERZ | zwraca kolor tapety |
| 2 | SYSCALL_EKRAN_CZYSC | wypelnia ekran kolorem z ebx |
| 3 | SYSCALL_EKRAN_PIKSEL | rysuje piksel (ebx, ecx) kolorem edx |
| 4 | SYSCALL_EKRAN_PROSTOKAT | rysuje prostokat (ebx, ecx, edx, esi) kolorem edi |
| 5 | SYSCALL_TEKST_RYSUJ | rysuje tekst z edx na (ebx, ecx) kolorem esi |
| 6 | SYSCALL_OKNO_STWORZ | tworzy okno, zwraca slot |
| 7 | SYSCALL_OKNO_RYSUJ | rysuje okno z slotu ebx |
| 8 | SYSCALL_MYSZ_POZYCJA | zwraca pozycje myszy |
| 9 | SYSCALL_DRUKUJ | drukuje tekst z ebx |
| 20 | SYSCALL_WRITE_CONSOLE | wypisuje ecx znakow z ebx |
| 21 | SYSCALL_EXIT | konczy proces |
| 22 | SYSCALL_BRK | ustawia break sterty uzytkownika |
| 23 | SYSCALL_OPEN | otwiera plik, zwraca fd |
| 24 | SYSCALL_READ | czyta z pliku fd do bufora ecx, edx bajtow |
| 25 | SYSCALL_CLOSE | zamyka plik fd |
| 26 | SYSCALL_WRITE_FILE | zapisuje edx bajtow z ecx do pliku fd |
| 27 | SYSCALL_PROCES_STWORZ | tworzy proces z ebx, ecx, edx |
| 28 | SYSCALL_PROCES_ZAKONCZ | konczy proces ebx |
| 29 | SYSCALL_PROCES_YIELD | przekazuje CPU |
| 30 | SYSCALL_PROCES_POBIERZ_PID | zwraca pid |
| 31 | SYSCALL_FS_LIST | listuje pliki |
| 32 | SYSCALL_FS_CD | zmienia katalog |
| 33 | SYSCALL_FS_PWD | zwraca sciezke katalogu |
| 34 | SYSCALL_FS_RESET_DIR | resetuje katalog |
| 35 | SYSCALL_OKNO_ZAMKNIJ | zamyka okno z slotu ebx |
| 36 | SYSCALL_OKNO_PIKSEL | rysuje piksel w oknie |
| 37 | SYSCALL_OKNO_PROSTOKAT | rysuje prostokat w oknie |
| 38 | SYSCALL_OKNO_TEKST | rysuje tekst w oknie |
| 39 | SYSCALL_OKNO_KLIK | obsluga klikniecia w oknie |
| 40 | SYSCALL_OKNO_RUCH | obsluga ruchu myszy w oknie |
| 41 | SYSCALL_KLAWIATURA_GETCHAR | pobiera znak z klawiatury |
| 42 | SYSCALL_PROCES_URUCHOM_ELF | laduje i uruchamia ELF z ebx |

```
uint32_t syscall_handler(void)
```

### syscall_install

instaluje obsluga syscall w IDT.

```
void syscall_install(void)
```

### syscall_zapisz_rejestry

zapisuje aktualne rejestry do struktury biezace_rejestry.

```
void syscall_zapisz_rejestry(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi)
```

## timer - kernel/timer.c

### timer_init

inicjalizuje timer z podana czestoscia.

```
void timer_init(uint32_t frequency)
```

### timer_handler

obsluguje przerwanie timera, wywoluje proces_tick.

```
void timer_handler(void)
```

## IDT - kernel/idt.c

### idt_install

instaluje tablice przerwan IDT.

```
void idt_install(void)
```

### idt_set_gate

ustawia brame przerwania o danym numerze, adresie, selektorze i flagach.

```
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
```

## GDT - kernel/gdt.c

### gdt_install

instaluje Global Descriptor Table.

```
void gdt_install(void)
```

### gdt_set_ring

zmienia piersten (ring) procesora.

```
void gdt_set_ring(uint8_t ring)
```

### gdt_tss

zwraca wskaznik na TSS.

```
struct tss *gdt_tss(void)
```

### gdt_kernel_stack_top

zwraca wierzcholek stosu jadra.

```
uint32_t gdt_kernel_stack_top(void)
```

## panika - kernel/panika.c

### panika

wyswietla ekran paniki z podanym powodem i zatrzymuje system.

```
void panika(const char *powod)
```

## wyjatki - kernel/wyjatki.c

obsluguje wyjatki procesora (dzielenie przez zero, page fault itp.).

## zegar - kernel/zegar.c

### zegar_init

inicjalizuje zegar systemowy.

```
void zegar_init(void)
```

### zegar_pobierz

pobiera aktualny czas do bufora 9 znakow (format HH:MM:SS).

```
int zegar_pobierz(char tekst[9])
```

### zegar_synchronizuj_http

synchronizuje czas przez HTTP z podanej odpowiedzi.

```
int zegar_synchronizuj_http(const char *response, uint32_t length)
```

## konfiguracja - kernel/konfiguracja.c

### konfiguracja

wyswietla ekran powitalny systemu. inicjalizuje klawiature i VGA, wyswietla tekst powitalny i czeka na enter.

```
void konfiguracja()
```

## loader - kernel/loader.c

### userspace_load_elf

laduje plik ELF z sciezki, zwraca adres wejscia.

```
int userspace_load_elf(const char *path, uint32_t *entry_out)
```

### userspace_load_and_run

laduje i uruchamia ELF.

```
int userspace_load_and_run(const char *path)
```

### proces_zaladuj_elf

laduje ELF jako proces uzytkownika.

```
int proces_zaladuj_elf(const char *path)
```

## terminal - kernel/terminal.c

obsluguje terminal systemowy.

## notatnik - kernel/notatnik.c

### notatnik_otworz

otwiera aplikacje notatnik.

```
void notatnik_otworz(void)
```

### notatnik_otworz_plik

otwiera podany plik w notatniku.

```
void notatnik_otworz_plik(const char *nazwa_pliku)
```

### notatnik_obsluz_klawiature

obsluguje klawiatura w notatniku.

```
void notatnik_obsluz_klawiature(void)
```

## kalkulator - kernel/kalkulator.c

### kalkulator_otworz

otwiera kalkulator.

```
void kalkulator_otworz(void)
```

### kalkulator_obsluz_klawiature

obsluguje klawiatura w kalkulatorze.

```
void kalkulator_obsluz_klawiature(void)
```

## przegladarka - kernel/browser.c

### browser_open

otwiera przegladarke.

```
void browser_open(void)
```

### browser_handle_keyboard

obsluguje klawiatura w przegladarce.

```
void browser_handle_keyboard(void)
```

### browser_scroll_wheel

obsluguje kolesko myszy.

```
void browser_scroll_wheel(int delta)
```

## konta uzytkownikow - user_land/konta.c

### konta_start

inicjalizuje system kont.

```
void konta_start(void)
```

### konta_czy_zalogowano

zwraca 1 gdy uzytkownik jest zalogowany, 0 gdy nie.

```
uint8_t konta_czy_zalogowano(void)
```

### konta_obsluz

obsluguje logowanie uzytkownika.

```
void konta_obsluz(void)
```

## tapeta - user_land/tapeta.c

obsluguje tlo pulpitu. funkcje tapeta_wczytaj, tapeta_zapisz, ustaw_kolor_tapety, pobierz_kolor_tapety sa w graficzny/rysowanie.c.

## okno uzytkownika - user_land/okno_user.c

okno dla aplikacji uzytkownika.

## grafika - graficzny/rysowanie.c

### rysowanie_init

inicjalizuje subsystem graficzny.

```
void rysowanie_init(void)
```

### narysuj_pasek_zadan

rysuje pasek zadan na dole ekranu.

```
void narysuj_pasek_zadan(void)
```

### wypelnij_ekran

wypelnia caly ekran podanym kolorem.

```
void wypelnij_ekran(uint32_t kolor)
```

### rysuj_piksel

rysuje pojedynczy piksel na (x, y) podanym kolorem.

```
void rysuj_piksel(int x, int y, uint32_t kolor)
```

### rysuj_prostokat

rysuje prostokat o podanym rozmiarze i kolorze.

```
void rysuj_prostokat(int x, int y, int szerokosc, int wysokosc, uint32_t kolor)
```

### rysuj_kwadrat

rysuje kwadrat o boku bok.

```
void rysuj_kwadrat(int x, int y, int bok, uint32_t kolor)
```

### kursor_graficzny_init

inicjalizuje kursor myszy.

```
void kursor_graficzny_init(void)
```

### kursor_graficzny_przesun

przesuwa kursor o delta_x, delta_y.

```
void kursor_graficzny_przesun(int delta_x, int delta_y)
```

### kursor_graficzny_pozycja

zwraca aktualna pozycje kursora.

```
void kursor_graficzny_pozycja(int *x, int *y)
```

### odswiez_widok

odswieza caly ekran.

```
void odswiez_widok(void)
```

### odswiez_fragment_widoku

odswieza wybrany fragment ekranu.

```
void odswiez_fragment_widoku(int x, int y, int szerokosc, int wysokosc)
```

### odtworz_fragment_tla

przywraca fragment tla.

```
void odtworz_fragment_tla(int x, int y, int szerokosc, int wysokosc)
```

### pobierz_kolor_tapety

zwraca aktualny kolor tapety.

```
uint32_t pobierz_kolor_tapety(void)
```

### ustaw_kolor_tapety

ustawia kolor tapety.

```
void ustaw_kolor_tapety(uint32_t kolor)
```

### ustaw_status_internetu

ustawia ikone internetu na pasku zadan.

```
void ustaw_status_internetu(uint8_t dostepny)
```

### ustaw_czas_paska

ustawia czas na pasku zadan.

```
void ustaw_czas_paska(const char *czas)
```

### tapeta_wczytaj

laduje tapete z dysku.

```
int tapeta_wczytaj(void)
```

### tapeta_zapisz

zapisuje tapete na dysk.

```
void tapeta_zapisz(void)
```

### kursor_tekstowy_tick

aktualizuje kursor tekstowy (miganie).

```
void kursor_tekstowy_tick(void)
```

### kursor_tekstowy_widoczny

zwraca 1 gdy kursor jest widoczny.

```
uint8_t kursor_tekstowy_widoczny(void)
```

### kursor_tekstowy_rysuj

rysuje kursor tekstowy na (x, y).

```
void kursor_tekstowy_rysuj(int x, int y)
```

stale:

| nazwa | wartosc | opis |
|-------|---------|------|
| EKRAN_SZEROKOSC | 1024 | szerokosc ekranu |
| EKRAN_WYSOKOSC | 768 | wysokosc ekranu |
| PASEK_ZADAN_WYSOKOSC | 30 | wysokosc paska zadan |
| KOLOR_CZARNY | 0x00000000 | czarny |
| KOLOR_BIALY | 0x00FFFFFF | bialy |
| KOLOR_SZARY | 0x00808080 | szary |
| KOLOR_JASNOSZARY | 0x00C0C0C0 | jasnoszary |
| KOLOR_NIEBIESKI | 0x000000FF | niebieski |
| KOLOR_CZERWONY | 0x00FF0000 | czerwony |
| KOLOR_ZIELONY | 0x0000FF00 | zielony |
| KOLOR_ZOLTY | 0x00FFFF00 | zolty |

## okna - graficzny/okno.c

### okno_stworz

tworzy okno na pozycji (x, y) o rozmiarze (szerokosc, wysokosc) z tytulem i funkcja rysowania.

```
void okno_stworz(struct okno *okno, int x, int y, int szerokosc, int wysokosc,
                 const char *tytul, okno_rysuj_zawartosc rysuj_zawartosc)
```

### okno_narysuj

rysuje okno na ekranie.

```
void okno_narysuj(struct okno *okno)
```

### okno_zamknij

zamyka okno.

```
void okno_zamknij(struct okno *okno)
```

### okno_obsluz_klikniecie

obsluguje klikniecie myszy na pozycji (x, y).

```
int okno_obsluz_klikniecie(int x, int y)
```

### okno_obsluz_ruch

obsluguje ruch myszy na pozycji (x, y).

```
void okno_obsluz_ruch(int x, int y)
```

## czcionka - graficzny/czcionka.c

obsluguje rysowanie tekstu czcionka Chicago. funkcja rysuj_tekst_chicago.

## menu start - graficzny/menu_start.c

### menu_start

wyswietla menu start.

```
void menu_start()
```

### menu_start_click

obsluguje klikniecie w menu start.

```
void menu_start_click(void)
```

### menu_start_mouse_click

obsluguje klikniecie myszy w menu.

```
void menu_start_mouse_click(void)
```

### menu_start_przelacz

wlacza/wylacza menu start.

```
void menu_start_przelacz(void)
```

## pulpit - graficzny/pulpit.c

### pulp_init

inicjalizuje pulpit.

```
void pulp_init(void)
```

### pulp_rysuj_ikony

rysuje ikony na pulpicie.

```
void pulp_rysuj_ikony(void)
```

### pulp_odswiez

odswieza pulpit.

```
void pulp_odswiez(void)
```

### pulp_mouse_press

obsluguje nacisniecie myszy na pulpicie.

```
int pulp_mouse_press(int x, int y)
```

### pulp_mouse_ruch

obsluguje ruch myszy na pulpicie.

```
void pulp_mouse_ruch(int x, int y)
```

### pulp_mouse_release

obsluguje puszczenie myszy na pulpicie.

```
void pulp_mouse_release(int x, int y)
```

### ikony_wczytaj_uklad

laduje uklad ikon z dysku.

```
int ikony_wczytaj_uklad(void)
```

### ikony_zapisz_uklad

zapisuje uklad ikon na dysk.

```
void ikony_zapisz_uklad(void)
```

## sterownik klawiatury - drivers/keyboard.c

### keyboard_init

inicjalizuje klawiature.

```
void keyboard_init(void)
```

### keyboard_dane_dostepne

zwraca 1 gdy jest znak do odczytu.

```
uint8_t keyboard_dane_dostepne(void)
```

### keyboard_getchar

pobiera znak z klawiatury (blokujace).

```
char keyboard_getchar(void)
```

### keyboard_try_getchar

probuje pobrac znak, zwraca 1 gdy jest znak.

```
uint8_t keyboard_try_getchar(char *character)
```

### keyboard_discard_pending

odrzuca oczekujace znaki.

```
void keyboard_discard_pending(void)
```

### keyboard_scancode_to_char

konwertuje kod skanu na znak z uwzglednieniem shift.

```
uint8_t keyboard_scancode_to_char(uint8_t scancode, uint8_t shift)
```

### keyboard_windows_pressed

zwraca 1 gdy klawisz Windows jest wcisniety.

```
uint8_t keyboard_windows_pressed(void)
```

### keyboard_ctrl_pressed

zwraca 1 gdy klawisz Ctrl jest wcisniety.

```
uint8_t keyboard_ctrl_pressed(void)
```

## sterownik myszy - drivers/mouse.c

### mouse_init

inicjalizuje mysz.

```
void mouse_init(void)
```

### mouse_poll

odczytuje zdarzenia myszy.

```
void mouse_poll(void)
```

### mouse_pozycja

zwraca pozycje myszy.

```
void mouse_pozycja(int *x, int *y)
```

## dyskietka - drivers/floppy.c

### floppy_init

inicjalizuje stacje dyskietek.

```
void floppy_init(void)
```

### floppy_read_sector

czeka sektor z dyskietki do bufora.

```
int floppy_read_sector(uint32_t sector, char *buffer)
```

### floppy_write_sector

zapisuje sektor na dyskietke z bufora.

```
int floppy_write_sector(uint32_t sector, const char *buffer)
```

### floppy_present

zwraca 1 gdy stacja dyskietek jest obecna.

```
int floppy_present(void)
```

## siec - drivers/net.c

### net_init

inicjalizuje stos sieciowy.

```
int net_init(void)
```

### net_ready

zwraca 1 gdy siec jest gotowa.

```
int net_ready(void)
```

### net_http_get

wysyla zapytanie HTTP GET do hosta i sciezki, zapisuje odpowiedz do bufora.

```
int net_http_get(const char *host, uint16_t port, const char *path, char *response, uint32_t capacity)
```

### net_tcp_connect

laczy sie przez TCP z hostem na podany port.

```
int net_tcp_connect(struct net_tcp_connection *connection, const char *host, uint16_t port)
```

### net_tcp_send

wysyla dane przez TCP.

```
int net_tcp_send(struct net_tcp_connection *connection, const uint8_t *data, uint16_t length)
```

### net_tcp_receive

odbiera dane przez TCP.

```
int net_tcp_receive(struct net_tcp_connection *connection, uint8_t *data, uint16_t capacity, uint32_t attempts)
```

### net_tcp_close

zamyka polaczenie TCP.

```
void net_tcp_close(struct net_tcp_connection *connection)
```

## sterownik RTL8168B - drivers/internet/rtl8168b.cpp

sterownik karty sieciowej Realtek RTL8168B / RTL8111B (PCIe Gigabit Ethernet) napisany w C++. obsluguje wysylanie i odbieranie ramek ethernet, integracja ze stosem sieciowym (`drivers/net.c`) przez funkcje `send`/`receive` takze przegladarka (`kernel/browser.c`) korzysta z tej karty automatycznie gdy jest dostepna.

### rtl8168b_init

inicjalizuje sterownik RTL8168B: wyszukuje urzadzenie na magistrali PCI, mapuje adresy MMIO, resetuje chip, czytuje adres MAC, konfiguruje rejestry TCR/RCR, ustawia bufor odbiorczy i deskryptory nadawcze, wlacza TX i RX.

```
int rtl8168b_init(void)
```

zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### rtl8168b_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy.

```
int rtl8168b_is_ready(void)
```

### rtl8168b_get_mac

pobiera adres MAC karty do tablicy 6 bajtow.

```
int rtl8168b_get_mac(uint8_t mac[6])
```

zwraca 1 przy sukcesie, 0 gdy sterownik nie jest gotowy.

### rtl8168b_send

wysyla ramke ethernet przez karte. oczekuje az deskryptor nadawczy zostanie zwolniony przez karte (wlasciciel = sterownik).

```
int rtl8168b_send(const void *frame, uint16_t length)
```

zwraca 1 przy sukcesie, 0 przy bledzie nadawania, -1 przy blednych argumentach.

### rtl8168b_receive

odbiera nastepna dostepna ramke z bufora odbiorczego do podanego bufora. tryb polling (bez przerwan), sprawdza rejestr ISR.

```
int rtl8168b_receive(void *frame, uint16_t capacity)
```

zwraca dlugosc odebranej ramki w bajtach, 0 gdy brak danych, -1 przy blednych argumentach.

### rtl8168b_stats_get

pobiera statystyki sterownika (liczniki pakietow i bajtow TX/RX oraz bledy).

```
void rtl8168b_stats_get(struct rtl8168b_stats *out)
```

struktura `rtl8168b_stats` zawiera:

| pole | opis |
|------|------|
| tx_packets | liczba wyslanych pakietow |
| tx_bytes | liczba wyslanych bajtow |
| tx_errors | bledy nadawania |
| rx_packets | liczba odebranych pakietow |
| rx_bytes | liczba odebranych bajtow |
| rx_errors | bledy odbioru |
| resets | liczba resetow chipa |

## sterownik Realtek RTL8111F - drivers/internet/rtl8111f.cpp

sterownik karty sieciowej Realtek RTL8111F (PCIe Gigabit Ethernet) napisany w C++. obsluguje wykrycie urzadzenia na PCI (vendor 0x10EC, urzadzenia 0x8168/0x8167/0x8169/0x8171/0x8172), mapowanie MMIO, soft-reset chipa, odczyt adresu MAC, dostep do rejestrow PHY, auto-negocjacje, monitorowanie lacza (link status), predkosc i tryb duplex, bufor odbiorczy z deskryptorami nadawczymi (8 deskryptorow TX), wysylanie i odbieranie ramek ethernet w trybie polling, statystyki TX/RX z licznikami bledow (underrun, aborted, carrier), test petli (loopback), konfiguracja multicast, wylaczenie sterownika oraz resetowanie statystyk. integracja ze stosem sieciowym (`drivers/net.c`) jako pierwszy priorytet przed RTL8111E/RTL8168B, co oznacza ze przegladarka (`kernel/browser.c`) automatycznie korzysta z tej karty gdy jest dostepna i zainicjowana.

### rtl8111f_init

inicjalizuje sterownik RTL8111F: wyszukuje urzadzenie na magistrali PCI, mapuje adresy MMIO, aktywuje magistrale PCI (bus master + memory space), resetuje chip (zatrzymanie TX/RX, soft reset), czytuje adres MAC z rejestrow chipu, czysci przerwania, konfiguruje TCR (bez loopback) oraz RCR (akceptuje wlasny MAC, broadcast, multicast, ramki fizyczne, bufor 16K), ustawia adres bufora odbiorczego i deskryptory nadawcze, wlacza start, TX i RX.

```
int rtl8111f_init(void)
```

zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### rtl8111f_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy.

```
int rtl8111f_is_ready(void)
```

### rtl8111f_get_mac

pobiera adres MAC karty sieciowej do bufora 6-bajtowego.

```
int rtl8111f_get_mac(uint8_t mac[6])
```

zwraca 1 przy sukcesie, 0 gdy sterownik nie jest gotowy.

### rtl8111f_send

wysyla ramke ethernet przez karte. oczekuje az deskryptor nadawczy zostanie zwolniony przez karte (wlasciciel = sterownik), kopiuje ramke do bufora nadawczego, ustawia bity OWN i dlugosc w deskryptorze TSD oraz oczekuje na zakonczenie nadawania przez karte. aktualizuje statystyki TX (pakiety, bajty, bledy, underrun, aborted, carrier).

```
int rtl8111f_send(const void *frame, uint16_t length)
```

zwraca 1 przy sukcesie, 0 przy bledzie nadawania (deskryptor zajety), -1 przy blednych argumentach.

### rtl8111f_receive

odbiera nastepna dostepna ramke z bufora odbiorczego do podanego bufora. tryb polling (bez przerwan), sprawdza rejestr ISR, potwierdza przerwanie odbioru, parsuje naglowek ramki (status + dlugosc) i kopiuje dane do bufora wyjsciowego. aktualizuje statystyki RX (pakiety, bajty, bledy).

```
int rtl8111f_receive(void *frame, uint16_t capacity)
```

zwraca dlugosc odebranej ramki w bajtach, 0 gdy brak danych, -1 przy blednych argumentach.

### rtl8111f_link_up

sprawdza stan lacza (link status) poprzez odczyt rejestru BMSR PHY. aktualizuje wewnetrzny stan lacza i zwraca 1 gdy lacze jest aktywne.

```
int rtl8111f_link_up(void)
```

### rtl8111f_link_info

pobiera pelne informacje o laczu do struktury `rtl8111f_link_info` (link_up, full_duplex, autoneg_done, speed_mbps).

```
int rtl8111f_link_info(struct rtl8111f_link_info *info)
```

zwraca 1 przy sukcesie, 0 przy bledzie.

### rtl8111f_speed_mbps

zwraca predkosc lacza w Mbps (10, 100 lub 1000) lub 0 gdy lacze jest nieaktywne.

```
int rtl8111f_speed_mbps(void)
```

### rtl8111f_duplex_full

zwraca 1 gdy lacze pracuje w trybie full-duplex, 0 gdy half-duplex lub lacze nieaktywne.

```
int rtl8111f_duplex_full(void)
```

### rtl8111f_autonegotiate

uruchamia auto-negocjacje PHY (reset + autoneg), czeka na zakonczenie negocjacji (BMSR_AN_DONE i BMSR_LINK) i aktualizuje informacje o laczu.

```
int rtl8111f_autonegotiate(void)
```

zwraca 1 gdy auto-negocjacje zakonczyly sie sukcesem i lacze jest aktywne, 0 w przeciwnym razie.

### rtl8111f_reset_link

resetuje PHY i ponownie uruchamia auto-negocjacje.

```
int rtl8111f_reset_link(void)
```

zwraca 1 gdy lacze zostalo przywrocone, 0 przy bledzie.

### rtl8111f_loopback_test

wykonuje test petli (loopback): wlacza loopback w TCR, wysyla ramke testowa, odbiera ja z powrotem i porownuje zawartosc. nastepnie wraca do poprzedniej wartosci TCR.

```
int rtl8111f_loopback_test(const void *frame, uint16_t length)
```

zwraca 1 gdy test powiodl sie (ramka wyslana i odebrana identycznie), 0 przy bledzie, -1 przy blednych argumentach.

### rtl8111f_set_multicast

konfiguruje filtr multicast (hash table MAR0/MAR1). obecna implementacja ustawia wszystkie bity na 1 (akceptuje wszystkie adresy multicast).

```
void rtl8111f_set_multicast(const uint8_t *addresses, uint32_t count)
```

### rtl8111f_stats_get

pobiera statystyki sterownika do struktury `rtl8111f_stats` (liczniki pakietow i bajtow TX/RX, bledy nadawania i odbioru, unterrun, aborted, carrier, rx_missed, rx_crc_errors, rx_frame_errors, rx_fifo_errors, resets, link_changes, autoneg_failures).

```
void rtl8111f_stats_get(struct rtl8111f_stats *out)
```

### rtl8111f_stats_reset

zeruje wszystkie liczniki statystyk sterownika.

```
void rtl8111f_stats_reset(void)
```

### rtl8111f_shutdown

zatrzymuje karte (stop TX/RX, soft reset) i oznacz sterownik jako niegotowy.

```
void rtl8111f_shutdown(void)
```

## sterownik RTL8188CE - drivers/internet/rtl8188ce.cpp

sterownik karty sieciowej Realtek RTL8188CE (PCIe 802.11n WiFi) napisany w C++. obsluguje wykrycie urzadzenia na PCI, mapowanie MMIO, odczyt adresu MAC, inicjalizacje modulu WiFi (kontroler, kanal, interwal beaconow), wysylanie i odbieranie ramek ethernet oraz statystyki TX/RX. integracja ze stosem sieciowym (`drivers/net.c`) przez funkcje `send`/`receive` takze przegladarka (`kernel/browser.c`) korzysta z tej karty automatycznie gdy jest dostepna i zainicjowana.

### rtl8188ce_init

inicjalizuje sterownik RTL8188CE: wyszukuje urzadzenie na magistrali PCI, mapuje adresy MMIO, resetuje chip, czytuje adres MAC, inicjalizuje modul WiFi (wlancza kontroler, ustawia kanal domyslny 1 dla 2.4 GHz i interwal beaconow), wlacza TX i RX, oraz ustawia adresy buforow odbiorczych i deskryptorow nadawczych.

```
int rtl8188ce_init(void)
```

zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### rtl8188ce_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy.

```
int rtl8188ce_is_ready(void)
```

### rtl8188ce_get_mac

pobiera adres MAC karty WiFi do tablicy 6 bajtow.

```
int rtl8188ce_get_mac(uint8_t mac[6])
```

zwraca 1 przy sukcesie, 0 gdy sterownik nie jest gotowy.

### rtl8188ce_send

wysyla ramke ethernet przez karte WiFi. oczekuje az karta zwolni deskryptor nadawczy (bity ownership w rejestrze TX_STATUS).

```
int rtl8188ce_send(const void *frame, uint16_t length)
```

zwraca 1 przy sukcesie, 0 przy bledzie nadawania, -1 przy blednych argumentach.

### rtl8188ce_receive

odbiera nastepna dostepna ramke z bufora odbiorczego do podanego bufora. tryb polling (bez przerwan), sprawdza rejestr ISR i potwierdza przerwanie odbioru.

```
int rtl8188ce_receive(void *frame, uint16_t capacity)
```

zwraca dlugosc odebranej ramki w bajtach, 0 gdy brak danych, -1 przy blednych argumentach.

### rtl8188ce_stats_get

pobiera statystyki sterownika (liczniki pakietow i bajtow TX/RX oraz bledy).

```
void rtl8188ce_stats_get(struct rtl8188ce_stats *out)
```

struktura `rtl8188ce_stats` zawiera:

| pole | opis |
|------|------|
| tx_packets | liczba wyslanych pakietow |
| tx_bytes | liczba wyslanych bajtow |
| tx_errors | bledy nadawania |
| rx_packets | liczba odebranych pakietow |
| rx_bytes | liczba odebranych bajtow |
| rx_errors | bledy odbioru |
| resets | liczba resetow chipa |

## sterownik intel 82578dc wifi - rust/drivers/wifi/intel_82578dc.rs

sterownik karty sieciowej Intel 82578DC (PCIe Gigabit Ethernet / WiFi) napisany w Rust. obsluguje wykrycie urzadzenia na PCI (vendor 0x8086, urzadzenie 0x1502), mapowanie MMIO, soft-reset chipa, konfiguracje adresu MAC, deskryptory nadawcze i odbiorcze (TX/RX), przerwania, odbiornik i nadajnik, monitorowanie lacza, wysylanie i odbieranie ramek ethernet oraz statystyki TX/RX. napisany w Rust z `#[no_std]` i `#[no_main]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/wifi/*.rs)`) i linkowany do jadra. integracja ze stosem sieciowym (`drivers/net.c`) jako pierwszy priorytet przed Realtek/E1000, co oznacza ze przegladarka (`kernel/browser.c`) automatycznie korzysta z tej karty gdy jest dostepna i zainicjowana.

### intel_82578dc_driver_init

inicjalizuje sterownik: wykrywa urzadzenie na PCI, mapuje MMIO, wykonuje soft-reset, konfiguruje adres MAC, deskryptory RX/TX, przerwania, wlacza odbiornik i nadajnik, ustawia lacze jako aktywne. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

```
bool intel_82578dc_driver_init(void);
```

### intel_82578dc_driver_detected

sprawdza czy karta Intel 82578DC jest wykryta na PCI (porownuje vendor ID i device ID). zwraca 1 gdy wykryta, 0 gdy nie.

```
bool intel_82578dc_driver_detected(void);
```

### intel_82578dc_send_packet

wysyla ramke ethernet przez karte Intel 82578DC. zapisuje dane do deskryptora nadawczego (TX) i uruchamia nadajnik przez rejestr TDT. zwraca 1 po udanym wyslaniu, 0 gdy lacze nie jest aktywne lub deskryptor TX jest zajety.

```
bool intel_82578dc_send_packet(const void *data, uint32_t len);
```

### intel_82578dc_receive_packet

odbiera ramke ethernet z karty Intel 82578DC. czytuje dane z deskryptora odbiorczego (RX) do bufora i przesuwa glowe odbiorcza (RDH). zwraca dlugosc odebranej ramki w bajtach, -1 gdy brak danych lub bledne argumenty.

```
int intel_82578dc_receive_packet(void *buffer, uint32_t len);
```

### intel_82578dc_link_up

sprawdza czy lacze sieciowe jest aktywne. zwraca 1 gdy lacze jest w gore, 0 gdy nie.

```
bool intel_82578dc_link_up(void);
```

### intel_82578dc_stats_packets_sent

zwraca liczbe wyslanych pakietow.

```
uint32_t intel_82578dc_stats_packets_sent(void);
```

### intel_82578dc_stats_packets_received

zwraca liczbe odebranych pakietow.

```
uint32_t intel_82578dc_stats_packets_received(void);
```

### intel_82578dc_stats_bytes_sent

zwraca liczbe wyslanych bajtow.

```
uint32_t intel_82578dc_stats_bytes_sent(void);
```

### intel_82578dc_stats_bytes_received

zwraca liczbe odebranych bajtow.

```
uint32_t intel_82578dc_stats_bytes_received(void);
```

### intel_82578dc_stats_errors

zwraca liczbe bledow.

```
uint32_t intel_82578dc_stats_errors(void);
```

### intel_82578dc_set_callbacks

ustawia funkcje zwrotne (send/receive) dla sterownika. uzywane przez integracje ze stosem sieciowym C (`drivers/net.c`).

```
void intel_82578dc_set_callbacks(int (*send_fn)(const void *, uint16_t), int (*receive_fn)(void *, uint16_t));
```

### intel_82578dc_mac_address

kopiuje 6-bajtowy adres MAC karty do bufora.

```
void intel_82578dc_mac_address(uint8_t *buffer);
```

## sterownik bluetooth - drivers/bluetooth/bluetooth.h

sterownik bluetooth dla placas glownej MSI MPG Z890 CARBON WIFI napisany w C++. implementuje pelny stos bluetooth zgodny ze specyfikacja bluetooth core: hci (host controller interface) po usb (transport hci:usb), l2cap (logical link control and adaptation protocol), a2dp (advanced audio distribution profile) dla sluchawek bluetooth, avctp (audio video control transport protocol) oraz sdp (service discovery protocol). obsluguje parowanie i bonding urzadzen oraz wyswietlanie ikony na pasku zadan z oknem wyboru urzadzenia do polaczenia.

### bluetooth_init

inicjalizuje stos bluetooth: wykrywa kontroler na usb (intel lub realtek), inicjalizuje hci, l2cap oraz a2dp, wlacza kontroler i przygotowuje do pracy.

```
int bluetooth_init(void)
```

zwraca 1 przy sukcesie, 0 gdy kontroler nie zostanie wykryty lub inicjalizacja nie powiedzie sie.

### bluetooth_is_ready

zwraca 1 gdy sterownik bluetooth jest zainicjowany i gotowy do pracy.

```
int bluetooth_is_ready(void)
```

### bluetooth_power_set / bluetooth_power_get

wlacza lub wylacza kontroler bluetooth oraz zwraca aktualny stan zasilania.

```
int bluetooth_power_set(uint8_t enable);
int bluetooth_power_get(void);
```

### bluetooth_inquiry_start / bluetooth_inquiry_stop / bluetooth_is_discovering

rozpoczyna skanowanie urzadzen bluetooth (inquiry), zatrzymuje skanowanie lub zwraca informacje czy skanowanie trwa.

```
int bluetooth_inquiry_start(void);
int bluetooth_inquiry_stop(void);
int bluetooth_is_discovering(void);
```

### bluetooth_device_count / bluetooth_device_get / bluetooth_device_find

pobiera liczbe wykrytych urzadzen, informacje o urzadzeniu po indeksie lub znajduje urzadzenie po adresie.

```
uint8_t bluetooth_device_count(void);
const struct bt_device *bluetooth_device_get(uint8_t index);
struct bt_device *bluetooth_device_find(const struct bt_addr *addr);
```

struktura `bt_device` zawiera:

| pole | opis |
|------|------|
| addr | adres bluetooth (bd_addr) |
| dev_class | klasa urzadzenia (3 bajty) |
| rssi | moc sygnalu |
| name | nazwa urzadzenia (64 bajty) |
| device_type | typ urzadzenia (sluchawki, glosnik, telefon, komputer) |
| bonded | czy urzadzenie jest zsparowane |
| connected | czy urzadzenie jest polaczone |
| handle | uchwyt polaczenia hci |
| services | lista uslug sdp |
| service_count | liczba uslug |

### bluetooth_connect / bluetooth_disconnect / bluetooth_is_connected

nawiazuje polaczenie z urzadzeniem bluetooth, rozlacza sie z urzadzeniem lub zwraca informacje czy urzadzenie jest polaczone.

```
int bluetooth_connect(struct bt_device *device);
int bluetooth_disconnect(struct bt_device *device);
int bluetooth_is_connected(struct bt_device *device);
```

### bluetooth_pair / bluetooth_unpair / bluetooth_is_bonded

sparowuje sie z urzadzeniem (wymiana kluczy laczenia, kod pin), usuwa parowanie lub zwraca informacje czy urzadzenie jest zsparowane.

```
int bluetooth_pair(struct bt_device *device);
int bluetooth_unpair(struct bt_device *device);
int bluetooth_is_bonded(struct bt_device *device);
```

### bluetooth_set_local_name / bluetooth_get_local_name / bluetooth_get_local_addr

ustawia nazwe lokalnego urzadzenia bluetooth, zwraca nazwe lokalnego urzadzenia lub adres lokalnego urzadzenia.

```
int bluetooth_set_local_name(const char *name);
const char *bluetooth_get_local_name(void);
const struct bt_addr *bluetooth_get_local_addr(void);
```

### a2dp - sluchawki bluetooth

#### bluetooth_a2dp_open

otwiera strumien audio a2dp do sluchawek bluetooth.

```
int bluetooth_a2dp_open(struct bt_device *device)
```

#### bluetooth_a2dp_close

zamyka strumien audio a2dp.

```
int bluetooth_a2dp_close(struct bt_device *device)
```

#### bluetooth_a2dp_send

wysyla ramke dzwiekowa sbc do sluchawek.

```
int bluetooth_a2dp_send(struct bt_device *device, const uint8_t *data, uint32_t length)
```

#### bluetooth_a2dp_streaming

zwraca 1 gdy strumien audio jest aktywny.

```
int bluetooth_a2dp_streaming(struct bt_device *device)
```

### interfejs uzytkownika (pasek zadan i okno parowania)

#### bluetooth_ui_toggle

otwiera lub zamyka okno parowania bluetooth po kliknieciu ikony na pasku zadan.

```
void bluetooth_ui_toggle(void)
```

#### bluetooth_ui_mouse_click

obsluguje klikniecie myszy na ikonie bluetooth na pasku zadan. zwraca 1 gdy klikniecie bylo na ikonie bluetooth.

```
int bluetooth_ui_mouse_click(int x, int y)
```

#### bluetooth_ui_draw_icon / bluetooth_ui_icon_at

rysuje ikone bluetooth na pasku zadan lub sprawdza czy pozycja (x, y) jest nad ikona bluetooth.

```
void bluetooth_ui_draw_icon(int x, int y);
int bluetooth_ui_icon_at(int x, int y);
```

#### bluetooth_ui_init / bluetooth_poll / bluetooth_ui_rysuj_pasek / bluetooth_ui_update_state

inicjalizuje interfejs uzytkownika bluetooth, wykonuje polling warstw bluetooth, rysuje ikone na pasku zadan lub aktualizuje stan ikony.

```
void bluetooth_ui_init(void);
void bluetooth_poll(void);
void bluetooth_ui_rysuj_pasek(void);
void bluetooth_ui_update_state(void);
```

### bluetooth_context_get

pobiera kontekst bluetooth (do odczytu).

```
void bluetooth_context_get(struct bt_context *out);
```

## system plikow - system_plikow/system_plikow.c

system plikow FAT32.

### init_fs

inicjalizuje system plikow.

```
void init_fs(void)
```

### create_file

tworzy plik o podanej nazwie i rozmiarze.

```
int create_file(const char *name, uint32_t size)
```

### read_file

czyta plik do bufora.

```
int read_file(const char *name, char *buffer, uint32_t size)
```

### write_file

zapisuje dane do pliku.

```
int write_file(const char *name, const char *data, uint32_t size)
```

### create_dir

tworzy katalog.

```
int create_dir(const char *name)
```

### ls

listuje pliki w katalogu.

```
int ls(const char *dir)
```

### cd

zmienia katalog biezacy.

```
int cd(const char *dir)
```

### fs_list_current

listuje aktualny katalog do tablicy entries.

```
int fs_list_current(struct fs_entry_info *entries, int capacity)
```

### fs_reset_directory

resetuje katalog do roota.

```
void fs_reset_directory(void)
```

### fs_parent_directory

przechodzi do katalogu nadrzednego.

```
int fs_parent_directory(void)
```

### fs_get_current_dir

zwraca biezacy katalog.

```
uint32_t fs_get_current_dir(void)
```

### fs_get_current_dir_name

zapisuje nazwe biezacego katalogu do bufora.

```
void fs_get_current_dir_name(char *buf)
```

### fs_set_current_dir

ustawia biezacy katalog.

```
void fs_set_current_dir(uint32_t dir)
```

### fs_fat_next

zwraca nastepny cluster w FAT.

```
uint32_t fs_fat_next(uint32_t cluster)
```

### fs_read_sector

czeka sektor z urzadzenia.

```
void fs_read_sector(uint32_t sector, char *buffer)
```

### fs_write_sector

zapisuje sektor na urzadzenie.

```
void fs_write_sector(uint32_t sector, const char *buffer)
```

### fs_cluster_sector

zwraca numer sektora dla clusteru.

```
uint32_t fs_cluster_sector(uint32_t cluster)
```

### fs_find_entry

szuka wpisu w katalogu po nazwie.

```
struct fat32_entry *fs_find_entry(const char *name, char *sector, uint32_t *sector_number)
```

### fs_alloc_cluster

alokuje nowy cluster.

```
uint32_t fs_alloc_cluster(void)
```

### fs_set_device_floppy

ustawia urzadzenie na dyskietke.

```
void fs_set_device_floppy(int use_floppy)
```

### fs_is_floppy

zwraca 1 gdy urzadzeniem jest dyskietka.

```
int fs_is_floppy(void)
```

## opoznienie - drivers/delay.c

### delay

czeka podany czas.

## USB - drivers/usb/core/usb.c

inicjalizacja i obsluga magistrali USB.

### usb_init

inicjalizuje USB.

### usb_poll

obsluguje zdarzenia USB.

## dzwiek - drivers/sound/intel_hd.c

### init_intel_hda

inicjalizuje dzwiek Intel HD Audio.

### alc650_init

inicjalizuje kodek ALC650.

## sterownik dzwieku - drivers/sound/audigy2.cpp

sterownik Creative Sound Blaster Audigy 2 (EMU10K2) napisany w C++. obsluguje pelny dzwiek przestrzenny 3D oraz standard EAX.

### klasa Audigy2

glowna klasa sterownika. singleton dostepny przez `Audigy2::instance()`.

```
static Audigy2* instance();
```

### probe

sprawdza czy urzadzenie to Audigy 2 (vendor 0x1102, device 0x0004).

```
uint8_t probe(uint16_t vendor, uint16_t device);
```

### init / deinit

inicjalizuje mikser, SPDIF, EAX i bufora DMA. `deinit` zatrzymuje odtwarzanie i zwalnia DMA.

```
void init(void);
void deinit(void);
```

### irq_handler

obsluguje przerwanie sprzegtu z karty dzwiekowej.

```
void irq_handler(void);
```

### PCM

```
int pcm_open(uint16_t channels, uint32_t rate, uint16_t format);
int pcm_write(const void* data, uint32_t bytes);
int pcm_read(void* data, uint32_t bytes);
void pcm_close(void);
```

otwiera strumien PCM, zapisuje/odczytuje dane przez bufor DMA i zamyka strumien.

### dzwiek przestrzenny 3D (64 glosy)

```
int voice3d_alloc(Voice3D* voice);
void voice3d_free(Voice3D* voice);
void voice3d_set_position(Voice3D* voice, float x, float y, float z);
void voice3d_set_velocity(Voice3D* voice, float x, float y, float z);
void voice3d_set_gain(Voice3D* voice, float gain);
void voice3d_play(Voice3D* voice, const void* buffer, uint32_t size);
void voice3d_stop(Voice3D* voice);
```

alokuje jeden z 64 glosow 3D, ustawia jego polozenie, predkosc, wzmocnienie, odtwarza bufor dzwiekowy i zatrzymuje glos. polozenie wplywa na glosnosc przez tlumienie odleglosciowe (min_distance / max_distance).

struktura `Voice3D` zawiera: voice_id, active, pos_x/y/z, velocity_x/y/z, orientation_x/y/z, gain, min_distance, max_distance, buffer_addr, buffer_size.

### EAX (Environmental Audio Extensions)

```
int eax_enable(uint8_t enable);
int eax_set_environment(const EaxEnvironment* env);
int eax_set_listener(float x, float y, float z,
                     float fx, float fy, float fz,
                     float ux, float uy, float uz);
```

wlacz/wylacz EAX, ustaw srodowisko dzwiekowe (pokoj, room_hf, odbicia, poglos, czas rozpadu, dyfuzyjnosc, gestosc, flagi) oraz polozenie i orientacje sluchacza.

struktura `EaxEnvironment` zawiera: room, room_hf, reflections, reflections_delay, reverb, reverb_delay, decay_time, decay_hf_ratio, reflections_hf, reverb_hf, diffusion, density, flags.

### glosnosc glowna

```
void set_master_volume(uint8_t percent);
uint8_t get_master_volume(void);
```

ustawia i zwraca glosnosc glowna w procentach (0-100).

## karta graficzna - drivers/karty_graficzne/gma_x4500.cpp

sterownik karty graficznej Intel GMA X4500HD (Gen4 GPU, chipset G45) napisany w C++. obsluguje potok wyswietlania 2D, blitter, ramke graficzna, kursor sprzetowy oraz pelna akceleracje 3D (Gen4 pipeline: vertex/index buffory, tekstury, shadery, render targety, depth-stencil, command buffer, presentation).

### gma_x4500_init

inicjalizuje sterownik GMA X4500HD: wyszukuje urzadzenie na PCI, mapuje adresy MMIO, czysci przerwania, wylacza wyswietlanie, konfiguruje DPLL, timing potoku, powierzchnie A, wlacza pipe i transcoder, inicjalizuje ring buffer blitera, kursor sprhetowy oraz potok 3D Gen4.

```
int gma_x4500_init(void)
```

zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### gma_x4500_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy.

```
int gma_x4500_is_ready(void)
```

### gma_x4500_putpixel

rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem.

```
void gma_x4500_putpixel(uint32_t x, uint32_t y, uint32_t color)
```

### gma_x4500_fillrect

wypelnia prostokat o podanym rozmiarze i kolorze. uzywa blitera sprzetowego gdy sterownik jest zainicjowany, w przeciwnym razie petla CPU.

```
void gma_x4500_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
```

### gma_x4500_blt

kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) uzywajac blitera sprhetowego XY_SRC_COPY_BLT.

```
void gma_x4500_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
```

### gma_x4500_cursor_set

ustawia kursor sprhetowy na pozycji (x, y) z podanym obrazem 64x64 (kopiowany do pamieci ramki graficznej).

```
void gma_x4500_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
```

### gma_x4500_cursor_hide

ukrywa kursor sprhetowy.

```
void gma_x4500_cursor_hide(void)
```

### gma_x4500_get_fb

zwraca wskaznik na ramke graficzna (framebuffer).

```
void *gma_x4500_get_fb(void)
```

### gma_x4500_get_width / gma_x4500_get_height

zwracaja szerokosc i wysokosc ekranu (1920x1080).

```
uint32_t gma_x4500_get_width(void);
uint32_t gma_x4500_get_height(void);
```

### gma_x4500_get_device_id / gma_x4500_get_revision

zwracaja id urzadzenia i revisje karty graficznej.

```
uint16_t gma_x4500_get_device_id(void);
uint8_t gma_x4500_get_revision(void);
```

### gma_x4500_info_get

pobiera pelne informacje o urzadzeniu do struktury gma_x4500_info (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized).

```
void gma_x4500_info_get(struct gma_x4500_info *out)
```

## akceleracja 3D - drivers/karty_graficzne/gma_x4500.cpp

### tworzenie i niszczenie zasobow 3D

#### gma_x4500_vertex_buffer_create

tworzy vertex buffer (wierzcholki). zwraca handle lub GMA_HANDLE_NULL przy bledzie.

```
gma_handle_t gma_x4500_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
```

#### gma_x4500_vertex_buffer_destroy

niszczy vertex buffer o podanym handle.

```
void gma_x4500_vertex_buffer_destroy(gma_handle_t handle)
```

#### gma_x4500_vertex_buffer_update

aktualizuje dane vertex buffera od podanego offsetu.

```
void gma_x4500_vertex_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### gma_x4500_index_buffer_create

tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe). zwraca handle.

```
gma_handle_t gma_x4500_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
```

#### gma_x4500_index_buffer_destroy / gma_x4500_index_buffer_update

niszczy index buffer lub aktualizuje jego dane.

```
void gma_x4500_index_buffer_destroy(gma_handle_t handle);
void gma_x4500_index_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### gma_x4500_texture_create

tworzy teksture 2D o podanym rozmiarze, formacie i liczbie poziom mipmap. zwraca handle.

```
gma_handle_t gma_x4500_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
```

#### gma_x4500_texture_destroy / gma_x4500_texture_update

niszczy teksture lub aktualizuje jej zawartosc (upload z pamieci systemowej do FB).

```
void gma_x4500_texture_destroy(gma_handle_t handle);
void gma_x4500_texture_update(gma_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
```

#### gma_x4500_render_target_create / gma_x4500_render_target_destroy

tworzy lub niszczy render target (bufor koloru do renderowania 3D).

```
gma_handle_t gma_x4500_render_target_create(uint32_t width, uint32_t height, uint32_t format);
void gma_x4500_render_target_destroy(gma_handle_t handle)
```

#### gma_x4500_depth_stencil_create / gma_x4500_depth_stencil_destroy

tworzy lub niszczy depth-stencil buffer (bufor glebi i stencil).

```
gma_handle_t gma_x4500_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format);
void gma_x4500_depth_stencil_destroy(gma_handle_t handle)
```

#### gma_x4500_shader_create / gma_x4500_shader_destroy

tworzy shader (vertex/pixel/geometry) z kodu bajtowego GPU lub niszczy shader.

```
gma_handle_t gma_x4500_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void gma_x4500_shader_destroy(gma_handle_t handle)
```

#### gma_x4500_sampler_state_create / gma_x4500_sampler_state_destroy

tworzy lub niszczy stan probki tekstury (filtrowanie, adresowanie, anisotropia).

```
gma_handle_t gma_x4500_sampler_state_create(uint32_t filter, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy);
void gma_x4500_sampler_state_destroy(gma_handle_t handle)
```

#### gma_x4500_blend_state_create / gma_x4500_blend_state_destroy

tworzy lub niszczy stan blendowania (kolor wyjsciowy vs render target).

```
gma_handle_t gma_x4500_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask);
void gma_x4500_blend_state_destroy(gma_handle_t handle)
```

#### gma_x4500_rasterizer_state_create / gma_x4500_rasterizer_state_destroy

tworzy lub niszczy stan rasterizera (wypelnienie, cullowanie, scissor, multisample).

```
gma_handle_t gma_x4500_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line);
void gma_x4500_rasterizer_state_destroy(gma_handle_t handle)
```

#### gma_x4500_depth_stencil_state_create / gma_x4500_depth_stencil_state_destroy

tworzy lub niszczy stan depth-stencil (testy glebi i stencil).

```
gma_handle_t gma_x4500_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void gma_x4500_depth_stencil_state_destroy(gma_handle_t handle)
```

#### gma_x4500_input_layout_create / gma_x4500_input_layout_destroy

tworzy lub niszczy input layout (mapowanie atrybutow vertex do slotow).

```
gma_handle_t gma_x4500_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void gma_x4500_input_layout_destroy(gma_handle_t handle)
```

#### gma_x4500_constant_buffer_create / gma_x4500_constant_buffer_destroy / gma_x4500_constant_buffer_update

tworzy, niszczy lub aktualizuje constant buffer (stale shadera / uniforms).

```
gma_handle_t gma_x4500_constant_buffer_create(uint32_t size);
void gma_x4500_constant_buffer_destroy(gma_handle_t handle);
void gma_x4500_constant_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

### bindowanie stanow potoku 3D

#### gma_x4500_bind_vertex_buffer / gma_x4500_bind_index_buffer / gma_x4500_bind_input_layout

binduje vertex buffer, index buffer lub input layout do potoku 3D.

```
void gma_x4500_bind_vertex_buffer(gma_handle_t handle);
void gma_x4500_bind_index_buffer(gma_handle_t handle);
void gma_x4500_bind_input_layout(gma_handle_t handle)
```

#### gma_x4500_bind_vertex_shader / gma_x4500_bind_pixel_shader / gma_x4500_bind_geometry_shader

binduje shader vertex, pixel lub geometry do potoku 3D.

```
void gma_x4500_bind_vertex_shader(gma_handle_t handle);
void gma_x4500_bind_pixel_shader(gma_handle_t handle);
void gma_x4500_bind_geometry_shader(gma_handle_t handle)
```

#### gma_x4500_bind_render_target / gma_x4500_bind_depth_stencil

binduje render target lub depth-stencil do potoku 3D.

```
void gma_x4500_bind_render_target(gma_handle_t handle);
void gma_x4500_bind_depth_stencil(gma_handle_t handle)
```

#### gma_x4500_bind_blend_state / gma_x4500_bind_rasterizer_state / gma_x4500_bind_depth_stencil_state

binduje stan blendowania, rasterizera lub depth-stencil do potoku 3D.

```
void gma_x4500_bind_blend_state(gma_handle_t handle);
void gma_x4500_bind_rasterizer_state(gma_handle_t handle);
void gma_x4500_bind_depth_stencil_state(gma_handle_t handle)
```

#### gma_x4500_bind_sampler_and_texture / gma_x4500_bind_constant_buffer

binduje probke tekstury i teksture do slotu lub constant buffer do slotu.

```
void gma_x4500_bind_sampler_and_texture(uint32_t slot, gma_handle_t sampler, gma_handle_t texture);
void gma_x4500_bind_constant_buffer(uint32_t slot, gma_handle_t handle)
```

### konfiguracja potoku 3D

#### gma_x4500_set_viewport

ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi.

```
void gma_x4500_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
```

#### gma_x4500_set_scissor

ustawia prostokat przyciecia renderowania (scissor).

```
void gma_x4500_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
```

#### gma_x4500_set_primitive_topology

ustawia typ primitivu do rysowania (punkt, linia, trojkat itp.).

```
void gma_x4500_set_primitive_topology(uint32_t topology)
```

### rysowanie 3D

#### gma_x4500_clear_render_target

czysci render target podanym kolorem.

```
void gma_x4500_clear_render_target(gma_handle_t rt, uint32_t color)
```

#### gma_x4500_clear_depth_stencil

czysci depth-stencil podanymi wartosciami glebi i stencil.

```
void gma_x4500_clear_depth_stencil(gma_handle_t ds, float depth, uint8_t stencil)
```

#### gma_x4500_draw

rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex.

```
void gma_x4500_draw(uint32_t vertex_count, uint32_t start_vertex)
```

#### gma_x4500_draw_indexed

rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex.

```
void gma_x4500_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
```

### command buffer (akceleracja 3D w strumieniu)

#### gma_x4500_command_begin / gma_x4500_command_end

rozpoczyna nagrywanie polecen do command buffer lub zakoncza nagrywanie i wykonuje wszystkie polecenia.

```
void gma_x4500_command_begin(void);
void gma_x4500_command_end(void)
```

### presentation (swap chain)

#### gma_x4500_swap_chain_create / gma_x4500_swap_chain_destroy / gma_x4500_present

tworzy swap chain (podwojne buforowanie z opcja vsync), niszczy swap chain lub przebija tylny bufor na przedni (present).

```
gma_handle_t gma_x4500_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync);
void gma_x4500_swap_chain_destroy(gma_handle_t handle);
void gma_x4500_present(gma_handle_t chain)
```

### informacje o akceleracji 3D

#### gma_x4500_context_get / gma_x4500_3d_available

pobiera biezacy kontekst 3D lub zwraca 1 gdy akceleracja 3D jest dostepna (Gen4 pipeline).

```
void gma_x4500_context_get(struct gma_x4500_context *out);
int gma_x4500_3d_available(void)
```

## karta graficzna - drivers/karty_graficzne/radeon_hd_8370d.cpp

sterownik karty graficznej AMD Radeon HD 8370D (GCN, Graphics Core Next, chipset Richland) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP), przezroczystosc (alpha blending, alpha to coverage), tekstury z mipmapami, shadery (vertex/pixel/geometry/compute), render targety, depth-stencil, command buffer oraz presentation. sterownik opiera sie na architekturze GCN 3.0 (Southern Islands).

### radeon_hd_8370d_init

inicjalizuje sterownik Radeon HD 8370D: wyszukuje urzadzenie na PCI (vendor 0x1002, urzadzenia 0x9907/0x9908/0x9909), mapuje adresy MMIO, czysci przerwania, resetuje Command Processor (ME i PFP), inicjalizuje ring buffer CP, konfiguruje silnik wyswietlania (CRTC0, plane, surface), wlacza potok 3D GCN (color buffer, depth buffer, blend, rasterizer, shader SQ, tekstury, sampler) oraz inicjalizuje kursor sprxetowy.

```
int radeon_hd_8370d_init(void)
```

zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### radeon_hd_8370d_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy.

```
int radeon_hd_8370d_is_ready(void)
```

### radeon_hd_8370d_putpixel

rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem.

```
void radeon_hd_8370d_putpixel(uint32_t x, uint32_t y, uint32_t color)
```

### radeon_hd_8370d_fillrect

wypelnia prostokat o podanym rozmiarze i kolorze. uzywa ring buffer CP gdy sterownik jest zainicjowany, w przeciwnym razie petla CPU.

```
void radeon_hd_8370d_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
```

### radeon_hd_8370d_blt

kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) uzywajac ring buffer CP sprzetowego.

```
void radeon_hd_8370d_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
```

### radeon_hd_8370d_cursor_set

ustawia kursor sprhetowy na pozycji (x, y) z podanym obrazem 64x64 (obsuguje przezroczystosc). obraz jest kopiowany do pamieci ramki graficznej.

```
void radeon_hd_8370d_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
```

### radeon_hd_8370d_cursor_hide

ukrywa kursor sprhetowy.

```
void radeon_hd_8370d_cursor_hide(void)
```

### radeon_hd_8370d_get_fb

zwraca wskaznik na ramke graficzna (framebuffer).

```
void *radeon_hd_8370d_get_fb(void)
```

### radeon_hd_8370d_get_width / radeon_hd_8370d_get_height

zwracaja szerokosc i wysokosc ekranu (1920x1080).

```
uint32_t radeon_hd_8370d_get_width(void);
uint32_t radeon_hd_8370d_get_height(void);
```

### radeon_hd_8370d_get_device_id / radeon_hd_8370d_get_revision

zwracaja id urzadzenia i revisje karty graficznej.

```
uint16_t radeon_hd_8370d_get_device_id(void);
uint8_t radeon_hd_8370d_get_revision(void);
```

### radeon_hd_8370d_info_get

pobiera pelne informacje o urzadzeniu do struktury radeon_hd_8370d_info (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized, gcn_generation).

```
void radeon_hd_8370d_info_get(struct radeon_hd_8370d_info *out)
```

## akceleracja 3D GCN - drivers/karty_graficzne/radeon_hd_8370d.cpp

### tworzenie i niszczenie zasobow 3D

#### radeon_hd_8370d_vertex_buffer_create

tworzy vertex buffer (wierzcholki). zwraca handle lub RADEON_HANDLE_NULL przy bledzie.

```
radeon_handle_t radeon_hd_8370d_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
```

#### radeon_hd_8370d_vertex_buffer_destroy / radeon_hd_8370d_vertex_buffer_update

niszczy vertex buffer lub aktualizuje jego dane od podanego offsetu.

```
void radeon_hd_8370d_vertex_buffer_destroy(radeon_handle_t handle);
void radeon_hd_8370d_vertex_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### radeon_hd_8370d_index_buffer_create

tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe). zwraca handle.

```
radeon_handle_t radeon_hd_8370d_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
```

#### radeon_hd_8370d_index_buffer_destroy / radeon_hd_8370d_index_buffer_update

niszczy index buffer lub aktualizuje jego dane.

```
void radeon_hd_8370d_index_buffer_destroy(radeon_handle_t handle);
void radeon_hd_8370d_index_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### radeon_hd_8370d_texture_create

tworzy teksture 2D o podanym rozmiarze, formacie i liczbie poziom mipmap. zwraca handle.

```
radeon_handle_t radeon_hd_8370d_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
```

#### radeon_hd_8370d_texture_destroy / radeon_hd_8370d_texture_update / radeon_hd_8370d_texture_generate_mipmaps

niszczy teksture, aktualizuje jej zawartosc (upload z pamieci systemowej do FB) lub generuje mipmapy.

```
void radeon_hd_8370d_texture_destroy(radeon_handle_t handle);
void radeon_hd_8370d_texture_update(radeon_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
void radeon_hd_8370d_texture_generate_mipmaps(radeon_handle_t handle)
```

#### radeon_hd_8370d_render_target_create / radeon_hd_8370d_render_target_destroy

tworzy lub niszczy render target (bufor koloru do renderowania 3D z obsluga przezroczystosci alpha).

```
radeon_handle_t radeon_hd_8370d_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend);
void radeon_hd_8370d_render_target_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_depth_stencil_create / radeon_hd_8370d_depth_stencil_destroy

tworzy lub niszczy depth-stencil buffer (bufor glebi i stencil).

```
radeon_handle_t radeon_hd_8370d_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format);
void radeon_hd_8370d_depth_stencil_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_shader_create / radeon_hd_8370d_shader_destroy

tworzy shader (vertex/pixel/geometry/compute) z kodu bajtowego ISA GPU lub niszczy shader.

```
radeon_handle_t radeon_hd_8370d_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void radeon_hd_8370d_shader_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_sampler_state_create / radeon_hd_8370d_sampler_state_destroy

tworzy lub niszczy stan probki tekstury (filtrowanie min/mag/mip, adresowanie, anisotropia, porownanie).

```
radeon_handle_t radeon_hd_8370d_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable);
void radeon_hd_8370d_sampler_state_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_blend_state_create / radeon_hd_8370d_blend_state_destroy

tworzy lub niszczy stan blendowania (przezroczystosc alpha, operacje blendowania, staly kolor blendowania).

```
radeon_handle_t radeon_hd_8370d_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor);
void radeon_hd_8370d_blend_state_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_rasterizer_state_create / radeon_hd_8370d_rasterizer_state_destroy

tworzy lub niszczy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygladzone, alpha to coverage dla przezroczystosci).

```
radeon_handle_t radeon_hd_8370d_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage);
void radeon_hd_8370d_rasterizer_state_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_depth_stencil_state_create / radeon_hd_8370d_depth_stencil_state_destroy

tworzy lub niszczy stan depth-stencil (testy glebi i stencil z przodem i tylem).

```
radeon_handle_t radeon_hd_8370d_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void radeon_hd_8370d_depth_stencil_state_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_input_layout_create / radeon_hd_8370d_input_layout_destroy

tworzy lub niszczy input layout (mapowanie atrybutow vertex do slotow GCN).

```
radeon_handle_t radeon_hd_8370d_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void radeon_hd_8370d_input_layout_destroy(radeon_handle_t handle)
```

#### radeon_hd_8370d_constant_buffer_create / radeon_hd_8370d_constant_buffer_destroy / radeon_hd_8370d_constant_buffer_update

tworzy, niszczy lub aktualizuje constant buffer (stale shadera / uniforms GCN).

```
radeon_handle_t radeon_hd_8370d_constant_buffer_create(uint32_t size);
void radeon_hd_8370d_constant_buffer_destroy(radeon_handle_t handle);
void radeon_hd_8370d_constant_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

### bindowanie stanow potoku 3D GCN

#### radeon_hd_8370d_bind_vertex_buffer / radeon_hd_8370d_bind_index_buffer / radeon_hd_8370d_bind_input_layout

binduje vertex buffer, index buffer lub input layout do potoku 3D GCN.

```
void radeon_hd_8370d_bind_vertex_buffer(radeon_handle_t handle);
void radeon_hd_8370d_bind_index_buffer(radeon_handle_t handle);
void radeon_hd_8370d_bind_input_layout(radeon_handle_t handle)
```

#### radeon_hd_8370d_bind_vertex_shader / radeon_hd_8370d_bind_pixel_shader / radeon_hd_8370d_bind_geometry_shader / radeon_hd_8370d_bind_compute_shader

binduje shader vertex, pixel, geometry lub compute do potoku 3D GCN.

```
void radeon_hd_8370d_bind_vertex_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_pixel_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_geometry_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_compute_shader(radeon_handle_t handle)
```

#### radeon_hd_8370d_bind_render_target / radeon_hd_8370d_bind_depth_stencil

binduje render target lub depth-stencil do potoku 3D GCN.

```
void radeon_hd_8370d_bind_render_target(radeon_handle_t handle);
void radeon_hd_8370d_bind_depth_stencil(radeon_handle_t handle)
```

#### radeon_hd_8370d_bind_blend_state / radeon_hd_8370d_bind_rasterizer_state / radeon_hd_8370d_bind_depth_stencil_state

binduje stan blendowania, rasterizera lub depth-stencil do potoku 3D GCN.

```
void radeon_hd_8370d_bind_blend_state(radeon_handle_t handle);
void radeon_hd_8370d_bind_rasterizer_state(radeon_handle_t handle);
void radeon_hd_8370d_bind_depth_stencil_state(radeon_handle_t handle)
```

#### radeon_hd_8370d_bind_sampler_and_texture / radeon_hd_8370d_bind_constant_buffer

binduje probke tekstury i teksture do slotu lub constant buffer do slotu.

```
void radeon_hd_8370d_bind_sampler_and_texture(uint32_t slot, radeon_handle_t sampler, radeon_handle_t texture);
void radeon_hd_8370d_bind_constant_buffer(uint32_t slot, radeon_handle_t handle)
```

### konfiguracja potoku 3D GCN

#### radeon_hd_8370d_set_viewport

ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi.

```
void radeon_hd_8370d_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
```

#### radeon_hd_8370d_set_scissor

ustawia prostokat przyciecia renderowania (scissor).

```
void radeon_hd_8370d_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
```

#### radeon_hd_8370d_set_primitive_topology

ustawia typ primitivu do rysowania (punkt, linia, trojkat, kwadrat itp.).

```
void radeon_hd_8370d_set_primitive_topology(uint32_t topology)
```

#### radeon_hd_8370d_set_alpha_blend_global

wlacza lub wylacza globalne blendowanie alfa (przezroczystosc) dla calego potoku 3D.

```
void radeon_hd_8370d_set_alpha_blend_global(uint8_t enable)
```

### rysowanie 3D

#### radeon_hd_8370d_clear_render_target

czysci render target podanym kolorem.

```
void radeon_hd_8370d_clear_render_target(radeon_handle_t rt, uint32_t color)
```

#### radeon_hd_8370d_clear_depth_stencil

czysci depth-stencil podanymi wartosciami glebi i stencil.

```
void radeon_hd_8370d_clear_depth_stencil(radeon_handle_t ds, float depth, uint8_t stencil)
```

#### radeon_hd_8370d_draw

rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex.

```
void radeon_hd_8370d_draw(uint32_t vertex_count, uint32_t start_vertex)
```

#### radeon_hd_8370d_draw_indexed

rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex.

```
void radeon_hd_8370d_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
```

### command buffer (akceleracja 3D w strumieniu GCN)

#### radeon_hd_8370d_command_begin / radeon_hd_8370d_command_end

rozpoczyna nagrywanie polecen do command buffer lub zakoncza nagrywanie i wykonuje wszystkie polecenia przez Command Processor.

```
void radeon_hd_8370d_command_begin(void);
void radeon_hd_8370d_command_end(void)
```

### presentation (swap chain)

#### radeon_hd_8370d_swap_chain_create / radeon_hd_8370d_swap_chain_destroy / radeon_hd_8370d_present

tworzy swap chain (podwojne buforowanie z opcja vsync i premultiplied alpha dla przezroczystosci), niszczy swap chain lub przebija tylny bufor na przedni (present).

```
radeon_handle_t radeon_hd_8370d_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied);
void radeon_hd_8370d_swap_chain_destroy(radeon_handle_t handle);
void radeon_hd_8370d_present(radeon_handle_t chain)
```

### informacje o akceleracji 3D GCN

#### radeon_hd_8370d_context_get / radeon_hd_8370d_3d_available

pobiera biezacy kontekst 3D GCN lub zwraca 1 gdy akceleracja 3D jest dostepna.

```
void radeon_hd_8370d_context_get(struct radeon_hd_8370d_context *out);
int radeon_hd_8370d_3d_available(void)
```

## karta graficzna - drivers/karty_graficzne/radeon_hd_6790.cpp

sterownik karty graficznej AMD Radeon HD 6790 (Barts, Northern Islands, VLIW4) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP) i DMA, przezroczystosc (alpha blending, alpha to coverage), tekstury z mipmapami i kompresja DXT, shadery (vertex/pixel/geometry/compute/hull/domain), render targety, depth-stencil z depth bounds, tessellation, stream output, zapytania okluzyjne, statystyki potoku, MSAA, command buffer, presentation oraz presentation. sterownik opiera sie na architekturze Northern Islands (Barts Pro, device id 0x683F).

### radeon_6790_init

inicjalizuje sterownik Radeon HD 6790: wyszukuje urzadzenie na PCI (vendor 0x1002, urzadzenia 0x683F/0x6840/0x683E/0x6841), mapuje adresy MMIO, czysci przerwania, inicjalizuje zasilanie i zegary, konfiguruje silnik wyswietlania (DCN CRTC, plane, surface), resetuje i inicjalizuje ring buffer Command Processor oraz ring buffer DMA, konfiguruje pelny potok 3D (color buffer, depth buffer, blend, rasterizer, shader SQ, tekstury, samplery, tessellation, stream output) oraz inicjalizuje kursor sprzetowy.

```
int radeon_6790_init(void)
```

zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### radeon_6790_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy.

```
int radeon_6790_is_ready(void)
```

### radeon_6790_shutdown

zatrzymuje Command Processor i DMA, przechodzi w stan zasilania D3 i oznacz sterownik jako niezalogowany.

```
void radeon_6790_shutdown(void)
```

### radeon_6790_putpixel

rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem.

```
void radeon_6790_putpixel(uint32_t x, uint32_t y, uint32_t color)
```

### radeon_6790_fillrect

wypelnia prostokat o podanym rozmiarze i kolorze. uzywa ring buffer CP gdy sterownik jest zainicjowany, w przeciwnym razie petla CPU.

```
void radeon_6790_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
```

### radeon_6790_blt

kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) uzywajac ring buffer CP sprzetowego.

```
void radeon_6790_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
```

### radeon_6790_cursor_set

ustawia kursor sprzetowy na pozycji (x, y) z podanym obrazem 64x64. obraz jest kopiowany do pamieci ramki graficznej.

```
void radeon_6790_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
```

### radeon_6790_cursor_hide

ukrywa kursor sprzetowy.

```
void radeon_6790_cursor_hide(void)
```

### radeon_6790_get_fb

zwraca wskaznik na ramke graficzna (framebuffer).

```
void *radeon_6790_get_fb(void)
```

### radeon_6790_get_width / radeon_6790_get_height

zwracaja szerokosc i wysokosc ekranu (1920x1080).

```
uint32_t radeon_6790_get_width(void);
uint32_t radeon_6790_get_height(void);
```

### radeon_6790_get_device_id / radeon_6790_get_revision

zwracaja id urzadzenia i revisje karty graficznej.

```
uint16_t radeon_6790_get_device_id(void);
uint8_t radeon_6790_get_revision(void);
```

### radeon_6790_info_get

pobiera pelne informacje o urzadzeniu do struktury radeon_6790_info (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized, architecture, vram_size, vram_used, gfx_clock_mhz, memory_clock_mhz, temperature_c, fan_rpm).

```
void radeon_6790_info_get(struct radeon_6790_info *out)
```

### zarzadzanie pamieci wideo (VRAM)

#### radeon_6790_vram_alloc

alokuje blok pamieci wideo o podanym rozmiarze i typie. zwraca offset w VRAM lub 0xFFFFFFFFFFFFFFFF przy bledzie.

```
uint64_t radeon_6790_vram_alloc(uint64_t size, uint32_t type)
```

#### radeon_6790_vram_free

zwalnia blok pamieci wideo o podanym offsetcie.

```
void radeon_6790_vram_free(uint64_t handle)
```

#### radeon_6790_vram_used / radeon_6790_vram_total

zwracaja uzywana i calkowita ilosc pamieci wideo.

```
uint64_t radeon_6790_vram_used(void);
uint64_t radeon_6790_vram_total(void);
```

## akceleracja 3D - drivers/karty_graficzne/radeon_6790.cpp

### tworzenie i niszczenie zasobow 3D

#### radeon_6790_vertex_buffer_create

tworzy vertex buffer (wierzcholki). zwraca handle lub RADEON_6790_HANDLE_NULL przy bledzie.

```
radeon_6790_handle_t radeon_6790_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
```

#### radeon_6790_vertex_buffer_destroy / radeon_6790_vertex_buffer_update

niszczy vertex buffer lub aktualizuje jego dane od podanego offsetu.

```
void radeon_6790_vertex_buffer_destroy(radeon_6790_handle_t handle);
void radeon_6790_vertex_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### radeon_6790_index_buffer_create

tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe). zwraca handle.

```
radeon_6790_handle_t radeon_6790_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
```

#### radeon_6790_index_buffer_destroy / radeon_6790_index_buffer_update

niszczy index buffer lub aktualizuje jego dane.

```
void radeon_6790_index_buffer_destroy(radeon_6790_handle_t handle);
void radeon_6790_index_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### radeon_6790_texture_create

tworzy teksture 2D o podanym rozmiarze, formacie i liczbie poziom mipmap. obsluguje kompresje DXT i generowanie mipmap. zwraca handle.

```
radeon_6790_handle_t radeon_6790_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
```

#### radeon_6790_texture_destroy / radeon_6790_texture_update / radeon_6790_texture_generate_mipmaps

niszczy teksture, aktualizuje jej zawartosc (upload z pamieci systemowej do FB) lub generuje mipmapy.

```
void radeon_6790_texture_destroy(radeon_6790_handle_t handle);
void radeon_6790_texture_update(radeon_6790_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
void radeon_6790_texture_generate_mipmaps(radeon_6790_handle_t handle)
```

#### radeon_6790_render_target_create / radeon_6790_render_target_destroy

tworzy lub niszczy render target (bufor koloru do renderowania 3D z obsluga przezroczystosci alpha i wielowarstwowosci).

```
radeon_6790_handle_t radeon_6790_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend);
void radeon_6790_render_target_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_depth_stencil_create / radeon_6790_depth_stencil_destroy

tworzy lub niszczy depth-stencil buffer (bufor glebi i stencil z obsluga kompresji i depth bounds).

```
radeon_6790_handle_t radeon_6790_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression);
void radeon_6790_depth_stencil_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_shader_create / radeon_6790_shader_destroy / radeon_6790_shader_compile

tworzy shader (vertex/pixel/geometry/compute/hull/domain) z kodu bajtowego ISA GPU, niszczy shader lub kompiluje shader.

```
radeon_6790_handle_t radeon_6790_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void radeon_6790_shader_destroy(radeon_6790_handle_t handle);
void radeon_6790_shader_compile(radeon_6790_handle_t handle)
```

#### radeon_6790_sampler_state_create / radeon_6790_sampler_state_destroy

tworzy lub niszczy stan probki tekstury (filtrowanie min/mag/mip, anisotropia, adresowanie U/V/W, porownanie tekstury, redukcja do zera).

```
radeon_6790_handle_t radeon_6790_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t max_anisotropy, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint8_t compare_enable, uint8_t compare_func, uint8_t reduction_zero);
void radeon_6790_sampler_state_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_blend_state_create / radeon_6790_blend_state_destroy

tworzy lub niszczy stan blendowania (przezroczystosc alpha, operacje blendowania koloru i alfa, staly kolor blendowania, alpha to coverage, dual source blend).

```
radeon_6790_handle_t radeon_6790_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor, uint8_t alpha_to_coverage, uint8_t dual_source_blend);
void radeon_6790_blend_state_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_rasterizer_state_create / radeon_6790_rasterizer_state_destroy

tworzy lub niszczy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygladzone, alpha to coverage, depth bias, rasteryzacja zachowawcza).

```
radeon_6790_handle_t radeon_6790_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage, uint8_t depth_bias_enable, float depth_bias_constant, float depth_bias_slope, uint8_t conservative_raster);
void radeon_6790_rasterizer_state_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_depth_stencil_state_create / radeon_6790_depth_stencil_state_destroy

tworzy lub niszczy stan depth-stencil (testy glebi i stencil z przodem i tylem, depth bounds test).

```
radeon_6790_handle_t radeon_6790_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func, uint8_t depth_bounds_enable);
void radeon_6790_depth_stencil_state_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_input_layout_create / radeon_6790_input_layout_destroy

tworzy lub niszczy input layout (mapowanie atrybutow vertex do slotow).

```
radeon_6790_handle_t radeon_6790_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void radeon_6790_input_layout_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_constant_buffer_create / radeon_6790_constant_buffer_destroy / radeon_6790_constant_buffer_update

tworzy, niszczy lub aktualizuje constant buffer (stale shadera / uniforms).

```
radeon_6790_handle_t radeon_6790_constant_buffer_create(uint32_t size);
void radeon_6790_constant_buffer_destroy(radeon_6790_handle_t handle);
void radeon_6790_constant_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data)
```

#### radeon_6790_stream_output_create / radeon_6790_stream_output_destroy / radeon_6790_stream_output_update

tworzy, niszczy lub aktualizuje buffer wyjscia strumieniowego (transform feedback) dla potoku geometrycznego.

```
radeon_6790_handle_t radeon_6790_stream_output_create(uint32_t buffer_count, const uint64_t *sizes);
void radeon_6790_stream_output_destroy(radeon_6790_handle_t handle);
void radeon_6790_stream_output_update(radeon_6790_handle_t handle, uint32_t slot, const void *data, uint32_t size)
```

#### radeon_6790_tessellation_state_create / radeon_6790_tessellation_state_destroy / radeon_6790_tessellation_state_update

tworzy, niszczy lub aktualizuje stan tessellation (liczba wierzcholkow latek, czynniki tessellation krawedzi i srodka, typ domeny, partycjonowanie).

```
radeon_6790_handle_t radeon_6790_tessellation_state_create(uint32_t patch_vertices, const float *edge_factors, const float *inside_factors, uint8_t domain_triangles, uint8_t fractional_odd);
void radeon_6790_tessellation_state_destroy(radeon_6790_handle_t handle);
void radeon_6790_tessellation_state_update(radeon_6790_handle_t handle, const float *edge_factors, const float *inside_factors)
```

### bindowanie stanow potoku 3D

#### radeon_6790_bind_vertex_buffer / radeon_6790_bind_index_buffer / radeon_6790_bind_input_layout

binduje vertex buffer, index buffer lub input layout do potoku 3D.

```
void radeon_6790_bind_vertex_buffer(radeon_6790_handle_t handle);
void radeon_6790_bind_index_buffer(radeon_6790_handle_t handle);
void radeon_6790_bind_input_layout(radeon_6790_handle_t handle)
```

#### radeon_6790_bind_vertex_shader / radeon_6790_bind_pixel_shader / radeon_6790_bind_geometry_shader / radeon_6790_bind_compute_shader / radeon_6790_bind_hull_shader / radeon_6790_bind_domain_shader

binduje shader vertex, pixel, geometry, compute, hull lub domain do potoku 3D.

```
void radeon_6790_bind_vertex_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_pixel_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_geometry_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_compute_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_hull_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_domain_shader(radeon_6790_handle_t handle)
```

#### radeon_6790_bind_render_target / radeon_6790_bind_depth_stencil

binduje render target lub depth-stencil do potoku 3D.

```
void radeon_6790_bind_render_target(radeon_6790_handle_t handle);
void radeon_6790_bind_depth_stencil(radeon_6790_handle_t handle)
```

#### radeon_6790_bind_blend_state / radeon_6790_bind_rasterizer_state / radeon_6790_bind_depth_stencil_state

binduje stan blendowania, rasterizera lub depth-stencil do potoku 3D.

```
void radeon_6790_bind_blend_state(radeon_6790_handle_t handle);
void radeon_6790_bind_rasterizer_state(radeon_6790_handle_t handle);
void radeon_6790_bind_depth_stencil_state(radeon_6790_handle_t handle)
```

#### radeon_6790_bind_sampler_and_texture / radeon_6790_bind_constant_buffer / radeon_6790_bind_stream_output / radeon_6790_bind_tessellation

binduje probke tekstury i teksture do slotu, constant buffer do slotu, buffer wyjscia strumieniowego do slotu lub stan tessellation do potoku.

```
void radeon_6790_bind_sampler_and_texture(uint32_t slot, radeon_6790_handle_t sampler, radeon_6790_handle_t texture);
void radeon_6790_bind_constant_buffer(uint32_t slot, radeon_6790_handle_t handle);
void radeon_6790_bind_stream_output(uint32_t slot, radeon_6790_handle_t handle);
void radeon_6790_bind_tessellation(radeon_6790_handle_t handle)
```

### konfiguracja potoku 3D

#### radeon_6790_set_viewport

ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi.

```
void radeon_6790_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
```

#### radeon_6790_set_scissor

ustawia prostokat przyciecia renderowania (scissor).

```
void radeon_6790_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
```

#### radeon_6790_set_primitive_topology

ustawia typ primitivu do rysowania (punkt, linia, trojkat, kwadrat itp.).

```
void radeon_6790_set_primitive_topology(uint32_t topology)
```

#### radeon_6790_set_alpha_blend_global

wlacza lub wylacza globalne blendowanie alfa (przezroczystosc) dla calego potoku 3D.

```
void radeon_6790_set_alpha_blend_global(uint8_t enable)
```

#### radeon_6790_set_depth_bounds

ustawia dolne i gorne granice testu depth bounds (odrzucanie pikseli poza zakresem glebi).

```
void radeon_6790_set_depth_bounds(float min_depth, float max_depth)
```

#### radeon_6790_set_render_target_array_slice

ustawia aktywna warstwe tablicy render targetow (rendered array / layered rendering).

```
void radeon_6790_set_render_target_array_slice(uint32_t slice)
```

#### radeon_6790_set_msaa_samples

ustawia liczba probek MSAA (wieloprobekowe wygladzanie krawedzi).

```
void radeon_6790_set_msaa_samples(uint32_t samples)
```

### rysowanie 3D

#### radeon_6790_clear_render_target

czysci render target podanym kolorem.

```
void radeon_6790_clear_render_target(radeon_6790_handle_t rt, uint32_t color)
```

#### radeon_6790_clear_depth_stencil

czysci depth-stencil podanymi wartosciami glebi i stencil.

```
void radeon_6790_clear_depth_stencil(radeon_6790_handle_t ds, float depth, uint8_t stencil)
```

#### radeon_6790_draw

rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex. obsluguje tessellation, render target array i MSAA.

```
void radeon_6790_draw(uint32_t vertex_count, uint32_t start_vertex)
```

#### radeon_6790_draw_indexed

rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex.

```
void radeon_6790_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
```

#### radeon_6790_draw_indexed_instanced

rysuje indeksowane primitivy z instancjami - index_count indeksow, instance_count instancji, start_instance jako pierwsza instancja.

```
void radeon_6790_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)
```

#### radeon_6790_draw_indirect

rysuje primitivy na podstawie danych rysunkowych z bufora po podanym adresie - draw_count rysunkow o podanym kroku (stride).

```
void radeon_6790_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)
```

### compute shaders

#### radeon_6790_compute_dispatch

wysyla zlecenie obliczeniowe (compute shader) na podana liczbe grup watkow (group_x, group_y, group_z).

```
void radeon_6790_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
```

#### radeon_6790_compute_dispatch_indirect

wysyla zlecenie obliczeniowe na podstawie danych z bufora po podanym adresie.

```
void radeon_6790_compute_dispatch_indirect(uint32_t buffer_addr)
```

#### radeon_6790_dma_transfer

przenosi blok danych o podanym rozmiarze z adresu src na adres dst uzywajac asynchronicznego silnika DMA (Command Processor oddzielny od grafiki).

```
void radeon_6790_dma_transfer(uint64_t dst, uint64_t src, uint64_t size)
```

### zapytania okluzyjne

#### radeon_6790_occlusion_query_create / radeon_6790_occlusion_query_destroy

tworzy lub niszczy zapytanie okluzyjne (test widzialnosci obiektu poprzez liczenie probek przechodzacych test glebi).

```
radeon_6790_handle_t radeon_6790_occlusion_query_create(void);
void radeon_6790_occlusion_query_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_occlusion_query_begin / radeon_6790_occlusion_query_end

rozpoczyna lub konczy zbieranie wyniku zapytania okluzyjnego.

```
void radeon_6790_occlusion_query_begin(radeon_6790_handle_t handle);
void radeon_6790_occlusion_query_end(radeon_6790_handle_t handle)
```

#### radeon_6790_occlusion_query_get

pobiera liczbe probek widzialnych dla zapytania okluzyjnego. zwraca 1 gdy wynik jest gotowy.

```
int radeon_6790_occlusion_query_get(radeon_6790_handle_t handle, uint32_t *samples)
```

### statystyki potoku 3D

#### radeon_6790_pipeline_stats_create / radeon_6790_pipeline_stats_destroy

tworzy lub niszczy liczniki statystyk potoku (wierzcholki, primitivy, wywolania shaderow).

```
radeon_6790_handle_t radeon_6790_pipeline_stats_create(void);
void radeon_6790_pipeline_stats_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_pipeline_stats_begin / radeon_6790_pipeline_stats_end

rozpoczyna lub konczy zbieranie statystyk potoku 3D.

```
void radeon_6790_pipeline_stats_begin(radeon_6790_handle_t handle);
void radeon_6790_pipeline_stats_end(radeon_6790_handle_t handle)
```

#### radeon_6790_pipeline_stats_get

pobiera zebrane statystyki potoku do struktury radeon_6790_pipeline_stats. zwraca 1 gdy dane sa dostepne.

```
int radeon_6790_pipeline_stats_get(radeon_6790_handle_t handle, struct radeon_6790_pipeline_stats *out)
```

### synchronizacja

#### radeon_6790_fence_create / radeon_6790_fence_signal / radeon_6790_fence_wait / radeon_6790_fence_destroy

tworzy, sygnalizuje, oczekuje na sygnal lub niszczy fence (punkt synchronizacji potoku GPU).

```
void radeon_6790_fence_create(radeon_6790_handle_t *out);
void radeon_6790_fence_signal(radeon_6790_handle_t handle, uint32_t value);
int radeon_6790_fence_wait(radeon_6790_handle_t handle, uint32_t value, uint32_t timeout_us);
void radeon_6790_fence_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_semaphore_create / radeon_6790_semaphore_signal / radeon_6790_semaphore_wait / radeon_6790_semaphore_destroy

tworzy, sygnalizuje, oczekuje na sygnal lub niszczy semafor (synchronizacja miedzy potokami graficznym i obliczeniowym).

```
void radeon_6790_semaphore_create(radeon_6790_handle_t *out);
void radeon_6790_semaphore_signal(radeon_6790_handle_t handle, uint32_t value);
void radeon_6790_semaphore_wait(radeon_6790_handle_t handle, uint32_t value);
void radeon_6790_semaphore_destroy(radeon_6790_handle_t handle)
```

#### radeon_6790_wait_idle / radeon_6790_timestamp

czeka az potok GPU stanie sie bezczynny lub pobiera znacznik czasu GPU.

```
int radeon_6790_wait_idle(uint32_t timeout_us);
uint64_t radeon_6790_timestamp(void)
```

### zasilanie, zegary i monitoring

#### radeon_6790_power_set_state / radeon_6790_power_get_state

ustawia lub zwraca stan zasilania GPU (D0-D3).

```
void radeon_6790_power_set_state(uint8_t state);
uint8_t radeon_6790_power_get_state(void);
```

#### radeon_6790_set_clock_gfx / radeon_6790_set_clock_memory / radeon_6790_get_clock_gfx / radeon_6790_get_clock_memory

ustawia lub zwraca czestotliwosc zegara grafiki i pamieci w MHz.

```
void radeon_6790_set_clock_gfx(uint32_t mhz);
void radeon_6790_set_clock_memory(uint32_t mhz);
uint32_t radeon_6790_get_clock_gfx(void);
uint32_t radeon_6790_get_clock_memory(void);
```

#### radeon_6790_get_temperature

odczytuje aktualna temperature GPU w stopniach Celsjusza.

```
uint32_t radeon_6790_get_temperature(void);
```

#### radeon_6790_set_fan_speed / radeon_6790_get_fan_speed

ustawia lub zwraca predkosc wentylatora w obrotach na minute.

```
void radeon_6790_set_fan_speed(uint32_t rpm);
uint32_t radeon_6790_get_fan_speed(void);
```

#### radeon_6790_rgb_set / radeon_6790_rgb_off

ustawia podswietlenie RGB karty graficznej (kolor i tryb) lub wylacza podswietlenie.

```
void radeon_6790_rgb_set(uint32_t color, uint8_t mode);
void radeon_6790_rgb_off(void);
```

### presentation (swap chain)

#### radeon_6790_swap_chain_create / radeon_6790_swap_chain_destroy / radeon_6790_present

tworzy swap chain (podwojne buforowanie z opcja vsync i premultiplied alpha dla przezroczystosci), niszczy swap chain lub przebija tylny bufor na przedni (present).

```
radeon_6790_handle_t radeon_6790_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied);
void radeon_6790_swap_chain_destroy(radeon_6790_handle_t handle);
void radeon_6790_present(radeon_6790_handle_t chain)
```

### command buffer (akceleracja 3D w strumieniu)

#### radeon_6790_command_begin / radeon_6790_command_end

rozpoczyna nagrywanie polecen do command buffer lub zakoncza nagrywanie i wykonuje wszystkie polecenia przez Command Processor.

```
void radeon_6790_command_begin(void);
void radeon_6790_command_end(void)
```

### informacje o akceleracji 3D

#### radeon_6790_context_get / radeon_6790_3d_available

pobiera biezacy kontekst 3D lub zwraca 1 gdy akceleracja 3D jest dostepna (potok Northern Islands / Barts).

```
void radeon_6790_context_get(struct radeon_6790_context *out);
int radeon_6790_3d_available(void)
```

## menedzer sterownikow - kernel/driver_manager.cpp

menedzer sterownikow odpowiedzialny za rejestracje, wykrywanie sprzetu, tworzenie procesow sterownikow oraz ich cykl zycia. kazdy sterownik jest rejestrowany za pomoca `driver_manager_register()` i tworzony jako osobny proces ring-0 przy wykryciu wspieranego sprzetu. sterowniki sa harmonogramowane przez `proces_tick()` razem z procesami uzytkownika i mozna je zakonczyc przez menedzer zadan (`zadania.c`).

rejestracja sterownika wymaga podania nazwy, funkcji inicjalizacji, funkcji zamykania (moze byc 0), funkcji poll (moze byc 0) oraz interwalu pollowania w tikach. funkcja inicjalizacji jest wywolywana raz w kontekscie jadra przed utworzeniem procesu. funkcja jest wywolywana w kontekscie procesu sterownika (ring 0, page directory jadra, wlasny stos kernela). jezeli sterownik nie ma funkcji poll, uzywana jest domyslna funkcja bezczynna (`proces_yield`).

```
void driver_manager_register(const char *name, driver_init_fn init, driver_shutdown_fn shutdown, driver_poll_fn poll, uint32_t poll_interval)
```

inicjalizuje menedzer sterownikow: iteruje po zarejestrowanych sterownikach, wywoluje ich funkcje inicjalizacji i tworzy proces ring-0 dla kazdego sterownik ktorego sprzet zostal wykryty. zwraca liczbe uruchomionych sterownikow.

```
int driver_manager_init_all(void)
```

wywolywane w kazdym tiku timera z glownej petli jadra. sprawdza stan procesow sterownikow - jezeli proces zostal zakonczony (np. przez menedzer zadan), wywoluje funkcje zamykania sterownika i oznacza go jako nieaktywny. wywoluje funkcje poll aktywnych sterownikow zgodnie z ich interwalem.

```
void driver_manager_poll_all(void)
```

wywoluje funkcje zamykania sterownika powiazanego z podanym PID. uzywane przez menedzer zadan gdy uzytkownik kliknie ZABIJ przy procesie sterownika.

```
void driver_manager_shutdown_by_pid(uint32_t pid)
```

zwraca 1 jezeli podany PID nalezy do aktywnego sterownika, 0 w przeciwnym razie.

```
int driver_manager_is_driver(uint32_t pid)
```

zwraca wskaznik na deskryptor sterownika dla podanego PID lub 0 jezeli nie znaleziono.

```
struct driver_desc *driver_manager_find_by_pid(uint32_t pid)
```

wywoluje funkcje zamykania wszystkich aktywnych sterownikow. uzywane przy zamykaniu systemu.

```
void driver_manager_shutdown_all(void)
```

## struktura plikow

```
tarixenOS/
├── boot.asm              # bootloader i wejscie w tryb chroniony
├── kernel/
│   ├── kernel.c          # glowna petla jadra, kernel_main
│   ├── proces.cpp          # zaawansowany harmonogram procesow (scheduler, C++)
│   ├── driver_manager.cpp  # menedzer sterownikow jako procesy ring-0
│   ├── paging.c          # stronicowanie pamieci
│   ├── syscall.c         # wywolania systemowe
│   ├── timer.c           # timer systemowy
│   ├── idt.c             # tablica przerwan
│   ├── gdt.c             # tablica deskryptorow
│   ├── panika.c          # ekran paniki
│   ├── zegar.c           # zegar systemowy
│   ├── loader.c          # ladownik ELF
│   ├── terminal.c        # terminal
│   ├── notatnik.c        # aplikacja notatnik
│   ├── kalkulator.c      # aplikacja kalkulator
│   ├── browser.c         # przegladarka
│   ├── konfiguracja.c    # ekran powitalny
│   └── *.h               # naglowki
├── graficzny/
│   ├── rysowanie.c       # rysowanie pikseli, prostokatow, tekstu
│   ├── okno.c            # zarzadzanie oknami
│   ├── czcionka.c        # czcionka Chicago
│   ├── menu_start.c      # menu start
│   ├── pulpit.c          # pulpit i ikony
│   └── *.h               # naglowki
├── drivers/
│   ├── keyboard.c        # klawiatura
│   ├── mouse.c           # mysz
│   ├── floppy.c          # dyskietka
│   ├── net.c             # siec
│   ├── delay.c           # opoznienie
│   ├── internet/         # sterowniki sieciowe (internet)
│   │   └── rtl8168b.cpp  # Realtek RTL8168B / RTL8111B (PCIe Gigabit Ethernet, C++)
│   ├── bluetooth/        # sterownik bluetooth (MSI MPG Z890 CARBON WIFI)
│   │   ├── bluetooth.h   # naglowek sterownika bluetooth (C API)
│   │   ├── hci.cpp       # warstwa hci (host controller interface) po usb
│   │   ├── l2cap.cpp     # warstwa l2cap (logical link control)
│   │   ├── a2dp.cpp      # warstwa a2dp (sluchawki bluetooth)
│   │   └── ui.cpp        # interfejs uzytkownika (pasek zadan, okno parowania)
│   ├── karty_graficzne/  # karty graficzne
│   │   ├── gma_x4500.cpp  # Intel GMA X4500HD (PCIe, Gen4 GPU, C++)
│   │   ├── gma_x4500.h    # naglowek Intel GMA X4500HD
│   │   ├── radeon_hd_8370d.cpp  # AMD Radeon HD 8370D (PCIe, GCN 3.0, C++)
│   │   └── radeon_hd_8370d.h  # naglowek AMD Radeon HD 8370D
│   ├── usb/              # USB
│   └── sound/            # dzwiek
│       ├── intel_hd.c    # Intel HD Audio
│       ├── intel_hd.h    # naglowek Intel HD Audio
│       ├── alc650.c      # kodek ALC650
│       ├── alc650.h      # naglowek ALC650
│       ├── audigy2.cpp   # Creative Sound Blaster Audigy 2 (EMU10K2)
│       └── audigy2.h     # naglowek Audigy 2
├── system_plikow/        # system plikow FAT32
├── user_land/            # aplikacje uzytkownika
│   ├── konta.c           # konta uzytkownikow
│   ├── tapeta.c          # tapeta
│   └── okno_user.c       # okno uzytkownika
└── kompilator/cc.c       # kompilator C
```

## konwencja pisania kodu

- komentarze po polsku
- commity po polsku
- kod w C (C++ tylko z pozwolenia, nie w miejscach krytycznych)
- komentarze tylko tam gdzie kod jest mniej czytelny

## sterownik dzwieku Realtek ALC880 - drivers/karty_dzwiekowe/alc880.cpp

sterownik karty dzwiekowej Realtek ALC880 (HD Audio codec) napisany w C++. obsluguje pelna konfiguracje kodeka przez magistrale Intel HDA: wykrycie urzadzenia na PCI, inicjalizacje kontrolera HDA, odkrycie widgetow audio, konfiguracje pinow jack, mixer, strumienie PCM odtwarzania i nagrywania, wyjscie cyfrowe SPDIF, wykrywanie wlaczenia jack (jack detection), zarzadzanie zasilaniem (power management) oraz statystyki sterownika. klasa ALC880 jest singletonem dostepna przez `ALC880::instance()`.

### ALC880::probe

sprawdza czy urzadzenie to Realtek ALC880 (vendor 0x10EC, device 0x0880).

```
uint8_t probe(uint16_t vendor, uint16_t device);
```

zwraca 1 gdy urzadzenie zostalo rozpoznane, 0 gdy nie.

### ALC880::init

inicjalizuje sterownik ALC880: wyszukuje kontroler Intel HDA na PCI, mapuje adresy MMIO, resetuje i wlacza kontroler, wykrywa adres kodeka, odkrywa widgety audio, parsuje konfiguracje pinow, konfiguruje domyslne piny, inicjalizuje mixer, jack detection, SPDIF i zarzadzanie zasilaniem.

```
void init(void);
```

### ALC880::deinit

zatrzymuje odtwarzanie i nagrywanie, zwalnia bufory DMA i dezaktywuje sterownik.

```
void deinit(void);
```

### ALC880::irq_handler

obsluguje przerwanie sprzegtu z kontrolera HDA i wykrywa zmiany stanu jack.

```
void irq_handler(void);
```

### ALC880::pcm_open / pcm_close

otwiera strumien PCM do odtwarzania lub nagrywania z podanymi parametrami (liczba kanalow, czestotliwosc, format) i zamyka strumien.

```
int pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback);
void pcm_close(uint8_t playback);
```

`playback` = 1 dla odtwarzania, 0 dla nagrywania. zwraca indeks strumienia lub -1 przy bledzie.

### ALC880::pcm_write / pcm_read

zapisuje dane do bufora odtwarzania lub odczytuje dane z bufora nagrywania.

```
int pcm_write(const void* data, uint32_t bytes);
int pcm_read(void* data, uint32_t bytes);
```

zwraca liczbe skopiowanych bajtow lub -1 przy bledzie.

### ALC880::mixer_get_control_count / mixer_get_control

pobiera liczbe sterownikow mixer oraz informacje o sterowniku po indeksie.

```
int mixer_get_control_count(void);
int mixer_get_control(uint8_t index, struct alc880_mixer_control* ctrl);
```

### ALC880::mixer_set_volume / mixer_get_volume

ustawia lub pobiera wzmocnienie danego sterownika mixer.

```
int mixer_set_volume(uint8_t index, int32_t gain);
int mixer_get_volume(uint8_t index, int32_t* gain);
```

### ALC880::mixer_set_mute / mixer_get_mute

ustawia lub pobiera stan wyciszenia danego sterownika mixer.

```
int mixer_set_mute(uint8_t index, uint8_t mute);
int mixer_get_mute(uint8_t index, uint8_t* mute);
```

### ALC880::jack_get_count / jack_get_status / jack_get_config

pobiera liczbe wykrytych jack, sprawdza czy dany jack ma wlaczone wtyczke oraz pobiera konfiguracje pinu jack.

```
int jack_get_count(void);
int jack_get_status(uint8_t index, uint8_t* present);
int jack_get_config(uint8_t index, struct alc880_pin_config* config);
```

### ALC880::spdif_open / spdif_close / spdif_write

otwiera wyjscie cyfrowe SPDIF, zamyka je oraz zapisuje dane audio do strumienia SPDIF.

```
int spdif_open(uint32_t rate, uint16_t format);
int spdif_write(const void* data, uint32_t bytes);
void spdif_close(void);
```

### ALC880::spdif_set_category / spdif_set_copyright / spdif_set_nonaudio / spdif_set_preemphasis

konfiguruje parametry strumienia cyfrowego SPDIF (kategoria, copyright, non-audio, pre-emphasis).

```
int spdif_set_category(uint8_t category);
int spdif_set_copyright(uint8_t copyright);
int spdif_set_nonaudio(uint8_t nonaudio);
int spdif_set_preemphasis(uint8_t preemphasis);
```

### ALC880::set_master_volume / get_master_volume

ustawia lub pobiera glosnosc glowna w procentach (0-100).

```
void set_master_volume(uint8_t percent);
uint8_t get_master_volume(void);
```

### ALC880::power_set_state / power_get_state

ustawia lub pobiera stan zasilania kodeka (D0-D3).

```
void power_set_state(uint8_t state);
uint8_t power_get_state(void);
```

### ALC880::stats_get

pobiera statystyki sterownika (liczba wyslanych verbow, bledy, przerwania, przepelnienia bufora, zmiany jack).

```
void stats_get(struct alc880_stats* out);
```

struktura `alc880_stats` zawiera:

| pole | opis |
|------|------|
| verbs_sent | liczba wyslanych verbow HDA |
| verbs_failed | bledy wysylania verbow |
| interrupts | liczba przerwan |
| buffer_underruns | niedobory bufora |
| buffer_overruns | przepelnienia bufora |
| jack_changes | zmiany stanu jack |

struktura `alc880_mixer_control` zawiera:

| pole | opis |
|------|------|
| nid | id widgetu |
| index | indeks sterownika |
| direction | kierunek (0=input, 1=output) |
| min_gain | minimalne wzmocnienie |
| max_gain | maksymalne wzmocnienie |
| current_gain | aktualne wzmocnienie |
| mute | stan wyciszenia |
| name | nazwa sterownika |

struktura `alc880_pin_config` zawiera:

| pole | opis |
|------|------|
| nid | id pinu |
| sequence | sekwencja priorytetu |
| default_association | domyselne skojarzenie |
| misc | rozne flagi |
| color | kolor gniazda |
| connection_type | typ polaczenia |
| location | lokalizacja (zewn./wewn.) |
| port_connectivity | typ connectivitiy portu |
| default_device | domyslne urzadzenie |

widgety ALC880:

| typ | opis |
|------|------|
| output | konwerter cyfrowo-analogowy (DAC) |
| input | konwerter analogowo-cyfrowy (ADC) |
| mixer | mikser audio |
| selector | selektor wejsc |
| pin_complex | pin gniazda audio |
| power_widget | widget zasilania |
| volume_knob | pokretlo glosnosci |

parametry HDA:

| parametr | opis |
|----------|------|
| vendor_id | id dostawcy kodeka |
| revision_id | rewizja kodeka |
| audio_widget_cap | moznosci widgetu audio |
| pin_cap | moznosci pinu |
| amp_in_cap / amp_out_cap | moznosci wzmacniacza wejsc/wyjsc |
| connection_list_len | dlugosc listy polaczen |

widgety ALC880 (domyslne NID):

- DAC: 0x02, 0x03, 0x04, 0x05, 0x06
- ADC: 0x07, 0x08, 0x09
- Piny: 0x0C - 0x1B
- Mixery: 0x1C - 0x23
- Selektory: 0x24 - 0x27

funkcja C do obslugi przerwan:

```
extern "C" void alc880_irq(void);
```

## sterownik dzwieku Realtek ALC1150 - drivers/karty_dzwiekowe/alc1150.cpp

sterownik karty dzwiekowej Realtek ALC1150 (HD Audio codec) napisany w C++. obsluguje pelna konfiguracje kodeka przez magistrale Intel HDA: wykrycie urzadzenia na PCI, inicjalizacje kontrolera HDA, odkrycie widgetow audio, konfiguracje pinow jack, mixer, strumienie PCM odtwarzania i nagrywania, wyjscie cyfrowe SPDIF, wykrywanie wlaczenia jack (jack detection), zarzadzanie zasilaniem (power management), pause/resume strumieni oraz statystyki sterownika. klasa ALC1150 jest singletonem dostepna przez `ALC1150::instance()`.

### ALC1150::probe

sprawdza czy urzadzenie to Realtek ALC1150 (vendor 0x10EC, device 0x1150).

```
uint8_t probe(uint16_t vendor, uint16_t device);
```

zwraca 1 gdy urzadzenie zostalo rozpoznane, 0 gdy nie.

### ALC1150::init

inicjalizuje sterownik ALC1150: wyszukuje kontroler Intel HDA na PCI, mapuje adresy MMIO, resetuje i wlacza kontroler, wykrywa adres kodeka, odkrywa widgety audio, parsuje konfiguracje pinow, konfiguruje domyslne piny, inicjalizuje mixer, jack detection, SPDIF i zarzadzanie zasilaniem.

```
void init(void);
```

### ALC1150::deinit

zatrzymuje odtwarzanie i nagrywanie, zwalnia bufory DMA i dezaktywuje sterownik.

```
void deinit(void);
```

### ALC1150::irq_handler

obsluguje przerwanie sprzegtu z kontrolera HDA i wykrywa zmiany stanu jack.

```
void irq_handler(void);
```

### ALC1150::pcm_open / pcm_close

otwiera strumien PCM do odtwarzania lub nagrywania z podanymi parametrami (liczba kanalow, czestotliwosc, format) i zamyka strumien.

```
int pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback);
void pcm_close(uint8_t playback);
```

`playback` = 1 dla odtwarzania, 0 dla nagrywania. zwraca indeks strumienia lub -1 przy bledzie.

### ALC1150::pcm_write / pcm_read

zapisuje dane do bufora odtwarzania lub odczytuje dane z bufora nagrywania.

```
int pcm_write(const void* data, uint32_t bytes);
int pcm_read(void* data, uint32_t bytes);
```

zwraca liczbe skopiowanych bajtow lub -1 przy bledzie.

### ALC1150::pcm_pause / pcm_resume

wstrzymuje lub wznawia odtwarzanie strumienia PCM.

```
int pcm_pause(uint8_t playback, uint8_t pause);
int pcm_resume(uint8_t playback);
```

### ALC1150::mixer_get_control_count / mixer_get_control

pobiera liczbe sterownikow mixer oraz informacje o sterowniku po indeksie.

```
int mixer_get_control_count(void);
int mixer_get_control(uint8_t index, struct alc1150_mixer_control* ctrl);
```

### ALC1150::mixer_set_volume / mixer_get_volume

ustawia lub pobiera wzmocnienie danego sterownika mixer.

```
int mixer_set_volume(uint8_t index, int32_t gain);
int mixer_get_volume(uint8_t index, int32_t* gain);
```

### ALC1150::mixer_set_mute / mixer_get_mute

ustawia lub pobiera stan wyciszenia danego sterownika mixer.

```
int mixer_set_mute(uint8_t index, uint8_t mute);
int mixer_get_mute(uint8_t index, uint8_t* mute);
```

### ALC1150::jack_get_count / jack_get_status / jack_get_config / jack_enable_unsolicited

pobiera liczbe wykrytych jack, sprawdza czy dany jack ma wlaczone wtyczke, pobiera konfiguracje pinu jack oraz wlacza/wylacza niesprosowane odpowiedzi dla jack.

```
int jack_get_count(void);
int jack_get_status(uint8_t index, uint8_t* present);
int jack_get_config(uint8_t index, struct alc1150_pin_config* config);
int jack_enable_unsolicited(uint8_t index, uint8_t enable);
```

### ALC1150::spdif_open / spdif_close / spdif_write

otwiera wyjscie cyfrowe SPDIF, zamyka je oraz zapisuje dane audio do strumienia SPDIF.

```
int spdif_open(uint32_t rate, uint16_t format);
int spdif_write(const void* data, uint32_t bytes);
void spdif_close(void);
```

### ALC1150::spdif_set_category / spdif_set_copyright / spdif_set_nonaudio / spdif_set_preemphasis

konfiguruje parametry strumienia cyfrowego SPDIF (kategoria, copyright, non-audio, pre-emphasis).

```
int spdif_set_category(uint8_t category);
int spdif_set_copyright(uint8_t copyright);
int spdif_set_nonaudio(uint8_t nonaudio);
int spdif_set_preemphasis(uint8_t preemphasis);
```

### ALC1150::set_master_volume / get_master_volume

ustawia lub pobiera glosnosc glowna w procentach (0-100).

```
void set_master_volume(uint8_t percent);
uint8_t get_master_volume(void);
```

### ALC1150::power_set_state / power_get_state / widget_power_set

ustawia lub pobiera stan zasilania kodeka (D0-D3) oraz ustawia stan zasilania konkretnego widgetu.

```
void power_set_state(uint8_t state);
uint8_t power_get_state(void);
void widget_power_set(uint8_t nid, uint8_t state);
```

### ALC1150::stats_get

pobiera statystyki sterownika (liczba wyslanych verbow, bledy, przerwania, przepelnienia bufora, zmiany jack, pakiety SPDIF).

```
void stats_get(struct alc1150_stats* out);
```

struktura `alc1150_stats` zawiera:

| pole | opis |
|------|------|
| verbs_sent | liczba wyslanych verbow HDA |
| verbs_failed | bledy wysylania verbow |
| interrupts | liczba przerwan |
| buffer_underruns | niedobory bufora |
| buffer_overruns | przepelnienia bufora |
| jack_changes | zmiany stanu jack |
| spdif_packets | liczba pakietow SPDIF |
| spdif_errors | bledy SPDIF |

struktura `alc1150_mixer_control` zawiera:

| pole | opis |
|------|------|
| nid | id widgetu |
| index | indeks sterownika |
| direction | kierunek (0=input, 1=output) |
| min_gain | minimalne wzmocnienie |
| max_gain | maksymalne wzmocnienie |
| current_gain | aktualne wzmocnienie |
| mute | stan wyciszenia |
| name | nazwa sterownika |

struktura `alc1150_pin_config` zawiera:

| pole | opis |
|------|------|
| nid | id pinu |
| sequence | sekwencja priorytetu |
| default_association | domyselne skojarzenie |
| misc | rozne flagi |
| color | kolor gniazda |
| connection_type | typ polaczenia |
| location | lokalizacja (zewn./wewn.) |
| port_connectivity | typ connectivitiy portu |
| default_device | domyslne urzadzenie |

widgety ALC1150:

| typ | opis |
|------|------|
| output | konwerter cyfrowo-analogowy (DAC) |
| input | konwerter analogowo-cyfrowy (ADC) |
| mixer | mikser audio |
| selector | selektor wejsc |
| pin_complex | pin gniazda audio |
| power_widget | widget zasilania |
| volume_knob | pokretlo glosnosci |

parametry HDA:

| parametr | opis |
|----------|------|
| vendor_id | id dostawcy kodeka |
| revision_id | rewizja kodeka |
| audio_widget_cap | moznosci widgetu audio |
| pin_cap | moznosci pinu |
| amp_in_cap / amp_out_cap | moznosci wzmacniacza wejsc/wyjsc |
| connection_list_len | dlugosc listy polaczen |

widgety ALC1150 (domyslne NID):

- DAC: 0x02, 0x03, 0x04, 0x05, 0x06
- ADC: 0x07, 0x08, 0x09
- Piny: 0x0C - 0x23
- Mixery: 0x24 - 0x2B
- Selektory: 0x2C - 0x2F

funkcja C do obslugi przerwan:

```
extern "C" void alc1150_irq(void);
```

## sterownik dzwieku VIA VT1611 - drivers/karty_dzwiekowe/vt1611.cpp

sterownik karty dzwiekowej VIA VT1611 (HD Audio codec) napisany w C++. obsluguje pelna konfiguracje kodeka przez magistrale Intel HDA: wykrycie urzadzenia na PCI, inicjalizacje kontrolera HDA, odkrycie widgetow audio, konfiguracje pinow jack, mixer, strumienie PCM odtwarzania i nagrywania, wyjscie cyfrowe SPDIF, wykrywanie wlaczenia jack (jack detection), zarzadzanie zasilaniem (power management), pause/resume strumieni oraz statystyki sterownika. klasa VT1611 jest singletonem dostepna przez `VT1611::instance()`.

### VT1611::probe

sprawdza czy urzadzenie to VIA VT1611 (vendor 0x1106, device 0x3182).

```
uint8_t probe(uint16_t vendor, uint16_t device);
```

zwraca 1 gdy urzadzenie zostalo rozpoznane, 0 gdy nie.

### VT1611::init

inicjalizuje sterownik VT1611: wyszukuje kontroler Intel HDA na PCI, mapuje adresy MMIO, resetuje i wlacza kontroler, wykrywa adres kodeka, odkrywa widgety audio, parsuje konfiguracje pinow, konfiguruje domyslne piny, inicjalizuje mixer, jack detection, SPDIF i zarzadzanie zasilaniem.

```
void init(void);
```

### VT1611::deinit

zatrzymuje odtwarzanie i nagrywanie, zwalnia bufory DMA i dezaktywuje sterownik.

```
void deinit(void);
```

### VT1611::irq_handler

obsluguje przerwanie sprzegtu z kontrolera HDA i wykrywa zmiany stanu jack.

```
void irq_handler(void);
```

### VT1611::pcm_open / pcm_close

otwiera strumien PCM do odtwarzania lub nagrywania z podanymi parametrami (liczba kanalow, czestotliwosc, format) i zamyka strumien.

```
int pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback);
void pcm_close(uint8_t playback);
```

`playback` = 1 dla odtwarzania, 0 dla nagrywania. zwraca indeks strumienia lub -1 przy bledzie.

### VT1611::pcm_write / pcm_read

zapisuje dane do bufora odtwarzania lub odczytuje dane z bufora nagrywania.

```
int pcm_write(const void* data, uint32_t bytes);
int pcm_read(void* data, uint32_t bytes);
```

zwraca liczbe skopiowanych bajtow lub -1 przy bledzie.

### VT1611::pcm_pause / pcm_resume

wstrzymuje lub wznawia odtwarzanie strumienia PCM.

```
int pcm_pause(uint8_t playback, uint8_t pause);
int pcm_resume(uint8_t playback);
```

### VT1611::mixer_get_control_count / mixer_get_control

pobiera liczbe sterownikow mixer oraz informacje o sterowniku po indeksie.

```
int mixer_get_control_count(void);
int mixer_get_control(uint8_t index, struct vt1611_mixer_control* ctrl);
```

### VT1611::mixer_set_volume / mixer_get_volume

ustawia lub pobiera wzmocnienie danego sterownika mixer.

```
int mixer_set_volume(uint8_t index, int32_t gain);
int mixer_get_volume(uint8_t index, int32_t* gain);
```

### VT1611::mixer_set_mute / mixer_get_mute

ustawia lub pobiera stan wyciszenia danego sterownika mixer.

```
int mixer_set_mute(uint8_t index, uint8_t mute);
int mixer_get_mute(uint8_t index, uint8_t* mute);
```

### VT1611::jack_get_count / jack_get_status / jack_get_config / jack_enable_unsolicited

pobiera liczbe wykrytych jack, sprawdza czy dany jack ma wlaczone wtyczke, pobiera konfiguracje pinu jack oraz wlacza/wylacza niesprosowane odpowiedzi dla jack.

```
int jack_get_count(void);
int jack_get_status(uint8_t index, uint8_t* present);
int jack_get_config(uint8_t index, struct vt1611_pin_config* config);
int jack_enable_unsolicited(uint8_t index, uint8_t enable);
```

### VT1611::spdif_open / spdif_close / spdif_write

otwiera wyjscie cyfrowe SPDIF, zamyka je oraz zapisuje dane audio do strumienia SPDIF.

```
int spdif_open(uint32_t rate, uint16_t format);
int spdif_write(const void* data, uint32_t bytes);
void spdif_close(void);
```

### VT1611::spdif_set_category / spdif_set_copyright / spdif_set_nonaudio / spdif_set_preemphasis

konfiguruje parametry strumienia cyfrowego SPDIF (kategoria, copyright, non-audio, pre-emphasis).

```
int spdif_set_category(uint8_t category);
int spdif_set_copyright(uint8_t copyright);
int spdif_set_nonaudio(uint8_t nonaudio);
int spdif_set_preemphasis(uint8_t preemphasis);
```

### VT1611::set_master_volume / get_master_volume

ustawia lub pobiera glosnosc glowna w procentach (0-100).

```
void set_master_volume(uint8_t percent);
uint8_t get_master_volume(void);
```

### VT1611::power_set_state / power_get_state / widget_power_set

ustawia lub pobiera stan zasilania kodeka (D0-D3) oraz ustawia stan zasilania konkretnego widgetu.

```
void power_set_state(uint8_t state);
uint8_t power_get_state(void);
void widget_power_set(uint8_t nid, uint8_t state);
```

### VT1611::stats_get

pobiera statystyki sterownika (liczba wyslanych verbow, bledy, przerwania, przepelnienia bufora, zmiany jack, pakiety SPDIF).

```
void stats_get(struct vt1611_stats* out);
```

struktura `vt1611_stats` zawiera:

| pole | opis |
|------|------|
| verbs_sent | liczba wyslanych verbow HDA |
| verbs_failed | bledy wysylania verbow |
| interrupts | liczba przerwan |
| buffer_underruns | niedobory bufora |
| buffer_overruns | przepelnienia bufora |
| jack_changes | zmiany stanu jack |
| spdif_packets | liczba pakietow SPDIF |
| spdif_errors | bledy SPDIF |

struktura `vt1611_mixer_control` zawiera:

| pole | opis |
|------|------|
| nid | id widgetu |
| index | indeks sterownika |
| direction | kierunek (0=input, 1=output) |
| min_gain | minimalne wzmocnienie |
| max_gain | maksymalne wzmocnienie |
| current_gain | aktualne wzmocnienie |
| mute | stan wyciszenia |
| name | nazwa sterownika |

struktura `vt1611_pin_config` zawiera:

| pole | opis |
|------|------|
| nid | id pinu |
| sequence | sekwencja priorytetu |
| default_association | domyselne skojarzenie |
| misc | rozne flagi |
| color | kolor gniazda |
| connection_type | typ polaczenia |
| location | lokalizacja (zewn./wewn.) |
| port_connectivity | typ connectivitiy portu |
| default_device | domyslne urzadzenie |

widgety VT1611:

| typ | opis |
|------|------|
| output | konwerter cyfrowo-analogowy (DAC) |
| input | konwerter analogowo-cyfrowy (ADC) |
| mixer | mikser audio |
| selector | selektor wejsc |
| pin_complex | pin gniazda audio |
| power_widget | widget zasilania |
| volume_knob | pokretlo glosnosci |

parametry HDA:

| parametr | opis |
|----------|------|
| vendor_id | id dostawcy kodeka |
| revision_id | rewizja kodeka |
| audio_widget_cap | moznosci widgetu audio |
| pin_cap | moznosci pinu |
| amp_in_cap / amp_out_cap | moznosci wzmacniacza wejsc/wyjsc |
| connection_list_len | dlugosc listy polaczen |

widgety VT1611 (domyslne NID):

- DAC: 0x02, 0x03, 0x04, 0x05, 0x06
- ADC: 0x07, 0x08, 0x09
- Piny: 0x0C - 0x1F
- Mixery: 0x20 - 0x27
- Selektory: 0x28 - 0x2B

funkcja C do obslugi przerwan:

```
extern "C" void vt1611_irq(void);
```

## sterownik dzwieku HDA Audio - drivers/karty_dzwiekowe/hda_codec.cpp

sterownik dzwieku Intel HD Audio napisany w C++. obsluguje pelna konfiguracje kodeka przez magistrale Intel HDA: wykrycie urzadzenia na PCI, inicjalizacje kontrolera HDA, odkrycie widgetow audio, konfiguracje pinow jack, mixer, strumienie PCM odtwarzania i nagrywania, wyjscie cyfrowe SPDIF, wykrywanie wlaczenia jack (jack detection), zarzadzanie zasilaniem (power management), pause/resume strumieni, IPC komunikacja, ladowanie firmware, tworzenie i zarzadzanie potokami (pipelines) oraz komponentami oraz statystyki sterownika. klasa HDAudioCodec jest singletonem dostepna przez `HDAudioCodec::instance()`.

### HDAudioCodec::probe

sprawdza czy urzadzenie to HDA Audio (vendor 0x8086, device 0x9DC8).

```
uint8_t probe(uint16_t vendor, uint16_t device);
```

zwraca 1 gdy urzadzenie zostalo rozpoznane, 0 gdy nie.

### HDAudioCodec::init

inicjalizuje sterownik HDA: wyszukuje kontroler Intel HDA na PCI, mapuje adresy MMIO, inicjalizuje bufor IPC, resetuje i wlacza kontroler, wykrywa adres kodeka, odkrywa widgety audio, parsuje konfiguracje pinow, konfiguruje domyslne piny, inicjalizuje mixer, jack detection, SPDIF i zarzadzanie zasilaniem.

```
void init(void);
```

### HDAudioCodec::deinit

zatrzymuje odtwarzanie i nagrywanie, niszczy potoki, zwalnia bufory DMA i dezaktywuje sterownik.

```
void deinit(void);
```

### HDAudioCodec::irq_handler

obsluguje przerwanie sprzegtu z kontrolera HDA, obsluguje drzwiowe (doorbell) IPC oraz wykrywa zmiany stanu jack.

```
void irq_handler(void);
```

### HDAudioCodec::pcm_open / pcm_close

otwiera strumien PCM do odtwarzania lub nagrywania z podanymi parametrami (liczba kanalow, czestotliwosc, format), tworzy potok dla strumienia i zamyka strumien.

```
int pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback);
void pcm_close(uint8_t playback);
```

`playback` = 1 dla odtwarzania, 0 dla nagrywania. zwraca indeks strumienia lub -1 przy bledzie.

### HDAudioCodec::pcm_write / pcm_read

zapisuje dane do bufora odtwarzania lub odczytuje dane z bufora nagrywania i uruchamia potok.

```
int pcm_write(const void* data, uint32_t bytes);
int pcm_read(void* data, uint32_t bytes);
```

zwraca liczbe skopiowanych bajtow lub -1 przy bledzie.

### HDAudioCodec::pcm_pause / pcm_resume

wstrzymuje lub wznawia odtwarzanie strumienia PCM.

```
int pcm_pause(uint8_t playback, uint8_t pause);
int pcm_resume(uint8_t playback);
```

### HDAudioCodec::mixer_get_control_count / mixer_get_control

pobiera liczbe sterownikow mixer oraz informacje o sterowniku po indeksie.

```
int mixer_get_control_count(void);
int mixer_get_control(uint8_t index, void* ctrl);
```

### HDAudioCodec::mixer_set_volume / mixer_get_volume

ustawia lub pobiera wzmocnienie danego sterownika mixer.

```
int mixer_set_volume(uint8_t index, int32_t gain);
int mixer_get_volume(uint8_t index, int32_t* gain);
```

### HDAudioCodec::mixer_set_mute / mixer_get_mute

ustawia lub pobiera stan wyciszenia danego sterownika mixer.

```
int mixer_set_mute(uint8_t index, uint8_t mute);
int mixer_get_mute(uint8_t index, uint8_t* mute);
```

### HDAudioCodec::jack_get_count / jack_get_status / jack_get_config / jack_enable_unsolicited

pobiera liczbe wykrytych jack, sprawdza czy dany jack ma wlaczone wtyczke, pobiera konfiguracje pinu jack oraz wlacza/wylacza niesprosowane odpowiedzi dla jack.

```
int jack_get_count(void);
int jack_get_status(uint8_t index, uint8_t* present);
int jack_get_config(uint8_t index, struct hda_codec_pin_config* config);
int jack_enable_unsolicited(uint8_t index, uint8_t enable);
```

### HDAudioCodec::spdif_open / spdif_close / spdif_write

otwiera wyjscie cyfrowe SPDIF, zamyka je oraz zapisuje dane audio do strumienia SPDIF.

```
int spdif_open(uint32_t rate, uint16_t format);
int spdif_write(const void* data, uint32_t bytes);
void spdif_close(void);
```

### HDAudioCodec::spdif_set_category / spdif_set_copyright / spdif_set_nonaudio / spdif_set_preemphasis

konfiguruje parametry strumienia cyfrowego SPDIF (kategoria, copyright, non-audio, pre-emphasis).

```
int spdif_set_category(uint8_t category);
int spdif_set_copyright(uint8_t copyright);
int spdif_set_nonaudio(uint8_t nonaudio);
int spdif_set_preemphasis(uint8_t preemphasis);
```

### HDAudioCodec::set_master_volume / get_master_volume

ustawia lub pobiera glosnosc glowna w procentach (0-100).

```
void set_master_volume(uint8_t percent);
uint8_t get_master_volume(void);
```

### HDAudioCodec::power_set_state / power_get_state / widget_power_set

ustawia lub pobiera stan zasilania kodeka (D0-D3) oraz ustawia stan zasilania konkretnego widgetu.

```
void power_set_state(uint8_t state);
uint8_t power_get_state(void);
void widget_power_set(uint8_t nid, uint8_t state);
```

### HDAudioCodec::stats_get

pobiera statystyki sterownika (liczba wyslanych verbow, bledy, przerwania, wiadomosci IPC, bledy IPC, przepelnienia bufora, zmiany jack, pakiety SPDIF, ladowania firmware).

```
void stats_get(struct hda_codec_stats* out);
```

struktura `hda_codec_stats` zawiera:

| pole | opis |
|------|------|
| verbs_sent | liczba wyslanych verbow HDA |
| verbs_failed | bledy wysylania verbow |
| interrupts | liczba przerwan |
| ipc_messages | liczba wiadomosci IPC |
| ipc_errors | bledy komunikacji IPC |
| buffer_underruns | niedobory bufora |
| buffer_overruns | przepelnienia bufora |
| jack_changes | zmiany stanu jack |
| spdif_packets | liczba pakietow SPDIF |
| spdif_errors | bledy SPDIF |
| firmware_loads | liczba ladowan firmware |

struktura `hda_codec_pin_config` zawiera:

| pole | opis |
|------|------|
| nid | id pinu |
| sequence | sekwencja priorytetu |
| default_association | domyselne skojarzenie |
| color | kolor gniazda |
| connection_type | typ polaczenia |
| location | lokalizacja (zewn./wewn.) |
| port_connectivity | typ connectivitiy portu |
| default_device | domyslne urzadzenie |

struktura `hda_codec_component` zawiera:

| pole | opis |
|------|------|
| component_id | id komponentu |
| type | typ komponentu (host, DAI, ASRC, mixer, itp.) |
| pipeline_id | id potoku do ktorego nalezy |
| config | konfiguracja komponentu |
| active | czy komponent jest aktywny |

struktura `hda_codec_pipeline` zawiera:

| pole | opis |
|------|------|
| pipeline_id | id potoku |
| type | typ potoku (playback lub capture) |
| component_count | liczba komponentow w potoku |
| component_ids | lista id komponentow |
| active | czy potok jest aktywny |
| rate | czestotliwosc probkowania |
| channels | liczba kanalow |
| format | format probek |

widgety HDA Audio:

| typ | opis |
|------|------|
| output | konwerter cyfrowo-analogowy (DAC) |
| input | konwerter analogowo-cyfrowy (ADC) |
| mixer | mikser audio |
| selector | selektor wejsc |
| pin_complex | pin gniazda audio |
| power_widget | widget zasilania |
| volume_knob | pokretlo glosnosci |

komponenty HDA:

| typ | opis |
|------|------|
| host | komponent hosta CPU |
| DAI | cyfrowy interfejs audio |
| ASRC | konwerter probkowania asynchronicznego |
| mixer | mikser audio |
| selector | selektor wejsc |
| mux | multipleksor |
| volume | sterowanie glosnoscia |
| switch | przelacznik |

typy potokow:

| typ | opis |
|------|------|
| playback | potok odtwarzania audio |
| capture | potok nagrywania audio |

komendy IPC:

| komenda | opis |
|--------|------|
| PCM_PARAMS | ustaw parametrow PCM |
| PCM_PREPARE | przygotuj strumien PCM |
| PCM_TRIGGER | wyzwalacz strumienia PCM |
| PCM_FREE | zwolnij strumien PCM |
| PCM_DRAIN | oprzezbuj strumien PCM |
| PCM_STOP | zatrzymaj strumien PCM |

tryby wyzwalacza PCM:

| tryb | opis |
|------|------|
| start | rozpocznij odtwarzanie/nagrywanie |
| stop | zatrzymaj odtwarzanie/nagrywanie |
| pause | wstrzymaj odtwarzanie/nagrywanie |
| release | zwolnij wstrzymanie |

widgety HDA Audio (domyslne NID):

- DAC: 0x02, 0x03, 0x04, 0x05, 0x06
- ADC: 0x07, 0x08, 0x09
- Piny: 0x0C - 0x17
- Mixery: 0x18 - 0x1F
- Selektory: 0x20 - 0x23

funkcja C do obslugi przerwan:

```
extern "C" void hda_codec_irq(void);
```

## sterownik plyty glownej ASUS P4P800 - drivers/plyty_glowne/asus_p4p800.cpp

Zaawansowany sterownik plyty glownej ASUS P4P800 (chipset Intel i848P + ICH4). Obsluguje wszystkie wbudowane kontrolery plyty: PCI, chipset, ACPI, SMBus, AGP, audio AC'97, ethernet Intel 82562ET, IDE z DMA, USB UHCI/EHCI, stacje dyskowetek, RTC/CMOS oraz monitorowanie sprzetowe (HWMON). Sterownik jest napisany w C++, eksportuje interfejs C przez naglwek asus_p4p800.h i jest inicjalizowany z kernel/kernel.c przez `p4p_init()`.

### Inicjalizacja i stan

| funkcja | opis |
|---------|------|
| `int p4p_init(void)` | Inicjalizuje wszystkie podsystemy plyty glownej (chipset, ACPI, SMBus, AGP, audio, ethernet, IDE, USB, floppy, RTC, HWMON). Zwraca 1 po udanej inicjalizacji. |
| `int p4p_is_ready(void)` | Zwraca 1 gdy sterownik zostal zainicjalizowany i jest gotowy. |
| `void p4p_shutdown(void)` | Wylacza plyte glowna przez ACPI (stan S5) lub fallback przez klawiatura 0x64. |
| `void p4p_reset(void)` | Restartuje komputer przez port reset 0xCF9 i klawiatura 0x64. |
| `uint32_t p4p_get_chipset_id(void)` | Zwraca polaczone ID MCH i ICH (MCH << 16 | ICH). |
| `void p4p_info_get(struct p4p_info *out)` | Kopiuje strukture z informacjami o wykrytych urzadzeniach i adresach bazowych. |
| `void p4p_stats_get(struct p4p_stats *out)` | Kopiuje statystyki wszystkich podsystemow (PCI, audio, ethernet, IDE, USB, floppy, HWMON, IRQ). |

### PCI

| funkcja | opis |
|---------|------|
| `int p4p_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)` | Wyszukuje urzadzenie PCI po vendor/device ID, zwraca index dla wielu urzadzen tego samego typu. |
| `uint32_t p4p_pci_read32(...)` / `void p4p_pci_write32(...)` | Odczyt/zapis 32-bitowy do rejestru konfiguracji PCI. |
| `uint16_t p4p_pci_read16(...)` / `void p4p_pci_write16(...)` | Odczyt/zapis 16-bitowy do rejestru konfiguracji PCI. |
| `uint8_t p4p_pci_read8(...)` / `void p4p_pci_write8(...)` | Odczyt/zapis 8-bitowy do rejestru konfiguracji PCI. |
| `int p4p_pci_set_bus_master(uint8_t bus, uint8_t slot, uint8_t func, uint8_t enable)` | Wlacza/wylacza bus mastering dla urzadzenia PCI (bit 2 w rejestrze command). |

### Chipset (Intel i848P + ICH4)

| funkcja | opis |
|---------|------|
| `int p4p_chipset_probe(void)` | Wykrywa MCH i ICH4, zapisuje ID, rewizje, adres ACPI i wlacza bus mastering. |
| `void p4p_chipset_configure_memory(void)` | Konfiguruje rejestry sterowania pamieci chipsetu (rejestr 0x5C MCH). |

### ACPI

| funkcja | opis |
|---------|------|
| `int p4p_acpi_available(void)` | Sprawdza dostepnosc ACPI i wlacza SCI (System Control Interrupt). |
| `void p4p_acpi_sleep(uint8_t state)` | Wprowadza system w stan uspienia ACPI (S1-S5) przez rejestr PM1A_CNT. |
| `void p4p_acpi_reboot(void)` | Restart komputera przez port 0xCF9 i klawiatura. |
| `void p4p_acpi_shutdown(void)` | Wylaczenie systemu przez ACPI S5 lub fallback. |

### SMBus (ICH4)

| funkcja | opis |
|---------|------|
| `int p4p_smbus_init(void)` | Wykrywa i inicjalizuje kontroler SMBus ICH4. |
| `int p4p_smbus_read_byte(uint8_t device, uint8_t offset, uint8_t *value)` | Odczyt bajtu z urzadzenia SMBus. |
| `int p4p_smbus_write_byte(uint8_t device, uint8_t offset, uint8_t value)` | Zapis bajtu do urzadzenia SMBus. |

### AGP (Intel i848P)

| funkcja | opis |
|---------|------|
| `int p4p_agp_init(void)` | Inicjalizuje aperture AGP i wykrywa jego adres bazowy. |
| `uint32_t p4p_agp_aperture(void)` | Zwraca rozmiar aperture AGP. |
| `int p4p_agp_bind(uint32_t gatt_index, uint32_t physical_addr)` | Bindowanie strony GATT do adresu fizycznego. |
| `int p4p_agp_unbind(uint32_t gatt_index)` | Unbindowanie strony GATT. |

### Audio AC'97 (ICH4 + kodek)

| funkcja | opis |
|---------|------|
| `int p4p_audio_init(void)` | Inicjalizuje kontroler AC'97 i resetuje kodek (cold reset). |
| `int p4p_audio_is_ready(void)` | Zwraca 1 gdy audio jest gotowe. |
| `int p4p_audio_codec_read(uint8_t reg, uint16_t *value)` | Odczyt rejestru kodeka AC'97. |
| `int p4p_audio_codec_write(uint8_t reg, uint16_t value)` | Zapis rejestru kodeka AC'97. |
| `int p4p_audio_codec_reset(int cold)` | Cold lub warm reset kodeka AC'97. |
| `int p4p_audio_set_volume(uint8_t channel, uint8_t percent)` | Ustawia glosnosc kanalu (0-3: master, master mono, PCM out, PCM in). |
| `uint8_t p4p_audio_get_volume(uint8_t channel)` | Zwraca aktualna glosnosc kanalu. |
| `int p4p_audio_playback_start(const void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)` | Rozpoczyna odtwarzanie PCM z bufora przez DMA kontrolera BM. |
| `int p4p_audio_playback_stop(void)` | Zatrzymuje odtwarzanie PCM. |
| `int p4p_audio_capture_start(void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)` | Rozpoczyna nagrywanie PCM do bufora przez DMA. |
| `int p4p_audio_capture_stop(void)` | Zatrzymuje nagrywanie PCM. |
| `void p4p_audio_stats_get(struct p4p_audio_stats *out)` | Statystyki audio (codec_reads, codec_writes, playback_bytes, capture_bytes, errors). |

### Ethernet (Intel 82562ET)

| funkcja | opis |
|---------|------|
| `int p4p_eth_init(void)` | Inicjalizuje kontroler ethernetu, resetuje port i wlacza CU/RU. |
| `int p4p_eth_is_ready(void)` | Zwraca 1 gdy ethernet jest gotowy. |
| `int p4p_eth_get_mac(uint8_t mac[6])` | Pobiera adres MAC z kontrolera. |
| `int p4p_eth_link_up(void)` | Zwraca 1 gdy lacze ethernetowe jest aktywne. |
| `uint32_t p4p_eth_speed_mbps(void)` | Zwraca predkosc lacza w Mbps (100). |
| `int p4p_eth_send(const void *frame, uint16_t length)` | Wysyla ramke ethernetowa. |
| `int p4p_eth_receive(void *frame, uint16_t capacity)` | Odbiera ramke ethernetowa. |
| `void p4p_eth_stats_get(struct p4p_eth_stats *out)` | Statystyki ethernetu (tx/rx pakiety, bajty, bledy, resety). |

### IDE (ICH4, PIO + DMA)

| funkcja | opis |
|---------|------|
| `int p4p_ide_init(void)` | Inicjalizuje kontrolery IDE (kanaly primary i secondary) i bus mastering. |
| `int p4p_ide_is_ready(void)` | Zwraca 1 gdy IDE jest gotowe. |
| `int p4p_ide_identify(uint8_t channel, uint8_t drive, uint16_t *buffer)` | Wysyla polecenie IDENTIFY i pobiera 512 bajtow identyfikacji dysku. |
| `int p4p_ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)` | Czytanie sektorow LBA w trybie PIO. |
| `int p4p_ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)` | Zapis sektorow LBA w trybie PIO. |
| `int p4p_ide_dma_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)` | Czytanie sektorow przez DMA bus mastering. |
| `int p4p_ide_dma_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)` | Zapis sektorow przez DMA bus mastering. |
| `void p4p_ide_reset(uint8_t channel)` | Reset kontrolera IDE przez port alternativ status. |
| `void p4p_ide_stats_get(struct p4p_ide_stats *out)` | Statystyki IDE (reads, writes, sektory, DMA transfery, bledy). |

### USB (ICH4 UHCI x3 + EHCI)

| funkcja | opis |
|---------|------|
| `int p4p_usb_init(void)` | Wykrywa i inicjalizuje trzy kontrolery UHCI oraz EHCI. |
| `int p4p_usb_is_ready(void)` | Zwraca 1 gdy USB jest gotowe. |
| `uint8_t p4p_usb_root_port_count(void)` | Zwraca liczba portow roota (6). |
| `int p4p_usb_port_reset(uint8_t port)` | Reset portu USB (bit reset w PORTSC). |
| `int p4p_usb_port_power(uint8_t port, uint8_t on)` | Wlacza/wylacza zasilanie portu USB. |
| `uint32_t p4p_usb_port_status(uint8_t port)` | Zwraca rejestr statusu portu USB. |
| `int p4p_usb_control_transfer(...)` | Transfer sterowniczy USB (placeholder gotowy do rozszerzenia). |
| `int p4p_usb_bulk_transfer(...)` | Transfer masowy USB (placeholder gotowy do rozszerzenia). |
| `void p4p_usb_stats_get(struct p4p_usb_stats *out)` | Statystyki USB (porty, resety, transfery, bledy). |

### Stacja dyskowetek (FDC)

| funkcja | opis |
|---------|------|
| `int p4p_floppy_init(void)` | Inicjalizuje kontroler FDC (0x3F0), reset i specify. |
| `int p4p_floppy_is_ready(void)` | Zwraca 1 gdy FDC jest gotowe. |
| `int p4p_floppy_recalibrate(uint8_t drive)` | Rekalibracja napedu dyskowetki (seek do track 0). |
| `int p4p_floppy_seek(uint8_t drive, uint8_t track)` | Posuniecie glowicy napedu na zadany track. |
| `int p4p_floppy_read_sectors(uint8_t drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t count, void *buffer)` | Czytanie sektorow z dyskowetki. |
| `int p4p_floppy_write_sectors(uint8_t drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t count, const void *buffer)` | Zapis sektorow na dyskietke. |
| `uint8_t p4p_floppy_status(void)` | Zwraca rejestr statusu FDC (Status A). |
| `void p4p_floppy_stats_get(struct p4p_floppy_stats *out)` | Statystyki FDC (reads, writes, seeks, bledy). |

### RTC / CMOS

| funkcja | opis |
|---------|------|
| `int p4p_rtc_init(void)` | Inicjalizuje RTC (tryb 24h, format binarny). |
| `int p4p_rtc_is_ready(void)` | Zwraca 1 gdy RTC jest gotowe. |
| `int p4p_rtc_get_time(struct p4p_rtc_time *out)` | Pobiera aktualny czas i date z RTC. |
| `int p4p_rtc_set_time(const struct p4p_rtc_time *t)` | Ustawia czas i date w RTC. |
| `int p4p_rtc_nvram_read(uint8_t offset, uint8_t *value)` | Odczyt bajtu z pamieci CMOS/NVRAM. |
| `int p4p_rtc_nvram_write(uint8_t offset, uint8_t value)` | Zapis bajtu do pamieci CMOS/NVRAM. |
| `int p4p_rtc_alarm_set(const struct p4p_rtc_time *t)` | Ustawia alarm RTC. |
| `int p4p_rtc_irq_enable(uint8_t mask, uint8_t enable)` | Wlacza/wylacza przerwania RTC (update, alarm, periodic). |
| `uint8_t p4p_rtc_irq_status(void)` | Zwraca rejestr statusu C RTC (flagi przerwan). |

### Monitorowanie sprzetowe (HWMON, Super I/O)

| funkcja | opis |
|---------|------|
| `int p4p_hwmon_init(void)` | Inicjalizuje Super I/O i wlacza logiczne urzadzenie HWMON. |
| `int p4p_hwmon_is_ready(void)` | Zwraca 1 gdy HWMON jest gotowe. |
| `int p4p_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius)` | Odczyt temperatury z czujnika w milicelcjuszach. |
| `int p4p_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt)` | Odczyt napiecia (placeholder). |
| `int p4p_hwmon_read_fan(uint8_t fan, uint32_t *rpm)` | Odczyt obrotow wentylatora (placeholder). |
| `void p4p_hwmon_stats_get(struct p4p_hwmon_stats *out)` | Statystyki HWMON (temperatury, napiecia, wentylatory, bledy). |

### Obsluga przerwan (IRQ)

| funkcja | opis |
|---------|------|
| `void p4p_irq_handler(uint8_t irq)` | Dysponuje przerwanie do zarejestrowanego handlera dla danego IRQ. |
| `int p4p_irq_register(uint8_t irq, void (*handler)(uint8_t irq))` | Rejestruje handler przerwania dla IRQ 0-15. |

### Integracja z jadrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `intel_uhd_init()`:

```
p4p_init();
```

Naglowek `drivers/plyty_glowne/asus_p4p800.h` jest dolaczany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywolywane z kodu C jadra.

## sterownik plyty glownej Intel 915 - drivers/plyty_glowne/intel_915.cpp

Zaawansowany sterownik plyty glownej Intel 915 (chipset MCH 82915G/915P/915X/915GV/915GL + ICH6). Sterownik jest napisany w C++ i obsluguje wszystkie wbudowane kontrolery plyty: PCI, chipset MCH/ICH6, pamieci RAM (PAM, DRB, DRA, TOLUD, GME, stolen memory), ACPI, SMBus, AGP, graficzne Intel GMA 900 z framebufferem, blitterem, kursorem sprzetowym i overlayem, AC'97 audio, IDE z PIO i DMA, SATA, USB UHCI x3 + EHCI, TCO watchdog, GPIO, PIRQ routing, SMI, RTC/CMOS, FWH/BIOS lock, monitorowanie sprzetowe (HWMON) oraz obsluge przerwan IRQ. Eksportuje interfejs C przez naglwek intel_915.h i jest inicjalizowany z kernel/kernel.c przez `i915_init()`.

### Inicjalizacja i stan

| funkcja | opis |
|---------|------|
| `int i915_init(void)` | Inicjalizuje wszystkie podsystemy plyty glownej (chipset, pamiec, ACPI, SMBus, AGP, grafika, audio, IDE, SATA, USB, TCO, RTC, HWMON, BIOS). Zwraca 1 po udanej inicjalizacji. |
| `int i915_is_ready(void)` | Zwraca 1 gdy sterownik zostal zainicjalizowany i jest gotowy. |
| `void i915_shutdown(void)` | Wylacza plyte glowna przez ACPI (stan S5) lub fallback przez klawiatura 0x64. |
| `void i915_reset(void)` | Restartuje komputer przez port reset 0xCF9 i klawiatura 0x64. |
| `uint32_t i915_get_chipset_id(void)` | Zwraca polaczone ID MCH i ICH (MCH << 16 | ICH). |
| `void i915_info_get(struct i915_info *out)` | Kopiuje strukture z informacjami o wykrytych urzadzeniach i adresach bazowych. |
| `void i915_stats_get(struct i915_stats *out)` | Kopiuje statystyki wszystkich podsystemow (PCI, pamiec, audio, ethernet, IDE, USB, floppy, HWMON, grafika, IRQ, ACPI, TCO, SMBus). |

### PCI

| funkcja | opis |
|---------|------|
| `int i915_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)` | Wyszukuje urzadzenie PCI po vendor/device ID, zwraca index dla wielu urzadzen tego samego typu. |
| `uint32_t i915_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)` | Odczyt 32-bitowy z rejestru konfiguracji PCI z licznikiem statystyk. |
| `void i915_pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)` | Zapis 32-bitowy do rejestru konfiguracji PCI z licznikiem statystyk. |
| `uint16_t i915_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)` | Odczyt 16-bitowy z rejestru konfiguracji PCI. |
| `void i915_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value)` | Zapis 16-bitowy do rejestru konfiguracji PCI. |
| `uint8_t i915_pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)` | Odczyt 8-bitowy z rejestru konfiguracji PCI. |
| `void i915_pci_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value)` | Zapis 8-bitowy do rejestru konfiguracji PCI. |
| `int i915_pci_set_bus_master(uint8_t bus, uint8_t slot, uint8_t func, uint8_t enable)` | Wlacza/wylacza bus mastering dla urzadzenia PCI (bit 2 w rejestrze command). |
| `uint8_t i915_pci_read_revision(uint8_t bus, uint8_t slot, uint8_t func)` | Odczyt rejestru revision ID urzadzenia PCI. |
| `int i915_pci_route_irq(uint8_t bus, uint8_t slot, uint8_t func, uint8_t *irq)` | Odczyt linii przerwania IRQ i pinu dla urzadzenia PCI. |

### Chipset (Intel 82915 + ICH6)

| funkcja | opis |
|---------|------|
| `int i915_chipset_probe(void)` | Wykrywa MCH (82915G/915P/915X/915GV/915GL) i ICH6, zapisuje ID, rewizje, adres ACPI i wlacza bus mastering. |
| `void i915_chipset_configure_memory(void)` | Konfiguruje rejestry sterowania pamieci chipsetu (GGC i DEVEN MCH). |

### Pamiec (PAM, DRAM, AGP)

| funkcja | opis |
|---------|------|
| `int i915_pam_read(uint8_t reg, uint8_t *value)` | Odczyt rejestru PAM (Programmable Attribute Map) chipsetu MCH. |
| `int i915_pam_write(uint8_t reg, uint8_t value)` | Zapis rejestru PAM chipsetu MCH. |
| `int i915_memory_map(struct i915_dram_info *out)` | Wykrywa konfiguracja pamieci DRAM (DRB, DRA), oblicza laczna pojemnosc i timingi. |
| `int i915_memory_remap_enable(uint8_t enable)` | Wlacza/wylacza remapowanie pamieci chipsetu. |
| `int i915_memory_set_timings(uint32_t tcl, uint32_t trcd, uint32_t trp, uint32_t tras)` | Ustawia timingi pamieci DRAM (TCL, TRCD, TRP, TRAS). |
| `int i915_memory_get_timings(uint32_t *tcl, uint32_t *trcd, uint32_t *trp, uint32_t *tras)` | Pobiera aktualne timingi pamieci DRAM. |
| `int i915_memory_scrub(uint32_t start, uint32_t size)` | Wyzerowanie (scrub) obszaru pamieci fizycznej. |
| `int i915_memory_interrupt_enable(uint8_t enable)` | Wlacza/wylacza przerwania kontrolera pamieci. |
| `int i915_gme_enable(uint8_t enable)` | Wlacza/wylacza Graficzna Engine MCH (GME). |
| `uint32_t i915_stolen_memory_size(void)` | Zwraca rozmiar pamieci ukradzionej (stolen memory) dla grafiki. |
| `int i915_agp_init(void)` | Inicjalizuje aperture AGP i wykrywa jego adres bazowy. |
| `uint32_t i915_agp_aperture(void)` | Zwraca rozmiar aperture AGP. |
| `int i915_agp_bind(uint32_t gatt_index, uint32_t physical_addr)` | Bindowanie strony GATT do adresu fizycznego. |
| `int i915_agp_unbind(uint32_t gatt_index)` | Unbindowanie strony GATT. |

### Grafika Intel GMA 900 (zintegrowana w MCH)

| funkcja | opis |
|---------|------|
| `int i915_graphics_init(void)` | Inicjalizuje kontroler graficzny: MMIO, DPLL, timingi pipe A, plaszczyzna A, ring buffer blitera, kursor sprzetowy. |
| `int i915_graphics_is_ready(void)` | Zwraca 1 gdy sterownik graficzny jest gotowy. |
| `void i915_graphics_putpixel(uint32_t x, uint32_t y, uint32_t color)` | Rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem. |
| `void i915_graphics_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)` | Wypelnia prostokat o podanym rozmiarze i kolorze (uzywa blitera sprzetowego lub fallback CPU). |
| `void i915_graphics_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)` | Kopiuje blok pikseli (XY_SRC_COPY_BLT) uzywajac blitera sprhetowego. |
| `void i915_graphics_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)` | Ustawia kursor sprhetowy na pozycji (x, y) z obrazem 64x64. |
| `void i915_graphics_cursor_move(uint32_t x, uint32_t y)` | Przesuwa kursor sprhetowy na nowa pozycje bez zmiany obrazu. |
| `void i915_graphics_cursor_hide(void)` | Ukrywa kursor sprhetowy. |
| `void i915_graphics_overlay_enable(uint8_t enable)` | Wlacza/wylacza overlay wideo. |
| `int i915_graphics_overlay_set_buffer(uint32_t addr, uint32_t width, uint32_t height, uint32_t pitch)` | Ustawia bufor overlay wideo. |
| `void i915_graphics_display_timings(uint32_t width, uint32_t height, uint32_t refresh)` | Konfiguruje timingi wyswietlania (placeholder). |
| `void i915_graphics_pipe_config(uint8_t pipe, uint8_t enable)` | Konfiguruje pipe wyswietlania (A lub B). |
| `void i915_graphics_plane_config(uint8_t plane, uint32_t addr, uint32_t width, uint32_t height, uint32_t stride)` | Konfiguruje plaszczyzne wyswietlania. |
| `void i915_graphics_blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)` | Blitter: wypelnienie prostokatem (XY_FILL_BLT). |
| `void i915_graphics_blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)` | Blitter: kopiowanie bloku pikseli (XY_SRC_COPY_BLT). |
| `void *i915_graphics_get_fb(void)` | Zwraca wskaznik na ramke graficzna (framebuffer). |
| `uint32_t i915_graphics_get_width(void)` | Zwraca szerokosc ekranu (1024). |
| `uint32_t i915_graphics_get_height(void)` | Zwraca wysokosc ekranu (768). |
| `uint32_t i915_graphics_get_pitch(void)` | Zwraca pitch (odstep miedzy wierszami) ramki graficznej (4096). |
| `uint16_t i915_graphics_get_device_id(void)` | Zwraca ID urzadzenia graficznego. |
| `uint8_t i915_graphics_get_revision(void)` | Zwraca rewizje karty graficznej. |
| `void i915_graphics_info_get(struct i915_graphics_info *out)` | Pobiera pelne informacje o urzadzeniu graficznym. |
| `void i915_graphics_stats_get(struct i915_graphics_stats *out)` | Pobiera statystyki grafiki (putpixels, fillrects, blts, cursor_moves, overlay_updates, ring_flushes, bledy). |
| `void i915_graphics_vga_enable(uint8_t enable)` | Wlacza/wylacza tryb VGA kompatybilny. |
| `void i915_graphics_ring_flush(void)` | Wymusza wykonanie polecen w ring bufferze blitera. |

### IDE (ICH6, PIO + DMA)

| funkcja | opis |
|---------|------|
| `int i915_ide_init(void)` | Inicjalizuje kontrolery IDE (primary i secondary) oraz bus mastering. |
| `int i915_ide_is_ready(void)` | Zwraca 1 gdy IDE jest gotowe. |
| `int i915_ide_identify(uint8_t channel, uint8_t drive, uint16_t *buffer)` | Wysyla polecenie IDENTIFY i pobiera 512 bajtow identyfikacji dysku. |
| `int i915_ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)` | Czytanie sektorow LBA w trybie PIO. |
| `int i915_ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)` | Zapis sektorow LBA w trybie PIO. |
| `int i915_ide_dma_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)` | Czytanie sektorow przez DMA bus mastering. |
| `int i915_ide_dma_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)` | Zapis sektorow przez DMA bus mastering. |
| `int i915_ide_flush(uint8_t channel)` | Wymusza zapis bufora dysku (flush cache). |
| `void i915_ide_reset(uint8_t channel)` | Reset kontrolera IDE przez port alternativ status. |
| `int i915_ide_set_dma(uint8_t channel, uint8_t drive, uint8_t mode)` | Ustawia tryb transferu DMA (UDMA mode). |
| `uint8_t i915_ide_get_udma_mode(uint8_t channel)` | Zwraca aktualny tryb UDMA dla kanalu. |
| `void i915_ide_stats_get(struct i915_ide_stats *out)` | Statystyki IDE (reads, writes, sektory, DMA transfery, bledy). |

### SATA (ICH6)

| funkcja | opis |
|---------|------|
| `int i915_sata_init(void)` | Inicjalizuje kontroler SATA ICH6. |
| `int i915_sata_is_ready(void)` | Zwraca 1 gdy SATA jest gotowe. |
| `uint8_t i915_sata_port_count(void)` | Zwraca liczba portow SATA (4). |
| `int i915_sata_read_sectors(uint8_t port, uint32_t lba, uint8_t count, void *buffer)` | Czytanie sektorow z portu SATA (placeholder). |
| `int i915_sata_write_sectors(uint8_t port, uint32_t lba, uint8_t count, const void *buffer)` | Zapis sektorow na port SATA (placeholder). |
| `int i915_sata_set_mode(uint8_t port, uint8_t ahci_mode)` | Ustawia tryb pracy portu SATA (AHCI/IDE). |

### USB (ICH6 UHCI x3 + EHCI)

| funkcja | opis |
|---------|------|
| `int i915_usb_init(void)` | Wykrywa i inicjalizuje trzy kontrolery UHCI oraz EHCI. |
| `int i915_usb_is_ready(void)` | Zwraca 1 gdy USB jest gotowe. |
| `uint8_t i915_usb_root_port_count(void)` | Zwraca liczba portow roota (6). |
| `int i915_usb_port_reset(uint8_t port)` | Reset portu USB (bit reset w PORTSC). |
| `int i915_usb_port_power(uint8_t port, uint8_t on)` | Wlacza/wylacza zasilanie portu USB. |
| `uint32_t i915_usb_port_status(uint8_t port)` | Zwraca rejestr statusu portu USB. |
| `int i915_usb_control_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data)` | Transfer sterowniczy USB. |
| `int i915_usb_bulk_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint32_t length, void *data)` | Transfer masowy USB. |
| `void i915_usb_stats_get(struct i915_usb_stats *out)` | Statystyki USB (porty, resety, transfery, bledy). |

### Audio AC'97 (ICH6 + kodek)

| funkcja | opis |
|---------|------|
| `int i915_audio_init(void)` | Inicjalizuje kontroler AC'97 i resetuje kodek (cold reset). |
| `int i915_audio_is_ready(void)` | Zwraca 1 gdy audio jest gotowe. |
| `int i915_audio_codec_read(uint8_t reg, uint16_t *value)` | Odczyt rejestru kodeka AC'97. |
| `int i915_audio_codec_write(uint8_t reg, uint16_t value)` | Zapis rejestru kodeka AC'97. |
| `int i915_audio_codec_reset(int cold)` | Cold lub warm reset kodeka AC'97. |
| `int i915_audio_set_volume(uint8_t channel, uint8_t percent)` | Ustawia glosnosc kanalu (0-3: master, master mono, PCM out, PCM in). |
| `uint8_t i915_audio_get_volume(uint8_t channel)` | Zwraca aktualna glosnosc kanalu. |
| `int i915_audio_playback_start(const void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)` | Rozpoczyna odtwarzanie PCM z bufora przez DMA kontrolera BM. |
| `int i915_audio_playback_stop(void)` | Zatrzymuje odtwarzanie PCM. |
| `int i915_audio_capture_start(void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)` | Rozpoczyna nagrywanie PCM do bufora przez DMA. |
| `int i915_audio_capture_stop(void)` | Zatrzymuje nagrywanie PCM. |
| `void i915_audio_stats_get(struct i915_audio_stats *out)` | Statystyki audio (codec_reads, codec_writes, playback_bytes, capture_bytes, errors). |

### SMBus (ICH6)

| funkcja | opis |
|---------|------|
| `int i915_smbus_init(void)` | Wykrywa i inicjalizuje kontroler SMBus ICH6. |
| `int i915_smbus_read_byte(uint8_t device, uint8_t offset, uint8_t *value)` | Odczyt bajtu z urzadzenia SMBus. |
| `int i915_smbus_write_byte(uint8_t device, uint8_t offset, uint8_t value)` | Zapis bajtu do urzadzenia SMBus. |
| `int i915_smbus_read_word(uint8_t device, uint8_t offset, uint16_t *value)` | Odczyt slowa (16 bit) z urzadzenia SMBus. |
| `int i915_smbus_write_word(uint8_t device, uint8_t offset, uint16_t value)` | Zapis slowa (16 bit) do urzadzenia SMBus. |
| `int i915_smbus_spd_read(uint8_t offset, uint8_t *value)` | Odczyt bajtu z modulu pamieci SPD przez SMBus. |
| `int i915_smbus_spd_write(uint8_t offset, uint8_t value)` | Zapis bajtu do modulu pamieci SPD przez SMBus. |
| `void i915_smbus_stats_get(void)` | Aktualizuje licznik operacji SMBus w statystykach. |

### ACPI (ICH6)

| funkcja | opis |
|---------|------|
| `int i915_acpi_available(void)` | Sprawdza dostepnosc ACPI i wlacza SCI (System Control Interrupt). |
| `void i915_acpi_sleep(uint8_t state)` | Wprowadza system w stan uspienia ACPI (S1-S5) przez rejestr PM1A_CNT. |
| `void i915_acpi_reboot(void)` | Restart komputera przez port 0xCF9 i klawiatura. |
| `void i915_acpi_shutdown(void)` | Wylaczenie systemu przez ACPI S5 lub fallback. |
| `int i915_acpi_sci_enable(uint8_t enable)` | Wlacza/wylacza przerwania SCI ACPI. |
| `uint16_t i915_acpi_read_pm1a_cnt(void)` | Odczyt rejestru PM1A_CNT ACPI. |
| `void i915_acpi_write_pm1a_cnt(uint16_t value)` | Zapis rejestru PM1A_CNT ACPI. |

### TCO Watchdog (ICH6)

| funkcja | opis |
|---------|------|
| `int i915_tco_init(void)` | Inicjalizuje TCO watchdog ICH6. |
| `int i915_tco_is_ready(void)` | Zwraca 1 gdy TCO jest gotowe. |
| `int i915_tco_watchdog_start(uint32_t seconds)` | Uruchamia watchdog TCO na podana liczbe sekund. |
| `int i915_tco_watchdog_feed(void)` | Karmi (resetuje licznik) watchdog TCO. |
| `int i915_tco_watchdog_stop(void)` | Zatrzymuje watchdog TCO. |
| `int i915_tco_irq_status(void)` | Zwraca status przerwan TCO. |

### GPIO

| funkcja | opis |
|---------|------|
| `int i915_gpio_read(uint8_t pin, uint8_t *value)` | Odczyt stanu pinu GPIO. |
| `int i915_gpio_write(uint8_t pin, uint8_t value)` | Zapis stanu pinu GPIO (0/1). |
| `int i915_gpio_direction(uint8_t pin, uint8_t output)` | Ustawia kierunek pinu GPIO (wejscie/wyjscie). |

### PIRQ i SMI

| funkcja | opis |
|---------|------|
| `int i915_pirq_read_router(uint8_t pirq, uint8_t *route)` | Odczyt trasy PIRQ (PIRQA-PIRQD). |
| `int i915_pirq_write_router(uint8_t pirq, uint8_t route)` | Zapis trasy PIRQ. |
| `int i915_smi_enable(uint8_t enable)` | Wlacza/wylacza System Management Interrupt. |
| `uint8_t i915_smi_status(void)` | Zwraca status SMI. |

### FWH / BIOS lock

| funkcja | opis |
|---------|------|
| `int i915_fwh_read(uint32_t offset, uint8_t *value)` | Odczyt bajtu z pamieci FWH (firmware hub). |
| `int i915_fwh_write(uint32_t offset, uint8_t value)` | Zapis bajtu do pamieci FWH. |
| `int i915_bios_lock(void)` | Blokuje zapisy do obszaru BIOS (ustawia bit lock w ICH6). |
| `int i915_bios_unlock(void)` | Odblokowuje zapisy do obszaru BIOS. |

### RTC / CMOS

| funkcja | opis |
|---------|------|
| `int i915_rtc_init(void)` | Inicjalizuje RTC (tryb 24h, format binarny). |
| `int i915_rtc_is_ready(void)` | Zwraca 1 gdy RTC jest gotowe. |
| `int i915_rtc_get_time(struct i915_rtc_time *out)` | Pobiera aktualny czas i date z RTC. |
| `int i915_rtc_set_time(const struct i915_rtc_time *t)` | Ustawia czas i date w RTC. |
| `int i915_rtc_nvram_read(uint8_t offset, uint8_t *value)` | Odczyt bajtu z pamieci CMOS/NVRAM. |
| `int i915_rtc_nvram_write(uint8_t offset, uint8_t value)` | Zapis bajtu do pamieci CMOS/NVRAM. |
| `int i915_rtc_alarm_set(const struct i915_rtc_time *t)` | Ustawia alarm RTC. |
| `int i915_rtc_irq_enable(uint8_t mask, uint8_t enable)` | Wlacza/wylacza przerwania RTC (update, alarm, periodic). |
| `uint8_t i915_rtc_irq_status(void)` | Zwraca rejestr statusu C RTC (flagi przerwan). |

### Monitorowanie sprzetowe (HWMON, Super I/O)

| funkcja | opis |
|---------|------|
| `int i915_hwmon_init(void)` | Inicjalizuje Super I/O i wlacza logiczne urzadzenie HWMON. |
| `int i915_hwmon_is_ready(void)` | Zwraca 1 gdy HWMON jest gotowe. |
| `int i915_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius)` | Odczyt temperatury z czujnika w milicelcjuszach. |
| `int i915_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt)` | Odczyt napiecia (placeholder). |
| `int i915_hwmon_read_fan(uint8_t fan, uint32_t *rpm)` | Odczyt obrotow wentylatora (placeholder). |
| `void i915_hwmon_stats_get(struct i915_hwmon_stats *out)` | Statystyki HWMON (temperatury, napiecia, wentylatory, bledy). |

### Obsluga przerwan (IRQ)

| funkcja | opis |
|---------|------|
| `void i915_irq_handler(uint8_t irq)` | Dysponuje przerwanie do zarejestrowanego handlera dla danego IRQ. |
| `int i915_irq_register(uint8_t irq, void (*handler)(uint8_t irq))` | Rejestruje handler przerwania dla IRQ 0-15. |
| `int i915_irq_unregister(uint8_t irq)` | Wyrejestrowuje handler przerwania dla danego IRQ. |

### Struktury danych

Struktura `i915_info` zawiera:

| pole | opis |
|------|------|
| vendor_id | id dostawcy (0x8086 Intel) |
| mch_device_id | id urzadzenia MCH (chipset norte) |
| ich_device_id | id urzadzenia ICH6 (chipset polnocny) |
| graphics_device_id | id urzadzenia graficznego GMA 900 |
| mch_revision | rewizja MCH |
| ich_revision | rewizja ICH6 |
| graphics_revision | rewizja grafiki |
| acpi_base | adres bazowy ACPI PM |
| smbus_base | adres bazowy SMBus |
| ac97_mixer_base | adres bazowy miksera AC'97 |
| ac97_bm_base | adres bazowy bus mastering AC'97 |
| ide_base[2] | adresy bazowe kanalow IDE |
| bm_ide_base[2] | adresy bazowe bus mastering IDE |
| sata_base | adres bazowy SATA |
| uhci_base[4] | adresy bazowe kontrolerow UHCI |
| ehci_base | adres bazowy kontrolera EHCI |
| eth_base | adres bazowy ethernetu |
| floppy_base | adres bazowy FDC |
| agp_aperture | rozmiar aperture AGP |
| gfx_mmio_base | adres bazowy MMIO grafiki |
| gfx_fb_addr | adres ramki graficznej |
| dram | informacje o pamieci DRAM |
| graphics | informacje o urzadzeniu graficznym |
| ide | informacje o kontrolerze IDE |
| usb | informacje o kontrolerze USB |
| audio | informacje o kontrolerze audio |
| acpi | informacje o kontrolerze ACPI |

Struktura `i915_stats` zawiera:

| pole | opis |
|------|------|
| audio | statystyki audio AC'97 |
| eth | statystyki ethernetu |
| ide | statystyki IDE |
| usb | statystyki USB |
| floppy | statystyki FDC |
| hwmon | statystyki monitorowania sprzetowego |
| graphics | statystyki grafiki GMA 900 |
| pci_reads | liczba odczytow konfiguracji PCI |
| pci_writes | liczba zapisow konfiguracji PCI |
| memory_ops | liczba operacji na pamieci chipsetu |
| smbus_ops | liczba operacji SMBus |
| acpi_events | liczba zdarzen ACPI |
| tco_resets | liczba resetow TCO watchdog |
| irq_dispatched | liczba dyspatchowanych przerwan |
| init_count | liczba wyolan inicjalizacji |
| errors | liczba bledow sterownika |

### Integracja z jadrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `p4p_init()`:

```
i915_init();
```

Naglowek `drivers/plyty_glowne/intel_915.h` jest dolaczany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywolywane z kodu C jadra. Plik zrodlowy `drivers/plyty_glowne/intel_915.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/plyty_glowne/*.cpp)`).

## karta graficzna - drivers/karty_graficzne/radeon_rx_9070_xt.cpp

Zaawansowany sterownik karty graficznej Gigabyte Radeon RX 9070 XT GAMING OC 16GB GDDR6 (architektura RDNA 4 / Navi 48) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP GFX, CP COMPUTE, CP DMA), ray tracing (RT), mesh shaders, variable rate shading (VRS), delta color compression (DCC), przezroczystosc (alpha blending, alpha to coverage), tekstury z mipmapami i kompresja (ASTC, DXT), shadery (vertex, pixel, geometry, compute, ray gen, any hit, closest hit, miss, callable, intersection, mesh, amplification), render targety, depth-stencil, fences, semaphory, command buffer oraz presentation (swap chain, vsync, HDR, triple buffer). Sterownik zarzadza 16 GB pamieci GDDR6 (VRAM allocator), taktowaniem GPU i pamieci, temperatura, wentylatorem oraz sterowaniem RGB. Eksportuje interfejs C przez naglowek radeon_rx_9070_xt.h i jest inicjalizowany z kernel/kernel.c przez `rx_9070_xt_init()`.

### Inicjalizacja i stan

| funkcja | opis |
|---------|------|
| `int rx_9070_xt_init(void)` | Inicjalizuje sterownik Radeon RX 9070 XT: wyszukuje urzadzenie na PCI (vendor 0x1002, Navi 48 XT), mapuje adresy MMIO, czysci przerwania, konfiguruje zasilanie (D0), zegary GPU i pamieci, inicjalizuje silnik wyswietlania DCN (CRTC, plane, surface, 1920x1080, FreeSync, HDR), resetuje i inicjalizuje Command Processor (ring buffory GFX/COMPUTE/DMA), konfiguruje potok 3D (color buffer, depth buffer, blend, rasterizer, shader SQ), inicjalizuje kursor sprzetowy. Zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie. |
| `int rx_9070_xt_is_ready(void)` | Zwraca 1 gdy sterownik zostal zainicjalizowany i jest gotowy. |
| `void rx_9070_xt_shutdown(void)` | Zatrzymuje Command Processor (ME i PFP Halt), przechodzi w stan zasilania D3 i oznacza sterownik jako nieinicjalizowany. |

### Podstawowa grafika 2D

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_putpixel(uint32_t x, uint32_t y, uint32_t color)` | Rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem. |
| `void rx_9070_xt_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)` | Wypelnia prostokat o podanym rozmiarze i kolorze. Uzywa ring buffer CP GFX gdy sterownik jest zainicjowany, w przeciwnym razie petla CPU. |
| `void rx_9070_xt_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)` | Kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) uzywajac ring buffer CP GFX sprzetowego. |
| `void rx_9070_xt_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)` | Ustawia kursor sprzetowy na pozycji (x, y) z obrazem 64x64 (kopiowany do pamieci ramki graficznej). |
| `void rx_9070_xt_cursor_hide(void)` | Ukrywa kursor sprzetowy. |
| `void *rx_9070_xt_get_fb(void)` | Zwraca wskaznik na ramke graficzna (framebuffer). |
| `uint32_t rx_9070_xt_get_width(void)` | Zwraca szerokosc ekranu (1920). |
| `uint32_t rx_9070_xt_get_height(void)` | Zwraca wysokosc ekranu (1080). |
| `uint16_t rx_9070_xt_get_device_id(void)` | Zwraca ID urzadzenia graficznego. |
| `uint8_t rx_9070_xt_get_revision(void)` | Zwraca rewizje karty graficznej. |
| `void rx_9070_xt_info_get(struct rx_9070_xt_info *out)` | Pobiera pelne informacje o urzadzeniu (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized, architecture, vram_size, vram_used, gfx_clock_mhz, memory_clock_mhz, temperature_c, fan_rpm). |

### Zarzadzanie pamiecia VRAM (16 GB GDDR6)

| funkcja | opis |
|---------|------|
| `uint64_t rx_9070_xt_vram_alloc(uint64_t size, uint32_t type)` | Alokuje obszar pamieci VRAM o podanym rozmiarze i typie (wyrownanie do strony 4 KB). Zwraca offset w VRAM lub 0xFFFFFFFFFFFFFFFF przy bledzie. |
| `void rx_9070_xt_vram_free(uint64_t handle)` | Zwalnia obszar pamieci VRAM o podanym offsecie (oznacza slot jako wolny). |
| `uint64_t rx_9070_xt_vram_used(void)` | Zwraca liczbe bajtow VRAM aktualnie uzytych. |
| `uint64_t rx_9070_xt_vram_total(void)` | Zwraca calkowita pojemnosc VRAM (16 GB). |

### Tworzenie i niszczenie zasobow 3D

| funkcja | opis |
|---------|------|
| `rx_9070_xt_handle_t rx_9070_xt_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)` | Tworzy vertex buffer (wierzcholki). Zwraca handle lub RX_9070_XT_HANDLE_NULL przy bledzie. |
| `void rx_9070_xt_vertex_buffer_destroy(rx_9070_xt_handle_t handle)` | Niszczy vertex buffer o podanym handle i zwalnia jego pamiec VRAM. |
| `void rx_9070_xt_vertex_buffer_update(rx_9070_xt_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane vertex buffera od podanego offsetu. |
| `rx_9070_xt_handle_t rx_9070_xt_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)` | Tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe). Zwraca handle. |
| `void rx_9070_xt_index_buffer_destroy(rx_9070_xt_handle_t handle)` | Niszczy index buffer i zwalnia jego pamiec VRAM. |
| `void rx_9070_xt_index_buffer_update(rx_9070_xt_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane index buffera od podanego offsetu. |
| `rx_9070_xt_handle_t rx_9070_xt_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)` | Tworzy teksture 2D o podanym rozmiarze, formacie i liczbie poziomow mipmap. Zwraca handle. |
| `void rx_9070_xt_texture_destroy(rx_9070_xt_handle_t handle)` | Niszczy teksture i zwalnia jej pamiec VRAM. |
| `void rx_9070_xt_texture_update(rx_9070_xt_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)` | Aktualizuje zawartosc tekstury (upload z pamieci systemowej do VRAM). |
| `void rx_9070_xt_texture_generate_mipmaps(rx_9070_xt_handle_t handle)` | Generuje mipmapy dla tekstury (powtarzajace polowowanie wymiarow). |
| `rx_9070_xt_handle_t rx_9070_xt_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend, uint8_t dcc)` | Tworzy render target (bufor koloru do renderowania 3D z obsluga przezroczystosci alpha i DCC). Zwraca handle. |
| `void rx_9070_xt_render_target_destroy(rx_9070_xt_handle_t handle)` | Niszczy render target i zwalnia jego pamiec VRAM. |
| `rx_9070_xt_handle_t rx_9070_xt_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression)` | Tworzy depth-stencil buffer (bufor glebi i stencil z opcjonalna kompresja). Zwraca handle. |
| `void rx_9070_xt_depth_stencil_destroy(rx_9070_xt_handle_t handle)` | Niszczy depth-stencil i zwalnia jego pamiec VRAM. |
| `rx_9070_xt_handle_t rx_9070_xt_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)` | Tworzy shader (vertex/pixel/geometry/compute/ray/mesh/amplification) z kodu bajtowego GPU. Zwraca handle. |
| `void rx_9070_xt_shader_destroy(rx_9070_xt_handle_t handle)` | Niszczy shader. |
| `void rx_9070_xt_shader_compile(rx_9070_xt_handle_t handle)` | Oznacza shader jako skompilowany (gotowy do uzycia w potoku). |
| `rx_9070_xt_handle_t rx_9070_xt_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable, uint8_t compare_func)` | Tworzy stan probki tekstury (filtrowanie min/mag/mip, adresowanie, anisotropia, porownanie). Zwraca handle. |
| `void rx_9070_xt_sampler_state_destroy(rx_9070_xt_handle_t handle)` | Niszczy stan probki tekstury. |
| `rx_9070_xt_handle_t rx_9070_xt_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor)` | Tworzy stan blendowania (przezroczystosc alpha, operacje blendowania, staly kolor blendowania). Zwraca handle. |
| `void rx_9070_xt_blend_state_destroy(rx_9070_xt_handle_t handle)` | Niszczy stan blendowania. |
| `rx_9070_xt_handle_t rx_9070_xt_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage, uint8_t vrs)` | Tworzy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygladzone, alpha to coverage, VRS). Zwraca handle. |
| `void rx_9070_xt_rasterizer_state_destroy(rx_9070_xt_handle_t handle)` | Niszczy stan rasterizera. |
| `rx_9070_xt_handle_t rx_9070_xt_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)` | Tworzy stan depth-stencil (testy glebi i stencil z przodem i tylem). Zwraca handle. |
| `void rx_9070_xt_depth_stencil_state_destroy(rx_9070_xt_handle_t handle)` | Niszczy stan depth-stencil. |
| `rx_9070_xt_handle_t rx_9070_xt_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)` | Tworzy input layout (mapowanie atrybutow vertex do slotow). Zwraca handle. |
| `void rx_9070_xt_input_layout_destroy(rx_9070_xt_handle_t handle)` | Niszczy input layout. |
| `rx_9070_xt_handle_t rx_9070_xt_constant_buffer_create(uint32_t size)` | Tworzy constant buffer (stale shadera / uniforms) z pamiecia VRAM. Zwraca handle. |
| `void rx_9070_xt_constant_buffer_destroy(rx_9070_xt_handle_t handle)` | Niszczy constant buffer i zwalnia jego pamiec VRAM. |
| `void rx_9070_xt_constant_buffer_update(rx_9070_xt_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane constant buffera od podanego offsetu. |
| `rx_9070_xt_handle_t rx_9070_xt_acceleration_structure_create(uint32_t geometry_count, const uint32_t *vertex_buffers, const uint32_t *index_buffers, const uint32_t *vertex_counts, const uint32_t *index_counts)` | Tworzy strukture przyspieszania ray tracingu (BVH) z podanych buforow wierzcholkow i indeksow. Zwraca handle. |
| `void rx_9070_xt_acceleration_structure_destroy(rx_9070_xt_handle_t handle)` | Niszczy strukture przyspieszania ray tracingu. |
| `void rx_9070_xt_acceleration_structure_build(rx_9070_xt_handle_t handle)` | Buduje strukture przyspieszania ray trackingu na GPU (emit polecenia do ring buffer CP GFX). |

### Bindowanie stanow potoku 3D

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_bind_vertex_buffer(rx_9070_xt_handle_t handle)` | Binduje vertex buffer do potoku 3D. |
| `void rx_9070_xt_bind_index_buffer(rx_9070_xt_handle_t handle)` | Binduje index buffer do potoku 3D. |
| `void rx_9070_xt_bind_input_layout(rx_9070_xt_handle_t handle)` | Binduje input layout do potoku 3D. |
| `void rx_9070_xt_bind_vertex_shader(rx_9070_xt_handle_t handle)` | Binduje shader vertex do potoku 3D. |
| `void rx_9070_xt_bind_pixel_shader(rx_9070_xt_handle_t handle)` | Binduje shader pixel do potoku 3D. |
| `void rx_9070_xt_bind_geometry_shader(rx_9070_xt_handle_t handle)` | Binduje shader geometry do potoku 3D. |
| `void rx_9070_xt_bind_compute_shader(rx_9070_xt_handle_t handle)` | Binduje shader compute do potoku 3D. |
| `void rx_9070_xt_bind_ray_gen_shader(rx_9070_xt_handle_t handle)` | Binduje shader ray generation do potoku ray tracingu. |
| `void rx_9070_xt_bind_any_hit_shader(rx_9070_xt_handle_t handle)` | Binduje shader any-hit do potoku ray tracingu. |
| `void rx_9070_xt_bind_closest_hit_shader(rx_9070_xt_handle_t handle)` | Binduje shader closest-hit do potoku ray tracingu. |
| `void rx_9070_xt_bind_miss_shader(rx_9070_xt_handle_t handle)` | Binduje shader miss do potoku ray tracingu. |
| `void rx_9070_xt_bind_callable_shader(rx_9070_xt_handle_t handle)` | Binduje shader callable do potoku ray tracingu. |
| `void rx_9070_xt_bind_intersection_shader(rx_9070_xt_handle_t handle)` | Binduje shader intersection do potoku ray tracingu. |
| `void rx_9070_xt_bind_mesh_shader(rx_9070_xt_handle_t handle)` | Binduje shader mesh do potoku 3D. |
| `void rx_9070_xt_bind_amplification_shader(rx_9070_xt_handle_t handle)` | Binduje shader amplification do potoku 3D. |
| `void rx_9070_xt_bind_render_target(rx_9070_xt_handle_t handle)` | Binduje render target do potoku 3D. |
| `void rx_9070_xt_bind_depth_stencil(rx_9070_xt_handle_t handle)` | Binduje depthstencil do potoku 3D. |
| `void rx_9070_xt_bind_blend_state(rx_9070_xt_handle_t handle)` | Binduje stan blendowania do potoku 3D. |
| `void rx_9070_xt_bind_rasterizer_state(rx_9070_xt_handle_t handle)` | Binduje stan rasterizera do potoku 3D. |
| `void rx_9070_xt_bind_depth_stencil_state(rx_9070_xt_handle_t handle)` | Binduje stan depth-stencil do potoku 3D. |
| `void rx_9070_xt_bind_acceleration_structure(rx_9070_xt_handle_t handle)` | Binduje strukture przyspieszania ray tracingu do potoku. |
| `void rx_9070_xt_bind_sampler_and_texture(uint32_t slot, rx_9070_xt_handle_t sampler, rx_9070_xt_handle_t texture)` | Binduje probke tekstury i teksture do slotu. |
| `void rx_9070_xt_bind_constant_buffer(uint32_t slot, rx_9070_xt_handle_t handle)` | Binduje constant buffer do slotu. |

### Konfiguracja potoku 3D

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)` | Ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi. |
| `void rx_9070_xt_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)` | Ustawia prostokat przyciecia renderowania (scissor). |
| `void rx_9070_xt_set_primitive_topology(uint32_t topology)` | Ustawia typ primitivu do rysowania (punkt, linia, trojkat, kwadrat itp.). |
| `void rx_9070_xt_set_alpha_blend_global(uint8_t enable)` | Wlacza/wylacza globalne blendowanie alfa (przezroczystosc) dla calego potoku 3D. |
| `void rx_9070_xt_set_ray_tracing(uint8_t enable)` | Wlacza/wylacza ray tracing (RT) w potoku graficznym. |
| `void rx_9070_xt_set_mesh_shader(uint8_t enable)` | Wlacza/wylacza mesh shaders w potoku graficznym. |
| `void rx_9070_xt_set_variable_rate_shading(uint8_t enable)` | Wlacza/wylacza variable rate shading (VRS). |
| `void rx_9070_xt_set_delta_color_compression(uint8_t enable)` | Wlacza/wylacza delta color compression (DCC) dla render targetow. |
| `void rx_9070_xt_set_hdr(uint8_t enable)` | Wlacza/wylacza HDR (High Dynamic Range) na wyjsciu display. |
| `void rx_9070_xt_set_freesync(uint8_t enable)` | Wlacza/wylacza FreeSync (zmienna czestotliwosc odswiezania) na CRTC. |

### Rysowanie 3D

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_clear_render_target(rx_9070_xt_handle_t rt, uint32_t color)` | Czysci render target podanym kolorem. |
| `void rx_9070_xt_clear_depth_stencil(rx_9070_xt_handle_t ds, float depth, uint8_t stencil)` | Czysci depth-stencil podanymi wartosciami glebi i stencil. |
| `void rx_9070_xt_draw(uint32_t vertex_count, uint32_t start_vertex)` | Rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex. |
| `void rx_9070_xt_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)` | Rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex. |
| `void rx_9070_xt_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)` | Rysuje indeksowane primitivy z instancjami - index_count indeksow, instance_count instancji poczynajac od start_instance. |
| `void rx_9070_xt_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)` | Rysuje z parametrami pobranymi z bufora indirect (draw indirect). |

### Compute i ray tracing

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)` | Uruchamia obliczenia compute (dispatch) z podana liczba grup watkow w X, Y, Z. |
| `void rx_9070_xt_compute_dispatch_indirect(uint32_t buffer_addr)` | Uruchamia obliczenia compute z parametrami z bufora indirect. |
| `void rx_9070_xt_ray_trace(uint32_t width, uint32_t height, uint32_t depth, uint32_t accel_struct, uint32_t ray_gen_shader)` | Uruchamia ray tracing (rzucone promienie) z podana struktura przyspieszania i shaderem ray gen. |
| `void rx_9070_xt_mesh_draw(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)` | Rysuje uzywajac mesh shaders (mesh draw z podana liczba grup watkow). |

### Synchronizacja (fences, semaphory)

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_fence_create(rx_9070_xt_handle_t *out)` | Tworzy fence (synchronizacja GPU) i zwraca jego handle. |
| `void rx_9070_xt_fence_signal(rx_9070_xt_handle_t handle, uint32_t value)` | Sygnalizuje fence z podana wartoscia (oznacza zakonczenie pracy GPU). |
| `int rx_9070_xt_fence_wait(rx_9070_xt_handle_t handle, uint32_t value, uint32_t timeout_us)` | Czeka na sygnalizacje fence do podanej wartosci z timeoutem w mikrosekundach. Zwraca 1 gdy fence zostal sygnalizowany, 0 przy timeoutcie. |
| `void rx_9070_xt_fence_destroy(rx_9070_xt_handle_t handle)` | Niszczy fence o podanym handle. |
| `void rx_9070_xt_semaphore_create(rx_9070_xt_handle_t *out)` | Tworzy semafor (synchronizacja miedzy etapami potoku) i zwraca jego handle. |
| `void rx_9070_xt_semaphore_signal(rx_9070_xt_handle_t handle, uint32_t value)` | Sygnalizuje semafor z podana wartoscia. |
| `void rx_9070_xt_semaphore_wait(rx_9070_xt_handle_t handle, uint32_t value)` | Czeka na sygnalizacje semafora do podanej wartosci (blokujace). |
| `void rx_9070_xt_semaphore_destroy(rx_9070_xt_handle_t handle)` | Niszczy semafor o podanym handle. |

### Zarzadzanie zasilaniem, zegarami, temperatura, wentylator, RGB

| funkcja | opis |
|---------|------|
| `int rx_9070_xt_wait_idle(uint32_t timeout_us)` | Czeka az GPU bedzie bzczynne (ring buffer pusty) z timeoutem. Zwraca 1 gdy GPU jest idle, 0 przy timeoutcie. |
| `uint64_t rx_9070_xt_timestamp(void)` | Pobiera aktualny znacznik czasu z rejestru timestamp GPU. |
| `void rx_9070_xt_power_set_state(uint8_t state)` | Ustawia stan zasilania GPU (D0-D3). |
| `uint8_t rx_9070_xt_power_get_state(void)` | Zwraca aktualny stan zasilania GPU. |
| `void rx_9070_xt_set_clock_gfx(uint32_t mhz)` | Ustawia taktowanie rdzenia grafiki (GFX clock) w MHz. |
| `void rx_9070_xt_set_clock_memory(uint32_t mhz)` | Ustawia taktowanie pamieci (memory clock) w MHz. |
| `uint32_t rx_9070_xt_get_clock_gfx(void)` | Zwraca aktualne taktowanie rdzenia grafiki w MHz. |
| `uint32_t rx_9070_xt_get_clock_memory(void)` | Zwraca aktualne taktowanie pamieci w MHz. |
| `uint32_t rx_9070_xt_get_temperature(void)` | Pobiera aktualna temperature GPU z czujnika termicznego (w stopniach Celsjusza). |
| `void rx_9070_xt_set_fan_speed(uint32_t rpm)` | Ustawia predkosc wentylatora GPU w obrotach na minute (RPM). |
| `uint32_t rx_9070_xt_get_fan_speed(void)` | Zwraca aktualna predkosc wentylatora GPU w RPM. |
| `void rx_9070_xt_rgb_set(uint32_t color, uint8_t mode)` | Ustawia sterowanie RGB na karcie (kolor i tryb swiecenia). |
| `void rx_9070_xt_rgb_off(void)` | Wylacza sterowanie RGB na karcie. |

### Funkcje sprawdzania obslugi cech (feature detection)

| funkcja | opis |
|---------|------|
| `int rx_9070_xt_feature_ray_tracing(void)` | Zwraca 1 gdy ray tracing jest wspierany i wlaczony. |
| `int rx_9070_xt_feature_mesh_shader(void)` | Zwraca 1 gdy mesh shaders sa wspierane i wlaczone. |
| `int rx_9070_xt_feature_variable_rate_shading(void)` | Zwraca 1 gdy variable rate shading jest wspierany i wlaczony. |
| `int rx_9070_xt_feature_sampler_feedback(void)` | Zwraca 1 gdy sampler feedback jest wspierany. |
| `int rx_9070_xt_feature_hdr(void)` | Zwraca 1 gdy HDR jest wspierany i wlaczony. |
| `int rx_9070_xt_feature_freesync(void)` | Zwraca 1 gdy FreeSync jest wspierany i wlaczony. |
| `int rx_9070_xt_feature_delta_color_compression(void)` | Zwraca 1 gdy delta color compression jest wspierane i wlaczone. |

### Presentation (swap chain)

| funkcja | opis |
|---------|------|
| `rx_9070_xt_handle_t rx_9070_xt_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied, uint8_t hdr, uint8_t triple_buffer)` | Tworzy swap chain (podwojne/trojne buforowanie z opcja vsync, premultiplied alpha i HDR). Zwraca handle. |
| `void rx_9070_xt_swap_chain_destroy(rx_9070_xt_handle_t handle)` | Niszczy swap chain i zwalnia pamiec VRAM bufora tylnego. |
| `void rx_9070_xt_present(rx_9070_xt_handle_t chain)` | Przebija tylny bufor na przedni (present) z oczekiwaniem na vsync (opcjonalnie) przez ring buffer CP GFX. |

### Command buffer

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_command_begin(void)` | Rozpoczyna nagrywanie polecen do command buffer. |
| `void rx_9070_xt_command_end(void)` | Zakoncza nagrywanie i wykonuje wszystkie polecenia command buffer przez ring buffer CP GFX. |

### Informacje o akceleracji 3D

| funkcja | opis |
|---------|------|
| `void rx_9070_xt_context_get(struct rx_9070_xt_context *out)` | Pobiera biezacy kontekst 3D (stan potoku). |
| `int rx_9070_xt_3d_available(void)` | Zwraca 1 gdy akceleracja 3D jest dostepna (sterownik zainicjowany). |

### Stale i typy

| nazwa | opis |
|-------|------|
| `RX_9070_XT_VENDOR_ID` | ID dostawcy AMD (0x1002). |
| `RX_9070_XT_DEVICE_ID_NAVI48_XT` | ID urzadzenia Navi 48 XT (0x747F) oraz warianty 0x747E/0x747D/0x747C/0x747B. |
| `RX_9070_XT_VRAM_SIZE_BYTES` | Calkowita pojemnosc VRAM (16 GB GDDR6). |
| `RX_9070_XT_FB_WIDTH / RX_9070_XT_FB_HEIGHT` | Rozdzielczosc ramki graficznej (1920x1080). |
| `RX_9070_XT_FB_PITCH` | Pitch ramki graficznej (7680 bajtow). |
| `RX_9070_XT_HANDLE_NULL` | Wartosc nulowa dla handle zasobow (0xFFFFFFFFU). |
| `rx_9070_xt_handle_t` | Typ handle zasobu (uint32_t). |
| `RX_9070_XT_*` | Stale dla formatow vertex, indeksow, primitivow, tekstur, shaderow, blendowania, operacji blendowania, porownan glebi, wypelnienia, cullowania, operacji stencil (patrz naglowek radeon_rx_9070_xt.h). |

### Integracja z jadrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `radeon_hd_8370d_init()`:

```
rx_9070_xt_init();
```

Naglowek `drivers/karty_graficzne/radeon_rx_9070_xt.h` jest dolaczany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywolywane z kodu C jadra. Plik zrodlowy `drivers/karty_graficzne/radeon_rx_9070_xt.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/karty_graficzne/*.cpp)`).

## karta graficzna - drivers/karty_graficzne/radeon_hd_6870.cpp

Zaawansowany sterownik karty graficznej AMD Radeon HD 6870 (architektura Northern Islands / Cayman, VLIW4) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP), przezroczystosc (alpha blending), tekstury z mipmapami i kompresja (DXT), shadery (vertex, pixel, geometry, compute), render targety, depth-stencil, fences, semaphory, command buffer oraz presentation (swap chain, vsync). Sterownik zarzadza pamiecia GDDR5 (VRAM allocator), taktowaniem GPU i pamieci, temperatura, wentylatorem oraz sterowaniem RGB. Eksportuje interfejs C przez naglowek radeon_hd_6870.h i jest inicjalizowany z kernel/kernel.c przez `radeon_6870_init()`.

### Inicjalizacja i stan

| funkcja | opis |
|---------|------|
| `int radeon_6870_init(void)` | Inicjalizuje sterownik Radeon HD 6870: wyszukuje urzadzenie na PCI (vendor 0x1002, Cayman), mapuje adresy MMIO, czysci przerwania, konfiguruje zasilanie (D0), zegary GPU i pamieci, inicjalizuje silnik wyswietlania DCN (CRTC, plane, surface, 1920x1080), resetuje i inicjalizuje Command Processor (ring buffer), konfiguruje potok 3D (color buffer, depth buffer, blend, rasterizer, shader SQ), inicjalizuje kursor sprzetowy. Zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie. |
| `int radeon_6870_is_ready(void)` | Zwraca 1 gdy sterownik zostal zainicjalizowany i jest gotowy. |
| `void radeon_6870_shutdown(void)` | Zatrzymuje Command Processor (ME i PFP Halt), przechodzi w stan zasilania D3 i oznacza sterownik jako nieinicjalizowany. |

### Podstawowa grafika 2D

| funkcja | opis |
|---------|------|
| `void radeon_6870_putpixel(uint32_t x, uint32_t y, uint32_t color)` | Rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem. |
| `void radeon_6870_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)` | Wypelnia prostokat o podanym rozmiarze i kolorze. Uzywa ring buffer CP gdy sterownik jest zainicjowany, w przeciwnym razie petla CPU. |
| `void radeon_6870_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)` | Kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) uzywajac ring buffer CP sprzetowego. |
| `void radeon_6870_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)` | Ustawia kursor sprzetowy na pozycji (x, y) z obrazem 64x64 (kopiowany do pamieci ramki graficznej). |
| `void radeon_6870_cursor_hide(void)` | Ukrywa kursor sprzetowy. |
| `void *radeon_6870_get_fb(void)` | Zwraca wskaznik na ramke graficzna (framebuffer). |
| `uint32_t radeon_6870_get_width(void)` | Zwraca szerokosc ekranu (1920). |
| `uint32_t radeon_6870_get_height(void)` | Zwraca wysokosc ekranu (1080). |
| `uint16_t radeon_6870_get_device_id(void)` | Zwraca ID urzadzenia graficznego. |
| `uint8_t radeon_6870_get_revision(void)` | Zwraca rewizje karty graficznej. |
| `void radeon_6870_info_get(struct radeon_6870_info *out)` | Pobiera pelne informacje o urzadzeniu (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized, architecture, vram_size, vram_used, gfx_clock_mhz, memory_clock_mhz, temperature_c, fan_rpm). |

### Zarzadzanie pamiecia VRAM (GDDR5)

| funkcja | opis |
|---------|------|
| `uint64_t radeon_6870_vram_alloc(uint64_t size, uint32_t type)` | Alokuje obszar pamieci VRAM o podanym rozmiarze i typie (wyrownanie do strony 4 KB). Zwraca offset w VRAM lub 0xFFFFFFFFFFFFFFFF przy bledzie. |
| `void radeon_6870_vram_free(uint64_t handle)` | Zwalnia obszar pamieci VRAM o podanym offsecie (oznacza slot jako wolny). |
| `uint64_t radeon_6870_vram_used(void)` | Zwraca liczbe bajtow VRAM aktualnie uzytych. |
| `uint64_t radeon_6870_vram_total(void)` | Zwraca calkowita pojemnosc VRAM. |

### Tworzenie i niszczenie zasobow 3D

| funkcja | opis |
|---------|------|
| `radeon_6870_handle_t radeon_6870_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)` | Tworzy vertex buffer (wierzcholki). Zwraca handle lub RADEON_6870_HANDLE_NULL przy bledzie. |
| `void radeon_6870_vertex_buffer_destroy(radeon_6870_handle_t handle)` | Niszczy vertex buffer o podanym handle i zwalnia jego pamiec VRAM. |
| `void radeon_6870_vertex_buffer_update(radeon_6870_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane vertex buffera od podanego offsetu. |
| `radeon_6870_handle_t radeon_6870_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)` | Tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe). Zwraca handle. |
| `void radeon_6870_index_buffer_destroy(radeon_6870_handle_t handle)` | Niszczy index buffer i zwalnia jego pamiec VRAM. |
| `void radeon_6870_index_buffer_update(radeon_6870_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane index buffera od podanego offsetu. |
| `radeon_6870_handle_t radeon_6870_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)` | Tworzy teksture 2D o podanym rozmiarze, formacie i liczbie poziomow mipmap. Zwraca handle. |
| `void radeon_6870_texture_destroy(radeon_6870_handle_t handle)` | Niszczy teksture i zwalnia jej pamiec VRAM. |
| `void radeon_6870_texture_update(radeon_6870_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)` | Aktualizuje zawartosc tekstury (upload z pamieci systemowej do VRAM). |
| `void radeon_6870_texture_generate_mipmaps(radeon_6870_handle_t handle)` | Generuje mipmapy dla tekstury (powtarzajace polowowanie wymiarow). |
| `radeon_6870_handle_t radeon_6870_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend)` | Tworzy render target (bufor koloru do renderowania 3D z obsluga przezroczystosci alpha). Zwraca handle. |
| `void radeon_6870_render_target_destroy(radeon_6870_handle_t handle)` | Niszczy render target i zwalnia jego pamiec VRAM. |
| `radeon_6870_handle_t radeon_6870_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression)` | Tworzy depth-stencil buffer (bufor glebi i stencil z opcjonalna kompresja). Zwraca handle. |
| `void radeon_6870_depth_stencil_destroy(radeon_6870_handle_t handle)` | Niszczy depth-stencil i zwalnia jego pamiec VRAM. |
| `radeon_6870_handle_t radeon_6870_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)` | Tworzy shader (vertex/pixel/geometry/compute) z kodu bajtowego GPU. Zwraca handle. |
| `void radeon_6870_shader_destroy(radeon_6870_handle_t handle)` | Niszczy shader. |
| `void radeon_6870_shader_compile(radeon_6870_handle_t handle)` | Oznacza shader jako skompilowany (gotowy do uzycia w potoku). |
| `radeon_6870_handle_t radeon_6870_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable, uint8_t compare_func)` | Tworzy stan probki tekstury (filtrowanie min/mag/mip, adresowanie, anisotropia, porownanie). Zwraca handle. |
| `void radeon_6870_sampler_state_destroy(radeon_6870_handle_t handle)` | Niszczy stan probki tekstury. |
| `radeon_6870_handle_t radeon_6870_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor)` | Tworzy stan blendowania (przezroczystosc alpha, operacje blendowania, staly kolor blendowania). Zwraca handle. |
| `void radeon_6870_blend_state_destroy(radeon_6870_handle_t handle)` | Niszczy stan blendowania. |
| `radeon_6870_handle_t radeon_6870_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage)` | Tworzy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygladzone, alpha to coverage). Zwraca handle. |
| `void radeon_6870_rasterizer_state_destroy(radeon_6870_handle_t handle)` | Niszczy stan rasterizera. |
| `radeon_6870_handle_t radeon_6870_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)` | Tworzy stan depth-stencil (testy glebi i stencil z przodem i tylem). Zwraca handle. |
| `void radeon_6870_depth_stencil_state_destroy(radeon_6870_handle_t handle)` | Niszczy stan depth-stencil. |
| `radeon_6870_handle_t radeon_6870_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)` | Tworzy input layout (mapowanie atrybutow vertex do slotow). Zwraca handle. |
| `void radeon_6870_input_layout_destroy(radeon_6870_handle_t handle)` | Niszczy input layout. |
| `radeon_6870_handle_t radeon_6870_constant_buffer_create(uint32_t size)` | Tworzy constant buffer (stale shadera / uniforms) z pamiecia VRAM. Zwraca handle. |
| `void radeon_6870_constant_buffer_destroy(radeon_6870_handle_t handle)` | Niszczy constant buffer i zwalnia jego pamiec VRAM. |
| `void radeon_6870_constant_buffer_update(radeon_6870_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane constant buffera od podanego offsetu. |

### Bindowanie stanow potoku 3D

| funkcja | opis |
|---------|------|
| `void radeon_6870_bind_vertex_buffer(radeon_6870_handle_t handle)` | Binduje vertex buffer do potoku 3D. |
| `void radeon_6870_bind_index_buffer(radeon_6870_handle_t handle)` | Binduje index buffer do potoku 3D. |
| `void radeon_6870_bind_input_layout(radeon_6870_handle_t handle)` | Binduje input layout do potoku 3D. |
| `void radeon_6870_bind_vertex_shader(radeon_6870_handle_t handle)` | Binduje shader vertex do potoku 3D. |
| `void radeon_6870_bind_pixel_shader(radeon_6870_handle_t handle)` | Binduje shader pixel do potoku 3D. |
| `void radeon_6870_bind_geometry_shader(radeon_6870_handle_t handle)` | Binduje shader geometry do potoku 3D. |
| `void radeon_6870_bind_compute_shader(radeon_6870_handle_t handle)` | Binduje shader compute do potoku 3D. |
| `void radeon_6870_bind_render_target(radeon_6870_handle_t handle)` | Binduje render target do potoku 3D. |
| `void radeon_6870_bind_depth_stencil(radeon_6870_handle_t handle)` | Binduje depth-stencil do potoku 3D. |
| `void radeon_6870_bind_blend_state(radeon_6870_handle_t handle)` | Binduje stan blendowania do potoku 3D. |
| `void radeon_6870_bind_rasterizer_state(radeon_6870_handle_t handle)` | Binduje stan rasterizera do potoku 3D. |
| `void radeon_6870_bind_depth_stencil_state(radeon_6870_handle_t handle)` | Binduje stan depth-stencil do potoku 3D. |
| `void radeon_6870_bind_sampler_and_texture(uint32_t slot, radeon_6870_handle_t sampler, radeon_6870_handle_t texture)` | Binduje probke tekstury i teksture do slotu. |
| `void radeon_6870_bind_constant_buffer(uint32_t slot, radeon_6870_handle_t handle)` | Binduje constant buffer do slotu. |

### Konfiguracja potoku 3D

| funkcja | opis |
|---------|------|
| `void radeon_6870_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)` | Ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi. |
| `void radeon_6870_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)` | Ustawia prostokat przyciecia renderowania (scissor). |
| `void radeon_6870_set_primitive_topology(uint32_t topology)` | Ustawia typ primitivu do rysowania (punkt, linia, trojkat, kwadrat itp.). |
| `void radeon_6870_set_alpha_blend_global(uint8_t enable)` | Wlacza/wylacza globalne blendowanie alfa (przezroczystosc) dla calego potoku 3D. |

### Rysowanie 3D

| funkcja | opis |
|---------|------|
| `void radeon_6870_clear_render_target(radeon_6870_handle_t rt, uint32_t color)` | Czysci render target podanym kolorem. |
| `void radeon_6870_clear_depth_stencil(radeon_6870_handle_t ds, float depth, uint8_t stencil)` | Czysci depth-stencil podanymi wartosciami glebi i stencil. |
| `void radeon_6870_draw(uint32_t vertex_count, uint32_t start_vertex)` | Rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex. |
| `void radeon_6870_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)` | Rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex. |
| `void radeon_6870_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)` | Rysuje indeksowane primitivy z instancjami - index_count indeksow, instance_count instancji poczynajac od start_instance. |
| `void radeon_6870_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)` | Rysuje z parametrami pobranymi z bufora indirect (draw indirect). |

### Compute

| funkcja | opis |
|---------|------|
| `void radeon_6870_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)` | Uruchamia obliczenia compute (dispatch) z podana liczba grup watkow w X, Y, Z. |
| `void radeon_6870_compute_dispatch_indirect(uint32_t buffer_addr)` | Uruchamia obliczenia compute z parametrami z bufora indirect. |

### Synchronizacja (fences, semaphory)

| funkcja | opis |
|---------|------|
| `void radeon_6870_fence_create(radeon_6870_handle_t *out)` | Tworzy fence (synchronizacja GPU) i zwraca jego handle. |
| `void radeon_6870_fence_signal(radeon_6870_handle_t handle, uint32_t value)` | Sygnalizuje fence z podana wartoscia (oznacza zakonczenie pracy GPU). |
| `int radeon_6870_fence_wait(radeon_6870_handle_t handle, uint32_t value, uint32_t timeout_us)` | Czeka na sygnalizacje fence do podanej wartosci z timeoutem w mikrosekundach. Zwraca 1 gdy fence zostal sygnalizowany, 0 przy timeoutcie. |
| `void radeon_6870_fence_destroy(radeon_6870_handle_t handle)` | Niszczy fence o podanym handle. |
| `void radeon_6870_semaphore_create(radeon_6870_handle_t *out)` | Tworzy semafor (synchronizacja miedzy etapami potoku) i zwraca jego handle. |
| `void radeon_6870_semaphore_signal(radeon_6870_handle_t handle, uint32_t value)` | Sygnalizuje semafor z podana wartoscia. |
| `void radeon_6870_semaphore_wait(radeon_6870_handle_t handle, uint32_t value)` | Czeka na sygnalizacje semafora do podanej wartosci (blokujace). |
| `void radeon_6870_semaphore_destroy(radeon_6870_handle_t handle)` | Niszczy semafor o podanym handle. |

### Zarzadzanie zasilaniem, zegarami, temperatura, wentylator, RGB

| funkcja | opis |
|---------|------|
| `int radeon_6870_wait_idle(uint32_t timeout_us)` | Czeka az GPU bedzie bezczynne (ring buffer pusty) z timeoutem. Zwraca 1 gdy GPU jest idle, 0 przy timeoutcie. |
| `uint64_t radeon_6870_timestamp(void)` | Pobiera aktualny znacznik czasu z rejestru timestamp GPU. |
| `void radeon_6870_power_set_state(uint8_t state)` | Ustawia stan zasilania GPU (D0-D3). |
| `uint8_t radeon_6870_power_get_state(void)` | Zwraca aktualny stan zasilania GPU. |
| `void radeon_6870_set_clock_gfx(uint32_t mhz)` | Ustawia taktowanie rdzenia grafiki (GFX clock) w MHz. |
| `void radeon_6870_set_clock_memory(uint32_t mhz)` | Ustawia taktowanie pamieci (memory clock) w MHz. |
| `uint32_t radeon_6870_get_clock_gfx(void)` | Zwraca aktualne taktowanie rdzenia grafiki w MHz. |
| `uint32_t radeon_6870_get_clock_memory(void)` | Zwraca aktualne taktowanie pamieci w MHz. |
| `uint32_t radeon_6870_get_temperature(void)` | Pobiera aktualna temperature GPU z czujnika termicznego (w stopniach Celsjusza). |
| `void radeon_6870_set_fan_speed(uint32_t rpm)` | Ustawia predkosc wentylatora GPU w obrotach na minute (RPM). |
| `uint32_t radeon_6870_get_fan_speed(void)` | Zwraca aktualna predkosc wentylatora GPU w RPM. |
| `void radeon_6870_rgb_set(uint32_t color, uint8_t mode)` | Ustawia sterowanie RGB na karcie (kolor i tryb swiecenia). |
| `void radeon_6870_rgb_off(void)` | Wylacza sterowanie RGB na karcie. |

### Presentation (swap chain)

| funkcja | opis |
|---------|------|
| `radeon_6870_handle_t radeon_6870_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied)` | Tworzy swap chain (podwojne buforowanie z opcja vsync i premultiplied alpha). Zwraca handle. |
| `void radeon_6870_swap_chain_destroy(radeon_6870_handle_t handle)` | Niszczy swap chain i zwalnia pamiec VRAM bufora tylnego. |
| `void radeon_6870_present(radeon_6870_handle_t chain)` | Przebija tylny bufor na przedni (present) z oczekiwaniem na vsync (opcjonalnie) przez ring buffer CP. |

### Command buffer

| funkcja | opis |
|---------|------|
| `void radeon_6870_command_begin(void)` | Rozpoczyna nagrywanie polecen do command buffer. |
| `void radeon_6870_command_end(void)` | Zakoncza nagrywanie i wykonuje wszystkie polecenia command buffer przez ring buffer CP. |

### Informacje o akceleracji 3D

| funkcja | opis |
|---------|------|
| `void radeon_6870_context_get(struct radeon_6870_context *out)` | Pobiera biezacy kontekst 3D (stan potoku). |
| `int radeon_6870_3d_available(void)` | Zwraca 1 gdy akceleracja 3D jest dostepna (sterownik zainicjowany). |

### Stale i typy

| nazwa | opis |
|-------|------|
| `RADEON_6870_VENDOR_ID` | ID dostawcy AMD (0x1002). |
| `RADEON_6870_DEVICE_ID_CAYMAN_XT` | ID urzadzenia Cayman XT (0x6738) oraz warianty 0x6739/0x673A/0x673B/0x673C/0x673D. |
| `RADEON_6870_VRAM_SIZE_BYTES` | Calkowita pojemnosc VRAM (GDDR5). |
| `RADEON_6870_FB_WIDTH / RADEON_6870_FB_HEIGHT` | Rozdzielczosc ramki graficznej (1920x1080). |
| `RADEON_6870_FB_PITCH` | Pitch ramki graficznej (7680 bajtow). |
| `RADEON_6870_HANDLE_NULL` | Wartosc nulowa dla handle zasobow (0xFFFFFFFFU). |
| `radeon_6870_handle_t` | Typ handle zasobu (uint32_t). |
| `RADEON_6870_*` | Stale dla formatow vertex, indeksow, primitivow, tekstur, shaderow, blendowania, operacji blendowania, porownan glebi, wypelnienia, cullowania, operacji stencil (patrz naglowek radeon_hd_6870.h). |

### Integracja z jadrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `rx_9070_xt_init()`:

```
radeon_6870_init();
```

Naglowek `drivers/karty_graficzne/radeon_hd_6870.h` jest dolaczany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywolywane z kodu C jadra. Plik zrodlowy `drivers/karty_graficzne/radeon_hd_6870.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/karty_graficzne/*.cpp)`).

## konwencja pisania kodu

- komentarze po polsku
- commity po polsku
- kod w C (C++ tylko z pozwolenia, nie w miejscach krytycznych)
- komentarze tylko tam gdzie kod jest mniej czytelny

## sterownik sieciowy realtek rtl8111e

plik `drivers/internet/rtl8111e.cpp` i naglowek `drivers/internet/rtl8111e.h` implementuja pelny sterownik PCIe Gigabit Ethernet dla kart Realtek RTL8111E (rodzina RTL8168/RTL8111, vendor 0x10EC, urzadzenie 0x8168). Kod jest napisany w C++, eksportuje interfejs C (`extern "C"`), jest wolny od wyjatkow i RTTI i kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/internet/*.cpp)`).

sterownik inicjalizuje sie przez:
1. skanowanie szyny PCI i wyszukanie urzadzenia Realtek 0x10EC:0x8168
2. odczyt BAR0 (MMIO), wlaczenie miejsca pamietnego i bus master w rejestrze komend PCI
3. zapis `REG_CFG9346 = 0` (tryb normalny), soft reset ukazu (`REG_CMDR`)
4. odczyt adresu MAC z rejestrow `REG_MAC0` / `REG_MAC4`
5. wyczyszczenie oczekujacych przerwan (`REG_ISR`), maska `REG_IMR = 0` (tryb poling, bez przerwan)
6. konfiguracja nadawania `REG_TCR = 0`, odbioru `REG_RCR` (wlasny MAC, broadcast, multicast, ramki fizyczne, bufor 16K)
7. adres bufora odbiorczego `REG_RBSTART`
4 deskryptory nadawcze `REG_TSAD0..3` z buforami `REG_TSD0..3` i bitem `TSD_OWN`
8. wlaczenie nadawania i odbioru (`CMD_START | CMD_TX_ENABLE | CMD_RX_ENABLE`)

odbior dziala w buforze liniowym cyklicznym pod `REG_RBSTART`; kazda ramka jest poprzedzona 4-bajtowym naglowkiem (status + dlugosc). Nadawanie czeka az karta zwolni deskryptor (bit `TSD_OWN`), kopiuje ramke, oddaje deskryptor karti i czeka na zakonczenie transmisji.

dostepne funkcje:

| funkcja | opis |
|---------|------|
| `rtl8111e_init()` | inicjalizuje karte: PCI, MMIO, reset, MAC, rejestry, bufor, TX/RX. zwraca 1 przy sukcesie, 0 przy porazce. |
| `rtl8111e_is_ready()` | zwraca 1 gdy sterownik jest zainicjowany i gotowy. |
| `rtl8111e_get_mac(mac)` | kopiuje 6-bajtowy adres MAC do tablicy `mac`. zwraca 1/0. |
| `rtl8111e_send(frame, length)` | wysyla ramke ethernet. zwraca 1 po zakonczeniu, 0 gdy deskryptor zajety, -1 przy bledzie argumentow. |
| `rtl8111e_receive(frame, capacity)` | odbiera ramke w trybie poling. zwraca dlugosc ramki, 0 gdy brak danych, -1 przy bledzie. |
| `rtl8111e_link_up()` | odczytuje status lacza z PHY (rejestr BMSR bit 2). zwraca 1 gdy lacze aktywne. |
| `rtl8111e_speed_mbps()` | zwraca predkosc lacza w Mb/s (1000 przy laczu, 0 bez). |
| `rtl8111e_duplex_full()` | zwraca 1 gdy lacze pracuje w pelnodupleksie. |
| `rtl8111e_autonegotiate()` | wymusza autonegocjacje PHY i czeka na zakonczenie oraz lacze. zwraca 1/0. |
| `rtl8111e_reset_link()` | resetuje PHY i ponawia autonegocjacje. zwraca 1/0. |
| `rtl8111e_loopback_test(frame, length)` | wlacza wewnetrzny loopback, wysyla ramke, odbiera i porownuje. zwraca 1 przy zgodzie, 0 przy rozbieznosci, -1 przy bledzie. |
| `rtl8111e_set_multicast(addresses, count)` | programuje filtr adresow multicast w rejestrach MAR. |
| `rtl8111e_stats_get(out)` | wypelnia strukture `rtl8111e_stats` (tx_packets, tx_bytes, tx_errors, rx_packets, rx_bytes, rx_errors, resets, link_changes). |

PHY jest dostepny przez rejestr `REG_PHYAR` (bit 31 = zajety, bit 30 = zapis, bity 23-16 = adres rejestru, bity 15-0 = dane). Uzywane rejestry PHY: BMCR (0) i BMSR (1).

integracja z przegladarka:

`drivers/net.c` wywoluje `rtl8111e_init()` jako pierwsze w lancuchu inicjalizacji sieci (przed `rtl8168b`, `rtl8188ce`, `e1000`, `rtl8139`). gdy karta RTL8111E jest wykryta, `net_send_fn` i `net_receive_fn` wskazuja na funkcje sterownika rtl8111e. przegladarka (`kernel/browser.c`) uzywa `net_ready()` oraz `net_http_get()` z `drivers/net.c`, wiec po udanej inicjalizacji sterownika RTL8111E przegladarka pobiera strony przez ta karte automatycznie. w srodowisku QEMU (domyslna karta RTL8139) lacze jest nawiazywane przez sterownik rezerwowy `rtl8139`.

## karta graficzna - drivers/karty_graficzne/radeon_8500.cpp

Zaawansowany sterownik karty graficznej ATI Radeon 8500 (architektura R200, DirectX 8.1) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, programowalne shadery (vertex, pixel, geometry), akceleracje sprzetowa przez Command Processor (CP), przezroczystosc (alpha blending), tekstury z mipmapami i kompresja (DXT, ATI1N, ATI2N), render targety, depth-stencil, fences, semaphory, command buffer oraz presentation (swap chain, vsync, potrojne buforowanie). Sterownik zarzadza pamiecia wideo (VRAM allocator), taktowaniem GPU i pamieci, temperatura, wentylatorem oraz sterowaniem RGB. Eksportuje interfejs C przez naglowek radeon_8500.h i jest inicjalizowany z kernel/kernel.c przez `radeon_8500_init()`.

### Inicjalizacja i stan

| funkcja | opis |
|---------|------|
| `int radeon_8500_init(void)` | Inicjalizuje sterownik Radeon 8500: wyszukuje urzadzenie na PCI (vendor 0x1002, R200), mapuje adresy MMIO, czysci przerwania, konfiguruje silnik wyswietlania (CRTC, plane, surface, 1920x1080), resetuje i inicjalizuje Command Processor (ring buffer), konfiguruje potok 3D (color buffer, depth buffer, blend, rasterizer, shader SQ), inicjalizuje kursor sprhetowy. Zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie. |
| `int radeon_8500_is_ready(void)` | Zwraca 1 gdy sterownik zostal zainicjalizowany i jest gotowy. |
| `void radeon_8500_shutdown(void)` | Zatrzymuje Command Processor (ME i PFP Halt), przechodzi w stan zasilania D3 i oznacza sterownik jako nieinicjalizowany. |

### Podstawowa grafika 2D

| funkcja | opis |
|---------|------|
| `void radeon_8500_putpixel(uint32_t x, uint32_t y, uint32_t color)` | Rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem. |
| `void radeon_8500_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)` | Wypelnia prostokat o podanym rozmiarze i kolorze. Uzywa ring buffer CP gdy sterownik jest zainicjowany, w przeciwnym razie petla CPU. |
| `void radeon_8500_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)` | Kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) uzywajac ring buffer CP sprzetowego. |
| `void radeon_8500_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)` | Ustawia kursor sprhetowy na pozycji (x, y) z obrazem 64x64 (kopiowany do pamieci ramki graficznej). |
| `void radeon_8500_cursor_hide(void)` | Ukrywa kursor sprhetowy. |
| `void *radeon_8500_get_fb(void)` | Zwraca wskaznik na ramke graficzna (framebuffer). |
| `uint32_t radeon_8500_get_width(void)` | Zwraca szerokosc ekranu (1024). |
| `uint32_t radeon_8500_get_height(void)` | Zwraca wysokosc ekranu (768). |
| `uint16_t radeon_8500_get_device_id(void)` | Zwraca ID urzadzenia graficznego. |
| `uint8_t radeon_8500_get_revision(void)` | Zwraca rewizje karty graficznej. |
| `void radeon_8500_info_get(struct radeon_8500_info *out)` | Pobiera pelne informacje o urzadzeniu (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized, architecture, vram_size, vram_used, gfx_clock_mhz, memory_clock_mhz, temperature_c, fan_rpm). |

### Zaawansowane shadery i akceleracja 3D

| funkcja | opis |
|---------|------|
| `radeon_8500_handle_t radeon_8500_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)` | Tworzy shader (vertex/pixel/geometry/compute) z kodu bajtowego GPU R200. Zwraca handle lub RADEON_8500_HANDLE_NULL przy bledzie. |
| `void radeon_8500_shader_destroy(radeon_8500_handle_t handle)` | Niszczy shader o podanym handle. |
| `void radeon_8500_shader_compile(radeon_8500_handle_t handle)` | Kompiluje shader i oznacza go jako gotowy do uzycia w potoku graficznym. |
| `void radeon_8500_bind_vertex_shader(radeon_8500_handle_t handle)` | Binduje shader vertex do potoku 3D (etap przetwarzania wierzcholkow). |
| `void radeon_8500_bind_pixel_shader(radeon_8500_handle_t handle)` | Binduje shader pixel do potoku 3D (etap cieniowania pikseli). |
| `void radeon_8500_bind_geometry_shader(radeon_8500_handle_t handle)` | Binduje shader geometry do potoku 3D (etap generowania primitivow geometrycznych). |
| `void radeon_8500_bind_compute_shader(radeon_8500_handle_t handle)` | Binduje shader compute do potoku (obliczenia ogolnego przeznaczenia na GPU). |

### Tworzenie i niszczenie zasobow 3D

| funkcja | opis |
|---------|------|
| `radeon_8500_handle_t radeon_8500_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)` | Tworzy vertex buffer (wierzcholki) z pamiecia VRAM. Zwraca handle. |
| `void radeon_8500_vertex_buffer_destroy(radeon_8500_handle_t handle)` | Niszczy vertex buffer i zwalnia jego pamiec VRAM. |
| `void radeon_8500_vertex_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane vertex buffera od podanego offsetu. |
| `radeon_8500_handle_t radeon_8500_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)` | Tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe) w VRAM. Zwraca handle. |
| `void radeon_8500_index_buffer_destroy(radeon_8500_handle_t handle)` | Niszczy index buffer i zwalnia jego pamiec VRAM. |
| `void radeon_8500_index_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane index buffera od podanego offsetu. |
| `radeon_8500_handle_t radeon_8500_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)` | Tworzy teksture 2D z obsuga mipmap i kompresji (DXT1/3/5, ATI1N, ATI2N) w VRAM. Zwraca handle. |
| `void radeon_8500_texture_destroy(radeon_8500_handle_t handle)` | Niszczy teksture i zwalnia jej pamiec VRAM. |
| `void radeon_8500_texture_update(radeon_8500_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)` | Aktualizuje zawartosc tekstury (upload z pamieci systemowej do VRAM). |
| `void radeon_8500_texture_generate_mipmaps(radeon_8500_handle_t handle)` | Generuje lancuch mipmap dla tekstury (kolejne poziomy przez polowowanie wymiarow). |
| `radeon_8500_handle_t radeon_8500_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend)` | Tworzy render target (bufor koloru do renderowania 3D z opcjonalna przezroczystoscia alpha). Zwraca handle. |
| `void radeon_8500_render_target_destroy(radeon_8500_handle_t handle)` | Niszczy render target i zwalnia jego pamiec VRAM. |
| `radeon_8500_handle_t radeon_8500_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression)` | Tworzy depth-stencil buffer (bufor glebi i stencil z opcjonalna kompresja). Zwraca handle. |
| `void radeon_8500_depth_stencil_destroy(radeon_8500_handle_t handle)` | Niszczy depth-stencil i zwalnia jego pamiec VRAM. |
| `radeon_8500_handle_t radeon_8500_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable, uint8_t compare_func)` | Tworzy stan probki tekstury (filtrowanie min/mag/mip, adresowanie U/V/W, anisotropia, porownanie). Zwraca handle. |
| `void radeon_8500_sampler_state_destroy(radeon_8500_handle_t handle)` | Niszczy stan probki tekstury. |
| `radeon_8500_handle_t radeon_8500_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor)` | Tworzy stan blendowania (przezroczystosc alpha, operacje blendowania RGB i alpha, staly kolor blendowania). Zwraca handle. |
| `void radeon_8500_blend_state_destroy(radeon_8500_handle_t handle)` | Niszczy stan blendowania. |
| `radeon_8500_handle_t radeon_8500_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage)` | Tworzy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygladzone, alpha-to-coverage). Zwraca handle. |
| `void radeon_8500_rasterizer_state_destroy(radeon_8500_handle_t handle)` | Niszczy stan rasterizera. |
| `radeon_8500_handle_t radeon_8500_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)` | Tworzy stan depth-stencil (testy glebi i stencil z przodem i tylem). Zwraca handle. |
| `void radeon_8500_depth_stencil_state_destroy(radeon_8500_handle_t handle)` | Niszczy stan depth-stencil. |
| `radeon_8500_handle_t radeon_8500_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)` | Tworzy input layout (mapowanie atrybutow vertex do slotow wejsciowych). Zwraca handle. |
| `void radeon_8500_input_layout_destroy(radeon_8500_handle_t handle)` | Niszczy input layout. |
| `radeon_8500_handle_t radeon_8500_constant_buffer_create(uint32_t size)` | Tworzy constant buffer (stale shadera / uniforms) w pamieci VRAM. Zwraca handle. |
| `void radeon_8500_constant_buffer_destroy(radeon_8500_handle_t handle)` | Niszczy constant buffer i zwalnia jego pamiec VRAM. |
| `void radeon_8500_constant_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data)` | Aktualizuje dane constant buffera od podanego offsetu. |

### Bindowanie stanow potoku 3D

| funkcja | opis |
|---------|------|
| `void radeon_8500_bind_vertex_buffer(radeon_8500_handle_t handle)` | Binduje vertex buffer do potoku 3D. |
| `void radeon_8500_bind_index_buffer(radeon_8500_handle_t handle)` | Binduje index buffer do potoku 3D. |
| `void radeon_8500_bind_input_layout(radeon_8500_handle_t handle)` | Binduje input layout do potoku 3D. |
| `void radeon_8500_bind_render_target(radeon_8500_handle_t handle)` | Binduje render target do potoku 3D. |
| `void radeon_8500_bind_depth_stencil(radeon_8500_handle_t handle)` | Binduje depth-stencil do potoku 3D. |
| `void radeon_8500_bind_blend_state(radeon_8500_handle_t handle)` | Binduje stan blendowania do potoku 3D. |
| `void radeon_8500_bind_rasterizer_state(radeon_8500_handle_t handle)` | Binduje stan rasterizera do potoku 3D. |
| `void radeon_8500_bind_depth_stencil_state(radeon_8500_handle_t handle)` | Binduje stan depth-stencil do potoku 3D. |
| `void radeon_8500_bind_sampler_and_texture(uint32_t slot, radeon_8500_handle_t sampler, radeon_8500_handle_t texture)` | Binduje probke tekstury i teksture do slotu tekstur. |
| `void radeon_8500_bind_constant_buffer(uint32_t slot, radeon_8500_handle_t handle)` | Binduje constant buffer do slotu stalych shadera. |

### Konfiguracja potoku 3D

| funkcja | opis |
|---------|------|
| `void radeon_8500_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)` | Ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi. |
| `void radeon_8500_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)` | Ustawia prostokat przyciecia renderowania (scissor). |
| `void radeon_8500_set_primitive_topology(uint32_t topology)` | Ustawia typ primitivu do rysowania (punkt, linia, trojkat, kwadrat itp.). |
| `void radeon_8500_set_alpha_blend_global(uint8_t enable)` | Wlacza/wylacza globalne blendowanie alfa (przezroczystosc) dla calego potoku 3D. |

### Rysowanie 3D

| funkcja | opis |
|---------|------|
| `void radeon_8500_clear_render_target(radeon_8500_handle_t rt, uint32_t color)` | Czysci render target podanym kolorem przez potok CB sprzetowy. |
| `void radeon_8500_clear_depth_stencil(radeon_8500_handle_t ds, float depth, uint8_t stencil)` | Czysci depth-stencil podanymi wartosciami glebi i stencil. |
| `void radeon_8500_draw(uint32_t vertex_count, uint32_t start_vertex)` | Rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex. |
| `void radeon_8500_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)` | Rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex. |
| `void radeon_8500_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)` | Rysuje indeksowane primitivy z instancjami - index_count indeksow, instance_count instancji poczynajac od start_instance. |
| `void radeon_8500_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)` | Rysuje z parametrami pobranymi z bufora indirect (draw indirect) sprzed GPU. |

### Compute (obliczenia GPU)

| funkcja | opis |
|---------|------|
| `void radeon_8500_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)` | Uruchamia obliczenia compute (dispatch) z podana liczba grup watkow w X, Y, Z. |
| `void radeon_8500_compute_dispatch_indirect(uint32_t buffer_addr)` | Uruchamia obliczenia compute z parametrami z bufora indirect. |

### Synchronizacja (fences, semaphory)

| funkcja | opis |
|---------|------|
| `void radeon_8500_fence_create(radeon_8500_handle_t *out)` | Tworzy fence (synchronizacja GPU) i zwraca jego handle. |
| `void radeon_8500_fence_signal(radeon_8500_handle_t handle, uint32_t value)` | Sygnalizuje fence z podana wartoscia (oznacza zakonczenie pracy GPU). |
| `int radeon_8500_fence_wait(radeon_8500_handle_t handle, uint32_t value, uint32_t timeout_us)` | Czeka na sygnalizacje fence do podanej wartosci z timeoutem w mikrosekundach. Zwraca 1 gdy fence zostal sygnalizowany, 0 przy timeoutcie. |
| `void radeon_8500_fence_destroy(radeon_8500_handle_t handle)` | Niszczy fence o podanym handle. |
| `void radeon_8500_semaphore_create(radeon_8500_handle_t *out)` | Tworzy semafor (synchronizacja miedzy etapami potoku) i zwraca jego handle. |
| `void radeon_8500_semaphore_signal(radeon_8500_handle_t handle, uint32_t value)` | Sygnalizuje semafor z podana wartoscia. |
| `void radeon_8500_semaphore_wait(radeon_8500_handle_t handle, uint32_t value)` | Czeka na sygnalizacje semafora do podanej wartosci (blokujace). |
| `void radeon_8500_semaphore_destroy(radeon_8500_handle_t handle)` | Niszczy semafor o podanym handle. |

### Zarzadzanie pamiecia VRAM

| funkcja | opis |
|---------|------|
| `uint64_t radeon_8500_vram_alloc(uint64_t size, uint32_t type)` | Alokuje obszar pamieci VRAM o podanym rozmiarze i typie (wyrownanie do strony 4 KB). Zwraca offset w VRAM lub 0xFFFFFFFFFFFFFFFF przy bledzie. |
| `void radeon_8500_vram_free(uint64_t handle)` | Zwalnia obszar pamieci VRAM o podanym offsecie (oznacza slot jako wolny). |
| `uint64_t radeon_8500_vram_used(void)` | Zwraca liczbe bajtow VRAM aktualnie uzytych. |
| `uint64_t radeon_8500_vram_total(void)` | Zwraca calkowita pojemnosc VRAM (64 MB). |

### Zarzadzanie zasilaniem, zegarami, temperatura, wentylator, RGB

| funkcja | opis |
|---------|------|
| `int radeon_8500_wait_idle(uint32_t timeout_us)` | Czeka az GPU bedzie bezczynne (ring buffer pusty) z timeoutem. Zwraca 1 gdy GPU jest idle, 0 przy timeoutcie. |
| `uint64_t radeon_8500_timestamp(void)` | Pobiera aktualny znacznik czasu z rejestru timestamp GPU (64-bitowy). |
| `void radeon_8500_power_set_state(uint8_t state)` | Ustawia stan zasilania GPU (D0-D3). |
| `uint8_t radeon_8500_power_get_state(void)` | Zwraca aktualny stan zasilania GPU. |
| `void radeon_8500_set_clock_gfx(uint32_t mhz)` | Ustawia taktowanie rdzenia grafiki (GFX clock) w MHz. |
| `void radeon_8500_set_clock_memory(uint32_t mhz)` | Ustawia taktowanie pamieci (memory clock) w MHz. |
| `uint32_t radeon_8500_get_clock_gfx(void)` | Zwraca aktualne taktowanie rdzenia grafiki w MHz. |
| `uint32_t radeon_8500_get_clock_memory(void)` | Zwraca aktualne taktowanie pamieci w MHz. |
| `uint32_t radeon_8500_get_temperature(void)` | Pobiera aktualna temperature GPU z czujnika termicznego (w stopniach Celsjusza). |
| `void radeon_8500_set_fan_speed(uint32_t rpm)` | Ustawia predkosc wentylatora GPU w obrotach na minute (RPM). |
| `uint32_t radeon_8500_get_fan_speed(void)` | Zwraca aktualna predkosc wentylatora GPU w RPM. |
| `void radeon_8500_rgb_set(uint32_t color, uint8_t mode)` | Ustawia sterowanie RGB na karcie (kolor i tryb swiecenia). |
| `void radeon_8500_rgb_off(void)` | Wylacza sterowanie RGB na karcie. |

### Presentation (swap chain)

| funkcja | opis |
|---------|------|
| `radeon_8500_handle_t radeon_8500_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied, uint8_t triple_buffer)` | Tworzy swap chain (podwojne lub potrojne buforowanie z opcja vsync i premultiplied alpha). Zwraca handle. |
| `void radeon_8500_swap_chain_destroy(radeon_8500_handle_t handle)` | Niszczy swap chain i zwalnia pamiec VRAM buforow. |
| `void radeon_8500_present(radeon_8500_handle_t chain)` | Przebija tylny bufor na przedni (present) z oczekiwaniem na vsync (opcjonalnie) przez ring buffer CP. |

### Command buffer

| funkcja | opis |
|---------|------|
| `void radeon_8500_command_begin(void)` | Rozpoczyna nagrywanie polecen do command buffer. |
| `void radeon_8500_command_end(void)` | Zakoncza nagrywanie i wykonuje wszystkie polecenia command buffer przez ring buffer CP. |

### Informacje o akceleracji 3D

| funkcja | opis |
|---------|------|
| `void radeon_8500_context_get(struct radeon_8500_context *out)` | Pobiera biezacy kontekst 3D (stan potoku). |
| `int radeon_8500_3d_available(void)` | Zwraca 1 gdy akceleracja 3D jest dostepna (sterownik zainicjowany). |

### Stale i typy

| nazwa | opis |
|-------|------|
| `RADEON_8500_VENDOR_ID` | ID dostawcy ATI/AMD (0x1002). |
| `RADEON_8500_DEVICE_ID_8500` | ID urzadzenia Radeon 8500 (0x4C48) oraz warianty LE i inne (0x4C4D/0x4C56/0x4C57/0x514C/0x514E/0x514F). |
| `RADEON_8500_VRAM_SIZE_BYTES` | Calkowita pojemnosc VRAM (64 MB). |
| `RADEON_8500_FB_WIDTH / RADEON_8500_FB_HEIGHT` | Rozdzielczosc ramki graficznej (1920x1080). |
| `RADEON_8500_FB_PITCH` | Pitch ramki graficznej (4096 bajtow). |
| `RADEON_8500_HANDLE_NULL` | Wartosc nulowa dla handle zasobow (0xFFFFFFFFU). |
| `radeon_8500_handle_t` | Typ handle zasobu (uint32_t). |
| `RADEON_8500_*` | Stale dla formatow vertex, indeksow, primitivow, tekstur, shaderow, blendowania, operacji blendowania, porownan glebi, wypelnienia, cullowania, operacji stencil (patrz naglowek radeon_8500.h). |

### Integracja z jadrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`:

```
radeon_8500_init();
```

Naglowek `drivers/karty_graficzne/radeon_8500.h` jest dolaczany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywolywane z kodu C jadra. Plik zrodlowy `drivers/karty_graficzne/radeon_8500.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/karty_graficzne/*.cpp)`).

## sterownik 3dfx Voodoo2 - rust/drivers/karty_graficzne/3dfx/voodoo2.rs

zaawansowany sterownik karty graficznej 3dfx Voodoo2 (chip SST2, architektura Voodoo2) napisany w Rust. obsluguje wykrycie urzadzenia na PCI (vendor 0x121A, urzadzenie 0x0005), mapowanie MMIO (baza 0xF0000000) i ramki graficznej (baza 0xE0000000), soft-reset chipa, inicjalizacje pelnego potoku graficzny (FBZ mode, tryb alpha, mgla, chroma key, stipple, format koloru, dithering, bufor koloru, bufor pomocniczy, rejestry FBI, kolor mgly, referencja alpha, tryb tekstury, baza tekstury, jednostki tekstury, setup trojkata, blendowanie, test glebi, stencil, mgla, filtrowanie trilinearne, viewport, scissor), wlaczenie zaawansowanej akceleracji 3D oraz pelny zestaw funkcji rysowania 3D (trojekaty, linie, czyszczenie ekranu, viewport, macierze rzutowania/swiata/widoku, test glebi, blendowanie, funkcje blendowania, tryb cienia, ramka druciana, multisampling, antyaliasing, granice glebi, stencil, operacje stencil, rownanie blendowania, kolor blendowania, operacja logiczna, dithering, offset wielokata, probka probkowania, tablice wierzcholkow/indeksow, rysowanie tablic/elementow/instancji, programy shadera, uniformy, tekstury, renderbuffery, framebuffery, blit, odczyt pikseli, finish, flush, status). napisany w Rust z `#[no_std]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/3dfx/*.rs)`) i linkowany do jadra.

### voodoo2_init

inicjalizuje sterownik Voodoo2: wykrywa urzadzenie na PCI, wykonuje soft-reset, czeka na bezczynnosc, konfiguruje wszystkie rejestry potoku graficznego i wlacza akceleracje 3D. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

```
bool voodoo2_init(void);
```

### voodoo2_detected

sprawdza czy karta 3dfx Voodoo2 jest wykryta na PCI (porownuje vendor ID i device ID). zwraca 1 gdy wykryta, 0 gdy nie.

```
bool voodoo2_detected(void);
```

### voodoo2_draw_triangle_3d

rysuje trojkat 3D o wierzcholkach (x1,y1,z1), (x2,y2,z2), (x3,y3,z3) i podanym kolorze.

```
void voodoo2_draw_triangle_3d(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, uint32_t color);
```

### voodoo2_clear_screen_3d

czysci ekran podanym kolorem w trybie 3D.

```
void voodoo2_clear_screen_3d(uint32_t color);
```

### voodoo2_set_viewport

ustawia viewport (prostokat wyswietlania) na pozycji (x, y) o rozmiarze (width, height).

```
void voodoo2_set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
```

### voodoo2_set_projection_matrix / voodoo2_set_world_matrix / voodoo2_set_view_matrix

ustawia macierz rzutowania, swiata lub widoku (4x4, podana jako wskaznik na `Matrix4x4`).

```
void voodoo2_set_projection_matrix(const Matrix4x4 *matrix);
void voodoo2_set_world_matrix(const Matrix4x4 *matrix);
void voodoo2_set_view_matrix(const Matrix4x4 *matrix);
```

### voodoo2_enable_depth_test / voodoo2_enable_blending

wlacza lub wylacza test glebi lub blendowanie.

```
void voodoo2_enable_depth_test(bool enabled);
void voodoo2_enable_blending(bool enabled);
```

### voodoo2_set_blend_func

ustawia funkcje blendowania zrodlo (src) i cel (dst).

```
void voodoo2_set_blend_func(uint32_t src, uint32_t dst);
```

### voodoo2_set_depth_func

ustawia funkcje porownania glebi.

```
void voodoo2_set_depth_func(uint32_t func);
```

### voodoo2_set_cull_mode

ustawia tryb cienia (culling) trojkatow.

```
void voodoo2_set_cull_mode(uint32_t mode);
```

### voodoo2_set_wireframe

wlacza lub wylacza tryb ramki drucianej (wireframe).

```
void voodoo2_set_wireframe(bool enabled);
```

### voodoo2_set_multisample_count / voodoo2_enable_multisampling

ustawia liczbe probek multisampling lub wlacza/wylacza multisampling.

```
void voodoo2_set_multisample_count(uint32_t count);
void voodoo2_enable_multisampling(bool enabled);
```

### voodoo2_enable_antialiasing

wlacza lub wylacza wygladzanie krawedzi (antialiasing).

```
void voodoo2_enable_antialiasing(bool enabled);
```

### voodoo2_finish / voodoo2_flush

czeka az GPU zakonczy wszystkie operacje (finish) lub wymusza wykonanie oczekujacych polecen (flush).

```
void voodoo2_finish(void);
void voodoo2_flush(void);
```

### voodoo2_get_status

zwraca aktualny status rejestru karty graficznej.

```
uint32_t voodoo2_get_status(void);
```

### Integracja z jadrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`:

```
voodoo2_init();
```

Wszystkie funkcje eksportowane sa jako C (`#[no_mangle] pub extern "C"`), wiec moga byc wywolywane z kodu C jadra. Plik zrodlowy `rust/drivers/karty_graficzne/3dfx/voodoo2.rs` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/3dfx/*.rs)`).

## sterownik plyt optycznych - drivers/plyty.cpp

zaawansowany sterownik napedow plyt optycznych (CD, DVD, Blu-ray, HD DVD, CD-i, Photo CD, MO, WO) napisany w C++. obsluguje interfejs ATAPI (ATA Packet Interface) dla glownej i pomocniczej kanalowe magistrali IDE, wykrycie napedu, identyfikacje urzadzenia, odczyt konfiguracji, pomiar pojemnosci, wykrycie systemu plikow (ISO 9660, UDF, Joliet, Rock Ridge, HFS, CDFS), czytanie sektorow, czytanie plikow, listowanie katalogu glownego, wysiadanie i zamykanie tacki, wykrywanie zmiany plyty. wspiera kazdy rodzaj plyty w tym formaty niszowe. kompilowany przez makefile (wildcard `$(wildcard drivers/*.cpp)`) i linkowany do jadra.

### plyty_init

inicjalizuje sterownik plyt optycznych: wykrywa naped na kanale IDE, wykonuje identyfikacje ATAPI, wykrywa typ plyty (CD/DVD/BD/HD DVD/niszowe), odczytuje pojemnosc i rozmiar sektora, wykrywa system plikow. zwraca 1 po udanej inicjalizacji, 0 gdy naped nie zostanie wykryty.

```
int plyty_init(void);
```

### plyty_present

zwraca 1 gdy naped plyty optycznej jest wykryty i obecny, 0 gdy nie.

```
int plyty_present(void);
```

### plyty_ready

zwraca 1 gdy naped jest gotowy do odczytu, 0 gdy nie.

```
int plyty_ready(void);
```

### plyty_type

zwraca typ wykrytej plyty jako kombinacja flag: PLITY_CDROM, PLITY_DVDROM, PLITY_BDROM, PLITY_HDDVD, PLITY_CDI, PLITY_PHOTOCD, PLITY_CDR, PLITY_CDRW, PLITY_DVDR, PLITY_DVDRW, PLITY_BDR, PLITY_BDRE, PLITY_MO, PLITY_WO.

```
uint32_t plyty_type(void);
```

### plyty_sector_count / plyty_sector_size

zwracaja liczbe sektorow na plycie oraz rozmiar sektora w bajtach (zazwyczaj 2048).

```
uint64_t plyty_sector_count(void);
uint32_t plyty_sector_size(void);
```

### plyty_read_sector / plyty_read_sectors

czytuje jeden sektor lub wiele sektorow z plyty do bufora.

```
int plyty_read_sector(uint64_t sector, char *buffer);
int plyty_read_sectors(uint64_t sector, uint32_t count, char *buffer);
```

### plyty_volume_label / plyty_fs_type

zwraca etykiete woluminu plyty oraz typ systemu plikow (PLITY_FS_ISO9660, PLITY_FS_UDF, PLITY_FS_JOLIET, PLITY_FS_ROCKRIDGE, PLITY_FS_HFS, PLITY_FS_CDFS).

```
const char *plyty_volume_label(void);
uint32_t plyty_fs_type(void);
```

### plyty_list_root / plyty_find_file / plyty_read_file

listuje pliki w katalogu glownym plyty, znajduje plik po nazwie, czytuje zawartosc pliku do bufora.

```
int plyty_list_root(struct plyty_file_entry *entries, int capacity);
int plyty_find_file(const char *name, struct plyty_file_entry *entry);
int plyty_read_file(const char *name, char *buffer, uint64_t size);
```

### plyty_disc_changed / plyty_eject / plyty_close_tray

wykrywa zmiane plyty w napedzie, wysadza tacka, zamykanie tacka.

```
int plyty_disc_changed(void);
void plyty_eject(void);
void plyty_close_tray(void);
```

### Integracja z menedzerem plikow

Sterownik jest rejestrowany w `kernel/kernel.c` w funkcji `kernel_main`:

```
driver_manager_register("plyty", plyty_init, 0, 0, 4);
```

Integracja z systemem plikow (`system_plikow.c`):

```
void fs_set_device_cdrom(int use_cdrom);
int fs_is_cdrom(void);
```

Gdy urzadzenie cdrom jest aktywne, funkcje `read_sector`, `read_file`, `fs_list_current` i `cd` sa przekierowywane do sterownika plyt (ATAPI + ISO 9660), co pozwala przegladac pliki na plycie optycznej przez eksplorator plikow.

Eksplorator plikow (`graficzny/eksplorator_plikow.c`) wyswietla opcje "CD-ROM" / "PLYTA OPTYCZNA" obok "DYSKIETKA" / "DYSK TWARDY" gdy naped plyty jest wykryty. Po wyborze plyty optycznej zostaje ona ustawiona jako aktywne urzadzenie systemu plikow, a zawartosc plyty jest wyswietlana w liscie plikow. Mozna entonces otwierac pliki .TXT, .C, .TSI oraz nawigowac po katalogach.

Naglowek `drivers/plyty.h` jest dolaczany w `kernel/kernel.c`. Plik zrodlowy `drivers/plyty.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/*.cpp)`).

## karta graficzna - rust/drivers/karty_graficzne/nvidia/gtx_1060.rs

sterownik karty graficznej NVIDIA GeForce GTX 1060 (GP106, arquitectura Pascal) napisany w Rust. obsluguje pelny potok graficzny 2D i 3D, zaawansowana akceleracja 3D przez Command Processor (CP), przezroczystosc (alpha blending, alpha to coverage, blend equation, blend color, logic op), tekstury z mipmapami i filtrowaniem, shadery (vertex, fragment, geometry, compute, tessellation), uniformy (1f, 2f, 3f, 4f, matrix4fv), render targety, depth-stencil z depth bounds test, tessellation, stream output, zapytania okluzyjne, statystyki potoku, MSAA (multisampling), antialiasing, command buffer z ring bufferami (gfx, compute, dma) i doorbellami, presentation oraz depth test, stencil test, cull mode, wireframe, polygon offset, sample coverage, dither. sterownik opiera sie na architekturze Pascal (GP106, device id 0x1C60, vendor 0x10DE). napisany w Rust z `#[no_std]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/nvidia/*.rs)`) i linkowany do jadra.

inicjalizuje sterownik GTX 1060: wykrywa urzadzenie na PCI (vendor 0x10DE, urzadzenie 0x1C60), mapuje adresy MMIO (0xF0000000), VRAM (0xE0000000), GART (0xD0000000) i doorbelle (0xC0000000), wykonuje soft-reset Command Processor, inicjalizuje PMC, PGRAPH, FECS (context switch), GPCCS (zarzadzanie zasilaniem i zegarami), GPC, TPC, SM (shader multiprocessor), ROP, tekstury, shadery, compute, rasterizer, depth buffer, stencil, blend, color buffer (4 render targety), framebuffery, depth-stencil buffer, multisampling, antialiasing, viewport, scissor, vertex cache, index buffer, primitive buffer, uniform buffer, shader storage buffer, atomic counters, query objects, PCIe, power play (DPM, zegary, wentylator, temperatura), SMU (zarzadzanie zasilaniem GFX/UVD/VCE), ring buffery (gfx, compute, dma) oraz wlacza pelna akceleracje 3D. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

zwraca 1 gdy karta GTX 1060 zostanie wykryta (poprawne vendor id, device id i revisja), 0 w przeciwnym razie.

rysuje pojedynczy trojkat 3D na karcie graficznej. parametry x1,y1,z1, x2,y2,z2, x3,y3,z3 definiuja wierzcholki trojkata w przestrzeni 3D, color to kolor w formacie ARGB. komenda jest wysylana przez ring buffer Command Processor.

rysuje pojedyncza linie 3D na karcie graficznej. parametry x1,y1,z1, x2,y2,z2 definiuja poczatek i koniec linii, color to kolor w formacie ARGB.

czysci caly ekran 3D podanym kolorem. komenda jest wysylana przez Command Processor i oczekuje na zakonczenie operacji.

ustawia viewport dla renderowania 3D. parametry x, y definiuja lewy gorny rog, width i height definiuja rozmiar obszaru renderowania.

ustawia macierz rzutowania (projection matrix) dla potoku 3D. matrix to 16-elementowa tablica float (4x4) zapisana w formacie kolumnowym.

ustawia macierz swiata (world matrix) dla potoku 3D. matrix to 16-elementowa tablica float (4x4) zapisana w formacie kolumnowym.

ustawia macierz widoku (view matrix) dla potoku 3D. matrix to 16-elementowa tablica float (4x4) zapisana w formacie kolumnowym.

wlacza lub wylacza test bufora glebokosci (depth test). enabled = 1 wlacza test, enabled = 0 wylacza.

wlacza lub wylacza mieszanie kolorow (alpha blending). enabled = 1 wlacza blending, enabled = 0 wylacza.

ustawia funkcje mieszania kolorow. src to funkcja zrodla (source blend factor), dst to funkcja celu (destination blend factor).

ustawia funkcje testu bufora glebokosci. func to porownanie depths (np. mniej, rowne, wieksze).

ustawia tryb usuwania tylich scian (culling). mode definiuje ktore sciany sa odrzucane (brak, przod, tyl, oba).

wlacza lub wylacza tryb siatki (wireframe). enabled = 1 rysuje tylko krawedzie wielokatow, enabled = 0 rysuje pelne wielokaty.

ustawia liczbe probek dla multisamplingu (MSAA). count to liczba probek na piksel (np. 4 dla 4x MSAA).

wlacza lub wylacza multisampling (MSAA). enabled = 1 wlacza wygladzanie krawedzi, enabled = 0 wylacza.

wlacza lub wylacza antialiasing. enabled = 1 wlacza wygladzanie, enabled = 0 wylacza.

wlacza lub wylacza test depth bounds. enabled = 1 wlacza test zakresu glebokosci, min i max definiuja dolny i gorny zakres glebokosci.

wlacza lub wylacza test stencila. enabled = 1 wlacza test, func to porownanie, ref_value to wartosc referencyjna, mask to maska bitowa.

ustawia operacje stencila. fail to operacja gdy test stencila nie powiedzie sie, zfail gdy test stencila przejdzie ale test depth nie, zpass gdy oba testy przejda.

ustawia rownanie mieszania (blend equation). rgb to rownanie dla kanalow RGB, alpha to rownanie dla kanalu alpha.

ustawia stalowy kolor mieszania (blend color). color to kolor uzywany w rownaniach mieszania.

ustawia operacje logiczna (logic op) dla bufora koloru. op to operacja logiczna (np. AND, OR, XOR, COPY), enabled wlacza lub wylacza operacje logiczna.

wlacza lub wylacza dithering. enabled = 1 wlacza dithering dla gradacji kolorow, enabled = 0 wylacza.

ustawia polygon offset. enabled = 1 wlacza offset, factor to mnoznik nachylenia, units to stalowy offset w jednostkach depth.

ustawia probkowanie probek (sample coverage). value to wartosc pokrycia probek (0.0-1.0), invert odwraca mazke pokrycia.

ustawia tablice wierzcholkow (vertex array). buffer to adres bufora w VRAM, stride to odstep miedzy wierzcholkami, offset to poczatkowe przesuniecie.

ustawia tablice indeksow (index array). buffer to adres bufora indeksow, format to typ indeksow (16-bit lub 32-bit), offset to przesuniecie.

rysuje wielokaty z tablicy wierzcholkow (draw arrays). mode to typ primitive (trojkaty, linie, punkty), first to indeks pierwszego wierzcholka, count to liczba wierzcholkow.

rysuje wielokaty z tablicy indeksow (draw elements). mode to typ primitive, count to liczba indeksow, type_ to typ indeksow, offset to przesuniecie w buforze indeksow.

rysuje wielokaty z tablicy wierzcholkow z instancjami (draw instanced). mode to typ primitive, first, count, instance_count to liczba instancji.

rysuje wielokaty z tablicy indeksow z instancjami (draw elements instanced). mode, count, type_, offset, instance_count.

ustawia aktywny program shaderow. program_id to identyfikator programu shaderow wbudowanego w GPU.

ustawia uniform typu float (1f). location to lokalizacja uniform, value to wartosc float.

ustawia uniform typu vec2 (2f). location to lokalizacja, x i y to wartosci.

ustawia uniform typu vec3 (3f). location to lokalizacja, x, y, z to wartosci.

ustawia uniform typu vec4 (4f). location to lokalizacja, x, y, z, w to wartosci.

ustawia uniform typu macierz 4x4 (matrix4fv). location to lokalizacja uniform, matrix to 16-elementowa tablica float.

binduje teksture do jednostki teksturyjnej. unit to indeks jednostki (0-N), texture_id to id tekstury.

binduje framebuffer do renderowania. framebuffer_id to id framebuffera.

binduje renderbuffer. renderbuffer_id to id renderbuffera.

tworzy teksture na karcie graficznej. width i height to wymiary, format to format pikseli. zwraca id nowej tekstury.

usuwa teksture o podanym id.

przesyla dane tekstury do GPU. texture_id to id tekstury, data to wskaznik na dane w pamieci, size to rozmiar w bajtach.

generuje mipmapy dla tekstury. texture_id to id tekstury.

ustawia filtry tekstury. texture_id to id tekstury, min_filter to filtr minimalizacji, mag_filter to filtr powiekienia.

ustawia owijanie tekstury (wrap). texture_id to id tekstury, wrap_s i wrap_t to tryby owijania osi S i T.

tworzy renderbuffer. width i height to wymiary, format to format. zwraca id renderbuffera.

usuwa renderbuffer o podanym id.

tworzy framebuffer. zwraca id nowego framebuffera.

usuwa framebuffer o podanym id.

przypisuje teksture do framebuffera jako attachment. framebuffer_id to id framebuffera, attachment to punkt przyczepienia (np. COLOR_ATTACHMENT0), texture_id to id tekstury, mip_level to poziom mipmapy.

przypisuje renderbuffer do framebuffera. framebuffer_id, attachment, renderbuffer_id.

ustawia bufora rysowania (draw buffers). count to liczba attachmenetow, attachments to tablica id attachmenetow.

ustawia bufor odczytu (read buffer). attachment to punkt przyczepienia od ktorego bedzie odczyt.

kopiowanie obszaru miedzy framebufferami (blit). src_x0, src_y0, src_x1, src_y1 definuja zrodlo, dst_x0, dst_y0, dst_x1, dst_y1 definuja cel, mask to maska buforow (color, depth, stencil), filter to filtr skalowania.

odczytuje piksele z framebuffera do pamieci. x, y, width, height definuja obszar, format i type definuja format wyjsciowy, pixels to wskaznik na bufor docelowy w pamieci.

czeka na zakonczenie wszystkich operacji GPU (finish). wlacza barrier i czeka az Command Processor zakonczy wszystkie zadania.

wyswietla wyrenderowana klatke (flush). wysyla komende flush do Command Processor.

zwraca status rejestru karty graficznej. zwraca 32-bitowy rejestr statusu z MMIO.

zwraca taktowanie rdzenia GPU (engine clock) w Hz.

zwraca taktowanie pamieci VRAM (memory clock) w Hz.

zwraca rozmiar pamieci VRAM w bajtach.

zwraca liczbe rdzeni CUDA (GP106 ma 1280 rdzeni CUDA).

zwraca liczba multiprocesorow strumieniowych (SM) (GP106 ma 10 SM).

zwraca liczbe jednostek raster operacji (ROP) (GP106 ma 48 ROP).

zwraca liczbe jednostek tekstur (GP106 ma 80 jednostek tekstur).

zwraca szerokosc magistrali pamieci w bitach (GP106 ma 192-bit).

zwraca rozmiar cache wierzcholkow w bajtach.

zwraca rozmiar bufora primitivow w bajtach.

zwraca rozmiar bufora komend w bajtach.

zwraca liczbe bledow sterownika (licznik bledow inicjalizacji i operacji MMIO).

sterownik jest rejestrowany w `kernel/kernel.c` w funkcji `kernel_main`:

```
driver_manager_register("gtx_1060", nvidia_gtx_1060_driver_init, 0, 0, 4);
```

Deklaracje funkcji C (z atrybutem `__attribute__((weak))` jako fallback gdy Rust jest niedostepny):

```
bool nvidia_gtx_1060_driver_init(void);
bool nvidia_gtx_1060_driver_detected(void);
```

Plik zrodlowy `rust/drivers/karty_graficzne/nvidia/gtx_1060.rs` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/nvidia/*.rs)`) i linkowany do jadra. Gdy `rustc` jest dostepny, silne symbole z obiektu Rust nadpisuja weak definicje C w `kernel/kernel.c`.

## karta graficzna - rust/drivers/karty_graficzne/nvidia/gtx_1070.rs

sterownik karty graficznej NVIDIA GeForce GTX 1070 (GP104, architektura Pascal) napisany w Rust. obsluguje pelny potok graficzny 2D i 3D, zaawansowana akceleracja 3D przez Command Processor (CP), przezroczystosc (alpha blending, alpha to coverage, blend equation, blend color, logic op), tekstury z mipmapami i filtrowaniem, shadery (vertex, fragment, geometry, compute, tessellation), uniformy (1f, 2f, 3f, 4f, matrix4fv), render targety, depth-stencil z depth bounds test, tessellation, stream output, zapytania okluzyjne, statystyki potoku, MSAA (multisampling), antialiasing, command buffer z ring bufferami (gfx, compute, dma) i doorbellami, presentation oraz depth test, stencil test, cull mode, wireframe, polygon offset, sample coverage, dither. sterownik opiera sie na architekturze Pascal (GP104, device id 0x1B81, vendor 0x10DE, 8GB VRAM, 1920 rdzeni CUDA, 20 SM, 64 ROP, 120 jednostek tekstur, 256-bit magistrala pamieci). napisany w Rust z `#[no_std]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/nvidia/*.rs)`) i linkowany do jadra.

### gtx_1070_init

inicjalizuje sterownik GTX 1070: wykrywa urzadzenie na PCI (vendor 0x10DE, urzadzenie 0x1B81), mapuje adresy MMIO (0xF0000000), VRAM (0xE0000000), GART (0xD0000000) i doorbelle (0xC0000000), wykonuje soft-reset Command Processor, inicjalizuje PMC, PGRAPH, FECS (context switch), GPCCS (zarzadzanie zasilaniem i zegarami), GPC, TPC, SM (shader multiprocessor), ROP, tekstury, shadery, compute, rasterizer, depth buffer, stencil, blend, color buffer (4 render targety), framebuffery, depth-stencil buffer, multisampling, antialiasing, viewport, scissor, vertex cache, index buffer, primitive buffer, uniform buffer, shader storage buffer, atomic counters, query objects, PCIe, power play (DPM, zegary, wentylator, temperatura), SMU (zarzadzanie zasilaniem GFX/UVD/VCE), ring buffery (gfx, compute, dma) oraz wlacza pelna akceleracje 3D. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

```
bool gtx_1070_init(void);
```

### gtx_1070_detected

sprawdza czy karta GTX 1070 jest wykryta na PCI (porownuje vendor id, device id i revisje). zwraca 1 gdy wykryta, 0 w przeciwnym razie.

```
bool gtx_1070_detected(void);
```

### gtx_1070_draw_triangle_3d

rysuje pojedynczy trojkat 3D na karcie graficznej. parametry x1,y1,z1, x2,y2,z2, x3,y3,z3 definiuja wierzcholki trojkata w przestrzeni 3D, color to kolor w formacie ARGB. komenda jest wysylana przez ring buffer Command Processor.

```
void gtx_1070_draw_triangle_3d(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, uint32_t color);
```

### gtx_1070_clear_screen_3d

czysci caly ekran 3D podanym kolorem. komenda jest wysylana przez Command Processor i oczekuje na zakonczenie operacji.

```
void gtx_1070_clear_screen_3d(uint32_t color);
```

### gtx_1070_set_viewport

ustawia viewport dla renderowania 3D. parametry x, y definiuja lewy gorny rog, width i height definiuja rozmiar obszaru renderowania.

```
void gtx_1070_set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
```

### gtx_1070_set_projection_matrix / gtx_1070_set_world_matrix / gtx_1070_set_view_matrix

ustawia macierz rzutowania, swiata lub widoku (4x4, podana jako wskaznik na `Matrix4x4`).

```
void gtx_1070_set_projection_matrix(const Matrix4x4 *matrix);
void gtx_1070_set_world_matrix(const Matrix4x4 *matrix);
void gtx_1070_set_view_matrix(const Matrix4x4 *matrix);
```

### gtx_1070_enable_depth_test / gtx_1070_enable_blending

wlacza lub wylacza test glebi lub blendowanie.

```
void gtx_1070_enable_depth_test(bool enabled);
void gtx_1070_enable_blending(bool enabled);
```

### gtx_1070_set_blend_func

ustawia funkcje blendowania zrodlo (src) i cel (dst).

```
void gtx_1070_set_blend_func(uint32_t src, uint32_t dst);
```

### gtx_1070_set_depth_func

ustawia funkcje porownania glebi.

```
void gtx_1070_set_depth_func(uint32_t func);
```

### gtx_1070_set_cull_mode

ustawia tryb cienia (culling) trojkatow.

```
void gtx_1070_set_cull_mode(uint32_t mode);
```

### gtx_1070_set_wireframe

wlacza lub wylacza tryb ramki drucianej (wireframe).

```
void gtx_1070_set_wireframe(bool enabled);
```

### gtx_1070_set_multisample_count / gtx_1070_enable_multisampling

ustawia liczbe probek multisampling lub wlacza/wylacza multisampling.

```
void gtx_1070_set_multisample_count(uint32_t count);
void gtx_1070_enable_multisampling(bool enabled);
```

### gtx_1070_enable_antialiasing

wlacza lub wylacza wygladzanie krawedzi (antialiasing).

```
void gtx_1070_enable_antialiasing(bool enabled);
```

### gtx_1070_finish / gtx_1070_flush

czeka az GPU zakonczy wszystkie operacje (finish) lub wymusza wykonanie oczekujacych polecen (flush).

```
void gtx_1070_finish(void);
void gtx_1070_flush(void);
```

### gtx_1070_get_status

zwraca aktualny status rejestru karty graficznej.

```
uint32_t gtx_1070_get_status(void);
```

### Integracja z jadrem

Sterownik jest rejestrowany w `kernel/kernel.c` w funkcji `kernel_main`:

```
driver_manager_register("gtx_1070", gtx_1070_driver_init, 0, 0, 4);
```

Deklaracje funkcji C:

```
bool gtx_1070_driver_init(void);
bool gtx_1070_driver_detected(void);
```

Plik zrodlowy `rust/drivers/karty_graficzne/nvidia/gtx_1070.rs` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/nvidia/*.rs)`) i linkowany do jadra. Gdy `rustc` jest dostepny, silne symbole z obiektu Rust nadpisuja weak definicje C w `kernel/kernel.c`.

## pasek zadan - zaawansowane rozmycie gaussowskie (aero) - graficzny/rysowanie.c

Pasek zadan (`narysuj_pasek_zadan`) wykorzystuje zaawansowane rozmycie gaussowskie w stylu systemu Windows Aero. Tapeta wyswietlana pod paskiem jest rozmyta i nakladana na kolor paska z zadania, tworzac efekt przezroczystego, rozmytego szkola.

Algorytm rozmycia gaussowskiego:
1. Funkcja `tapeta_piksel(x, y)` pobiera piksel tapety z podanej pozycji z obsluga krawedzi (clamping) i obsluga trzech trybow tapety: wczytany bitmapa RAW, gradientowy i kolor stalowy.
2. Funkcja `rozmyj_pasek_zadan()` wykonuje dwukierunkowe separowalne rozmycie gaussowskie z 9-elementowym jadrem gaussa: {1, 4, 9, 14, 16, 14, 9, 4, 1} o sumie 72.
3. Najpierw wykonuje sie rozmycie w kierunku poziomym (prawo-lewo) dla kazdego wiersza paska z zadan, zapisujac wyniki do bufora tymczasowego `blur_temp`.
4. Nastepnie wykonuje sie rozmycie w kierunku pionowym (gora-dol) na buforze tymczasowym, lacznie z obsluga krawedzi paska (clamping do range 0..PASEK_ZADAN_WYSOKOSC-1).
5. Rozmyty obraz tapety jest laczony z kolorem paska (`KOLOR_PASKA_ZADAN`) za pomoca blendowania alfa.
6. Wartosc alpha jest zmienna wzdosupie osi pionowej paska (gradient alpha): na krawedziach paska alpha jest mniejsza (wiecej przezroczystosci), w srodku wieksza (mniej przezroczystosci), co daje naturalny efekt szkla aero.
7. Wszystkie operacje wykonuje sie na kanale RGB rozdzielczo, z dzieleniem przez 255 na koncu.

Wynikiem jest pasek z zadan ktory rozmywa tapete pod soba i ma przezroczysty, szklany wyglad w stylu aero, z ikonami minimalizowanych okien, zegarem, ikona internetu i ikona bluetooth rysowanymi na wierzchu rozmytego tla.

## rozmycie wnetrza okien - graficzny/okno.c

Biale tlo wnetrza okien (plikow, przegladarki, menedzera zadan itp.) rowniez korzysta z tego samego rozmycia gaussa co ramki okien i pasek zadan. Funkcja `narysuj_okno_bez_odswiezania` wykonuje nastepujace kroki:

1. `rozmyj_prostokat()` rozmywa obszar wnetrza okna z jadrem gaussa 5-elementowym {1, 4, 6, 4, 1}, co dynamicznie dostosowuje sie do tego co znajduje sie za oknem (tapeta, inne okna, pasek zadan).
2. `rysuj_prostokat_polprzezroczysty()` naklada biale tlo (`KOLOR_OKNA`) z alpha=70 na rozmyte tlo, co daje efekt szkla aero na wnetrzu okna.

Dzieki temu przesuwanie okna powoduje dynamiczna zmiane rozmycia w zaleznosci od tego co znajduje sie pod nim, a caly system zachowuje sprawnosc thanks to ograniczeniu blur do tylko obszaru wnetrza okna (nie calego ekranu) oraz uzyciu malego 5-elementowego jadra gaussa.

## przycisk menu start - graficzny/menu_start.c

Przycisk otwierajacy menu start rowniez korzysta z tego samego rozmycia gaussa co pasek zadan i ramki okien. Funkcja `menu_start_rysuj_przycisk` wywoluje `rozmyj_prostokat()` na obszarze przycisku przed narysowaniem jego tla (`KOLOR_PRZYCISKU_START`) z alpha=70, co daje spojny efekt szkla aero w calym interfejsie systemu.

## sterownik nvidia gtx 5090 - rust/drivers/karty_graficzne/nvidia/gtx_5090.rs

Zaawansowany sterownik karty graficznej NVIDIA GeForce GTX 5090 (architektura Blackwell, chip GB202) z obsluga zaawansowanej akceleracji 3D i shaderow. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury ring bufferow do przesylania komend do GPU.

### stalewi i adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzenia: `NVIDIA_VENDOR_ID` (0x10DE), `NVIDIA_GTX_5090_DEVICE_ID` (0x2A04), `CHIP_GB202` (0x2A04), `BLACKWELL_ARCH_VERSION` (2).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem i inicjalizacja), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, UVD, VCE, SDMA, CP, VGT, PA_SC, DB, CB.
- Rejestrow specyficznych dla Blackwell: ray tracing (`NV_RAY_TRACING_BASE`), mesh shader (`NV_MESH_SHADER_BASE`), tensor core (`NV_TENSOR_CORE_BASE`), DLSS (`NV_DLSS_BASE`), variable rate shading (`NV_VARIABLE_RATE_SHADING_BASE`), meshlet compression (`NV_MESHLET_COMPRESSION_BASE`), displacement micro map (`NV_DISPLACEMENT_MICRO_MAP_BASE`), opacity micro map (`NV_OPACITY_MICRO_MAP_BASE`), serialized write (`NV_SERIALIZED_WRITE_BASE`), shader execution reorder (`NV_SHADER_EXECUTION_REORDER_BASE`), path tracing (`NV_PATH_TRACING_BASE`), pipeline i shader module (`NV_GRAPHICS_PIPELINE_BASE`, `NV_RAY_TRACING_PIPELINE_BASE`, `NV_SHADER_MODULE_BASE`), descriptor set (`NV_DESCRIPTOR_SET_BASE`), command buffer (`NV_COMMAND_BUFFER_BASE`), query pool (`NV_QUERY_POOL_BASE`), swap chain (`NV_SWAP_CHAIN_BASE`).

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji zwiazanych z kamera, swiatem i rzutowaniem.
- `Vector4`, `Vector3`, `Vector2` - wektory dla pozycji, koloru, tekstur, normalnych, tangentow i bitangentow.
- `Vertex` - struktura wierzcholka z pozycja, kolorem, tekstura, normalna, tangentem i bitangentem.
- `Color` - kolor RGBA w formacie float.
- `Rect`, `Viewport`, `Scissor` - prostokaty, widoki i области odciecia.
- `BufferDescriptor`, `TextureDescriptor` - deskryptory buforow i tekstur.
- `ShaderProgram` - program cieniujacy z adresami shaderow vertex, fragment, geometry, tessellation, compute, mesh, task, ray generation, miss, closest hit, any hit, intersection oraz offsetami uniformow i deskryptorow.
- `RingBuffer` - kolejek cyklicznych dla komend GFX, compute i DMA z atomowymi wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - stan procesora komend (ME, queue, prefetch).
- `ShaderModule`, `DescriptorSet`, `CommandBuffer`, `QueryPool`, `SwapChain` - obiekty pipeline Vulkanopodobnego API.
- `RayTracingPipeline`, `MeshShader` - potoki ray tracing i mesh shader.
- `GpuInstance` - glowna instancja GPU z bazami MMIO/VRAM/GART/doorbell, rozmiarem VRAM, zegarami, liczba rdzeni CUDA, SM, ROP, jednostek tekstur, szerokosci magistrali, tensor core, RT core, DLSS, flagami funkcji Blackwell (VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), tablicami shader moduli, deskryptorow, buforow komend, pul zapytan, swap chainow, potokow ray tracing i mesh shader.

### inicjalizacja

- `gtx_5090_init()` -> bool - inicjalizuje sterownik: wykrywa karte, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU, ray tracing, mesh shader, tensor core, DLSS, VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), inicjalizuje ring buffer, wlacza akceleracje 3D.
- `gtx_5090_detected()` -> bool - sprawdza czy karta GTX 5090 jest wykrywalna przez odczyt rejestrow vendor/device/revision.
- `gtx_5090_driver_init()` / `gtx_5090_driver_detected()` - eksportowane funkcje C dla wywolania z jadra (no_mangle, extern "C").

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `read_mmio_64(offset: u32) -> u64` - odczyt 64-bitowy (dwa rejestry 32-bitowe).
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### ring buffer i doorbell

- `init_ring_buffers()` - konfiguruje bazy i rejestry procesora komend oraz SDMA dla tresci ring bufferow GFX, compute i DMA.
- `submit_doorbell(ring_index: u32, value: u32)` - wysyla sygnal doorbell do wybranego ring bufferu (0=GFX, 1=compute, 2=DMA).
- `ring_write(ring_index: u32, dword: u32)` - zapisuje slowo do ring bufferu z atomowa aktualizacja wptr.
- `ring_submit(ring_index: u32)` - zatwierdzania ring bufferu przez zapis RPTR i doorbell.

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych.
- `submit_3d_command(command, param1, param2, param3)` - przesyla komende 3D z trzema parametrami do GPU.
- `draw_triangle_3d(x1,y1,z1,x2,y2,z2,x3,y3,z3,color)` - rysuje trojkat 3D o podanych wierzcholkach i kolorze.
- `draw_line_3d(x1,y1,z1,x2,y2,z2,color)` - rysuje linie 3D.
- `clear_screen_3d(color)` - czysci ekran podanym kolorem.
- `set_viewport(x,y,width,height)` - ustawia viewport renderowania.
- `set_projection_matrix(matrix)` / `set_world_matrix(matrix)` / `set_view_matrix(matrix)` - ustawia macierze rzutowania, swiata i widoku (16 floatow kazda).
- `enable_depth_test(enabled)` / `set_depth_func(func)` - konfiguracja testu bufora glebokosci.
- `enable_blending(enabled)` / `set_blend_func(src,dst)` / `set_blend_equation(rgb,alpha)` / `set_blend_color(color)` - konfiguracja mieszania kolorow.
- `set_cull_mode(mode)` / `set_wireframe(enabled)` - tryb wycinania i ramki druciane.
- `set_multisample_count(count)` / `enable_multisampling(enabled)` / `enable_antialiasing(enabled)` - MSAA i antialiasing.
- `set_depth_bounds_test(enabled,min,max)` - test granic glebokosci.
- `set_stencil_test(enabled,func,ref,mask)` / `set_stencil_op(fail,zfail,zpass)` - test i operacje stencila.
- `set_logic_op(op,enabled)` / `set_dither(enabled)` - operacje logiczne i dithering.
- `set_polygon_offset(enabled,factor,units)` / `set_sample_coverage(value,invert)` - offset polygonu i probkowanie.
- `set_vertex_array(buffer,stride,offset)` / `set_index_array(buffer,format,offset)` - tablice wierzcholkow i indeksow.
- `draw_arrays(mode,first,count)` / `draw_elements(mode,count,type,offset)` / `draw_instanced(...)` / `draw_elements_instanced(...)` - rysowanie tablic i elementow, w tym instancjonowane.
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `wait_idle_pub()` - status GPU i oczekiwanie na bezczynnosc.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform_1f/2f/3f/4f(location, value...)` - ustawia uniformy skalarnie/wektorowe.
- `set_uniform_matrix4fv(location, matrix)` - ustawia uniform macierzy 4x4.

### tekstury i framebuffery

- `bind_texture(unit, texture_id)` / `bind_framebuffer(id)` / `bind_renderbuffer(id)` - wiazanie tekstur, framebufferow i renderbufferow do jednostek.
- `create_texture(width,height,format) -> u32` / `delete_texture(id)` - tworzenie i usuwanie tekstur.
- `upload_texture_data(texture_id, data, size)` - przesylanie danych tekstury do VRAM.
- `generate_mipmaps(texture_id)` - generowanie mipmap.
- `set_texture_filter(texture_id, min_filter, mag_filter)` / `set_texture_wrap(texture_id, wrap_s, wrap_t)` - filtry i owijanie tekstur.
- `create_renderbuffer(width,height,format) -> u32` / `delete_renderbuffer(id)` - renderbuffery.
- `create_framebuffer() -> u32` / `delete_framebuffer(id)` - framebuffery.
- `attach_texture_to_framebuffer(fbo, attachment, texture_id, mip_level)` / `attach_renderbuffer_to_framebuffer(fbo, attachment, rbo_id)` - podlaczenie tekstur/renderbufferow do framebufferu.
- `set_draw_buffers(count, attachments)` / `set_read_buffer(attachment)` - buforowe rysowania i odczytu.
- `blit_framebuffer(src_x0,y0,x1,y1, dst_x0,y0,x1,y1, mask, filter)` - blitowanie miedzy framebufferami.
- `read_pixels(x,y,width,height,format,type,pixels)` - odczyt pikseli z framebufferu.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.
- `tensor_cores()` / `rt_cores()` / `dlss_units()` - liczba tensor core, RT core i jednostek DLSS.
- `variable_rate_shading()` / `meshlet_compression()` / `displacement_micro_map()` / `opacity_micro_map()` / `serialized_write()` / `shader_execution_reorder()` / `path_tracing()` - flagi funkcji specyficznych dla Blackwell.

### zarzadzanie shaderami i pipeline (Blackwell)

- `create_shader_module(spirv_addr, spirv_size, entry_point_addr, entry_point_size, stage) -> u32` - tworzy modul cieniujacy z kodu SPIR-V.
- `destroy_shader_module(id)` - niszczy modul cieniujacy.
- `create_descriptor_set(addr, size, binding_count, bindings) -> u32` - tworzy zbior deskryptorow.
- `create_command_buffer(addr, size) -> u32` - tworzy bufor komend.
- `create_query_pool(addr, count, query_type, results_addr) -> u32` - tworzy pule zapytan.
- `create_swap_chain(image_count, image_addr, image_size, image_format) -> u32` - tworzy lancuch swap (swap chain).
- `create_ray_tracing_pipeline(shader_group_count, shader_group_addrs, shader_group_sizes, max_recursion_depth, traversal_addr, traversal_size, sbt_addr, sbt_size, ray_gen_offset, miss_offset, hit_group_offset) -> u32` - tworzy potok ray tracing z grupami shaderow i tabela wiazan.
- `create_mesh_shader(task_shader_addr, task_shader_size, mesh_shader_addr, mesh_shader_size, amplification_shader_addr, amplification_shader_size, vertex_attr_count, vertex_attr_addrs) -> u32` - tworzy mesh shader z shaderami task, mesh i amplification.

### operacje Blackwell specyficzne

- `trace_rays(pipeline_id, width, height, depth)` - rozpoczyna sledzenie promieni (ray tracing) dla podanych wymiarow.
- `dispatch_mesh(mesh_id, task_count_x, task_count_y, task_count_z)` - wysyla mesh shader do wykonania z podana liczba taskow.
- `tensor_op(mma_addr, dot_addr, accum_addr, m, n, k)` - operacja macierzowa na tensor core (MMA, DOT, ACCUM).
- `dlss_upscale(input_addr, output_addr, input_w, input_h, output_w, output_h)` - skalowanie DLSS (super resolution, ray reconstruction, frame generation).
- `path_trace(pipeline_id, samples, bounces, denoise)` - sciezkowe renderowanie (path tracing) z podana liczba probek, odbic i opcjonalnym denoisingiem.

### globalna instancja

- `gpu_instance() -> &'static mut GpuInstance` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton).

## sterownik nvidia gtx 5080 - rust/drivers/karty_graficzne/nvidia/gtx_5080.rs

Zaawansowany sterownik karty graficznej NVIDIA GeForce GTX 5080 (architektura Blackwell, chip GB203) z obsluga zaawansowanej akceleracji 3D i shaderow. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury ring bufferow do przesylania komend do GPU. Jest to wariant o nizszej wydajnosci niz GTX 5090, ale z tym samym zestawem funkcji architektonicznych Blackwell.

### stalewi i adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzenia: `NVIDIA_VENDOR_ID` (0x10DE), `NVIDIA_GTX_5080_DEVICE_ID` (0x2A02), `CHIP_GB203` (0x2A02), `BLACKWELL_ARCH_VERSION` (2).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem i inicjalizacja), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, UVD, VCE, SDMA, CP, VGT, PA_SC, DB, CB.
- Rejestrow specyficznych dla Blackwell: ray tracing (`NV_RAY_TRACING_BASE`), mesh shader (`NV_MESH_SHADER_BASE`), tensor core (`NV_TENSOR_CORE_BASE`), DLSS (`NV_DLSS_BASE`), variable rate shading (`NV_VARIABLE_RATE_SHADING_BASE`), meshlet compression (`NV_MESHLET_COMPRESSION_BASE`), displacement micro map (`NV_DISPLACEMENT_MICRO_MAP_BASE`), opacity micro map (`NV_OPACITY_MICRO_MAP_BASE`), serialized write (`NV_SERIALIZED_WRITE_BASE`), shader execution reorder (`NV_SHADER_EXECUTION_REORDER_BASE`), path tracing (`NV_PATH_TRACING_BASE`), pipeline i shader module (`NV_GRAPHICS_PIPELINE_BASE`, `NV_RAY_TRACING_PIPELINE_BASE`, `NV_SHADER_MODULE_BASE`), descriptor set (`NV_DESCRIPTOR_SET_BASE`), command buffer (`NV_COMMAND_BUFFER_BASE`), query pool (`NV_QUERY_POOL_BASE`), swap chain (`NV_SWAP_CHAIN_BASE`).

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji zwiazanych z kamera, swiatem i rzutowaniem.
- `Vector4`, `Vector3`, `Vector2` - wektory dla pozycji, koloru, tekstur, normalnych, tangentow i bitangentow.
- `Vertex` - struktura wierzcholka z pozycja, kolorem, tekstura, normalna, tangentem i bitangentem.
- `Color` - kolor RGBA w formacie float.
- `Rect`, `Viewport`, `Scissor` - prostokaty, widoki i области odciecia.
- `BufferDescriptor`, `TextureDescriptor` - deskryptory buforow i tekstur.
- `ShaderProgram` - program cieniujacy z adresami shaderow vertex, fragment, geometry, tessellation, compute, mesh, task, ray generation, miss, closest hit, any hit, intersection oraz offsetami uniformow i deskryptorow.
- `RingBuffer` - kolejek cyklicznych dla komend GFX, compute i DMA z atomowymi wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - stan procesora komend (ME, queue, prefetch).
- `ShaderModule`, `DescriptorSet`, `CommandBuffer`, `QueryPool`, `SwapChain` - obiekty pipeline Vulkanopodobnego API.
- `RayTracingPipeline`, `MeshShader` - potoki ray tracing i mesh shader.
- `GpuInstance` - glowna instancja GPU z bazami MMIO/VRAM/GART/doorbell, rozmiarem VRAM, zegarami, liczba rdzeni CUDA, SM, ROP, jednostek tekstur, szerokosci magistrali, tensor core, RT core, DLSS, flagami funkcji Blackwell (VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), tablicami shader moduli, deskryptorow, buforow komend, pul zapytan, swap chainow, potokow ray tracing i mesh shader.

### inicjalizacja

- `gtx_5080_init()` -> bool - inicjalizuje sterownik: wykrywa karte, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU, ray tracing, mesh shader, tensor core, DLSS, VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), inicjalizuje ring buffer, wlacza akceleracje 3D.
- `gtx_5080_detected()` -> bool - sprawdza czy karta GTX 5080 jest wykrywalna przez odczyt rejestrow vendor/device/revision.
- `gtx_5080_driver_init()` / `gtx_5080_driver_detected()` - eksportowane funkcje C dla wywolania z jadra (no_mangle, extern "C").

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `read_mmio_64(offset: u32) -> u64` - odczyt 64-bitowy (dwa rejestry 32-bitowe).
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### ring buffer i doorbell

- `init_ring_buffers()` - konfiguruje bazy i rejestry procesora komend oraz SDMA dla tresci ring bufferow GFX, compute i DMA.
- `submit_doorbell(ring_index: u32, value: u32)` - wysyla sygnal doorbell do wybranego ring bufferu (0=GFX, 1=compute, 2=DMA).
- `ring_write(ring_index: u32, dword: u32)` - zapisuje slowo do ring bufferu z atomowa aktualizacja wptr.
- `ring_submit(ring_index: u32)` - zatwierdza ring bufferu przez zapis RPTR i doorbell.

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych.
- `submit_3d_command(command, param1, param2, param3)` - przesyla komende 3D z trzema parametrami do GPU.
- `draw_triangle_3d(x1,y1,z1,x2,y2,z2,x3,y3,z3,color)` - rysuje trojkat 3D o podanych wierzcholkach i kolorze.
- `draw_line_3d(x1,y1,z1,x2,y2,z2,color)` - rysuje linie 3D.
- `clear_screen_3d(color)` - czysci ekran podanym kolorem.
- `set_viewport(x,y,width,height)` - ustawia viewport renderowania.
- `set_projection_matrix(matrix)` / `set_world_matrix(matrix)` / `set_view_matrix(matrix)` - ustawia macierze rzutowania, swiata i widoku (16 floatow kazda).
- `enable_depth_test(enabled)` / `set_depth_func(func)` - konfiguracja testu bufora glebokosci.
- `enable_blending(enabled)` / `set_blend_func(src,dst)` / `set_blend_equation(rgb,alpha)` / `set_blend_color(color)` - konfiguracja mieszania kolorow.
- `set_cull_mode(mode)` / `set_wireframe(enabled)` - tryb wycinania i ramki druciane.
- `set_multisample_count(count)` / `enable_multisampling(enabled)` / `enable_antialiasing(enabled)` - MSAA i antialiasing.
- `set_depth_bounds_test(enabled,min,max)` - test granic glebokosci.
- `set_stencil_test(enabled,func,ref,mask)` / `set_stencil_op(fail,zfail,zpass)` - test i operacje stencila.
- `set_logic_op(op,enabled)` / `set_dither(enabled)` - operacje logiczne i dithering.
- `set_polygon_offset(enabled,factor,units)` / `set_sample_coverage(value,invert)` - offset polygonu i probkowanie.
- `set_vertex_array(buffer,stride,offset)` / `set_index_array(buffer,format,offset)` - tablice wierzcholkow i indeksow.
- `draw_arrays(mode,first,count)` / `draw_elements(mode,count,type,offset)` / `draw_instanced(...)` / `draw_elements_instanced(...)` - rysowanie tablic i elementow, w tym instancjonowane.
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `wait_idle_pub()` - status GPU i oczekiwanie na bezczynnosc.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform_1f/2f/3f/4f(location, value...)` - ustawia uniformy skalarnie/wektorowe.
- `set_uniform_matrix4fv(location, matrix)` - ustawia uniform macierzy 4x4.

### tekstury i framebuffery

- `bind_texture(unit, texture_id)` / `bind_framebuffer(id)` / `bind_renderbuffer(id)` - wiazanie tekstur, framebufferow i renderbufferow do jednostek.
- `create_texture(width,height,format) -> u32` / `delete_texture(id)` - tworzenie i usuwanie tekstur.
- `upload_texture_data(texture_id, data, size)` - przesylanie danych tekstury do VRAM.
- `generate_mipmaps(texture_id)` - generowanie mipmap.
- `set_texture_filter(texture_id, min_filter, mag_filter)` / `set_texture_wrap(texture_id, wrap_s, wrap_t)` - filtry i owijanie tekstur.
- `create_renderbuffer(width,height,format) -> u32` / `delete_renderbuffer(id)` - renderbuffery.
- `create_framebuffer() -> u32` / `delete_framebuffer(id)` - framebuffery.
- `attach_texture_to_framebuffer(fbo, attachment, texture_id, mip_level)` / `attach_renderbuffer_to_framebuffer(fbo, attachment, rbo_id)` - podlaczenie tekstur/renderbufferow do framebufferu.
- `set_draw_buffers(count, attachments)` / `set_read_buffer(attachment)` - buforowe rysowania i odczytu.
- `blit_framebuffer(src_x0,y0,x1,y1, dst_x0,y0,x1,y1, mask, filter)` - blitowanie miedzy framebufferami.
- `read_pixels(x,y,width,height,format,type,pixels)` - odczyt pikseli z framebufferu.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.
- `tensor_cores()` / `rt_cores()` / `dlss_units()` - liczba tensor core, RT core i jednostek DLSS.
- `variable_rate_shading()` / `meshlet_compression()` / `displacement_micro_map()` / `opacity_micro_map()` / `serialized_write()` / `shader_execution_reorder()` / `path_tracing()` - flagi funkcji specyficznych dla Blackwell.

### zarzadzanie shaderami i pipeline (Blackwell)

- `create_shader_module(spirv_addr, spirv_size, entry_point_addr, entry_point_size, stage) -> u32` - tworzy modul cieniujacy z kodu SPIR-V.
- `destroy_shader_module(id)` - niszczy modul cieniujacy.
- `create_descriptor_set(addr, size, binding_count, bindings) -> u32` - tworzy zbior deskryptorow.
- `create_command_buffer(addr, size) -> u32` - tworzy bufor komend.
- `create_query_pool(addr, count, query_type, results_addr) -> u32` - tworzy pule zapytan.
- `create_swap_chain(image_count, image_addr, image_size, image_format) -> u32` - tworzy lancuch swap (swap chain).
- `create_ray_tracing_pipeline(shader_group_count, shader_group_addrs, shader_group_sizes, max_recursion_depth, traversal_addr, traversal_size, sbt_addr, sbt_size, ray_gen_offset, miss_offset, hit_group_offset) -> u32` - tworzy potok ray tracing z grupami shaderow i tabela wiazan.
- `create_mesh_shader(task_shader_addr, task_shader_size, mesh_shader_addr, mesh_shader_size, amplification_shader_addr, amplification_shader_size, vertex_attr_count, vertex_attr_addrs) -> u32` - tworzy mesh shader z shaderami task, mesh i amplification.

### operacje Blackwell specyficzne

- `trace_rays(pipeline_id, width, height, depth)` - rozpoczyna sledzenie promieni (ray tracing) dla podanych wymiarow.
- `dispatch_mesh(mesh_id, task_count_x, task_count_y, task_count_z)` - wysyla mesh shader do wykonania z podana liczba taskow.
- `tensor_op(mma_addr, dot_addr, accum_addr, m, n, k)` - operacja macierzowa na tensor core (MMA, DOT, ACCUM).
- `dlss_upscale(input_addr, output_addr, input_w, input_h, output_w, output_h)` - skalowanie DLSS (super resolution, ray reconstruction, frame generation).
- `path_trace(pipeline_id, samples, bounces, denoise)` - sciezkowe renderowanie (path tracing) z podana liczba probek, odbic i opcjonalnym denoisingiem.

### globalna instancja

- `gpu_instance() -> &'static mut GpuInstance` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton).

## sterownik nvidia gtx 5070 - rust/drivers/karty_graficzne/nvidia/gtx_5070.rs

Zaawansowany sterownik karty graficznej NVIDIA GeForce GTX 5070 (architektura Blackwell, chip GB205) z obsluga zaawansowanej akceleracji 3D i shaderow. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury ring bufferow do przesylania komend do GPU. Jest to wariant o nizszej wydajnosci niz GTX 5080 i GTX 5090, ale z tym samym zestawem funkcji architektonicznych Blackwell.

### adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzenia: `NVIDIA_VENDOR_ID` (0x10DE), `NVIDIA_GTX_5070_DEVICE_ID` (0x2A03), `CHIP_GB205` (0x2A03), `BLACKWELL_ARCH_VERSION` (2).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem i inicjalizacja), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, UVD, VCE, SDMA, CP, VGT, PA_SC, DB, CB.
- Rejestrow specyficznych dla Blackwell: ray tracing (`NV_RAY_TRACING_BASE`), mesh shader (`NV_MESH_SHADER_BASE`), tensor core (`NV_TENSOR_CORE_BASE`), DLSS (`NV_DLSS_BASE`), variable rate shading (`NV_VARIABLE_RATE_SHADING_BASE`), meshlet compression (`NV_MESHLET_COMPRESSION_BASE`), displacement micro map (`NV_DISPLACEMENT_MICRO_MAP_BASE`), opacity micro map (`NV_OPACITY_MICRO_MAP_BASE`), serialized write (`NV_SERIALIZED_WRITE_BASE`), shader execution reorder (`NV_SHADER_EXECUTION_REORDER_BASE`), path tracing (`NV_PATH_TRACING_BASE`), pipeline i shader module (`NV_GRAPHICS_PIPELINE_BASE`, `NV_RAY_TRACING_PIPELINE_BASE`, `NV_SHADER_MODULE_BASE`), descriptor set (`NV_DESCRIPTOR_SET_BASE`), command buffer (`NV_COMMAND_BUFFER_BASE`), query pool (`NV_QUERY_POOL_BASE`), swap chain (`NV_SWAP_CHAIN_BASE`).

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji zwiazanych z kamera, swiatem i rzutowaniem.
- `Vector4`, `Vector3`, `Vector2` - wektory dla pozycji, koloru, tekstur, normalnych, tangentow i bitangentow.
- `Vertex` - struktura wierzcholka z pozycja, kolorem, tekstura, normalna, tangentem i bitangentem.
- `Color` - kolor RGBA w formacie float.
- `Rect`, `Viewport`, `Scissor` - prostokaty, widoki i oblasti odciecia.
- `BufferDescriptor`, `TextureDescriptor` - deskryptory buforow i tekstur.
- `ShaderProgram` - program cieniujacy z adresami shaderow vertex, fragment, geometry, tessellation, compute, mesh, task, ray generation, miss, closest hit, any hit, intersection oraz offsetami uniformow i deskryptorow.
- `RingBuffer` - kolejek cyklicznych dla komend GFX, compute i DMA z atomowymi wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - stan procesora komend (ME, queue, prefetch).
- `ShaderModule`, `DescriptorSet`, `CommandBuffer`, `QueryPool`, `SwapChain` - obiekty pipeline Vulkanopodobnego API.
- `RayTracingPipeline`, `MeshShader` - potoki ray tracing i mesh shader.
- `GpuInstance` - glowna instancja GPU z bazami MMIO/VRAM/GART/doorbell, rozmiarem VRAM, zegarami, liczba rdzeni CUDA, SM, ROP, jednostek tekstur, szerokosci magistrali, tensor core, RT core, DLSS, flagami funkcji Blackwell (VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), tablicami shader moduli, deskryptorow, buforow komend, pul zapytan, swap chainow, potokow ray tracing i mesh shader.

### inicjalizacja

- `gtx_5070_init()` -> bool - inicjalizuje sterownik: wykrywa karte, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU, ray tracing, mesh shader, tensor core, DLSS, VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), inicjalizuje ring buffer, wlacza akceleracje 3D.
- `gtx_5070_detected()` -> bool - sprawdza czy karta GTX 5070 jest wykrywalna przez odczyt rejestrow vendor/device/revision.
- `gtx_5070_driver_init()` / `gtx_5070_driver_detected()` - eksportowane funkcje C dla wywolania z jadra (no_mangle, extern "C").

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `read_mmio_64(offset: u32) -> u64` - odczyt 64-bitowy (dwa rejestry 32-bitowe).
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### ring buffer i doorbell

- `init_ring_buffers()` - konfiguruje bazy i rejestry procesora komend oraz SDMA dla tresci ring bufferow GFX, compute i DMA.
- `submit_doorbell(ring_index: u32, value: u32)` - wysyla sygnal doorbell do wybranego ring bufferu (0=GFX, 1=compute, 2=DMA).
- `ring_write(ring_index: u32, dword: u32)` - zapisuje slowo do ring bufferu z atomowa aktualizacja wptr.
- `ring_submit(ring_index: u32)` - zatwierdza ring bufferu przez zapis RPTR i doorbell.

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych.
- `submit_3d_command(command, param1, param2, param3)` - przesyla komende 3D z trzema parametrami do GPU.
- `draw_triangle_3d(x1,y1,z1,x2,y2,z2,x3,y3,z3,color)` - rysuje trojkat 3D o podanych wierzcholkach i kolorze.
- `draw_line_3d(x1,y1,z1,x2,y2,z2,color)` - rysuje linie 3D.
- `clear_screen_3d(color)` - czysci ekran podanym kolorem.
- `set_viewport(x,y,width,height)` - ustawia viewport renderowania.
- `set_projection_matrix(matrix)` / `set_world_matrix(matrix)` / `set_view_matrix(matrix)` - ustawia macierze rzutowania, swiata i widoku (16 floatow kazda).
- `enable_depth_test(enabled)` / `set_depth_func(func)` - konfiguracja testu bufora glebokosci.
- `enable_blending(enabled)` / `set_blend_func(src,dst)` / `set_blend_equation(rgb,alpha)` / `set_blend_color(color)` - konfiguracja mieszania kolorow.
- `set_cull_mode(mode)` / `set_wireframe(enabled)` - tryb wycinania i ramki druciane.
- `set_multisample_count(count)` / `enable_multisampling(enabled)` / `enable_antialiasing(enabled)` - MSAA i antialiasing.
- `set_depth_bounds_test(enabled,min,max)` - test granic glebokosci.
- `set_stencil_test(enabled,func,ref,mask)` / `set_stencil_op(fail,zfail,zpass)` - test i operacje stencila.
- `set_logic_op(op,enabled)` / `set_dither(enabled)` - operacje logiczne i dithering.
- `set_polygon_offset(enabled,factor,units)` / `set_sample_coverage(value,invert)` - offset polygonu i probkowanie.
- `set_vertex_array(buffer,stride,offset)` / `set_index_array(buffer,format,offset)` - tablice wierzcholkow i indeksow.
- `draw_arrays(mode,first,count)` / `draw_elements(mode,count,type,offset)` / `draw_instanced(...)` / `draw_elements_instanced(...)` - rysowanie tablic i elementow, w tym instancjonowane.
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `wait_idle_pub()` - status GPU i oczekiwanie na bezczynnosc.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform_1f/2f/3f/4f(location, value...)` - ustawia uniformy skalarnie/wektorowe.
- `set_uniform_matrix4fv(location, matrix)` - ustawia uniform macierzy 4x4.

### tekstury i framebuffery

- `bind_texture(unit, texture_id)` / `bind_framebuffer(id)` / `bind_renderbuffer(id)` - wiazanie tekstur, framebufferow i renderbufferow do jednostek.
- `create_texture(width,height,format) -> u32` / `delete_texture(id)` - tworzenie i usuwanie tekstur.
- `upload_texture_data(texture_id, data, size)` - przesylanie danych tekstury do VRAM.
- `generate_mipmaps(texture_id)` - generowanie mipmap.
- `set_texture_filter(texture_id, min_filter, mag_filter)` / `set_texture_wrap(texture_id, wrap_s, wrap_t)` - filtry i owijanie tekstur.
- `create_renderbuffer(width,height,format) -> u32` / `delete_renderbuffer(id)` - renderbuffery.
- `create_framebuffer() -> u32` / `delete_framebuffer(id)` - framebuffery.
- `attach_texture_to_framebuffer(fbo, attachment, texture_id, mip_level)` / `attach_renderbuffer_to_framebuffer(fbo, attachment, rbo_id)` - podlaczenie tekstur/renderbufferow do framebufferu.
- `set_draw_buffers(count, attachments)` / `set_read_buffer(attachment)` - buforowe rysowania i odczytu.
- `blit_framebuffer(src_x0,y0,x1,y1, dst_x0,y0,x1,y1, mask, filter)` - blitowanie miedzy framebufferami.
- `read_pixels(x,y,width,height,format,type,pixels)` - odczyt pikseli z framebufferu.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.
- `tensor_cores()` / `rt_cores()` / `dlss_units()` - liczba tensor core, RT core i jednostek DLSS.
- `variable_rate_shading()` / `meshlet_compression()` / `displacement_micro_map()` / `opacity_micro_map()` / `serialized_write()` / `shader_execution_reorder()` / `path_tracing()` - flagi funkcji specyficznych dla Blackwell.

### zarzadzanie shaderami i pipeline (Blackwell)

- `create_shader_module(spirv_addr, spirv_size, entry_point_addr, entry_point_size, stage) -> u32` - tworzy modul cieniujacy z kodu SPIR-V.
- `destroy_shader_module(id)` - niszczy modul cieniujacy.
- `create_descriptor_set(addr, size, binding_count, bindings) -> u32` - tworzy zbior deskryptorow.
- `create_command_buffer(addr, size) -> u32` - tworzy bufor komend.
- `create_query_pool(addr, count, query_type, results_addr) -> u32` - tworzy pule zapytan.
- `create_swap_chain(image_count, image_addr, image_size, image_format) -> u32` - tworzy lancuch swap (swap chain).
- `create_ray_tracing_pipeline(shader_group_count, shader_group_addrs, shader_group_sizes, max_recursion_depth, traversal_addr, traversal_size, sbt_addr, sbt_size, ray_gen_offset, miss_offset, hit_group_offset) -> u32` - tworzy potok ray tracing z grupami shaderow i tabela wiazan.
- `create_mesh_shader(task_shader_addr, task_shader_size, mesh_shader_addr, mesh_shader_size, amplification_shader_addr, amplification_shader_size, vertex_attr_count, vertex_attr_addrs) -> u32` - tworzy mesh shader z shaderami task, mesh i amplification.

### operacje Blackwell specyficzne

- `trace_rays(pipeline_id, width, height, depth)` - rozpoczyna sledzenie promieni (ray tracing) dla podanych wymiarow.
- `dispatch_mesh(mesh_id, task_count_x, task_count_y, task_count_z)` - wysyla mesh shader do wykonania z podana liczba taskow.
- `tensor_op(mma_addr, dot_addr, accum_addr, m, n, k)` - operacja macierzowa na tensor core (MMA, DOT, ACCUM).
- `dlss_upscale(input_addr, output_addr, input_w, input_h, output_w, output_h)` - skalowanie DLSS (super resolution, ray reconstruction, frame generation).
- `path_trace(pipeline_id, samples, bounces, denoise)` - sciezkowe renderowanie (path tracing) z podana liczba probek, odbic i opcjonalnym denoisingiem.

### globalna instancja

- `gpu_instance() -> &'static mut GpuInstance` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton).

## sterownik nvidia gtx 1050 - rust/drivers/karty_graficzne/nvidia/gtx_1050.rs

Zaawansowany sterownik karty graficznej NVIDIA GeForce GTX 1050 (architektura Pascal, chip GP107) z obsluga zaawansowanej akceleracji 3D i shaderow. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury ring bufferow do przesylania komend do GPU.

### stalewi i adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzenia: `NVIDIA_VENDOR_ID` (0x10DE), `NVIDIA_GTX_1050_DEVICE_ID` (0x1C86), `CHIP_GP107` (0x1C86), `PASCAL_ARCH_VERSION` (1).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem i inicjalizacja), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, UVD, VCE, SDMA, CP, VGT, PA_SC, DB, CB.

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji zwiazanych z kamera, swiatem i rzutowaniem.
- `Vector4`, `Vector3`, `Vector2` - wektory dla pozycji, koloru, tekstur, normalnych, tangentow i bitangentow.
- `Vertex` - struktura wierzcholka z pozycja, kolorem, tekstura, normalna, tangentem i bitangentem.
- `Color` - kolor RGBA w formacie float.
- `Rect`, `Viewport`, `Scissor` - prostokaty, widoki i oblasti odciecia.
- `BufferDescriptor`, `TextureDescriptor` - deskryptory buforow i tekstur.
- `ShaderProgram` - program cieniujacy z adresami shaderow vertex, fragment, geometry, tessellation, compute oraz offsetami uniformow.
- `RingBuffer` - kolejek cyklicznych dla komend GFX, compute i DMA z atomowymi wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - stan procesora komend (ME, queue, prefetch).
- `GpuInstance` - glowna instancja GPU z bazami MMIO/VRAM/GART/doorbell, rozmiarem VRAM, zegarami, liczba rdzeni CUDA, SM, ROP, jednostek tekstur, szerokosci magistrali, tablicami ring bufferow dla tresci komend.

### inicjalizacja

- `gtx_1050_init()` -> bool - inicjalizuje sterownik: wykrywa karte, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, depth/stencil buffer, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU), inicjalizuje ring buffer, wlacza akceleracje 3D.
- `gtx_1050_detected()` -> bool - sprawdza czy karta GTX 1050 jest wykrywalna przez odczyt rejestrow vendor/device/revision.
- `gtx_1050_driver_init()` / `gtx_1050_driver_detected()` - eksportowane funkcje C dla wywolania z jadra (no_mangle, extern "C").

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `read_mmio_64(offset: u32) -> u64` - odczyt 64-bitowy (dwa rejestry 32-bitowe).
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### ring buffer i doorbell

- `init_ring_buffers()` - konfiguruje bazy i rejestry procesora komend oraz SDMA dla tresci ring bufferow GFX, compute i DMA.
- `submit_doorbell(ring_index: u32, value: u32)` - wysyla sygnal doorbell do wybranego ring bufferu (0=GFX, 1=compute, 2=DMA).
- `ring_write(ring_index: u32, dword: u32)` - zapisuje slowo do ring bufferu z atomowa aktualizacja wptr.
- `ring_submit(ring_index: u32)` - zatwierdza ring bufferu przez zapis RPTR i doorbell.

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych.
- `submit_3d_command(command, param1, param2, param3)` - przesyla komende 3D z trzema parametrami do GPU.
- `draw_triangle_3d(x1,y1,z1,x2,y2,z2,x3,y3,z3,color)` - rysuje trojkat 3D o podanych wierzcholkach i kolorze.
- `draw_line_3d(x1,y1,z1,x2,y2,z2,color)` - rysuje linie 3D.
- `clear_screen_3d(color)` - czysci ekran podanym kolorem.
- `set_viewport(x,y,width,height)` - ustawia viewport renderowania.
- `set_projection_matrix(matrix)` / `set_world_matrix(matrix)` / `set_view_matrix(matrix)` - ustawia macierze rzutowania, swiata i widoku (16 floatow kazda).
- `enable_depth_test(enabled)` / `set_depth_func(func)` - konfiguracja testu bufora glebokosci.
- `enable_blending(enabled)` / `set_blend_func(src,dst)` / `set_blend_equation(rgb,alpha)` / `set_blend_color(color)` - konfiguracja mieszania kolorow.
- `set_cull_mode(mode)` / `set_wireframe(enabled)` - tryb wycinania i ramki druciane.
- `set_multisample_count(count)` / `enable_multisampling(enabled)` / `enable_antialiasing(enabled)` - MSAA i antialiasing.
- `set_depth_bounds_test(enabled,min,max)` - test granic glebokosci.
- `set_stencil_test(enabled,func,ref,mask)` / `set_stencil_op(fail,zfail,zpass)` - test i operacje stencila.
- `set_logic_op(op,enabled)` / `set_dither(enabled)` - operacje logiczne i dithering.
- `set_polygon_offset(enabled,factor,units)` / `set_sample_coverage(value,invert)` - offset polygonu i probkowanie.
- `set_vertex_array(buffer,stride,offset)` / `set_index_array(buffer,format,offset)` - tablice wierzcholkow i indeksow.
- `draw_arrays(mode,first,count)` / `draw_elements(mode,count,type,offset)` / `draw_instanced(...)` / `draw_elements_instanced(...)` - rysowanie tablic i elementow, w tym instancjonowane.
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `wait_idle_pub()` - status GPU i oczekiwanie na bezczynnosc.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform_1f/2f/3f/4f(location, value...)` - ustawia uniformy skalarnie/wektorowe.
- `set_uniform_matrix4fv(location, matrix)` - ustawia uniform macierzy 4x4.

### tekstury i framebuffery

- `bind_texture(unit, texture_id)` / `bind_framebuffer(id)` / `bind_renderbuffer(id)` - wiazanie tekstur, framebufferow i renderbufferow do jednostek.
- `create_texture(width,height,format) -> u32` / `delete_texture(id)` - tworzenie i usuwanie tekstur.
- `upload_texture_data(texture_id, data, size)` - przesylanie danych tekstury do VRAM.
- `generate_mipmaps(texture_id)` - generowanie mipmap.
- `set_texture_filter(texture_id, min_filter, mag_filter)` / `set_texture_wrap(texture_id, wrap_s, wrap_t)` - filtry i owijanie tekstur.
- `create_renderbuffer(width,height,format) -> u32` / `delete_renderbuffer(id)` - renderbuffery.
- `create_framebuffer() -> u32` / `delete_framebuffer(id)` - framebuffery.
- `attach_texture_to_framebuffer(fbo, attachment, texture_id, mip_level)` / `attach_renderbuffer_to_framebuffer(fbo, attachment, rbo_id)` - podlaczenie tekstur/renderbufferow do framebufferu.
- `set_draw_buffers(count, attachments)` / `set_read_buffer(attachment)` - buforowe rysowania i odczytu.
- `blit_framebuffer(src_x0,y0,x1,y1, dst_x0,y0,x1,y1, mask, filter)` - blitowanie miedzy framebufferami.
- `read_pixels(x,y,width,height,format,type,pixels)` - odczyt pikseli z framebufferu.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.

### globalna instancja

- `gpu_instance() -> &'static mut GpuInstance` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton).

## sterownik nvidia gtx 1040 - rust/drivers/karty_graficzne/nvidia/gtx_1040.rs

Zaawansowany sterownik karty graficznej NVIDIA GeForce GTX 1040 (architektura Pascal, chip GP107) z obsluga zaawansowanej akceleracji 3D i shaderow. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury ring bufferow do przesylania komend do GPU. Jest to wariant o nizszej wydajnosci niz GTX 1050, ale z tym samym zestawem funkcji architektonicznych Pascala.

### stale i i adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzenia: `NVIDIA_VENDOR_ID` (0x10DE), `NVIDIA_GTX_1040_DEVICE_ID` (0x1C82), `CHIP_GP107` (0x1C82), `PASCAL_ARCH_VERSION` (1).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem i inicjalizacja), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, UVD, VCE, SDMA, CP, VGT, PA_SC, DB, CB.

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji zwiazanych z kamera, swiatem i rzutowaniem.
- `Vector4`, `Vector3`, `Vector2` - wektory dla pozycji, koloru, tekstur, normalnych, tangentow i bitangentow.
- `Vertex` - struktura wierzcholka z pozycja, kolorem, tekstura, normalna, tangentem i bitangentem.
- `Color` - kolor RGBA w formacie float.
- `Rect`, `Viewport`, `Scissor` - prostokaty, widoki i oblasti odciecia.
- `BufferDescriptor`, `TextureDescriptor` - deskryptory buforow i tekstur.
- `ShaderProgram` - program cieniujacy z adresami shaderow vertex, fragment, geometry, tessellation, compute oraz offsetami uniformow.
- `RingBuffer` - kolejek cyklicznych dla komend GFX, compute i DMA z atomowymi wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - stan procesora komend (ME, queue, prefetch).
- `GpuInstance` - glowna instancja GPU z bazami MMIO/VRAM/GART/doorbell, rozmiarem VRAM, zegarami, liczba rdzeni CUDA, SM, ROP, jednostek tekstur, szerokosci magistrali, tablicami ring bufferow dla tresci komend.

### inicjalizacja

- `gtx_1040_init()` -> bool - inicjalizuje sterownik: wykrywa karte, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, depth/stencil buffer, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU), inicjalizuje ring buffer, wlacza akceleracje 3D.
- `gtx_1040_detected()` -> bool - sprawdza czy karta GTX 1040 jest wykrywalna przez odczyt rejestrow vendor/device/revision.
- `gtx_1040_driver_init()` / `gtx_1040_driver_detected()` - eksportowane funkcje C dla wywolania z jadra (no_mangle, extern "C").

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `read_mmio_64(offset: u32) -> u64` - odczyt 64-bitowy (dwa rejestry 32-bitowe).
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### ring buffer i doorbell

- `init_ring_buffers()` - konfiguruje bazy i rejestry procesora komend oraz SDMA dla tresci ring bufferow GFX, compute i DMA.
- `submit_doorbell(ring_index: u32, value: u32)` - wysyla sygnal doorbell do wybranego ring bufferu (0=GFX, 1=compute, 2=DMA).
- `ring_write(ring_index: u32, dword: u32)` - zapisuje slowo do ring bufferu z atomowa aktualizacja wptr.
- `ring_submit(ring_index: u32)` - zatwierdza ring bufferu przez zapis RPTR i doorbell.

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych.
- `submit_3d_command(command, param1, param2, param3)` - przesyla komende 3D z trzema parametrami do GPU.
- `draw_triangle_3d(x1,y1,z1,x2,y2,z2,x3,y3,z3,color)` - rysuje trojkat 3D o podanych wierzcholkach i kolorze.
- `draw_line_3d(x1,y1,z1,x2,y2,z2,color)` - rysuje linie 3D.
- `clear_screen_3d(color)` - czysci ekran podanym kolorem.
- `set_viewport(x,y,width,height)` - ustawia viewport renderowania.
- `set_projection_matrix(matrix)` / `set_world_matrix(matrix)` / `set_view_matrix(matrix)` - ustawia macierze rzutowania, swiata i widoku (16 floatow kazda).
- `enable_depth_test(enabled)` / `set_depth_func(func)` - konfiguracja testu bufora glebokosci.
- `enable_blending(enabled)` / `set_blend_func(src,dst)` / `set_blend_equation(rgb,alpha)` / `set_blend_color(color)` - konfiguracja mieszania kolorow.
- `set_cull_mode(mode)` / `set_wireframe(enabled)` - tryb wycinania i ramki druciane.
- `set_multisample_count(count)` / `enable_multisampling(enabled)` / `enable_antialiasing(enabled)` - MSAA i antialiasing.
- `set_depth_bounds_test(enabled,min,max)` - test granic glebokosci.
- `set_stencil_test(enabled,func,ref,mask)` / `set_stencil_op(fail,zfail,zpass)` - test i operacje stencila.
- `set_logic_op(op,enabled)` / `set_dither(enabled)` - operacje logiczne i dithering.
- `set_polygon_offset(enabled,factor,units)` / `set_sample_coverage(value,invert)` - offset polygonu i probkowanie.
- `set_vertex_array(buffer,stride,offset)` / `set_index_array(buffer,format,offset)` - tablice wierzcholkow i indeksow.
- `draw_arrays(mode,first,count)` / `draw_elements(mode,count,type,offset)` / `draw_instanced(...)` / `draw_elements_instanced(...)` - rysowanie tablic i elementow, w tym instancjonowane.
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `wait_idle_pub()` - status GPU i oczekiwanie na bezczynnosc.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform_1f/2f/3f/4f(location, value...)` - ustawia uniformy skalarnie/wektorowe.
- `set_uniform_matrix4fv(location, matrix)` - ustawia uniform macierzy 4x4.

### tekstury i framebuffery

- `bind_texture(unit, texture_id)` / `bind_framebuffer(id)` / `bind_renderbuffer(id)` - wiazanie tekstur, framebufferow i renderbufferow do jednostek.
- `create_texture(width,height,format) -> u32` / `delete_texture(id)` - tworzenie i usuwanie tekstur.
- `upload_texture_data(texture_id, data, size)` - przesylanie danych tekstury do VRAM.
- `generate_mipmaps(texture_id)` - generowanie mipmap.
- `set_texture_filter(texture_id, min_filter, mag_filter)` / `set_texture_wrap(texture_id, wrap_s, wrap_t)` - filtry i owijanie tekstur.
- `create_renderbuffer(width,height,format) -> u32` / `delete_renderbuffer(id)` - renderbuffery.
- `create_framebuffer() -> u32` / `delete_framebuffer(id)` - framebuffery.
- `attach_texture_to_framebuffer(fbo, attachment, texture_id, mip_level)` / `attach_renderbuffer_to_framebuffer(fbo, attachment, rbo_id)` - podlaczenie tekstur/renderbufferow do framebufferu.
- `set_draw_buffers(count, attachments)` / `set_read_buffer(attachment)` - buforowe rysowania i odczytu.
- `blit_framebuffer(src_x0,y0,x1,y1, dst_x0,y0,x1,y1, mask, filter)` - blitowanie miedzy framebufferami.
- `read_pixels(x,y,width,height,format,type,pixels)` - odczyt pikseli z framebufferu.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.

### globalna instancja

- `gpu_instance() -> &'static mut GpuInstance` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton).

## sterownik nvidia gtx 1030 - rust/drivers/karty_graficzne/nvidia/gtx_1030.rs

Zaawansowany sterownik karty graficznej NVIDIA GeForce GTX 1030 (architektura Pascal, chip GP108) z obsluga zaawansowanej akceleracji 3D i shaderow. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury do zarzadzania zasobami GPU (shader module, descriptor set, command buffer, query pool, swap chain). Karta GTX 1030 to wariant niskiej i sredniej klasy Pascala: 384 rdzenie CUDA, 3 SM, 16 ROP, 20 jednostek tekstur, 2GB VRAM (GDDR5), magistrala pamieci 64-bit, zegar rdzenia 1227 MHz, zegar pamieci 6000 MHz.

### stale i i adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzenia: `NVIDIA_VENDOR_ID` (0x10DE), `NVIDIA_GTX_1030_DEVICE_ID` (0x1D01), `CHIP_GP108` (0x1D01), `PASCAL_ARCH_VERSION` (1).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem i inicjalizacja), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, ray tracing, mesh shader, tensor core, DLSS, variable rate shading, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing, shader module, descriptor set, command buffer, query pool, swap chain, ray tracing pipeline.

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji zwiazanych z kamera, swiatem i rzutowaniem.
- `ShaderModule` - modul cieniujacy (SPIR-V) z adresem kodu, punktem wejscia, rozmiarami i etapem.
- `DescriptorSet` - zestaw deskryptorow z adresem, rozmiarem, liczba powiazan i tablica adresow powiazan.
- `CommandBuffer` - bufor komend z adresem, rozmiarem, atomowym offsetem i flaga wyslania.
- `QueryPool` - pula zapytan z adresem, liczba, typem i adresem wynikow.
- `SwapChain` - lancuch zamiany z liczba obrazow, aktualnym obrazem, adresami obrazow, rozmiarem, formatem i flagami acquired/presented.
- `RayTracingPipeline` - potok ray tracing z grupami shaderow, adresami, rozmiarami, maksymalna rekurencja, adresami traversal/SBT i offsetami.
- `MeshShader` - shader siatki z adresami i rozmiarami shaderow task, mesh i amplification oraz adresami atrybutow wierzcholkow.
- `GpuInstance` - glowna instancja GPU z identyfikatorami, zegarami, rozmiarem VRAM, liczba rdzeni CUDA/SM/ROP/jednostek tekstur, szerokosci magistrali, licznikiem bledow i tablicami zasobow (shadery, deskryptory, bufora komend, pule zapytan, lancuchy zamiany, potoki ray tracing, shadery siatki).

### inicjalizacja

- `gtx_1030_init()` -> bool - inicjalizuje sterownik: wykrywa karte, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, depth/stencil buffer, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU, ray tracing, mesh shader, tensor core, DLSS, VRS, meshlet compression, displacement/opacity micro map, serialized write, shader execution reorder, path tracing), wlacza akceleracje 3D.
- `gtx_1030_detected()` -> bool - sprawdza czy karta GTX 1030 jest wykrywalna przez odczyt rejestrow vendor/device.
- `gtx_1030_driver_init()` / `gtx_1030_driver_detected()` - eksportowane funkcje C dla wywolania z jadra (no_mangle, extern "C").

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych.
- `submit_3d_command(command, param1, param2, param3)` - przesyla komende 3D z trzema parametrami do GPU.
- `draw_triangle_3d(x1,y1,z1,x2,y2,z2,x3,y3,z3,color)` - rysuje trojkat 3D o podanych wierzcholkach i kolorze.
- `draw_line_3d(x1,y1,z1,x2,y2,z2,color)` - rysuje linie 3D.
- `clear_screen_3d(color)` - czysci ekran podanym kolorem.
- `set_viewport(x,y,width,height)` - ustawia viewport renderowania.
- `set_projection_matrix(matrix)` / `set_world_matrix(matrix)` / `set_view_matrix(matrix)` - ustawia macierze rzutowania, swiata i widoku (16 floatow kazda).
- `enable_depth_test(enabled)` / `set_depth_func(func)` - konfiguracja testu bufora glebokosci.
- `enable_blending(enabled)` / `set_blend_func(src,dst)` / `set_blend_equation(rgb,alpha)` / `set_blend_color(color)` - konfiguracja mieszania kolorow.
- `set_cull_mode(mode)` / `set_wireframe(enabled)` - tryb wycinania i ramki druciane.
- `set_multisample_count(count)` / `enable_multisampling(enabled)` / `enable_antialiasing(enabled)` - MSAA i antialiasing.
- `set_depth_bounds_test(enabled,min,max)` - test granic glebokosci.
- `set_stencil_test(enabled,func,ref,mask)` / `set_stencil_op(fail,zfail,zpass)` - test i operacje stencila.
- `set_logic_op(op,enabled)` / `set_dither(enabled)` - operacje logiczne i dithering.
- `set_polygon_offset(enabled,factor,units)` / `set_sample_coverage(value,invert)` - offset polygonu i probkowanie.
- `set_vertex_array(buffer,stride,offset)` / `set_index_array(buffer,format,offset)` - tablice wierzcholkow i indeksow.
- `draw_arrays(mode,first,count)` / `draw_elements(mode,count,type,offset)` / `draw_instanced(...)` / `draw_elements_instanced(...)` - rysowanie tablic i elementow, w tym instancjonowane.
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `wait_idle_pub()` - status GPU i oczekiwanie na bezczynnosc.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform_1f/2f/3f/4f(location, value...)` - ustawia uniformy skalarnie/wektorowe.
- `set_uniform_matrix4fv(location, matrix)` - ustawia uniform macierzy 4x4.
- `create_shader_module(spirv_addr, spirv_size, entry_point_addr, entry_point_size, stage) -> u32` - tworzy modul cieniujacy z kodu SPIR-V i punktu wejscia.
- `destroy_shader_module(id)` - niszczy modul cieniujacy.

### deskryptory, bufora komend, zapytania i lancuch zamiany

- `create_descriptor_set(addr, size, binding_count, bindings) -> u32` - tworzy zestaw deskryptorow.
- `create_command_buffer(addr, size) -> u32` - tworzy bufor komend.
- `create_query_pool(addr, count, query_type, results_addr) -> u32` - tworzy pule zapytan.
- `create_swap_chain(image_count, image_addr, image_size, image_format) -> u32` - tworzy lancuch zamiany.

### zaawansowane funkcje GPU (ray tracing, mesh shader, tensor, DLSS, path tracing)

- `create_ray_tracing_pipeline(...) -> u32` - tworzy potok ray tracing z grupami shaderow i maksymalna rekurencja.
- `create_mesh_shader(...) -> u32` - tworzy shader siatki (task/mesh/amplification).
- `trace_rays(pipeline_id, width, height, depth)` - rzuca promienie w potoku ray tracing.
- `dispatch_mesh(mesh_id, task_count_x, task_count_y, task_count_z)` - wysyla siatke do wykonania.
- `tensor_op(mma_addr, dot_addr, accum_addr, m, n, k)` - operacja macierzowa na tensorach (MMA/DOT/accumulate).
- `dlss_upscale(input_addr, output_addr, input_w, input_h, output_w, output_h)` - skalowanie DLSS.
- `path_trace(pipeline_id, samples, bounces, denoise)` - sciezkowe slonewanie z probami, odbiciami i denoizacja.

### tekstury i framebuffery

- `bind_texture(unit, texture_id)` / `bind_framebuffer(id)` / `bind_renderbuffer(id)` - wiazanie tekstur, framebufferow i renderbufferow do jednostek.
- `create_texture(width,height,format) -> u32` / `delete_texture(id)` - tworzenie i usuwanie tekstur.
- `upload_texture_data(texture_id, data, size)` - przesylanie danych tekstury do VRAM.
- `generate_mipmaps(texture_id)` - generowanie mipmap.
- `set_texture_filter(texture_id, min_filter, mag_filter)` / `set_texture_wrap(texture_id, wrap_s, wrap_t)` - filtry i owijanie tekstur.
- `create_renderbuffer(width,height,format) -> u32` / `delete_renderbuffer(id)` - renderbuffery.
- `create_framebuffer() -> u32` / `delete_framebuffer(id)` - framebuffery.
- `attach_texture_to_framebuffer(fbo, attachment, texture_id, mip_level)` / `attach_renderbuffer_to_framebuffer(fbo, attachment, rbo_id)` - podlaczenie tekstur/renderbufferow do framebufferu.
- `set_draw_buffers(count, attachments)` / `set_read_buffer(attachment)` - buforowe rysowania i odczytu.
- `blit_framebuffer(src_x0,y0,x1,y1, dst_x0,y0,x1,y1, mask, filter)` - blitowanie miedzy framebufferami.
- `read_pixels(x,y,width,height,format,type,pixels)` - odczyt pikseli z framebufferu.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.

### globalna instancja

- `gpu_instance() -> &'static mut GpuInstance` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton).

## sterownik nvidia gtx 1000 - rust/drivers/karty_graficzne/nvidia/gtx_1000.rs

Zaawansowany sterownik kart graficznych NVIDIA GeForce GTX 1000 (architektura Pascal, chipy GP104/GP106/GP107/GP108) z obsluga zaawansowanej akceleracji 3D i shaderow. Obsluguje modele: GTX 1080, GTX 1070, GTX 1060, GTX 1050 Ti, GTX 1050, GTX 1030. Sterownik napisany w Rust, kompilowany dla trybu freestanding (#[no_std]), wykorzystujacy bezpieczne operacje MMIO (volatile read/write) oraz atomowe struktury ring bufferow do przesylania komend do GPU.

### stale i adresy rejestrow

Sterownik definiuje stale dla:
- Identyfikatorow dostawcy i urzadzen: `NVIDIA_VENDOR_ID` (0x10DE), `GTX_1080_DEVICE_ID` (0x1B80), `GTX_1070_DEVICE_ID` (0x1B81), `GTX_1060_DEVICE_ID` (0x1C60), `GTX_1050_TI_DEVICE_ID` (0x1C8C), `GTX_1050_DEVICE_ID` (0x1C8D), `GTX_1030_DEVICE_ID` (0x1D01), `CHIP_GP104` (0x1B80), `CHIP_GP106` (0x1C60), `CHIP_GP107` (0x1C8C), `CHIP_GP108` (0x1D01), `PASCAL_ARCH_VERSION` (1).
- Baz MMIO, VRAM, GART i doorbell: `REG_MMIO_BASE` (0xF0000000), `REG_VRAM_BASE` (0xE0000000), `REG_GART_BASE` (0xD0000000), `REG_DOORBELL_BASE` (0xC0000000).
- Rejestrow PMC (zarzadzanie zasilaniem), PGRAPH (potok graficzny, konteksty, przerwania, FECS), GPCCS, GPC, TPC, SM (procesory strumieniowe), ROP, TEX, SHADER (vertex, fragment, geometry, compute, tessellation, uniform, attribute), COMPUTE, RASTER, DEPTH, STENCIL, BLEND, COLOR_BUFFER, FRAMEBUFFER, RENDER_TARGET, DEPTH_STENCIL_BUFFER, MULTISAMPLE, ANTIALIAS, VIEWPORT, SCISSOR, VERTEX_CACHE, INDEX_BUFFER, PRIMITIVE_BUFFER, UNIFORM_BUFFER, SHADER_STORAGE, ATOMIC_COUNTER, QUERY_OBJECT, PCIE, POWER_PLAY, SMU, SDMA, CP, VGT, PA_SC, DB, CB.
- Rejestrow shaderow i potoku graficznego: `NV_SHADER_MODULE_*`, `NV_DESCRIPTOR_SET_*`, `NV_COMMAND_BUFFER_*`, `NV_QUERY_POOL_*`, `NV_SWAP_CHAIN_*`, `NV_GRAPHICS_PIPELINE_*`.

### struktury danych

- `Matrix4x4` - macierz 4x4 (16 floatow) dla transformacji kamery, swiata i rzutowania.
- `Vector4`, `Vector3`, `Vector2` - wektory dla pozycji, koloru, tekstur, normalnych, tangentow i bitangentow.
- `Vertex` - struktura wierzcholka z pozycja, kolorem, tekstura, normalna, tangentem i bitangentem.
- `Color` - kolor RGBA w formacie float.
- `Rect`, `Viewport`, `Scissor` - prostokaty, widoki i oblasti odciecia.
- `BufferDescriptor`, `TextureDescriptor` - deskryptory buforow i tekstur.
- `ShaderProgram` - program cieniujacy z adresami shaderow vertex, fragment, geometry, tessellation, compute oraz offsetami uniformow.
- `RingBuffer` - kolejki cykliczne dla komend GFX, compute i DMA z atomowymi wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - stan procesora komend (ME, queue, prefetch).
- `GraphicsPipeline` - potok graficzny z adresami etapow shaderow, wejscia wierzcholkow, zestawiania, rasteryzacji, mieszania kolorow, depth-stencil, viewporta, multisamplingu i stanu dynamicznego.
- `GpuInstance` - glowna instancja GPU z bazami MMIO/VRAM/GART/doorbell, rozmiarem VRAM, zegarami, liczba rdzeni CUDA, SM, ROP, jednostek tekstur, szerokosci magistrali, rozmiarem cache wierzcholkow i bufora primitivow, tablicami shaderow, deskryptorow, buforow komend, pul zapytan, lancuchow wymiany i potokow graficznych.

### inicjalizacja

- `gtx_1000_init()` -> bool (wewnetrzna, wywolywana przez `gtx_1000_driver_init`) - inicjalizuje sterownik: wykrywa karte na podstawie device ID, wykonuje soft reset, konfiguruje wszystkie rejestry (PMC, PGRAPH, FECS, GPCCS, GPC, TPC, SM, ROP, tekstury, shadery, compute, rasterizer, depth/stencil, blend, kolory, framebuffery, render targety, depth/stencil buffer, multisampling, antialiasing, viewport, scissor, vertex cache, index/primitive buffer, uniform/shader storage/atomic counters, query objects, PCIE, power play, SMU), inicjalizuje ring buffer, wlacza akceleracje 3D.
- `gtx_1000_detected()` -> bool - sprawdza czy karta z rodziny GTX 1000 jest wykrywalna przez odczyt rejestrow vendor/device/revision.
- `gtx_1000_driver_init()` -> bool - eksportowana funkcja C (`#[no_mangle]`, `extern "C"`), wywolywana przez `driver_manager_register` w jadrze. Inicjalizuje instancje GPU i wywoluje `init()`.
- `gtx_1000_driver_detected()` -> bool - eksportowana funkcja C, sprawdza wykrycie karty.
- `gtx_1000_is_ready()` -> bool - zwraca 1 gdy sterownik jest zainicjowany i gotowy.
- `driver_manager_register("gtx_1000", gtx_1000_driver_init, 0, 0, 4)` - rejestracja sterownika w `kernel/kernel.c:124`, uruchamiana automatycznie przy starcie systemu.

### operacje MMIO

- `read_mmio(offset: u32) -> u32` - odczyt 32-bitowy z rejestru MMIO pod podanym offsetem.
- `write_mmio(offset: u32, value: u32)` - zapis 32-bitowy do rejestru MMIO.
- `read_mmio_64(offset: u32) -> u64` - odczyt 64-bitowy (dwa rejestry 32-bitowe).
- `write_mmio_64(offset: u32, value: u64)` - zapis 64-bitowy (dwa rejestry 32-bitowe).

### ring buffer i doorbell

- `init_ring_buffers()` - konfiguruje bazy i rejestry procesora komend oraz SDMA dla tresci ring bufferow GFX, compute i DMA.
- `submit_doorbell(ring_index: u32, value: u32)` - wysyla sygnal doorbell do wybranego ring bufferu (0=GFX, 1=compute, 2=DMA).
- `ring_write(ring_index: u32, dword: u32)` - zapisuje slowo do ring bufferu z atomowa aktualizacja wptr.
- `ring_submit(ring_index: u32)` - zatwierdza ring bufferu przez zapis RPTR i doorbell.

### akceleracja 3D i potok graficzny

- `enable_3d_acceleration()` - wlacza akceleracje 3D w rejestrach sterujacych (shader, raster, ROP, depth, blend, tex, compute, PGRAPH, GPC, SM).
- `gtx_1000_enable_3d_acceleration()` - eksportowana funkcja C.
- `compile_shader(stage, spirv_addr, spirv_size, entry_point, entry_point_size) -> u32` - kompiluje modul shaderu (SPIR-V) dla podanego etapu, zwraca ID modulu.
- `destroy_shader_module(id)` - niszczy modul shaderu o podanym ID.
- `create_shader_program(...) -> u32` - tworzy program cieniujacy z adresami shaderow vertex/fragment/geometry/tessellation/compute, offsetami uniformow, zestawami deskryptorow i push constantami. Zwraca ID programu.
- `bind_shader_program(program_id)` - wiaze program cieniujacy do potoku.
- `set_uniform(program_id, index, values)` - ustawia wartosci uniform dla danego programu i indeksu.
- `set_viewport(viewport)` - ustawia viewport renderowania (x, y, szerokosc, wysokosc, min/max depth).
- `gtx_1000_set_viewport(x, y, width, height, min_depth, max_depth)` - eksportowana funkcja C.
- `set_scissor(scissor)` - ustawia obszar odciecia (scissor).
- `set_blend_state(enable, src_rgb, dst_rgb, src_alpha, dst_alpha, equation_rgb, equation_alpha)` - konfiguracja mieszania kolorow.
- `set_depth_state(enable, func, write_mask)` - konfiguracja testu bufora glebokosci.
- `set_stencil_state(enable, func, ref_val, mask, op_fail, op_zfail, op_zpass)` - konfiguracja testu stencila.
- `set_rasterizer_state(cull, clip, msaa, aa)` - konfiguracja rasteryzatora (wycinanie, przycinanie, MSAA, antialiasing).
- `create_texture(desc) -> u32` - tworzy teksture na podstawie deskryptora.
- `bind_texture(unit, address)` - wiaze teksture pod adresem do jednostki tekstur.
- `create_framebuffer(width, height, format, address) -> u32` - tworzy framebuffer.
- `bind_framebuffer(address)` - wiaze framebuffer pod podanym adresem.
- `clear(color, depth, stencil)` - czysci buffory koloru, glebokosci i stencila.
- `gtx_1000_clear(color, depth, stencil)` - eksportowana funkcja C.
- `draw_arrays(primitive, first, count)` - rysuje z tablicy wierzcholkow (typ prymitywu, poczatek, liczba).
- `gtx_1000_draw_arrays(primitive, first, count)` - eksportowana funkcja C.
- `draw_elements(primitive, index_count, index_format, index_addr)` - rysuje z tablicy indeksow (typ prymitywu, liczba indeksow, format, adres indeksow).
- `gtx_1000_draw_elements(primitive, index_count, index_format, index_addr)` - eksportowana funkcja C.
- `create_command_buffer(size) -> u32` - tworzy bufor komend o podanym rozmiarze.
- `command_buffer_begin(id)` / `command_buffer_write(id, dword)` / `command_buffer_end(id)` - nagrywanie komend do bufora.
- `command_buffer_submit(id)` - wyslanie bufora komend do wykonania na GPU.
- `command_buffer_reset(id)` - reset bufora komend.
- `create_swap_chain(image_count, width, height, format) -> u32` - tworzy lancuch wymiany (swap chain) z podana liczba obrazow.
- `swap_chain_acquire(id) -> u32` - pobiera indeks nastepnego obrazu.
- `swap_chain_present(id)` - prezentuje obraz na ekranie.
- `compute_dispatch(group_x, group_y, group_z)` - uruchamia obliczenia compute (dispatch grup watkow).
- `finish()` / `flush()` - zsynchronizowanie i oproznienie potoku graficznego.
- `get_status() -> u32` / `gtx_1000_get_status() -> u32` - zwraca liczbe bledow sterownika.
- `wait_idle()` - oczekuje na bezczynnosc GPU.

### shadery i uniformy

- `set_shader_program(program_id)` - wiaze program cieniujacy.
- `set_uniform(program_id, index, values)` - ustawia uniformy (tablica floatow).

### tekstury i framebuffery

- `create_texture(desc) -> u32` / `bind_texture(unit, address)` - tworzenie i wiazanie tekstur.
- `create_framebuffer(width, height, format, address) -> u32` / `bind_framebuffer(address)` - tworzenie i wiazanie framebufferow.

### lancuch wymiany (swap chain)

- `create_swap_chain(image_count, width, height, format) -> u32` - tworzy lancuch wymiany.
- `swap_chain_acquire(id) -> u32` - pobiera indeks nastepnego obrazu.
- `swap_chain_present(id)` - prezentuje obraz na ekranie.

### informacje o sprzecie

- `engine_clock()` / `memory_clock()` / `vram_size()` / `cuda_cores()` / `sm_count()` / `rop_count()` / `texture_units()` / `memory_bus_width()` / `vertex_cache_size()` / `primitive_buffer_size()` / `command_buffer_size()` / `error_count()` - podstawowe parametry karty.

### globalna instancja

- `gpu_instance() -> Option<&'static mut GpuInstance>` - zwraca statyczna, leniwie inicjalizowana instancje GPU (singleton) lub None gdy nie zainicjowano.

## sterownik ekranu dotykowego - rust/drivers/ekrany_dotykowe/

Zaawansowany sterownik ekranow dotykowych napisany w Rust (`#[no_std]`), kompilowany przez makefile (wildcard `$(wildcard rust/drivers/ekrany_dotykowe/*.rs)`) i linkowany do jadra. obsluguje wykrycie urzadzenia na magistrali PCI/MMIO, soft-reset, konfiguracje magistrali, kolejke zdarzen dotykowych, kalibracje affine (macierz 2D), obsluge wielodotykowa (max 10 punktow jednoczesnie), gesty (tap, double tap, swipe, pinch zoom, rotate, long press, edge swipe), tryb polling i przerwania, statystyki zdarzen i bledow. eksponuje pelne API przez strukture `TouchDriverApi` oraz funkcje `#[no_mangle]` dla integracji ze stosem C (`kernel/kernel.c` rejestruje go przez `driver_manager_register`). caly kod jest wolny od komentarzy w zrodle.

### struktury danych

| nazwa | opis |
|------|------|
| `TouchEventType` | typ zdarzenia: Press, Release, Move, Gesture |
| `GestureType` | typ gestu: None, Tap, DoubleTap, Swipe, PinchZoom, Rotate, LongPress, EdgeSwipeLeft/Right/Up/Down |
| `TouchPoint` | pojedynczy punkt dotyku (id, x, y, raw_x, raw_y, pressure, width, height, active) |
| `TouchEvent` | zdarzenie dotykowe (typ, id, pozycja, surowa pozycja, cisnienie, rozmiar, timestamp, gest) |
| `TouchEventQueue` | kolejka FIFO zdarzen (rozmiar 64) z push/pop/peek/clear |
| `TouchState` | aktualny stan dotykowy (do 10 punktow jednoczesnie, licznik aktywnych) |
| `CalibrationMatrix` | macierz affine 2D (a, b, c, d, e, f) do mapowania surowych wspolrzednych na ekran |
| `CalibrationPoint` | punkt kalibracji (raw_x, raw_y, screen_x, screen_y) |
| `Calibration` | kalibracja (macierz, 5 punkty, flaga skalibrowany, rozmiar ekranu) |
| `TouchConfig` | konfiguracja (tryb polling/przerwanie, nr IRQ, interwal, rozmiar ekranu, gesty, multi-touch, prog cisnienia) |
| `TouchScreenController` | glowna kontroler sterownika (MMIO, konfiguracja, kalibracja, stan, kolejka, statystyki) |
| `TouchDriverApi` | eksportowane API C (wszystkie funkcje sterownika jako wskazniki) |

### stale

| nazwa | wartosc | opis |
|-------|---------|------|
| `TOUCH_VENDOR_ID` | 0x1234 | vendor ID urzadzenia dotykowego |
| `TOUCH_DEVICE_ID` | 0x5678 | device ID urzadzenia dotykowego |
| `REG_MMIO_BASE` | 0xF0001000 | baza MMIO kontrolera |
| `REG_IRQ_BASE` | 0x10 | baza numeru przerwania |
| `REG_STATUS` | 0x00 | rejestr statusu |
| `REG_DATA` | 0x04 | rejestr danych |
| `REG_CONTROL` | 0x08 | rejestr sterowania |
| `REG_CONFIG` | 0x0C | rejestr konfiguracji |
| `REG_INT_ENABLE` | 0x10 | rejestr wlaczenia przerwan |
| `REG_INT_STATUS` | 0x14 | rejestr statusu przerwan |
| `REG_INT_CLEAR` | 0x18 | rejestr potwierdzenia przerwan |
| `REG_X_MIN / REG_X_MAX` | 0x20 / 0x24 | zakres osi X |
| `REG_Y_MIN / REG_Y_MAX` | 0x28 / 0x2C | zakres osi Y |
| `REG_WIDTH / REG_HEIGHT` | 0x30 / 0x34 | rozmiar ekranu |
| `REG_PRESSURE_MAX` | 0x38 | maksymalne cisnienie |
| `REG_FIRMWARE` | 0x3C | wersja firmware |
| `REG_RESET` | 0x40 | rejestr resetu |
| `CTRL_ENABLE` | 0x00000001 | wlacz kontroler |
| `CTRL_INTERRUPT` | 0x00000002 | wlacz przerwania |
| `CTRL_CALIBRATION_MODE` | 0x00000004 | tryb kalibracji |
| `CTRL_GESTURE_MODE` | 0x00000008 | wlacz gesty |
| `STATUS_DATA_READY` | 0x00000001 | dane gotowe |
| `STATUS_ERROR` | 0x00000002 | blad |
| `STATUS_BUSY` | 0x00000004 | zajete |
| `INT_TOUCH` | 0x00000001 | przerwanie dotyku |
| `INT_GESTURE` | 0x00000002 | przerwanie gestu |
| `INT_ERROR` | 0x00000004 | przerwanie bledu |
| `MODE_POLLING` | 0 | tryb polling |
| `MODE_INTERRUPT` | 1 | tryb przerwan |
| `MAX_TOUCH_POINTS` | 10 | maksymalna liczba punktow jednoczesnych |
| `EVENT_QUEUE_SIZE` | 64 | rozmiar kolejki zdarzen |
| `NUM_CALIBRATION_POINTS` | 5 | liczba punktow kalibracji |

### funkcje sterownika

#### touch_driver_init

inicjalizuje sterownik ekranu dotykowego: wykrywa urzadzenie na MMIO, wykonuje soft-reset, czeka na gotowosc, wlacza kontroler i przerwania, konfiguruje gesty i multi-touch, ustawia zakresy osi i rozmiar ekranu. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

```
bool touch_driver_init(void);
```

#### touch_driver_shutdown

zatrzymuje sterownik: wylacza kontroler i przerwania, czysci kolejke zdarzen i stan dotykowy, oznacz jako niezainicjowany.

```
void touch_driver_shutdown(void);
```

#### touch_driver_detected

sprawdza czy urzadzenie ekranu dotykowego jest wykryte na MMIO (porownuje vendor ID, device ID i revision). zwraca 1 gdy wykryte, 0 gdy nie.

```
bool touch_driver_detected(void);
```

#### touch_driver_is_initialized

zwraca 1 gdy sterownik jest zainicjowany, 0 gdy nie.

```
bool touch_driver_is_initialized(void);
```

#### touch_driver_poll

wykonuje polling zdarzen dotykowych (wywolywane co interwal z konfiguracji, przez driver_manager). sprawdza status MMIO, potwierdza przerwania, przetwarza zdarzenia dotykowe i gesty. zwraca 1 gdy wystapilo zdarzenie, 0 gdy brak.

```
bool touch_driver_poll(uint32_t tick);
```

#### touch_driver_interrupt

obsluguje przerwanie sprzetowe z kontrolera dotykowego: czysci status przerwan i przetwarza oczekujace zdarzenia dotykowe oraz gesty.

```
void touch_driver_interrupt(void);
```

#### touch_driver_read_event

pobiera nastepne zdarzenie z kolejki FIFO. zwraca 1 i zapisuje zdarzenie do bufora wyjsciowego gdy jest dostepne, 0 gdy kolejka pusta.

```
bool touch_driver_read_event(struct TouchEvent *out);
```

#### touch_driver_peek_event

podglada nastepne zdarzenie z kolejki bez usuwania go. zwraca 1 gdy jest dostepne, 0 gdy puste.

```
bool touch_driver_peek_event(struct TouchEvent *out);
```

#### touch_driver_event_count

zwraca liczbe oczekujacych zdarzen w kolejce.

```
uint32_t touch_driver_event_count(void);
```

#### touch_driver_clear_events

czysci cala kolejke zdarzen dotykowych.

```
void touch_driver_clear_events(void);
```

#### touch_driver_set_calibration_point

dodaje punkt kalibracji (indeks 0-4, surowa pozycja i odpowiadajaca pozycja na ekranie). zwraca 1 przy sukcesie, 0 gdy indeks poza zakresem.

```
bool touch_driver_set_calibration_point(uint8_t index, int32_t raw_x, int32_t raw_y, int32_t screen_x, int32_t screen_y);
```

#### touch_driver_compute_calibration

oblicza macierz kalibracji affine na podstawie zarejestrowanych punktow (wymaga conajmniej 3). zwraca 1 przy sukcesie, 0 przy bledzie (za malo punktow lub osobliwa macierz).

```
bool touch_driver_compute_calibration(void);
```

#### touch_driver_is_calibrated

zwraca 1 gdy ekran jest skalibrowany, 0 gdy nie.

```
bool touch_driver_is_calibrated(void);
```

#### touch_driver_reset_calibration

resetuje kalibracje do macierzy jednostkowej, czysci punkty i flage skalibrowany.

```
void touch_driver_reset_calibration(void);
```

#### touch_driver_map_to_screen

mapuje surowe wspolrzedne dotykowe (raw_x, raw_y) na wspolrzedne ekranu uzywajac macierzy kalibracji. zapisuje wyniki do out_x i out_y.

```
void touch_driver_map_to_screen(int32_t raw_x, int32_t raw_y, int32_t *out_x, int32_t *out_y);
```

#### touch_driver_total_events

zwraca calkowita liczbe przetworzonych zdarzen dotykowych.

```
uint32_t touch_driver_total_events(void);
```

#### touch_driver_error_count

zwraca liczbe bledow sterownika.

```
uint32_t touch_driver_error_count(void);
```

#### touch_driver_api

zwraca wskaznik na statyczna strukture `TouchDriverApi` zawierajaca wszystkie funkcje sterownika jako wskazniki C (uzywane przez integracje ze stosem C).

```
const struct TouchDriverApi *touch_driver_api(void);
```

### integracja z jadrem

sterownik jest rejestrowany w `kernel/kernel.c` przez:

```
driver_manager_register("ekran_dotykowy", touch_driver_init, 0, 0, 4);
```

co oznacza ze jest inicjowany przez `driver_manager_init_all()` i polling jest wywolywany co 4 tiki przez `driver_manager_poll_all()`. funkcje `touch_driver_poll(tick)` i `touch_driver_interrupt()` moga byc wywolywane bezposrednio z obslugi przerwan lub petli kernela.

## sterownik kamery Logitech QuickCam Express - rust/drivers/kamerki/quickcam.rs

zaawansowany sterownik kamery internetowej Logitech QuickCam Express napisany w Rust z `#[no_std]` i `#[no_main]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/kamerki/*.rs)`) i linkowany do jadra. obsluguje wykrycie urzadzenia na magistrali USB (vendor 0x046C, produkt 0x0900), inicjalizacje konfiguracji USB (set address, set configuration, set interface), pobieranie klatek wideo przez transfery bulk (endpoint IN), konwersje formatu YUYV na RGB oraz podglad na zywo w oknie graficznym. po wykryciu sprzetu na pasku zadan pojawia sie czerwona kropka, a po kliknieciu ikony otwiera sie okno z podgladem tego co kamera aktualnie widzi.

### quickcam_driver_init

inicjalizuje sterownik kamery: wykrywa urzadzenie Logitech QuickCam Express na USB, konfiguruje adres USB, konfiguracje i interfejs, przygotowuje bufor klatki. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione.

```
bool quickcam_driver_init(void);
```

### quickcam_driver_detected

sprawdza czy kamera Logitech QuickCam Express jest wykryta na USB (porownuje vendor ID i product ID). zwraca 1 gdy wykryta, 0 gdy nie.

```
uint8_t quickcam_driver_detected(void);
```

### quickcam_driver_shutdown

zatrzymuje strumieniowanie, dezaktywuje kamere i czysci stan sterownika.

```
void quickcam_driver_shutdown(void);
```

### quickcam_driver_poll

wywolywane przez `driver_manager_poll_all()` co zadeklarowany interwal (5 tiki). pobiera nastepna klatke wideo z kamery przez transfer bulk na endpoint IN. gdy brak danych ze sprzetu, wypelnia bufor wzorcem testowym (pasek kolorow z zegarem), aby podglad dzialal nawet bez podlaczonego urzadzenia.

```
uint8_t quickcam_driver_poll(void);
```

### quickcam_driver_is_streaming

zwraca 1 gdy kamera strumieniuje klatki (aktywny podglad), 0 gdy wylaczona.

```
uint8_t quickcam_driver_is_streaming(void);
```

### quickcam_driver_toggle_preview

wlacza lub wylacza podglad na zywo z kamery (strumieniowanie klatek).

```
void quickcam_driver_toggle_preview(void);
```

### quickcam_driver_get_frame

zwraca wskaznik na bufor z ostatnia klatka wideo w formacie YUYV (640x480, 2 bajty na piksel).

```
unsigned char *quickcam_driver_get_frame(void);
```

### quickcam_driver_get_frame_size

zwraca rozmiar ostatniej klatki w bajtach.

```
unsigned int quickcam_driver_get_frame_size(void);
```

### quickcam_driver_get_width

zwraca szerokosc klatki w pikselach (640).

```
unsigned int quickcam_driver_get_width(void);
```

### quickcam_driver_get_height

zwraca wysokosc klatki w pikselach (480).

```
unsigned int quickcam_driver_get_height(void);
```

### quickcam_driver_is_detected

zwraca 1 gdy kamera zostala wykryta na USB.

```
uint8_t quickcam_driver_is_detected(void);
```

### quickcam_driver_is_active

zwraca 1 gdy sterownik jest zainicjowany i aktywny.

```
uint8_t quickcam_driver_is_active(void);
```

### interfejs uzytkownika (pasek zadan i okno podgladu)

#### quickcam_ui_init

inicjalizuje interfejs uzytkownika kamery (ikona na pasku zadan).

```
void quickcam_ui_init(void);
```

#### quickcam_ui_rysuj_pasek

rysuje ikone kamery na pasku zadan (wywolywane z `narysuj_pasek_zadan`). ikona jest biala gdy kamera jest wykryta i nieaktywna, czerwona gdy strumieniuje klatki (kropka nagrywania).

```
void quickcam_ui_rysuj_pasek(void);
```

#### quickcam_ui_icon_at

zwraca 1 gdy pozycja (x, y) znajduje sie nad ikona kamery na pasku zadan.

```
int quickcam_ui_icon_at(int x, int y);
```

#### quickcam_ui_mouse_click

obsluguje klikniecie myszy na ikonie kamery na pasku zadan. otwiera lub zamyka okno podgladu kamery. zwraca 1 gdy klikniecie bylo na ikonie kamery.

```
int quickcam_ui_mouse_click(int x, int y);
```

#### quickcam_ui_toggle

otwiera lub zamyka okno podgladu kamery z obrazem na zywo.

```
void quickcam_ui_toggle(void);
```

#### kamera_ui_toggle

otwiera lub zamyka okno podgladu kamery (`graficzny/kamera_ui.c`). okno wyswietla konwertowana klatke YUYV->RGB z tytulem "KAMERA - QUICKCAM EXPRESS".

```
void kamera_ui_toggle(void);
```

### integracja z jadrem

sterownik jest rejestrowany w `kernel/kernel.c` przez:

```
driver_manager_register("kamera_quickcam", quickcam_driver_init, quickcam_driver_shutdown, quickcam_driver_poll, 5);
```

co oznacza ze jest inicjowany przez `driver_manager_init_all()` i polling (pobieranie klatek) jest wywolywany co 5 tikow przez `driver_manager_poll_all()`. interfejs uzytkownika jest inicjowany przez `kamera_ui_init()` w `kernel_main`, a ikona kamery oraz czerwona kropka sa rysowane na pasku zadan przez `quickcam_ui_rysuj_pasek()` wywolywane z `narysuj_pasek_zadan()`. klikniecie ikony na pasku zadan jest obslugiwane w `drivers/mouse.c` przez `quickcam_ui_mouse_click()`, ktore otwiera okno podgladu kamery.

## sterownik AMD Radeon HD 5850 - rust/drivers/karty_graficzne/amd/hd_5850.rs

sterownik karty graficznej AMD Radeon HD 5850 (chip Cypress, architektura Evergreen, generacja VCE 3.0) napisany w Rust z `#[no_std]`. obsluguje wykrycie urzadzenia na PCI (vendor 0x1002, urzadzenie 0x6899), mapowanie MMIO, soft-reset chipa (GRBM + SRBM), inicjalizacje procesora komend (CP), potoku wierzcholkowego (VGT), rastrowania (PA/SC), eksportu (SX), bufora glebi (DB), bufora kolorow (CB), shaderow, rastrowania, testu glebi, stencila, blendowania, buforow koloru, ramki graficznej, render targetow, bufora glebi-stencila, probkowania wielokrotnego, wygladzania krawedzi, widoku, nozyca, cache wierzcholkow, bufora indeksow, bufora primitivow, buforow jednostajnych, buforow skladnikow shaderow, licznikow atomowych, obiektow zapytan, PCI Express, zarzadzania zasilaniem oraz SMU. nastepnie inicjalizuje bufora pikcieli (graficzny, obliczeniowy, DMA) i wlacza zaawansowana akceleracje 3D. kompilowany przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/amd/*.rs)`) i linkowany do jadra.

### hd_5850_driver_init

inicjalizuje sterownik: tworzy instancje GPU, wykrywa urzadzenie na PCI, wykonuje soft-reset, konfiguruje wszystkie bloki sprzetowe, inicjalizuje bufora pikcieli i wlacza akceleracje 3D. zwraca 1 po udanej inicjalizacji, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

```
bool hd_5850_driver_init(void);
```

### hd_5850_driver_detected

sprawdza czy karta AMD Radeon HD 5850 jest wykryta na PCI (porownuje vendor ID i device ID). zwraca 1 gdy wykryta, 0 gdy nie.

```
bool hd_5850_driver_detected(void);
```

### hd_5850_draw_triangle_3d

rysuje trojkat 3D o wierzcholkach (x1,y1,z1), (x2,y2,z2), (x3,y3,z3) i podanym kolorze.

```
void hd_5850_draw_triangle_3d(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, uint32_t color);
```

### hd_5850_draw_line_3d

rysuje linie 3D od (x1,y1,z1) do (x2,y2,z2) podanym kolorem.

```
void hd_5850_draw_line_3d(int x1, int y1, int z1, int x2, int y2, int z2, uint32_t color);
```

### hd_5850_clear_screen_3d

czysci ekran podanym kolorem w trybie 3D.

```
void hd_5850_clear_screen_3d(uint32_t color);
```

### hd_5850_set_viewport

ustawia viewport na pozycji (x, y) o rozmiarze (width, height).

```
void hd_5850_set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
```

### hd_5850_set_projection_matrix / hd_5850_set_world_matrix / hd_5850_set_view_matrix

ustawia odpowiednio macierz rzutowania, swiata lub widoku (16 wartosci f32 w formacie column-major).

```
void hd_5850_set_projection_matrix(const struct Matrix4x4 *matrix);
void hd_5850_set_world_matrix(const struct Matrix4x4 *matrix);
void hd_5850_set_view_matrix(const struct Matrix4x4 *matrix);
```

struktura `Matrix4x4` zawiera pole `m` — tablice 16 floatow.

### hd_5850_enable_depth_test / hd_5850_enable_blending

wlacza/wylacza test glebi lub blendowanie.

```
void hd_5850_enable_depth_test(uint8_t enabled);
void hd_5850_enable_blending(uint8_t enabled);
```

### hd_5850_set_blend_func

ustawia funkcje blendowania zrodlowego i docelowego.

```
void hd_5850_set_blend_func(uint32_t src, uint32_t dst);
```

### hd_5850_set_depth_func

ustawia funkcje porownania glebi.

```
void hd_5850_set_depth_func(uint32_t func);
```

### hd_5850_set_cull_mode

ustawia tryb usuwania scian (culling).

```
void hd_5850_set_cull_mode(uint32_t mode);
```

### hd_5850_set_wireframe

wlacza/wylacza tryb siatki (wireframe).

```
void hd_5850_set_wireframe(uint8_t enabled);
```

### hd_5850_set_multisample_count

ustawia liczba probek dla probkowania wielokrotnego (MSAA).

```
void hd_5850_set_multisample_count(uint32_t count);
```

### hd_5850_enable_multisampling / hd_5850_enable_antialiasing

wlacza/wylacza probkowanie wielokrotne lub wygladzanie krawedzi.

```
void hd_5850_enable_multisampling(uint8_t enabled);
void hd_5850_enable_antialiasing(uint8_t enabled);
```

### hd_5850_finish / hd_5850_flush

czeka na zakonczenie wszystkich operacji GPU (finish) lub wymusza wypelnienie potoku komend (flush).

```
void hd_5850_finish(void);
void hd_5850_flush(void);
```

### hd_5850_get_status

zwraca aktualny stan rejestru statusu GPU.

```
uint32_t hd_5850_get_status(void);
```

### hd_5850_engine_clock / hd_5850_memory_clock / hd_5850_vram_size

zwracaja odpowiednio taktowanie rdzenia, pamieci i rozmiar VRAM.

```
uint32_t hd_5850_engine_clock(void);
uint32_t hd_5850_memory_clock(void);
uint32_t hd_5850_vram_size(void);
```

### hd_5850_stream_cores / hd_5850_simd_count / hd_5850_rop_count / hd_5850_texture_units / hd_5850_memory_bus_width

zwracaja liczba rdzeni strumieniowych, SIMD, ROP, jednostek teksturowych i szerokosc szyny pamieci.

```
uint32_t hd_5850_stream_cores(void);
uint32_t hd_5850_simd_count(void);
uint32_t hd_5850_rop_count(void);
uint32_t hd_5850_texture_units(void);
uint32_t hd_5850_memory_bus_width(void);
```

### hd_5850_error_count

zwraca liczbe bledow zanotowanych przez sterownik.

```
uint32_t hd_5850_error_count(void);
```

### hd_5850_create_shader_module / hd_5850_destroy_shader_module

tworzy modul shadera z kodu SPIR-V pod podanym adresem i rozmiarze, z podanym punktem wejscia i etapie (vertex, fragment, geometry, tessellation, compute). niszczy modul shadera o podanym id.

```
uint32_t hd_5850_create_shader_module(uint64_t spirv_addr, uint32_t spirv_size, uint64_t entry_point_addr, uint32_t entry_point_size, uint32_t stage);
void hd_5850_destroy_shader_module(uint32_t id);
```

### hd_5850_create_descriptor_set

tworzy zbior deskryptorow pod podanym adresie, rozmiarze i liczbie powiazan (bindings). zwraca id zbioru.

```
uint32_t hd_5850_create_descriptor_set(uint64_t addr, uint32_t size, uint32_t binding_count, const uint64_t *bindings);
```

### hd_5850_create_command_buffer

tworzy bufor komend pod podanym adresem i rozmiarze. zwraca id bufora.

```
uint32_t hd_5850_create_command_buffer(uint64_t addr, uint32_t size);
```

### hd_5850_create_query_pool

tworzy pule zapytan o podanej liczbie i typie, z buforem wynikow pod podanym adresem. zwraca id puli.

```
uint32_t hd_5850_create_query_pool(uint64_t addr, uint32_t count, uint32_t query_type, uint64_t results_addr);
```

### hd_5850_create_swap_chain

tworzy lancuch wymiany (swap chain) o podanej liczbie obrazow, adresie, rozmiarze i formacie. zwraca id lancucha.

```
uint32_t hd_5850_create_swap_chain(uint32_t image_count, uint64_t image_addr, uint32_t image_size, uint32_t image_format);
```

### hd_5850_create_shader_pipeline / hd_5850_create_compute_pipeline

tworzy potok graficzny (shader pipeline) z modulami vertex, fragment, geometry oraz tessellation ctrl/eval, lub potok obliczeniowy (compute pipeline) z modulem compute, pod adresem i rozmiarem stanu. zwraca id potoku.

```
uint32_t hd_5850_create_shader_pipeline(uint32_t vs_module_id, uint32_t fs_module_id, uint32_t gs_module_id, uint32_t tess_ctrl_module_id, uint32_t tess_eval_module_id, uint64_t state_addr, uint32_t state_size);
uint32_t hd_5850_create_compute_pipeline(uint32_t compute_module_id, uint64_t state_addr, uint32_t state_size);
```

### hd_5850_bind_shader_pipeline / hd_5850_bind_compute_pipeline

powiazuje aktywny potok graficzny lub obliczeniowy o podanym id.

```
void hd_5850_bind_shader_pipeline(uint32_t pipeline_id);
void hd_5850_bind_compute_pipeline(uint32_t pipeline_id);
```

### hd_5850_dispatch_compute

wysyla zadanie obliczeniowe (compute dispatch) o podanej liczbie grup roboczych (x, y, z).

```
void hd_5850_dispatch_compute(uint32_t group_x, uint32_t group_y, uint32_t group_z);
```

### hd_5850_present / hd_5850_acquire_image

wyswietla nastepny obraz z lancucha wymiany lub nabywa nastepny dostepny obraz (zwraca indeks obrazu).

```
void hd_5850_present(uint32_t swap_chain_id);
uint32_t hd_5850_acquire_image(uint32_t swap_chain_id);
```

### funkcje zwiazane z teksturami, framebufferami i renderbufferami (dostepne przez instancje GPU)

- `create_texture` / `delete_texture` — tworzy/niszy teksture o podanej szerokosci, wysokosci i formacie.
- `upload_texture_data` — wgrywa dane tekstury do VRAM.
- `generate_mipmaps` — generuje mipmapy dla tekstury.
- `set_texture_filter` / `set_texture_wrap` — ustawia filtr i tryb owijania tekstury.
- `bind_texture` — powiazuje teksture z jednostka teksturowa.
- `create_framebuffer` / `delete_framebuffer` — tworzy/niszy framebuffer.
- `create_renderbuffer` / `delete_renderbuffer` — tworzy/niszy renderbuffer.
- `attach_texture_to_framebuffer` / `attach_renderbuffer_to_framebuffer` — dolacza teksture/renderbuffer do punktu zaczepienia framebuffera.
- `set_draw_buffers` / `set_read_buffer` — ustawia bufora rysowania i odczytu.
- `blit_framebuffer` — kopiuje obszar miedzy framebufferami.
- `read_pixels` — odczytuje piksele z framebuffera do bufora.

### funkcje zwiazane z tablicami wierzcholkow i rysowaniem (dostepne przez instancje GPU)

- `set_vertex_array` / `set_index_array` — ustawia tablice wierzcholkow i indeksow.
- `draw_arrays` / `draw_elements` — rysuje z tablicy wierzcholkow lub z indeksow.
- `draw_instanced` / `draw_elements_instanced` — rysuje z instancjami.
- `set_shader_program` — aktywuje program shadera.
- `set_uniform_1f/2f/3f/4f` / `set_uniform_matrix4fv` — ustawia wartosci uniformow (skalar, wektor 2/3/4, macierz 4x4).

### struktura GpuInstance

| pole | typ | opis |
|------|-----|------|
| mmio_base | *mut u32 | adres bazowy MMIO karty |
| vram_base | *mut u32 | adres bazowy pamieci VRAM |
| gart_base | *mut u32 | adres bazowy GART |
| doorbell_base | *mut u32 | adres bazowy doorbelli |
| vram_size | u64 | rozmiar pamieci VRAM (1 GB) |
| vram_used | AtomicU64 | zuzycie pamieci VRAM |
| gart_size | u64 | rozmiar GART (512 MB) |
| engine_clock | u32 | taktowanie rdzenia (725 MHz) |
| memory_clock | u32 | taktowanie pamieci (1000 MHz) |
| stream_cores | u32 | liczba rdzeni strumieniowych (1440) |
| simd_count | u32 | liczba jednostek SIMD (18) |
| rop_count | u32 | liczba jednostek ROP (32) |
| texture_units | u32 | liczba jednostek teksturowych (72) |
| memory_bus_width | u32 | szerokosc szyny pamieci (256 bit) |
| vertex_cache_size | u32 | rozmiar cache wierzcholkow (64 KB) |
| primitive_buffer_size | u32 | rozmiar bufora primitivow (32 KB) |
| command_buffer_size | u32 | rozmiar bufora komend (16 MB) |
| initialized | AtomicBool | czy sterownik jest zainicjowany |
| error_count | AtomicU32 | licznik bledow sterownika |

stale:

| nazwa | wartosc | opis |
|-------|---------|------|
| AMD_RADEON_HD_5850_VENDOR_ID | 0x1002 | vendor ID AMD/ATI |
| AMD_RADEON_HD_5850_DEVICE_ID | 0x6899 | device ID Radeon HD 5850 |
| CHIP_CYPRESS | 0x6899 | oznaczenie chipa Cypress |
| EVERGREEN_ARCH_VERSION | 5 | wersja architektury Evergreen |
| HD5850_VRAM_SIZE | 1073741824 | rozmiar VRAM w bajtach (1 GB) |
| HD5850_GART_SIZE | 536870912 | rozmiar GART w bajtach (512 MB) |
| HD5850_ENGINE_CLOCK | 725000 | taktowanie rdzenia w kHz |
| HD5850_MEMORY_CLOCK | 1000000 | taktowanie pamieci w kHz |
| HD5850_STREAM_CORES | 1440 | liczba rdzeni strumieniowych |
| HD5850_SIMD_COUNT | 18 | liczba jednostek SIMD |
| HD5850_ROP_COUNT | 32 | liczba jednostek ROP |
| HD5850_TEXTURE_UNITS | 72 | liczba jednostek teksturowych |
| HD5850_MEMORY_BUS_WIDTH | 256 | szerokosc szyny pamieci w bitach |

## karta graficzna - rust/drivers/karty_graficzne/amd/radeon_hd_6870.rs

sterownik karty graficznej AMD Radeon HD 6870 (chip Barts, architektura Northern Islands) napisany w Rust. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP), tekstury z mipmapami, shadery (vertex/pixel/geometry/compute/tessellation), render targety, depth-stencil, command buffer oraz presentation. napisany z `#[no_std]`, kompilowany przez makefile i linkowany do jadra.

### radeon_hd_6870_init

inicjalizuje sterownik Radeon HD 6870: wyszukuje urzadzenie na PCI (vendor 0x1002, urzadzenie 0x6738), mapuje adresy MMIO, resetuje Command Processor, inicjalizuje ring buffer CP i SDMA, wlacza potok 3D Northern Islands.

```
bool radeon_hd_6870_init(void);
```

zwraca true po udanej inicjalizacji, false gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

### radeon_hd_6870_is_ready

zwraca true gdy sterownik jest zainicjowany i gotowy do pracy.

```
bool radeon_hd_6870_is_ready(void);
```

### radeon_hd_6870_putpixel

rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem.

```
void radeon_hd_6870_putpixel(uint32_t x, uint32_t y, uint32_t color);
```

### radeon_hd_6870_fillrect

wypelnia prostokat o podanym rozmiarze i kolorem petla CPU.

```
void radeon_hd_6870_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
```

### radeon_hd_6870_blt

kopiuje blok pikseli z (src_x, src_y) do (dst_x, dst_y) o rozmiarze (w, h) petla CPU.

```
void radeon_hd_6870_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
```

### radeon_hd_6870_get_width / radeon_hd_6870_get_height

zwracaja szerokosc i wysokosc ekranu (1920x1080).

```
uint32_t radeon_hd_6870_get_width(void);
uint32_t radeon_hd_6870_get_height(void);
```

### radeon_hd_6870_get_device_id / radeon_hd_6870_get_revision

zwracaja id urzadzenia i revisje karty graficznej.

```
uint16_t radeon_hd_6870_get_device_id(void);
uint8_t radeon_hd_6870_get_revision(void);
```

### radeon_hd_6870_info_get

pobiera pelne informacje o urzadzeniu do struktury radeon_hd_6870_info (vendor_id, device_id, revision_id, mmio_base, fb_addr, fb_width, fb_height, fb_pitch, initialized, gcn_generation).

```
void radeon_hd_6870_info_get(struct radeon_hd_6870_info *out);
```

## akceleracja 3D Northern Islands - rust/drivers/karty_graficzne/amd/radeon_hd_6870.rs

### tworzenie i niszczenie zasobow 3D

#### radeon_hd_6870_vertex_buffer_create

tworzy vertex buffer (wierzcholki). zwraca handle lub HANDLE_NULL przy bledzie.

```
uint32_t radeon_hd_6870_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data);
```

#### radeon_hd_6870_vertex_buffer_destroy / radeon_hd_6870_vertex_buffer_update

niszczy vertex buffer lub aktualizuje jego dane od podanego offsetu.

```
void radeon_hd_6870_vertex_buffer_destroy(uint32_t handle);
void radeon_hd_6870_vertex_buffer_update(uint32_t handle, uint32_t offset, uint32_t size, const void *data);
```

#### radeon_hd_6870_index_buffer_create

tworzy index buffer (indeksy wierzcholkow, 16 lub 32 bitowe). zwraca handle.

```
uint32_t radeon_hd_6870_index_buffer_create(uint32_t index_count, uint32_t format, const void *data);
```

#### radeon_hd_6870_index_buffer_destroy / radeon_hd_6870_index_buffer_update

niszczy index buffer lub aktualizuje jego dane.

```
void radeon_hd_6870_index_buffer_destroy(uint32_t handle);
void radeon_hd_6870_index_buffer_update(uint32_t handle, uint32_t offset, uint32_t size, const void *data);
```

#### radeon_hd_6870_texture_create

tworzy teksture 2D o podanym rozmiarze, formacie i liczbie poziomow mipmap. zwraca handle.

```
uint32_t radeon_hd_6870_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data);
```

#### radeon_hd_6870_texture_destroy / radeon_hd_6870_texture_update / radeon_hd_6870_texture_generate_mipmaps

niszczy teksture, aktualizuje jej zawartosc (upload z pamieci systemowej do FB) lub generuje mipmapy.

```
void radeon_hd_6870_texture_destroy(uint32_t handle);
void radeon_hd_6870_texture_update(uint32_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
void radeon_hd_6870_texture_generate_mipmaps(uint32_t handle);
```

#### radeon_hd_6870_render_target_create / radeon_hd_6870_render_target_destroy

tworzy lub niszczy render target (bufor koloru do renderowania 3D z obsluga przezroczystosci alpha).

```
uint32_t radeon_hd_6870_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend);
void radeon_hd_6870_render_target_destroy(uint32_t handle);
```

#### radeon_hd_6870_depth_stencil_create / radeon_hd_6870_depth_stencil_destroy

tworzy lub niszczy depth-stencil buffer (bufor glebi i stencil).

```
uint32_t radeon_hd_6870_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format);
void radeon_hd_6870_depth_stencil_destroy(uint32_t handle);
```

#### radeon_hd_6870_shader_create / radeon_hd_6870_shader_destroy

tworzy shader (vertex/pixel/geometry/compute/tessellation) z kodu bajtowego ISA GPU lub niszczy shader.

```
uint32_t radeon_hd_6870_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void radeon_hd_6870_shader_destroy(uint32_t handle);
```

#### radeon_hd_6870_sampler_state_create / radeon_hd_6870_sampler_state_destroy

tworzy lub niszczy stan probki tekstury (filtrowanie min/mag/mip, adresowanie, anisotropia, porownanie).

```
uint32_t radeon_hd_6870_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable);
void radeon_hd_6870_sampler_state_destroy(uint32_t handle);
```

#### radeon_hd_6870_blend_state_create / radeon_hd_6870_blend_state_destroy

tworzy lub niszczy stan blendowania (przezroczystosc alpha, operacje blendowania, staly kolor blendowania).

```
uint32_t radeon_hd_6870_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor);
void radeon_hd_6870_blend_state_destroy(uint32_t handle);
```

#### radeon_hd_6870_rasterizer_state_create / radeon_hd_6870_rasterizer_state_destroy

tworzy lub niszczy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygладzone, alpha to coverage dla przezroczystosci).

```
uint32_t radeon_hd_6870_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage);
void radeon_hd_6870_rasterizer_state_destroy(uint32_t handle);
```

#### radeon_hd_6870_depth_stencil_state_create / radeon_hd_6870_depth_stencil_state_destroy

tworzy lub niszczy stan depth-stencil (testy glebi i stencil z przodem i tylem).

```
uint32_t radeon_hd_6870_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void radeon_hd_6870_depth_stencil_state_destroy(uint32_t handle);
```

#### radeon_hd_6870_input_layout_create / radeon_hd_6870_input_layout_destroy

tworzy lub niszczy input layout (mapowanie atrybutow vertex do slotow Northern Islands).

```
uint32_t radeon_hd_6870_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void radeon_hd_6870_input_layout_destroy(uint32_t handle);
```

#### radeon_hd_6870_constant_buffer_create / radeon_hd_6870_constant_buffer_destroy / radeon_hd_6870_constant_buffer_update

tworzy, niszczy lub aktualizuje constant buffer (stale shadera / uniforms Northern Islands).

```
uint32_t radeon_hd_6870_constant_buffer_create(uint32_t size);
void radeon_hd_6870_constant_buffer_destroy(uint32_t handle);
void radeon_hd_6870_constant_buffer_update(uint32_t handle, uint32_t offset, uint32_t size, const void *data);
```

### bindowanie stanow potoku 3D Northern Islands

#### radeon_hd_6870_bind_vertex_buffer / radeon_hd_6870_bind_index_buffer / radeon_hd_6870_bind_input_layout

binduje vertex buffer, index buffer lub input layout do potoku 3D Northern Islands.

```
void radeon_hd_6870_bind_vertex_buffer(uint32_t handle);
void radeon_hd_6870_bind_index_buffer(uint32_t handle);
void radeon_hd_6870_bind_input_layout(uint32_t handle);
```

#### radeon_hd_6870_bind_vertex_shader / radeon_hd_6870_bind_pixel_shader / radeon_hd_6870_bind_geometry_shader / radeon_hd_6870_bind_compute_shader

binduje shader vertex, pixel, geometry lub compute do potoku 3D Northern Islands.

```
void radeon_hd_6870_bind_vertex_shader(uint32_t handle);
void radeon_hd_6870_bind_pixel_shader(uint32_t handle);
void radeon_hd_6870_bind_geometry_shader(uint32_t handle);
void radeon_hd_6870_bind_compute_shader(uint32_t handle);
```

#### radeon_hd_6870_bind_render_target / radeon_hd_6870_bind_depth_stencil

binduje render target lub depth-stencil do potoku 3D Northern Islands.

```
void radeon_hd_6870_bind_render_target(uint32_t handle);
void radeon_hd_6870_bind_depth_stencil(uint32_t handle);
```

#### radeon_hd_6870_bind_blend_state / radeon_hd_6870_bind_rasterizer_state / radeon_hd_6870_bind_depth_stencil_state

binduje stan blendowania, rasterizera lub depth-stencil do potoku 3D Northern Islands.

```
void radeon_hd_6870_bind_blend_state(uint32_t handle);
void radeon_hd_6870_bind_rasterizer_state(uint32_t handle);
void radeon_hd_6870_bind_depth_stencil_state(uint32_t handle);
```

#### radeon_hd_6870_bind_sampler_and_texture / radeon_hd_6870_bind_constant_buffer

binduje probke tekstury i teksture do slotu lub constant buffer do slotu.

```
void radeon_hd_6870_bind_sampler_and_texture(uint32_t slot, uint32_t sampler, uint32_t texture);
void radeon_hd_6870_bind_constant_buffer(uint32_t slot, uint32_t handle);
```

### konfiguracja potoku 3D Northern Islands

#### radeon_hd_6870_set_viewport

ustawia viewport (prostokat wyswietlania w NDCS do ekranu) oraz granice glebi.

```
void radeon_hd_6870_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
```

#### radeon_hd_6870_set_scissor

ustawia prostokat przyciecia renderowania (scissor).

```
void radeon_hd_6870_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
```

#### radeon_hd_6870_set_primitive_topology

ustawia typ primitivu do rysowania (punkt, linia, trojkat itp.).

```
void radeon_hd_6870_set_primitive_topology(uint32_t topology);
```

### rysowanie 3D

#### radeon_hd_6870_clear_render_target

czysci render target podanym kolorem.

```
void radeon_hd_6870_clear_render_target(uint32_t rt, uint32_t color);
```

#### radeon_hd_6870_clear_depth_stencil

czysci depth-stencil podanymi wartosciami glebi i stencil.

```
void radeon_hd_6870_clear_depth_stencil(uint32_t ds, float depth, uint8_t stencil);
```

#### radeon_hd_6870_draw

rysuje primitivy z vertex buffera (bez indeksow) - vertex_count wierzcholkow poczynajac od start_vertex.

```
void radeon_hd_6870_draw(uint32_t vertex_count, uint32_t start_vertex);
```

#### radeon_hd_6870_draw_indexed

rysuje indeksowane primitivy z index buffera - index_count indeksow poczynajac od start_index z base_vertex.

```
void radeon_hd_6870_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex);
```

### command buffer (akceleracja 3D w strumieniu)

#### radeon_hd_6870_command_begin / radeon_hd_6870_command_end

rozpoczyna nagrywanie polecen do command buffer lub zakoncza nagrywanie i wykonuje wszystkie polecenia.

```
void radeon_hd_6870_command_begin(void);
void radeon_hd_6870_command_end(void);
```

### presentation (swap chain)

#### radeon_hd_6870_swap_chain_create / radeon_hd_6870_swap_chain_destroy / radeon_hd_6870_present

tworzy swap chain (podwojne buforowanie z opcja vsync), niszczy swap chain lub przebija tylny bufor na przedni (present).

```
uint32_t radeon_hd_6870_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync);
void radeon_hd_6870_swap_chain_destroy(uint32_t handle);
void radeon_hd_6870_present(uint32_t chain);
```

### informacje o akceleracji 3D

#### radeon_hd_6870_context_get / radeon_hd_6870_3d_available

pobiera biezacy kontekst 3D lub zwraca true gdy akceleracja 3D jest dostepna (pipeline Northern Islands).

```
void radeon_hd_6870_context_get(struct radeon_hd_6870_context *out);
bool radeon_hd_6870_3d_available(void);
```

## karta graficzna - rust/drivers/karty_graficzne/amd/radeon_hd_5570.rs

sterownik karty graficznej AMD Radeon HD 5570 (chip Redwood, rodzina Evergreen, architektura TeraScale 2 / VLIW5) napisany w Rust. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP), tekstury, shadery (vertex/fragment/geometry/compute), render targety, depth-stencil, ring buffory (gfx/compute/dma), doorbelle oraz presentation. napisany z `#[no_std]`, kompilowany przez makefile i linkowany do jadra. rejestrowany w `lib.rs` jako `pub mod radeon_hd_5570`.

### stale

| nazwa | wartosc | opis |
|-------|---------|------|
| RADEON_VENDOR_ID | 0x1002 | vendor ID AMD/ATI |
| RADEON_HD_5570_DEVICE_ID | 0x68BE | device ID Radeon HD 5570 |
| CHIP_REDWOOD | 0x68BE | identyfikator chipa Redwood |
| EVERGREEN_ARCH_VERSION | 2 | wersja architektury Evergreen |
| REG_MMIO_BASE | 0xF0000000 | baza rejestrow MMIO |
| REG_VRAM_BASE | 0xE0000000 | baza pamieci wideo |
| REG_GART_BASE | 0xD0000000 | baza GART |
| REG_DOORBELL_BASE | 0xC0000000 | baza doorbelli |
| RADEON_HD5570_VRAM_SIZE | 1024*1024*1024 | rozmiar VRAM (1 GB) |
| RADEON_HD5570_GART_SIZE | 512*1024*1024 | rozmiar GART (512 MB) |
| RADEON_HD5570_ENGINE_CLOCK | 650000 | taktowanie rdzenia |
| RADEON_HD5570_MEMORY_CLOCK | 900000 | taktowanie pamieci |
| RADEON_HD5570_SHADER_UNITS | 400 | liczba jednostek shaderowych |
| RADEON_HD5570_SIMD_COUNT | 5 | liczba SIMD |
| RADEON_HD5570_ROP_COUNT | 8 | liczba ROP |
| RADEON_HD5570_TEXTURE_UNITS | 20 | liczba jednostek tekstur |
| RADEON_HD5570_MEMORY_BUS_WIDTH | 128 | szerokosc szyny pamieci |

### inicjalizacja

#### radeon_hd5570_driver_init

inicjalizuje sterownik Radeon HD 5570: tworzy instancje GPU, wykrywa urzadzenie na PCI (porownanie vendor/device ID), wykonuje soft-reset GRBM, konfiguruje CP, VGT, PA, DB, CB, SQ, SPI, SX, GB_TILE, zasilanie, inicjalizuje ring buffory i wlacza akceleracje 3D.

```
bool radeon_hd5570_driver_init(void);
```

zwraca true po udanej inicjalizacji, false gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

#### radeon_hd5570_driver_detected

sprawdza czy karta Radeon HD 5570 jest wykryta (vendor 0x1002, urzadzenie 0x68BE, revisja rozna od 0xFFFFFFFF).

```
bool radeon_hd5570_driver_detected(void);
```

zwraca true gdy wykryta, false gdy nie.

#### radeon_hd5570_driver_is_ready

zwraca true gdy sterownik jest zainicjowany i gotowy do pracy.

```
bool radeon_hd5570_driver_is_ready(void);
```

### rysowanie 2D

#### radeon_hd5570_driver_putpixel

rysuje pojedynczy piksel w ramce graficznej na pozycji (x, y) podanym kolorem (bezposredni zapis do VRAM).

```
void radeon_hd5570_driver_putpixel(uint32_t x, uint32_t y, uint32_t color);
```

#### radeon_hd5570_driver_fillrect

wypelnia prostokat o rozmiarze (w, h) na pozycji (x, y) podanym kolorem petla CPU z zapisem do VRAM.

```
void radeon_hd5570_driver_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
```

#### radeon_hd5570_driver_get_width / radeon_hd5570_driver_get_height

zwracaja szerokosc i wysokosc ekranu (1024x768).

```
uint32_t radeon_hd5570_driver_get_width(void);
uint32_t radeon_hd5570_driver_get_height(void);
```

#### radeon_hd5570_driver_get_vram_size

zwraca rozmiar pamieci wideo karty w bajtach.

```
uint64_t radeon_hd5570_driver_get_vram_size(void);
```

#### radeon_hd5570_driver_get_shader_units

zwraca liczbe jednostek shaderowych (400).

```
uint32_t radeon_hd5570_driver_get_shader_units(void);
```

#### radeon_hd5570_driver_get_error_count

zwraca liczbe bledow zliczonych przez sterownik (timeouty wait_idle).

```
uint32_t radeon_hd5570_driver_get_error_count(void);
```

### shadery i akceleracja 3D (Evergreen / TeraScale 2)

#### radeon_hd5570_driver_load_shader

laduje modul shaderu (kod TGSI) pod podanym id, adresem, rozmiarem i etapie potoku (stage: 0=vertex, 1=fragment, 2=geometry, 3=tessellation). zapisuje modul w tablicy shaderow (max 32).

```
bool radeon_hd5570_driver_load_shader(uint32_t id, uint64_t addr, uint32_t size, uint32_t stage);
```

zwraca true po udanym zaladowaniu, false gdy tablica shaderow jest pelna.

#### radeon_hd5570_driver_bind_shader

binduje shader o danym id do wybranego etapu potoku (ustawia adres poczatkowy programu SQ oraz zasoby).

```
void radeon_hd5570_driver_bind_shader(uint32_t stage, uint32_t module_id);
```

#### radeon_hd5570_driver_set_uniform

ustawia stala jednorodna (uniform/constant register) w rejestrach SQ_VTX_CONSTANT_WORD dla podanej lokalizacji (cztery wartosci f32).

```
void radeon_hd5570_driver_set_uniform(uint32_t location, float v0, float v1, float v2, float v3);
```

### rysowanie 3D

#### radeon_hd5570_driver_draw_arrays

rysuje primitivy z vertex buffera bez indeksow (vertex_count wierzcholkow) przez ring buffer gfx i doorbell CP.

```
void radeon_hd5570_driver_draw_arrays(uint32_t primitive, uint32_t count);
```

#### radeon_hd5570_driver_draw_indexed

rysuje indeksowane primitivy z index buffera (index_count indeksow, adres indexow w VRAM) przez ring buffer gfx i doorbell CP.

```
void radeon_hd5570_driver_draw_indexed(uint32_t primitive, uint32_t count, uint64_t addr);
```

#### radeon_hd5570_driver_present

oznacza swap chain jako prezentowany i wysyla doorbell CP (podwojne buforowanie).

```
void radeon_hd5570_driver_present(uint32_t swap_chain_id);
```

#### radeon_hd5570_driver_clear_color

wlacza czyszczenie koloru (CB_COLOR_CONTROL).

```
void radeon_hd5570_driver_clear_color(uint32_t color);
```

#### radeon_hd5570_driver_clear_depth

wlacza czyszczenie glebi (DB_Z_INFO / DB_DEPTH_CONTROL).

```
void radeon_hd5570_driver_clear_depth(uint32_t depth);
```

### konfiguracja potoku 3D

#### radeon_hd5570_driver_set_viewport

ustawia viewport (skala i przesuniecie X/Y oraz granice glebi) w rejestrach PA_CL_VPORT i PA_SC_VPORT.

```
void radeon_hd5570_driver_set_viewport(float x, float y, float width, float height);
```

#### radeon_hd5570_driver_set_render_target

ustawia render target (adres, rozmiar, pitch) w rejestrach CB_COLOR_BASE/SIZE/PITCH dla danego indeksu (0 lub 1).

```
void radeon_hd5570_driver_set_render_target(uint32_t index, uint64_t address, uint32_t size, uint32_t pitch);
```

### struktury

#### GpuInstance

glowna instancja GPU. pola: mmio_base, vram_base, gart_base, doorbell_base, vram_size, vram_used, gart_size, engine_clock, memory_clock, shader_units, simd_count, rop_count, texture_units, memory_bus_width, vertex_cache_size, primitive_buffer_size, command_buffer_size, initialized, error_count, ring_gfx, ring_compute, ring_dma, shader_modules, descriptor_sets, command_buffers, query_pools, swap_chains, tessellation, geometry_shader, compute_shader, vertex_texture_fetch, stream_out.

#### ShaderProgram

program shaderowy: id, adresy shaderow vertex/fragment/geometry/tessellation/compute, liczba uniformow, offsety uniformow, liczba rejestrow stalych i ich offsety.

#### RingBuffer

ring buffer CP: base_addr, size, rptr, wptr, doorbell_offset, doorbell_page.

#### ShaderModule

modul shaderu: id, tgsi_addr, tgsi_size, entry_point_addr, entry_point_size, stage, gpr_count, stack_count.

#### Viewport

x, y, width, height, min_depth, max_depth.

#### Scissor

x, y, width, height.

#### BufferDescriptor

address, size, stride, flags.

#### TextureDescriptor

width, height, depth, mip_levels, array_size, format, flags, address.

### wazne uwagi

sterownik jest symulacja sprzetowa sterownika graficznego dla systemu operacyjnego tarixenOS. rejestry MMIO (EVERGREEN_*) sa mapowane na adresy pod REG_MMIO_BASE i nie odzwierciedlaja rzeczywistego chipu AMD Redwood. wszystkie operacje VRAM korzystaja z mapowania pamieci pod REG_VRAM_BASE.

## karta graficzna - rust/drivers/karty_graficzne/amd/hd_2900_xt.rs

sterownik karty graficznej ATI Radeon HD 2900 XT (chip RV670, vendor 0x1002, urzadzenie 0x9505) napisany w Rust. obsluguje wykrycie urzadzenia na PCI (porownanie vendor ID i device ID), mapowanie MMIO, soft-reset, potok grafiki 3D (VGT, primitive assembly, rasterizer, depth buffer, stencil, blend, color buffer), jednostki teksturujace (TA), samplery, cache wierzcholkow, bufor indeksow, bufor primityw, rejestry shaderow, jednostki shaderow (vertex, fragment, geometry, tessellation, compute), potok geometrii, tessellation, potok compute, ramki graficzne, render targety, bufor depth-stencil, multisampling, antialiasing, depth bounds, stencil ref, blend constants, viewport, scissor, PCIe, zasilanie i zegary (power play), SMU, ring buffory (grafika, compute, DMA) oraz akceleracje 3D. napisany w Rust z `#[no_std]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/amd/*.rs)`) i linkowany do jadra.

### hd_2900_xt_init

inicjalizuje sterownik: wykrywa urzadzenie na MMIO, wykonuje soft-reset, konfiguruje wszystkie jednostki graficzne (SRBM, GRBM, procesor komend, VGT, montaz primitywow, interpolator shaderow, rastrowanie, bufor glebi, stencil, blend, bufor koloru, jednostki tekstur, samplery, cache wierzcholkow, bufor indeksow, bufor primityw, rejestry shaderow, jednostki shaderow, potok geometrii, tessellation, potok compute, ramki graficzne, render targety, bufor depth-stencil, multisampling, antialiasing, depth bounds, stencil ref, blend constants, viewport, scissor, PCIe, power play, SMU), inicjalizuje ring buffory i wlacza akceleracje 3D. zwraca true po udanej inicjalizacji, false gdy urzadzenie nie zostalo wykryte lub inicjalizacja nie powiodla sie.

```
pub fn init(&mut self) -> bool;
```

### hd_2900_xt_detect

sprawdza czy karta ATI Radeon HD 2900 XT jest wykryta na MMIO (porownuje vendor ID, device ID i rewizje). zwraca true gdy wykryta, false gdy nie.

```
pub fn detect(&self) -> bool;
```

### hd_2900_xt_soft_reset

wykonuje soft-reset chipa: wlacza SRBM GFX CNTL, resetuje GRBM, czeka az chip wrocí do stanu bezczynnego i wylacza reset.

```
pub fn soft_reset(&mut self);
```

### hd_2900_xt_wait_idle

czeka az karta graficzna wroci do stanu bezczynnego (odczyt SRBM STATUS).

```
pub fn wait_idle(&self);
```

### hd_2900_xt_init_ring_buffers

inicjalizuje ring buffory (grafika, compute, DMA) oraz procesor komend (CP): ustawia adresy bazowe, prefetch, cntl, maski przerwan i wskazniki rptr/wptr.

```
pub fn init_ring_buffers(&mut self);
```

### hd_2900_xt_submit_doorbell

wysyla sygnal doorbell dla danego ringu (0 = grafika, 1 = compute, 2 = DMA) z podana wartoscia.

```
pub fn submit_doorbell(&mut self, ring_index: u32, value: u32);
```

### hd_2900_xt_ring_write

zapisuje pojedyncze slowo (dword) do ring buffora o danym indeksie i przesuwa wptr.

```
pub fn ring_write(&mut self, ring_index: u32, dword: u32);
```

### hd_2900_xt_ring_submit

zatwierdza ring buffor: zapisuje wptr do CP_PFFETCH_RPTR i wysyla doorbell.

```
pub fn ring_submit(&mut self, ring_index: u32);
```

### hd_2900_xt_read_mmio / hd_2900_xt_write_mmio

odczytuje/zapisuje 32-bitowa wartosc z/rejestru MMIO o podanym offsetcie.

```
pub fn read_mmio(&self, offset: u32) -> u32;
pub fn write_mmio(&mut self, offset: u32, value: u32);
```

### hd_2900_xt_read_mmio_64 / hd_2900_xt_write_mmio_64

odczytuje/zapisuje 64-bitowa wartosc z/rejestru MMIO o podanym offsetcie.

```
pub fn read_mmio_64(&self, offset: u32) -> u64;
pub fn write_mmio_64(&mut self, offset: u32, value: u64);
```

### hd_2900_xt_submit_3d_command

wysyla pojedyncza komende 3D z trzema parametrami do karty graficznej.

```
pub fn submit_3d_command(&mut self, command: u32, param1: u32, param2: u32, param3: u32);
```

### hd_2900_xt_draw_triangle_3d

rysuje trojkat 3D z trzema wierzcholkami (x,y,z) i kolorem.

```
pub fn draw_triangle_3d(&mut self, x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32);
```

### hd_2900_xt_draw_line_3d

rysuje linie 3D z dwoma punktami (x,y,z) i kolorem.

```
pub fn draw_line_3d(&mut self, x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, color: u32);
```

### hd_2900_xt_clear_screen_3d

czysci ekran podanym kolorem w trybie 3D.

```
pub fn clear_screen_3d(&mut self, color: u32);
```

### hd_2900_xt_set_viewport

ustawia viewport (x, y, szerokosc, wysokosc).

```
pub fn set_viewport(&mut self, x: u32, y: u32, width: u32, height: u32);
```

### hd_2900_xt_set_projection_matrix / hd_2900_xt_set_world_matrix / hd_2900_xt_set_view_matrix

ustawia macierz rzutowania, swiata lub widoku (16 wartosci f32).

```
pub fn set_projection_matrix(&mut self, matrix: &Matrix4x4);
pub fn set_world_matrix(&mut self, matrix: &Matrix4x4);
pub fn set_view_matrix(&mut self, matrix: &Matrix4x4);
```

### hd_2900_xt_enable_depth_test / hd_2900_xt_enable_blending

wlacza/wylacza test glebi lub blendowanie.

```
pub fn enable_depth_test(&mut self, enabled: bool);
pub fn enable_blending(&mut self, enabled: bool);
```

### hd_2900_xt_set_blend_func / hd_2900_xt_set_depth_func / hd_2900_xt_set_cull_mode

ustawia funkcje blendowania (src, dst), funkcje testu glebi lub tryb usuwania tylnych scian (cull mode).

```
pub fn set_blend_func(&mut self, src: u32, dst: u32);
pub fn set_depth_func(&mut self, func: u32);
pub fn set_cull_mode(&mut self, mode: u32);
```

### hd_2900_xt_set_wireframe / hd_2900_xt_enable_multisampling / hd_2900_xt_enable_antialiasing

wlacza/wylacza tryb siatki (wireframe), multisampling lub wygladzanie krawedzi (antialiasing).

```
pub fn set_wireframe(&mut self, enabled: bool);
pub fn enable_multisampling(&mut self, enabled: bool);
pub fn enable_antialiasing(&mut self, enabled: bool);
```

### hd_2900_xt_set_multisample_count

ustawia liczbe probek multisamplingu.

```
pub fn set_multisample_count(&mut self, count: u32);
```

### hd_2900_xt_set_depth_bounds_test

wlacza/wylacza test granic glebi z podanym minimum i maximum.

```
pub fn set_depth_bounds_test(&mut self, enabled: bool, min: f32, max: f32);
```

### hd_2900_xt_set_stencil_test / hd_2900_xt_set_stencil_op

konfiguruje test stencila (wlaczenie, funkcja, referencja, maska) oraz operacje stencila (fail, zfail, zpass).

```
pub fn set_stencil_test(&mut self, enabled: bool, func: u32, ref_value: u32, mask: u32);
pub fn set_stencil_op(&mut self, fail: u32, zfail: u32, zpass: u32);
```

### hd_2900_xt_set_blend_equation / hd_2900_xt_set_blend_color / hd_2900_xt_set_logic_op / hd_2900_xt_set_dither

ustawia rownanie blendowania (rgb, alpha), kolor blendowania, operacje logiczna oraz dithering.

```
pub fn set_blend_equation(&mut self, rgb: u32, alpha: u32);
pub fn set_blend_color(&mut self, color: u32);
pub fn set_logic_op(&mut self, op: u32, enabled: bool);
pub fn set_dither(&mut self, enabled: bool);
```

### hd_2900_xt_set_polygon_offset / hd_2900_xt_set_sample_coverage

ustawia offset wielokata (wlaczenie, factor, units) oraz probkowanie probek (sample coverage, invert).

```
pub fn set_polygon_offset(&mut self, enabled: bool, factor: f32, units: f32);
pub fn set_sample_coverage(&mut self, value: f32, invert: bool);
```

### hd_2900_xt_set_vertex_array / hd_2900_xt_set_index_array

ustawia tablice wierzcholkow (bufor, stride, offset) lub tablice indeksow (bufor, format, offset).

```
pub fn set_vertex_array(&mut self, buffer: u32, stride: u32, offset: u32);
pub fn set_index_array(&mut self, buffer: u32, format: u32, offset: u32);
```

### hd_2900_xt_draw_arrays / hd_2900_xt_draw_elements

rysuje primitive z tablicy wierzcholkow (tryb, first, count) lub z indeksow (tryb, count, typ, offset).

```
pub fn draw_arrays(&mut self, mode: u32, first: u32, count: u32);
pub fn draw_elements(&mut self, mode: u32, count: u32, type_: u32, offset: u32);
```

### hd_2900_xt_draw_instanced / hd_2900_xt_draw_elements_instanced

rysuje z instancjami (liczba instancji) z tablicy wierzcholkow lub indeksow.

```
pub fn draw_instanced(&mut self, mode: u32, first: u32, count: u32, instance_count: u32);
pub fn draw_elements_instanced(&mut self, mode: u32, count: u32, type_: u32, offset: u32, instance_count: u32);
```

### hd_2900_xt_set_shader_program

ustawia aktywny program shadera po id.

```
pub fn set_shader_program(&mut self, program_id: u32);
```

### hd_2900_xt_set_uniform_1f / 2f / 3f / 4f

ustawia jednolity (uniform) o podanej lokalizacji jako 1, 2, 3 lub 4 wartosci float.

```
pub fn set_uniform_1f(&mut self, location: u32, value: f32);
pub fn set_uniform_2f(&mut self, location: u32, x: f32, y: f32);
pub fn set_uniform_3f(&mut self, location: u32, x: f32, y: f32, z: f32);
pub fn set_uniform_4f(&mut self, location: u32, x: f32, y: f32, z: f32, w: f32);
```

### hd_2900_xt_set_uniform_matrix4fv

ustawia jednolita macierz 4x4 o podanej lokalizacji.

```
pub fn set_uniform_matrix4fv(&mut self, location: u32, matrix: &Matrix4x4);
```

### hd_2900_xt_bind_texture / hd_2900_xt_bind_framebuffer / hd_2900_xt_bind_renderbuffer

binduje teksture (jednostka, id), ramke graficzna (id) lub bufor renderowania (id).

```
pub fn bind_texture(&mut self, unit: u32, texture_id: u32);
pub fn bind_framebuffer(&mut self, framebuffer_id: u32);
pub fn bind_renderbuffer(&mut self, renderbuffer_id: u32);
```

### hd_2900_xt_create_texture / hd_2900_xt_delete_texture

tworzy teksture (szerokosc, wysokosc, format) zwracajac id lub usuwa teksture o danym id.

```
pub fn create_texture(&mut self, width: u32, height: u32, format: u32) -> u32;
pub fn delete_texture(&mut self, texture_id: u32);
```

### hd_2900_xt_upload_texture_data

wgrywa dane tekstury (id, wskaznik na dane, rozmiar).

```
pub fn upload_texture_data(&mut self, texture_id: u32, data: *const u32, size: u32);
```

### hd_2900_xt_generate_mipmaps

generuje mipmapy dla tekstury o danym id.

```
pub fn generate_mipmaps(&mut self, texture_id: u32);
```

### hd_2900_xt_set_texture_filter / hd_2900_xt_set_texture_wrap

ustawia filtr tekstury (min, mag) lub owijanie tekstury (wrap_s, wrap_t).

```
pub fn set_texture_filter(&mut self, texture_id: u32, min_filter: u32, mag_filter: u32);
pub fn set_texture_wrap(&mut self, texture_id: u32, wrap_s: u32, wrap_t: u32);
```

### hd_2900_xt_create_renderbuffer / hd_2900_xt_delete_renderbuffer

tworzy bufor renderowania (szerokosc, wysokosc, format) zwracajac id lub usuwa bufor o danym id.

```
pub fn create_renderbuffer(&mut self, width: u32, height: u32, format: u32) -> u32;
pub fn delete_renderbuffer(&mut self, renderbuffer_id: u32);
```

### hd_2900_xt_create_framebuffer / hd_2900_xt_delete_framebuffer

tworzy ramke graficzna zwracajac id lub usuwa ramke o danym id.

```
pub fn create_framebuffer(&mut self) -> u32;
pub fn delete_framebuffer(&mut self, framebuffer_id: u32);
```

### hd_2900_xt_attach_texture_to_framebuffer / hd_2900_xt_attach_renderbuffer_to_framebuffer

dolacza teksture lub bufor renderowania do ramki graficznej jako danego przylaczenia (attachment).

```
pub fn attach_texture_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, texture_id: u32, mip_level: u32);
pub fn attach_renderbuffer_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, renderbuffer_id: u32);
```

### hd_2900_xt_set_draw_buffers / hd_2900_xt_set_read_buffer

ustawia bufora rysowania (liczba, wskaznik na przylacza) lub bufor odczytu (przylacze).

```
pub fn set_draw_buffers(&mut self, count: u32, attachments: *const u32);
pub fn set_read_buffer(&mut self, attachment: u32);
```

### hd_2900_xt_blit_framebuffer

wykonuje blit ramki graficznej z obszaru zrodlowego do obszaru docelowego z maska i filtrem.

```
pub fn blit_framebuffer(&mut self, src_x0: i32, src_y0: i32, src_x1: i32, src_y1: i32, dst_x0: i32, dst_y0: i32, dst_x1: i32, dst_y1: i32, mask: u32, filter: u32);
```

### hd_2900_xt_create_shader_program / hd_2900_xt_delete_shader_program

tworzy program shadera z adresami shaderow (vertex, fragment, geometry, tess ctrl, tess eval, compute) zwracajac id lub usuwa program o danym id.

```
pub fn create_shader_program(&mut self, vs_addr: u64, fs_addr: u64, gs_addr: u64, tc_addr: u64, te_addr: u64, cs_addr: u64) -> u32;
pub fn delete_shader_program(&mut self, program_id: u32);
```

### hd_2900_xt_load_vertex_shader / hd_2900_xt_load_fragment_shader / hd_2900_xt_load_geometry_shader / hd_2900_xt_load_compute_shader

wadruje shader wierzcholkow, fragmentow, geometrii lub compute pod danym adresem i rozmiarem dla programu o danym id.

```
pub fn load_vertex_shader(&mut self, program_id: u32, addr: u64, size: u32);
pub fn load_fragment_shader(&mut self, program_id: u32, addr: u64, size: u32);
pub fn load_geometry_shader(&mut self, program_id: u32, addr: u64, size: u32);
pub fn load_compute_shader(&mut self, program_id: u32, addr: u64, size: u32);
```

### hd_2900_xt_compile_shader

kompiluje program shadera o danym id. zwraca true przy sukcesie.

```
pub fn compile_shader(&mut self, program_id: u32) -> bool;
```

### hd_2900_xt_bind_shader_resources

wiaz zasoby shaderow (bufora, tekstury) dla programu o danym id.

```
pub fn bind_shader_resources(&mut self, program_id: u32);
```

### hd_2900_xt_set_shader_constant

ustawia stala shadera (lokalizacja, wartosci float) dla programu o danym id.

```
pub fn set_shader_constant(&mut self, program_id: u32, location: u32, values: &[f32]);
```

### hd_2900_xt_set_vertex_shader / hd_2900_xt_set_fragment_shader / hd_2900_xt_set_geometry_shader

ustawia aktywny shader wierzcholkow, fragmentow lub geometrii po id programu.

```
pub fn set_vertex_shader(&mut self, program_id: u32);
pub fn set_fragment_shader(&mut self, program_id: u32);
pub fn set_geometry_shader(&mut self, program_id: u32);
```

### hd_2900_xt_enable_shader_stage

wlacza/wylacza etap shadera o danym indeksie.

```
pub fn enable_shader_stage(&mut self, stage: u32, enabled: bool);
```

### hd_2900_xt_set_uniform_block / hd_2900_xt_set_shader_storage_buffer

ustawia blok jednolity (block index, adres, rozmiar) lub bufor almacen shaderow (binding, adres, rozmiar).

```
pub fn set_uniform_block(&mut self, block_index: u32, addr: u64, size: u32);
pub fn set_shader_storage_buffer(&mut self, binding: u32, addr: u64, size: u32);
```

### hd_2900_xt_dispatch_compute

uruchamia obliczenia compute dla programu o danym id z wymiarami (x, y, z).

```
pub fn dispatch_compute(&mut self, program_id: u32, x: u32, y: u32, z: u32);
```

### struktura GpuInstance

| pole | typ | opis |
|------|-----|------|
| mmio_base | *mut u32 | baza rejestrow MMIO |
| vram_base | *mut u32 | baza pamieci VRAM |
| gart_base | *mut u32 | baza GART |
| doorbell_base | *mut u32 | baza doorbell |
| vram_size | u64 | rozmiar pamieci VRAM |
| vram_used | AtomicU64 | zuzycie pamieci VRAM |
| gart_size | u64 | rozmiar GART |
| engine_clock | u32 | zegar rdzenia |
| memory_clock | u32 | zegar pamieci |
| compute_units | u32 | liczba jednostek obliczeniowych |
| shader_engines | u32 | liczba silnikow shaderow |
| shader_arrays | u32 | liczba tablic shaderow |
| rasterizers | u32 | liczba rastrowaczy |
| backends | u32 | liczba backendu |
| texture_units | u32 | liczba jednostek teksturujacych |
| vertex_cache_size | u32 | rozmiar cache wierzcholkow |
| primitive_buffer_size | u32 | rozmiar bufora primitywow |
| command_buffer_size | u32 | rozmiar bufora komend |
| initialized | AtomicBool | czy sterownik jest zainicjowany |
| error_count | AtomicU32 | licznik bledow |
| ring_gfx | RingBuffer | ring buffor grafiki |
| ring_compute | RingBuffer | ring buffor compute |
| ring_dma | RingBuffer | ring buffor DMA |

stale:

| nazwa | wartosc | opis |
|-------|---------|------|
| ATI_RADEON_HD_2900_XT_VENDOR_ID | 0x1002 | vendor ID ATI |
| ATI_RADEON_HD_2900_XT_DEVICE_ID | 0x9505 | device ID HD 2900 XT |
| CHIP_RV670 | 0x9505 | identyfikator chipu RV670 |
| R600_ARCH_VERSION | 2 | wersja architektury R600 |
| HD2900_XT_VRAM_SIZE | 512MB | rozmiar pamieci VRAM |
| HD2900_XT_GART_SIZE | 256MB | rozmiar GART |
| HD2900_XT_ENGINE_CLOCK | 742000 | zegar rdzenia |
| HD2900_XT_MEMORY_CLOCK | 1656000 | zegar pamieci |
| HD2900_XT_COMPUTE_UNITS | 32 | liczba jednostek obliczeniowych |
| HD2900_XT_SHADER_ENGINES | 4 | liczba silnikow shaderow |
| HD2900_XT_RASTERIZERS | 4 | liczba rastrowaczy |
| HD2900_XT_BACKENDS | 16 | liczba backendu |
| HD2900_XT_TEXTURE_UNITS | 16 | liczba jednostek teksturujacych |
| REG_MMIO_BASE | 0xF0000000 | baza MMIO |
| REG_VRAM_BASE | 0xE0000000 | baza VRAM |
| REG_GART_BASE | 0xD0000000 | baza GART |
| REG_DOORBELL_BASE | 0xC0000000 | baza doorbell |

## sterownik karty graficznej AMD Radeon RX 590 - rust/drivers/karty_graficzne/amd/rx_590.rs

sterownik zaawansowanej akceleracji 3D i shaderow dla karty AMD Radeon RX 590 (Polaris 30, GCN 4, 8 GB VRAM, 36 CU). zapisany w Rust (#![no_std]), bez zaleznienia od standardowej biblioteki. rejestrowany jako `pub mod rx_590;` w `rust/drivers/karty_graficzne/amd/lib.rs`.

### stale

| nazwa | wartosc | opis |
|-------|---------|------|
| AMD_RADEON_RX_590_VENDOR_ID | 0x1002 | vendor ID AMD |
| AMD_RADEON_RX_590_DEVICE_ID | 0x67DF | device ID RX 590 |
| CHIP_POLARIS_30 | 0x67DF | identyfikator chipu Polaris 30 |
| GCN_ARCH_VERSION | 4 | wersja architektury GCN |
| REG_MMIO_BASE | 0xF0000000 | baza rejestrow MMIO |
| REG_VRAM_BASE | 0xE0000000 | baza pamieci VRAM |
| REG_GART_BASE | 0xD0000000 | baza GART |
| REG_DOORBELL_BASE | 0xC0000000 | baza doorbell |
| RX590_VRAM_SIZE | 8 GB | rozmiar pamieci VRAM |
| RX590_GART_SIZE | 4 GB | rozmiar GART |
| RX590_ENGINE_CLOCK | 1545000 | zegar rdzenia w kHz |
| RX590_MEMORY_CLOCK | 8000000 | zegar pamieci w kHz |
| RX590_COMPUTE_UNITS | 36 | liczba jednostek obliczeniowych |
| RX590_SHADER_ENGINES | 4 | liczba silnikow shaderow |
| RX590_RASTERIZERS | 4 | liczba rastrowaczy |
| RX590_BACKENDS | 16 | liczba backendu |
| RX590_TEXTURE_UNITS | 72 | liczba jednostek teksturujacych |

### typy

- `Matrix4x4` - macierz 4x4 (16 x f32), uzywana dla macierzy rzutowania, widoku i swiata.
- `Vector4` / `Vector3` / `Vector2` - wektory 4-, 3- i 2-elementowe (f32).
- `Vertex` - wierzcholek: pozycja, kolor, wspolrzedne tekstury, normalna, styczna i po-drugiej stycznej.
- `Color` - kolor RGBA (f32).
- `Rect` - prostokat (x, y, szerokosc, wysokosc, i32).
- `Viewport` - viewport: polozenie, wymiary, zakres glebokosci.
- `Scissor` - obszar scissor testu.
- `BufferDescriptor` - deskryptor bufora: adres, rozmiar, krok, flagi.
- `TextureDescriptor` - deskryptor tekstury: wymiary, poziomy mipmap, tablica, format, adres.
- `ShaderProgram` - program shaderowy z adresami shaderow wierzcholkow, fragmentow, geometrii, teselacji, obliczeniowych i lista offsetow uniformow.
- `ShaderConstant` - stala shadera: lokalizacja, typ, liczba, offset.
- `ShaderResource` - zasob shadera: wiazanie, typ, adres, rozmiar.
- `ShaderModule` - modul shadera (SPIR-V): adres kodu, rozmiar, punkt wejscia, etykiet.
- `DescriptorSet` - zbior deskryptorow z wiazaniami zasobow.
- `CommandBuffer` - bufor komend GPU: adres, rozmiar, offset, flaga wyslania.
- `QueryPool` - pula zapytan (occlusion, timestamp, pipeline statistics).
- `SwapChain` - lancuch wymiany obrazow: adresy obrazow, format, stan nabycia/prezentacji.
- `RayTracingPipeline` - potok ray tracingu: grupy shaderow, tabela shaderow, max rekurencja.
- `MeshShader` - shader siatki: task, mesh, amplification, atrybuty wierzcholkow.
- `RingBuffer` - ring buffer z wskaznikami rptr/wptr i strona doorbell.
- `CommandProcessor` - konfiguracja procesora komend (ME cntl, pfetch, maski przerwan).
- `GpuInstance` - instancja GPU: bazy MMIO/VRAM/GART/doorbell, rozmiary pamieci, zegary, liczniki CU/SE/RB/backendow/JT, ringi (grafika/compute/DMA), programy shaderowe, stale, zasoby, moduly, zbiory deskryptorow, bufora komend, pule zapytan, lancuchy wymiany, potoki ray tracingu i shadery siatki, flaga inicjalizacji, licznik bledow.

### funkcje inicjalizacji

`pub fn new() -> GpuInstance`

tworzy nowa instancje GPU z domyslnymi adresami bazowymi, rozmiarami pamieci RX 590, zegarami i pustymi ringami. nie wymaga wywolania `detect`.

`pub fn init(&mut self) -> bool`

inicjalizuje karte: sprawdza czy juz zainicjalizowana, wykrywa urzadzenie (`detect`), wykonuje soft reset, konfiguruje moduly SRBM, GRBM, procesor komend, VGT, montaz primitywow, interpolator shaderow, rastrowacz, bufor glebokosci, stencil, mieszanie, bufor koloru, jednostki tekstur, probowniki, cache wierzcholkow, bufor indeksow, bufor primitywow, rejestry shaderow, jednostki shaderow, potok geometrii, teselacje, potok obliczeniowy, ramki, cele renderowania, bufor glebokosci/stencila, probkowanie wielokrotne, wygladzanie krawedzi, granice glebokosci, referencje stencila, stale mieszania, viewport, scissor, PCIe, zarzadzanie zasilaniem, SMU, moduly shaderowe, zbiory deskryptorow, bufora komend, pule zapytan, lancuchy wymiany, ray tracing, shadery siatki, teselacje rozszerzone i cieniowanie zmiennej stawki. na koncu wlacza akceleracje 3D i ustawia flage `initialized`. zwraca `true` przy powodzeniu.

`pub fn detect(&self) -> bool`

odczytuje vendor ID, device ID i rewizje z rejestrow MMIO (offsety 0x00, 0x04, 0x08) i porownuje z identyfikatorami RX 590.

`pub fn soft_reset(&mut self)`

resetuje blok graficzny przez rejestr `GRBM_SOFT_RESET` i oczekuje na bezczynnosc (`wait_idle_inner`).

`pub fn wait_idle(&self)`

czeka na bit bezczynnosci w rejestrze `SRBM_STATUS`.

`pub fn init_ring_buffers(&mut self)`

konfiguruje prefetch procesora komend oraz ringi DMA (SDMA0, SDMA1) w pamieci VRAM.

`pub fn submit_doorbell(&mut self, ring_index: u32, value: u32)`

wysyla wartosc doorbell dla danego ringa (0 = grafika, 1 = compute, 2 = DMA).

`pub fn ring_write(&mut self, ring_index: u32, dword: u32)`

zapisuje slowo do ringa danego typu i przesuwa wptr.

`pub fn ring_submit(&mut self, ring_index: u32)`

zatwierdza ring przez rejestr `CP_PFFETCH_RPTR` i doorbell.

`pub fn get_error_count(&self) -> u32`

zwraca licznik bledow sterownika.

`pub fn is_initialized(&self) -> bool`

zwraca czy sterownik zostal zainicjalizowany.

### funkcje MMIO

`pub fn read_mmio(&self, offset: u32) -> u32`

odczytuje u32 z MMIO pod podanym offsetem (odczyt wolatylny).

`pub fn write_mmio(&mut self, offset: u32, value: u32)`

zapisuje u32 do MMIO pod podanym offsetem (zapis wolatylny).

`pub fn read_mmio_64(&self, offset: u32) -> u64`

odczytuje u64 z MMIO jako dwa slowa u32 (lo + hi << 32).

`pub fn write_mmio_64(&mut self, offset: u32, value: u64)`

zapisuje u64 do MMIO jako dwa slowa u32.

### funkcje akceleracji 3D

`pub fn submit_3d_command(&mut self, command: u32, param1: u32, param2: u32, param3: u32)`

wysyla komende 3D z trzema parametrami przez rejestry MMIO 0x1000-0x100C i wyzwalacz 0x1010.

`pub fn draw_triangle_3d(&mut self, x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32)`

rysuje trojkat 3D z trzema wierzcholkami i kolorem.

`pub fn draw_line_3d(&mut self, x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, color: u32)`

rysuje linie 3D z dwoma punktami i kolorem.

`pub fn clear_screen_3d(&mut self, color: u32)`

czysci ekran podanym kolorem.

`pub fn set_viewport(&mut self, x: u32, y: u32, width: u32, height: u32)`

ustawia viewport przez komendy 3D.

`pub fn set_projection_matrix(&mut self, matrix: &Matrix4x4)`

wgrywa macierz rzutowania do rejestrow 0x1100-0x113C.

`pub fn set_world_matrix(&mut self, matrix: &Matrix4x4)`

wgrywa macierz swiata do rejestrow 0x1200-0x123C.

`pub fn set_view_matrix(&mut self, matrix: &Matrix4x4)`

wgrywa macierz widoku do rejestrow 0x1300-0x133C.

`pub fn enable_depth_test(&mut self, enabled: bool)`

wlacza/wylacza test glebokosci (rejestr 0x1400).

`pub fn enable_blending(&mut self, enabled: bool)`

wlacza/wylacza mieszanie kolorow (rejestr 0x1500).

`pub fn set_blend_func(&mut self, src: u32, dst: u32)`

ustawia funkcje mieszania zrodla i celu (rejestry 0x1504, 0x1508).

`pub fn set_depth_func(&mut self, func: u32)`

ustawia funkcje porownania glebokosci (rejestr 0x1404).

`pub fn set_cull_mode(&mut self, mode: u32)`

ustawia tryb odrzucania scian (rejestr 0x1600).

`pub fn set_wireframe(&mut self, enabled: bool)`

wlacza/wylacza tryb szkieletowy (rejestr 0x1604).

`pub fn set_multisample_count(&mut self, count: u32)`

ustawia liczbe probek probkowania wielokrotnego (rejestr 0x1608).

`pub fn enable_multisampling(&mut self, enabled: bool)`

wlacza/wylacza probkowanie wielokrotne (rejestr 0x160C).

`pub fn enable_antialiasing(&mut self, enabled: bool)`

wlacza/wylacza wygladzanie krawedzi (rejestr 0x1610).

`pub fn set_depth_bounds_test(&mut self, enabled: bool, min: f32, max: f32)`

konfiguruje test granic glebokosci z zakresem min/max (rejestry 0x1614-0x161C).

`pub fn set_stencil_test(&mut self, enabled: bool, func: u32, ref_value: u32, mask: u32)`

konfiguruje test stencila: wlaczenie, funkcja, referencja, maska (rejestry 0x1620-0x162C).

`pub fn set_stencil_op(&mut self, fail: u32, zfail: u32, zpass: u32)`

ustawia operacje stencila dla fail, zfail, zpass (rejestry 0x1630-0x1638).

`pub fn set_blend_equation(&mut self, rgb: u32, alpha: u32)`

ustawia rownanie mieszania dla kanalu RGB i alpha (rejestry 0x1640, 0x1644).

`pub fn set_blend_color(&mut self, color: u32)`

ustawia stale koloru mieszania (rejestr 0x1648).

`pub fn set_logic_op(&mut self, op: u32, enabled: bool)`

ustawia operacje logiczna i wlaczenie/wylaczenie (rejestry 0x164C, 0x1650).

`pub fn set_dither(&mut self, enabled: bool)`

wlacza/wylacza dithering (rejestr 0x1654).

`pub fn set_polygon_offset(&mut self, enabled: bool, factor: f32, units: f32)`

ustawia offset polygona: wlaczenie, czynnik i jednostki (rejestry 0x1658-0x1660).

`pub fn set_sample_coverage(&mut self, value: f32, invert: bool)`

ustawia pokrycie probki i opcja odwrocenia (rejestry 0x1664, 0x1668).

`pub fn set_vertex_array(&mut self, buffer: u32, stride: u32, offset: u32)`

ustawia tablice wierzcholkow: bufor, krok, offset (rejestry 0x1700-0x1708).

`pub fn set_index_array(&mut self, buffer: u32, format: u32, offset: u32)`

ustawia tablice indeksow: bufor, format, offset (rejestry 0x170C-0x1714).

`pub fn draw_arrays(&mut self, mode: u32, first: u32, count: u32)`

rysuje primitywy z tablicy: tryb, poczatek, liczba (rejestry 0x1718-0x1724).

`pub fn draw_elements(&mut self, mode: u32, count: u32, type_: u32, offset: u32)`

rysuje indeksowane primitywy: tryb, liczba, typ, offset (rejestry 0x1728-0x1738).

`pub fn draw_instanced(&mut self, mode: u32, first: u32, count: u32, instance_count: u32)`

rysuje primitywy z instancjami: tryb, poczatek, liczba, liczba instancji (rejestry 0x173C-0x174C).

`pub fn draw_elements_instanced(&mut self, mode: u32, count: u32, type_: u32, offset: u32, instance_count: u32)`

rysuje indeksowane primitywy z instancjami (rejestry 0x1750-0x1764).

### funkcje shaderow i programow

`pub fn set_shader_program(&mut self, program_id: u32)`

ustawia aktywny program shaderowy (rejestr 0x1800).

`pub fn set_uniform_1f(&mut self, location: u32, value: f32)`

ustawia uniform typu float (1 skladnik).

`pub fn set_uniform_2f(&mut self, location: u32, x: f32, y: f32)`

ustawia uniform float 2 skladniki.

`pub fn set_uniform_3f(&mut self, location: u32, x: f32, y: f32, z: f32)`

ustawia uniform float 3 skladniki.

`pub fn set_uniform_4f(&mut self, location: u32, x: f32, y: f32, z: f32, w: f32)`

ustawia uniform float 4 skladniki.

`pub fn set_uniform_matrix4fv(&mut self, location: u32, matrix: &Matrix4x4)`

wgrywa macierz 4x4 jako uniform do podanej lokalizacji.

`pub fn bind_texture(&mut self, unit: u32, texture_id: u32)`

wiazanie tekstury do jednostki teksturujacej (rejestry 0x1900, 0x1904).

`pub fn bind_framebuffer(&mut self, framebuffer_id: u32)`

wiazanie bufora ramki (rejestr 0x1910).

`pub fn bind_renderbuffer(&mut self, renderbuffer_id: u32)`

wiazanie bufora renderowania (rejestr 0x1920).

`pub fn create_texture(&mut self, width: u32, height: u32, format: u32) -> u32`

tworzy teksture i zwraca jej id (rejestry 0x1A00-0x1A10).

`pub fn delete_texture(&mut self, texture_id: u32)`

usuwa teksture o danym id (rejestr 0x1A20).

`pub fn upload_texture_data(&mut self, texture_id: u32, data: *const u32, size: u32)`

wgrywa dane tekstury do pamieci GPU (rejestry 0x1A30-0x1A3C).

`pub fn generate_mipmaps(&mut self, texture_id: u32)`

generuje mipmapy dla tekstury (rejestry 0x1A40, 0x1A44).

`pub fn set_texture_filter(&mut self, texture_id: u32, min_filter: u32, mag_filter: u32)`

ustawia filtry min i mag tekstury (rejestry 0x1A50-0x1A58).

`pub fn set_texture_wrap(&mut self, texture_id: u32, wrap_s: u32, wrap_t: u32)`

ustawia owijanie tekstury osi S i T (rejestry 0x1A60-0x1A68).

`pub fn create_renderbuffer(&mut self, width: u32, height: u32, format: u32) -> u32`

tworzy bufor renderowania i zwraca jego id (rejestry 0x1A80-0x1A90).

`pub fn delete_renderbuffer(&mut self, renderbuffer_id: u32)`

usuwa bufor renderowania (rejestr 0x1AA0).

`pub fn create_framebuffer(&mut self) -> u32`

tworzy bufor ramki i zwraca jego id (rejestry 0x1AB0-0x1AB4).

`pub fn delete_framebuffer(&mut self, framebuffer_id: u32)`

usuwa bufor ramki (rejestr 0x1AC0).

`pub fn attach_texture_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, texture_id: u32, mip_level: u32)`

przylacza teksture do bufora ramki jako danego zaobrazenia (rejestry 0x1AD0-0x1AE0).

`pub fn attach_renderbuffer_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, renderbuffer_id: u32)`

przylacza bufor renderowania do bufora ramki (rejestry 0x1AF0-0x1AFC).

`pub fn set_draw_buffers(&mut self, count: u32, attachments: *const u32)`

ustawia bufora rysowania (rejestry 0x1B00, 0x1B04).

`pub fn set_read_buffer(&mut self, attachment: u32)`

ustawia bufor odczytu (rejestr 0x1B10).

`pub fn blit_framebuffer(&mut self, src_x0: i32, src_y0: i32, src_x1: i32, src_y1: i32, dst_x0: i32, dst_y0: i32, dst_x1: i32, dst_y1: i32, mask: u32, filter: u32)`

kopiuje obszar miedzy buforami ramki z maska i filtrem (rejestry 0x1B20-0x1B48).

`pub fn create_shader_program(&mut self, vs_addr: u64, fs_addr: u64, gs_addr: u64, tc_addr: u64, te_addr: u64, cs_addr: u64) -> u32`

tworzy program shaderowy z adresami shaderow: wierzcholkow, fragmentow, geometrii, teselacji sterujacej, teselacji wyliczajacej i obliczeniowego. zwraca id (maksymalnie 32 programy).

`pub fn delete_shader_program(&mut self, program_id: u32)`

usuwa program shaderowy o danym id.

`pub fn load_vertex_shader(&mut self, program_id: u32, addr: u64, size: u32)`

wczytuje shader wierzcholkow pod podany adres i rozmiar.

`pub fn load_fragment_shader(&mut self, program_id: u32, addr: u64, size: u32)`

wczytuje shader fragmentow.

`pub fn load_geometry_shader(&mut self, program_id: u32, addr: u64, size: u32)`

wczytuje shader geometrii.

`pub fn load_compute_shader(&mut self, program_id: u32, addr: u64, size: u32)`

wczytuje shader obliczeniowy.

`pub fn compile_shader(&mut self, program_id: u32) -> bool`

kompiluje program shaderowy (rejestry 0x2200, 0x2204). zwraca true przy powodzeniu.

`pub fn bind_shader_resources(&mut self, program_id: u32)`

wiazanie zasobow shadera: wiazania, typy, adresy, rozmiary (rejestry 0x2240-0x2248 + 0x204 na zasob).

`pub fn set_shader_constant(&mut self, program_id: u32, location: u32, values: &[f32])`

ustawia stala shadera pod dana lokalizacja (do 4 wartosci f32, maksymalnie 512 stalych).

`pub fn set_vertex_shader(&mut self, program_id: u32)`

aktywuje shader wierzcholkow programu (rejestr 0x22C0).

`pub fn set_fragment_shader(&mut self, program_id: u32)`

aktywuje shader fragmentow (rejestr 0x22C4).

`pub fn set_geometry_shader(&mut self, program_id: u32)`

aktywuje shader geometrii (rejestr 0x22C8).

`pub fn enable_shader_stage(&mut self, stage: u32, enabled: bool)`

wlacza/wylacza etap potoku shaderowego (rejestry 0x22D0 + etapa * 4).

`pub fn set_uniform_block(&mut self, block_index: u32, addr: u64, size: u32)`

ustawia blok uniformow: indeks, adres, rozmiar (rejestry 0x22E0-0x22E8).

`pub fn set_shader_storage_buffer(&mut self, binding: u32, addr: u64, size: u32)`

ustawia bufor magazynu shadera: wiazanie, adres, rozmiar (rejestry 0x22F0-0x22F8).

`pub fn dispatch_compute(&mut self, program_id: u32, x: u32, y: u32, z: u32)`

uruchamia obliczenia compute dla programu o danym id z wymiarami (x, y, z) (rejestry 0x2300-0x2310).

### funkcje modulow shaderowych, deskryptorow i buforow komend

`pub fn create_shader_module(&mut self, spirv_addr: u64, spirv_size: u32, entry_point_addr: u64, entry_point_size: u32, stage: u32) -> u32`

tworzy modul shadera z kodu SPIR-V: adres, rozmiar, punkt wejscia, rozmiar wejscia, etykiet (vertex/fragment/geometry/compute/ray). zwraca id (maksymalnie 64 moduly).

`pub fn destroy_shader_module(&mut self, module_id: u32)`

usuwa modul shadera o danym id (rejestr `SHADER_MODULE_DESTROY`).

`pub fn create_descriptor_set(&mut self, addr: u64, size: u32, binding_count: u32, bindings: &[u64]) -> u32`

tworzy zbior deskryptorow z adresem, rozmiarem i lista wiazan zasobow (maksymalnie 128 wiazan, 64 zbiory).

`pub fn destroy_descriptor_set(&mut self, set_id: u32)`

usuwa zbior deskryptorow (rejestr `DESCRIPTOR_SET_FREE`).

`pub fn update_descriptor_set(&mut self, set_id: u32)`

aktualizuje zbior deskryptorow w GPU (rejestr `DESCRIPTOR_SET_UPDATE`).

`pub fn bind_descriptor_set(&mut self, set_id: u32, slot: u32)`

wiazuje zbior deskryptorow do gniazda slot (rejestry `DESCRIPTOR_SET_BIND`, `DESCRIPTOR_SET_BIND_ADDR`).

`pub fn create_command_buffer(&mut self, addr: u64, size: u32) -> u32`

tworzy bufor komend GPU (maksymalnie 64 bufora).

`pub fn destroy_command_buffer(&mut self, cb_id: u32)`

usuwa bufor komend (rejestr `COMMAND_BUFFER_FREE`).

`pub fn begin_command_buffer(&mut self, cb_id: u32)`

rozpoczyna nagrywanie komend do bufora (rejestr `COMMAND_BUFFER_BEGIN`).

`pub fn end_command_buffer(&mut self, cb_id: u32)`

zakoncza nagrywanie komend (rejestr `COMMAND_BUFFER_END`).

`pub fn submit_command_buffer(&mut self, cb_id: u32)`

wysyla bufor komend do wykonania (rejestr `COMMAND_BUFFER_SUBMIT`).

`pub fn reset_command_buffer(&mut self, cb_id: u32)`

resetuje bufor komend: offset i flaga wyslania (rejestr `COMMAND_BUFFER_RESET`).

### funkcje pul zapytan i lancuchow wymiany

`pub fn create_query_pool(&mut self, addr: u64, count: u32, query_type: u32, results_addr: u64, results_size: u32) -> u32`

tworzy pule zapytan GPU (occlusion, timestamp, statystyki potoku).

`pub fn destroy_query_pool(&mut self, pool_id: u32)`

usuwa pule zapytan (rejestr `QUERY_POOL_FREE`).

`pub fn reset_query_pool(&mut self, pool_id: u32)`

resetuje pule zapytan (rejestr `QUERY_POOL_RESET`).

`pub fn get_query_pool_results(&mut self, pool_id: u32) -> u64`

pobiera wyniki zapytan z pules (rejestr `QUERY_POOL_RESULTS`).

`pub fn create_swap_chain(&mut self, image_count: u32, image_addrs: &[u64], image_size: u32, image_format: u32) -> u32`

tworzy lancuch wymiany obrazow: liczba obrazow, ich adresy, rozmiar, format (maksymalnie 8 lancuchow, 8 obrazow na lancuch).

`pub fn destroy_swap_chain(&mut self, chain_id: u32)`

usuwa lancuch wymiany (rejestr `SWAP_CHAIN_DESTROY`).

`pub fn present_swap_chain(&mut self, chain_id: u32)`

prezentuje obraz lancucha wymiany na ekranie (rejestr `SWAP_CHAIN_PRESENT`).

`pub fn acquire_swap_chain_image(&mut self, chain_id: u32) -> u32`

nabiera nastepny obraz z lancucha wymiany i zwraca jego indeks (rejestr `SWAP_CHAIN_ACQUIRE` przez pole `current_image`).

### funkcje ray tracingu i shaderow siatki

`pub fn create_ray_tracing_pipeline(&mut self, group_addrs: &[u64], group_sizes: &[u32], max_recursion: u32, sbt_addr: u64, sbt_size: u32, ray_gen: u32, miss: u32, hit_group: u32) -> u32`

tworzy potok ray tracingu z grupami shaderow, tabela shaderow (SBT), maksymalna rekurencja i offsety generowania promieni, chybien i trafien (maksymalnie 16 potokow).

`pub fn destroy_ray_tracing_pipeline(&mut self, rt_id: u32)`

usuwa potok ray tracingu o danym id.

`pub fn trace_rays(&mut self, rt_id: u32, width: u32, height: u32, depth: u32)`

uruchamia sladowanie promieni dla danego potoku o wymiarach obrazu i glebokosci (rejestry `RAY_TRACING_TRACE`, `RAY_TRACING_BVH_BUILD`).

`pub fn create_mesh_shader(&mut self, task_addr: u64, task_size: u32, mesh_addr: u64, mesh_size: u32, amp_addr: u64, amp_size: u32, attr_count: u32, attr_addrs: &[u64]) -> u32`

tworzy shader siatki z shaderami task, mesh, amplification i adresami atrybutow wierzcholkow (maksymalnie 16 shaderow siatki, 32 atrybuty).

`pub fn destroy_mesh_shader(&mut self, ms_id: u32)`

usuwa shader siatki o danym id.

`pub fn dispatch_mesh(&mut self, ms_id: u32, group_x: u32, group_y: u32, group_z: u32)`

uruchamia siatke o zadanej liczbie grup (x, y, z) (rejestry `MESH_SHADER_TASK_CNTL`).

## sterownik AMD Radeon R9 280X - rust/drivers/karty_graficzne/amd/r9_280x.rs

sterownik karty graficznej AMD Radeon R9 280X (chip Tahiti, architektura GCN 1.0) napisany w Rust. obsluguje wykrycie urzadzenia na PCI (vendor 0x1002, urzadzenie 0x6798), mapowanie MMIO, soft-reset chipa (GRBM_SOFT_RESET), inicjalizacje wszystkich blokow potoku graficznego GCN (Command Processor, Vertex Group Transfer, Primitive Assembly, Scanout, Depth Buffer, Color Buffer, Shader Export, Texture Address, Texture Cache, Shader Processor Interpolator, Shader Quad Cache, SDMA, SMU), kolejki pierscieniowe (GFX, compute, DMA), command buffer, lancuch wymiany klatek, kompilacje modulow shaderow (SPIRV), programy shaderowe (vertex, fragment, geometry, tessellation, compute), potoki graficzne, tekstury, ramki graficzne, stany blendowania, glebi, stencli, rastrowania, wywolania rysowania (draw arrays, draw elements), dyspatch compute, viewport, scissor oraz akceleracje 3D. napisany w Rust z `#[no_std]`, kompilowany przez makefile (wildcard `$(wildcard rust/drivers/karty_graficzne/amd/*.rs)`) i linkowany do jadra.

### r9_280x_driver_init

inicjalizuje sterownik R9 280X: tworzy instancje GPU, wykrywa urzadzenie na PCI, wykonuje soft-reset, konfiguruje wszystkie bloki potoku, inicjalizuje kolejki pierscieniowe i wlacza akceleracje 3D. zwraca 1 przy sukcesie, 0 gdy urzadzenie nie zostalo znalezione lub inicjalizacja nie powiodla sie.

```
bool r9_280x_driver_init(void);
```

### r9_280x_driver_detected

sprawdzaczy karta R9 280X jest wykrta na PCI (porownuje vendor ID i device ID oraz rewizje). zwraca 1 gdy wykryta, 0 gdy nie.

```
bool r9_280x_driver_detected(void);
```

### r9_280x_enable_3d_acceleration

wlacza akceleracje 3D na karcie: aktywuje VGT, PA/SC, depth buffer, color buffer, SX, TA, CP i SPI, nastepnie czeka az karta bedzie w stanie bezczynnym.

```
void r9_280x_enable_3d_acceleration(void);
```

### r9_280x_draw_arrays

rysuje primitive z tablicy wierzcholkow bez indeksow. ustawia typ primitive, wlacza potok shaderow i zapisuje komende do kolejki GFX, nastepnie wysyla doorbell i czeka na zakonczenie.

```
void r9_280x_draw_arrays(uint32_t primitive, uint32_t first, uint32_t count);
```

### r9_280x_draw_elements

rysuje primitive z indeksami z bfora indeksow. konfiguruje format i adres bufora indeksow, zapisuje komende do kolejki GFX, wysyla doorbell i czeka na zakonczenie.

```
void r9_280x_draw_elements(uint32_t primitive, uint32_t index_count, uint32_t index_format, uint64_t index_addr);
```

### r9_280x_clear

czysci kolor, bufor glebi i stencl podanymi wartosciami i aktywuje sterowanie kolorem (CB_COLOR_CONTROL).

```
void r9_280x_clear(uint32_t color, uint32_t depth, uint32_t stencil);
```

### r9_280x_set_viewustawia viewport potoku graficznego (skale i przesuniecia X/Y/Z oraz zakres glebi) w rejestrach SC_PRIM_VPORT.

```
void r9_280x_set_viewport(float x, float y, float width, float height, float min_depth, float max_depth);
```

### r9_280x_get_status

zwraca liczbe bledow sterownika (licznik error_count).

```
uint32_t r9_280x_get_status(void);
```

### r9_280x_is_ready

zwraca 1 gdy sterownik jest zainicjowany i gotowy do pracy, 0 w przeciwnym razie.

```
bool r9_280x_is_ready(void);
```

struktura `GpuInstance` zawiera:

| pole | typ | opis |
|------|-----|------|
| mmio_base | *mut u32 | adres bazowy MMIO karty |
| vram_base | *mut u32 | adres bazowy pamieci wideo |
| gart_base | *mut u32 | adres bazowy GART |
| doorbell_base | *mut u32 | adres bazowy doorbelli |
| vram_size | u64 | rozmiar pamieci wideo (3 GB) |
| vram_used | AtomicU64 | zuzyta pamiec wideo |
| gart_size | u64 | rozmiar GART (4 GB) |
| engine_clock | u32 | czestotliwosc rdzenia (1 GHz) |
| memory_clock | u32 | czestotliwosc pamieci (6 GHz) |
| compute_units | u32 | liczba jednostek obliczeniowych (32) |
| stream_processors | u32 | liczba procesorow strumieniowych (2048) |
| rop_count | u32 | liczba jednostek ROP (32) |
| texture_units | u32 | liczba jednostek tekstur (128) |
| memory_bus_width | u32 | szerokosc szyny pamieci (384 bit) |
| vertex_cache_size | u32 | rozmiar cache wierzcholkow |
| primitive_buffer_size | u32 | rozmiar bufora primitive |
| command_buffer_size | u32 | rozmiar bufora komend |
| initialized | AtomicBool | czy sterownik jest zainicjowany |
| error_count | AtomicU32 | licznik bledow |

stale:

| nazwa | wartosc | opis |
|-------|---------|------|
| AMD_VENDOR_ID | 0x1002 | identyfikator producenta AMD |
| R9_280X_DEVICE_ID | 0x6798 | identyfikator urzadzenia R9 280X (Tahiti) |
| CHIP_TAHITI | 0x6798 | kod chipu Tahiti |
| GCN_ARCH_VERSION | 1 | wersja architektury GCN |
| REG_MMIO_BASE | 0xF0000000 | adres bazowy MMIO |
| REG_VRAM_BASE | 0xE0000000 | adres bazowy VRAM |
| REG_GART_BASE | 0xD0000000 | adres bazowy GART |
| REG_DOORBELL_BASE | 0xC0000000 | adres bazowy doorbelli |
| R9_280X_VRAM_SIZE | 3 * 1024^3 | rozmiar pamieci wideo |
| R9_280X_GART_SIZE | 4 * 1024^3 | rozmiar GART |
| R9_280X_ENGINE_CLOCK | 1000000 | czestotliwosc rdzenia |
| R9_280X_MEMORY_CLOCK | 6000000 | czestotliwosc pamieci |
| R9_280X_COMPUTE_UNITS | 32 | liczba jednostek obliczeniowych |
| R9_280X_STREAM_PROCESSORS | 2048 | liczba procesorow strumieniowych |
| R9_280X_ROP_COUNT | 32 | liczba jednostek ROP |
| R9_280X_TEXTURE_UNITS | 128 | liczba jednostek tekstur |
| R9_280X_MEMORY_BUS_WIDTH | 384 | szerokosc szyny pamieci |
| R9_280X_VERTEX_CACHE_SIZE | 32 * 1024 | rozmiar cache wierzcholkow |
| R9_280X_PRIMITIVE_BUFFER_SIZE | 32 * 1024 | rozmiar bufora primitive |

### rejestry MMIO

| nazwa | offset | opis |
|-------|--------|------|
| AMD_GRBM_CNTL | 0x8000 | sterowanie GRBM |
| AMD_GRBM_STATUS | 0x8004 | status GRBM |
| AMD_GRBM_SOFT_RESET | 0x8040 | soft-reset GRBM |
| AMD_GRBM_GFX_INDEX | 0x8400 | indeks GFX GRBM |
| AMD_GRBM_TIMESTAMP | 0x89C0 | znacznik czasu GRBM |
| AMD_CP_RB_CNTL | 0x8C00 | sterowanie ring buffer CP |
| AMD_CP_RB_BASE | 0x8C04 | adres bazowy ring buffer CP |
| AMD_CP_RB_RPTR | 0x8C08 | wskaznik odczytu ring buffer CP |
| AMD_CP_RB_WPTR | 0x8C0C | wskaznik zapisu ring buffer CP |
| AMD_CP_ME_CNTL | 0x8C38 | sterowanie meccanizmem CP |
| AMD_CP_ME_CNTL2 | 0x8C3C | sterowanie meccanizmem CP 2 |
| AMD_CP_INT_CNTL | 0x8C70 | sterowanie przerwaniami CP |
| AMD_CP_INT_STATUS | 0x8C74 | status przerwan CP |
| AMD_CP_STALL_CTRL | 0x8C94 | sterowanie blokowaniem CP |
| AMD_VGT_CNTL | 0x8800 | sterowanie VGT |
| AMD_VGT_PRIMITIVE_TYPE | 0x8804 | typ primitive VGT |
| AMD_VGT_SHADER_STAGES_EN | 0x8A80 | wlaczenie etapow shaderow VGT |
| AMD_VGT_CONFIG | 0x8B10 | konfiguracja VGT |
| AMD_PA_SC_MODE_CNTL_0 | 0x8C94 | tryb PA/SC 0 |
| AMD_PA_SC_AA_CONFIG | 0x8D2C | konfiguracja AA PA/SC |
| AMD_DB_DEPTH_CONTROL | 0x2800 | sterowanie glebia DB |
| AMD_DB_STENCIL_CONTROL | 0x2804 | sterowanie stencl DB |
| AMD_CB_COLOR_CONTROL | 0x16C8 | sterowanie kolorem CB |
| AMD_CB_COLOR0_BASE | 0x16CC | adres bazowy koloru 0 CB |
| AMD_SX_MISC | 0x9C00 | rozne SX |
| AMD_TA_CNTL | 0xCA04 | sterowanie teksturami TA |
| AMD_TC_CNTL | 0xCC00 | sterowanie cache tekstur TC |
| AMD_SPI_CONFIG_CNTL | 0x9C40 | konfiguracja SPI |
| AMD_SDMA0_RB_CNTL | 0x2E00 | sterowanie ring buffer SDMA0 |
| AMD_SDMA1_RB_CNTL | 0x2F00 | sterowanie ring buffer SDMA1 |
| AMD_SMU_CNTL | 0xA000 | sterowanie SMU |
| AMD_SMU_GFX_CLOCK_CNTL | 0xA010 | zegar grafiki SMU |
| AMD_SMU_MEM_CLOCK_CNTL | 0xA014 | zegar pamieci SMU |
| AMD_SMU_POWER_STATE | 0xA018 | stan zasilania SMU |

## sterownik baterii - drivers/baterie

sterownik obsluguje baterie laptopowe przez kontroler EC (Embedded Controller) i protokole SBS (Smart Battery System). pliki: `drivers/baterie/bateria.h` i `drivers/baterie/bateria.cpp`. kompilowany jako C++ (`g++`, bez wyjatkow i RTTI) i laczony do jadra 32-bitowego.

### architektura

sterownik probuje dwie sciezki odczytu:

1. SBS przez EC - polecenia SBS przesylane poczta EC (porty 0x66 komenda, 0x62 dane), odczyt napiecia, pradu, temperatury, pojemnosci, cykli i identyfikatorow.
2. EC directamente - odczyty surowych rejestrow EC dla baterii bez obslugi SBS.

wykrycie baterii: odczyt `SBS_PROJEKTOWA_POJEMNOSC` (0x11). jesli wartosc rozna od 0 i 0xFFFF, bateria jest uznana za inteligentna. w przeciwnym razie probowany jest rejestr EC 0x00.

### enum stan_baterii

| wartosc | nazwa | znaczenie |
|---------|-------|-----------|
| 0 | BATERIA_NIEZNANA | stan nie zostal określony |
| 1 | BATERIA_ROZLADOWYWUJE_SIE | rozladowanie |
| 2 | BATERIA_LADUJE_SIE | ladowanie |
| 3 | BATERIA_PENA | pelne naladowanie |
| 4 | BATERIA_KRYTYCZNA | poziom krytyczny |
| 5 | BATERIA_BRAK_BATERII | bateria nieobecna |

### struct bateria_info

| pole | typ | opis |
|------|-----|------|
| procent | uint8_t | poziom naladowania 0-100 |
| pojemnosc_aktualna_mah | uint32_t | aktualna pojemnosc mAh |
| pojemnosc_projektowa_mah | uint32_t | pojemnosc projektowa mAh |
| pojemnosc_pelna_mah | uint32_t | aktualna pelna pojemnosc mAh |
| napiecie_mv | uint32_t | napiecie mV |
| prad_ma | int32_t | prad mA (dodatni ladowanie) |
| temperatura_c | int32_t | temperatura stopni C |
| cykle_ladowania | uint16_t | liczba cykli |
| zuzycie_procent | uint8_t | zuzycie baterii 0-100 |
| czas_do_pelnego_min | uint32_t | szacowany czas do pelna |
| czas_do_rozladowania_min | uint32_t | szacowany czas do rozladunku |
| stan | enum stan_baterii | aktualny stan |
| producent | char[32] | nazwa producenta |
| model | char[32] | model baterii |
| numer_seryjny | char[32] | numer seryjny |
| chemia | char[16] | chemia ogniw |
| dostepna | uint8_t | flaga obecnosci baterii |
| ladowarka_podlaczone | uint8_t | flaga podlaczonej ladowarki |
| inteligentna | uint8_t | flaga obslugi SBS |

### klasa Bateria

| metoda | opis |
|--------|------|
| Bateria() | konstruktor, zeruje stan |
| int inicjalizuj(void) | wykrywa baterie, czytuje identyfikatory, inicjalizuje pomiary, zwraca 0 przy sukcesie |
| void odswiez(void) | odczytuje pomiary, aktualizuje stan, wywoluje callback przy zmianie |
| uint8_t czy_dostepna(void) | zwraca 1 jesli bateria wykryta |
| void pobierz_info(struct bateria_info *info) | kopelnia pelnej struktury informacji |
| uint8_t pobierz_procent(void) | poziom naladowania 0-100 |
| uint32_t pobierz_pojemnosc_mah(void) | aktualna pojemnosc mAh |
| uint32_t pobierz_napiecie_mv(void) | napiecie mV |
| int32_t pobierz_prad_ma(void) | prad mA |
| int32_t pobierz_temperature_c(void) | temperatura stopni C |
| enum stan_baterii pobierz_stan(void) | aktualny stan baterii |
| uint16_t pobierz_cykle(void) | liczba cykli ladowania |
| uint8_t pobierz_zuzycie(void) | zuzycie w procentach |
| uint32_t pobierz_czas_do_pelnego_min(void) | szacowany czas do pelnego naladowania |
| uint32_t pobierz_czas_do_rozladowania_min(void) | szacowany czas do rozladunku |
| uint8_t czy_ladowarka_podlaczone(void) | 1 gdy ladowarka podlaczone |
| void ustaw_prog_krytyczny(uint8_t procent) | ustawia próg krytyczny (domyslnie 10) |
| uint8_t pobierz_prog_krytyczny(void) | zwraca ustawiony próg |
| uint8_t czy_krytyczny(void) | 1 gdy procent <= prog krytyczny |
| void zarejestruj_callback(void (*)(enum stan_baterii)) | rejestruje funkcja wywolywana przy zmianie stanu |
| void usun_callback(void) | usuwa zarejestrowany callback |

### API C (dla kodu C)

wszystkie funkcje z prefixes `bateria_` otaczaja metode klasy `Bateria` i maja linkage C: `bateria_init`, `bateria_odswiez`, `bateria_czy_dostepna`, `bateria_pobierz_info`, `bateria_pobierz_procent`, `bateria_pobierz_pojemnosc_mah`, `bateria_pobierz_napiecie_mv`, `bateria_pobierz_prad_ma`, `bateria_pobierz_temperature_c`, `bateria_pobierz_stan`, `bateria_pobierz_cykle`, `bateria_pobierz_zuzycie`, `bateria_pobierz_czas_do_pelnego_min`, `bateria_pobierz_czas_do_rozladowania_min`, `bateria_czy_ladowarka_podlaczone`, `bateria_ustaw_prog_krytyczny`, `bateria_pobierz_prog_krytyczny`, `bateria_czy_krytyczny`, `bateria_zarejestruj_callback`, `bateria_usun_callback`.

### rejestry SBS (mapa sterownika)

| polecenie | nazwa | opis |
|-----------|-------|------|
| 0x08 | SBS_TEMPERATURA | temperatura 0.1 K |
| 0x09 | SBS_NAPIECIE | napiecie mV |
| 0x0a | SBS_PRAD | prad mA |
| 0x0d | SBS_STATUS | status baterii (flagi) |
| 0x0e | SBS_CYKLE | liczba cykli ladowania |
| 0x0f | SBS_POZOSTALA_POJEMNOSC | pozostal pojemnosc mAh |
| 0x10 | SBS_PELNA_POJEMNOSC | pelna pojemnosc mAh |
| 0x11 | SBS_PROJEKTOWA_POJEMNOSC | pojemnosc projektowa mAh |
| 0x12 | SBS_NAPIECIE_PROJEKTOWE | napiecie projektowe mWh |
| 0x13 | SBS_NUMER_SERYJNY | numer seryjny (lanuch) |
| 0x14 | SBS_PRODUCENT | producent (lanuch) |
| 0x15 | SBS_MODEL | model (lanuch) |
| 0x16 | SBS_CHEMIA | chemia (lanuch) |

flagi SBS_STATUS: 0x0001 rozladowanie, 0x0002 ladowanie, 0x0004 krytyczna, 0x0008 pelna, 0x0010 brak baterii, 0x0020 ladowarka podlaczone.

### protokół EC

porty: 0x66 komenda EC, 0x62 dane EC. komendy sterownika: 0x81 dostep do rejestru EC, 0x82 transakcja SBS. status EC: bit 0 OBF (output buffer full), bit 1 IBF (input buffer full). odblokowanie IBF przed zapisem, oczekiwanie na OBF przed odczytem, timeout 10000 iteracji.

### obliczenia

- procent = pojemnosc_aktualna * 100 / pojemnosc_pelna
- zuzycie = 100 - (pojemnosc_pelna * 100 / pojemnosc_projektowa)
- czas do pelna = (pojemnosc_pelna - pojemnosc_aktualna) * 60 / prad (przy ladowaniu)
- czas do rozladunku = pojemnosc_aktualna * 60 / |prad| (przy rozladowaniu)
- przy pradzie 0 czasy sa 0xFFFFFFFF (brak szacunku)

### przyklad uzycia

```c
struct bateria_info info;
bateria_init();
bateria_odswiez();
if (bateria_czy_dostepna()) {
    bateria_pobierz_info(&info);
    // info.procent, info.stan, info.napiecie_mv itd.
}
if (bateria_czy_krytyczny()) {
    /* dzialanie awaryjne */
}
bateria_zarejestruj_callback(moja_funkcja_stanu);
```

### ikona baterii na pasku zadań

jesli bateria zostanie wykryta, na pasku zadan (prawy dolny rog, przed ikona kamery i bluetooth) rysowana jest ikona baterii z aktualnym procentem naladowania:

- obudowa baterii z wtyczka (gora) i wypełnieniem proporcjonalnym do poziomu naladowania
- kolor wypełnienia: czerwony ponizej 15% lub w stanie krytycznym, zolty ponizej 40%, zielony powyzej, niebieski gdy ladowarka podlaczone
- obok ikony wyswietlany jest tekst procentu, np. `78%`
- ikona jest odswiezana przy kazdym wyolaniu `narysuj_pasek_zadan()` i znikaja gdy brak wykrytej baterii

pozycja: `BATERIA_IKONA_X = EKRAN_SZEROKOSC - 106`, `BATERIA_IKONA_Y = EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 4`, rozmiar 24x22 pikseli.

### integracja z driver managerem

sterownik jest rejestrowany przez `driver_manager_register` w `kernel/kernel.c`:

```
driver_manager_register("bateria", bateria_init, 0, bateria_poll, 2);
```

`bateria_init` wywoluje sie raz przy starcie systemu przez menedzer sterownikow. `bateria_poll` wywoluje `bateria_odswiez()` w kazdej iteracji petli kernela (przez `driver_manager_poll_all()`), wiec ikona i procent na pasku sa stale aktualizowane. Nie ma potrzeby wywolywania `bateria_init` ani `bateria_odswiez` recznie z kodu kernela.

## sterownik ACPI - drivers/acpi

sterownik ACPI (Advanced Configuration and Power Interface) obsluguje wykrywanie tabel ACPI, stany uśpienia procesora (C-staty), stany wydajnosci (P-staty), czestotliwosc CPU, temperatura, wiatraki oraz zapne/reboot przez ACPI. pliki: `drivers/acpi/acpi.h` i `drivers/acpi/acpi.cpp`. kompilowany jako C++ i laczony do jadra 32-bitowego.

### wykrywanie ACPI

sterownik skanuje pamiec w zakresie 0xE0000-0xFFFF0 w poszukiwaniu sygnatury "RSD PTR " (tabela RSDP). nastepnie okresla czy uzywac XSDT (ACPI 2.0+) czy RSDT. z lokalizacji XSDT/RSDT wyszukiwana jest tabela FADT (Fixed ACPI Description Table), z ktorej pobierane sa adresy blokow sterowania zasilaniem PM1A/PM1B, latencje C-stat oraz inne informacje platformy.

### struct acpi_platform_info

| pole | typ | opis |
|------|-----|------|
| available | uint8_t | ACPI wykryte i dostepne |
| pm1_available | uint8_t | bloki PM1 dostepne |
| pm1a_cnt / pm1b_cnt | uint32_t | adresy rejestrow sterowania PM1 |
| pm1a_evt / pm1b_evt | uint16_t | adresy rejestrow zdarzen PM1 |
| c1_available / c2_available / c3_available | uint8_t | dostepnosc stanu C1/C2/C3 |
| c2_latency / c3_latency | uint16_t | latencja wejscia w C2/C3 (mikrosekundy) |
| pstate_available | uint8_t | czy dostepne P-staty (zmiana czestotliwosci) |
| cpu_current_freq_mhz | uint32_t | aktualna czestotliwosc CPU MHz |
| cpu_max_freq_mhz | uint32_t | maksymalna czestotliwosc CPU MHz |
| cpu_min_freq_mhz | uint32_t | minimalna czestotliwosc CPU MHz |
| thermal_available | uint8_t | czy dostepny odczyt temperatury |
| cpu_temperature_c | int32_t | temperatura CPU w stopniach C |
| fan_available | uint8_t | czy wiatrak jest wykryty |
| fan_running | uint8_t | czy wiatrak pracuje |
| fan_rpm | uint32_t | obroty wiatraka na minute |
| throttling_available | uint8_t | czy dostepne ograniczenie wydajnosci |
| throttling_active | uint8_t | czy ograniczenie jest aktywne |

### klasa Acpi

| metoda | opis |
|--------|------|
| Acpi() | konstruktor, zeruje stan |
| int inicjalizuj(void) | skanuje RSDP/XSDT/RSDT, wczytuje FADT, inicjalizuje SCI, zwraca 0 przy sukcesie |
| void odswiez(void) | odswieza dane termiczne i wiatraka |
| uint8_t czy_dostepny(void) | 1 gdy ACPI wykryte |
| struct acpi_platform_info *pobierz_info(void) | zwraca pelna struktura informacji platformy |
| void wlacz_sci(void) | wlacza przerwania SCI (System Control Interrupt) w PM1A_CNT |
| void wylacz_sci(void) | wylacza przerwania SCI |
| uint16_t czytaj_pm1a_cnt(void) | odczytuje rejestr PM1A_CNT |
| void zapisz_pm1a_cnt(uint16_t wartosc) | zapisuje do rejestru PM1A_CNT |
| void wejdz_w_sleep(uint8_t stan) | wprowadza system w stan uśpienia ACPI (S1-S5) |
| void zamknij(void) | wlacza stan S5 (zapne systemu) |
| void zrestartuj(void) | resetuje komputer przez port 0xCF9 / ACPI reset |
| int cpu_wejdz_w_cstate(int stan) | wprowadza CPU w stan C1 (hlt), C2 lub C3 |
| int cpu_ustaw_pstate(uint8_t pstate) | ustawia P-state CPU (zmiana czestotliwosci) |
| uint32_t cpu_pobierz_czestotliwosc_mhz(void) | zwraca aktualna czestotliwosc CPU w MHz |
| int cpu_ogranicz(uint8_t procent) | ogranicza czestotliwosc CPU do podanego procentu (1-100) |
| int wiatrak_pobierz_stan(void) | zwraca 1 gdy wiatrak wlaczony, -1 gdy niedostepny |
| int wiatrak_ustaw_stan(uint8_t wlaczony) | wlacza/wylacza wiatrak |
| uint32_t wiatrak_pobierz_obroty(void) | zwraca obroty wiatraka RPM |

### API C (dla kodu C)

wszystkie funkcje z prefixes `acpi_` otaczaja metode klasy `Acpi` i maja linkage C: `acpi_init`, `acpi_odswiez`, `acpi_czy_dostepny`, `acpi_info`, `acpi_wlacz_sci`, `acpi_wylacz_sci`, `acpi_czytaj_pm1a_cnt`, `acpi_zapisz_pm1a_cnt`, `acpi_wejdz_w_sleep`, `acpi_zamknij`, `acpi_zrestartuj`, `acpi_cpu_wejdz_w_cstate`, `acpi_cpu_ustaw_pstate`, `acpi_cpu_pobierz_czestotliwosc_mhz`, `acpi_cpu_ogranicz`, `acpi_wiatrak_pobierz_stan`, `acpi_wiatrak_ustaw_stan`, `acpi_wiatrak_pobierz_obroty`.

### integracja z driver managerem

sterownik jest rejestrowany przez `driver_manager_register` w `kernel/kernel.c`:

```
driver_manager_register("acpi", acpi_init, 0, acpi_odswiez, 2);
```

`acpi_init` wywoluje sie raz przy starcie systemu. `acpi_odswiez` wywoluje sie w kazdej iteracji petli kernela przez `driver_manager_poll_all()`, stale aktualizujac temperatura, obroty wiatraka i stan ograniczenia wydajnosci.

### oszczedzanie energii

- C-staty: `cpu_wejdz_w_cstate(1)` wywoluje `hlt` (C1), C2/C3 wywolują `hlt` z wyłączonymi przerwaniami (cli/sti) gdy są dostępne.
- P-staty: `cpu_ogranicz(procent)` obniża czestotliwosc CPU proporcjonalnie, co zmniejsza pobor mocy i ciepło.
- wiatrak: `wiatrak_ustaw_stan(1/0)` pozwala recznie sterowac chlodzeniem; przy temperaturze powyżej 80°C włączane jest automatyczne throttling.

## sterownik grafiki AMD Radeon HD 6000 - drivers/karty_graficzne/amd/r6000

zaawansowany sterownik kart graficznych AMD Radeon HD 6000 (Barts, Cayman, Turks) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa, tekstury, shadery (wierzcholkowe, pikselowe, geometrii, obliczeniowe), render targety, depth-stencil, command buffer, presentation oraz zarzadzanie zasilaniem i chlodzeniem. pliki: `drivers/karty_graficzne/amd/r6000.h` i `drivers/karty_graficzne/amd/r6000.cpp`. kompilowany jako C++ i laczony do jadra 32-bitowego.

### wykrywanie urzadzenia

sterownik skanuje szyne PCI w poszukiwaniu urzadzenia klasy 0x03 (grafika) z vendor ID 0x1002 (AMD) i device ID z serii HD 6000 (Barts, Cayman, Turks). po wykryciu odczytuje BAR0 (adres MMIO), wlacza dostep do pamieci i rejestrów poleceń, inicjalizuje ring buffer Command Processor (CP), pamiec VRAM oraz wyswietlacz (CRTC/DCN).

### klasa RadeonHD6000

| metoda | opis |
|--------|------|
| RadeonHD6000() | konstruktor, zeruje stan wszystkich buforow, tekstur, shaderow i kontekstu |
| int inicjalizuj(void) | wykrywa karte PCI, inicjalizuje MMIO, ring buffer, VRAM, wyswietlacz, resetuje potok graficzny |
| void zamknij(void) | zatrzymuje CP i dezaktywuje sterownik |
| uint8_t czy_gotowy(void) | 1 gdy sterownik zainicjalizowany |
| void putpixel(x, y, kolor) | rysuje pojedynczy piksel w framebufferze |
| void fillrect(x, y, w, h, kolor) | wypelnia prostokat kolorem |
| void blt(dst_x, dst_y, src_x, src_y, w, h) | kopiuje blok pikseli (bitblt) |
| void cursor_set(x, y, obraz) | ustawia kursor myszy i jego bitmape |
| void cursor_hide(void) | ukrywa kursor |
| void *get_fb(void) | zwraca wskaznik do framebuffera |
| uint32_t get_szerokosc(void) | szerokosc framebuffera |
| uint32_t get_wysokosc(void) | wysokosc framebuffera |
| uint16_t get_device_id(void) | identyfikator urzadzenia PCI |
| uint8_t get_rewizja(void) | rewizja ukladu |
| void info_pobierz(struct radeon_hd6000_info *wyjscie) | pelne informacje o urzadzeniu |

### zarzadzanie pamiecia VRAM

| metoda | opis |
|--------|------|
| uint64_t vram_alokuj(rozmiar, typ) | alokuje blok w VRAM, zwraca offset lub HANDLE_NULL |
| void vram_zwolnij(uchwyt) | zwalnia blok VRAM |
| uint64_t vram_uzyto(void) | uzyta pamiec VRAM |
| uint64_t vram_calkowicie(void) | calkowita pamiec VRAM |

### obiekty graficzne (bufory, tekstury, shaders, stany)

sterownik obsluguje tworzenie, niszczenie, aktualizacje i wiazanie:
- buforow wierzcholkow (`bufor_wierzcholkow_stworz/zniszcz/zaktualizuj`)
- buforow indeksow (`bufor_indeksow_stworz/zniszcz/zaktualizuj`)
- tekstur z mipmapami (`tekstura_stworz/zniszcz/zaktualizuj/wygeneruj_mipmapy`)
- celow renderowania (`cel_renderowania_stworz/zniszcz`)
- buforow glebi-stencil (`bufor_glebii_stworz/zniszcz`)
- shaderow wierzcholkowych, pikselowych, geometrii i obliczeniowych (`shader_stworz/zniszcz/skompiluj`)
- stanu probnika (`stan_probnika_stworz/zniszcz`)
- stanu mieszania alpha (`stan_mieszania_stworz/zniszcz`)
- stanu rasteryzacji (`stan_rasteryzacji_stworz/zniszcz`)
- stanu glebi-stencil (`stan_glebii_stencil_stworz/zniszcz`)
- ukladow wejścia (`uklad_wejsc_stworz/zniszcz`)
- buforow stalych (`bufor_stale_stworz/zniszcz/zaktualizuj`)

### wiazanie i rysowanie 3D

| metoda | opis |
|--------|------|
| powiaz_bufor_wierzcholkow(uchwyt) | wiaze aktywny bufor wierzcholkow |
| powiaz_shader_wierzcholkow/pikseli/geometrii/obliczeniowy(uchwyt) | wiaze shader danego typu |
| powiaz_cel_renderowania(uchwyt) | wiaze cel renderowania (framebuffer wyjsciowy) |
| powiaz_bufor_glebii(uchwyt) | wiaze bufor glebi |
| powiaz_stan_mieszania/rasteryzacji/glebii_stencil(uchwyt) | wiaze stany potoku |
| powiaz_probnik_i_teksture(gniazdo, probnik, tekstura) | wiaze probnik i teksture do gniazda |
| powiaz_bufor_stale(gniazdo, uchwyt) | wiaze bufor stalych |
| ustaw_viewport(x, y, w, h, min_glebia, max_glebia) | ustawia okno widoku |
| ustaw_scissor(lewa, gora, prawa, dol) | ustawia region odciecia |
| ustaw_typ_pierwotnego(typ) | ustawia typ primitive (punkt, linia, trojkat, kwad) |
| ustaw_mieszanie_alpha_globalne(wlacz) | globalne mieszanie alpha |
| wyczysc_cel_renderowania(cel, kolor) | czyści cel renderowania |
| wyczysc_bufor_glebii(bufor, glebia, stencil) | czyści bufor glebi-stencil |
| rysuj(liczba_wierzcholkow, pierwszy) | rysuje geometryczne primitive |
| rysuj_indeksowane(liczba_indeksow, pierwszy_indeks, baza) | rysuje z indeksami |
| rysuj_indeksowane_instancjonowane(...) | rysuje instancjonowane |
| rysuj_po_indirekcie(adres, liczba, krok) | rysowanie po indirekcie |

### compute i synchronizacja

| metoda | opis |
|--------|------|
| oblicz_wyslij(grupa_x, grupa_y, grupa_z) | wysyla zadanie compute shader |
| oblicz_wyslij_po_indirekcie(adres) | compute po indirekcie |
| ogrodzenie_stworz/sygnalizuj/poczekaj/zniszcz | ogrodzenia (fences) |
| semafor_stworz/sygnalizuj/poczekaj/zniszcz | semafory |
| int czekaj_na_bezczynnosc(timeout_us) | czeka na bezczynnosc GPU |
| uint64_t znacznik_czasu(void) | zwraca znacznik czasu GPU |

### zasilanie, zegary, temperatura, wiatrak, RGB

| metoda | opis |
|--------|------|
| ustaw_stan_zasilania(stan) | ustawia stan ACPI D0-D3 |
| pobierz_stan_zasilania(void) | zwraca aktualny stan zasilania |
| ustaw_zegar_graficzny(mhz) | ustawia zegar rdzenia GPU |
| ustaw_zegar_pamieci(mhz) | ustawia zegar pamieci |
| pobierz_zegar_graficzny(void) | zwraca zegar rdzenia MHz |
| pobierz_zegar_pamieci(void) | zwraca zegar pamieci MHz |
| pobierz_temperature(void) | zwraca temperature GPU |
| ustaw_predkosc_wiatraka(obroty) | ustawia predkosc wiatraka RPM |
| pobierz_predkosc_wiatraka(void) | zwraca predkosc wiatraka |
| ustaw_rgb(kolor, tryb) | sterowanie podswietleniem RGB |
| wylacz_rgb(void) | wylacza RGB |

### swap chain i presentation

| metoda | opis |
|--------|------|
| lancuch_stworz(szerokosc, wysokosc, vsync, alpha_premultiplied) | tworzy lancuch zamiany buforow |
| lancuch_zniszcz(uchwyt) | niszczy lancuch |
| prezentuj(lancuch) | prezentuje klatke (przelacza front/back buffer) |

### command buffer

| metoda | opis |
|--------|------|
| polecenia_rozpocznij() | rozpoczyna nagrywanie do command buffera |
| polecenia_zakoncz() | konczy i oddaje command buffer do wykonania |
| kontekst_pobierz(wyjscie) | zwraca aktualny kontekst graficzny |
| int akceleracja_3d_dostepna(void) | 1 gdy akceleracja 3D aktywna |

### API C

wszystkie funkcje z prefixem `radeon_hd6000_` otaczaja metody klasy `RadeonHD6000` i maja linkage C (np. `radeon_hd6000_init`, `radeon_hd6000_putpixel`, `radeon_hd6000_shader_stworz`, `radeon_hd6000_rysuj`, `radeon_hd6000_oblicz_wyslij`, `radeon_hd6000_prezentuj` itd.).

### rejestry MMIO (wybrane)

| rejestr | offset | opis |
|---------|--------|------|
| CP_ME_CNTL | 0x08BC | sterowanie Command Processor |
| CP_ME_RB_ADDR | 0x08C4 | adres ring buffer |
| CP_ME_RB_SIZE | 0x08C8 | rozmiar ring buffer |
| CP_ME_RB_WPTR | 0x08CC | wskaznik zapisu ring buffer |
| CP_ME_RB_RPTR | 0x08D0 | wskaznik odczytu ring buffer |
| INT_STAT | 0x0100 | status przerwan |
| GFX_CLOCK_CNTL | 0x1000 | zegar grafiki |
| MEMORY_CLOCK_CNTL | 0x1004 | zegar pamieci |
| POWER_STATE | 0x1010 | stan zasilania |
| THERMAL_TEMP | 0x1014 | temperatura GPU |
| FAN_PWM | 0x1018 | PWM wiatraka |
| FAN_RPM | 0x101C | obroty wiatraka |
| CB_TARGET0_BASE | 0x8800 | adres celu renderowania 0 |
| DB_Z_INFO | 0x8A00 | format bufora glebi |
| SQ_CONFIG | 0x9800 | konfiguracja shaderow |
| SQ_PROGRAM | 0x9860 | aktywny program shader |
| DCN_CRTC_CONTROL | 0x2004 | sterowanie wyswietlaczem |

### integracja z driver managerem

sterownik jest rejestrowany przez `driver_manager_register` w `kernel/kernel.c`:

```
driver_manager_register("radeon_hd6000", radeon_hd6000_init, 0, 0, 4);
```

`radeon_hd6000_init` wywoluje sie raz przy starcie systemu przez menedzer sterownikow. Nie ma potrzeby wywolywania go recznie z kodu jadra.

