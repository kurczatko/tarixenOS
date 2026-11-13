#ifndef INTEL_915_H
#define INTEL_915_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I915_VENDOR_INTEL 0x8086U

#define I915_PCI_CONFIG_ADDRESS 0xCF8U
#define I915_PCI_CONFIG_DATA 0xCFCU
#define I915_PCI_REG_VENDOR_DEVICE 0x00U
#define I915_PCI_REG_COMMAND 0x04U
#define I915_PCI_REG_REVISION 0x08U
#define I915_PCI_REG_CLASS 0x08U
#define I915_PCI_REG_BAR0 0x10U
#define I915_PCI_REG_BAR1 0x14U
#define I915_PCI_REG_BAR2 0x18U
#define I915_PCI_REG_BAR3 0x1CU
#define I915_PCI_REG_BAR4 0x20U
#define I915_PCI_REG_BAR5 0x24U
#define I915_PCI_REG_INTERRUPT_LINE 0x3CU
#define I915_PCI_REG_INTERRUPT_PIN 0x3DU

#define I915_DEVICE_MCH_915G 0x2580U
#define I915_DEVICE_MCH_915P 0x2590U
#define I915_DEVICE_MCH_915X 0x2591U
#define I915_DEVICE_MCH_915GV 0x2592U
#define I915_DEVICE_MCH_915GL 0x2582U
#define I915_DEVICE_GRAPHICS_915G 0x2582U
#define I915_DEVICE_ICH6_LPC 0x2640U
#define I915_DEVICE_ICH6_IDE 0x2641U
#define I915_DEVICE_ICH6_SATA 0x2642U
#define I915_DEVICE_ICH6_USB1 0x2644U
#define I915_DEVICE_ICH6_USB2 0x2645U
#define I915_DEVICE_ICH6_USB3 0x2646U
#define I915_DEVICE_ICH6_EHCI 0x264CU
#define I915_DEVICE_ICH6_AC97 0x2668U
#define I915_DEVICE_ICH6_SMBUS 0x2643U
#define I915_DEVICE_ICH6_LAN 0x1050U

#define I915_MCH_REG_GGC 0x50U
#define I915_MCH_REG_DEVEN 0x52U
#define I915_MCH_REG_PAM0 0x59U
#define I915_MCH_REG_PAM1 0x5AU
#define I915_MCH_REG_PAM2 0x5BU
#define I915_MCH_REG_PAM3 0x5CU
#define I915_MCH_REG_PAM4 0x5DU
#define I915_MCH_REG_PAM5 0x5EU
#define I915_MCH_REG_PAM6 0x5FU
#define I915_MCH_REG_DRAM_CLK 0x5CU
#define I915_MCH_REG_DRB0 0x60U
#define I915_MCH_REG_DRB1 0x61U
#define I915_MCH_REG_DRB2 0x62U
#define I915_MCH_REG_DRB3 0x63U
#define I915_MCH_REG_DRA0 0x68U
#define I915_MCH_REG_DRA1 0x69U
#define I915_MCH_REG_DRA2 0x6AU
#define I915_MCH_REG_DRA3 0x6BU
#define I915_MCH_REG_TOLUD 0xBCU
#define I915_MCH_REG_GME 0xC0U
#define I915_MCH_REG_STOLEN 0xC4U

#define I915_GFX_REG_VGA 0x71400U
#define I915_GFX_REG_PIPEA_CONF 0x60000U
#define I915_GFX_REG_PIPEA_STAT 0x60004U
#define I915_GFX_REG_PIPEA_HBLANK 0x60008U
#define I915_GFX_REG_PIPEA_VBLANK 0x6000CU
#define I915_GFX_REG_PIPEA_SRC 0x60010U
#define I915_GFX_REG_PIPEA_HSYNC 0x60014U
#define I915_GFX_REG_PIPEA_VSYNC 0x60018U
#define I915_GFX_REG_PIPEB_CONF 0x61000U
#define I915_GFX_REG_PIPEB_STAT 0x61004U
#define I915_GFX_REG_PIPEB_HBLANK 0x61008U
#define I915_GFX_REG_PIPEB_VBLANK 0x6100CU
#define I915_GFX_REG_PIPEB_SRC 0x61010U
#define I915_GFX_REG_PIPEB_HSYNC 0x61014U
#define I915_GFX_REG_PIPEB_VSYNC 0x61018U
#define I915_GFX_REG_DPLL_A 0x70000U
#define I915_GFX_REG_DPLL_B 0x70004U
#define I915_GFX_REG_FPA0 0x70040U
#define I915_GFX_REG_FPB0 0x70044U
#define I915_GFX_REG_DSPACNTR 0x70180U
#define I915_GFX_REG_DSPABASE 0x70184U
#define I915_GFX_REG_DSPASTRIDE 0x70188U
#define I915_GFX_REG_DSPBCNTR 0x70190U
#define I915_GFX_REG_DSPBBASE 0x70194U
#define I915_GFX_REG_DSPBSTRIDE 0x70198U
#define I915_GFX_REG_CURSOR_CTL 0x70080U
#define I915_GFX_REG_CURSOR_POS 0x70084U
#define I915_GFX_REG_CURSOR_BASE 0x70088U
#define I915_GFX_REG_OVADD 0xF0000U
#define I915_GFX_REG_OVSTA 0xF0004U
#define I915_GFX_REG_OVCTL 0xF0008U
#define I915_GFX_REG_RING_CTL 0x2030U
#define I915_GFX_REG_RING_START 0x2034U
#define I915_GFX_REG_RING_SIZE 0x2038U
#define I915_GFX_REG_RING_HEAD 0x203CU
#define I915_GFX_REG_RING_TAIL 0x2040U
#define I915_GFX_REG_BLT_CTL 0x20000U

#define I915_FB_ADDR_DEFAULT 0xE8000000U
#define I915_FB_WIDTH 1024U
#define I915_FB_HEIGHT 768U
#define I915_FB_PITCH 4096U
#define I915_RING_SIZE 4096U

#define I915_ICH6_REG_PMBASE 0x44U
#define I915_ICH6_REG_ACPI_CNT 0x44U
#define I915_ACPI_PM1A_CNT 0x00U
#define I915_ACPI_PM1A_EVT 0x04U
#define I915_ACPI_PM1B_CNT 0x08U
#define I915_ACPI_PM1B_EVT 0x0CU
#define I915_ACPI_PM2_CNT 0x10U
#define I915_ACPI_SLP_EN 0x2000U
#define I915_ACPI_SCI_EN 0x0001U
#define I915_ACPI_TCO_CNT 0x20U
#define I915_ACPI_TCO_STS 0x24U

#define I915_IDE_REG_DATA 0x00U
#define I915_IDE_REG_FEATURES 0x01U
#define I915_IDE_REG_SECCOUNT 0x02U
#define I915_IDE_REG_LBA0 0x03U
#define I915_IDE_REG_LBA1 0x04U
#define I915_IDE_REG_LBA2 0x05U
#define I915_IDE_REG_LBA3 0x06U
#define I915_IDE_REG_DRIVE_HEAD 0x06U
#define I915_IDE_REG_COMMAND 0x07U
#define I915_IDE_REG_ALTSTATUS 0x206U
#define I915_IDE_REG_CONTROL 0x206U
#define I915_IDE_CMD_IDENTIFY 0xECU
#define I915_IDE_CMD_READ 0x20U
#define I915_IDE_CMD_WRITE 0x30U
#define I915_IDE_CMD_READ_DMA 0xC8U
#define I915_IDE_CMD_WRITE_DMA 0xCAU
#define I915_IDE_CMD_FLUSH 0xE7U
#define I915_IDE_BM_REG_CMD 0x00U
#define I915_IDE_BM_REG_STATUS 0x02U
#define I915_IDE_BM_REG_PRDT 0x04U
#define I915_IDE_BM_CMD_START 0x01U
#define I915_IDE_BM_CMD_READ 0x08U

#define I915_AC97_REG_RESET 0x00U
#define I915_AC97_REG_MASTER 0x02U
#define I915_AC97_REG_MASTER_MONO 0x0AU
#define I915_AC97_REG_PCM_OUT 0x18U
#define I915_AC97_REG_PCM_IN 0x1CU
#define I915_AC97_REG_RECORD_SEL 0x1AU
#define I915_AC97_REG_GENERAL_PURPOSE 0x32U
#define I915_AC97_REG_POWER 0x34U
#define I915_AC97_REG_VENDOR_ID1 0x7CU
#define I915_AC97_MIXER_BAR_OFFSET 0x14U
#define I915_AC97_BM_BAR_OFFSET 0x18U

#define I915_SMBUS_REG_HOST_C 0x00U
#define I915_SMBUS_REG_HOST_S 0x02U
#define I915_SMBUS_REG_HOST_D0 0x04U
#define I915_SMBUS_REG_HOST_D1 0x05U
#define I915_SMBUS_REG_HOST_A 0x06U
#define I915_SMBUS_REG_HOST_CMD 0x07U
#define I915_SMBUS_HOST_KILL 0x01U
#define I915_SMBUS_HOST_START 0x40U
#define I915_SMBUS_HOST_INTR 0x02U
#define I915_SMBUS_HOST_ERROR 0x04U
#define I915_SMBUS_HOST_FAILED 0x08U
#define I915_SMBUS_HOST_BYTE_DATA 0x48U
#define I915_SMBUS_HOST_WORD_DATA 0x4CU
#define I915_SMBUS_HOST_BLOCK_DATA 0x50U

#define I915_UHC_REG_USBCMD 0x00U
#define I915_UHC_REG_USBSTS 0x02U
#define I915_UHC_REG_USBINTR 0x04U
#define I915_UHC_REG_FRNUM 0x06U
#define I915_UHC_REG_BASEADD 0x08U
#define I915_UHC_REG_SOFMOD 0x0CU
#define I915_UHC_REG_PORTSC1 0x10U
#define I915_UHC_REG_PORTSC2 0x12U

#define I915_EHCI_REG_USBCMD 0x00U
#define I915_EHCI_REG_USBSTS 0x04U
#define I915_EHCI_REG_USBINTR 0x08U
#define I915_EHCI_REG_FRNUM 0x0CU
#define I915_EHCI_REG_BASEADD 0x14U
#define I915_EHCI_REG_PORTSC1 0x44U
#define I915_EHCI_REG_PORTSC2 0x48U

#define I915_RTC_INDEX 0x70U
#define I915_RTC_DATA 0x71U
#define I915_RTC_REG_SECONDS 0x00U
#define I915_RTC_REG_MINUTES 0x02U
#define I915_RTC_REG_HOURS 0x04U
#define I915_RTC_REG_DAY_OF_WEEK 0x06U
#define I915_RTC_REG_DAY_OF_MONTH 0x07U
#define I915_RTC_REG_MONTH 0x08U
#define I915_RTC_REG_YEAR 0x09U
#define I915_RTC_REG_STATUS_A 0x0AU
#define I915_RTC_REG_STATUS_B 0x0BU
#define I915_RTC_REG_STATUS_C 0x0CU
#define I915_RTC_REG_ALARM_SECONDS 0x01U
#define I915_RTC_REG_ALARM_MINUTES 0x03U
#define I915_RTC_REG_ALARM_HOURS 0x05U
#define I915_RTC_STATUS_B_24H 0x02U
#define I915_RTC_STATUS_B_DM_BINARY 0x04U

#define I915_SIO_INDEX 0x2EU
#define I915_SIO_DATA 0x2FU
#define I915_SIO_KEY_ENTER 0x87U
#define I915_SIO_KEY_EXIT 0xAAU
#define I915_SIO_REG_CHIP_ID 0x20U
#define I915_SIO_REG_CONFIG 0x2EU
#define I915_SIO_LDN_HWMON 0x0BU

struct i915_rtc_time {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day_of_month;
    uint8_t month;
    uint8_t year;
};

struct i915_dram_info {
    uint32_t total_mb;
    uint32_t channels;
    uint8_t ddr2;
    uint32_t tcl;
    uint32_t trcd;
    uint32_t trp;
    uint32_t tras;
    uint32_t rank_count;
    uint32_t remap_base;
    uint32_t remap_limit;
    uint8_t ecc;
    uint8_t dual_channel;
};

struct i915_graphics_info {
    uint32_t mmio_base;
    uint32_t fb_addr;
    uint32_t fb_size;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t device_id;
    uint8_t revision;
    uint8_t initialized;
    uint8_t overlay_available;
    uint8_t cursor_available;
    uint8_t double_buffer;
};

struct i915_ide_info {
    uint32_t base[2];
    uint32_t bm_base[2];
    uint32_t sata_base;
    uint8_t sata_ports;
    uint8_t sata_mode;
    uint8_t dma_available;
    uint8_t ultra_dma_mode;
};

struct i915_usb_info {
    uint32_t uhci_base[4];
    uint32_t ehci_base;
    uint8_t ports;
    uint8_t ehci_available;
};

struct i915_audio_info {
    uint32_t mixer_base;
    uint32_t bm_base;
    uint16_t codec_vendor;
    uint8_t ready;
    uint8_t volume[4];
};

struct i915_acpi_info {
    uint32_t base;
    uint8_t available;
    uint8_t sci_enabled;
    uint8_t tco_available;
};

struct i915_info {
    uint16_t vendor_id;
    uint16_t mch_device_id;
    uint16_t ich_device_id;
    uint16_t graphics_device_id;
    uint8_t mch_revision;
    uint8_t ich_revision;
    uint8_t graphics_revision;
    uint32_t acpi_base;
    uint32_t smbus_base;
    uint32_t ac97_mixer_base;
    uint32_t ac97_bm_base;
    uint32_t ide_base[2];
    uint32_t bm_ide_base[2];
    uint32_t sata_base;
    uint32_t uhci_base[4];
    uint32_t ehci_base;
    uint32_t eth_base;
    uint32_t floppy_base;
    uint32_t agp_aperture;
    uint32_t gfx_mmio_base;
    uint32_t gfx_fb_addr;
    uint8_t acpi_available;
    uint8_t agp_available;
    uint8_t audio_ready;
    uint8_t eth_ready;
    uint8_t ide_ready;
    uint8_t sata_ready;
    uint8_t usb_ready;
    uint8_t floppy_ready;
    uint8_t rtc_ready;
    uint8_t hwmon_ready;
    uint8_t graphics_ready;
    uint8_t initialized;
    struct i915_dram_info dram;
    struct i915_graphics_info graphics;
    struct i915_ide_info ide;
    struct i915_usb_info usb;
    struct i915_audio_info audio;
    struct i915_acpi_info acpi;
};

struct i915_audio_stats {
    uint32_t codec_reads;
    uint32_t codec_writes;
    uint32_t playback_bytes;
    uint32_t capture_bytes;
    uint32_t errors;
};

struct i915_eth_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t resets;
};

struct i915_ide_stats {
    uint32_t reads;
    uint32_t writes;
    uint32_t sectors_read;
    uint32_t sectors_written;
    uint32_t errors;
    uint32_t dma_transfers;
};

struct i915_usb_stats {
    uint32_t ports;
    uint32_t resets;
    uint32_t transfers;
    uint32_t errors;
};

struct i915_floppy_stats {
    uint32_t reads;
    uint32_t writes;
    uint32_t seeks;
    uint32_t errors;
};

struct i915_hwmon_stats {
    uint32_t temperatures;
    uint32_t voltages;
    uint32_t fans;
    uint32_t errors;
};

struct i915_graphics_stats {
    uint32_t putpixels;
    uint32_t fillrects;
    uint32_t blts;
    uint32_t cursor_moves;
    uint32_t overlay_updates;
    uint32_t ring_flushes;
    uint32_t errors;
};

struct i915_stats {
    struct i915_audio_stats audio;
    struct i915_eth_stats eth;
    struct i915_ide_stats ide;
    struct i915_usb_stats usb;
    struct i915_floppy_stats floppy;
    struct i915_hwmon_stats hwmon;
    struct i915_graphics_stats graphics;
    uint32_t pci_reads;
    uint32_t pci_writes;
    uint32_t memory_ops;
    uint32_t smbus_ops;
    uint32_t acpi_events;
    uint32_t tco_resets;
    uint32_t irq_dispatched;
    uint32_t init_count;
    uint32_t errors;
};

int i915_init(void);
int i915_is_ready(void);
void i915_shutdown(void);
void i915_reset(void);
uint32_t i915_get_chipset_id(void);
void i915_info_get(struct i915_info *out);
void i915_stats_get(struct i915_stats *out);

int i915_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func);
uint32_t i915_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void i915_pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
uint16_t i915_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void i915_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
uint8_t i915_pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void i915_pci_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value);
int i915_pci_set_bus_master(uint8_t bus, uint8_t slot, uint8_t func, uint8_t enable);
uint8_t i915_pci_read_revision(uint8_t bus, uint8_t slot, uint8_t func);
int i915_pci_route_irq(uint8_t bus, uint8_t slot, uint8_t func, uint8_t *irq);

int i915_chipset_probe(void);
void i915_chipset_configure_memory(void);
int i915_pam_read(uint8_t reg, uint8_t *value);
int i915_pam_write(uint8_t reg, uint8_t value);
int i915_memory_map(struct i915_dram_info *out);
int i915_memory_remap_enable(uint8_t enable);
int i915_memory_set_timings(uint32_t tcl, uint32_t trcd, uint32_t trp, uint32_t tras);
int i915_memory_get_timings(uint32_t *tcl, uint32_t *trcd, uint32_t *trp, uint32_t *tras);
int i915_memory_scrub(uint32_t start, uint32_t size);
int i915_memory_interrupt_enable(uint8_t enable);
int i915_gme_enable(uint8_t enable);
uint32_t i915_stolen_memory_size(void);
int i915_agp_init(void);
uint32_t i915_agp_aperture(void);
int i915_agp_bind(uint32_t gatt_index, uint32_t physical_addr);
int i915_agp_unbind(uint32_t gatt_index);

int i915_graphics_init(void);
int i915_graphics_is_ready(void);
void i915_graphics_putpixel(uint32_t x, uint32_t y, uint32_t color);
void i915_graphics_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void i915_graphics_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void i915_graphics_cursor_set(uint32_t x, uint32_t y, const uint32_t *image);
void i915_graphics_cursor_move(uint32_t x, uint32_t y);
void i915_graphics_cursor_hide(void);
void i915_graphics_overlay_enable(uint8_t enable);
int i915_graphics_overlay_set_buffer(uint32_t addr, uint32_t width, uint32_t height, uint32_t pitch);
void i915_graphics_display_timings(uint32_t width, uint32_t height, uint32_t refresh);
void i915_graphics_pipe_config(uint8_t pipe, uint8_t enable);
void i915_graphics_plane_config(uint8_t plane, uint32_t addr, uint32_t width, uint32_t height, uint32_t stride);
void i915_graphics_blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void i915_graphics_blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void *i915_graphics_get_fb(void);
uint32_t i915_graphics_get_width(void);
uint32_t i915_graphics_get_height(void);
uint32_t i915_graphics_get_pitch(void);
uint16_t i915_graphics_get_device_id(void);
uint8_t i915_graphics_get_revision(void);
void i915_graphics_info_get(struct i915_graphics_info *out);
void i915_graphics_stats_get(struct i915_graphics_stats *out);
void i915_graphics_vga_enable(uint8_t enable);
void i915_graphics_ring_flush(void);

int i915_ide_init(void);
int i915_ide_is_ready(void);
int i915_ide_identify(uint8_t channel, uint8_t drive, uint16_t *buffer);
int i915_ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer);
int i915_ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer);
int i915_ide_dma_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer);
int i915_ide_dma_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer);
int i915_ide_flush(uint8_t channel);
void i915_ide_reset(uint8_t channel);
int i915_ide_set_dma(uint8_t channel, uint8_t drive, uint8_t mode);
uint8_t i915_ide_get_udma_mode(uint8_t channel);
void i915_ide_stats_get(struct i915_ide_stats *out);

int i915_sata_init(void);
int i915_sata_is_ready(void);
uint8_t i915_sata_port_count(void);
int i915_sata_read_sectors(uint8_t port, uint32_t lba, uint8_t count, void *buffer);
int i915_sata_write_sectors(uint8_t port, uint32_t lba, uint8_t count, const void *buffer);
int i915_sata_set_mode(uint8_t port, uint8_t ahci_mode);

int i915_usb_init(void);
int i915_usb_is_ready(void);
uint8_t i915_usb_root_port_count(void);
int i915_usb_port_reset(uint8_t port);
int i915_usb_port_power(uint8_t port, uint8_t on);
uint32_t i915_usb_port_status(uint8_t port);
int i915_usb_control_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data);
int i915_usb_bulk_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint32_t length, void *data);
void i915_usb_stats_get(struct i915_usb_stats *out);

int i915_audio_init(void);
int i915_audio_is_ready(void);
int i915_audio_codec_read(uint8_t reg, uint16_t *value);
int i915_audio_codec_write(uint8_t reg, uint16_t value);
int i915_audio_codec_reset(int cold);
int i915_audio_set_volume(uint8_t channel, uint8_t percent);
uint8_t i915_audio_get_volume(uint8_t channel);
int i915_audio_playback_start(const void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits);
int i915_audio_playback_stop(void);
int i915_audio_capture_start(void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits);
int i915_audio_capture_stop(void);
void i915_audio_stats_get(struct i915_audio_stats *out);

int i915_smbus_init(void);
int i915_smbus_read_byte(uint8_t device, uint8_t offset, uint8_t *value);
int i915_smbus_write_byte(uint8_t device, uint8_t offset, uint8_t value);
int i915_smbus_read_word(uint8_t device, uint8_t offset, uint16_t *value);
int i915_smbus_write_word(uint8_t device, uint8_t offset, uint16_t value);
int i915_smbus_spd_read(uint8_t offset, uint8_t *value);
int i915_smbus_spd_write(uint8_t offset, uint8_t value);
void i915_smbus_stats_get(void);

int i915_acpi_available(void);
void i915_acpi_sleep(uint8_t state);
void i915_acpi_reboot(void);
void i915_acpi_shutdown(void);
int i915_acpi_sci_enable(uint8_t enable);
uint16_t i915_acpi_read_pm1a_cnt(void);
void i915_acpi_write_pm1a_cnt(uint16_t value);

int i915_tco_init(void);
int i915_tco_is_ready(void);
int i915_tco_watchdog_start(uint32_t seconds);
int i915_tco_watchdog_feed(void);
int i915_tco_watchdog_stop(void);
int i915_tco_irq_status(void);

int i915_gpio_read(uint8_t pin, uint8_t *value);
int i915_gpio_write(uint8_t pin, uint8_t value);
int i915_gpio_direction(uint8_t pin, uint8_t output);

int i915_pirq_read_router(uint8_t pirq, uint8_t *route);
int i915_pirq_write_router(uint8_t pirq, uint8_t route);
int i915_smi_enable(uint8_t enable);
uint8_t i915_smi_status(void);

int i915_rtc_init(void);
int i915_rtc_is_ready(void);
int i915_rtc_get_time(struct i915_rtc_time *out);
int i915_rtc_set_time(const struct i915_rtc_time *t);
int i915_rtc_nvram_read(uint8_t offset, uint8_t *value);
int i915_rtc_nvram_write(uint8_t offset, uint8_t value);
int i915_rtc_alarm_set(const struct i915_rtc_time *t);
int i915_rtc_irq_enable(uint8_t mask, uint8_t enable);
uint8_t i915_rtc_irq_status(void);

int i915_fwh_read(uint32_t offset, uint8_t *value);
int i915_fwh_write(uint32_t offset, uint8_t value);
int i915_bios_lock(void);
int i915_bios_unlock(void);

int i915_hwmon_init(void);
int i915_hwmon_is_ready(void);
int i915_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius);
int i915_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt);
int i915_hwmon_read_fan(uint8_t fan, uint32_t *rpm);
void i915_hwmon_stats_get(struct i915_hwmon_stats *out);

void i915_irq_handler(uint8_t irq);
int i915_irq_register(uint8_t irq, void (*handler)(uint8_t irq));
int i915_irq_unregister(uint8_t irq);

#ifdef __cplusplus
}
#endif

#endif
