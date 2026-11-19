/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/*
* adapter z powiazaniem C (C linkage) dla wszystkich sterownikow dzwieku w projekcie.
* udostepnia funkcje o zwiazce C, ktore mozna wywolywac z warstwy abstrakcji
* kernel/sound.c (kompilowanej jako C). Obsluguje Wszystkie sterowniki dzwieku:
* Ensoniq ES1370, Intel HDA, Intel Raptor Lake CAVS, ALC650, VT1611, HDAudioCodec,
* ALC880, ALC1150, Audigy2.
*/

#include <drivers/sound/ensoniq_es1370.h>
#include <drivers/audio/alc650.h>
#include <drivers/karty_dzwiekowe/intel_raptorlake_cavs.h>
#include <drivers/karty_dzwiekowe/vt1611.h>
#include <drivers/karty_dzwiekowe/hda_codec.h>
#include <drivers/karty_dzwiekowe/alc880.h>
#include <drivers/karty_dzwiekowe/alc1150.h>
#include <drivers/sound/audigy2.h>
#include <include/printf/vga.h>

extern "C" {
    void init_intel_hda(void);
    void intel_hda_beep(void);
}

/* -------------------------------------------------------------
 * Pomocniczy dostep do konfiguracji PCI (32-bit, porty 0xCF8/0xCFC)
 * ------------------------------------------------------------- */

static inline uint32_t adapter_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    uint16_t port_addr = 0xCF8U;
    uint16_t port_data = 0xCFCU;
    uint32_t value;
    __asm__ volatile("outl %0, %1" : : "a"(address), "d"(port_addr));
    __asm__ volatile("inl %1, %0" : "=a"(value) : "d"(port_data));
    return value;
}

static inline void adapter_pci_write32(uint8_t bus, uint8_t slot, uint8_t func,
                                       uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    uint16_t port_addr = 0xCF8U;
    uint16_t port_data = 0xCFCU;
    __asm__ volatile("outl %0, %1" : : "a"(address), "d"(port_addr));
    __asm__ volatile("outl %0, %1" : : "a"(value), "d"(port_data));
}

static inline void adapter_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t adapter_inb(uint16_t port)
{
    uint8_t v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void adapter_delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++) {
        for (uint32_t j = 0U; j < 48000U; j++);
    }
}

/* -------------------------------------------------------------
 * Wrappers: Ensoniq ES1370 (posiada wlasne powiazanie C w es1370.cpp)
 * ------------------------------------------------------------- */

extern "C" int adapter_es1370_init(void)   { return es1370_init(); }
extern "C" void adapter_es1370_shutdown(void) { es1370_shutdown(); }
extern "C" void adapter_es1370_poll(void)   { es1370_poll(); }
extern "C" int adapter_es1370_beep(uint32_t hz, uint32_t ms) { return es1370_beep(hz, ms); }

/* -------------------------------------------------------------
 * Wrappers: Intel HDA (plik C, ma juz powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_intel_hda_init(void)
{
    init_intel_hda();
    return 1;
}

extern "C" void adapter_intel_hda_shutdown(void)
{
    uint8_t val = adapter_inb(0x61U);
    adapter_outb(0x61U, (uint8_t)(val & ~3U));
}

extern "C" void adapter_intel_hda_poll(void)
{
    (void)0;
}

extern "C" int adapter_intel_hda_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    intel_hda_beep();
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: Intel Raptor Lake CAVS (posiada wlasne powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_raptorlake_cavs_init(void)   { return intel_raptorlake_cavs_init(); }
extern "C" void adapter_raptorlake_cavs_shutdown(void) { intel_raptorlake_cavs_shutdown(); }
extern "C" void adapter_raptorlake_cavs_poll(void)   { intel_raptorlake_cavs_poll(); }

extern "C" int adapter_raptorlake_cavs_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    IntelRaptorLakeCAVS* drv = IntelRaptorLakeCAVS::instance();
    if (!drv || !drv->is_initialized()) return -1;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 8000 : -8000;
    int rc = drv->pcm_open(2, 48000, 16, 1);
    if (rc != 0) return -1;
    drv->pcm_write(tone, sizeof(tone));
    adapter_delay_ms(ms ? ms : 200U);
    drv->pcm_close(1);
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: ALC650 (plik C++, powiazanie C przez adapter)
 * ------------------------------------------------------------- */

extern "C" int adapter_alc650_init(void)
{
    return alc650_init();
}

extern "C" void adapter_alc650_shutdown(void)
{
    alc650_shutdown();
}

extern "C" void adapter_alc650_poll(void)
{
    alc650_poll();
}

extern "C" int adapter_alc650_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 10000 : -10000;
    alc650_play_pcm(tone, sizeof(tone), 48000, 0);
    adapter_delay_ms(ms ? ms : 200U);
    alc650_stop();
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: VT1611 (klasa C++, skanowanie PCI + powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_vt1611_init(void)
{
    VT1611* drv = VT1611::instance();
    if (!drv) return 0;
    for (uint16_t bus = 0U; bus < 256U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = adapter_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                uint32_t bar0 = adapter_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;
                uint32_t cmd = adapter_pci_read32(bus, slot, func, 0x04U);
                adapter_pci_write32(bus, slot, func, 0x04U, cmd | 0x00000006U);
                drv->init();
                return 1;
            }
        }
    }
    return 0;
}

extern "C" void adapter_vt1611_shutdown(void)
{
    VT1611* drv = VT1611::instance();
    if (drv) drv->deinit();
}

extern "C" void adapter_vt1611_poll(void)
{
    VT1611* drv = VT1611::instance();
    if (drv) drv->irq_handler();
}

extern "C" int adapter_vt1611_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    VT1611* drv = VT1611::instance();
    if (!drv) return -1;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 9000 : -9000;
    int rc = drv->pcm_open(2, 48000, 16, 1);
    if (rc != 0) return -1;
    drv->pcm_write(tone, sizeof(tone));
    adapter_delay_ms(ms ? ms : 200U);
    drv->pcm_close(1);
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: HDAudioCodec (klasa C++, skanowanie PCI + powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_hda_codec_init(void)
{
    HDAudioCodec* drv = HDAudioCodec::instance();
    if (!drv) return 0;
    for (uint16_t bus = 0U; bus < 256U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = adapter_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                uint32_t bar0 = adapter_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;
                uint32_t cmd = adapter_pci_read32(bus, slot, func, 0x04U);
                adapter_pci_write32(bus, slot, func, 0x04U, cmd | 0x00000006U);
                drv->init();
                return 1;
            }
        }
    }
    return 0;
}

extern "C" void adapter_hda_codec_shutdown(void)
{
    HDAudioCodec* drv = HDAudioCodec::instance();
    if (drv) drv->deinit();
}

extern "C" void adapter_hda_codec_poll(void)
{
    HDAudioCodec* drv = HDAudioCodec::instance();
    if (drv) drv->irq_handler();
}

extern "C" int adapter_hda_codec_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    HDAudioCodec* drv = HDAudioCodec::instance();
    if (!drv) return -1;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 7000 : -7000;
    int rc = drv->pcm_open(2, 48000, 16, 1);
    if (rc != 0) return -1;
    drv->pcm_write(tone, sizeof(tone));
    adapter_delay_ms(ms ? ms : 200U);
    drv->pcm_close(1);
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: ALC880 (klasa C++, skanowanie PCI + powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_alc880_init(void)
{
    ALC880* drv = ALC880::instance();
    if (!drv) return 0;
    for (uint16_t bus = 0U; bus < 256U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = adapter_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                uint32_t bar0 = adapter_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;
                uint32_t cmd = adapter_pci_read32(bus, slot, func, 0x04U);
                adapter_pci_write32(bus, slot, func, 0x04U, cmd | 0x00000006U);
                drv->init();
                return 1;
            }
        }
    }
    return 0;
}

extern "C" void adapter_alc880_shutdown(void)
{
    ALC880* drv = ALC880::instance();
    if (drv) drv->deinit();
}

extern "C" void adapter_alc880_poll(void)
{
    ALC880* drv = ALC880::instance();
    if (drv) drv->irq_handler();
}

extern "C" int adapter_alc880_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    ALC880* drv = ALC880::instance();
    if (!drv) return -1;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 11000 : -11000;
    int rc = drv->pcm_open(2, 48000, 16, 1);
    if (rc != 0) return -1;
    drv->pcm_write(tone, sizeof(tone));
    adapter_delay_ms(ms ? ms : 200U);
    drv->pcm_close(1);
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: ALC1150 (klasa C++, skanowanie PCI + powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_alc1150_init(void)
{
    ALC1150* drv = ALC1150::instance();
    if (!drv) return 0;
    for (uint16_t bus = 0U; bus < 256U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = adapter_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                uint32_t bar0 = adapter_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;
                uint32_t cmd = adapter_pci_read32(bus, slot, func, 0x04U);
                adapter_pci_write32(bus, slot, func, 0x04U, cmd | 0x00000006U);
                drv->init();
                return 1;
            }
        }
    }
    return 0;
}

extern "C" void adapter_alc1150_shutdown(void)
{
    ALC1150* drv = ALC1150::instance();
    if (drv) drv->deinit();
}

extern "C" void adapter_alc1150_poll(void)
{
    ALC1150* drv = ALC1150::instance();
    if (drv) drv->irq_handler();
}

extern "C" int adapter_alc1150_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    ALC1150* drv = ALC1150::instance();
    if (!drv) return -1;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 13000 : -13000;
    int rc = drv->pcm_open(2, 48000, 16, 1);
    if (rc != 0) return -1;
    drv->pcm_write(tone, sizeof(tone));
    adapter_delay_ms(ms ? ms : 200U);
    drv->pcm_close(1);
    return 0;
}

/* -------------------------------------------------------------
 * Wrappers: Audigy2 (klasa C++, skanowanie PCI + powiazanie C)
 * ------------------------------------------------------------- */

extern "C" int adapter_audigy2_init(void)
{
    Audigy2* drv = Audigy2::instance();
    if (!drv) return 0;
    for (uint16_t bus = 0U; bus < 256U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = adapter_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                uint32_t bar0 = adapter_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;
                uint32_t cmd = adapter_pci_read32(bus, slot, func, 0x04U);
                adapter_pci_write32(bus, slot, func, 0x04U, cmd | 0x00000006U);
                drv->init();
                return 1;
            }
        }
    }
    return 0;
}

extern "C" void adapter_audigy2_shutdown(void)
{
    Audigy2* drv = Audigy2::instance();
    if (drv) drv->deinit();
}

extern "C" void adapter_audigy2_poll(void)
{
    Audigy2* drv = Audigy2::instance();
    if (drv) drv->irq_handler();
}

extern "C" int adapter_audigy2_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    Audigy2* drv = Audigy2::instance();
    if (!drv) return -1;
    static int16_t tone[2048];
    for (int i = 0; i < 2048; i++) tone[i] = (i & 256) ? 14000 : -14000;
    int rc = drv->pcm_open(2, 44100, 16);
    if (rc != 0) return -1;
    drv->pcm_write(tone, sizeof(tone));
    adapter_delay_ms(ms ? ms : 200U);
    drv->pcm_close();
    return 0;
}
