/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/*
* ten plik definiuje sterownik karty dzwiekowej Ensoniq ES1370 / ES1371 (AudioPCI)
* zawiera pelna obsluge PCI, rejestrow, kodeka AC'97, odtwarzania PCM, miksera,
* glowosned 3D, dzwieku systemowego oraz statystyk
*/

#ifndef ENSONIQ_ES1370_H
#define ENSONIQ_ES1370_H

#include <include/printf/types.h>

/* -------------------------------------------------------------
 * Identyfikatory PCI Ensoniq ES1370 / ES1371 (Creative AudioPCI)
 * ------------------------------------------------------------- */

#define ENSONIQ_VENDOR_ID      0x1274U
#define ENSONIQ_DEVICE_ES1370  0x1370U
#define ENSONIQ_DEVICE_ES1371  0x5880U

/* Rejestry ES1370 (offset od bazy IO / MMIO) */
#define ES1370_REG_CONTROL         0x00
#define ES1370_REG_STATUS          0x04
#define ES1370_REG_SERIAL          0x10
#define ES1370_REG_DAC1_RATE       0x18
#define ES1370_REG_DAC2_RATE       0x1C
#define ES1370_REG_ADC_RATE        0x20
#define ES1370_REG_DAC1_FRAMES     0x40
#define ES1370_REG_DAC2_FRAMES     0x44
#define ES1370_REG_ADC_FRAMES      0x48
#define ES1370_REG_DAC1_ADDR       0x50
#define ES1370_REG_DAC2_ADDR       0x54
#define ES1370_REG_ADC_ADDR        0x58
#define ES1370_REG_DAC1_COUNT      0x60
#define ES1370_REG_DAC2_COUNT      0x64
#define ES1370_REG_ADC_COUNT       0x68
#define ES1370_REG_AC97_INDEX      0x7C
#define ES1370_REG_AC97_DATA       0x80

/* Bity rejestru kontrolnego */
#define ES1370_CTRL_DAC1_EN        (1U << 0)
#define ES1370_CTRL_DAC2_EN        (1U << 1)
#define ES1370_CTRL_ADC_EN         (1U << 2)
#define ES1370_CTRL_DAC1_PAUSE     (1U << 8)
#define ES1370_CTRL_DAC2_PAUSE     (1U << 9)
#define ES1370_CTRL_ADC_PAUSE      (1U << 10)

/* Rejestry kodeka AC'97 */
#define ES1370_AC97_RESET          0x00
#define ES1370_AC97_MASTER_VOL     0x02
#define ES1370_AC97_PCM_VOL        0x18
#define ES1370_AC97_MIC_VOL        0x0C
#define ES1370_AC97_CD_VOL         0x0E
#define ES1370_AC97_REC_SEL        0x0A
#define ES1370_AC97_POWER          0x26
#define ES1370_AC97_EXT_AUDIO      0x2C
#define ES1370_AC97_SERIAL         0x04

#define ES1370_MAX_VOICES          32U
#define ES1370_PCM_BUFFER_SIZE     65536U
#define ES1370_TONE_BUFFER_SIZE    4096U

struct es1370_pcm_buffer {
    uint32_t *dma_addr;
    uint32_t size;
    uint32_t pos;
    uint32_t period;
    uint16_t channels;
    uint32_t rate;
    uint16_t format;
};

struct es1370_voice {
    uint32_t voice_id;
    uint8_t active;
    float pos_x, pos_y, pos_z;
    float velocity_x, velocity_y, velocity_z;
    float gain;
    float min_distance;
    float max_distance;
    uint32_t buffer_addr;
    uint32_t buffer_size;
};

struct es1370_stats {
    uint32_t pcm_bytes_written;
    uint32_t pcm_bytes_read;
    uint32_t beeps_played;
    uint32_t interrupts;
    uint32_t ac97_errors;
    uint32_t pcm_underruns;
};

#ifdef __cplusplus
class ES1370 {
public:
    static ES1370* instance();

    uint8_t probe(uint16_t vendor, uint16_t device);
    void init(void);
    void deinit(void);
    void irq_handler(void);
    void poll(void);

    int pcm_open(uint16_t channels, uint32_t rate, uint16_t format);
    int pcm_write(const void* data, uint32_t bytes);
    int pcm_read(void* data, uint32_t bytes);
    void pcm_close(void);
    int pcm_pause(uint8_t pause);

    int beep(uint32_t czestotliwosc_hz, uint32_t czas_ms);
    void sygnal_systemowy(void);
    void pip_krotki(void);
    void pip_dlugi(void);

    void set_master_volume(uint8_t percent);
    uint8_t get_master_volume(void);
    void set_pcm_volume(uint8_t percent);
    uint8_t get_pcm_volume(void);
    void set_mic_volume(uint8_t percent);
    uint8_t get_mic_volume(void);
    void mute_output(int mute);
    void mute_input(int mute);

    uint8_t is_initialized(void);

    struct es1370_stats* stats_get(void);

public:
    ES1370();
    ~ES1370();

    static ES1370* g_instance;

    uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
    void pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

private:
    void delay_us(uint32_t us);
    void delay_ms(uint32_t ms);

    uint8_t inb_reg(uint16_t reg);
    uint16_t inw_reg(uint16_t reg);
    uint32_t inl_reg(uint16_t reg);
    void outb_reg(uint16_t reg, uint8_t value);
    void outw_reg(uint16_t reg, uint16_t value);
    void outl_reg(uint16_t reg, uint32_t value);

    uint32_t ac97_read(uint8_t reg);
    void ac97_write(uint8_t reg, uint16_t value);
    uint8_t ac97_wait_ready(void);

    void dma_alloc(struct es1370_pcm_buffer* buf, uint32_t size);
    void dma_free(struct es1370_pcm_buffer* buf);
    void dma_start_playback(struct es1370_pcm_buffer* buf);
    void dma_stop_playback(struct es1370_pcm_buffer* buf);

    void mixer_init(void);
    void ac97_init(void);
    void generate_tone(uint16_t* buffer, uint32_t samples, uint32_t czestotliwosc, uint32_t rate);

    uint32_t mmio_base;
    uint32_t io_base;
    uint16_t pci_seg, pci_bus, pci_dev, pci_func;
    uint8_t irq;
    uint8_t initialized;
    uint8_t ac97_ready;

    struct es1370_pcm_buffer playback_buf;
    struct es1370_pcm_buffer capture_buf;

    struct es1370_voice voices[ES1370_MAX_VOICES];
    uint32_t voice_mask;

    uint8_t master_volume;
    uint8_t pcm_volume;
    uint8_t mic_volume;
    uint8_t output_muted;
    uint8_t input_muted;

    struct es1370_stats stats;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

int es1370_init(void);
void es1370_shutdown(void);
void es1370_poll(void);
int es1370_beep(uint32_t czestotliwosc_hz, uint32_t czas_ms);

#ifdef __cplusplus
}
#endif

#endif
