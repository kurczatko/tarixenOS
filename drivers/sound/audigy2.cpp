#include "audigy2.h"
#include "../../include/printf/vga.h"

#define AUDIGY2_VENDOR_ID 0x1102
#define AUDIGY2_DEVICE_ID 0x0004

#define REG_HCFG 0x00
#define REG_ICWR 0x04
#define REG_ICDR 0x08
#define REG_PTR 0x7C
#define REG_DATA 0x80

#define DSP_PLAYBACK_LIST_ADDR 0x00
#define DSP_CAPTURE_LIST_ADDR 0x01
#define DSP_EFX_RAM_ADDR 0x02

static Audigy2* g_audigy2 = 0;

Audigy2::Audigy2()
    : mmio_base(0), io_base(0), pci_seg(0), pci_bus(0), pci_dev(0), pci_func(0),
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
    playback_buf.rate = 44100;
    playback_buf.format = 16;
    capture_buf.dma_addr = 0;
    capture_buf.size = 0;
    capture_buf.pos = 0;
    capture_buf.period = 0;
    capture_buf.channels = 2;
    capture_buf.rate = 44100;
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
    return g_audigy2;
}

uint8_t Audigy2::probe(uint16_t vendor, uint16_t device)
{
    if (vendor != AUDIGY2_VENDOR_ID || device != AUDIGY2_DEVICE_ID) return 0;
    return 1;
}

void Audigy2::write_reg(uint32_t offset, uint32_t value)
{
    if (io_base == 0) return;
    volatile uint32_t* ptr = (volatile uint32_t*)(io_base + REG_PTR);
    volatile uint32_t* data = (volatile uint32_t*)(io_base + REG_DATA);
    *ptr = offset;
    *data = value;
}

uint32_t Audigy2::read_reg(uint32_t offset)
{
    if (io_base == 0) return 0;
    volatile uint32_t* ptr = (volatile uint32_t*)(io_base + REG_PTR);
    volatile uint32_t* data = (volatile uint32_t*)(io_base + REG_DATA);
    *ptr = offset;
    return *data;
}

void Audigy2::write_dsp(uint32_t addr, uint32_t value)
{
    write_reg(0x100 + (addr & 0xFF), value);
}

uint32_t Audigy2::read_dsp(uint32_t addr)
{
    return read_reg(0x100 + (addr & 0xFF));
}

void Audigy2::init(void)
{
    if (initialized) return;
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
    initialized = 0;
}

void Audigy2::irq_handler(void)
{
    if (!initialized) return;
    uint32_t status = read_reg(REG_ICDR);
    (void)status;
}

void Audigy2::mixer_init(void)
{
    write_reg(0x20, master_vol);
    write_reg(0x24, master_vol);
}

void Audigy2::spdif_init(void)
{
    write_reg(0x40, 0x00000001);
}

void Audigy2::efx_init(void)
{
    write_reg(0x60, eax_enabled ? 1 : 0);
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
    buf->dma_addr = (uint32_t*)0x10000000;
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
    write_reg(0x80, (uint32_t)buf->dma_addr);
    write_reg(0x84, buf->size);
    write_reg(0x88, 0x00000001);
}

void Audigy2::dma_stop_playback(PcmBuffer* buf)
{
    write_reg(0x88, 0x00000000);
}

int Audigy2::pcm_open(uint16_t channels, uint32_t rate, uint16_t format)
{
    if (!initialized) return -1;
    playback_buf.channels = channels;
    playback_buf.rate = rate;
    playback_buf.format = format;
    write_reg(0x90, rate);
    write_reg(0x94, channels);
    write_reg(0x98, format);
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
    voice->buffer_addr = (uint32_t)buffer;
    voice->buffer_size = size;
    voice->active = 1;
    voices[voice->voice_id] = *voice;
    write_dsp(0x200 + voice->voice_id * 8, (uint32_t)buffer);
    write_dsp(0x204 + voice->voice_id * 8, size);
    write_dsp(0x208 + voice->voice_id * 8, 0x00000001);
    spatial_mix_update();
}

void Audigy2::voice3d_stop(Voice3D* voice)
{
    voice->active = 0;
    voices[voice->voice_id] = *voice;
    write_dsp(0x208 + voice->voice_id * 8, 0x00000000);
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
    write_reg(0x60, eax_enabled);
    return 0;
}

int Audigy2::eax_set_listener(float x, float y, float z,
                              float fx, float fy, float fz,
                              float ux, float uy, float uz)
{
    write_dsp(0x340, *(uint32_t*)&x);
    write_dsp(0x344, *(uint32_t*)&y);
    write_dsp(0x348, *(uint32_t*)&z);
    write_dsp(0x34C, *(uint32_t*)&fx);
    write_dsp(0x350, *(uint32_t*)&fy);
    write_dsp(0x354, *(uint32_t*)&fz);
    write_dsp(0x358, *(uint32_t*)&ux);
    write_dsp(0x35C, *(uint32_t*)&uy);
    write_dsp(0x360, *(uint32_t*)&uz);
    return 0;
}

void Audigy2::set_master_volume(uint8_t percent)
{
    master_vol = percent;
    if (initialized) {
        write_reg(0x20, percent);
        write_reg(0x24, percent);
    }
}

uint8_t Audigy2::get_master_volume(void)
{
    return master_vol;
}

extern "C" void audigy2_irq(void)
{
    if (g_audigy2) g_audigy2->irq_handler();
}
