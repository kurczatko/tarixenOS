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

wysyla ramke ethernet przez kartę. oczekuje az deskryptor nadawczy zostanie zwolniony przez kartę (wlasciciel = sterownik).

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
| services | lista usług sdp |
| service_count | liczba usług |

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

zwracaja szerokosc i wysokosc ekranu (1024x768).

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

czyści render target podanym kolorem.

```
void gma_x4500_clear_render_target(gma_handle_t rt, uint32_t color)
```

#### gma_x4500_clear_depth_stencil

czyści depth-stencil podanymi wartosciami glebi i stencil.

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

zwracaja szerokosc i wysokosc ekranu (1024x768).

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

tworzy lub niszczy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygładzone, alpha to coverage dla przezroczystosci).

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

czyści render target podanym kolorem.

```
void radeon_hd_8370d_clear_render_target(radeon_handle_t rt, uint32_t color)
```

#### radeon_hd_8370d_clear_depth_stencil

czyści depth-stencil podanymi wartosciami glebi i stencil.

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

tworzy lub niszczy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygładzone, alpha to coverage, depth bias, rasteryzacja zachowawcza).

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

ustawia liczba probek MSAA (wieloprobekowe wygładzanie krawedzi).

```
void radeon_6790_set_msaa_samples(uint32_t samples)
```

### rysowanie 3D

#### radeon_6790_clear_render_target

czyści render target podanym kolorem.

```
void radeon_6790_clear_render_target(radeon_6790_handle_t rt, uint32_t color)
```

#### radeon_6790_clear_depth_stencil

czyści depth-stencil podanymi wartosciami glebi i stencil.

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
| switch | przełącznik |

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

Zaawansowany sterownik plyty glownej ASUS P4P800 (chipset Intel i848P + ICH4). Obsluguje wszystkie wbudowane kontrolery plyty: PCI, chipset, ACPI, SMBus, AGP, audio AC'97, ethernet Intel 82562ET, IDE z DMA, USB UHCI/EHCI, stację dyskowetek, RTC/CMOS oraz monitorowanie sprzętowe (HWMON). Sterownik jest napisany w C++, eksportuje interfejs C przez naglwek asus_p4p800.h i jest inicjalizowany z kernel/kernel.c przez `p4p_init()`.

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
| `int p4p_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)` | Wyszukuje urządzenie PCI po vendor/device ID, zwraca index dla wielu urządzeń tego samego typu. |
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
| `void p4p_acpi_sleep(uint8_t state)` | Wprowadza system w stan uśpienia ACPI (S1-S5) przez rejestr PM1A_CNT. |
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
| `void p4p_eth_stats_get(struct p4p_eth_stats *out)` | Statystyki ethernetu (tx/rx pakiety, bajty, błędy, resety). |

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
| `void p4p_ide_stats_get(struct p4p_ide_stats *out)` | Statystyki IDE (reads, writes, sektory, DMA transfery, błędy). |

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
| `void p4p_usb_stats_get(struct p4p_usb_stats *out)` | Statystyki USB (porty, resety, transfery, błędy). |

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
| `void p4p_floppy_stats_get(struct p4p_floppy_stats *out)` | Statystyki FDC (reads, writes, seeks, błędy). |

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

### Monitorowanie sprzętowe (HWMON, Super I/O)

| funkcja | opis |
|---------|------|
| `int p4p_hwmon_init(void)` | Inicjalizuje Super I/O i wlacza logiczne urzadzenie HWMON. |
| `int p4p_hwmon_is_ready(void)` | Zwraca 1 gdy HWMON jest gotowe. |
| `int p4p_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius)` | Odczyt temperatury z czujnika w milicelcjuszach. |
| `int p4p_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt)` | Odczyt napiecia (placeholder). |
| `int p4p_hwmon_read_fan(uint8_t fan, uint32_t *rpm)` | Odczyt obrotow wentylatora (placeholder). |
| `void p4p_hwmon_stats_get(struct p4p_hwmon_stats *out)` | Statystyki HWMON (temperatury, napiecia, wentylatory, błędy). |

### Obsluga przerwan (IRQ)

| funkcja | opis |
|---------|------|
| `void p4p_irq_handler(uint8_t irq)` | Dysponuje przerwanie do zarejestrowanego handlera dla danego IRQ. |
| `int p4p_irq_register(uint8_t irq, void (*handler)(uint8_t irq))` | Rejestruje handler przerwania dla IRQ 0-15. |

### Integracja z jądrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `intel_uhd_init()`:

```
p4p_init();
```

Nagłówek `drivers/plyty_glowne/asus_p4p800.h` jest dołączany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywoływane z kodu C jadra.

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

### Pamieć (PAM, DRAM, AGP)

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
| `int i915_gme_enable(uint8_t enable)` | Wlacza/wylacza Graficzną Engine MCH (GME). |
| `uint32_t i915_stolen_memory_size(void)` | Zwraca rozmiar pamieci ukradzionej (stolen memory) dla grafiki. |
| `int i915_agp_init(void)` | Inicjalizuje aperture AGP i wykrywa jego adres bazowy. |
| `uint32_t i915_agp_aperture(void)` | Zwraca rozmiar aperture AGP. |
| `int i915_agp_bind(uint32_t gatt_index, uint32_t physical_addr)` | Bindowanie strony GATT do adresu fizycznego. |
| `int i915_agp_unbind(uint32_t gatt_index)` | Unbindowanie strony GATT. |

### Grafika Intel GMA 900 (zintegrowana w MCH)

| funkcja | opis |
|---------|------|
| `int i915_graphics_init(void)` | Inicjalizuje kontroler graficzny: MMIO, DPLL, timingi pipe A, płaszczyzna A, ring buffer blitera, kursor sprzetowy. |
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
| `void i915_graphics_pipe_config(uint8_t pipe, uint8_t enable)` | Konfiguruje pipe wyświetlania (A lub B). |
| `void i915_graphics_plane_config(uint8_t plane, uint32_t addr, uint32_t width, uint32_t height, uint32_t stride)` | Konfiguruje płaszczyzne wyswietlania. |
| `void i915_graphics_blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)` | Blitter: wypelnienie prostokatem (XY_FILL_BLT). |
| `void i915_graphics_blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)` | Blitter: kopiowanie bloku pikseli (XY_SRC_COPY_BLT). |
| `void *i915_graphics_get_fb(void)` | Zwraca wskaznik na ramke graficzna (framebuffer). |
| `uint32_t i915_graphics_get_width(void)` | Zwraca szerokosc ekranu (1024). |
| `uint32_t i915_graphics_get_height(void)` | Zwraca wysokosc ekranu (768). |
| `uint32_t i915_graphics_get_pitch(void)` | Zwraca pitch (odstep miedzy wierszami) ramki graficznej (4096). |
| `uint16_t i915_graphics_get_device_id(void)` | Zwraca ID urzadzenia graficznego. |
| `uint8_t i915_graphics_get_revision(void)` | Zwraca rewizje karty graficznej. |
| `void i915_graphics_info_get(struct i915_graphics_info *out)` | Pobiera pelne informacje o urzadzeniu graficznym. |
| `void i915_graphics_stats_get(struct i915_graphics_stats *out)` | Pobiera statystyki grafiki (putpixels, fillrects, blts, cursor_moves, overlay_updates, ring_flushes, błędy). |
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
| `void i915_ide_stats_get(struct i915_ide_stats *out)` | Statystyki IDE (reads, writes, sektory, DMA transfery, błędy). |

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
| `void i915_usb_stats_get(struct i915_usb_stats *out)` | Statystyki USB (porty, resety, transfery, błędy). |

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

### Monitorowanie sprzętowe (HWMON, Super I/O)

| funkcja | opis |
|---------|------|
| `int i915_hwmon_init(void)` | Inicjalizuje Super I/O i wlacza logiczne urzadzenie HWMON. |
| `int i915_hwmon_is_ready(void)` | Zwraca 1 gdy HWMON jest gotowe. |
| `int i915_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius)` | Odczyt temperatury z czujnika w milicelcjuszach. |
| `int i915_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt)` | Odczyt napiecia (placeholder). |
| `int i915_hwmon_read_fan(uint8_t fan, uint32_t *rpm)` | Odczyt obrotow wentylatora (placeholder). |
| `void i915_hwmon_stats_get(struct i915_hwmon_stats *out)` | Statystyki HWMON (temperatury, napiecia, wentylatory, błędy). |

### Obsługa przerwan (IRQ)

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

### Integracja z jądrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `p4p_init()`:

```
i915_init();
```

Nagłówek `drivers/plyty_glowne/intel_915.h` jest dołączany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywoływane z kodu C jadra. Plik zrodlowy `drivers/plyty_glowne/intel_915.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/plyty_glowne/*.cpp)`).

## karta graficzna - drivers/karty_graficzne/radeon_rx_9070_xt.cpp

Zaawansowany sterownik karty graficznej Gigabyte Radeon RX 9070 XT GAMING OC 16GB GDDR6 (architektura RDNA 4 / Navi 48) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP GFX, CP COMPUTE, CP DMA), ray tracing (RT), mesh shaders, variable rate shading (VRS), delta color compression (DCC), przezroczystosc (alpha blending, alpha to coverage), tekstury z mipmapami i kompresja (ASTC, DXT), shadery (vertex, pixel, geometry, compute, ray gen, any hit, closest hit, miss, callable, intersection, mesh, amplification), render targety, depth-stencil, fences, semaphory, command buffer oraz presentation (swap chain, vsync, HDR, triple buffer). Sterownik zarządza 16 GB pamięci GDDR6 (VRAM allocator), taktowaniem GPU i pamięci, temperatura, wentylatorem oraz sterowaniem RGB. Eksportuje interfejs C przez nagłówek radeon_rx_9070_xt.h i jest inicjalizowany z kernel/kernel.c przez `rx_9070_xt_init()`.

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

### Zarządzanie pamięcią VRAM (16 GB GDDR6)

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
| `rx_9070_xt_handle_t rx_9070_xt_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage, uint8_t vrs)` | Tworzy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygładzone, alpha to coverage, VRS). Zwraca handle. |
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
| `void rx_9070_xt_clear_render_target(rx_9070_xt_handle_t rt, uint32_t color)` | Czyści render target podanym kolorem. |
| `void rx_9070_xt_clear_depth_stencil(rx_9070_xt_handle_t ds, float depth, uint8_t stencil)` | Czyści depth-stencil podanymi wartosciami glebi i stencil. |
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
| `int rx_9070_xt_fence_wait(rx_9070_xt_handle_t handle, uint32_t value, uint32_t timeout_us)` | Czeka na sygnalizację fence do podanej wartosci z timeoutem w mikrosekundach. Zwraca 1 gdy fence zostal sygnalizowany, 0 przy timeoutcie. |
| `void rx_9070_xt_fence_destroy(rx_9070_xt_handle_t handle)` | Niszczy fence o podanym handle. |
| `void rx_9070_xt_semaphore_create(rx_9070_xt_handle_t *out)` | Tworzy semafor (synchronizacja miedzy etapami potoku) i zwraca jego handle. |
| `void rx_9070_xt_semaphore_signal(rx_9070_xt_handle_t handle, uint32_t value)` | Sygnalizuje semafor z podana wartoscia. |
| `void rx_9070_xt_semaphore_wait(rx_9070_xt_handle_t handle, uint32_t value)` | Czeka na sygnalizację semafora do podanej wartosci (blokujace). |
| `void rx_9070_xt_semaphore_destroy(rx_9070_xt_handle_t handle)` | Niszczy semafor o podanym handle. |

### Zarządzanie zasilaniem, zegarami, temperatura, wentylator, RGB

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
| `RX_9070_XT_HANDLE_NULL` | Wartość nulowa dla handle zasobow (0xFFFFFFFFU). |
| `rx_9070_xt_handle_t` | Typ handle zasobu (uint32_t). |
| `RX_9070_XT_*` | Stale dla formatow vertex, indeksow, primitivow, tekstur, shaderow, blendowania, operacji blendowania, porownan glebi, wypelnienia, cullowania, operacji stencil (patrz naglowek radeon_rx_9070_xt.h). |

### Integracja z jądrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `radeon_hd_8370d_init()`:

```
rx_9070_xt_init();
```

Nagłówek `drivers/karty_graficzne/radeon_rx_9070_xt.h` jest dołączany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywoływane z kodu C jadra. Plik zrodlowy `drivers/karty_graficzne/radeon_rx_9070_xt.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/karty_graficzne/*.cpp)`).

## karta graficzna - drivers/karty_graficzne/radeon_hd_6870.cpp

Zaawansowany sterownik karty graficznej AMD Radeon HD 6870 (architektura Northern Islands / Cayman, VLIW4) napisany w C++. obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa przez Command Processor (CP), przezroczystosc (alpha blending), tekstury z mipmapami i kompresja (DXT), shadery (vertex, pixel, geometry, compute), render targety, depth-stencil, fences, semaphory, command buffer oraz presentation (swap chain, vsync). Sterownik zarządza pamiecia GDDR5 (VRAM allocator), taktowaniem GPU i pamięci, temperatura, wentylatorem oraz sterowaniem RGB. Eksportuje interfejs C przez nagłówek radeon_hd_6870.h i jest inicjalizowany z kernel/kernel.c przez `radeon_6870_init()`.

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

### Zarządzanie pamięcią VRAM (GDDR5)

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
| `radeon_6870_handle_t radeon_6870_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage)` | Tworzy stan rasterizera (wypelnienie, cullowanie, scissor, multisample, linie wygładzone, alpha to coverage). Zwraca handle. |
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
| `void radeon_6870_clear_render_target(radeon_6870_handle_t rt, uint32_t color)` | Czyści render target podanym kolorem. |
| `void radeon_6870_clear_depth_stencil(radeon_6870_handle_t ds, float depth, uint8_t stencil)` | Czyści depth-stencil podanymi wartosciami glebi i stencil. |
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
| `int radeon_6870_fence_wait(radeon_6870_handle_t handle, uint32_t value, uint32_t timeout_us)` | Czeka na sygnalizację fence do podanej wartosci z timeoutem w mikrosekundach. Zwraca 1 gdy fence zostal sygnalizowany, 0 przy timeoutcie. |
| `void radeon_6870_fence_destroy(radeon_6870_handle_t handle)` | Niszczy fence o podanym handle. |
| `void radeon_6870_semaphore_create(radeon_6870_handle_t *out)` | Tworzy semafor (synchronizacja miedzy etapami potoku) i zwraca jego handle. |
| `void radeon_6870_semaphore_signal(radeon_6870_handle_t handle, uint32_t value)` | Sygnalizuje semafor z podana wartoscia. |
| `void radeon_6870_semaphore_wait(radeon_6870_handle_t handle, uint32_t value)` | Czeka na sygnalizację semafora do podanej wartosci (blokujace). |
| `void radeon_6870_semaphore_destroy(radeon_6870_handle_t handle)` | Niszczy semafor o podanym handle. |

### Zarządzanie zasilaniem, zegarami, temperatura, wentylator, RGB

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
| `RADEON_6870_HANDLE_NULL` | Wartość nulowa dla handle zasobow (0xFFFFFFFFU). |
| `radeon_6870_handle_t` | Typ handle zasobu (uint32_t). |
| `RADEON_6870_*` | Stale dla formatow vertex, indeksow, primitivow, tekstur, shaderow, blendowania, operacji blendowania, porownan glebi, wypelnienia, cullowania, operacji stencil (patrz naglowek radeon_hd_6870.h). |

### Integracja z jądrem

Sterownik jest inicjalizowany w `kernel/kernel.c` w funkcji `kernel_main`, zaraz po `rx_9070_xt_init()`:

```
radeon_6870_init();
```

Nagłówek `drivers/karty_graficzne/radeon_hd_6870.h` jest dołączany w `kernel/kernel.c`. Wszystkie funkcje eksportowane sa jako C (`extern "C"`), wiec moga byc wywoływane z kodu C jadra. Plik zrodlowy `drivers/karty_graficzne/radeon_hd_6870.cpp` jest kompilowany automatycznie przez makefile (wildcard `$(wildcard drivers/karty_graficzne/*.cpp)`).

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
| `rtl8111e_init()` | inicjalizuje kartę: PCI, MMIO, reset, MAC, rejestry, bufor, TX/RX. zwraca 1 przy sukcesie, 0 przy porażce. |
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

PHY jest dostępny przez rejestr `REG_PHYAR` (bit 31 = zajęty, bit 30 = zapis, bity 23-16 = adres rejestru, bity 15-0 = dane). Uzywane rejestry PHY: BMCR (0) i BMSR (1).

integracja z przegladarka:

`drivers/net.c` wywoluje `rtl8111e_init()` jako pierwsze w lancuchu inicjalizacji sieci (przed `rtl8168b`, `rtl8188ce`, `e1000`, `rtl8139`). gdy karta RTL8111E jest wykryta, `net_send_fn` i `net_receive_fn` wskazuja na funkcje sterownika rtl8111e. przegladarka (`kernel/browser.c`) uzywa `net_ready()` oraz `net_http_get()` z `drivers/net.c`, wiec po udanej inicjalizacji sterownika RTL8111E przegladarka pobiera strony przez ta karte automatycznie. w środowisku QEMU (domyslna karta RTL8139) lacze jest nawiazywane przez sterownik rezerwowy `rtl8139`.

przyklad uruchomienia:

```
make
cd build
qemu-system-x86_64 -m 2G -boot c -drive file=tarixenOS.img,format=raw -machine pc -display sdl
```

