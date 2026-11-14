/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/kontrolery/intel_rst_vmd.h>

typedef unsigned int uintptr_t;

static IntelRSTVMD g_instance_storage;

static inline uint8_t vmd_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t vmd_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t vmd_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void vmd_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void vmd_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void vmd_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t vmd_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    vmd_outl(0xCF8U, address);
    return vmd_inl(0xCFCU);
}

static void vmd_pci_write32(uint8_t bus, uint8_t slot, uint8_t func,
                             uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    vmd_outl(0xCF8U, address);
    vmd_outl(0xCFCU, value);
}

static void vmd_delay_us(uint32_t us)
{
    for (uint32_t i = 0U; i < us; i++) {
        for (uint32_t j = 0U; j < 48U; j++);
    }
}

static void vmd_delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++) {
        for (uint32_t j = 0U; j < 48000U; j++);
    }
}

IntelRSTVMD* IntelRSTVMD::instance()
{
    return &g_instance_storage;
}

void IntelRSTVMD::set_mmio_base(uint8_t index, volatile uint8_t *base)
{
    if (index < INTEL_RST_VMD_MAX_CONTROLLERS) {
        controllers[index].mmio = base;
    }
}

IntelRSTVMD::IntelRSTVMD()
    : controller_count_val(0U), initialized(0U)
{
    for (uint32_t i = 0U; i < INTEL_RST_VMD_MAX_CONTROLLERS; i++) {
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
        controllers[i].nvme_support = 0U;
        controllers[i].raid_support = 0U;
        controllers[i].hot_plug = 0U;
        controllers[i].power_mgmt = 0U;
        controllers[i].temp_monitor = 0U;
        controllers[i].power_state = VMD_PWR_D0;
        controllers[i].cap_reg = 0U;
        controllers[i].fw_version = 0U;
        controllers[i].temperature_c = 0;
        device_counts[i] = 0U;
        volume_counts[i] = 0U;
        for (uint32_t j = 0U; j < INTEL_RST_VMD_MAX_DEVICES; j++) {
            devices[i][j].controller_index = i;
            devices[i][j].port_index = 0U;
            devices[i][j].vendor_id = 0U;
            devices[i][j].device_id = 0U;
            devices[i][j].present = 0U;
            devices[i][j].link_up = 0U;
            devices[i][j].nvme = 0U;
            devices[i][j].nvme_nsid = 0U;
            devices[i][j].capacity_bytes = 0U;
            devices[i][j].block_size = 512U;
            devices[i][j].temperature_c = 0;
        }
        for (uint32_t j = 0U; j < INTEL_RST_VMD_MAX_VOLUMES; j++) {
            volumes[i][j].volume_id = 0U;
            volumes[i][j].volume_type = VMD_VOL_TYPE_SINGLE;
            volumes[i][j].volume_state = VMD_VOL_STATE_FAILED;
            volumes[i][j].member_count = 0U;
            volumes[i][j].capacity_bytes = 0U;
            volumes[i][j].block_size = 512U;
            volumes[i][j].progress_percent = 0U;
            for (uint32_t k = 0U; k < INTEL_RST_VMD_MAX_PORTS; k++) {
                volumes[i][j].member_ports[k] = 0U;
            }
        }
    }
    stats.controllers_found = 0U;
    stats.controllers_init = 0U;
    stats.devices_found = 0U;
    stats.devices_online = 0U;
    stats.volumes_created = 0U;
    stats.volumes_online = 0U;
    stats.hot_plug_events = 0U;
    stats.power_changes = 0U;
    stats.temp_warnings = 0U;
    stats.temp_critical = 0U;
    stats.errors = 0U;
    stats.resets = 0U;
}

IntelRSTVMD::~IntelRSTVMD()
{
}

uint8_t IntelRSTVMD::probe(uint16_t vendor, uint16_t device)
{
    if (vendor == INTEL_RST_VMD_VENDOR_ID &&
        (device == INTEL_RST_VMD_DEVICE_ID_VMD1 ||
         device == INTEL_RST_VMD_DEVICE_ID_VMD2 ||
         device == INTEL_RST_VMD_DEVICE_ID_VMD3 ||
         device == INTEL_RST_VMD_DEVICE_ID_VMD4 ||
         device == INTEL_RST_VMD_DEVICE_ID_VMD5 ||
         device == INTEL_RST_VMD_DEVICE_ID_VMD6 ||
         device == INTEL_RST_VMD_DEVICE_ID_VMD7)) {
        return 1U;
    }
    return 0U;
}

uint32_t IntelRSTVMD::mmio_read32(uint8_t index, uint32_t offset)
{
    if (index >= controller_count_val || !controllers[index].mmio) return 0xFFFFFFFFU;
    return *(volatile uint32_t *)(controllers[index].mmio + offset);
}

void IntelRSTVMD::mmio_write32(uint8_t index, uint32_t offset, uint32_t value)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    *(volatile uint32_t *)(controllers[index].mmio + offset) = value;
}

int IntelRSTVMD::find_controller(uint8_t bus, uint8_t slot, uint8_t func)
{
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        if (controllers[i].bus == bus && controllers[i].slot == slot && controllers[i].func == func) {
            return (int)i;
        }
    }
    return -1;
}

void IntelRSTVMD::scan_controllers(void)
{
    controller_count_val = 0U;
    stats.controllers_found = 0U;

    for (uint16_t bus = 0U; bus < 256U && controller_count_val < INTEL_RST_VMD_MAX_CONTROLLERS; bus++) {
        for (uint8_t slot = 0U; slot < 32U && controller_count_val < INTEL_RST_VMD_MAX_CONTROLLERS; slot++) {
            for (uint8_t func = 0U; func < 8U && controller_count_val < INTEL_RST_VMD_MAX_CONTROLLERS; func++) {
                uint32_t id0 = vmd_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!probe(vendor, device)) continue;

                uint32_t bar0 = vmd_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;

                uint32_t command = vmd_pci_read32(bus, slot, func, 0x04U);
                vmd_pci_write32(bus, slot, func, 0x04U, command | 0x00000006U);

                struct rst_vmd_controller *c = &controllers[controller_count_val];
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
                c->nvme_support = 0U;
                c->raid_support = 0U;
                c->hot_plug = 0U;
                c->power_mgmt = 0U;
                c->temp_monitor = 0U;
                c->power_state = VMD_PWR_D0;
                c->cap_reg = 0U;
                c->fw_version = 0U;
                c->temperature_c = 0;
                controller_count_val++;
                stats.controllers_found++;
            }
        }
    }
}

void IntelRSTVMD::read_controller_regs(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;

    struct rst_vmd_controller *c = &controllers[index];
    c->cap_reg = mmio_read32(index, VMD_REG_CAPABILITY);
    c->fw_version = mmio_read32(index, VMD_REG_FW_VERSION);

    uint32_t cap = c->cap_reg;
    c->nvme_support = (uint8_t)((cap & VMD_CAP_NVME_SUPPORT) ? 1U : 0U);
    c->raid_support = (uint8_t)((cap & VMD_CAP_RAID_SUPPORT) ? 1U : 0U);
    c->hot_plug = (uint8_t)((cap & VMD_CAP_HOT_PLUG) ? 1U : 0U);
    c->power_mgmt = (uint8_t)((cap & VMD_CAP_POWER_MGMT) ? 1U : 0U);
    c->temp_monitor = (uint8_t)((cap & VMD_CAP_TEMP_MONITOR) ? 1U : 0U);
    c->port_count = (uint8_t)((cap >> VMD_CAP_PORT_COUNT_SHIFT) & 0x0FU);
    if (c->port_count == 0U) c->port_count = 1U;

    update_temperature(index);
}

void IntelRSTVMD::enable_controller(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    mmio_write32(index, VMD_REG_CONTROL,
                 (uint32_t)(VMD_CTL_ENABLE |
                            (controllers[index].hot_plug ? VMD_CTL_HOT_PLUG_EN : 0U) |
                            (controllers[index].raid_support ? VMD_CTL_RAID_EN : 0U) |
                            (controllers[index].power_mgmt ? VMD_CTL_POWER_MGMT_EN : 0U) |
                            (controllers[index].temp_monitor ? VMD_CTL_TEMP_MON_EN : 0U)));
    controllers[index].enabled = 1U;
    vmd_delay_us(100U);
}

void IntelRSTVMD::disable_controller(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    mmio_write32(index, VMD_REG_CONTROL, VMD_CTL_DISABLE);
    controllers[index].enabled = 0U;
}

void IntelRSTVMD::configure_ports(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    for (uint8_t port = 0U; port < controllers[index].port_count; port++) {
        uint32_t port_reg = VMD_REG_PORT_BASE + (uint32_t)port * VMD_REG_PORT_STRIDE;
        uint32_t val = mmio_read32(index, port_reg);
        if (val & VMD_PORT_PRESENT) {
            mmio_write32(index, port_reg, val | VMD_PORT_NVME);
        }
    }
}

void IntelRSTVMD::scan_devices(uint8_t index)
{
    if (index >= controller_count_val) return;
    device_counts[index] = 0U;

    for (uint8_t port = 0U; port < controllers[index].port_count && device_counts[index] < INTEL_RST_VMD_MAX_DEVICES; port++) {
        struct rst_vmd_device *d = &devices[index][device_counts[index]];
        d->controller_index = index;
        d->port_index = port;
        d->present = 0U;
        d->link_up = 0U;
        d->nvme = 0U;
        d->nvme_nsid = 0U;
        d->capacity_bytes = 0U;
        d->block_size = 512U;
        d->temperature_c = 0;

        uint32_t port_reg = VMD_REG_PORT_BASE + (uint32_t)port * VMD_REG_PORT_STRIDE;
        uint32_t val = mmio_read32(index, port_reg);
        if (val & VMD_PORT_PRESENT) {
            d->present = 1U;
            d->link_up = (uint8_t)((val & VMD_PORT_LINK_UP) ? 1U : 0U);
            d->nvme = (uint8_t)((val & VMD_PORT_NVME) ? 1U : 0U);
            d->vendor_id = INTEL_RST_VMD_VENDOR_ID;
            d->device_id = 0x7A02U;
            d->capacity_bytes = 512ULL * 1024ULL * 1024ULL * 1024ULL;
            d->block_size = 4096U;
            d->temperature_c = 35;
            device_counts[index]++;
            stats.devices_found++;
            if (d->link_up) stats.devices_online++;
        }
    }
}

void IntelRSTVMD::scan_volumes(uint8_t index)
{
    if (index >= controller_count_val) return;
    volume_counts[index] = 0U;

    if (controllers[index].raid_support) {
        struct rst_vmd_volume *v = &volumes[index][volume_counts[index]];
        v->volume_id = 0U;
        v->volume_type = VMD_VOL_TYPE_RAID1;
        v->volume_state = VMD_VOL_STATE_ONLINE;
        v->member_count = 2U;
        v->member_ports[0] = 0U;
        v->member_ports[1] = 1U;
        v->capacity_bytes = 512ULL * 1024ULL * 1024ULL * 1024ULL;
        v->block_size = 4096U;
        v->progress_percent = 100U;
        volume_counts[index]++;
        stats.volumes_created++;
        stats.volumes_online++;
    }
}

void IntelRSTVMD::update_temperature(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].temp_monitor) return;
    int32_t temp = (int32_t)(mmio_read32(index, VMD_REG_TEMP_SENSOR) & 0xFFU);
    if (temp == 0) temp = 40;
    controllers[index].temperature_c = temp;
    if (temp > 80) {
        stats.temp_warnings++;
    }
    if (temp > 95) {
        stats.temp_critical++;
    }
}

void IntelRSTVMD::handle_hot_plug(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].hot_plug) return;
    uint32_t isr = mmio_read32(index, VMD_REG_INT_STATUS);
    if (isr & VMD_INT_PORT_CHANGE) {
        stats.hot_plug_events++;
        scan_devices(index);
        mmio_write32(index, VMD_REG_INT_STATUS, VMD_INT_PORT_CHANGE);
    }
}

void IntelRSTVMD::process_interrupts(uint8_t index)
{
    if (index >= controller_count_val || !controllers[index].mmio) return;
    uint32_t isr = mmio_read32(index, VMD_REG_INT_STATUS);
    if (!isr) return;

    if (isr & VMD_INT_PORT_CHANGE) {
        handle_hot_plug(index);
    }
    if (isr & VMD_INT_VOL_CHANGE) {
        scan_volumes(index);
        mmio_write32(index, VMD_REG_INT_STATUS, VMD_INT_VOL_CHANGE);
    }
    if (isr & VMD_INT_THRESHOLD) {
        update_temperature(index);
        mmio_write32(index, VMD_REG_INT_STATUS, VMD_INT_THRESHOLD);
    }
    if (isr & VMD_INT_ERROR) {
        stats.errors++;
        mmio_write32(index, VMD_REG_INT_STATUS, VMD_INT_ERROR);
    }
    mmio_write32(index, VMD_REG_INT_STATUS, isr);
}

void IntelRSTVMD::init_controller(uint8_t index)
{
    if (index >= controller_count_val || controllers[index].initialized) return;

    read_controller_regs(index);
    enable_controller(index);
    configure_ports(index);
    scan_devices(index);
    scan_volumes(index);

    controllers[index].initialized = 1U;
    controllers[index].power_state = VMD_PWR_D0;
    stats.controllers_init++;
}

void IntelRSTVMD::init(void)
{
    if (initialized) return;
    scan_controllers();
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        init_controller(i);
    }
    initialized = 1U;
}

void IntelRSTVMD::deinit(void)
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

void IntelRSTVMD::poll(void)
{
    if (!initialized) return;
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        if (controllers[i].initialized) {
            process_interrupts(i);
            update_temperature(i);
        }
    }
}

uint8_t IntelRSTVMD::is_ready(void)
{
    return initialized;
}

uint8_t IntelRSTVMD::controller_count(void)
{
    return controller_count_val;
}

int IntelRSTVMD::controller_info(uint8_t index, struct rst_vmd_controller *out)
{
    if (index >= controller_count_val || !out) return 0;
    *out = controllers[index];
    return 1;
}

uint8_t IntelRSTVMD::device_count(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0U;
    return device_counts[controller_index];
}

int IntelRSTVMD::device_info(uint8_t controller_index, uint8_t device_index, struct rst_vmd_device *out)
{
    if (controller_index >= controller_count_val || !out) return 0;
    if (device_index >= device_counts[controller_index]) return 0;
    *out = devices[controller_index][device_index];
    return 1;
}

uint8_t IntelRSTVMD::volume_count(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0U;
    return volume_counts[controller_index];
}

int IntelRSTVMD::volume_info(uint8_t controller_index, uint8_t volume_index, struct rst_vmd_volume *out)
{
    if (controller_index >= controller_count_val || !out) return 0;
    if (volume_index >= volume_counts[controller_index]) return 0;
    *out = volumes[controller_index][volume_index];
    return 1;
}

int IntelRSTVMD::port_enable(uint8_t controller_index, uint8_t port_index)
{
    if (controller_index >= controller_count_val) return 0;
    uint32_t port_reg = VMD_REG_PORT_BASE + (uint32_t)port_index * VMD_REG_PORT_STRIDE;
    uint32_t val = mmio_read32(controller_index, port_reg);
    val |= VMD_PORT_PRESENT | VMD_PORT_NVME;
    mmio_write32(controller_index, port_reg, val);
    return 1;
}

int IntelRSTVMD::port_disable(uint8_t controller_index, uint8_t port_index)
{
    if (controller_index >= controller_count_val) return 0;
    uint32_t port_reg = VMD_REG_PORT_BASE + (uint32_t)port_index * VMD_REG_PORT_STRIDE;
    uint32_t val = mmio_read32(controller_index, port_reg);
    val &= ~(VMD_PORT_PRESENT | VMD_PORT_NVME);
    mmio_write32(controller_index, port_reg, val);
    return 1;
}

int IntelRSTVMD::port_rescan(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0;
    scan_devices(controller_index);
    return 1;
}

int IntelRSTVMD::volume_create(uint8_t controller_index, uint8_t volume_type, const uint8_t *member_ports, uint8_t member_count)
{
    if (controller_index >= controller_count_val || !controllers[controller_index].raid_support) return 0;
    if (volume_counts[controller_index] >= INTEL_RST_VMD_MAX_VOLUMES) return 0;
    if (!member_ports || member_count == 0U || member_count > INTEL_RST_VMD_MAX_PORTS) return 0;

    struct rst_vmd_volume *v = &volumes[controller_index][volume_counts[controller_index]];
    v->volume_id = volume_counts[controller_index];
    v->volume_type = volume_type;
    v->volume_state = VMD_VOL_STATE_ONLINE;
    v->member_count = member_count;
    for (uint8_t i = 0U; i < member_count; i++) {
        v->member_ports[i] = member_ports[i];
    }
    v->capacity_bytes = 512ULL * 1024ULL * 1024ULL * 1024ULL * (uint64_t)member_count;
    v->block_size = 4096U;
    v->progress_percent = 100U;
    volume_counts[controller_index]++;
    stats.volumes_created++;
    stats.volumes_online++;
    return (int)v->volume_id;
}

int IntelRSTVMD::volume_delete(uint8_t controller_index, uint8_t volume_id)
{
    if (controller_index >= controller_count_val) return 0;
    for (uint32_t i = 0U; i < volume_counts[controller_index]; i++) {
        if (volumes[controller_index][i].volume_id == volume_id) {
            volumes[controller_index][i].volume_state = VMD_VOL_STATE_FAILED;
            stats.volumes_online--;
            return 1;
        }
    }
    return 0;
}

int IntelRSTVMD::volume_rebuild(uint8_t controller_index, uint8_t volume_id)
{
    if (controller_index >= controller_count_val) return 0;
    for (uint32_t i = 0U; i < volume_counts[controller_index]; i++) {
        if (volumes[controller_index][i].volume_id == volume_id) {
            volumes[controller_index][i].volume_state = VMD_VOL_STATE_REBUILDING;
            volumes[controller_index][i].progress_percent = 0U;
            return 1;
        }
    }
    return 0;
}

int IntelRSTVMD::power_set_state(uint8_t controller_index, uint8_t power_state)
{
    if (controller_index >= controller_count_val) return 0;
    if (!controllers[controller_index].power_mgmt) return 0;
    mmio_write32(controller_index, VMD_REG_POWER_STATE, power_state);
    controllers[controller_index].power_state = power_state;
    stats.power_changes++;
    return 1;
}

int IntelRSTVMD::power_get_state(uint8_t controller_index, uint8_t *power_state)
{
    if (controller_index >= controller_count_val || !power_state) return 0;
    *power_state = controllers[controller_index].power_state;
    return 1;
}

int IntelRSTVMD::temperature_read(uint8_t controller_index, int32_t *temperature_c)
{
    if (controller_index >= controller_count_val || !temperature_c) return 0;
    update_temperature(controller_index);
    *temperature_c = controllers[controller_index].temperature_c;
    return 1;
}

int IntelRSTVMD::reset_controller(uint8_t controller_index)
{
    if (controller_index >= controller_count_val) return 0;
    mmio_write32(controller_index, VMD_REG_CONTROL, VMD_CTL_RESET);
    vmd_delay_ms(100U);
    disable_controller(controller_index);
    vmd_delay_us(50U);
    enable_controller(controller_index);
    read_controller_regs(controller_index);
    scan_devices(controller_index);
    scan_volumes(controller_index);
    controllers[controller_index].power_state = VMD_PWR_D0;
    stats.resets++;
    return 1;
}

void IntelRSTVMD::stats_get(struct rst_vmd_stats *out)
{
    if (!out) return;
    *out = stats;
}

void IntelRSTVMD::stats_reset(void)
{
    stats.controllers_found = 0U;
    stats.controllers_init = 0U;
    stats.devices_found = 0U;
    stats.devices_online = 0U;
    stats.volumes_created = 0U;
    stats.volumes_online = 0U;
    stats.hot_plug_events = 0U;
    stats.power_changes = 0U;
    stats.temp_warnings = 0U;
    stats.temp_critical = 0U;
    stats.errors = 0U;
    stats.resets = 0U;
}

void IntelRSTVMD::debug_print_controllers(void)
{
    for (uint32_t i = 0U; i < controller_count_val; i++) {
        (void)controllers[i].bus;
        (void)controllers[i].slot;
        (void)controllers[i].device_id;
    }
}

extern "C" int intel_rst_vmd_init(void)
{
    IntelRSTVMD *drv = IntelRSTVMD::instance();
    drv->init();
    return drv->is_ready() ? 1 : 0;
}

extern "C" void intel_rst_vmd_shutdown(void)
{
    IntelRSTVMD *drv = IntelRSTVMD::instance();
    if (drv) drv->deinit();
}

extern "C" void intel_rst_vmd_poll(void)
{
    IntelRSTVMD *drv = IntelRSTVMD::instance();
    if (drv && drv->is_ready()) drv->poll();
}
