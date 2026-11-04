#include "asus_p4p800.h"

static inline void p4p_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t p4p_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void p4p_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t p4p_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void p4p_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t p4p_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void p4p_io_delay(void)
{
    p4p_inb(0x80U);
}

static inline uint8_t p4p_wait_not_busy(uint16_t status_port, uint8_t mask, uint32_t tries)
{
    for (uint32_t i = 0; i < tries; i++) {
        if ((p4p_inb(status_port) & mask) == 0U) return 1;
    }
    return 0;
}

static inline void p4p_memcpy(void *dst, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (uint32_t i = 0; i < n; i++) d[i] = s[i];
}

static inline void p4p_memset(void *dst, uint8_t value, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    for (uint32_t i = 0; i < n; i++) d[i] = value;
}

static inline void p4p_bcd_to_bin(uint8_t *value)
{
    *value = (uint8_t)(((*value >> 4) * 10U) + (*value & 0x0FU));
}

static inline uint8_t p4p_bin_to_bcd(uint8_t value)
{
    return (uint8_t)(((value / 10U) << 4) | (value % 10U));
}

static uint32_t p4p_pci_read32_raw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xFCU);
    p4p_outl(P4P_PCI_CONFIG_ADDRESS, address);
    p4p_io_delay();
    return p4p_inl(P4P_PCI_CONFIG_DATA);
}

static void p4p_pci_write32_raw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xFCU);
    p4p_outl(P4P_PCI_CONFIG_ADDRESS, address);
    p4p_io_delay();
    p4p_outl(P4P_PCI_CONFIG_DATA, value);
}

static uint8_t p4p_pci_find_bus_slot_func(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)
{
    uint32_t found = 0;
    for (uint8_t b = 0; b < 8; b++) {
        for (uint8_t s = 0; s < 32; s++) {
            for (uint8_t f = 0; f < 8; f++) {
                uint32_t id = p4p_pci_read32_raw(b, s, f, P4P_PCI_REG_VENDOR_DEVICE);
                if ((uint16_t)id == 0xFFFFU) continue;
                if ((uint16_t)id == vendor && (uint16_t)(id >> 16) == device) {
                    if (found == index) {
                        *bus = b;
                        *slot = s;
                        *func = f;
                        return 1;
                    }
                    found++;
                }
            }
        }
    }
    return 0;
}

static volatile uint8_t p4p_ready;
static volatile uint32_t p4p_init_counter;
static struct p4p_info p4p_dev;
static struct p4p_stats p4p_stats;
static void (*p4p_irq_handlers[16])(uint8_t irq) = {0};

int p4p_init(void)
{
    if (p4p_ready) return 1;
    p4p_init_counter++;
    p4p_memset(&p4p_dev, 0, sizeof(p4p_dev));
    p4p_memset(&p4p_stats, 0, sizeof(p4p_stats));
    p4p_memset(p4p_irq_handlers, 0, sizeof(p4p_irq_handlers));

    p4p_chipset_probe();
    p4p_acpi_available();
    p4p_smbus_init();
    p4p_agp_init();
    p4p_audio_init();
    p4p_eth_init();
    p4p_ide_init();
    p4p_usb_init();
    p4p_floppy_init();
    p4p_rtc_init();
    p4p_hwmon_init();

    p4p_dev.initialized = 1;
    p4p_ready = 1;
    return 1;
}

int p4p_is_ready(void)
{
    return p4p_ready;
}

void p4p_shutdown(void)
{
    p4p_acpi_shutdown();
    p4p_ready = 0;
}

void p4p_reset(void)
{
    p4p_acpi_reboot();
}

uint32_t p4p_get_chipset_id(void)
{
    return ((uint32_t)p4p_dev.mch_device_id << 16) | p4p_dev.ich_device_id;
}

void p4p_info_get(struct p4p_info *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_dev, sizeof(p4p_dev));
}

void p4p_stats_get(struct p4p_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats, sizeof(p4p_stats));
}

int p4p_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)
{
    if (!bus || !slot || !func) return 0;
    return p4p_pci_find_bus_slot_func(vendor, device, index, bus, slot, func);
}

uint32_t p4p_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    p4p_stats.pci_reads++;
    return p4p_pci_read32_raw(bus, slot, func, offset);
}

void p4p_pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    p4p_stats.pci_writes++;
    p4p_pci_write32_raw(bus, slot, func, offset, value);
}

uint16_t p4p_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    p4p_stats.pci_reads++;
    return (uint16_t)p4p_pci_read32_raw(bus, slot, func, offset & 0xFCU);
}

void p4p_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value)
{
    p4p_stats.pci_writes++;
    uint32_t reg = p4p_pci_read32_raw(bus, slot, func, offset & 0xFCU);
    uint8_t shift = (offset & 0x02U) * 8U;
    reg = (reg & ~(0xFFFFU << shift)) | ((uint32_t)value << shift);
    p4p_pci_write32_raw(bus, slot, func, offset & 0xFCU, reg);
}

uint8_t p4p_pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    p4p_stats.pci_reads++;
    return (uint8_t)(p4p_pci_read32_raw(bus, slot, func, offset & 0xFCU) >> ((offset & 0x03U) * 8U));
}

void p4p_pci_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value)
{
    p4p_stats.pci_writes++;
    uint32_t reg = p4p_pci_read32_raw(bus, slot, func, offset & 0xFCU);
    uint8_t shift = (offset & 0x03U) * 8U;
    reg = (reg & ~(0xFFU << shift)) | ((uint32_t)value << shift);
    p4p_pci_write32_raw(bus, slot, func, offset & 0xFCU, reg);
}

int p4p_pci_set_bus_master(uint8_t bus, uint8_t slot, uint8_t func, uint8_t enable)
{
    uint16_t command = p4p_pci_read16(bus, slot, func, P4P_PCI_REG_COMMAND);
    if (enable) command |= 0x0004U;
    else command &= (uint16_t)~0x0004U;
    p4p_pci_write16(bus, slot, func, P4P_PCI_REG_COMMAND, command);
    return 1;
}

int p4p_chipset_probe(void)
{
    uint8_t bus, slot, func;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_MCH_I848P, 0, &bus, &slot, &func)) {
        p4p_dev.mch_device_id = P4P_DEVICE_MCH_I848P;
        p4p_dev.mch_revision = (uint8_t)p4p_pci_read8(bus, slot, func, P4P_PCI_REG_REVISION);
        p4p_pci_set_bus_master(bus, slot, func, 1);
    } else {
        p4p_dev.mch_device_id = 0xFFFFU;
    }
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_LPC, 0, &bus, &slot, &func)) {
        p4p_dev.ich_device_id = P4P_DEVICE_ICH4_LPC;
        p4p_dev.ich_revision = (uint8_t)p4p_pci_read8(bus, slot, func, P4P_PCI_REG_REVISION);
        p4p_dev.acpi_base = p4p_pci_read32(bus, slot, func, P4P_ICH4_ACPI_CNT_OFFSET) & 0xFFFFFF00U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
    } else {
        p4p_dev.ich_device_id = 0xFFFFU;
    }
    p4p_chipset_configure_memory();
    return 1;
}

void p4p_chipset_configure_memory(void)
{
    uint8_t bus, slot, func;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_MCH_I848P, 0, &bus, &slot, &func)) {
        p4p_pci_write8(bus, slot, func, 0x5CU, p4p_pci_read8(bus, slot, func, 0x5CU) | 0x01U);
    }
}

static uint8_t p4p_acpi_present;

int p4p_acpi_available(void)
{
    p4p_acpi_present = 0;
    if (p4p_dev.acpi_base != 0U && p4p_dev.acpi_base != 0xFFFFFFFFU) {
        p4p_outw(p4p_dev.acpi_base + P4P_ACPI_PM1A_CNT, p4p_inw(p4p_dev.acpi_base + P4P_ACPI_PM1A_CNT) | P4P_ACPI_SCI_EN);
        p4p_acpi_present = 1;
    }
    return p4p_acpi_present;
}

void p4p_acpi_sleep(uint8_t state)
{
    if (!p4p_acpi_present) return;
    uint16_t val = p4p_inw(p4p_dev.acpi_base + P4P_ACPI_PM1A_CNT);
    val &= (uint16_t)~P4P_ACPI_SLP_EN;
    val |= (uint16_t)((state & 0x07U) << 10U);
    p4p_outw(p4p_dev.acpi_base + P4P_ACPI_PM1A_CNT, val | P4P_ACPI_SLP_EN);
    for (volatile uint32_t wait = 0; wait < 100000U; wait++);
}

void p4p_acpi_reboot(void)
{
    p4p_outb(0xCF9U, 0x02U);
    for (volatile uint32_t wait = 0; wait < 1000000U; wait++);
    p4p_outb(0x64U, 0xFEU);
}

void p4p_acpi_shutdown(void)
{
    if (p4p_acpi_present) {
        p4p_acpi_sleep(5U);
    }
    p4p_outb(0x64U, 0xFEU);
}

static uint32_t p4p_smbus_base;

int p4p_smbus_init(void)
{
    uint8_t bus, slot, func;
    p4p_smbus_base = 0;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_SMBUS, 0, &bus, &slot, &func)) {
        p4p_smbus_base = p4p_pci_read32(bus, slot, func, 0x04U) & 0xFFFFFFF0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        return 1;
    }
    return 0;
}

int p4p_smbus_read_byte(uint8_t device, uint8_t offset, uint8_t *value)
{
    if (!p4p_smbus_base || !value) return 0;
    uint8_t index = (device << 1) | 0x01U;
    p4p_outb(p4p_smbus_base, index);
    p4p_outb(p4p_smbus_base + 2U, offset);
    p4p_outb(p4p_smbus_base + 3U, 0x48U);
    for (uint32_t i = 0; i < 10000U; i++) {
        if (!(p4p_inb(p4p_smbus_base + 2U) & 0x01U)) {
            *value = p4p_inb(p4p_smbus_base + 4U);
            return 1;
        }
    }
    p4p_stats.hwmon.errors++;
    return 0;
}

int p4p_smbus_write_byte(uint8_t device, uint8_t offset, uint8_t value)
{
    if (!p4p_smbus_base) return 0;
    uint8_t index = device << 1;
    p4p_outb(p4p_smbus_base, index);
    p4p_outb(p4p_smbus_base + 2U, offset);
    p4p_outb(p4p_smbus_base + 4U, value);
    p4p_outb(p4p_smbus_base + 3U, 0x48U);
    for (uint32_t i = 0; i < 10000U; i++) {
        if (!(p4p_inb(p4p_smbus_base + 2U) & 0x01U)) return 1;
    }
    p4p_stats.hwmon.errors++;
    return 0;
}

static uint32_t p4p_agp_aperture_size;

int p4p_agp_init(void)
{
    uint8_t bus, slot, func;
    p4p_agp_aperture_size = 0;
    p4p_dev.agp_aperture = 0;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_MCH_I848P, 0, &bus, &slot, &func)) {
        p4p_dev.agp_aperture = p4p_pci_read32(bus, slot, func, P4P_AGP_REG_AGPBASE) & 0xFFFFF000U;
        p4p_agp_aperture_size = 64U * 1024U * 1024U;
        p4p_dev.agp_available = 1;
        return 1;
    }
    return 0;
}

uint32_t p4p_agp_aperture(void)
{
    return p4p_dev.agp_aperture;
}

int p4p_agp_bind(uint32_t gatt_index, uint32_t physical_addr)
{
    (void)gatt_index;
    (void)physical_addr;
    if (!p4p_dev.agp_available) return 0;
    return 1;
}

int p4p_agp_unbind(uint32_t gatt_index)
{
    (void)gatt_index;
    if (!p4p_dev.agp_available) return 0;
    return 1;
}

static uint32_t p4p_ac97_base;
static uint32_t p4p_ac97_bm_base;
static uint8_t p4p_audio_ready;
static uint8_t p4p_ac97_volume[4] = {50U, 50U, 50U, 50U};
static uint8_t *p4p_audio_playback_buffer;
static uint32_t p4p_audio_playback_bytes;
static uint32_t p4p_audio_playback_offset;
static uint8_t *p4p_audio_capture_buffer;
static uint32_t p4p_audio_capture_bytes;
static uint32_t p4p_audio_capture_offset;

int p4p_audio_init(void)
{
    uint8_t bus, slot, func;
    p4p_ac97_base = 0;
    p4p_ac97_bm_base = 0;
    p4p_audio_ready = 0;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_AC97, 0, &bus, &slot, &func)) {
        p4p_ac97_base = p4p_pci_read32(bus, slot, func, P4P_AC97_MIXER_BAR_OFFSET) & 0xFFFFFFC0U;
        p4p_ac97_bm_base = p4p_pci_read32(bus, slot, func, P4P_AC97_BM_BAR_OFFSET) & 0xFFFFFFC0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        p4p_audio_codec_reset(1);
        p4p_audio_set_volume(0U, 50U);
        p4p_audio_set_volume(1U, 50U);
        p4p_dev.ac97_base = p4p_ac97_base;
        p4p_audio_ready = 1;
        p4p_dev.audio_ready = 1;
        return 1;
    }
    return 0;
}

int p4p_audio_is_ready(void)
{
    return p4p_audio_ready;
}

int p4p_audio_codec_read(uint8_t reg, uint16_t *value)
{
    if (!p4p_ac97_base || !value) return 0;
    p4p_outw(p4p_ac97_base + P4P_AC97_REG_RESET + reg, 0x8000U);
    for (volatile uint32_t wait = 0; wait < 1000U; wait++);
    *value = p4p_inw(p4p_ac97_base + P4P_AC97_REG_RESET + reg);
    p4p_stats.audio.codec_reads++;
    return 1;
}

int p4p_audio_codec_write(uint8_t reg, uint16_t value)
{
    if (!p4p_ac97_base) return 0;
    p4p_outw(p4p_ac97_base + P4P_AC97_REG_RESET + reg, value);
    p4p_stats.audio.codec_writes++;
    return 1;
}

int p4p_audio_codec_reset(int cold)
{
    if (!p4p_ac97_base) return 0;
    if (cold) p4p_outb(p4p_ac97_base, 0x01U);
    else p4p_outb(p4p_ac97_base, 0x02U);
    for (volatile uint32_t wait = 0; wait < 10000U; wait++);
    p4p_outb(p4p_ac97_base, 0x00U);
    return 1;
}

int p4p_audio_set_volume(uint8_t channel, uint8_t percent)
{
    if (channel > 3U) return 0;
    uint16_t att = (uint16_t)((100U - percent) * 2U);
    if (att > 0x3EU) att = 0x3EU;
    p4p_ac97_volume[channel] = percent;
    if (channel == 0U) p4p_audio_codec_write(P4P_AC97_REG_MASTER, (uint16_t)(att | (att << 8)));
    else if (channel == 1U) p4p_audio_codec_write(P4P_AC97_REG_MASTER_MONO, att);
    else if (channel == 2U) p4p_audio_codec_write(P4P_AC97_REG_PCM_OUT, (uint16_t)(att | (att << 8)));
    else p4p_audio_codec_write(P4P_AC97_REG_PCM_IN, (uint16_t)(att | (att << 8)));
    return 1;
}

uint8_t p4p_audio_get_volume(uint8_t channel)
{
    if (channel > 3U) return 0;
    return p4p_ac97_volume[channel];
}

int p4p_audio_playback_start(const void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)
{
    (void)rate; (void)channels; (void)bits;
    if (!p4p_audio_ready || !buffer || bytes == 0) return 0;
    p4p_audio_playback_buffer = (uint8_t *)buffer;
    p4p_audio_playback_bytes = bytes;
    p4p_audio_playback_offset = 0;
    p4p_outl(p4p_ac97_bm_base, reinterpret_cast<uint32_t>(buffer));
    p4p_outw(p4p_ac97_bm_base + 4U, (uint16_t)bytes);
    p4p_outb(p4p_ac97_bm_base + 1U, 0x02U);
    return 1;
}

int p4p_audio_playback_stop(void)
{
    if (!p4p_audio_ready) return 0;
    p4p_outb(p4p_ac97_bm_base + 1U, 0x00U);
    p4p_audio_playback_buffer = 0;
    p4p_audio_playback_bytes = 0;
    p4p_audio_playback_offset = 0;
    return 1;
}

int p4p_audio_capture_start(void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)
{
    (void)rate; (void)channels; (void)bits;
    if (!p4p_audio_ready || !buffer || bytes == 0) return 0;
    p4p_audio_capture_buffer = (uint8_t *)buffer;
    p4p_audio_capture_bytes = bytes;
    p4p_audio_capture_offset = 0;
    p4p_outl(p4p_ac97_bm_base + 8U, reinterpret_cast<uint32_t>(buffer));
    p4p_outw(p4p_ac97_bm_base + 12U, (uint16_t)bytes);
    p4p_outb(p4p_ac97_bm_base + 9U, 0x02U);
    return 1;
}

int p4p_audio_capture_stop(void)
{
    if (!p4p_audio_ready) return 0;
    p4p_outb(p4p_ac97_bm_base + 9U, 0x00U);
    p4p_audio_capture_buffer = 0;
    p4p_audio_capture_bytes = 0;
    p4p_audio_capture_offset = 0;
    return 1;
}

void p4p_audio_stats_get(struct p4p_audio_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats.audio, sizeof(p4p_stats.audio));
}

static uint32_t p4p_eth_base;
static uint8_t p4p_eth_ready;
static uint8_t p4p_eth_mac[6];

int p4p_eth_init(void)
{
    uint8_t bus, slot, func;
    p4p_eth_base = 0;
    p4p_eth_ready = 0;
    p4p_memset(p4p_eth_mac, 0, 6);
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ETH_82562ET, 0, &bus, &slot, &func)) {
        p4p_eth_base = p4p_pci_read32(bus, slot, func, P4P_PCI_REG_BAR0) & 0xFFFFFFF0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        p4p_outl(p4p_eth_base + P4P_ETH_CSR_PORT, P4P_ETH_CSR_PORT_SW_RESET);
        for (volatile uint32_t wait = 0; wait < 10000U; wait++);
        p4p_outl(p4p_eth_base + P4P_ETH_CSR_PORT, P4P_ETH_CSR_PORT_SEL_RESET);
        for (volatile uint32_t wait = 0; wait < 10000U; wait++);
        for (uint32_t i = 0; i < 6; i++) p4p_eth_mac[i] = p4p_inb(p4p_eth_base + 0x14U + i);
        p4p_outl(p4p_eth_base + P4P_ETH_CSR_SCB, P4P_ETH_SCB_CMD_IACK);
        p4p_outl(p4p_eth_base + P4P_ETH_CSR_SCB, P4P_ETH_SCB_CMD_CU_START | P4P_ETH_SCB_CMD_RU_START);
        p4p_eth_ready = 1;
        p4p_dev.eth_base = p4p_eth_base;
        p4p_dev.eth_ready = 1;
        return 1;
    }
    return 0;
}

int p4p_eth_is_ready(void)
{
    return p4p_eth_ready;
}

int p4p_eth_get_mac(uint8_t mac[6])
{
    if (!p4p_eth_ready || !mac) return 0;
    p4p_memcpy(mac, p4p_eth_mac, 6);
    return 1;
}

int p4p_eth_link_up(void)
{
    if (!p4p_eth_ready) return 0;
    p4p_outl(p4p_eth_base + P4P_ETH_CSR_SCB, P4P_ETH_SCB_CMD_IACK);
    return (p4p_inl(p4p_eth_base + P4P_ETH_CSR_SCB) & 0x00000004U) != 0U;
}

uint32_t p4p_eth_speed_mbps(void)
{
    if (!p4p_eth_ready) return 0;
    return 100U;
}

int p4p_eth_send(const void *frame, uint16_t length)
{
    if (!p4p_eth_ready || !frame || length == 0) return 0;
    p4p_outl(p4p_eth_base + P4P_ETH_CSR_PORT, reinterpret_cast<uint32_t>(frame));
    p4p_outw(p4p_eth_base + P4P_ETH_CSR_SCB, P4P_ETH_SCB_CMD_CU_START);
    p4p_stats.eth.tx_packets++;
    p4p_stats.eth.tx_bytes += length;
    return 1;
}

int p4p_eth_receive(void *frame, uint16_t capacity)
{
    if (!p4p_eth_ready || !frame || capacity == 0) return 0;
    uint32_t status = p4p_inl(p4p_eth_base + P4P_ETH_CSR_SCB);
    if (!(status & 0x0001U)) return 0;
    uint16_t length = (uint16_t)(status & 0x0FFFU);
    if (length == 0 || length > capacity) return 0;
    p4p_memcpy(frame, reinterpret_cast<const void *>(p4p_inl(p4p_eth_base + P4P_ETH_CSR_PORT)), length);
    p4p_outl(p4p_eth_base + P4P_ETH_CSR_SCB, P4P_ETH_SCB_CMD_IACK);
    p4p_stats.eth.rx_packets++;
    p4p_stats.eth.rx_bytes += length;
    return (int)length;
}

void p4p_eth_stats_get(struct p4p_eth_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats.eth, sizeof(p4p_stats.eth));
}

static uint32_t p4p_ide_base[2];
static uint32_t p4p_ide_bm_base[2];
static uint8_t p4p_ide_ready;

int p4p_ide_init(void)
{
    uint8_t bus, slot, func;
    p4p_ide_base[0] = 0x1F0U;
    p4p_ide_base[1] = 0x170U;
    p4p_ide_bm_base[0] = 0;
    p4p_ide_bm_base[1] = 0;
    p4p_ide_ready = 0;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_IDE, 0, &bus, &slot, &func)) {
        p4p_ide_bm_base[0] = p4p_pci_read32(bus, slot, func, P4P_PCI_REG_BAR4) & 0xFFFFFFF0U;
        p4p_ide_bm_base[1] = p4p_ide_bm_base[0] + 8U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        p4p_dev.ide_base = p4p_ide_bm_base[0];
        p4p_dev.bm_ide_base = p4p_ide_bm_base[0];
        p4p_ide_ready = 1;
        p4p_dev.ide_ready = 1;
        return 1;
    }
    return 1;
}

int p4p_ide_is_ready(void)
{
    return p4p_ide_ready;
}

int p4p_ide_identify(uint8_t channel, uint8_t drive, uint16_t *buffer)
{
    if (!p4p_ide_ready || channel > 1U || !buffer) return 0;
    uint16_t reg = p4p_ide_base[channel];
    p4p_outb(reg + P4P_IDE_REG_DRIVE_HEAD, (uint8_t)(0xA0U | (drive << 4U)));
    p4p_outb(reg + P4P_IDE_REG_FEATURES, 0x00U);
    p4p_outb(reg + P4P_IDE_REG_COMMAND, P4P_IDE_CMD_IDENTIFY);
    if (!p4p_wait_not_busy(reg + P4P_IDE_REG_COMMAND, 0x80U, 100000U)) return 0;
    uint8_t status = p4p_inb(reg + P4P_IDE_REG_COMMAND);
    if (status & 0x01U) return 0;
    p4p_memset(buffer, 0, 512);
    for (uint32_t i = 0; i < 256; i++) buffer[i] = p4p_inw(reg + P4P_IDE_REG_DATA);
    p4p_stats.ide.reads++;
    return 1;
}

int p4p_ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)
{
    if (!p4p_ide_ready || channel > 1U || !buffer || count == 0) return 0;
    uint16_t reg = p4p_ide_base[channel];
    p4p_outb(reg + P4P_IDE_REG_SECCOUNT, count);
    p4p_outb(reg + P4P_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    p4p_outb(reg + P4P_IDE_REG_COMMAND, P4P_IDE_CMD_READ);
    if (!p4p_wait_not_busy(reg + P4P_IDE_REG_COMMAND, 0x80U, 1000000U)) return 0;
    uint16_t *dst = (uint16_t *)buffer;
    for (uint32_t s = 0; s < count; s++) {
        if (!p4p_wait_not_busy(reg + P4P_IDE_REG_COMMAND, 0x88U, 1000000U)) { p4p_stats.ide.errors++; return 0; }
        for (uint32_t i = 0; i < 256; i++) dst[s * 256 + i] = p4p_inw(reg + P4P_IDE_REG_DATA);
    }
    p4p_stats.ide.reads++;
    p4p_stats.ide.sectors_read += count;
    return (int)count;
}

int p4p_ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)
{
    if (!p4p_ide_ready || channel > 1U || !buffer || count == 0) return 0;
    uint16_t reg = p4p_ide_base[channel];
    p4p_outb(reg + P4P_IDE_REG_SECCOUNT, count);
    p4p_outb(reg + P4P_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    p4p_outb(reg + P4P_IDE_REG_COMMAND, P4P_IDE_CMD_WRITE);
    if (!p4p_wait_not_busy(reg + P4P_IDE_REG_COMMAND, 0x80U, 1000000U)) return 0;
    const uint16_t *src = (const uint16_t *)buffer;
    for (uint32_t s = 0; s < count; s++) {
        if (!p4p_wait_not_busy(reg + P4P_IDE_REG_COMMAND, 0x80U, 1000000U)) { p4p_stats.ide.errors++; return 0; }
        for (uint32_t i = 0; i < 256; i++) p4p_outw(reg + P4P_IDE_REG_DATA, src[s * 256 + i]);
    }
    p4p_outb(reg + P4P_IDE_REG_COMMAND, 0xE7U);
    p4p_stats.ide.writes++;
    p4p_stats.ide.sectors_written += count;
    return (int)count;
}

int p4p_ide_dma_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)
{
    (void)buffer;
    if (!p4p_ide_ready || channel > 1U || p4p_ide_bm_base[channel] == 0) return 0;
    uint16_t bm = p4p_ide_bm_base[channel];
    p4p_outb(bm + P4P_IDE_BM_REG_CMD, 0x00U);
    p4p_outl(bm + P4P_IDE_BM_REG_PRDT, reinterpret_cast<uint32_t>(buffer));
    p4p_outb(bm + P4P_IDE_BM_REG_CMD, P4P_IDE_BM_CMD_START | P4P_IDE_BM_CMD_READ);
    uint16_t reg = p4p_ide_base[channel];
    p4p_outb(reg + P4P_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    p4p_outb(reg + P4P_IDE_REG_COMMAND, P4P_IDE_CMD_READ_DMA);
    for (uint32_t i = 0; i < 1000000U; i++) {
        if (!(p4p_inb(bm + P4P_IDE_BM_REG_STATUS) & 0x02U)) break;
    }
    p4p_outb(bm + P4P_IDE_BM_REG_CMD, 0x00U);
    p4p_stats.ide.dma_transfers++;
    p4p_stats.ide.sectors_read += count;
    return (int)count;
}

int p4p_ide_dma_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)
{
    (void)buffer;
    if (!p4p_ide_ready || channel > 1U || p4p_ide_bm_base[channel] == 0) return 0;
    uint16_t bm = p4p_ide_bm_base[channel];
    p4p_outb(bm + P4P_IDE_BM_REG_CMD, 0x00U);
    p4p_outl(bm + P4P_IDE_BM_REG_PRDT, reinterpret_cast<uint32_t>(buffer));
    p4p_outb(bm + P4P_IDE_BM_REG_CMD, P4P_IDE_BM_CMD_START);
    uint16_t reg = p4p_ide_base[channel];
    p4p_outb(reg + P4P_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    p4p_outb(reg + P4P_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    p4p_outb(reg + P4P_IDE_REG_COMMAND, P4P_IDE_CMD_WRITE_DMA);
    for (uint32_t i = 0; i < 1000000U; i++) {
        if (!(p4p_inb(bm + P4P_IDE_BM_REG_STATUS) & 0x02U)) break;
    }
    p4p_outb(bm + P4P_IDE_BM_REG_CMD, 0x00U);
    p4p_stats.ide.dma_transfers++;
    p4p_stats.ide.sectors_written += count;
    return (int)count;
}

void p4p_ide_reset(uint8_t channel)
{
    if (!p4p_ide_ready || channel > 1U) return;
    p4p_outb(p4p_ide_base[channel] + 0x206U, 0x04U);
    p4p_outb(p4p_ide_base[channel] + 0x206U, 0x00U);
}

void p4p_ide_stats_get(struct p4p_ide_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats.ide, sizeof(p4p_stats.ide));
}

static uint32_t p4p_usb_base[3];
static uint8_t p4p_usb_ready;
static uint8_t p4p_usb_ports;

int p4p_usb_init(void)
{
    uint8_t bus, slot, func;
    p4p_usb_base[0] = 0;
    p4p_usb_base[1] = 0;
    p4p_usb_base[2] = 0;
    p4p_usb_ready = 0;
    p4p_usb_ports = 6U;
    int found = 0;
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_USB1, 0, &bus, &slot, &func)) {
        p4p_usb_base[0] = p4p_pci_read32(bus, slot, func, P4P_PCI_REG_BAR0) & 0xFFFFFFF0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_USB2, 0, &bus, &slot, &func)) {
        p4p_usb_base[1] = p4p_pci_read32(bus, slot, func, P4P_PCI_REG_BAR0) & 0xFFFFFFF0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_USB3, 0, &bus, &slot, &func)) {
        p4p_usb_base[2] = p4p_pci_read32(bus, slot, func, P4P_PCI_REG_BAR0) & 0xFFFFFFF0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (p4p_pci_find_bus_slot_func(P4P_VENDOR_INTEL, P4P_DEVICE_ICH4_EHCI, 0, &bus, &slot, &func)) {
        p4p_dev.ehci_base = p4p_pci_read32(bus, slot, func, P4P_PCI_REG_BAR0) & 0xFFFFFFF0U;
        p4p_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (found) p4p_usb_ready = 1;
    p4p_dev.usb_ready = p4p_usb_ready;
    return found;
}

int p4p_usb_is_ready(void)
{
    return p4p_usb_ready;
}

uint8_t p4p_usb_root_port_count(void)
{
    return p4p_usb_ports;
}

int p4p_usb_port_reset(uint8_t port)
{
    if (!p4p_usb_ready || port > p4p_usb_ports) return 0;
    uint32_t base = p4p_usb_base[port / 2U];
    uint32_t port_reg = P4P_UHC_REG_PORTSC1 + (uint32_t)(port % 2U) * 2U;
    if (base == 0) return 0;
    p4p_outw(base + port_reg, p4p_inw(base + port_reg) | 0x0010U);
    p4p_stats.usb.resets++;
    return 1;
}

int p4p_usb_port_power(uint8_t port, uint8_t on)
{
    if (!p4p_usb_ready || port > p4p_usb_ports) return 0;
    uint32_t base = p4p_usb_base[port / 2U];
    uint32_t port_reg = P4P_UHC_REG_PORTSC1 + (uint32_t)(port % 2U) * 2U;
    if (base == 0) return 0;
    uint16_t val = p4p_inw(base + port_reg);
    val = on ? (val | 0x0040U) : (uint16_t)(val & ~0x0040U);
    p4p_outw(base + port_reg, val);
    return 1;
}

uint32_t p4p_usb_port_status(uint8_t port)
{
    if (!p4p_usb_ready || port > p4p_usb_ports) return 0;
    uint32_t base = p4p_usb_base[port / 2U];
    uint32_t port_reg = P4P_UHC_REG_PORTSC1 + (uint32_t)(port % 2U) * 2U;
    if (base == 0) return 0;
    return (uint32_t)p4p_inw(base + port_reg);
}

int p4p_usb_control_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data)
{
    (void)addr; (void)endpoint; (void)direction; (void)request_type; (void)request; (void)value; (void)index; (void)data;
    if (!p4p_usb_ready || length == 0) return 0;
    p4p_stats.usb.transfers++;
    return (int)length;
}

int p4p_usb_bulk_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint32_t length, void *data)
{
    (void)addr; (void)endpoint; (void)direction; (void)data;
    if (!p4p_usb_ready || length == 0) return 0;
    p4p_stats.usb.transfers++;
    return (int)length;
}

void p4p_usb_stats_get(struct p4p_usb_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats.usb, sizeof(p4p_stats.usb));
}

static uint16_t p4p_floppy_base;
static uint8_t p4p_floppy_ready;

int p4p_floppy_init(void)
{
    p4p_floppy_base = 0x3F0U;
    p4p_floppy_ready = 0;
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_DOR, P4P_FDC_DOR_RESET);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_DOR, P4P_FDC_DOR_DMA | P4P_FDC_DOR_DRIVE0);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CCR, 0x00U);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, P4P_FDC_CMD_SPECIFY);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0xDFU);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x02U);
    p4p_floppy_ready = 1;
    p4p_dev.floppy_base = p4p_floppy_base;
    p4p_dev.floppy_ready = 1;
    return 1;
}

int p4p_floppy_is_ready(void)
{
    return p4p_floppy_ready;
}

int p4p_floppy_recalibrate(uint8_t drive)
{
    if (!p4p_floppy_ready || drive > 1U) return 0;
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_DOR, (uint8_t)(P4P_FDC_DOR_DMA | (drive & 0x03U)));
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, P4P_FDC_CMD_CALIBRATE);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, drive);
    p4p_stats.floppy.seeks++;
    return 1;
}

int p4p_floppy_seek(uint8_t drive, uint8_t track)
{
    if (!p4p_floppy_ready || drive > 1U) return 0;
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_DOR, (uint8_t)(P4P_FDC_DOR_DMA | (drive & 0x03U)));
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, P4P_FDC_CMD_SEEK);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, drive);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, track);
    p4p_stats.floppy.seeks++;
    return 1;
}

int p4p_floppy_read_sectors(uint8_t drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t count, void *buffer)
{
    if (!p4p_floppy_ready || drive > 1U || !buffer || count == 0) return 0;
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_DOR, (uint8_t)(P4P_FDC_DOR_DMA | (drive & 0x03U) | (head << 2U)));
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, P4P_FDC_CMD_READ);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, (uint8_t)((drive << 2U) | head));
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, track);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, head);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, sector);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x02U);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x1AU);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x06U);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0xFFU);
    p4p_memcpy(buffer, buffer, count * 512U);
    p4p_stats.floppy.reads++;
    return (int)count;
}

int p4p_floppy_write_sectors(uint8_t drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t count, const void *buffer)
{
    (void)buffer;
    if (!p4p_floppy_ready || drive > 1U || count == 0) return 0;
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_DOR, (uint8_t)(P4P_FDC_DOR_DMA | (drive & 0x03U) | (head << 2U)));
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, P4P_FDC_CMD_WRITE);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, (uint8_t)((drive << 2U) | head));
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, track);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, head);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, sector);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x02U);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x1AU);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0x06U);
    p4p_outb(p4p_floppy_base + P4P_FDC_REG_CMD, 0xFFU);
    p4p_stats.floppy.writes++;
    return (int)count;
}

uint8_t p4p_floppy_status(void)
{
    if (!p4p_floppy_ready) return 0;
    return p4p_inb(p4p_floppy_base + P4P_FDC_REG_STATUS_A);
}

void p4p_floppy_stats_get(struct p4p_floppy_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats.floppy, sizeof(p4p_stats.floppy));
}

static uint8_t p4p_rtc_ready;

int p4p_rtc_init(void)
{
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_B);
    uint8_t b = p4p_inb(P4P_RTC_DATA) | P4P_RTC_STATUS_B_24H | P4P_RTC_STATUS_B_DM_BINARY;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_B);
    p4p_outb(P4P_RTC_DATA, b);
    p4p_rtc_ready = 1;
    p4p_dev.rtc_ready = 1;
    return 1;
}

int p4p_rtc_is_ready(void)
{
    return p4p_rtc_ready;
}

int p4p_rtc_get_time(struct p4p_rtc_time *out)
{
    if (!p4p_rtc_ready || !out) return 0;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_A);
    while (p4p_inb(P4P_RTC_DATA) & 0x80U);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_SECONDS);
    out->seconds = p4p_inb(P4P_RTC_DATA);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_MINUTES);
    out->minutes = p4p_inb(P4P_RTC_DATA);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_HOURS);
    out->hours = p4p_inb(P4P_RTC_DATA);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_DAY_OF_WEEK);
    out->day_of_week = p4p_inb(P4P_RTC_DATA);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_DAY_OF_MONTH);
    out->day_of_month = p4p_inb(P4P_RTC_DATA);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_MONTH);
    out->month = p4p_inb(P4P_RTC_DATA);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_YEAR);
    out->year = p4p_inb(P4P_RTC_DATA);
    return 1;
}

int p4p_rtc_set_time(const struct p4p_rtc_time *t)
{
    if (!p4p_rtc_ready || !t) return 0;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_B);
    p4p_outb(P4P_RTC_DATA, p4p_inb(P4P_RTC_DATA) | 0x80U);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_SECONDS);
    p4p_outb(P4P_RTC_DATA, t->seconds);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_MINUTES);
    p4p_outb(P4P_RTC_DATA, t->minutes);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_HOURS);
    p4p_outb(P4P_RTC_DATA, t->hours);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_DAY_OF_WEEK);
    p4p_outb(P4P_RTC_DATA, t->day_of_week);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_DAY_OF_MONTH);
    p4p_outb(P4P_RTC_DATA, t->day_of_month);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_MONTH);
    p4p_outb(P4P_RTC_DATA, t->month);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_YEAR);
    p4p_outb(P4P_RTC_DATA, t->year);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_B);
    p4p_outb(P4P_RTC_DATA, p4p_inb(P4P_RTC_DATA) & (uint8_t)~0x80U);
    return 1;
}

int p4p_rtc_nvram_read(uint8_t offset, uint8_t *value)
{
    if (!p4p_rtc_ready || !value) return 0;
    p4p_outb(P4P_RTC_INDEX, offset);
    *value = p4p_inb(P4P_RTC_DATA);
    return 1;
}

int p4p_rtc_nvram_write(uint8_t offset, uint8_t value)
{
    if (!p4p_rtc_ready) return 0;
    p4p_outb(P4P_RTC_INDEX, offset);
    p4p_outb(P4P_RTC_DATA, value);
    return 1;
}

int p4p_rtc_alarm_set(const struct p4p_rtc_time *t)
{
    if (!p4p_rtc_ready || !t) return 0;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_ALARM_SECONDS);
    p4p_outb(P4P_RTC_DATA, t->seconds);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_ALARM_MINUTES);
    p4p_outb(P4P_RTC_DATA, t->minutes);
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_ALARM_HOURS);
    p4p_outb(P4P_RTC_DATA, t->hours);
    return 1;
}

int p4p_rtc_irq_enable(uint8_t mask, uint8_t enable)
{
    if (!p4p_rtc_ready) return 0;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_B);
    uint8_t b = p4p_inb(P4P_RTC_DATA);
    if (enable) b |= mask;
    else b &= (uint8_t)~mask;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_B);
    p4p_outb(P4P_RTC_DATA, b);
    return 1;
}

uint8_t p4p_rtc_irq_status(void)
{
    if (!p4p_rtc_ready) return 0;
    p4p_outb(P4P_RTC_INDEX, P4P_RTC_REG_STATUS_C);
    return p4p_inb(P4P_RTC_DATA);
}

static uint16_t p4p_sio_base;
static uint8_t p4p_hwmon_ready;

static void p4p_sio_select_ldn(uint8_t ldn)
{
    p4p_outb(P4P_SIO_INDEX, P4P_SIO_REG_CONFIG);
    p4p_outb(P4P_SIO_DATA, ldn);
}

static uint8_t p4p_sio_read_reg(uint8_t reg)
{
    p4p_outb(P4P_SIO_INDEX, reg);
    return p4p_inb(P4P_SIO_DATA);
}

static void p4p_sio_write_reg(uint8_t reg, uint8_t value)
{
    p4p_outb(P4P_SIO_INDEX, reg);
    p4p_outb(P4P_SIO_DATA, value);
}

int p4p_hwmon_init(void)
{
    p4p_sio_base = P4P_SIO_INDEX;
    p4p_hwmon_ready = 0;
    p4p_outb(P4P_SIO_INDEX, P4P_SIO_KEY_ENTER);
    p4p_outb(P4P_SIO_INDEX, P4P_SIO_KEY_ENTER);
    uint8_t id = p4p_sio_read_reg(P4P_SIO_REG_CHIP_ID);
    p4p_sio_select_ldn(P4P_SIO_LDN_HWMON);
    uint8_t enable = p4p_sio_read_reg(0x30U);
    p4p_sio_write_reg(0x30U, (uint8_t)(enable | 0x01U));
    p4p_outb(P4P_SIO_INDEX, P4P_SIO_KEY_EXIT);
    p4p_hwmon_ready = 1;
    p4p_dev.hwmon_ready = 1;
    (void)id;
    return 1;
}

int p4p_hwmon_is_ready(void)
{
    return p4p_hwmon_ready;
}

int p4p_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius)
{
    if (!p4p_hwmon_ready || !millicelsius) return 0;
    p4p_sio_select_ldn(P4P_SIO_LDN_HWMON);
    uint8_t base_hi = p4p_sio_read_reg(0x60U);
    uint8_t base_lo = p4p_sio_read_reg(0x61U);
    uint16_t base = ((uint16_t)base_hi << 8) | base_lo;
    if (base == 0) return 0;
    p4p_outb(base, 0x27U + sensor);
    uint8_t raw = p4p_inb(base + 1U);
    *millicelsius = (int32_t)((int8_t)raw) * 1000;
    p4p_stats.hwmon.temperatures++;
    return 1;
}

int p4p_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt)
{
    (void)sensor;
    if (!p4p_hwmon_ready || !millivolt) return 0;
    *millivolt = 0;
    p4p_stats.hwmon.voltages++;
    return 1;
}

int p4p_hwmon_read_fan(uint8_t fan, uint32_t *rpm)
{
    (void)fan;
    if (!p4p_hwmon_ready || !rpm) return 0;
    *rpm = 0;
    p4p_stats.hwmon.fans++;
    return 1;
}

void p4p_hwmon_stats_get(struct p4p_hwmon_stats *out)
{
    if (!out) return;
    p4p_memcpy(out, &p4p_stats.hwmon, sizeof(p4p_stats.hwmon));
}

void p4p_irq_handler(uint8_t irq)
{
    if (irq < 16U && p4p_irq_handlers[irq]) {
        p4p_irq_handlers[irq](irq);
        p4p_stats.irq_dispatched++;
    }
}

int p4p_irq_register(uint8_t irq, void (*handler)(uint8_t irq))
{
    if (irq > 15U || !handler) return 0;
    p4p_irq_handlers[irq] = handler;
    return 1;
}
