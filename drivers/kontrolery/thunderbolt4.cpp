/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/kontrolery/thunderbolt4.h>

typedef unsigned int uintptr_t;

static Thunderbolt4 g_tb4_storage;

static inline uint8_t tb4_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t tb4_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t tb4_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void tb4_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void tb4_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void tb4_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t tb4_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    tb4_outl(0xCF8U, address);
    return tb4_inl(0xCFCU);
}

static void tb4_pci_write32(uint8_t bus, uint8_t slot, uint8_t func,
                             uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    tb4_outl(0xCF8U, address);
    tb4_outl(0xCFCU, value);
}

static void tb4_delay_us(uint32_t us)
{
    for (uint32_t i = 0U; i < us; i++) {
        for (uint32_t j = 0U; j < 48U; j++);
    }
}

static void tb4_delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++) {
        for (uint32_t j = 0U; j < 48000U; j++);
    }
}

Thunderbolt4* Thunderbolt4::instance()
{
    return &g_tb4_storage;
}

void Thunderbolt4::set_mmio_base(uint8_t index, volatile uint8_t *base)
{
    if (index < TB4_MAX_CONTROLLERS) {
        controllers[index].mmio = base;
    }
}

Thunderbolt4::Thunderbolt4()
    : controller_count_val(0U), initialized(0U)
{
    for (uint32_t i = 0U; i < TB4_MAX_CONTROLLERS; i++) {
        controllers[i].bus = 0U;
        controllers[i].slot = 0U;
        controllers[i].func = 0U;
        controllers[i].vendor_id = 0U;
        controllers[i].device_id = 0U;
        controllers[i].bar0 = 0U;
        controllers[i].mmio = 0;
        controllers[i].initialized = 0U;
        controllers[i].enabled = 0U;
        controllers[i].port_count = 0U;
        controllers[i].usb32_support = 0U;
        controllers[i].tbt4_support = 0U;
        controllers[i].dp_alt_mode = 0U;
        controllers[i].pcie_tunneling = 0U;
        controllers[i].power_state = TB4_POWER_D0;
        controllers[i].security_level = TB4_SECURITY_NONE;
        controllers[i].sw_version = 0U;
        controllers[i].temperature_c = 0;
        port_counts[i] = 0U;
        device_counts[i] = 0U;
        domain_counts[i] = 0U;
        for (uint32_t j = 0U; j < TB4_MAX_PORTS; j++) {
            ports[i][j].controller_index = i;
            ports[i][j].port_index = (uint8_t)j;
            ports[i][j].port_type = TB4_PORT_TYPE_USB_C;
            ports[i][j].port_state = TB4_PORT_STATE_DISCONNECTED;
            ports[i][j].connected = 0U;
            ports[i][j].link_up = 0U;
            ports[i][j].usb32_active = 0U;
            ports[i][j].tbt4_active = 0U;
            ports[i][j].speed_mbps = 0U;
            ports[i][j].device_attached = 0U;
        }
        for (uint32_t j = 0U; j < TB4_MAX_DEVICES; j++) {
            devices[i][j].controller_index = i;
            devices[i][j].port_index = 0U;
            devices[i][j].device_index = (uint8_t)j;
            devices[i][j].vendor_id = 0U;
            devices[i][j].device_id = 0U;
            devices[i][j].device_type = TB4_DEVICE_TYPE_STORAGE;
            devices[i][j].connected = 0U;
            devices[i][j].configured = 0U;
            devices[i][j].authorized = 0U;
            devices[i][j].bandwidth_mbps = 0U;
            devices[i][j].nvme_tunnel = 0U;
            devices[i][j].display_tunnel = 0U;
        }
        for (uint32_t j = 0U; j < TB4_MAX_DOMAINS; j++) {
            domains[i][j].domain_id = 0U;
            domains[i][j].domain_state = 0U;
            domains[i][j].member_count = 0U;
            domains[i][j].domain_master = 0U;
            for (uint32_t k = 0U; k < TB4_MAX_PORTS; k++) {
                domains[i][j].member_ports[k] = 0U;
            }
        }
    }
    stats.controllers_found = 0U;
    stats.controllers_init = 0U;
    stats.ports_configured = 0U;
    stats.devices_enumerated = 0U;
    stats.devices_authorized = 0U;
    stats.hot_plug_events = 0U;
    stats.domain_changes = 0U;
    stats.power_changes = 0U;
    stats.temp_warnings = 0U;
    stats.temp_critical = 0U;
    stats.errors = 0U;
    stats.resets = 0U;
    stats.usb32_transfers = 0U;
    stats.tbt4_transfers = 0U;
}

Thunderbolt4::~Thunderbolt4()
{
}

uint8_t Thunderbolt4::probe(uint16_t vendor, uint16_t device)
{
    if (vendor == TB4_VENDOR_ID_INTEL &&
        (device == TB4_DEVICE_ID_TBT4 ||
         device == TB4_DEVICE_ID_USB32 ||
         device == TB4_DEVICE_ID_ALPINE)) {
        return 1U;
    }
    return 0U;
}

uint32_t Thunderbolt4::mmio_read32(uint8_t index, uint32_t offset)
{
    if (index >= controller_count_val || !controllers[index].mmio) return 0xFFFFFFFFU;
    return *(volatile uint32_t *)(controllers[index].mmio + offset);
}

void Thunderbolt4::mmio_write32(uint8_t index, uint32_t offset, uint32_t value)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    *(volatile uint32_t *)(controllers[index].mmio + offset) = value;
}

int Thunderbolt4::find_controller(uint8_t bus, uint8_t slot, uint8_t func)
{
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        if (controllers[i].bus == bus && controllers[i].slot == slot && controllers[i].func == func) {
            return (int)i;
        }
    }
    return -1;
}

void Thunderbolt4::scan_controllers(void)
{
    controller_count_val = 0U;
    stats.controllers_found = 0U;

    for (uint16_t bus = 0U; bus < 256U && controller_count_val < TB4_MAX_CONTROLLERS; bus++) {
        for (uint8_t slot = 0U; slot < 32U && controller_count_val < TB4_MAX_CONTROLLERS; slot++) {
            for (uint8_t func = 0U; func < 8U && controller_count_val < TB4_MAX_CONTROLLERS; func++) {
                uint32_t id0 = tb4_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!probe(vendor, device)) continue;

                uint32_t bar0 = tb4_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;

                uint32_t command = tb4_pci_read32(bus, slot, func, 0x04U);
                tb4_pci_write32(bus, slot, func, 0x04U, command | 0x00000006U);

                struct tb4_controller *c = &controllers[controller_count_val];
                c->bus = bus;
                c->slot = slot;
                c->func = func;
                c->vendor_id = vendor;
                c->device_id = device;
                c->bar0 = bar0 & 0xFFFFFFF0U;
                c->mmio = (volatile uint8_t *)(bar0 & 0xFFFFFFF0U);
                c->initialized = 0U;
                c->enabled = 0U;
                c->port_count = 0U;
                c->usb32_support = 0U;
                c->tbt4_support = 0U;
                c->dp_alt_mode = 0U;
                c->pcie_tunneling = 0U;
                c->power_state = TB4_POWER_D0;
                c->security_level = TB4_SECURITY_NONE;
                c->sw_version = 0U;
                c->temperature_c = 0;
                controller_count_val++;
                stats.controllers_found++;
            }
        }
    }
}

void Thunderbolt4::read_controller_regs(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;

    struct tb4_controller *c = &controllers[index];
    c->sw_version = mmio_read32(index, TB4_REG_SW_VERSION);

    uint32_t sw_status = mmio_read32(index, TB4_REG_SW_STATUS);
    c->usb32_support = (uint8_t)((sw_status & 0x00000010U) ? 1U : 0U);
    c->tbt4_support = (uint8_t)((sw_status & 0x00000020U) ? 1U : 0U);
    c->dp_alt_mode = (uint8_t)((sw_status & 0x00000040U) ? 1U : 0U);
    c->pcie_tunneling = (uint8_t)((sw_status & 0x00000080U) ? 1U : 0U);

    update_temperature(index);
}

void Thunderbolt4::enable_controller(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    mmio_write32(index, TB4_REG_SW_CONTROL,
                 (uint32_t)(TB4_SW_CONTROL_ENABLE |
                            (controllers[index].security_level > TB4_SECURITY_NONE ? TB4_SW_CONTROL_SECURITY : 0U) |
                            (controllers[index].power_state == TB4_POWER_D0 ? TB4_SW_CONTROL_POWER_MGMT : 0U)));
    controllers[index].enabled = 1U;
    tb4_delay_us(100U);
}

void Thunderbolt4::disable_controller(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    mmio_write32(index, TB4_REG_SW_CONTROL, TB4_SW_CONTROL_DISABLE);
    controllers[index].enabled = 0U;
}

void Thunderbolt4::configure_ports(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    for (uint8_t port = 0U; port < controllers[index].port_count; port++) {
        uint32_t port_reg = TB4_REG_PORT_BASE + (uint32_t)port * TB4_REG_PORT_STRIDE;
        uint32_t val = mmio_read32(index, port_reg);
        if (val & 0x00000001U) {
            ports[index][port].connected = 1U;
            ports[index][port].link_up = (uint8_t)((val & 0x00000002U) ? 1U : 0U);
            ports[index][port].port_state = TB4_PORT_STATE_ACTIVE;
            if (val & 0x00000100U) ports[index][port].usb32_active = 1U;
            if (val & 0x00000200U) ports[index][port].tbt4_active = 1U;
            ports[index][port].speed_mbps = 40000U;
            stats.ports_configured++;
        }
    }
}

void Thunderbolt4::scan_ports(uint8_t index)
{
    if (index >= controller_count_val) return;
    port_counts[index] = 0U;

    for (uint8_t port = 0U; port < TB4_MAX_PORTS && port_counts[index] < TB4_MAX_PORTS; port++) {
        ports[index][port].connected = 0U;
        ports[index][port].link_up = 0U;
        ports[index][port].usb32_active = 0U;
        ports[index][port].tbt4_active = 0U;
        ports[index][port].speed_mbps = 0U;
        ports[index][port].device_attached = 0U;

        uint32_t port_reg = TB4_REG_PORT_BASE + (uint32_t)port * TB4_REG_PORT_STRIDE;
        uint32_t val = mmio_read32(index, port_reg);
        if (val & 0x00000001U) {
            ports[index][port].connected = 1U;
            ports[index][port].link_up = (uint8_t)((val & 0x00000002U) ? 1U : 0U);
            ports[index][port].port_state = TB4_PORT_STATE_ACTIVE;
            if (val & 0x00000100U) ports[index][port].usb32_active = 1U;
            if (val & 0x00000200U) ports[index][port].tbt4_active = 1U;
            ports[index][port].speed_mbps = 40000U;
            ports[index][port].port_type = TB4_PORT_TYPE_TBT4;
            port_counts[index]++;
            stats.ports_configured++;
        }
    }
}

void Thunderbolt4::scan_devices(uint8_t index)
{
    if (index >= controller_count_val) return;
    device_counts[index] = 0U;

    for (uint8_t port = 0U; port < port_counts[index] && device_counts[index] < TB4_MAX_DEVICES; port++) {
        if (!ports[index][port].connected) continue;

        struct tb4_device *d = &devices[index][device_counts[index]];
        d->controller_index = index;
        d->port_index = port;
        d->device_index = device_counts[index];
        d->vendor_id = TB4_VENDOR_ID_INTEL;
        d->device_id = 0x43EDU;
        d->device_type = TB4_DEVICE_TYPE_STORAGE;
        d->connected = 1U;
        d->configured = 1U;
        d->authorized = 0U;
        d->bandwidth_mbps = 40000U;
        d->nvme_tunnel = 0U;
        d->display_tunnel = 0U;
        device_counts[index]++;
        stats.devices_enumerated++;
    }
}

void Thunderbolt4::scan_domains(uint8_t index)
{
    if (index >= controller_count_val) return;
    domain_counts[index] = 0U;

    if (controllers[index].tbt4_support) {
        struct tb4_domain *dom = &domains[index][domain_counts[index]];
        dom->domain_id = 0U;
        dom->domain_state = 1U;
        dom->member_count = port_counts[index];
        dom->domain_master = 0U;
        for (uint8_t i = 0U; i < port_counts[index]; i++) {
            dom->member_ports[i] = i;
        }
        domain_counts[index]++;
    }
}

void Thunderbolt4::update_temperature(uint8_t index)
{
    if (index >= controller_count_val) return;
    int32_t temp = (int32_t)(mmio_read32(index, TB4_REG_TEMP_SENSOR) & 0xFFU);
    if (temp == 0) temp = 45;
    controllers[index].temperature_c = temp;
    if (temp > 75) {
        stats.temp_warnings++;
    }
    if (temp > 90) {
        stats.temp_critical++;
    }
}

void Thunderbolt4::handle_hot_plug(uint8_t index)
{
    if (index >= controller_count_val) return;
    uint32_t isr = mmio_read32(index, TB4_REG_SW_INT_STATUS);
    if (isr & TB4_INT_PORT_HOT_PLUG) {
        stats.hot_plug_events++;
        scan_ports(index);
        mmio_write32(index, TB4_REG_SW_INT_STATUS, TB4_INT_PORT_HOT_PLUG);
    }
}

void Thunderbolt4::process_interrupts(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    uint32_t isr = mmio_read32(index, TB4_REG_SW_INT_STATUS);
    if (!isr) return;

    if (isr & TB4_INT_PORT_HOT_PLUG) {
        handle_hot_plug(index);
    }
    if (isr & TB4_INT_PORT_LINK_CHANGE) {
        scan_ports(index);
        mmio_write32(index, TB4_REG_SW_INT_STATUS, TB4_INT_PORT_LINK_CHANGE);
    }
    if (isr & TB4_INT_DEVICE_REMOVED) {
        stats.devices_authorized--;
        mmio_write32(index, TB4_REG_SW_INT_STATUS, TB4_INT_DEVICE_REMOVED);
    }
    if (isr & TB4_INT_DOMAIN_CHANGE) {
        stats.domain_changes++;
        scan_domains(index);
        mmio_write32(index, TB4_REG_SW_INT_STATUS, TB4_INT_DOMAIN_CHANGE);
    }
    if (isr & TB4_INT_ERROR) {
        stats.errors++;
        mmio_write32(index, TB4_REG_SW_INT_STATUS, TB4_INT_ERROR);
    }
}

void Thunderbolt4::init_controller(uint8_t index)
{
    if (index >= controller_count_val || controllers[index].initialized) return;

    read_controller_regs(index);
    enable_controller(index);
    configure_ports(index);
    scan_ports(index);
    scan_devices(index);
    scan_domains(index);

    controllers[index].initialized = 1U;
    controllers[index].power_state = TB4_POWER_D0;
    stats.controllers_init++;
}

void Thunderbolt4::init(void)
{
    if (initialized) return;
    scan_controllers();
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        init_controller(i);
    }
    initialized = 1U;
}

void Thunderbolt4::deinit(void)
{
    if (!initialized) return;
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        if (controllers[i].initialized) {
            disable_controller(i);
            controllers[i].initialized = 0U;
        }
    }
    controller_count_val = 0U;
    initialized = 0U;
}

void Thunderbolt4::poll(void)
{
    if (!initialized) return;
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        if (controllers[i].initialized) {
            process_interrupts(i);
            update_temperature(i);
        }
    }
}

uint8_t Thunderbolt4::is_ready(void)
{
    return initialized;
}

uint8_t Thunderbolt4::controller_count(void)
{
    return controller_count_val;
}

int Thunderbolt4::controller_info(uint8_t index, struct tb4_controller *out)
{
    if (index >= controller_count_val || !out) return 0;
    *out = controllers[index];
    return 1;
}

uint8_t Thunderbolt4::port_count(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0U;
    return port_counts[controller_index];
}

int Thunderbolt4::port_info(uint8_t controller_index, uint8_t port_index, struct tb4_port *out)
{
    if (controller_index >= controller_count_val || !out) return 0;
    if (port_index >= port_counts[controller_index]) return 0;
    *out = ports[controller_index][port_index];
    return 1;
}

uint8_t Thunderbolt4::device_count(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0U;
    return device_counts[controller_index];
}

int Thunderbolt4::device_info(uint8_t controller_index, uint8_t device_index, struct tb4_device *out)
{
    if (controller_index >= controller_count_val || !out) return 0;
    if (device_index >= device_counts[controller_index]) return 0;
    *out = devices[controller_index][device_index];
    return 1;
}

uint8_t Thunderbolt4::domain_count(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0U;
    return domain_counts[controller_index];
}

int Thunderbolt4::domain_info(uint8_t controller_index, uint8_t domain_index, struct tb4_domain *out)
{
    if (controller_index >= controller_count_val || !out) return 0;
    if (domain_index >= domain_counts[controller_index]) return 0;
    *out = domains[controller_index][domain_index];
    return 1;
}

int Thunderbolt4::port_enable(uint8_t controller_index, uint8_t port_index)
{
    if (controller_index >= controller_count_val) return 0;
    uint32_t port_reg = TB4_REG_PORT_BASE + (uint32_t)port_index * TB4_REG_PORT_STRIDE;
    uint32_t val = mmio_read32(controller_index, port_reg);
    val |= 0x00000001U;
    mmio_write32(controller_index, port_reg, val);
    return 1;
}

int Thunderbolt4::port_disable(uint8_t controller_index, uint8_t port_index)
{
    if (controller_index >= controller_count_val) return 0;
    uint32_t port_reg = TB4_REG_PORT_BASE + (uint32_t)port_index * TB4_REG_PORT_STRIDE;
    uint32_t val = mmio_read32(controller_index, port_reg);
    val &= ~0x00000001U;
    mmio_write32(controller_index, port_reg, val);
    return 1;
}

int Thunderbolt4::port_rescan(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0;
    scan_ports(controller_index);
    return 1;
}

int Thunderbolt4::device_authorize(uint8_t controller_index, uint8_t device_index)
{
    if (controller_index >= controller_count_val) return 0;
    if (device_index >= device_counts[controller_index]) return 0;
    devices[controller_index][device_index].authorized = 1U;
    stats.devices_authorized++;
    return 1;
}

int Thunderbolt4::device_deauthorize(uint8_t controller_index, uint8_t device_index)
{
    if (controller_index >= controller_count_val) return 0;
    if (device_index >= device_counts[controller_index]) return 0;
    devices[controller_index][device_index].authorized = 0U;
    stats.devices_authorized--;
    return 1;
}

int Thunderbolt4::device_config(uint8_t controller_index, uint8_t device_index)
{
    if (controller_index >= controller_count_val) return 0;
    if (device_index >= device_counts[controller_index]) return 0;
    devices[controller_index][device_index].configured = 1U;
    return 1;
}

int Thunderbolt4::domain_create(uint8_t controller_index, uint8_t *member_ports, uint8_t member_count)
{
    if (controller_index >= controller_count_val) return 0;
    if (domain_counts[controller_index] >= TB4_MAX_DOMAINS) return 0;
    if (!member_ports || member_count == 0U || member_count > TB4_MAX_PORTS) return 0;

    struct tb4_domain *dom = &domains[controller_index][domain_counts[controller_index]];
    dom->domain_id = domain_counts[controller_index];
    dom->domain_state = 1U;
    dom->member_count = member_count;
    dom->domain_master = member_ports[0];
    for (uint8_t i = 0U; i < member_count; i++) {
        dom->member_ports[i] = member_ports[i];
    }
    domain_counts[controller_index]++;
    stats.domain_changes++;
    return (int)dom->domain_id;
}

int Thunderbolt4::domain_delete(uint8_t controller_index, uint8_t domain_id)
{
    if (controller_index >= controller_count_val) return 0;
    for (uint32_t i = 0U; i < domain_counts[controller_index]; i++) {
        if (domains[controller_index][i].domain_id == domain_id) {
            domains[controller_index][i].domain_state = 0U;
            stats.domain_changes--;
            return 1;
        }
    }
    return 0;
}

int Thunderbolt4::power_set_state(uint8_t controller_index, uint8_t power_state)
{
    if (controller_index >= controller_count_val) return 0;
    mmio_write32(controller_index, TB4_REG_POWER_STATE, power_state);
    controllers[controller_index].power_state = power_state;
    stats.power_changes++;
    return 1;
}

int Thunderbolt4::power_get_state(uint8_t controller_index, uint8_t *power_state)
{
    if (controller_index >= controller_count_val || !power_state) return 0;
    *power_state = controllers[controller_index].power_state;
    return 1;
}

int Thunderbolt4::temperature_read(uint8_t controller_index, int32_t *temperature_c)
{
    if (controller_index >= controller_count_val || !temperature_c) return 0;
    update_temperature(controller_index);
    *temperature_c = controllers[controller_index].temperature_c;
    return 1;
}

int Thunderbolt4::security_set_level(uint8_t controller_index, uint8_t level)
{
    if (controller_index >= controller_count_val) return 0;
    if (level > TB4_SECURITY_DPONLY) return 0;
    controllers[controller_index].security_level = level;
    mmio_write32(controller_index, TB4_REG_SW_CONTROL,
                 (uint32_t)(TB4_SW_CONTROL_ENABLE |
                            (level > TB4_SECURITY_NONE ? TB4_SW_CONTROL_SECURITY : 0U)));
    return 1;
}

int Thunderbolt4::security_get_level(uint8_t controller_index, uint8_t *level)
{
    if (controller_index >= controller_count_val || !level) return 0;
    *level = controllers[controller_index].security_level;
    return 1;
}

int Thunderbolt4::reset_controller(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0;
    mmio_write32(controller_index, TB4_REG_SW_CONTROL, TB4_SW_CONTROL_RESET);
    tb4_delay_ms(100U);
    disable_controller(controller_index);
    tb4_delay_us(50U);
    enable_controller(controller_index);
    read_controller_regs(controller_index);
    scan_ports(controller_index);
    scan_devices(controller_index);
    scan_domains(controller_index);
    controllers[controller_index].power_state = TB4_POWER_D0;
    stats.resets++;
    return 1;
}

void Thunderbolt4::stats_get(struct tb4_stats *out)
{
    if (!out) return;
    *out = stats;
}

void Thunderbolt4::stats_reset(void)
{
    stats.controllers_found = 0U;
    stats.controllers_init = 0U;
    stats.ports_configured = 0U;
    stats.devices_enumerated = 0U;
    stats.devices_authorized = 0U;
    stats.hot_plug_events = 0U;
    stats.domain_changes = 0U;
    stats.power_changes = 0U;
    stats.temp_warnings = 0U;
    stats.temp_critical = 0U;
    stats.errors = 0U;
    stats.resets = 0U;
    stats.usb32_transfers = 0U;
    stats.tbt4_transfers = 0U;
}

void Thunderbolt4::debug_print_controllers(void)
{
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        (void)controllers[i].bus;
        (void)controllers[i].slot;
        (void)controllers[i].device_id;
    }
}

extern "C" int thunderbolt4_init(void)
{
    Thunderbolt4 *drv = Thunderbolt4::instance();
    drv->init();
    return drv->is_ready() ? 1 : 0;
}

extern "C" void thunderbolt4_shutdown(void)
{
    Thunderbolt4 *drv = Thunderbolt4::instance();
    if (drv) drv->deinit();
}

extern "C" void thunderbolt4_poll(void)
{
    Thunderbolt4 *drv = Thunderbolt4::instance();
    if (drv && drv->is_ready()) drv->poll();
}
