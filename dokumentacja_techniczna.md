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
