#ifndef ASUS_P4P800_H
#define ASUS_P4P800_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define P4P_PCI_CONFIG_ADDRESS 0xCF8U
#define P4P_PCI_CONFIG_DATA 0xCFCU
#define P4P_PCI_REG_VENDOR_DEVICE 0x00U
#define P4P_PCI_REG_COMMAND 0x04U
#define P4P_PCI_REG_REVISION 0x08U
#define P4P_PCI_REG_BAR0 0x10U
#define P4P_PCI_REG_BAR4 0x18U
#define P4P_PCI_REG_INTERRUPT_LINE 0x3CU

#define P4P_VENDOR_INTEL 0x8086U
#define P4P_DEVICE_MCH_I848P 0x2530U
#define P4P_DEVICE_ICH4_LPC 0x24C0U
#define P4P_DEVICE_ICH4_AC97 0x24C5U
#define P4P_DEVICE_ICH4_USB1 0x24C2U
#define P4P_DEVICE_ICH4_USB2 0x24C4U
#define P4P_DEVICE_ICH4_USB3 0x24C7U
#define P4P_DEVICE_ICH4_EHCI 0x24CDU
#define P4P_DEVICE_ICH4_IDE 0x24C1U
#define P4P_DEVICE_ICH4_SMBUS 0x24C3U
#define P4P_DEVICE_ETH_82562ET 0x1050U

#define P4P_ICH4_ACPI_CNT_OFFSET 0x40U
#define P4P_ACPI_PM1A_CNT 0x00U
#define P4P_ACPI_PM1A_EVT 0x04U
#define P4P_ACPI_SLP_EN 0x2000U
#define P4P_ACPI_SCI_EN 0x0001U

#define P4P_AC97_MIXER_BAR_OFFSET 0x14U
#define P4P_AC97_BM_BAR_OFFSET 0x18U
#define P4P_AC97_REG_RESET 0x00U
#define P4P_AC97_REG_MASTER 0x02U
#define P4P_AC97_REG_MASTER_MONO 0x0AU
#define P4P_AC97_REG_PCM_OUT 0x18U
#define P4P_AC97_REG_PCM_IN 0x1CU
#define P4P_AC97_REG_RECORD_SEL 0x1AU
#define P4P_AC97_REG_GENERAL_PURPOSE 0x32U
#define P4P_AC97_REG_POWER 0x34U
#define P4P_AC97_REG_VENDOR_ID1 0x7CU
#define P4P_AC97_CODEC_AD1985 0x11DU

#define P4P_ETH_CSR_PORT 0x00U
#define P4P_ETH_CSR_SCB 0x08U
#define P4P_ETH_CSR_SCB_GENERAL 0x0CU
#define P4P_ETH_CSR_SCB_CMD_HI 0x0EU
#define P4P_ETH_CSR_SCB_CMD_LO 0x10U
#define P4P_ETH_CSR_FLASH 0x14U
#define P4P_ETH_CSR_EEPROM 0x14U
#define P4P_ETH_CSR_MDI 0x18U
#define P4P_ETH_CSR_DUMP 0x1CU
#define P4P_ETH_CSR_MISC 0x20U
#define P4P_ETH_CSR_DIAG 0x24U
#define P4P_ETH_SCB_CMD_IACK 0x1000U
#define P4P_ETH_SCB_CMD_CU_START 0x0100U
#define P4P_ETH_SCB_CMD_RU_START 0x0010U
#define P4P_ETH_SCB_CMD_RU_ABORT 0x0040U
#define P4P_ETH_CSR_PORT_SW_RESET 0x00000000U
#define P4P_ETH_CSR_PORT_SEL_RESET 0x00004000U

#define P4P_IDE_REG_DATA 0x00U
#define P4P_IDE_REG_FEATURES 0x01U
#define P4P_IDE_REG_SECCOUNT 0x02U
#define P4P_IDE_REG_LBA0 0x03U
#define P4P_IDE_REG_LBA1 0x04U
#define P4P_IDE_REG_LBA2 0x05U
#define P4P_IDE_REG_LBA3 0x06U
#define P4P_IDE_REG_DRIVE_HEAD 0x06U
#define P4P_IDE_REG_COMMAND 0x07U
#define P4P_IDE_REG_ALTSTATUS 0x206U
#define P4P_IDE_CMD_IDENTIFY 0xECU
#define P4P_IDE_CMD_READ 0x20U
#define P4P_IDE_CMD_WRITE 0x30U
#define P4P_IDE_CMD_READ_DMA 0xC8U
#define P4P_IDE_CMD_WRITE_DMA 0xCAU
#define P4P_IDE_BM_REG_CMD 0x00U
#define P4P_IDE_BM_REG_STATUS 0x02U
#define P4P_IDE_BM_REG_PRDT 0x04U
#define P4P_IDE_BM_CMD_START 0x01U
#define P4P_IDE_BM_CMD_READ 0x08U

#define P4P_FDC_REG_STATUS_A 0x00U
#define P4P_FDC_REG_STATUS_B 0x01U
#define P4P_FDC_REG_DOR 0x02U
#define P4P_FDC_REG_TDR 0x04U
#define P4P_FDC_REG_MSR 0x04U
#define P4P_FDC_REG_FIFO 0x05U
#define P4P_FDC_REG_CCR 0x07U
#define P4P_FDC_REG_CMD 0x05U
#define P4P_FDC_CMD_SPECIFY 0x03U
#define P4P_FDC_CMD_CALIBRATE 0x07U
#define P4P_FDC_CMD_CHECK_STATUS 0x08U
#define P4P_FDC_CMD_SEEK 0x0FU
#define P4P_FDC_CMD_READ 0xE6U
#define P4P_FDC_CMD_WRITE 0xC5U
#define P4P_FDC_DOR_RESET 0x04U
#define P4P_FDC_DOR_DMA 0x08U
#define P4P_FDC_DOR_DRIVE0 0x00U

#define P4P_RTC_INDEX 0x70U
#define P4P_RTC_DATA 0x71U
#define P4P_RTC_REG_SECONDS 0x00U
#define P4P_RTC_REG_ALARM_SECONDS 0x01U
#define P4P_RTC_REG_MINUTES 0x02U
#define P4P_RTC_REG_ALARM_MINUTES 0x03U
#define P4P_RTC_REG_HOURS 0x04U
#define P4P_RTC_REG_ALARM_HOURS 0x05U
#define P4P_RTC_REG_DAY_OF_WEEK 0x06U
#define P4P_RTC_REG_DAY_OF_MONTH 0x07U
#define P4P_RTC_REG_MONTH 0x08U
#define P4P_RTC_REG_YEAR 0x09U
#define P4P_RTC_REG_STATUS_A 0x0AU
#define P4P_RTC_REG_STATUS_B 0x0BU
#define P4P_RTC_REG_STATUS_C 0x0CU
#define P4P_RTC_REG_DIAGNOSTIC_STATUS 0x0DU
#define P4P_RTC_STATUS_B_24H 0x02U
#define P4P_RTC_STATUS_B_DM_BINARY 0x04U
#define P4P_RTC_STATUS_B_UPDATE_IRQ 0x10U
#define P4P_RTC_STATUS_B_ALARM_IRQ 0x20U
#define P4P_RTC_STATUS_B_PERIODIC_IRQ 0x40U

#define P4P_SIO_INDEX 0x2EU
#define P4P_SIO_DATA 0x2FU
#define P4P_SIO_KEY_ENTER 0x87U
#define P4P_SIO_KEY_EXIT 0xAAU
#define P4P_SIO_REG_CHIP_ID 0x20U
#define P4P_SIO_REG_CONFIG 0x2EU
#define P4P_SIO_LDN_HWMON 0x0BU

#define P4P_UHC_REG_USBCMD 0x00U
#define P4P_UHC_REG_USBSTS 0x02U
#define P4P_UHC_REG_USBINTR 0x04U
#define P4P_UHC_REG_FRNUM 0x06U
#define P4P_UHC_REG_BASEADD 0x08U
#define P4P_UHC_REG_SOFMOD 0x0CU
#define P4P_UHC_REG_PORTSC1 0x10U
#define P4P_UHC_REG_PORTSC2 0x12U

#define P4P_AGP_REG_AGPBASE 0x00U

struct p4p_rtc_time {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day_of_month;
    uint8_t month;
    uint8_t year;
};

struct p4p_audio_stats {
    uint32_t codec_reads;
    uint32_t codec_writes;
    uint32_t playback_bytes;
    uint32_t capture_bytes;
    uint32_t errors;
};

struct p4p_eth_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t resets;
};

struct p4p_ide_stats {
    uint32_t reads;
    uint32_t writes;
    uint32_t sectors_read;
    uint32_t sectors_written;
    uint32_t errors;
    uint32_t dma_transfers;
};

struct p4p_usb_stats {
    uint32_t ports;
    uint32_t resets;
    uint32_t transfers;
    uint32_t errors;
};

struct p4p_floppy_stats {
    uint32_t reads;
    uint32_t writes;
    uint32_t seeks;
    uint32_t errors;
};

struct p4p_hwmon_stats {
    uint32_t temperatures;
    uint32_t voltages;
    uint32_t fans;
    uint32_t errors;
};

struct p4p_info {
    uint16_t vendor_id;
    uint16_t mch_device_id;
    uint16_t ich_device_id;
    uint8_t mch_revision;
    uint8_t ich_revision;
    uint32_t acpi_base;
    uint32_t smbus_base;
    uint32_t ac97_base;
    uint32_t ide_base;
    uint32_t bm_ide_base;
    uint32_t uhci_base[3];
    uint32_t ehci_base;
    uint32_t eth_base;
    uint32_t floppy_base;
    uint32_t sio_base;
    uint32_t agp_aperture;
    uint8_t acpi_available;
    uint8_t agp_available;
    uint8_t audio_ready;
    uint8_t eth_ready;
    uint8_t ide_ready;
    uint8_t usb_ready;
    uint8_t floppy_ready;
    uint8_t rtc_ready;
    uint8_t hwmon_ready;
    uint8_t initialized;
};

struct p4p_stats {
    struct p4p_audio_stats audio;
    struct p4p_eth_stats eth;
    struct p4p_ide_stats ide;
    struct p4p_usb_stats usb;
    struct p4p_floppy_stats floppy;
    struct p4p_hwmon_stats hwmon;
    uint32_t pci_reads;
    uint32_t pci_writes;
    uint32_t irq_dispatched;
    uint32_t init_count;
};

int p4p_init(void);
int p4p_is_ready(void);
void p4p_shutdown(void);
void p4p_reset(void);
uint32_t p4p_get_chipset_id(void);
void p4p_info_get(struct p4p_info *out);
void p4p_stats_get(struct p4p_stats *out);

int p4p_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func);
uint32_t p4p_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void p4p_pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
uint16_t p4p_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void p4p_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
uint8_t p4p_pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void p4p_pci_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value);
int p4p_pci_set_bus_master(uint8_t bus, uint8_t slot, uint8_t func, uint8_t enable);

int p4p_chipset_probe(void);
void p4p_chipset_configure_memory(void);

int p4p_acpi_available(void);
void p4p_acpi_sleep(uint8_t state);
void p4p_acpi_reboot(void);
void p4p_acpi_shutdown(void);

int p4p_smbus_init(void);
int p4p_smbus_read_byte(uint8_t device, uint8_t offset, uint8_t *value);
int p4p_smbus_write_byte(uint8_t device, uint8_t offset, uint8_t value);

int p4p_agp_init(void);
uint32_t p4p_agp_aperture(void);
int p4p_agp_bind(uint32_t gatt_index, uint32_t physical_addr);
int p4p_agp_unbind(uint32_t gatt_index);

int p4p_audio_init(void);
int p4p_audio_is_ready(void);
int p4p_audio_codec_read(uint8_t reg, uint16_t *value);
int p4p_audio_codec_write(uint8_t reg, uint16_t value);
int p4p_audio_codec_reset(int cold);
int p4p_audio_set_volume(uint8_t channel, uint8_t percent);
uint8_t p4p_audio_get_volume(uint8_t channel);
int p4p_audio_playback_start(const void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits);
int p4p_audio_playback_stop(void);
int p4p_audio_capture_start(void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits);
int p4p_audio_capture_stop(void);
void p4p_audio_stats_get(struct p4p_audio_stats *out);

int p4p_eth_init(void);
int p4p_eth_is_ready(void);
int p4p_eth_get_mac(uint8_t mac[6]);
int p4p_eth_link_up(void);
uint32_t p4p_eth_speed_mbps(void);
int p4p_eth_send(const void *frame, uint16_t length);
int p4p_eth_receive(void *frame, uint16_t capacity);
void p4p_eth_stats_get(struct p4p_eth_stats *out);

int p4p_ide_init(void);
int p4p_ide_is_ready(void);
int p4p_ide_identify(uint8_t channel, uint8_t drive, uint16_t *buffer);
int p4p_ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer);
int p4p_ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer);
int p4p_ide_dma_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer);
int p4p_ide_dma_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer);
void p4p_ide_reset(uint8_t channel);
void p4p_ide_stats_get(struct p4p_ide_stats *out);

int p4p_usb_init(void);
int p4p_usb_is_ready(void);
uint8_t p4p_usb_root_port_count(void);
int p4p_usb_port_reset(uint8_t port);
int p4p_usb_port_power(uint8_t port, uint8_t on);
uint32_t p4p_usb_port_status(uint8_t port);
int p4p_usb_control_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data);
int p4p_usb_bulk_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint32_t length, void *data);
void p4p_usb_stats_get(struct p4p_usb_stats *out);

int p4p_floppy_init(void);
int p4p_floppy_is_ready(void);
int p4p_floppy_recalibrate(uint8_t drive);
int p4p_floppy_seek(uint8_t drive, uint8_t track);
int p4p_floppy_read_sectors(uint8_t drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t count, void *buffer);
int p4p_floppy_write_sectors(uint8_t drive, uint8_t head, uint8_t track, uint8_t sector, uint8_t count, const void *buffer);
uint8_t p4p_floppy_status(void);
void p4p_floppy_stats_get(struct p4p_floppy_stats *out);

int p4p_rtc_init(void);
int p4p_rtc_is_ready(void);
int p4p_rtc_get_time(struct p4p_rtc_time *out);
int p4p_rtc_set_time(const struct p4p_rtc_time *t);
int p4p_rtc_nvram_read(uint8_t offset, uint8_t *value);
int p4p_rtc_nvram_write(uint8_t offset, uint8_t value);
int p4p_rtc_alarm_set(const struct p4p_rtc_time *t);
int p4p_rtc_irq_enable(uint8_t mask, uint8_t enable);
uint8_t p4p_rtc_irq_status(void);

int p4p_hwmon_init(void);
int p4p_hwmon_is_ready(void);
int p4p_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius);
int p4p_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt);
int p4p_hwmon_read_fan(uint8_t fan, uint32_t *rpm);
void p4p_hwmon_stats_get(struct p4p_hwmon_stats *out);

void p4p_irq_handler(uint8_t irq);
int p4p_irq_register(uint8_t irq, void (*handler)(uint8_t irq));

#ifdef __cplusplus
}
#endif

#endif
