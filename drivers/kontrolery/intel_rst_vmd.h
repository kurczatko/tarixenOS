#ifndef INTEL_RST_VMD_H
#define INTEL_RST_VMD_H

#include <include/printf/types.h>

#define INTEL_RST_VMD_VENDOR_ID      0x8086U
#define INTEL_RST_VMD_DEVICE_ID_VMD1 0x5060U
#define INTEL_RST_VMD_DEVICE_ID_VMD2 0x5062U
#define INTEL_RST_VMD_DEVICE_ID_VMD3 0x5063U
#define INTEL_RST_VMD_DEVICE_ID_VMD4 0x5065U
#define INTEL_RST_VMD_DEVICE_ID_VMD5 0x7A02U
#define INTEL_RST_VMD_DEVICE_ID_VMD6 0x7A03U
#define INTEL_RST_VMD_DEVICE_ID_VMD7 0x7A08U

#define INTEL_RST_VMD_MAX_CONTROLLERS  4U
#define INTEL_RST_VMD_MAX_DEVICES      16U
#define INTEL_RST_VMD_MAX_VOLUMES      8U
#define INTEL_RST_VMD_MAX_PORTS        8U

#define VMD_REG_VERSION          0x00U
#define VMD_REG_CAPABILITY       0x04U
#define VMD_REG_CONTROL          0x08U
#define VMD_REG_STATUS           0x0CU
#define VMD_REG_INT_STATUS       0x10U
#define VMD_REG_INT_MASK         0x14U
#define VMD_REG_CFG_BASE         0x20U
#define VMD_REG_CFG_LIMIT        0x24U
#define VMD_REG_BAR0             0x30U
#define VMD_REG_BAR1             0x34U
#define VMD_REG_DEV_EN           0x40U
#define VMD_REG_DEV_DIS          0x44U
#define VMD_REG_HOT_PLUG         0x48U
#define VMD_REG_LINK_STATUS      0x4CU
#define VMD_REG_POWER_STATE      0x50U
#define VMD_REG_TEMP_SENSOR      0x54U
#define VMD_REG_THROTTLE         0x58U
#define VMD_REG_ERR_INJ          0x5CU
#define VMD_REG_ERR_STATUS       0x60U
#define VMD_REG_ERR_MASK         0x64U
#define VMD_REG_SMBUS            0x70U
#define VMD_REG_GPIO             0x74U
#define VMD_REG_FW_VERSION       0x80U
#define VMD_REG_FW_STATUS        0x84U
#define VMD_REG_FW_UPDATE        0x88U
#define VMD_REG_VOL_BASE         0x100U
#define VMD_REG_VOL_STRIDE       0x10U
#define VMD_REG_PORT_BASE        0x200U
#define VMD_REG_PORT_STRIDE      0x08U
#define VMD_REG_NVME_BASE        0x300U
#define VMD_REG_NVME_STRIDE      0x100U

#define VMD_CAP_VMD_TYPE_SHIFT     0U
#define VMD_CAP_VMD_TYPE_MASK      0x0000000FU
#define VMD_CAP_PORT_COUNT_SHIFT   4U
#define VMD_CAP_PORT_COUNT_MASK    0x000000F0U
#define VMD_CAP_NVME_SUPPORT       (1U << 8U)
#define VMD_CAP_RAID_SUPPORT       (1U << 9U)
#define VMD_CAP_HOT_PLUG           (1U << 10U)
#define VMD_CAP_POWER_MGMT         (1U << 11U)
#define VMD_CAP_TEMP_MONITOR       (1U << 12U)

#define VMD_CTL_ENABLE           (1U << 0U)
#define VMD_CTL_DISABLE          (1U << 1U)
#define VMD_CTL_HOT_PLUG_EN      (1U << 2U)
#define VMD_CTL_RAID_EN          (1U << 3U)
#define VMD_CTL_POWER_MGMT_EN    (1U << 4U)
#define VMD_CTL_TEMP_MON_EN      (1U << 5U)
#define VMD_CTL_RESET            (1U << 31U)

#define VMD_STS_READY            (1U << 0U)
#define VMD_STS_HOT_PLUG_EVENT   (1U << 1U)
#define VMD_STS_POWER_CHANGE     (1U << 2U)
#define VMD_STS_TEMP_WARNING     (1U << 3U)
#define VMD_STS_TEMP_CRITICAL    (1U << 4U)
#define VMD_STS_ERROR            (1U << 7U)

#define VMD_INT_PORT_CHANGE      (1U << 0U)
#define VMD_INT_VOL_CHANGE       (1U << 1U)
#define VMD_INT_THRESHOLD        (1U << 2U)
#define VMD_INT_ERROR            (1U << 7U)

#define VMD_PWR_D0               0U
#define VMD_PWR_D1               1U
#define VMD_PWR_D2               2U
#define VMD_PWR_D3               3U

#define VMD_PORT_PRESENT         (1U << 0U)
#define VMD_PORT_LINK_UP         (1U << 1U)
#define VMD_PORT_NVME            (1U << 2U)
#define VMD_PORT_HP_CAPABLE      (1U << 3U)

#define VMD_VOL_TYPE_RAID0       0U
#define VMD_VOL_TYPE_RAID1       1U
#define VMD_VOL_TYPE_RAID5       2U
#define VMD_VOL_TYPE_RAID10      3U
#define VMD_VOL_TYPE_SINGLE      4U

#define VMD_VOL_STATE_ONLINE     0U
#define VMD_VOL_STATE_DEGRADED   1U
#define VMD_VOL_STATE_FAILED     2U
#define VMD_VOL_STATE_REBUILDING 3U

struct rst_vmd_controller {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t bar0;
    volatile uint8_t *mmio;
    uint8_t initialized;
    uint8_t enabled;
    uint8_t port_count;
    uint8_t nvme_support;
    uint8_t raid_support;
    uint8_t hot_plug;
    uint8_t power_mgmt;
    uint8_t temp_monitor;
    uint8_t power_state;
    uint32_t cap_reg;
    uint32_t fw_version;
    int32_t temperature_c;
};

struct rst_vmd_device {
    uint8_t controller_index;
    uint8_t port_index;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t present;
    uint8_t link_up;
    uint8_t nvme;
    uint8_t nvme_nsid;
    uint64_t capacity_bytes;
    uint32_t block_size;
    uint32_t temperature_c;
};

struct rst_vmd_volume {
    uint8_t volume_id;
    uint8_t volume_type;
    uint8_t volume_state;
    uint8_t member_count;
    uint8_t member_ports[INTEL_RST_VMD_MAX_PORTS];
    uint64_t capacity_bytes;
    uint32_t block_size;
    uint32_t progress_percent;
};

struct rst_vmd_stats {
    uint32_t controllers_found;
    uint32_t controllers_init;
    uint32_t devices_found;
    uint32_t devices_online;
    uint32_t volumes_created;
    uint32_t volumes_online;
    uint32_t hot_plug_events;
    uint32_t power_changes;
    uint32_t temp_warnings;
    uint32_t temp_critical;
    uint32_t errors;
    uint32_t resets;
};

#ifdef __cplusplus
class IntelRSTVMD {
public:
    static IntelRSTVMD* instance();
    void set_mmio_base(uint8_t index, volatile uint8_t *base);
    uint8_t is_ready(void);

    IntelRSTVMD();
    ~IntelRSTVMD();

    uint8_t probe(uint16_t vendor, uint16_t device);
    void init(void);
    void deinit(void);
    void poll(void);

    uint8_t controller_count(void);
    int controller_info(uint8_t index, struct rst_vmd_controller *out);
    uint8_t device_count(uint8_t controller_index);
    int device_info(uint8_t controller_index, uint8_t device_index, struct rst_vmd_device *out);
    uint8_t volume_count(uint8_t controller_index);
    int volume_info(uint8_t controller_index, uint8_t volume_index, struct rst_vmd_volume *out);

    int port_enable(uint8_t controller_index, uint8_t port_index);
    int port_disable(uint8_t controller_index, uint8_t port_index);
    int port_rescan(uint8_t controller_index);

    int volume_create(uint8_t controller_index, uint8_t volume_type, const uint8_t *member_ports, uint8_t member_count);
    int volume_delete(uint8_t controller_index, uint8_t volume_id);
    int volume_rebuild(uint8_t controller_index, uint8_t volume_id);

    int power_set_state(uint8_t controller_index, uint8_t power_state);
    int power_get_state(uint8_t controller_index, uint8_t *power_state);
    int temperature_read(uint8_t controller_index, int32_t *temperature_c);
    int reset_controller(uint8_t controller_index);

    void stats_get(struct rst_vmd_stats *out);
    void stats_reset(void);
    void debug_print_controllers(void);

private:
    int find_controller(uint8_t bus, uint8_t slot, uint8_t func);
    void scan_controllers(void);
    void init_controller(uint8_t index);
    void read_controller_regs(uint8_t index);
    void scan_devices(uint8_t index);
    void scan_volumes(uint8_t index);
    void update_temperature(uint8_t index);
    void handle_hot_plug(uint8_t index);
    void process_interrupts(uint8_t index);
    uint32_t mmio_read32(uint8_t index, uint32_t offset);
    void mmio_write32(uint8_t index, uint32_t offset, uint32_t value);
    void enable_controller(uint8_t index);
    void disable_controller(uint8_t index);
    void configure_ports(uint8_t index);

    static IntelRSTVMD* g_instance;
    struct rst_vmd_controller controllers[INTEL_RST_VMD_MAX_CONTROLLERS];
    uint8_t controller_count_val;
    uint8_t initialized;
    struct rst_vmd_device devices[INTEL_RST_VMD_MAX_CONTROLLERS][INTEL_RST_VMD_MAX_DEVICES];
    uint8_t device_counts[INTEL_RST_VMD_MAX_CONTROLLERS];
    struct rst_vmd_volume volumes[INTEL_RST_VMD_MAX_CONTROLLERS][INTEL_RST_VMD_MAX_VOLUMES];
    uint8_t volume_counts[INTEL_RST_VMD_MAX_CONTROLLERS];
    struct rst_vmd_stats stats;
};

extern "C" {
#endif

int intel_rst_vmd_init(void);
void intel_rst_vmd_shutdown(void);
void intel_rst_vmd_poll(void);

#ifdef __cplusplus
}
#endif

#endif
