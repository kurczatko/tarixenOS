#ifndef AUDIGY2_H
#define AUDIGY2_H

#include <include/printf/types.h>

class Audigy2 {
public:
    struct PcmBuffer {
        uint32_t* dma_addr;
        uint32_t size;
        uint32_t pos;
        uint32_t period;
        uint16_t channels;
        uint32_t rate;
        uint16_t format;
    };

    struct Voice3D {
        uint32_t voice_id;
        uint8_t active;
        float pos_x, pos_y, pos_z;
        float velocity_x, velocity_y, velocity_z;
        float orientation_x, orientation_y, orientation_z;
        float gain;
        float min_distance;
        float max_distance;
        uint32_t buffer_addr;
        uint32_t buffer_size;
    };

    struct EaxEnvironment {
        uint32_t room;
        uint32_t room_hf;
        uint32_t reflections;
        uint32_t reflections_delay;
        uint32_t reverb;
        uint32_t reverb_delay;
        uint32_t decay_time;
        uint32_t decay_hf_ratio;
        uint32_t reflections_hf;
        uint32_t reverb_hf;
        uint32_t diffusion;
        uint32_t density;
        uint32_t flags;
    };

    static Audigy2* instance();

    uint8_t probe(uint16_t vendor, uint16_t device);
    void init(void);
    void deinit(void);
    void irq_handler(void);
    uint8_t is_initialized(void) const { return initialized; }

    int pcm_open(uint16_t channels, uint32_t rate, uint16_t format);
    int pcm_write(const void* data, uint32_t bytes);
    int pcm_read(void* data, uint32_t bytes);
    void pcm_close(void);

    int voice3d_alloc(Voice3D* voice);
    void voice3d_free(Voice3D* voice);
    void voice3d_set_position(Voice3D* voice, float x, float y, float z);
    void voice3d_set_velocity(Voice3D* voice, float x, float y, float z);
    void voice3d_set_gain(Voice3D* voice, float gain);
    void voice3d_play(Voice3D* voice, const void* buffer, uint32_t size);
    void voice3d_stop(Voice3D* voice);

    int eax_set_environment(const EaxEnvironment* env);
    int eax_enable(uint8_t enable);
    int eax_set_listener(float x, float y, float z,
                         float fx, float fy, float fz,
                         float ux, float uy, float uz);

    void set_master_volume(uint8_t percent);
    uint8_t get_master_volume(void);

    Audigy2();
    ~Audigy2();

    static Audigy2* g_instance;

private:
    uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
    void pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

    void write_reg(uint32_t reg, uint32_t ch, uint32_t value);
    uint32_t read_reg(uint32_t reg, uint32_t ch);

    void write_direct32(uint32_t offset, uint32_t value);
    uint32_t read_direct32(uint32_t offset);
    void write_direct16(uint32_t offset, uint16_t value);
    uint16_t read_direct16(uint32_t offset);
    void write_direct8(uint32_t offset, uint8_t value);
    uint8_t read_direct8(uint32_t offset);

    void write_dsp(uint32_t addr, uint32_t value);
    uint32_t read_dsp(uint32_t addr);

    void ac97_write(uint8_t reg, uint16_t val);
    uint16_t ac97_read(uint8_t reg);

    void dma_alloc(PcmBuffer* buf, uint32_t size);
    void dma_free(PcmBuffer* buf);
    void dma_start_playback(PcmBuffer* buf);
    void dma_stop_playback(PcmBuffer* buf);

    void mixer_init(void);
    void spdif_init(void);
    void efx_init(void);
    void spatial_mix_update(void);

    uint32_t io_base;
    uint16_t pci_seg, pci_bus, pci_dev, pci_func;
    uint8_t irq;
    uint8_t initialized;

    PcmBuffer playback_buf;
    PcmBuffer capture_buf;

    Voice3D voices[64];
    uint32_t voice_mask;

    EaxEnvironment current_env;
    uint8_t eax_enabled;

    uint8_t master_vol;
};

extern "C" void audigy2_irq(void);

#endif
