/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <kernel/syscall.h>
#include <kernel/paging.h>
#include <graficzny/rysowanie.h>
#include <graficzny/okno.h>
#include <graficzny/czcionka.h>
#include <include/printf/printf.h>
#include <system_plikow/system_plikow.h>
#include <kernel/proces.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <kernel/loader.h>
#include <user_land/tapeta.h>

struct rejestry biezace_rejestry;

static struct open_file_desc {
    uint8_t used;
    uint32_t cluster;
    uint32_t position;
    uint32_t size;
    char name[13];
} open_files[16];

uint32_t userspace_break = 0x00100000;

static int alloc_fd(void) {
    for (int i = 0; i < 16; i++) if (!open_files[i].used) return i;
    return -1;
}

static uint32_t __attribute__((unused)) heap_end(void) {
    return userspace_break;
}

#define KOLO_SRODEK_X 250
#define KOLO_SRODEK_Y 80
#define KOLO_PROMIEN 40

static uint32_t pierwiastkuj(uint32_t n)
{
    if (n == 0) return 0;
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

static int kat_xy(int dx, int dy)
{
    if (dx == 0 && dy == 0) return 0;
    int ax = dx < 0 ? -dx : dx;
    int ay = dy < 0 ? -dy : dy;
    int kat;
    if (ax >= ay) {
        int baza = (ay * 45) / ax;
        if (dx >= 0 && dy >= 0) kat = baza;
        else if (dx < 0 && dy >= 0) kat = 180 - baza;
        else if (dx < 0 && dy < 0) kat = 180 + baza;
        else kat = 360 - baza;
    } else {
        int baza = 90 - (ax * 45) / ay;
        if (dx >= 0 && dy >= 0) kat = baza;
        else if (dx < 0 && dy >= 0) kat = 180 - baza;
        else if (dx < 0 && dy < 0) kat = 180 + baza;
        else kat = 360 - baza;
    }
    if (kat == 360) kat = 0;
    return kat;
}

static uint32_t hsv_na_xrgb(int h, uint32_t s, uint32_t v)
{
    uint32_t c = (v * s) / 255;
    uint32_t sektor = (uint32_t)h / 60;
    uint32_t f = (uint32_t)h % 60;
    uint32_t r, g, b;
    switch (sektor) {
    case 0: r = c; g = (c * f) / 60; b = 0; break;
    case 1: r = (c * (60 - f)) / 60; g = c; b = 0; break;
    case 2: r = 0; g = c; b = (c * f) / 60; break;
    case 3: r = 0; g = (c * (60 - f)) / 60; b = c; break;
    case 4: r = (c * f) / 60; g = 0; b = c; break;
    default: r = c; g = 0; b = (c * (60 - f)) / 60; break;
    }
    uint32_t m = v - c;
    r += m;
    g += m;
    b += m;
    return (r << 16) | (g << 8) | b;
}

void tapeta_picker_rysuj(int x, int y, int szerokosc, int wysokosc)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    int srodek_x = x + KOLO_SRODEK_X - 8;
    int srodek_y = y + KOLO_SRODEK_Y - 30;
    int granica = (KOLO_PROMIEN - 1) * (KOLO_PROMIEN - 1);
    (void)szerokosc;
    (void)wysokosc;
    rysuj_tekst_chicago(x, y, "WYBIERZ TAPETE", 0x00000000);
    uint32_t kolor_ciemnej = tapeta_czy_jasna() ? 0x00C0C0C0 : 0x003B82F6;
    uint32_t kolor_jasnej = tapeta_czy_jasna() ? 0x003B82F6 : 0x00C0C0C0;
    rysuj_prostokat(x + 8, y + 100, 144, 24, kolor_ciemnej);
    rysuj_tekst_chicago(x + 18, y + 108, "CIEMNA", 0x00000000);
    rysuj_prostokat(x + 176, y + 100, 144, 24, kolor_jasnej);
    rysuj_tekst_chicago(x + 186, y + 108, "JASNA", 0x00000000);
    for (int indeks = 0; indeks < 8; indeks++)
        rysuj_prostokat(x + 8 + (indeks % 4) * 32, y + 18 + (indeks / 4) * 32, 24, 24, kolory[indeks]);
    for (int dy = -KOLO_PROMIEN; dy <= KOLO_PROMIEN; dy++) {
        for (int dx = -KOLO_PROMIEN; dx <= KOLO_PROMIEN; dx++) {
            uint32_t r2 = (uint32_t)(dx * dx + dy * dy);
            if (r2 > (uint32_t)(KOLO_PROMIEN * KOLO_PROMIEN)) continue;
            if (r2 > (uint32_t)granica) {
                rysuj_piksel(srodek_x + dx, srodek_y + dy, 0x00000000);
                continue;
            }
            uint32_t r = pierwiastkuj(r2);
            int h = kat_xy(dx, dy);
            uint32_t nasycenie = r * 255 / KOLO_PROMIEN;
            rysuj_piksel(srodek_x + dx, srodek_y + dy, hsv_na_xrgb(h, nasycenie, 255));
        }
    }
}

void tapeta_picker_klik(int x, int y)
{
    static const uint32_t kolory[] = {
        0x001E293B, 0x00334155, 0x00E74C3C, 0x0027AE60,
        0x003B82F6, 0x0010B981, 0x00EF4444, 0x00FBBF24
    };
    int kolumna;
    int wiersz;
    if (y >= 130 && y < 154) {
        if (x >= 16 && x < 160 && tapeta_ustaw_obraz(0)) tapeta_zapisz();
        else if (x >= 184 && x < 328 && tapeta_ustaw_obraz(1)) tapeta_zapisz();
        return;
    }
    if (x >= 16 && x < 144 && y >= 48 && y < 112) {
        kolumna = (x - 16) / 32;
        wiersz = (y - 48) / 32;
        if (kolumna >= 0 && kolumna < 4 && wiersz >= 0 && wiersz < 2) {
            ustaw_kolor_tapety(kolory[wiersz * 4 + kolumna]);
            tapeta_zapisz();
        }
        return;
    }
    int dx = x - KOLO_SRODEK_X;
    int dy = y - KOLO_SRODEK_Y;
    uint32_t r2 = (uint32_t)(dx * dx + dy * dy);
    if (r2 > (uint32_t)(KOLO_PROMIEN * KOLO_PROMIEN)) return;
    uint32_t r = pierwiastkuj(r2);
    int h = kat_xy(dx, dy);
    uint32_t nasycenie = r * 255 / KOLO_PROMIEN;
    ustaw_kolor_tapety(hsv_na_xrgb(h, nasycenie, 255));
    tapeta_zapisz();
}

static struct okno *pobierz_okno_procesu(int handle)
{
    struct proces *p = proces_aktualny();
    if (!p) return 0;
    if (handle < 0 || handle >= MAX_OKNA_PROCESU) return 0;
    if (!p->okna_uzyte[handle]) return 0;
    return &p->okna[handle];
}

static int znajdz_wolny_slot_okna(void)
{
    struct proces *p = proces_aktualny();
    if (!p) return -1;
    for (int i = 0; i < MAX_OKNA_PROCESU; i++) {
        if (!p->okna_uzyte[i]) return i;
    }
    return -1;
}

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
            return pobierz_kolor_tapety();
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
            if (!wskaznik_uzytkownika_prawidlowy((const void *)edx))
                return 0xFFFFFFFF;
            rysuj_tekst_chicago((int)ebx, (int)ecx, (const char *)edx, (uint32_t)esi);
            return 0;
        case SYSCALL_OKNO_STWÓRZ: {
            if (!wskaznik_uzytkownika_prawidlowy((const void *)edi))
                return 0xFFFFFFFF;
            int slot = znajdz_wolny_slot_okna();
            if (slot < 0) return 0xFFFFFFFF;
            struct proces *proc = proces_aktualny();
            if (!proc) return 0xFFFFFFFF;
            struct okno *okno = &proc->okna[slot];
            okno_stworz(okno, (int)ebx, (int)ecx, (int)edx, (int)esi, (const char *)edi, 0);
            proc->okna_uzyte[slot] = 1;
            return slot;
        }
        case SYSCALL_OKNO_RYSUJ: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            okno_narysuj(okno);
            return 0;
        }
        case SYSCALL_OKNO_ZAMKNIJ: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            okno_zamknij(okno);
            struct proces *proc = proces_aktualny();
            if (proc) proc->okna_uzyte[ebx] = 0;
            return 0;
        }
        case SYSCALL_OKNO_PIKSEL: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            int x = (int)ecx;
            int y = (int)edx;
            uint32_t color = (uint32_t)esi;
            if (x < 0 || x >= okno->szerokosc - 16 || y < 22 || y >= okno->wysokosc - 2)
                return 0xFFFFFFFF;
            rysuj_piksel(okno->x + 8 + x, okno->y + 30 + y, color);
            return 0;
        }
        case SYSCALL_OKNO_PROSTOKAT: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            int x = (int)ecx;
            int y = (int)edx;
            int w = (int)esi;
            int h = (int)edi;
            if (x < 0 || y < 22 || x + w > okno->szerokosc - 16 || y + h > okno->wysokosc - 2)
                return 0xFFFFFFFF;
            rysuj_prostokat(okno->x + 8 + x, okno->y + 30 + y, w, h, 0);
            return 0;
        }
        case SYSCALL_OKNO_TEKST: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            if (!wskaznik_uzytkownika_prawidlowy((const void *)edx))
                return 0xFFFFFFFF;
            int x = (int)ecx;
            int y = (int)edx;
            const char *text = (const char *)esi;
            uint32_t color = (uint32_t)edi;
            if (x < 0 || y < 22 || x >= okno->szerokosc - 16 || y >= okno->wysokosc - 2)
                return 0xFFFFFFFF;
            rysuj_tekst_chicago(okno->x + 8 + x, okno->y + 30 + y, text, color);
            return 0;
        }
        case SYSCALL_OKNO_KLIK: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            int x = (int)ecx;
            int y = (int)edx;
            if (x < 0 || x >= okno->szerokosc || y < 0 || y >= okno->wysokosc)
                return 0xFFFFFFFF;
            okno_obsluz_klikniecie(okno->x + x, okno->y + y);
            return 0;
        }
        case SYSCALL_OKNO_RUCH: {
            struct okno *okno = pobierz_okno_procesu((int)ebx);
            if (!okno) return 0xFFFFFFFF;
            int x = (int)ecx;
            int y = (int)edx;
            if (x < 0 || x >= okno->szerokosc || y < 0 || y >= okno->wysokosc)
                return 0xFFFFFFFF;
            okno_obsluz_ruch(okno->x + x, okno->y + y);
            return 0;
        }
        case SYSCALL_MYSZ_POZYCJA: {
            int mx, my;
            mouse_pozycja(&mx, &my);
            return (uint32_t)((mx & 0xFFFF) | ((my & 0xFFFF) << 16));
        }
        case SYSCALL_DRUKUJ:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx))
                return 0xFFFFFFFF;
            printf((const char *)ebx);
            return 0;
        case SYSCALL_WRITE_CONSOLE:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx))
                return 0xFFFFFFFF;
            for (uint32_t i = 0; i < ecx; i++) {
                char c = ((const char *)ebx)[i];
                if (c == '\n') printf("\n"); else { char tmp[2] = {c, 0}; printf(tmp); }
            }
            return (uint32_t)ecx;
        case SYSCALL_EXIT:
            proces_zakoncz(proces_pobierz_pid());
            return 0;
        case SYSCALL_BRK:
            if (ebx == 0) return userspace_break;
            if (ebx < USER_MEM_START || ebx > USER_MEM_END) return 0xFFFFFFFF;
            userspace_break = ebx;
            return userspace_break;
        case SYSCALL_OPEN: {
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx)) return 0xFFFFFFFF;
            char sector[512]; uint32_t sector_number = 0;
            struct fat32_entry *entry = fs_find_entry((const char *)ebx, sector, &sector_number);
            if (!entry || (entry->attributes & 0x10)) return -1;
            int fd = alloc_fd();
            if (fd < 0) return -1;
            open_files[fd].used = 1;
            open_files[fd].cluster = ((uint32_t)entry->cluster_high << 16) | entry->cluster_low;
            open_files[fd].position = 0;
            open_files[fd].size = entry->size;
            char name_buf[13]; uint32_t n = 0; const char *s = (const char *)ebx; while (*s && n < 12) { name_buf[n++] = *s++; } name_buf[n] = 0; for (uint32_t i = 0; i < 12; i++) open_files[fd].name[i] = name_buf[i];
            return fd;
        }
        case SYSCALL_READ: {
            if (ebx >= 16 || !open_files[ebx].used) return -1;
            if (!wskaznik_uzytkownika_prawidlowy((void *)ecx)) return 0xFFFFFFFF;
            uint32_t cluster = open_files[ebx].cluster;
            uint32_t pos = open_files[ebx].position;
            uint32_t left = open_files[ebx].size - pos;
            if (left > edx) left = edx;
            uint32_t copied = 0;
            uint32_t skip = pos;
            char sector[512];
            while (cluster >= 2 && cluster < FAT32_EOC && copied < left) {
                uint32_t csec = fs_cluster_sector(cluster);
                fs_read_sector(csec, sector);
                for (uint32_t i = 0; i < 512 && copied < left; i++) {
                    if (skip > 0) { skip--; continue; }
                    ((char *)ecx)[copied++] = sector[i];
                    open_files[ebx].position++;
                }
                cluster = fs_fat_next(cluster);
            }
            return copied;
        }
        case SYSCALL_CLOSE: {
            if (ebx >= 16 || !open_files[ebx].used) return -1;
            open_files[ebx].used = 0;
            return 0;
        }
        case SYSCALL_WRITE_FILE: {
            if (ebx >= 16 || !open_files[ebx].used) return -1;
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ecx)) return 0xFFFFFFFF;
            uint32_t cluster = open_files[ebx].cluster;
            uint32_t pos = open_files[ebx].position;
            uint32_t to_write = edx;
            uint32_t written = 0;
            char sector[512];
            uint32_t cur_cluster = cluster;
            uint32_t skip = pos;
            while (cur_cluster >= 2 && cur_cluster < FAT32_EOC && skip >= 512) {
                skip -= 512;
                cur_cluster = fs_fat_next(cur_cluster);
            }
            while (written < to_write) {
                if (cur_cluster < 2 || cur_cluster >= FAT32_EOC) {
                    cur_cluster = fs_alloc_cluster();
                    if (!cur_cluster) break;
                }
                uint32_t csec = fs_cluster_sector(cur_cluster);
                fs_read_sector(csec, sector);
                uint32_t space = 512 - skip;
                if (space > to_write - written) space = to_write - written;
                for (uint32_t i = 0; i < space; i++) sector[skip + i] = ((const char *)ecx)[written + i];
                fs_write_sector(csec, sector);
                written += space;
                open_files[ebx].position += space;
                skip = 0;
                cur_cluster = fs_fat_next(cur_cluster);
            }
            if (open_files[ebx].position > open_files[ebx].size) open_files[ebx].size = open_files[ebx].position;
            return written;
        }
        case SYSCALL_PROCES_STWORZ: {
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ecx))
                return 0xFFFFFFFF;
            struct proces *p = proces_stworz(ebx, (const char *)ecx, edx);
            if (!p) return 0xFFFFFFFF;
            return p->pid;
        }
        case SYSCALL_PROCES_ZAKONCZ:
            proces_zakoncz(ebx);
            return 0;
        case SYSCALL_PROCES_YIELD:
            proces_yield();
            return 0;
        case SYSCALL_PROCES_POBIERZ_PID:
            return proces_pobierz_pid();
        case SYSCALL_FS_LIST:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx))
                return 0xFFFFFFFF;
            return fs_list_current((struct fs_entry_info *)ebx, ecx);
        case SYSCALL_FS_CD:
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx))
                return 0xFFFFFFFF;
            return cd((const char *)ebx);
        case SYSCALL_FS_PWD:
            if (!wskaznik_uzytkownika_prawidlowy((void *)ebx))
                return 0xFFFFFFFF;
            fs_get_current_dir_name((char *)ebx);
            return 0;
        case SYSCALL_FS_RESET_DIR:
            fs_reset_directory();
            return 0;
        case SYSCALL_KLAWIATURA_GETCHAR: {
            char znak = 0;
            uint8_t ctrl = 0;
            if (keyboard_try_getchar(&znak)) {
                ctrl = keyboard_ctrl_pressed();
                return (uint32_t)(unsigned char)znak | ((uint32_t)ctrl << 8);
            }
            return 0xFFFFFFFF;
        }
        case SYSCALL_PROCES_URUCHOM_ELF: {
            if (!wskaznik_uzytkownika_prawidlowy((const void *)ebx))
                return 0xFFFFFFFF;
            int pid = proces_zaladuj_elf((const char *)ebx);
            if (pid <= 0) return 0xFFFFFFFF;
            return pid;
        }
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
