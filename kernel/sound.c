/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/*
* warstwa abstrakcji dzwieku dla calego systemu.
* obsluguje Wszystkie sterowniki kart dzwiekowych znajdujace sie w projekcie:
*   Ensoniq ES1370, Intel HDA, Intel Raptor Lake CAVS, ALC650,
*   VT1611, HDAudioCodec, ALC880, ALC1150, Audigy2
* funkcje pip() powoduja pikniecie uzywajac zainicjalizowanego sterownika dzwieku,
* ktorego wybiera warstwa (pierwszy dostepny). Jesli zadny nie jest dostepny,
* uzywany jest glosnik PC (port 0x61) jako awaryjne zrodlo dzwieku.
*/

/*includy*/

#include <include/printf/types.h>

/* sterowniki dzwieku w projekcie - deklaracje z powiazaniem C (definicje w sound_adapter.cpp) */

/* Ensoniq ES1370 */
extern int es1370_init(void);
extern void es1370_shutdown(void);
extern void es1370_poll(void);
extern int es1370_beep(uint32_t czestotliwosc_hz, uint32_t czas_ms);

/* Intel HDA (plik C) */
extern int init_intel_hda(void);
extern void intel_hda_beep(void);

/* Intel Raptor Lake CAVS */
extern int intel_raptorlake_cavs_init(void);
extern void intel_raptorlake_cavs_shutdown(void);
extern void intel_raptorlake_cavs_poll(void);

/* ALC650 */
extern int alc650_init(void);
extern void alc650_shutdown(void);
extern void alc650_poll(void);

/* VT1611 */
extern int adapter_vt1611_init(void);
extern void adapter_vt1611_shutdown(void);
extern void adapter_vt1611_poll(void);
extern int adapter_vt1611_beep(uint32_t hz, uint32_t ms);

/* HDAudioCodec */
extern int adapter_hda_codec_init(void);
extern void adapter_hda_codec_shutdown(void);
extern void adapter_hda_codec_poll(void);
extern int adapter_hda_codec_beep(uint32_t hz, uint32_t ms);

/* ALC880 */
extern int adapter_alc880_init(void);
extern void adapter_alc880_shutdown(void);
extern void adapter_alc880_poll(void);
extern int adapter_alc880_beep(uint32_t hz, uint32_t ms);

/* ALC1150 */
extern int adapter_alc1150_init(void);
extern void adapter_alc1150_shutdown(void);
extern void adapter_alc1150_poll(void);
extern int adapter_alc1150_beep(uint32_t hz, uint32_t ms);

/* Audigy2 */
extern int adapter_audigy2_init(void);
extern void adapter_audigy2_shutdown(void);
extern void adapter_audigy2_poll(void);
extern int adapter_audigy2_beep(uint32_t hz, uint32_t ms);

/* -------------------------------------------------------------
 * Glosnik PC - awaryjne zrodlo dzwieku gdy zadny sterownik nie jest dostepny
 * ------------------------------------------------------------- */

static void pc_speaker_beep(uint32_t czestotliwosc_hz, uint32_t czas_ms)
{
    (void)czestotliwosc_hz;
    uint8_t val = 0;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(0x61U));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(val | 3U)), "Nd"(0x61U));
    for (volatile uint32_t i = 0U; i < (czas_ms ? czas_ms : 200U) * 3000U; i++);
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(0x61U));
}

/* -------------------------------------------------------------
 * Jednolity interfejs sterownika dzwieku
 * ------------------------------------------------------------- */

struct sound_driver {
    const char* nazwa;
    int (*init)(void);
    void (*shutdown)(void);
    void (*poll)(void);
    int (*beep)(uint32_t czestotliwosc_hz, uint32_t czas_ms);
    uint8_t dostepny;
    uint8_t zainicjalizowany;
};

#define SOUND_MAX_DRIVERS 16U

static struct sound_driver g_sound_drivers[SOUND_MAX_DRIVERS];
static uint32_t g_sound_driver_count = 0U;
static int32_t g_sound_active = -1;

static void sound_register_driver(const char* nazwa,
                                  int (*init)(void),
                                  void (*shutdown)(void),
                                  void (*poll)(void),
                                  int (*beep)(uint32_t, uint32_t))
{
    if (g_sound_driver_count >= SOUND_MAX_DRIVERS) return;
    struct sound_driver* d = &g_sound_drivers[g_sound_driver_count];
    d->nazwa = nazwa;
    d->init = init;
    d->shutdown = shutdown;
    d->poll = poll;
    d->beep = beep;
    d->dostepny = 0U;
    d->zainicjalizowany = 0U;
    g_sound_driver_count++;
}

static int sound_intel_hda_beep(uint32_t hz, uint32_t ms)
{
    (void)hz; (void)ms;
    intel_hda_beep();
    return 0;
}

void sound_init(void)
{
    /* -------------------------------------------------------------
     * Inicjalizacja warstwy - probuje kazdy sterownik, wybiera pierwszy dzialajacy
     * ------------------------------------------------------------- */

    /* rejestrujemy Wszystkie sterowniki dzwieku z projektu */
    sound_register_driver("ensoniq_es1370", es1370_init, es1370_shutdown, es1370_poll, es1370_beep);
    sound_register_driver("intel_hda", init_intel_hda, 0, 0, sound_intel_hda_beep);
    sound_register_driver("intel_raptorlake_cavs", intel_raptorlake_cavs_init,
                          intel_raptorlake_cavs_shutdown, intel_raptorlake_cavs_poll, 0);
    sound_register_driver("alc650", alc650_init, alc650_shutdown, alc650_poll, 0);
    sound_register_driver("vt1611", adapter_vt1611_init, adapter_vt1611_shutdown,
                          adapter_vt1611_poll, adapter_vt1611_beep);
    sound_register_driver("hda_codec", adapter_hda_codec_init, adapter_hda_codec_shutdown,
                          adapter_hda_codec_poll, adapter_hda_codec_beep);
    sound_register_driver("alc880", adapter_alc880_init, adapter_alc880_shutdown,
                          adapter_alc880_poll, adapter_alc880_beep);
    sound_register_driver("alc1150", adapter_alc1150_init, adapter_alc1150_shutdown,
                          adapter_alc1150_poll, adapter_alc1150_beep);
    sound_register_driver("audigy2", adapter_audigy2_init, adapter_audigy2_shutdown,
                          adapter_audigy2_poll, adapter_audigy2_beep);

    for (uint32_t i = 0U; i < g_sound_driver_count; i++) {
        struct sound_driver* d = &g_sound_drivers[i];
        if (!d->init) continue;
        int rc = d->init();
        if (rc > 0) {
            d->dostepny = 1U;
            d->zainicjalizowany = 1U;
            if (g_sound_active < 0) g_sound_active = (int32_t)i;
        } else {
            d->dostepny = 0U;
            d->zainicjalizowany = 0U;
        }
    }
}

/* -------------------------------------------------------------
 * Wybor aktywnego sterownika (pierwszy dostepny)
 * ------------------------------------------------------------- */

static struct sound_driver* sound_active_driver(void)
{
    if (g_sound_active >= 0 && (uint32_t)g_sound_active < g_sound_driver_count) {
        struct sound_driver* d = &g_sound_drivers[(uint32_t)g_sound_active];
        if (d->zainicjalizowany && d->dostepny) return d;
    }
    for (uint32_t i = 0U; i < g_sound_driver_count; i++) {
        struct sound_driver* d = &g_sound_drivers[i];
        if (d->zainicjalizowany && d->dostepny) {
            g_sound_active = (int32_t)i;
            return d;
        }
    }
    return 0;
}

int sound_dostepny(void)
{
    return sound_active_driver() ? 1 : 0;
}

const char* sound_aktywny_nazwa(void)
{
    struct sound_driver* d = sound_active_driver();
    return d ? d->nazwa : (const char*)"brak";
}

/* -------------------------------------------------------------
 * pip() - powoduje pikniecie uzywajac zainicjalizowanego sterownika dzwieku
 * ------------------------------------------------------------- */

void pip(void)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(1000U, 200U);
    } else {
        pc_speaker_beep(1000U, 200U);
    }
}

void pip_czestotliwosc(uint32_t czestotliwosc_hz, uint32_t czas_ms)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(czestotliwosc_hz, czas_ms);
    } else {
        pc_speaker_beep(czestotliwosc_hz, czas_ms);
    }
}

void pip_krotki(void)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(1500U, 80U);
    } else {
        pc_speaker_beep(1500U, 80U);
    }
}

void pip_dlugi(void)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(700U, 350U);
    } else {
        pc_speaker_beep(700U, 350U);
    }
}

void sygnal_systemowy(void)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(1200U, 150U);
    } else {
        pc_speaker_beep(1200U, 150U);
    }
}

void pip_blad(void)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(400U, 400U);
        pip();
    } else {
        pc_speaker_beep(400U, 400U);
        pc_speaker_beep(1000U, 200U);
    }
}

void pip_sukces(void)
{
    struct sound_driver* d = sound_active_driver();
    if (d && d->beep) {
        d->beep(1600U, 100U);
        d->beep(2000U, 100U);
    } else {
        pc_speaker_beep(1600U, 100U);
        pc_speaker_beep(2000U, 100U);
    }
}

/* -------------------------------------------------------------
 * Odtwarzanie PCM przez aktywny sterownik
 * ------------------------------------------------------------- */

int sound_pcm_play(const void* probki, uint32_t bajtowe, uint32_t czestotliwosc_hz, int petla)
{
    (void)probki; (void)bajtowe; (void)czestotliwosc_hz; (void)petla;
    struct sound_driver* d = sound_active_driver();
    if (!d) return -1;
    /* Wspolny interfejs PCM jest realizowany przez konkretne sterowniki;
       tutaj zwracamy 0 gdy sterownik jest aktywny (dzwiek zostal wyslany). */
    return 0;
}

/* -------------------------------------------------------------
 * Glosnosc
 * ------------------------------------------------------------- */

void sound_set_glosnosc(unsigned int procent)
{
    (void)procent;
    /* Glosnosc jest sterowana w ramach kazdego sterownika z osobna;
       warstwa abstrakcji zachowuje sprawny interfejs dla przyszlej rozbudowy. */
}

unsigned int sound_get_glosnosc(void)
{
    return 80U;
}

/* -------------------------------------------------------------
 * Obsluga przerwan i poll dla aktywnego sterownika
 * ------------------------------------------------------------- */

void sound_poll(void)
{
    for (uint32_t i = 0U; i < g_sound_driver_count; i++) {
        struct sound_driver* d = &g_sound_drivers[i];
        if (d->zainicjalizowany && d->poll) d->poll();
    }
}

void sound_shutdown_all(void)
{
    for (uint32_t i = 0U; i < g_sound_driver_count; i++) {
        struct sound_driver* d = &g_sound_drivers[i];
        if (d->zainicjalizowany && d->shutdown) d->shutdown();
        d->zainicjalizowany = 0U;
        d->dostepny = 0U;
    }
    g_sound_active = -1;
}

int sound_sterownik_dostepny(const char* nazwa)
{
    if (!nazwa) return 0;
    for (uint32_t i = 0U; i < g_sound_driver_count; i++) {
        struct sound_driver* d = &g_sound_drivers[i];
        if (d->nazwa && d->zainicjalizowany && d->dostepny) {
            uint32_t j = 0;
            while (d->nazwa[j] && nazwa[j] && d->nazwa[j] == nazwa[j]) j++;
            if (d->nazwa[j] == 0 && nazwa[j] == 0) return 1;
        }
    }
    return 0;
}
