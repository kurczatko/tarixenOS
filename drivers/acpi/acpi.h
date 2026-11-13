#ifndef ACPI_H
#define ACPI_H

#include "../include/printf/types.h"

struct acpi_rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct acpi_rsdp20 {
    struct acpi_rsdp rsdp;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct acpi_sdt_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct acpi_rsdt {
    struct acpi_sdt_header header;
    uint32_t entries[1];
} __attribute__((packed));

struct acpi_xsdt {
    struct acpi_sdt_header header;
    uint64_t entries[1];
} __attribute__((packed));

struct acpi_fadt {
    struct acpi_sdt_header header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_block_length;
    uint8_t gpe1_block_length;
    uint8_t gpe1_base;
    uint8_t cstate_cnt;
    uint16_t c2_latency;
    uint16_t c3_latency;
    uint16_t cpu_flush_size;
    uint16_t cpu_flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
    uint16_t boot_architecture_flags;
    uint8_t reserved2;
    uint32_t flags;
    uint8_t reset_reg[12];
    uint8_t reset_value;
    uint16_t arm_boot_flags;
    uint8_t fadt_minor_version;
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;
    uint8_t x_pm1a_event_block[12];
    uint8_t x_pm1b_event_block[12];
    uint8_t x_pm1a_control_block[12];
    uint8_t x_pm1b_control_block[12];
    uint8_t x_gpe0_block[12];
    uint8_t x_gpe1_block[12];
} __attribute__((packed));

struct acpi_generic_address {
    uint8_t space_id;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} __attribute__((packed));

struct acpi_platform_info {
    uint8_t available;
    uint8_t pm1_available;
    uint32_t pm1a_cnt;
    uint32_t pm1b_cnt;
    uint16_t pm1a_evt;
    uint16_t pm1b_evt;
    uint8_t c1_available;
    uint8_t c2_available;
    uint8_t c3_available;
    uint16_t c2_latency;
    uint16_t c3_latency;
    uint8_t pstate_available;
    uint32_t cpu_current_freq_mhz;
    uint32_t cpu_max_freq_mhz;
    uint32_t cpu_min_freq_mhz;
    uint8_t thermal_available;
    int32_t cpu_temperature_c;
    uint8_t fan_available;
    uint8_t fan_running;
    uint32_t fan_rpm;
    uint8_t throttling_available;
    uint8_t throttling_active;
};

#ifdef __cplusplus

class Acpi {
public:
    Acpi(void);
    int inicjalizuj(void);
    void odswiez(void);
    uint8_t czy_dostepny(void);
    struct acpi_platform_info *pobierz_info(void);

    void wlacz_sci(void);
    void wylacz_sci(void);
    uint16_t czytaj_pm1a_cnt(void);
    void zapisz_pm1a_cnt(uint16_t wartosc);

    void wejdz_w_sleep(uint8_t stan);
    void zamknij(void);
    void zrestartuj(void);

    int cpu_wejdz_w_cstate(int stan);
    int cpu_ustaw_pstate(uint8_t pstate);
    uint32_t cpu_pobierz_czestotliwosc_mhz(void);
    int cpu_ogranicz(uint8_t procent);

    int wiatrak_pobierz_stan(void);
    int wiatrak_ustaw_stan(uint8_t wlaczony);
    uint32_t wiatrak_pobierz_obroty(void);

private:
    struct acpi_platform_info dane;
    uint8_t inicjalizowane;
    uint8_t sci_wlaczony;

    static uint8_t sprawdz_sume_kontrolna(const uint8_t *dane, uint32_t dlugosc);
    static const struct acpi_sdt_header *znajdz_tabele(const char *sygnatura);
    static int pobierz_bajt_msr(uint32_t msr, uint64_t *wartosc);
    static int ustaw_bajt_msr(uint32_t msr, uint64_t wartosc);
    static void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
};

Acpi *acpi_pobierz(void);

#endif

#ifdef __cplusplus
extern "C" {
#endif

int acpi_init(void);
void acpi_odswiez(void);
uint8_t acpi_czy_dostepny(void);
struct acpi_platform_info *acpi_info(void);
void acpi_wlacz_sci(void);
void acpi_wylacz_sci(void);
uint16_t acpi_czytaj_pm1a_cnt(void);
void acpi_zapisz_pm1a_cnt(uint16_t wartosc);
void acpi_wejdz_w_sleep(uint8_t stan);
void acpi_zamknij(void);
void acpi_zrestartuj(void);
int acpi_cpu_wejdz_w_cstate(int stan);
int acpi_cpu_ustaw_pstate(uint8_t pstate);
uint32_t acpi_cpu_pobierz_czestotliwosc_mhz(void);
int acpi_cpu_ogranicz(uint8_t procent);
int acpi_wiatrak_pobierz_stan(void);
int acpi_wiatrak_ustaw_stan(uint8_t wlaczony);
uint32_t acpi_wiatrak_pobierz_obroty(void);

#ifdef __cplusplus
}
#endif

#endif
