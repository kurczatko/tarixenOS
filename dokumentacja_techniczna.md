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

`kernel_main` to punkt wejscia jadra. robi nastepujące rzeczy:

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

## zarządzanie procesami - kernel/proces.c

### proces_init

inicjalizuje tablice procesow. wszystkie procesy ustawiane na zakonczone, pid na 0, licznik na 0.

```
void proces_init(void)
```

### proces_stworz

tworzy nowy proces. szuka wolnego slotu w tablicy, nadaje pid, kopiuje page directory, ustawia stos. zwraca wskaznik na proces lub 0 gdy nie ma miejsca.

```
struct proces *proces_stworz(uint32_t entry_point, const char *name, uint32_t arg)
```

### proces_zakoncz

oznacza proces o danym pid jako zakonczony i zwalnia jego slot.

```
void proces_zakoncz(uint32_t pid)
```

### proces_tick

przełącza nastepny proces gotowy do uruchomienia. round-robin po tablicy procesow.

```
void proces_tick(void)
```

### proces_aktualny

zwraca wskaznik na aktualnie dzialajacy proces.

```
struct proces *proces_aktualny(void)
```

### proces_yield

wymusza przełączenie kontekstu poprzez przerwanie softwarowe int 0x80 z eax=29.

```
void proces_yield(void)
```

### proces_pobierz_pid

zwraca pid aktualnego procesu lub 0 gdy brak procesu.

```
uint32_t proces_pobierz_pid(void)
```

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

glowna obsługa wywołań systemowych. czyta eax jako numer funkcji i reszte rejestrow jako argumenty. zwraca 0xFFFFFFFF przy bledzie.

dostępne syscally:

| numer | nazwa | opis |
|-------|-------|------|
| 0 | SYSCALL_TAPETA_ZMIEŃ | zmienia kolor tapety z ebx |
| 1 | SYSCALL_TAPETA_POBIERZ | zwraca kolor tapety |
| 2 | SYSCALL_EKRAN_CZYŚĆ | wypelnia ekran kolorem z ebx |
| 3 | SYSCALL_EKRAN_PIKSEL | rysuje piksel (ebx, ecx) kolorem edx |
| 4 | SYSCALL_EKRAN_PROSTOKĄT | rysuje prostokat (ebx, ecx, edx, esi) kolorem edi |
| 5 | SYSCALL_TEKST_RYSUJ | rysuje tekst z edx na (ebx, ecx) kolorem esi |
| 6 | SYSCALL_OKNO_STWÓRZ | tworzy okno, zwraca slot |
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

instaluje obsługa syscall w IDT.

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

### menu_start_przełącz

wlacza/wylacza menu start.

```
void menu_start_przełącz(void)
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

laduje układ ikon z dysku.

```
int ikony_wczytaj_uklad(void)
```

### ikony_zapisz_uklad

zapisuje układ ikon na dysk.

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

probuje pobrać znak, zwraca 1 gdy jest znak.

```
uint8_t keyboard_try_getchar(char *character)
```

### keyboard_discard_pending

odrzuca oczekujace znaki.

```
void keyboard_discard_pending(void)
```

### keyboard_scancode_to_char

konwertuje kod skanu na znak z uwzględnieniem shift.

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
int net_http_get(const char *host, const char *path, char *response, uint32_t capacity)
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

czytą plik do bufora.

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

zmienia katalog bieżący.

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

inicjalizacja i obsługa magistrali USB.

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

wlacz/wylacz EAX, ustaw srodowisko dzwiekowe (pokoj, room_hf, odbicia, poglos, czas rozpadu, dyfuzyjnosc, gestosc, flagi) oraz polozenie i orientacje słuchacza.

struktura `EaxEnvironment` zawiera: room, room_hf, reflections, reflections_delay, reverb, reverb_delay, decay_time, decay_hf_ratio, reflections_hf, reverb_hf, diffusion, density, flags.

### glosnosc glowna

```
void set_master_volume(uint8_t percent);
uint8_t get_master_volume(void);
```

ustawia i zwraca glosnosc glowna w procentach (0-100).

## struktura plikow

```
tarixenOS/
├── boot.asm              # bootloader i wejscie w tryb chroniony
├── kernel/
│   ├── kernel.c          # glowna petla jadra, kernel_main
│   ├── proces.c          # zarządzanie procesami
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
│   ├── okno.c            # zarządzanie oknami
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
