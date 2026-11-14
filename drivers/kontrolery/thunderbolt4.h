#ifndef THUNDERBOLT4_H
#define THUNDERBOLT4_H

#include <include/printf/types.h>

#define TB4_VENDOR_ID_INTEL  0x8086U
#define TB4_DEVICE_ID_TBT4   0x15D8U
#define TB4_DEVICE_ID_USB32  0x43EDU
#define TB4_DEVICE_ID_ALPINE 0x1579U

#define TB4_MAX_CONTROLLERS   4U
#define TB4_MAX_PORTS         8U
#define TB4_MAX_DEVICES       16U
#define TB4_MAX_DOMAINS       4U

#define TB4_REG_VENDOR_ID     0x00U
#define TB4_REG_DEVICE_ID     0x02U
#define TB4_REG_COMMAND       0x04U
#define TB4_REG_STATUS        0x06U
#define TB4_REG_REVISION      0x08U
#define TB4_REG_CLASS         0x09U
#define TB4_REG_BAR0          0x10U
#define TB4_REG_BAR1          0x14U
#define TB4_REG_INT_LINE      0x3CU
#define TB4_REG_INT_PIN       0x3DU

#define TB4_REG_SW_VERSION    0x100U
#define TB4_REG_SW_STATUS     0x104U
#define TB4_REG_SW_CONTROL    0x108U
#define TB4_REG_SW_INT_STATUS 0x10CU
#define TB4_REG_SW_INT_MASK   0x110U
#define TB4_REG_PORT_BASE     0x200U
#define TB4_REG_PORT_STRIDE   0x20U
#define TB4_REG_DEVICE_BASE   0x400U
#define TB4_REG_DEVICE_STRIDE 0x40U
#define TB4_REG_DOMAIN_BASE   0x600U
#define TB4_REG_DOMAIN_STRIDE 0x80U
#define TB4_REG_POWER_STATE   0x800U
#define TB4_REG_TEMP_SENSOR   0x804U
#define TB4_REG_ERR_STATUS    0x808U
#define TB4_REG_ERR_MASK      0x80CU

#define TB4_SW_STATUS_READY        (1U << 0U)
#define TB4_SW_STATUS_LINK_UP      (1U << 1U)
#define TB4_SW_STATUS_DEVICE_CONN  (1U << 2U)
#define TB4_SW_STATUS_ERROR        (1U << 7U)

#define TB4_SW_CONTROL_ENABLE      (1U << 0U)
#define TB4_SW_CONTROL_DISABLE     (1U << 1U)
#define TB4_SW_CONTROL_RESET       (1U << 2U)
#define TB4_SW_CONTROL_POWER_MGMT  (1U << 3U)
#define TB4_SW_CONTROL_SECURITY    (1U << 4U)

#define TB4_INT_PORT_HOT_PLUG    (1U << 0U)
#define TB4_INT_PORT_LINK_CHANGE (1U << 1U)
#define TB4_INT_DEVICE_REMOVED   (1U << 2U)
#define TB4_INT_DOMAIN_CHANGE    (1U << 3U)
#define TB4_INT_ERROR            (1U << 7U)

#define TB4_PORT_TYPE_USB_C      0U
#define TB4_PORT_TYPE_TBT4       1U
#define TB4_PORT_TYPE_DP         2U
#define TB4_PORT_TYPE_PCIE       3U

#define TB4_PORT_STATE_DISCONNECTED 0U
#define TB4_PORT_STATE_CONNECTED    1U
#define TB4_PORT_STATE_CONFIGURED   2U
#define TB4_PORT_STATE_ACTIVE       3U

#define TB4_DEVICE_TYPE_STORAGE   0U
#define TB4_DEVICE_TYPE_DISPLAY   1U
#define TB4_DEVICE_TYPE_NETWORK   2U
#define TB4_DEVICE_TYPE_DOCK      3U
#define TB4_DEVICE_TYPE_HOST      4U

#define TB4_POWER_D0 0U
#define TB4_POWER_D1 1U
#define TB4_POWER_D2 2U
#define TB4_POWER_D3 3U

#define TB4_SECURITY_NONE     0U
#define TB4_SECURITY_USER     1U
#define TB4_SECURITY_SECURE   2U
#define TB4_SECURITY_DPONLY   3U

struct tb4_controller {
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
    uint8_t usb32_support;
    uint8_t tbt4_support;
    uint8_t dp_alt_mode;
    uint8_t pcie_tunneling;
    uint8_t power_state;
    uint8_t security_level;
    uint32_t sw_version;
    int32_t temperature_c;
};

struct tb4_port {
    uint8_t controller_index;
    uint8_t port_index;
    uint8_t port_type;
    uint8_t port_state;
    uint8_t connected;
    uint8_t link_up;
    uint8_t usb32_active;
    uint8_t tbt4_active;
    uint32_t speed_mbps;
    uint8_t device_attached;
};

struct tb4_device {
    uint8_t controller_index;
    uint8_t port_index;
    uint8_t device_index;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t device_type;
    uint8_t connected;
    uint8_t configured;
    uint8_t authorized;
    uint64_t bandwidth_mbps;
    uint8_t nvme_tunnel;
    uint8_t display_tunnel;
};

struct tb4_domain {
    uint8_t domain_id;
    uint8_t domain_state;
    uint8_t member_count;
    uint8_t member_ports[TB4_MAX_PORTS];
    uint8_t domain_master;
};

struct tb4_stats {
    uint32_t controllers_found;
    uint32_t controllers_init;
    uint32_t ports_configured;
    uint32_t devices_enumerated;
    uint32_t devices_authorized;
    uint32_t hot_plug_events;
    uint32_t domain_changes;
    uint32_t power_changes;
    uint32_t temp_warnings;
    uint32_t temp_critical;
    uint32_t errors;
    uint32_t resets;
    uint32_t usb32_transfers;
    uint32_t tbt4_transfers;
};

#ifndef __cplusplus
int thunderbolt4_init(void);
void thunderbolt4_shutdown(void);
void thunderbolt4_poll(void);
int thunderbolt4_is_ready(void);

int thunderbolt4_controller_count(void);
int thunderbolt4_controller_info(uint8_t index, struct tb4_controller *out);
int thunderbolt4_port_count(uint8_t controller_index);
int thunderbolt4_port_info(uint8_t controller_index, uint8_t port_index, struct tb4_port *out);
int thunderbolt4_device_count(uint8_t controller_index);
int thunderbolt4_device_info(uint8_t controller_index, uint8_t device_index, struct tb4_device *out);
int thunderbolt4_domain_count(uint8_t controller_index);
int thunderbolt4_domain_info(uint8_t controller_index, uint8_t domain_index, struct tb4_domain *out);

int thunderbolt4_port_enable(uint8_t controller_index, uint8_t port_index);
int thunderbolt4_port_disable(uint8_t controller_index, uint8_t port_index);
int thunderbolt4_port_rescan(uint8_t controller_index);

int thunderbolt4_device_authorize(uint8_t controller_index, uint8_t device_index);
int thunderbolt4_device_deauthorize(uint8_t controller_index, uint8_t device_index);
int thunderbolt4_device_config(uint8_t controller_index, uint8_t device_index);

int thunderbolt4_domain_create(uint8_t controller_index, uint8_t *member_ports, uint8_t member_count);
int thunderbolt4_domain_delete(uint8_t controller_index, uint8_t domain_id);

int thunderbolt4_power_set_state(uint8_t controller_index, uint8_t power_state);
int thunderbolt4_power_get_state(uint8_t controller_index, uint8_t *power_state);
int thunderbolt4_temperature_read(uint8_t controller_index, int32_t *temperature_c);
int thunderbolt4_security_set_level(uint8_t controller_index, uint8_t level);
int thunderbolt4_security_get_level(uint8_t controller_index, uint8_t *level);
int thunderbolt4_reset(uint8_t controller_index);

void thunderbolt4_stats_get(struct tb4_stats *out);
void thunderbolt4_stats_reset(void);
#endif

#ifdef __cplusplus
class Thunderbolt4 {
public:
    static Thunderbolt4* instance();
    void set_mmio_base(uint8_t index, volatile uint8_t *base);
    uint8_t is_ready(void);

    Thunderbolt4();
    ~Thunderbolt4();

    uint8_t probe(uint16_t vendor, uint16_t device);
    void init(void);
    void deinit(void);
    void poll(void);

    uint8_t controller_count(void);
    int controller_info(uint8_t index, struct tb4_controller *out);
    uint8_t port_count(uint8_t controller_index);
    int port_info(uint8_t controller_index, uint8_t port_index, struct tb4_port *out);
    uint8_t device_count(uint8_t controller_index);
    int device_info(uint8_t controller_index, uint8_t device_index, struct tb4_device *out);
    uint8_t domain_count(uint8_t controller_index);
    int domain_info(uint8_t controller_index, uint8_t domain_index, struct tb4_domain *out);

    int port_enable(uint8_t controller_index, uint8_t port_index);
    int port_disable(uint8_t controller_index, uint8_t port_index);
    int port_rescan(uint8_t controller_index);

    int device_authorize(uint8_t controller_index, uint8_t device_index);
    int device_deauthorize(uint8_t controller_index, uint8_t device_index);
    int device_config(uint8_t controller_index, uint8_t device_index);

    int domain_create(uint8_t controller_index, uint8_t *member_ports, uint8_t member_count);
    int domain_delete(uint8_t controller_index, uint8_t domain_id);

    int power_set_state(uint8_t controller_index, uint8_t power_state);
    int power_get_state(uint8_t controller_index, uint8_t *power_state);
    int temperature_read(uint8_t controller_index, int32_t *temperature_c);
    int security_set_level(uint8_t controller_index, uint8_t level);
    int security_get_level(uint8_t controller_index, uint8_t *level);
    int reset_controller(uint8_t controller_index);

    void stats_get(struct tb4_stats *out);
    void stats_reset(void);
    void debug_print_controllers(void);

private:
    int find_controller(uint8_t bus, uint8_t slot, uint8_t func);
    void scan_controllers(void);
    void init_controller(uint8_t index);
    void read_controller_regs(uint8_t index);
    void scan_ports(uint8_t index);
    void scan_devices(uint8_t index);
    void scan_domains(uint8_t index);
    void update_temperature(uint8_t index);
    void handle_hot_plug(uint8_t index);
    void process_interrupts(uint8_t index);
    uint32_t mmio_read32(uint8_t index, uint32_t offset);
    void mmio_write32(uint8_t index, uint32_t offset, uint32_t value);
    void enable_controller(uint8_t index);
    void disable_controller(uint8_t index);
    void configure_ports(uint8_t index);

    static Thunderbolt4* g_instance;
    struct tb4_controller controllers[TB4_MAX_CONTROLLERS];
    uint8_t controller_count_val;
    uint8_t initialized;
    struct tb4_port ports[TB4_MAX_CONTROLLERS][TB4_MAX_PORTS];
    uint8_t port_counts[TB4_MAX_CONTROLLERS];
    struct tb4_device devices[TB4_MAX_CONTROLLERS][TB4_MAX_DEVICES];
    uint8_t device_counts[TB4_MAX_CONTROLLERS];
    struct tb4_domain domains[TB4_MAX_CONTROLLERS][TB4_MAX_DOMAINS];
    uint8_t domain_counts[TB4_MAX_CONTROLLERS];
    struct tb4_stats stats;
};

extern "C" {
#endif

int thunderbolt4_init(void);
void thunderbolt4_shutdown(void);
void thunderbolt4_poll(void);

#ifdef __cplusplus
}
#endif

#endif
