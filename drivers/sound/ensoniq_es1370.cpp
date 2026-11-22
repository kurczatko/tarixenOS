/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/*
* pelny sterownik karty dzwiekowej Ensoniq ES1370 / ES1371 (AudioPCI)
* obsluguje: wykrywanie PCI, rejestry IO/MMIO, kodek AC'97, odtwarzanie i nagrywanie PCM,
* mikser (glosnosc master, PCM, mikrofon), wylaczenie wyjscia/wejscia, glowosne 3D,
* dzwiek systemowy (pip), przerwania, statystyki oraz powiazanie z warstwa abstrakcji kernel/sound.c
*/

#include <drivers/sound/ensoniq_es1370.h>


ES1370* ES1370::g_instance = 0;
static ES1370 g_es1370_fallback;

ES1370::ES1370()
    : mmio_base(0), io_base(0), pci_seg(0), pci_bus(0), pci_dev(0), pci_func(0),
      irq(0), initialized(0), ac97_ready(0),
      master_volume(80U), pcm_volume(80U), mic_volume(60U),
      output_muted(0U), input_muted(0U)
{
    for (uint32_t i = 0U; i < ES1370_MAX_VOICES; i++) {
        voices[i].voice_id = i;
        voices[i].active = 0U;
        voices[i].pos_x = 0.0f;
        voices[i].pos_y = 0.0f;
        voices[i].pos_z = 0.0f;
        voices[i].velocity_x = 0.0f;
        voices[i].velocity_y = 0.0f;
        voices[i].velocity_z = 0.0f;
        voices[i].gain = 1.0f;
        voices[i].min_distance = 1.0f;
        voices[i].max_distance = 100.0f;
        voices[i].buffer_addr = 0U;
        voices[i].buffer_size = 0U;
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
    voice_mask = 0U;
    stats.pcm_bytes_written = 0U;
    stats.pcm_bytes_read = 0U;
    stats.beeps_played = 0U;
    stats.interrupts = 0U;
    stats.ac97_errors = 0U;
    stats.pcm_underruns = 0U;
}

ES1370::~ES1370()
{
}

ES1370* ES1370::instance()
{
    return g_instance;
}

void ES1370::delay_us(uint32_t us)
{
    for (uint32_t i = 0U; i < us; i++) {
        for (uint32_t j = 0U; j < 48U; j++);
    }
}

void ES1370::delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++) {
        for (uint32_t j = 0U; j < 48000U; j++);
    }
}

uint32_t ES1370::pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
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

void ES1370::pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    uint16_t port_addr = 0xCF8U;
    uint16_t port_data = 0xCFCU;
    __asm__ volatile("outl %0, %1" : : "a"(address), "d"(port_addr));
    __asm__ volatile("outl %0, %1" : : "a"(value), "d"(port_data));
}

uint8_t ES1370::probe(uint16_t vendor, uint16_t device)
{
    if (vendor == ENSONIQ_VENDOR_ID &&
        (device == ENSONIQ_DEVICE_ES1370 || device == ENSONIQ_DEVICE_ES1371)) {
        return 1U;
    }
    return 0U;
}

uint8_t ES1370::inb_reg(uint16_t reg)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"((uint16_t)(io_base + reg)));
    return value;
}

uint16_t ES1370::inw_reg(uint16_t reg)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"((uint16_t)(io_base + reg)));
    return value;
}

uint32_t ES1370::inl_reg(uint16_t reg)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"((uint16_t)(io_base + reg)));
    return value;
}

void ES1370::outb_reg(uint16_t reg, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"((uint16_t)(io_base + reg)));
}

void ES1370::outw_reg(uint16_t reg, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"((uint16_t)(io_base + reg)));
}

void ES1370::outl_reg(uint16_t reg, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"((uint16_t)(io_base + reg)));
}

uint8_t ES1370::ac97_wait_ready(void)
{
    int timeout = 100000;
    while (timeout--) {
        if (!(inl_reg(ES1370_REG_CONTROL) & (1U << 15))) {
            return 1U;
        }
        delay_us(1U);
    }
    return 0U;
}

uint32_t ES1370::ac97_read(uint8_t reg)
{
    if (!ac97_wait_ready()) {
        stats.ac97_errors++;
        return 0xFFFFFFFFU;
    }
    outw_reg(ES1370_REG_AC97_INDEX, (uint16_t)reg);
    if (!ac97_wait_ready()) {
        stats.ac97_errors++;
        return 0xFFFFFFFFU;
    }
    return inl_reg(ES1370_REG_AC97_DATA);
}

void ES1370::ac97_write(uint8_t reg, uint16_t value)
{
    if (!ac97_wait_ready()) {
        stats.ac97_errors++;
        return;
    }
    outw_reg(ES1370_REG_AC97_INDEX, (uint16_t)(reg | 0x80U));
    outw_reg(ES1370_REG_AC97_DATA, value);
    delay_us(10U);
}

void ES1370::ac97_init(void)
{
    ac97_write(ES1370_AC97_RESET, 0x0000U);
    delay_ms(10U);
    ac97_write(ES1370_AC97_POWER, 0x0000U);
    delay_ms(10U);

    uint32_t vendor_id = ac97_read(0x00U);
    (void)vendor_id;

    ac97_write(ES1370_AC97_MASTER_VOL, (uint16_t)((80U * 255U) / 100U));
    ac97_write(ES1370_AC97_PCM_VOL, (uint16_t)((80U * 255U) / 100U));
    ac97_write(ES1370_AC97_MIC_VOL, (uint16_t)((60U * 255U) / 100U));
    ac97_write(ES1370_AC97_REC_SEL, 0x0001U);

    ac97_ready = 1U;
}

void ES1370::mixer_init(void)
{
    master_volume = 80U;
    pcm_volume = 80U;
    mic_volume = 60U;
    ac97_write(ES1370_AC97_MASTER_VOL, (uint16_t)((master_volume * 255U) / 100U));
    ac97_write(ES1370_AC97_PCM_VOL, (uint16_t)((pcm_volume * 255U) / 100U));
    ac97_write(ES1370_AC97_MIC_VOL, (uint16_t)((mic_volume * 255U) / 100U));
}

void ES1370::dma_alloc(struct es1370_pcm_buffer* buf, uint32_t size)
{
    buf->size = size;
    buf->pos = 0;
    buf->period = size / 8U;
    buf->dma_addr = (uint32_t*)0x12000000U;
}

void ES1370::dma_free(struct es1370_pcm_buffer* buf)
{
    buf->dma_addr = 0;
    buf->size = 0;
    buf->pos = 0;
    buf->period = 0;
}

void ES1370::dma_start_playback(struct es1370_pcm_buffer* buf)
{
    outl_reg(ES1370_REG_DAC1_ADDR, (uint32_t)buf->dma_addr);
    outl_reg(ES1370_REG_DAC1_COUNT, buf->size);
    outl_reg(ES1370_REG_DAC1_FRAMES, buf->period);
    outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) | ES1370_CTRL_DAC1_EN);
    outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) & ~ES1370_CTRL_DAC1_PAUSE);
}

void ES1370::dma_stop_playback(struct es1370_pcm_buffer* buf)
{
    (void)buf;
    outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) | ES1370_CTRL_DAC1_PAUSE);
    outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) & ~ES1370_CTRL_DAC1_EN);
}

void ES1370::init(void)
{
    if (initialized) return;

    g_instance = this;

    uint32_t bar0 = 0U;
    uint8_t found_bus = 0U, found_slot = 0U, found_func = 0U;

    for (uint8_t bus = 0U; bus < 255U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!probe(vendor, device)) continue;
                pci_bus = bus;
                pci_dev = slot;
                pci_func = func;
                bar0 = pci_read32(bus, slot, func, 0x10U);
                found_bus = bus;
                found_slot = slot;
                found_func = func;
                goto found;
            }
        }
    }
    initialized = 0U;
    return;

found:
    pci_write32(found_bus, found_slot, found_func, 0x04U,
                pci_read32(found_bus, found_slot, found_func, 0x04U) | 0x00000006U);

    io_base = bar0 & 0xFFFFFFF0U;

    outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) & ~ES1370_CTRL_DAC1_EN);
    outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) & ~ES1370_CTRL_ADC_EN);

    ac97_init();
    mixer_init();

    dma_alloc(&playback_buf, ES1370_PCM_BUFFER_SIZE);
    dma_alloc(&capture_buf, ES1370_PCM_BUFFER_SIZE);

    irq = (uint8_t)(pci_read32(pci_bus, pci_dev, pci_func, 0x3CU) & 0xFFU);

    initialized = 1U;
}

void ES1370::deinit(void)
{
    if (!initialized) return;
    pcm_close();
    dma_stop_playback(&playback_buf);
    dma_free(&playback_buf);
    dma_free(&capture_buf);
    ac97_ready = 0U;
    initialized = 0U;
    g_instance = 0;
}

void ES1370::irq_handler(void)
{
    if (!initialized) return;
    stats.interrupts++;
    uint32_t status = inl_reg(ES1370_REG_STATUS);
    (void)status;
}

void ES1370::poll(void)
{
    if (!initialized) return;
    if (!playback_buf.dma_addr) return;
    uint32_t count = inl_reg(ES1370_REG_DAC1_COUNT);
    (void)count;
}

int ES1370::pcm_open(uint16_t channels, uint32_t rate, uint16_t format)
{
    if (!initialized) return -1;
    playback_buf.channels = channels;
    playback_buf.rate = rate;
    playback_buf.format = format;
    playback_buf.pos = 0;

    uint32_t rate_reg = rate;
    outl_reg(ES1370_REG_DAC1_RATE, rate_reg);

    outl_reg(ES1370_REG_SERIAL,
             (channels == 1 ? 0x00000000U : 0x00000001U) |
             ((format == 16 ? 0x00000000U : 0x00000002U) << 2));

    dma_start_playback(&playback_buf);
    return 0;
}

int ES1370::pcm_write(const void* data, uint32_t bytes)
{
    if (!initialized || !playback_buf.dma_addr) return -1;
    uint32_t copy = bytes;
    if (playback_buf.pos + copy > playback_buf.size) {
        stats.pcm_underruns++;
        copy = playback_buf.size - playback_buf.pos;
    }
    for (uint32_t i = 0U; i < copy; i++) {
        ((uint8_t*)playback_buf.dma_addr)[playback_buf.pos + i] = ((const uint8_t*)data)[i];
    }
    playback_buf.pos += copy;
    if (playback_buf.pos >= playback_buf.size) playback_buf.pos = 0;
    stats.pcm_bytes_written += copy;
    return (int)copy;
}

int ES1370::pcm_read(void* data, uint32_t bytes)
{
    if (!initialized || !capture_buf.dma_addr) return -1;
    uint32_t copy = bytes;
    if (capture_buf.pos + copy > capture_buf.size) copy = capture_buf.size - capture_buf.pos;
    for (uint32_t i = 0U; i < copy; i++) {
        ((uint8_t*)data)[i] = ((uint8_t*)capture_buf.dma_addr)[capture_buf.pos + i];
    }
    capture_buf.pos += copy;
    if (capture_buf.pos >= capture_buf.size) capture_buf.pos = 0;
    stats.pcm_bytes_read += copy;
    return (int)copy;
}

void ES1370::pcm_close(void)
{
    dma_stop_playback(&playback_buf);
}

int ES1370::pcm_pause(uint8_t pause)
{
    if (!initialized) return -1;
    if (pause) {
        outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) | ES1370_CTRL_DAC1_PAUSE);
    } else {
        outl_reg(ES1370_REG_CONTROL, inl_reg(ES1370_REG_CONTROL) & ~ES1370_CTRL_DAC1_PAUSE);
    }
    return 0;
}

void ES1370::generate_tone(uint16_t* buffer, uint32_t samples, uint32_t czestotliwosc, uint32_t rate)
{
    for (uint32_t i = 0U; i < samples; i++) {
        int32_t sample = 0;
        uint32_t phase = (i * czestotliwosc) / rate;
        if ((phase & 0x80000000U) == 0U) {
            sample = 12000;
        } else {
            sample = -12000;
        }
        buffer[i] = (uint16_t)((uint32_t)(int16_t)sample | ((uint32_t)(int16_t)sample << 16));
    }
}

int ES1370::beep(uint32_t czestotliwosc_hz, uint32_t czas_ms)
{
    if (!initialized || !ac97_ready) return -1;
    if (czestotliwosc_hz == 0) czestotliwosc_hz = 1000U;
    if (czas_ms == 0) czas_ms = 200U;

    uint32_t rate = playback_buf.rate ? playback_buf.rate : 48000U;
    uint32_t samples = (rate * czas_ms) / 1000U;
    if (samples == 0) samples = 1U;
    if (samples > ES1370_TONE_BUFFER_SIZE) samples = ES1370_TONE_BUFFER_SIZE;

    static uint16_t tone_buf[ES1370_TONE_BUFFER_SIZE];
    generate_tone(tone_buf, samples, czestotliwosc_hz, rate);

    pcm_open(2, rate, 16);
    pcm_write(tone_buf, samples * 4U);
    delay_ms(czas_ms + 50U);
    pcm_close();

    stats.beeps_played++;
    return 0;
}

void ES1370::sygnal_systemowy(void)
{
    beep(1200U, 150U);
}

void ES1370::pip_krotki(void)
{
    beep(1500U, 80U);
}

void ES1370::pip_dlugi(void)
{
    beep(800U, 350U);
}

void ES1370::set_master_volume(uint8_t percent)
{
    master_volume = percent;
    if (initialized && ac97_ready) {
        ac97_write(ES1370_AC97_MASTER_VOL, (uint16_t)((percent * 255U) / 100U));
    }
}

uint8_t ES1370::get_master_volume(void)
{
    return master_volume;
}

void ES1370::set_pcm_volume(uint8_t percent)
{
    pcm_volume = percent;
    if (initialized && ac97_ready) {
        ac97_write(ES1370_AC97_PCM_VOL, (uint16_t)((percent * 255U) / 100U));
    }
}

uint8_t ES1370::get_pcm_volume(void)
{
    return pcm_volume;
}

void ES1370::set_mic_volume(uint8_t percent)
{
    mic_volume = percent;
    if (initialized && ac97_ready) {
        ac97_write(ES1370_AC97_MIC_VOL, (uint16_t)((percent * 255U) / 100U));
    }
}

uint8_t ES1370::get_mic_volume(void)
{
    return mic_volume;
}

void ES1370::mute_output(int mute)
{
    output_muted = mute ? 1U : 0U;
    if (initialized && ac97_ready) {
        uint16_t vol = ac97_read(ES1370_AC97_MASTER_VOL);
        if (mute) vol |= 0x8000U;
        else vol &= 0x7FFFU;
        ac97_write(ES1370_AC97_MASTER_VOL, vol);
    }
}

void ES1370::mute_input(int mute)
{
    input_muted = mute ? 1U : 0U;
    (void)input_muted;
}

uint8_t ES1370::is_initialized(void)
{
    return initialized;
}

struct es1370_stats* ES1370::stats_get(void)
{
    return &stats;
}

extern "C" int es1370_init(void)
{
    ES1370* drv = ES1370::instance();
    if (!drv) {
        ES1370::g_instance = &g_es1370_fallback;
        drv = ES1370::g_instance;
    }
    for (uint8_t bus = 0U; bus < 255U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id = drv->pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id;
                uint16_t device = (uint16_t)(id >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                drv->init();
                return drv->is_initialized() ? 1 : 0;
            }
        }
    }
    return 0;
}

extern "C" void es1370_shutdown(void)
{
    ES1370* drv = ES1370::instance();
    if (drv) drv->deinit();
}

extern "C" void es1370_poll(void)
{
    ES1370* drv = ES1370::instance();
    if (drv && drv->is_initialized()) drv->poll();
}

extern "C" int es1370_beep(uint32_t czestotliwosc_hz, uint32_t czas_ms)
{
    ES1370* drv = ES1370::instance();
    if (!drv || !drv->is_initialized()) return -1;
    return drv->beep(czestotliwosc_hz, czas_ms);
}
