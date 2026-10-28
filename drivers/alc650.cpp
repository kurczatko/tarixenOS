#include "alc650.h"

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t function,
                                   uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

static void delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        delay_us(1000);
    }
}

static void *memcpy(void *dest, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

static void *memset(void *s, int c, uint32_t n)
{
    uint8_t *p = (uint8_t *)s;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return s;
}

#define AC97_REG_RESET           0x00
#define AC97_REG_MASTER_VOLUME   0x02
#define AC97_REG_HEADPHONE_VOL   0x04
#define AC97_REG_MASTER_MONO     0x06
#define AC97_REG_PC_BEEP         0x08
#define AC97_REG_PHONE           0x0A
#define AC97_REG_MIC             0x0C
#define AC97_REG_LINE_IN         0x0E
#define AC97_REG_CD              0x10
#define AC97_REG_VIDEO           0x12
#define AC97_REG_AUX             0x14
#define AC97_REG_PCM_OUT         0x16
#define AC97_REG_RECORD_SELECT   0x18
#define AC97_REG_RECORD_GAIN     0x1A
#define AC97_REG_RECORD_GAIN_MIC 0x1C
#define AC97_REG_GENERAL_PURPOSE 0x20
#define AC97_REG_3D_CONTROL      0x22
#define AC97_REG_POWERDOWN       0x26
#define AC97_REG_EXTENDED_AUDIO  0x28
#define AC97_REG_EXTENDED_STATUS 0x2A
#define AC97_REG_PCM_DAC_RATE    0x2C
#define AC97_REG_PCM_ADC_RATE    0x32
#define AC97_REG_VENDOR_ID1      0x7C
#define AC97_REG_VENDOR_ID2      0x7E

#define AC97_IO_CRST      0x00
#define AC97_IO_CSTAT     0x04
#define AC97_IO_CNT       0x08
#define AC97_IO_GLOB_CNT  0x10
#define AC97_IO_GLOB_STA  0x14
#define AC97_IO_PO_CR     0x20
#define AC97_IO_PO_PICB   0x24
#define AC97_IO_PO_CIV    0x28
#define AC97_IO_PI_CR     0x30
#define AC97_IO_MIC_CR    0x40
#define AC97_IO_PO_BDL    0x60
#define AC97_IO_PO_BDL_HI 0x64
#define AC97_IO_CODEC_INDEX 0xA0
#define AC97_IO_CODEC_DATA  0xA4

#define ALC650_VENDOR_ID 0x10ECU
#define ALC650_BUFFER_SIZE 88200U
#define ALC650_BDL_ENTRIES 8U

static uint16_t g_ac97_base = 0;
static uint8_t g_ac97_bus = 0;
static uint8_t g_ac97_slot = 0;
static uint8_t g_ac97_func = 0;
static volatile uint8_t *g_ac97_mmio = 0;
static int g_ac97_is_mmio = 0;
static int g_initialized = 0;
static int g_codec_found = 0;
static uint8_t g_codec_address = 0;
static int g_playing = 0;
static uint32_t g_sample_rate = 44100U;
static uint32_t g_buffer_bytes = 0;
static int g_buffer_loop = 0;
static uint16_t g_master_volume = 100U;
static uint16_t g_pcm_volume = 100U;

static uint16_t g_audio_buffer[ALC650_BUFFER_SIZE] __attribute__((aligned(1024)));

struct ac97_bdl_entry {
    uint32_t address_low;
    uint32_t address_high;
    uint32_t length_ioc;
} __attribute__((packed));

static struct ac97_bdl_entry g_bdl[ALC650_BDL_ENTRIES] __attribute__((aligned(128)));

static inline void ac97_write32(uint32_t reg, uint32_t value)
{
    if (g_ac97_is_mmio) {
        *(volatile uint32_t *)(g_ac97_mmio + reg) = value;
    } else {
        outl((uint16_t)(g_ac97_base + reg), value);
    }
}

static inline uint32_t ac97_read32(uint32_t reg)
{
    if (g_ac97_is_mmio) {
        return *(volatile uint32_t *)(g_ac97_mmio + reg);
    }
    return inl((uint16_t)(g_ac97_base + reg));
}

static inline void ac97_write16(uint32_t reg, uint16_t value)
{
    if (g_ac97_is_mmio) {
        *(volatile uint16_t *)(g_ac97_mmio + reg) = value;
    } else {
        outw((uint16_t)(g_ac97_base + reg), value);
    }
}

static inline uint16_t ac97_read16(uint32_t reg)
{
    if (g_ac97_is_mmio) {
        return *(volatile uint16_t *)(g_ac97_mmio + reg);
    }
    return inw((uint16_t)(g_ac97_base + reg));
}

static int ac97_find_controller(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read_config_dword((uint8_t)bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                uint32_t class_code = pci_read_config_dword((uint8_t)bus, slot, func, 0x08);
                uint8_t base_class = (uint8_t)(class_code >> 24);
                uint8_t sub_class = (uint8_t)((class_code >> 16) & 0xFF);
                if (base_class != 0x04 || sub_class != 0x01) continue;
                uint32_t bar = pci_read_config_dword((uint8_t)bus, slot, func, 0x10);
                if (bar == 0 || bar == 0xFFFFFFFFU) continue;
                g_ac97_bus = (uint8_t)bus;
                g_ac97_slot = slot;
                g_ac97_func = func;
                if ((bar & 1U) != 0) {
                    g_ac97_is_mmio = 1;
                    g_ac97_mmio = (volatile uint8_t *)(bar & 0xFFFFFFF0U);
                } else {
                    g_ac97_is_mmio = 0;
                    g_ac97_base = (uint16_t)(bar & 0xFFFEU);
                }
                return 1;
            }
        }
    }
    return 0;
}

static int ac97_reset_link(void)
{
    ac97_write16(AC97_IO_CRST, 0x0001);
    delay_ms(10);
    ac97_write16(AC97_IO_CRST, 0x0000);
    for (volatile int wait = 0; wait < 1000000; wait++) {
        if (!(ac97_read16(AC97_IO_CRST) & 0x0001)) break;
    }
    delay_ms(10);
    uint32_t stat = ac97_read32(AC97_IO_GLOB_STA);
    if ((stat & 0x00000001U) != 0U) return 0;
    return 1;
}

static void codec_write_reg(uint16_t reg, uint16_t value)
{
    ac97_write16(AC97_IO_CODEC_INDEX, (uint16_t)(reg & 0x7FU));
    ac97_write16(AC97_IO_CODEC_DATA, value);
    for (volatile int wait = 0; wait < 1000000; wait++) {
        if ((ac97_read16(AC97_IO_CSTAT) & 0x0001U) == 0U) break;
    }
}

static uint16_t codec_read_reg(uint16_t reg)
{
    ac97_write16(AC97_IO_CODEC_INDEX, (uint16_t)((reg & 0x7FU) | 0x8000U));
    for (volatile int wait = 0; wait < 1000000; wait++) {
        if ((ac97_read16(AC97_IO_CSTAT) & 0x0001U) == 0U) break;
    }
    return ac97_read16(AC97_IO_CODEC_DATA);
}

static int ac97_detect_codec(void)
{
    uint16_t vendor1 = codec_read_reg(AC97_REG_VENDOR_ID1);
    uint16_t vendor2 = codec_read_reg(AC97_REG_VENDOR_ID2);
    if (vendor1 == 0xFFFF || vendor1 == 0x0000) return 0;
    if (vendor1 != ALC650_VENDOR_ID) return 0;
    g_codec_found = 1;
    g_codec_address = 0;
    (void)vendor2;
    return 1;
}

static void ac97_init_mixer(void)
{
    codec_write_reg(AC97_REG_POWERDOWN, 0x0000);
    codec_write_reg(AC97_REG_MASTER_VOLUME, 0x0808);
    codec_write_reg(AC97_REG_PCM_OUT, 0x0808);
    codec_write_reg(AC97_REG_MIC, 0x0808);
    codec_write_reg(AC97_REG_CD, 0x0808);
    codec_write_reg(AC97_REG_RECORD_SELECT, 0x0000);
    codec_write_reg(AC97_REG_RECORD_GAIN, 0x0808);
    g_master_volume = 100U;
    g_pcm_volume = 100U;
}

int alc650_init(void)
{
    if (g_initialized) return 1;
    if (!ac97_find_controller()) return 0;
    uint32_t command = pci_read_config_dword(g_ac97_bus, g_ac97_slot, g_ac97_func, 0x04);
    pci_write_config_dword(g_ac97_bus, g_ac97_slot, g_ac97_func, 0x04, command | 0x00000006U);
    if (!ac97_reset_link()) return 0;
    if (!ac97_detect_codec()) return 0;
    ac97_init_mixer();
    memset(g_audio_buffer, 0, sizeof(g_audio_buffer));
    memset(g_bdl, 0, sizeof(g_bdl));
    g_bdl[0].address_low = (uint32_t)(void *)&g_audio_buffer[0];
    g_bdl[0].address_high = 0;
    g_bdl[0].length_ioc = 0;
    ac97_write32(AC97_IO_PO_BDL, (uint32_t)(void *)&g_bdl[0]);
    ac97_write32(AC97_IO_PO_BDL_HI, 0);
    ac97_write16(AC97_IO_PO_CR, 0x0000);
    ac97_write16(AC97_IO_CNT, 0x0001);
    g_initialized = 1;
    return 1;
}

void alc650_shutdown(void)
{
    if (!g_initialized) return;
    alc650_stop();
    ac97_write16(AC97_IO_PO_CR, 0x0000);
    g_initialized = 0;
    g_playing = 0;
}

int alc650_play_pcm(const void *samples, uint32_t byte_count, uint32_t sample_rate_hz, int loop)
{
    if (!g_initialized || !samples || byte_count == 0) return -1;
    alc650_stop();
    uint32_t copy_bytes = byte_count;
    if (copy_bytes > sizeof(g_audio_buffer)) copy_bytes = sizeof(g_audio_buffer);
    memcpy((void *)g_audio_buffer, samples, copy_bytes);
    g_buffer_bytes = copy_bytes;
    g_buffer_loop = loop != 0 ? 1 : 0;
    g_sample_rate = sample_rate_hz == 0 ? 44100U : sample_rate_hz;
    uint16_t rate_reg = 0xAC44U;
    if (g_sample_rate == 48000U) rate_reg = 0xBB80U;
    else if (g_sample_rate == 22050U) rate_reg = 0x5622U;
    else if (g_sample_rate == 16000U) rate_reg = 0x3E80U;
    else if (g_sample_rate == 8000U) rate_reg = 0x1F40U;
    codec_write_reg(AC97_REG_PCM_DAC_RATE, rate_reg);
    g_bdl[0].length_ioc = copy_bytes | 0x80000000U;
    ac97_write32(AC97_IO_PO_BDL, (uint32_t)(void *)&g_bdl[0]);
    ac97_write32(AC97_IO_PO_BDL_HI, 0);
    ac97_write16(AC97_IO_PO_CR, 0x0000);
    delay_us(100);
    ac97_write16(AC97_IO_PO_CR, 0x0001);
    g_playing = 1;
    return 0;
}

int alc650_playing(void)
{
    if (!g_initialized) return 0;
    if (!g_playing) return 0;
    uint16_t cr = ac97_read16(AC97_IO_PO_CR);
    if ((cr & 0x0001U) == 0U) {
        g_playing = 0;
        if (g_buffer_loop) {
            alc650_play_pcm((const void *)g_audio_buffer, g_buffer_bytes, g_sample_rate, 1);
        }
        return 0;
    }
    return 1;
}

void alc650_stop(void)
{
    if (!g_initialized) return;
    ac97_write16(AC97_IO_PO_CR, 0x0000);
    g_playing = 0;
}

void alc650_set_master_volume(unsigned int percent)
{
    if (percent > 100U) percent = 100U;
    uint16_t atten = (uint16_t)(((100U - percent) * 31U) / 100U);
    uint16_t value = (uint16_t)((atten << 8) | atten);
    if (percent == 0U) value = (uint16_t)(value | 0x8000U);
    codec_write_reg(AC97_REG_MASTER_VOLUME, value);
    g_master_volume = (uint16_t)percent;
}

void alc650_set_pcm_volume(unsigned int percent)
{
    if (percent > 100U) percent = 100U;
    uint16_t atten = (uint16_t)(((100U - percent) * 31U) / 100U);
    uint16_t value = (uint16_t)((atten << 8) | atten);
    if (percent == 0U) value = (uint16_t)(value | 0x8000U);
    codec_write_reg(AC97_REG_PCM_OUT, value);
    g_pcm_volume = (uint16_t)percent;
}

void alc650_set_mic_volume(unsigned int percent)
{
    if (percent > 100U) percent = 100U;
    uint16_t atten = (uint16_t)(((100U - percent) * 31U) / 100U);
    uint16_t value = (uint16_t)((atten << 8) | atten);
    if (percent == 0U) value = (uint16_t)(value | 0x8000U);
    codec_write_reg(AC97_REG_MIC, value);
}

void alc650_set_cd_volume(unsigned int percent)
{
    if (percent > 100U) percent = 100U;
    uint16_t atten = (uint16_t)(((100U - percent) * 31U) / 100U);
    uint16_t value = (uint16_t)((atten << 8) | atten);
    if (percent == 0U) value = (uint16_t)(value | 0x8000U);
    codec_write_reg(AC97_REG_CD, value);
}

void alc650_mute_output(int mute)
{
    uint16_t vol = codec_read_reg(AC97_REG_MASTER_VOLUME);
    if (mute) vol = (uint16_t)(vol | 0x8000U);
    else vol = (uint16_t)(vol & ~0x8000U);
    codec_write_reg(AC97_REG_MASTER_VOLUME, vol);
}

void alc650_mute_input(int mute)
{
    uint16_t vol = codec_read_reg(AC97_REG_MIC);
    if (mute) vol = (uint16_t)(vol | 0x8000U);
    else vol = (uint16_t)(vol & ~0x8000U);
    codec_write_reg(AC97_REG_MIC, vol);
}

unsigned int alc650_get_master_volume(void)
{
    return (unsigned int)g_master_volume;
}

unsigned int alc650_get_pcm_volume(void)
{
    return (unsigned int)g_pcm_volume;
}

const char *alc650_codec_name(void)
{
    return "Realtek ALC650";
}

uint32_t alc650_get_sample_rate(void)
{
    return g_sample_rate;
}

void alc650_poll(void)
{
    (void)alc650_playing();
}
