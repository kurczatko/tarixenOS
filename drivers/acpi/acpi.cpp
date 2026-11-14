/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/acpi/acpi.h>

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline void hlt(void)
{
    __asm__ volatile("hlt" ::: "memory");
}

static inline void cli(void)
{
    __asm__ volatile("cli" ::: "memory");
}

static inline void sti(void)
{
    __asm__ volatile("sti" ::: "memory");
}

static Acpi acpi_g;

Acpi *acpi_pobierz(void)
{
    return &acpi_g;
}

static uint8_t *acpi_map_fizyczny(uint32_t addr, uint32_t rozmiar)
{
    (void)rozmiar;
    return (uint8_t *)addr;
}

static uint8_t acpi_sprawdz_sume_kontrolna(const uint8_t *dane, uint32_t dlugosc)
{
    uint32_t i;
    uint8_t suma = 0;
    for (i = 0; i < dlugosc; i++) {
        suma += dane[i];
    }
    return suma;
}

static const struct acpi_sdt_header *acpi_znajdz_tabele(const char *sygnatura)
{
    (void)sygnatura;
    return 0;
}

Acpi::Acpi(void)
{
    inicjalizowane = 0;
    sci_wlaczony = 0;
    dane.available = 0;
    dane.pm1_available = 0;
    dane.pm1a_cnt = 0;
    dane.pm1b_cnt = 0;
    dane.pm1a_evt = 0;
    dane.pm1b_evt = 0;
    dane.c1_available = 0;
    dane.c2_available = 0;
    dane.c3_available = 0;
    dane.c2_latency = 0;
    dane.c3_latency = 0;
    dane.pstate_available = 0;
    dane.cpu_current_freq_mhz = 0;
    dane.cpu_max_freq_mhz = 0;
    dane.cpu_min_freq_mhz = 0;
    dane.thermal_available = 0;
    dane.cpu_temperature_c = 0;
    dane.fan_available = 0;
    dane.fan_running = 0;
    dane.fan_rpm = 0;
    dane.throttling_available = 0;
    dane.throttling_active = 0;
}

int Acpi::inicjalizuj(void)
{
    uint8_t i;
    const struct acpi_rsdp *rsdp = 0;
    const struct acpi_rsdp20 *rsdp20 = 0;
    const struct acpi_fadt *fadt = 0;
    const struct acpi_sdt_header *tabela = 0;

    rsdp = (const struct acpi_rsdp *)0x000E0000;
    for (i = 0; i < 0x20000 / 16; i++) {
        const struct acpi_rsdp *kandydat = (const struct acpi_rsdp *)((uint8_t *)0x000E0000 + i * 16);
        uint8_t j;
        int dopasowanie = 1;
        for (j = 0; j < 8; j++) {
            const char sig[8] = { 'R','S','D',' ','P','T','R',' ' };
            if (kandydat->signature[j] != sig[j]) {
                dopasowanie = 0;
                break;
            }
        }
        if (dopasowanie && acpi_sprawdz_sume_kontrolna((const uint8_t *)kandydat, sizeof(struct acpi_rsdp)) == 0) {
            rsdp = kandydat;
            break;
        }
    }

    if (!rsdp) {
        return -1;
    }

    if (rsdp->revision >= 2) {
        rsdp20 = (const struct acpi_rsdp20 *)rsdp;
        if (acpi_sprawdz_sume_kontrolna((const uint8_t *)rsdp20, sizeof(struct acpi_rsdp20)) != 0) {
            rsdp20 = 0;
        }
    }

    tabela = 0;
    if (rsdp20 && rsdp20->xsdt_address != 0) {
        tabela = (const struct acpi_sdt_header *)acpi_map_fizyczny(rsdp20->xsdt_address, sizeof(struct acpi_sdt_header));
    } else if (rsdp->rsdt_address != 0) {
        tabela = (const struct acpi_sdt_header *)acpi_map_fizyczny(rsdp->rsdt_address, sizeof(struct acpi_sdt_header));
    }

    if (!tabela) {
        return -1;
    }

    fadt = (const struct acpi_fadt *)acpi_znajdz_tabele("FACP");
    if (!fadt) {
        return -1;
    }

    dane.available = 1;
    dane.pm1_available = 0;

    if (fadt->pm1a_control_block != 0) {
        dane.pm1a_cnt = fadt->pm1a_control_block;
        dane.pm1a_evt = fadt->pm1a_event_block;
        dane.pm1_available = 1;
    }
    if (fadt->pm1b_control_block != 0) {
        dane.pm1b_cnt = fadt->pm1b_control_block;
        dane.pm1b_evt = fadt->pm1b_event_block;
    }

    dane.c1_available = 1;
    dane.c2_available = (fadt->c2_latency != 0xFFFF);
    dane.c3_available = (fadt->c3_latency != 0xFFFF);
    dane.c2_latency = fadt->c2_latency;
    dane.c3_latency = fadt->c3_latency;

    dane.pstate_available = 1;
    dane.cpu_max_freq_mhz = 2400;
    dane.cpu_min_freq_mhz = 800;
    dane.cpu_current_freq_mhz = dane.cpu_max_freq_mhz;

    dane.thermal_available = 1;
    dane.cpu_temperature_c = 45;

    dane.fan_available = 1;
    dane.fan_running = 1;
    dane.fan_rpm = 2000;

    dane.throttling_available = 1;
    dane.throttling_active = 0;

    wlacz_sci();
    inicjalizowane = 1;
    return 0;
}

void Acpi::odswiez(void)
{
    if (!inicjalizowane) {
        return;
    }
    dane.cpu_temperature_c = 45;
    dane.fan_rpm = 2000;
    dane.throttling_active = (dane.cpu_temperature_c > 80) ? 1 : 0;
}

uint8_t Acpi::czy_dostepny(void)
{
    return dane.available;
}

struct acpi_platform_info *Acpi::pobierz_info(void)
{
    return &dane;
}

void Acpi::wlacz_sci(void)
{
    if (!dane.available || !dane.pm1_available) {
        return;
    }
    uint16_t val = inw((uint16_t)dane.pm1a_cnt);
    outw((uint16_t)dane.pm1a_cnt, (uint16_t)(val | 0x0001U));
    sci_wlaczony = 1;
}

void Acpi::wylacz_sci(void)
{
    if (!dane.available || !dane.pm1_available) {
        return;
    }
    uint16_t val = inw((uint16_t)dane.pm1a_cnt);
    outw((uint16_t)dane.pm1a_cnt, (uint16_t)(val & (uint16_t)~0x0001U));
    sci_wlaczony = 0;
}

uint16_t Acpi::czytaj_pm1a_cnt(void)
{
    if (!dane.available || !dane.pm1_available) {
        return 0;
    }
    return inw((uint16_t)dane.pm1a_cnt);
}

void Acpi::zapisz_pm1a_cnt(uint16_t wartosc)
{
    if (!dane.available || !dane.pm1_available) {
        return;
    }
    outw((uint16_t)dane.pm1a_cnt, wartosc);
}

void Acpi::wejdz_w_sleep(uint8_t stan)
{
    if (!dane.available || !dane.pm1_available) {
        return;
    }
    uint16_t val = inw((uint16_t)dane.pm1a_cnt);
    val &= (uint16_t)~0x0007U;
    val |= (uint16_t)((stan & 0x7U) << 1);
    val |= 0x2000U;
    outw((uint16_t)dane.pm1a_cnt, val);
    hlt();
}

void Acpi::zamknij(void)
{
    wejdz_w_sleep(5U);
}

void Acpi::zrestartuj(void)
{
    uint8_t reset_val = 0;
    uint16_t val;

    if (dane.available) {
        reset_val = 0x06;
    }

    cli();
    val = inw(0xCF9);
    outw(0xCF9, (uint16_t)(val & (uint16_t)~0x02U));
    outb(0x06, reset_val);
    hlt();
}

int Acpi::cpu_wejdz_w_cstate(int stan)
{
    if (!dane.available) {
        return -1;
    }
    if (stan == 1) {
        hlt();
        return 0;
    }
    if (stan == 2 && dane.c2_available) {
        cli();
        hlt();
        sti();
        return 0;
    }
    if (stan == 3 && dane.c3_available) {
        cli();
        hlt();
        sti();
        return 0;
    }
    return -1;
}

int Acpi::cpu_ustaw_pstate(uint8_t pstate)
{
    (void)pstate;
    if (!dane.available || !dane.pstate_available) {
        return -1;
    }
    return 0;
}

uint32_t Acpi::cpu_pobierz_czestotliwosc_mhz(void)
{
    return dane.cpu_current_freq_mhz;
}

int Acpi::cpu_ogranicz(uint8_t procent)
{
    if (!dane.available) {
        return -1;
    }
    if (procent > 100) {
        procent = 100;
    }
    if (procent == 0) {
        procent = 10;
    }
    dane.cpu_current_freq_mhz = dane.cpu_max_freq_mhz * (uint32_t)procent / 100U;
    if (dane.cpu_current_freq_mhz < dane.cpu_min_freq_mhz) {
        dane.cpu_current_freq_mhz = dane.cpu_min_freq_mhz;
    }
    return 0;
}

int Acpi::wiatrak_pobierz_stan(void)
{
    if (!dane.available || !dane.fan_available) {
        return -1;
    }
    return dane.fan_running ? 1 : 0;
}

int Acpi::wiatrak_ustaw_stan(uint8_t wlaczony)
{
    if (!dane.available || !dane.fan_available) {
        return -1;
    }
    dane.fan_running = wlaczony ? 1 : 0;
    return 0;
}

uint32_t Acpi::wiatrak_pobierz_obroty(void)
{
    if (!dane.available || !dane.fan_available) {
        return 0;
    }
    return dane.fan_rpm;
}

int acpi_init(void)
{
    return acpi_g.inicjalizuj();
}

void acpi_odswiez(void)
{
    acpi_g.odswiez();
}

uint8_t acpi_czy_dostepny(void)
{
    return acpi_g.czy_dostepny();
}

struct acpi_platform_info *acpi_info(void)
{
    return acpi_g.pobierz_info();
}

void acpi_wlacz_sci(void)
{
    acpi_g.wlacz_sci();
}

void acpi_wylacz_sci(void)
{
    acpi_g.wylacz_sci();
}

uint16_t acpi_czytaj_pm1a_cnt(void)
{
    return acpi_g.czytaj_pm1a_cnt();
}

void acpi_zapisz_pm1a_cnt(uint16_t wartosc)
{
    acpi_g.zapisz_pm1a_cnt(wartosc);
}

void acpi_wejdz_w_sleep(uint8_t stan)
{
    acpi_g.wejdz_w_sleep(stan);
}

void acpi_zamknij(void)
{
    acpi_g.zamknij();
}

void acpi_zrestartuj(void)
{
    acpi_g.zrestartuj();
}

int acpi_cpu_wejdz_w_cstate(int stan)
{
    return acpi_g.cpu_wejdz_w_cstate(stan);
}

int acpi_cpu_ustaw_pstate(uint8_t pstate)
{
    return acpi_g.cpu_ustaw_pstate(pstate);
}

uint32_t acpi_cpu_pobierz_czestotliwosc_mhz(void)
{
    return acpi_g.cpu_pobierz_czestotliwosc_mhz();
}

int acpi_cpu_ogranicz(uint8_t procent)
{
    return acpi_g.cpu_ogranicz(procent);
}

int acpi_wiatrak_pobierz_stan(void)
{
    return acpi_g.wiatrak_pobierz_stan();
}

int acpi_wiatrak_ustaw_stan(uint8_t wlaczony)
{
    return acpi_g.wiatrak_ustaw_stan(wlaczony);
}

uint32_t acpi_wiatrak_pobierz_obroty(void)
{
    return acpi_g.wiatrak_pobierz_obroty();
}
