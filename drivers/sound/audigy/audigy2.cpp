/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <drivers/sound/audigy/audigy2.h>

typedef unsigned int uintptr_t;

#define AUDIGY2_VENDOR_ID                           0x1102
#define AUDIGY2_DEVICE_ID                           0x0004

/* Bezpośrednie rejestry BAR0 */
#define REG_PTR                                     0x00
#define REG_DATA                                    0x04
#define REG_IPR                                     0x08
#define REG_INTE                                    0x0C
#define REG_WC                                      0x10
#define REG_HCFG                                    0x14
#define REG_A_IOCFG                                 0x18
#define REG_AC97DATA                                0x1C
#define REG_AC97ADDRESS                             0x1E
#define REG_A_PTR                                   0x20
#define REG_A_DATA                                  0x24

/* Flagi rejestru HCFG */
#define HCFG_AUDIOENABLE                            0x00000001
#define HCFG_LOCKSOUNDCACHE                         0x00000008
#define HCFG_LOCKTANKCACHE                          0x00000010
#define HCFG_MUTEBUTTONENABLE                       0x00000002
#define HCFG_JOYENABLE                              0x00000200
#define HCFG_AUTOMUTE                               0x00004000

/* Flagi rejestru A_IOCFG */
#define A_IOCFG_GPOUT0                              0x0040
#define A_IOCFG_GPOUT1                              0x0004

Audigy2* Audigy2::g_instance = 0;
static Audigy2 g_audigy2_fallback;

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t val;
    __asm__ volatile("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline uint32_t f2u(float v) { union { float f; uint32_t u; } u = {v}; return u.u; }

Audigy2::Audigy2()
    : io_base(0), pci_seg(0), pci_bus(0), pci_dev(0), pci_func(0),
      irq(0), initialized(0), voice_mask(0), eax_enabled(0), master_vol(100)
{
    for (int i = 0; i < 64; i++) {
        voices[i].voice_id = i;
        voices[i].active = 0;
        voices[i].pos_x = 0.0f;
        voices[i].pos_y = 0.0f;
        voices[i].pos_z = 0.0f;
        voices[i].velocity_x = 0.0f;
        voices[i].velocity_y = 0.0f;
        voices[i].velocity_z = 0.0f;
        voices[i].orientation_x = 0.0f;
        voices[i].orientation_y = 0.0f;
        voices[i].orientation_z = 0.0f;
        voices[i].gain = 1.0f;
        voices[i].min_distance = 1.0f;
        voices[i].max_distance = 100.0f;
        voices[i].buffer_addr = 0;
        voices[i].buffer_size = 0;
    }
    playback_buf.dma_addr = 0;
    playback_buf.size = 0;
    playback_buf.pos = 0;
    playback_buf.period = 0;
    playback_buf.channels = 2;
    playback_buf.rate = 48000;
    playback_buf.format = 16;
    capture_buf.dma_addr = 0;
    capture_buf.size = 0;
    capture_buf.pos = 0;
    capture_buf.period = 0;
    capture_buf.channels = 2;
    capture_buf.rate = 48000;
    capture_buf.format = 16;
    current_env.room = 0;
    current_env.room_hf = 0;
    current_env.reflections = 0;
    current_env.reflections_delay = 0;
    current_env.reverb = 0;
    current_env.reverb_delay = 0;
    current_env.decay_time = 0;
    current_env.decay_hf_ratio = 0;
    current_env.reflections_hf = 0;
    current_env.reverb_hf = 0;
    current_env.diffusion = 0;
    current_env.density = 0;
    current_env.flags = 0;
}

Audigy2::~Audigy2()
{
}

Audigy2* Audigy2::instance()
{
    if (!g_instance) {
        g_instance = &g_audigy2_fallback;
    }
    return g_instance;
}

uint32_t Audigy2::pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFCU);
    outl(0xCF8, address);
    return inl(0xCFC);
}

void Audigy2::pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFCU);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

uint8_t Audigy2::probe(uint16_t vendor, uint16_t device)
{
    if (vendor == AUDIGY2_VENDOR_ID && device == AUDIGY2_DEVICE_ID) {
        return 1;
    }
    return 0;
}

void Audigy2::write_direct32(uint32_t offset, uint32_t value)
{
    if (io_base == 0) return;
    outl((uint16_t)(io_base + offset), value);
}

uint32_t Audigy2::read_direct32(uint32_t offset)
{
    if (io_base == 0) return 0;
    return inl((uint16_t)(io_base + offset));
}

void Audigy2::write_direct16(uint32_t offset, uint16_t value)
{
    if (io_base == 0) return;
    outw((uint16_t)(io_base + offset), value);
}

uint16_t Audigy2::read_direct16(uint32_t offset)
{
    if (io_base == 0) return 0;
    return inw((uint16_t)(io_base + offset));
}

void Audigy2::write_direct8(uint32_t offset, uint8_t value)
{
    if (io_base == 0) return;
    outb((uint16_t)(io_base + offset), value);
}

uint8_t Audigy2::read_direct8(uint32_t offset)
{
    if (io_base == 0) return 0;
    return inb((uint16_t)(io_base + offset));
}

void Audigy2::write_reg(uint32_t reg, uint32_t ch, uint32_t value)
{
    if (io_base == 0) return;
    uint32_t ptr_val = ((reg & 0x0FFF) << 16) | (ch & 0x003F);
    outl((uint16_t)(io_base + REG_PTR), ptr_val);
    outl((uint16_t)(io_base + REG_DATA), value);
}

uint32_t Audigy2::read_reg(uint32_t reg, uint32_t ch)
{
    if (io_base == 0) return 0;
    uint32_t ptr_val = ((reg & 0x0FFF) << 16) | (ch & 0x003F);
    outl((uint16_t)(io_base + REG_PTR), ptr_val);
    return inl((uint16_t)(io_base + REG_DATA));
}

void Audigy2::write_dsp(uint32_t addr, uint32_t value)
{
    write_reg(addr, 0, value);
}

uint32_t Audigy2::read_dsp(uint32_t addr)
{
    return read_reg(addr, 0);
}

void Audigy2::ac97_write(uint8_t reg, uint16_t val)
{
    if (io_base == 0) return;
    outb((uint16_t)(io_base + REG_AC97ADDRESS), reg & 0x7F);
    outw((uint16_t)(io_base + REG_AC97DATA), val);
}

uint16_t Audigy2::ac97_read(uint8_t reg)
{
    if (io_base == 0) return 0;
    outb((uint16_t)(io_base + REG_AC97ADDRESS), reg | 0x80);
    return inw((uint16_t)(io_base + REG_AC97DATA));
}

void Audigy2::init(void)
{
    if (initialized) return;

    g_instance = this;

    /* Skanowanie magistrali PCI w celu znalezienia karty Audigy 2 */
    uint32_t found_bar0 = 0;
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id = pci_read32((uint8_t)bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (probe(vendor, device)) {
                    uint32_t bar0 = pci_read32((uint8_t)bus, slot, func, 0x10);
                    if (bar0 && bar0 != 0xFFFFFFFF && (bar0 & 1)) {
                        found_bar0 = bar0 & ~0x3;
                        pci_seg = 0;
                        pci_bus = bus;
                        pci_dev = slot;
                        pci_func = func;
                        /* Włączenie obsługi Bus Master oraz przestrzeni I/O */
                        uint32_t cmd = pci_read32((uint8_t)bus, slot, func, 0x04);
                        pci_write32((uint8_t)bus, slot, func, 0x04, cmd | 0x00000006);
                        break;
                    }
                }
            }
            if (found_bar0) break;
        }
        if (found_bar0) break;
    }

    if (found_bar0 == 0) {
        /* Awaryjny domyślny adres bazowy I/O jeśli nie znaleziono przez skanowanie PCI */
        found_bar0 = 0xB000;
    }

    io_base = found_bar0;

    /* 1. Konfiguracja resetu i wyciszenia */
    write_direct32(REG_INTE, 0x00000000); /* Wyłączenie przerwań */
    write_direct32(REG_IPR, 0xFFFFFFFF);  /* Wyczyszczenie zaległych przerwań */

    /* 2. Konfiguracja sprzętowa: zablokowanie pamięci podręcznej cache, włączenie przycisku wyciszenia */
    write_direct32(REG_HCFG, HCFG_LOCKSOUNDCACHE | HCFG_LOCKTANKCACHE | HCFG_MUTEBUTTONENABLE);

    /* 3. Zatrzymanie i wyczyszczenie wszystkich 64 głosów */
    for (uint32_t ch = 0; ch < 64; ch++) {
        write_reg(0x15, ch, 0x00000000); /* DCYSUSV */
        write_reg(0x03, ch, 0x00000000); /* VTFT */
        write_reg(0x02, ch, 0x00000000); /* CVCF */
        write_reg(0x01, ch, 0x00000000); /* PTRX */
        write_reg(0x00, ch, 0x00000000); /* CPF */
    }

    /* 4. Inicjalizacja wirtualnej tablicy stron DMA (PTB pod rejestrem 0x40) */
    static uint32_t ptb_dummy[4096];
    uint32_t ptb_phys = (uint32_t)(uintptr_t)ptb_dummy;
    for (int i = 0; i < 4096; i++) {
        ptb_dummy[i] = (ptb_phys & 0xFFFFF000) << 1; /* Wskaźnik strony ciszy */
    }
    write_reg(0x40, 0, ptb_phys);
    write_reg(0x41, 0, 0); /* TCB */
    write_reg(0x42, 0, 0); /* TCBS */

    /* 5. Załadowanie mikrokodu DSP FX8010 (minimalny mikser typu pass-through) */
    write_reg(0x53, 0, 0x00020000); /* Zatrzymanie DSP (DBG_SINGLE_STEP / A_DBG) */
    for (uint32_t i = 0; i < 1024; i++) {
        /* Instrukcja NOP: ACC3(0x0C0, 0x0C0, 0x0C0, 0x0C0) [op=0x06] */
        uint32_t lo = ((0x0C0 & 0x7FF) << 12) | (0x0C0 & 0x7FF);
        uint32_t hi = ((0x06 & 0x0F) << 24) | ((0x0C0 & 0x7FF) << 12) | (0x0C0 & 0x7FF);
        write_reg(0x600 + (i * 2), 0, lo);
        write_reg(0x601 + (i * 2), 0, hi);
    }
    /* Ustawienie głośności głównej GPR na maksimum (0x7FFFFFFF) */
    write_reg(0x400, 0, 0x7FFFFFFF);
    write_reg(0x401, 0, 0x7FFFFFFF);
    /* Przepuszczanie sygnału Front L/R oraz S/PDIF */
    uint32_t lo_0 = ((0x000 & 0x7FF) << 12) | (0x400 & 0x7FF);
    uint32_t hi_0 = ((0x00 & 0x0F) << 24) | ((0x060 & 0x7FF) << 12) | (0x0C0 & 0x7FF);
    write_reg(0x600, 0, lo_0);
    write_reg(0x601, 0, hi_0);
    uint32_t lo_1 = ((0x001 & 0x7FF) << 12) | (0x401 & 0x7FF);
    uint32_t hi_1 = ((0x00 & 0x0F) << 24) | ((0x061 & 0x7FF) << 12) | (0x0C0 & 0x7FF);
    write_reg(0x602, 0, lo_1);
    write_reg(0x603, 0, hi_1);

    write_reg(0x53, 0, 0x00000000); /* Wznowienie pracy DSP */

    /* 6. Konfiguracja kodeka AC'97 */
    ac97_write(0x00, 0x0000); /* Reset AC'97 */
    ac97_write(0x02, 0x0000); /* Głośność główna 0 dB */
    ac97_write(0x18, 0x0808); /* Głośność wyjściowa PCM */

    /* 7. Konfiguracja GPIO i odblokowanie wyjścia analogowego DAC CS4382 */
    uint32_t iocfg = read_direct32(REG_A_IOCFG);
    iocfg &= ~A_IOCFG_GPOUT1; /* Wyczyszczenie bitu tylko-wyjście-cyfrowe */
    iocfg |= A_IOCFG_GPOUT0;  /* Włączenie przekaźnika wyjścia analogowego */
    write_direct32(REG_A_IOCFG, iocfg);

    /* 8. Włączenie audio w rejestrze HCFG */
    write_direct32(REG_HCFG, HCFG_AUDIOENABLE | HCFG_JOYENABLE | HCFG_AUTOMUTE);

    /* 9. Włączenie przerwań (Interval Timer) */
    write_direct32(REG_INTE, 0x00000100);

    mixer_init();
    spdif_init();
    efx_init();
    dma_alloc(&playback_buf, 256 * 1024);
    dma_alloc(&capture_buf, 256 * 1024);

    initialized = 1;
}

void Audigy2::deinit(void)
{
    if (!initialized) return;
    dma_stop_playback(&playback_buf);
    dma_free(&playback_buf);
    dma_free(&capture_buf);
    write_direct32(REG_INTE, 0x00000000);
    initialized = 0;
}

void Audigy2::irq_handler(void)
{
    if (!initialized) return;
    uint32_t status = read_direct32(REG_IPR);
    if (status != 0) {
        write_direct32(REG_IPR, status); /* Write-1-to-Clear */
    }
}

void Audigy2::mixer_init(void)
{
    set_master_volume(master_vol);
}

void Audigy2::spdif_init(void)
{
    write_dsp(0x42, 0x00000001);
}

void Audigy2::efx_init(void)
{
    write_reg(0x60, 0, eax_enabled ? 1 : 0);
}

void Audigy2::spatial_mix_update(void)
{
    for (int i = 0; i < 64; i++) {
        if (!(voice_mask & (1u << i))) continue;
        Voice3D* v = &voices[i];
        float dist = v->pos_x * v->pos_x + v->pos_y * v->pos_y + v->pos_z * v->pos_z;
        float att = 1.0f;
        if (dist > v->min_distance * v->min_distance) {
            att = v->min_distance / (v->min_distance + (dist - v->min_distance * v->min_distance) / (v->max_distance - v->min_distance));
            if (att < 0.0f) att = 0.0f;
            if (att > 1.0f) att = 1.0f;
        }
        uint32_t vol = (uint32_t)(v->gain * att * 255.0f);
        write_dsp(0x100 + i * 4, vol);
    }
}

void Audigy2::dma_alloc(PcmBuffer* buf, uint32_t size)
{
    buf->size = size;
    buf->pos = 0;
    buf->period = size / 4;
    static uint32_t dummy_dma_mem[65536];
    buf->dma_addr = dummy_dma_mem;
}

void Audigy2::dma_free(PcmBuffer* buf)
{
    buf->dma_addr = 0;
    buf->size = 0;
    buf->pos = 0;
    buf->period = 0;
}

void Audigy2::dma_start_playback(PcmBuffer* buf)
{
    if (!buf->dma_addr) return;
    /* Konfiguracja głosów 0 i 1 do odtwarzania PCM stereo */
    uint32_t phys = (uint32_t)(uintptr_t)buf->dma_addr;
    uint32_t samples = buf->size / (buf->channels * (buf->format / 8));

    /* Programowanie głosu 0 (Lewy / Mono) */
    write_reg(0x06, 0, 0);                 /* Początek pętli (PSST) */
    write_reg(0x07, 0, samples);           /* Koniec pętli (DSL) */
    write_reg(0x08, 0, phys >> 1);         /* Bieżący adres (CCCA) */
    write_reg(0x00, 0, 0x10000000);        /* Krok częstotliwości (CPF) */
    write_reg(0x01, 0, 0x1000FFFF);        /* Docelowy krok oraz wysyłka FX A/B (PTRX) */
    write_reg(0x02, 0, 0xFFFFFFFF);        /* Głośność i filtr (CVCF) */
    write_reg(0x03, 0, 0xFFFFFFFF);        /* Docelowa głośność i filtr (VTFT) */
    write_reg(0x13, 0, 0x00007F00);        /* DCYSUSM */
    write_reg(0x15, 0, 0x00007F00);        /* DCYSUSV */
    write_reg(0x0B, 0, 0x03020100);        /* Routing FXRT1 */
}

void Audigy2::dma_stop_playback(PcmBuffer* buf)
{
    (void)buf;
    write_reg(0x15, 0, 0x00000000); /* Zatrzymanie obwiedni głośności */
    write_reg(0x02, 0, 0x00000000);
}

int Audigy2::pcm_open(uint16_t channels, uint32_t rate, uint16_t format)
{
    if (!initialized) return -1;
    playback_buf.channels = channels;
    playback_buf.rate = rate;
    playback_buf.format = format;
    dma_start_playback(&playback_buf);
    return 0;
}

int Audigy2::pcm_write(const void* data, uint32_t bytes)
{
    if (!initialized || !playback_buf.dma_addr) return -1;
    uint32_t copy = bytes;
    if (playback_buf.pos + copy > playback_buf.size) copy = playback_buf.size - playback_buf.pos;
    for (uint32_t i = 0; i < copy; i++) {
        ((uint8_t*)playback_buf.dma_addr)[playback_buf.pos + i] = ((const uint8_t*)data)[i];
    }
    playback_buf.pos += copy;
    if (playback_buf.pos >= playback_buf.size) playback_buf.pos = 0;
    return (int)copy;
}

int Audigy2::pcm_read(void* data, uint32_t bytes)
{
    if (!initialized || !capture_buf.dma_addr) return -1;
    uint32_t copy = bytes;
    if (capture_buf.pos + copy > capture_buf.size) copy = capture_buf.size - capture_buf.pos;
    for (uint32_t i = 0; i < copy; i++) {
        ((uint8_t*)data)[i] = ((uint8_t*)capture_buf.dma_addr)[capture_buf.pos + i];
    }
    capture_buf.pos += copy;
    if (capture_buf.pos >= capture_buf.size) capture_buf.pos = 0;
    return (int)copy;
}

void Audigy2::pcm_close(void)
{
    dma_stop_playback(&playback_buf);
}

int Audigy2::voice3d_alloc(Voice3D* voice)
{
    for (int i = 0; i < 64; i++) {
        if (!(voice_mask & (1u << i))) {
            voice_mask |= (1u << i);
            voice->voice_id = i;
            voice->active = 1;
            voice->pos_x = 0.0f;
            voice->pos_y = 0.0f;
            voice->pos_z = 0.0f;
            voice->gain = 1.0f;
            voices[i] = *voice;
            spatial_mix_update();
            return i;
        }
    }
    return -1;
}

void Audigy2::voice3d_free(Voice3D* voice)
{
    voice_mask &= ~(1u << voice->voice_id);
    voice->active = 0;
    voices[voice->voice_id] = *voice;
    spatial_mix_update();
}

void Audigy2::voice3d_set_position(Voice3D* voice, float x, float y, float z)
{
    voice->pos_x = x;
    voice->pos_y = y;
    voice->pos_z = z;
    voices[voice->voice_id] = *voice;
    spatial_mix_update();
}

void Audigy2::voice3d_set_velocity(Voice3D* voice, float x, float y, float z)
{
    voice->velocity_x = x;
    voice->velocity_y = y;
    voice->velocity_z = z;
    voices[voice->voice_id] = *voice;
}

void Audigy2::voice3d_set_gain(Voice3D* voice, float gain)
{
    voice->gain = gain;
    voices[voice->voice_id] = *voice;
    spatial_mix_update();
}

void Audigy2::voice3d_play(Voice3D* voice, const void* buffer, uint32_t size)
{
    voice->buffer_addr = (uint32_t)(uintptr_t)buffer;
    voice->buffer_size = size;
    voice->active = 1;
    voices[voice->voice_id] = *voice;

    uint32_t ch = voice->voice_id;
    uint32_t samples = size / 2;
    write_reg(0x06, ch, 0);
    write_reg(0x07, ch, samples);
    write_reg(0x08, ch, voice->buffer_addr >> 1);
    write_reg(0x00, ch, 0x10000000);
    write_reg(0x01, ch, 0x1000FFFF);
    write_reg(0x02, ch, 0xFFFFFFFF);
    write_reg(0x03, ch, 0xFFFFFFFF);
    write_reg(0x13, ch, 0x00007F00);
    write_reg(0x15, ch, 0x00007F00);
    spatial_mix_update();
}

void Audigy2::voice3d_stop(Voice3D* voice)
{
    voice->active = 0;
    voices[voice->voice_id] = *voice;
    uint32_t ch = voice->voice_id;
    write_reg(0x15, ch, 0x00000000);
    write_reg(0x02, ch, 0x00000000);
}

int Audigy2::eax_set_environment(const EaxEnvironment* env)
{
    if (!eax_enabled) return -1;
    current_env = *env;
    write_dsp(0x300, env->room);
    write_dsp(0x304, env->room_hf);
    write_dsp(0x308, env->reflections);
    write_dsp(0x30C, env->reflections_delay);
    write_dsp(0x310, env->reverb);
    write_dsp(0x314, env->reverb_delay);
    write_dsp(0x318, env->decay_time);
    write_dsp(0x31C, env->decay_hf_ratio);
    write_dsp(0x320, env->reflections_hf);
    write_dsp(0x324, env->reverb_hf);
    write_dsp(0x328, env->diffusion);
    write_dsp(0x32C, env->density);
    write_dsp(0x330, env->flags);
    return 0;
}

int Audigy2::eax_enable(uint8_t enable)
{
    eax_enabled = enable ? 1 : 0;
    write_reg(0x60, 0, eax_enabled);
    return 0;
}

int Audigy2::eax_set_listener(float x, float y, float z,
                              float fx, float fy, float fz,
                              float ux, float uy, float uz)
{
    write_dsp(0x340, f2u(x));
    write_dsp(0x344, f2u(y));
    write_dsp(0x348, f2u(z));
    write_dsp(0x34C, f2u(fx));
    write_dsp(0x350, f2u(fy));
    write_dsp(0x354, f2u(fz));
    write_dsp(0x358, f2u(ux));
    write_dsp(0x35C, f2u(uy));
    write_dsp(0x360, f2u(uz));
    return 0;
}

void Audigy2::set_master_volume(uint8_t percent)
{
    master_vol = percent > 100 ? 100 : percent;
    if (initialized) {
        uint16_t ac97_val = (uint16_t)((100 - master_vol) * 31 / 100);
        ac97_write(0x02, (ac97_val << 8) | ac97_val);
    }
}

uint8_t Audigy2::get_master_volume(void)
{
    return master_vol;
}

extern "C" void audigy2_irq(void)
{
    if (Audigy2::g_instance) {
        Audigy2::g_instance->irq_handler();
    }
}
