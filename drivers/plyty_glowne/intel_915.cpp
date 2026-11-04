#include "intel_915.h"

static inline void i915_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t i915_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void i915_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t i915_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void i915_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t i915_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void i915_io_delay(void)
{
    i915_inb(0x80U);
}

static inline void i915_memcpy(void *dst, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (uint32_t i = 0; i < n; i++) d[i] = s[i];
}

static inline void i915_memset(void *dst, uint8_t value, uint32_t n)
{
    uint8_t *d = (uint8_t *)dst;
    for (uint32_t i = 0; i < n; i++) d[i] = value;
}

static inline void i915_bcd_to_bin(uint8_t *value)
{
    *value = (uint8_t)(((*value >> 4) * 10U) + (*value & 0x0FU));
}

static inline uint8_t i915_bin_to_bcd(uint8_t value)
{
    return (uint8_t)(((value / 10U) << 4) | (value % 10U));
}

static uint32_t i915_pci_read32_raw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xFCU);
    i915_outl(I915_PCI_CONFIG_ADDRESS, address);
    i915_io_delay();
    return i915_inl(I915_PCI_CONFIG_DATA);
}

static void i915_pci_write32_raw(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xFCU);
    i915_outl(I915_PCI_CONFIG_ADDRESS, address);
    i915_io_delay();
    i915_outl(I915_PCI_CONFIG_DATA, value);
}

static uint8_t i915_pci_find_bus_slot_func(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)
{
    uint32_t found = 0;
    for (uint8_t b = 0; b < 8; b++) {
        for (uint8_t s = 0; s < 32; s++) {
            for (uint8_t f = 0; f < 8; f++) {
                uint32_t id = i915_pci_read32_raw(b, s, f, I915_PCI_REG_VENDOR_DEVICE);
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

static volatile uint8_t i915_ready;
static volatile uint32_t i915_init_counter;
static struct i915_info i915_dev;
static struct i915_stats i915_stats;
static void (*i915_irq_handlers[16])(uint8_t irq) = {0};

static volatile uint32_t *i915_gfx_mmio = 0;
static volatile uint32_t *i915_fb = (volatile uint32_t *)I915_FB_ADDR_DEFAULT;
static uint32_t i915_gfx_ring[I915_RING_SIZE];
static uint32_t i915_gfx_ring_head = 0;
static uint32_t i915_gfx_ring_tail = 0;

int i915_init(void)
{
    if (i915_ready) return 1;
    i915_init_counter++;
    i915_memset(&i915_dev, 0, sizeof(i915_dev));
    i915_memset(&i915_stats, 0, sizeof(i915_stats));
    i915_memset(i915_irq_handlers, 0, sizeof(i915_irq_handlers));
    i915_memset(i915_gfx_ring, 0, sizeof(i915_gfx_ring));
    i915_gfx_ring_head = 0;
    i915_gfx_ring_tail = 0;

    i915_chipset_probe();
    i915_chipset_configure_memory();
    i915_acpi_available();
    i915_smbus_init();
    i915_agp_init();
    i915_graphics_init();
    i915_audio_init();
    i915_ide_init();
    i915_sata_init();
    i915_usb_init();
    i915_tco_init();
    i915_rtc_init();
    i915_hwmon_init();

    i915_dev.initialized = 1;
    i915_ready = 1;
    return 1;
}

int i915_is_ready(void)
{
    return i915_ready;
}

void i915_shutdown(void)
{
    i915_acpi_shutdown();
    i915_ready = 0;
}

void i915_reset(void)
{
    i915_acpi_reboot();
}

uint32_t i915_get_chipset_id(void)
{
    return ((uint32_t)i915_dev.mch_device_id << 16) | i915_dev.ich_device_id;
}

void i915_info_get(struct i915_info *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_dev, sizeof(i915_dev));
}

void i915_stats_get(struct i915_stats *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_stats, sizeof(i915_stats));
}

int i915_pci_find_device(uint16_t vendor, uint16_t device, uint8_t index, uint8_t *bus, uint8_t *slot, uint8_t *func)
{
    if (!bus || !slot || !func) return 0;
    return i915_pci_find_bus_slot_func(vendor, device, index, bus, slot, func);
}

uint32_t i915_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    i915_stats.pci_reads++;
    return i915_pci_read32_raw(bus, slot, func, offset);
}

void i915_pci_write32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value)
{
    i915_stats.pci_writes++;
    i915_pci_write32_raw(bus, slot, func, offset, value);
}

uint16_t i915_pci_read16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    i915_stats.pci_reads++;
    return (uint16_t)i915_pci_read32_raw(bus, slot, func, offset & 0xFCU);
}

void i915_pci_write16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value)
{
    i915_stats.pci_writes++;
    uint32_t reg = i915_pci_read32_raw(bus, slot, func, offset & 0xFCU);
    uint8_t shift = (offset & 0x02U) * 8U;
    reg = (reg & ~(0xFFFFU << shift)) | ((uint32_t)value << shift);
    i915_pci_write32_raw(bus, slot, func, offset & 0xFCU, reg);
}

uint8_t i915_pci_read8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    i915_stats.pci_reads++;
    return (uint8_t)(i915_pci_read32_raw(bus, slot, func, offset & 0xFCU) >> ((offset & 0x03U) * 8U));
}

void i915_pci_write8(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t value)
{
    i915_stats.pci_writes++;
    uint32_t reg = i915_pci_read32_raw(bus, slot, func, offset & 0xFCU);
    uint8_t shift = (offset & 0x03U) * 8U;
    reg = (reg & ~(0xFFU << shift)) | ((uint32_t)value << shift);
    i915_pci_write32_raw(bus, slot, func, offset & 0xFCU, reg);
}

int i915_pci_set_bus_master(uint8_t bus, uint8_t slot, uint8_t func, uint8_t enable)
{
    uint16_t command = i915_pci_read16(bus, slot, func, I915_PCI_REG_COMMAND);
    if (enable) command |= 0x0004U;
    else command &= (uint16_t)~0x0004U;
    i915_pci_write16(bus, slot, func, I915_PCI_REG_COMMAND, command);
    return 1;
}

uint8_t i915_pci_read_revision(uint8_t bus, uint8_t slot, uint8_t func)
{
    return (uint8_t)i915_pci_read8(bus, slot, func, I915_PCI_REG_REVISION);
}

int i915_pci_route_irq(uint8_t bus, uint8_t slot, uint8_t func, uint8_t *irq)
{
    if (!irq) return 0;
    uint8_t pin = i915_pci_read8(bus, slot, func, I915_PCI_REG_INTERRUPT_PIN);
    *irq = i915_pci_read8(bus, slot, func, I915_PCI_REG_INTERRUPT_LINE);
    (void)pin;
    return pin != 0U;
}

int i915_chipset_probe(void)
{
    uint8_t bus, slot, func;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func)) {
        i915_dev.mch_device_id = I915_DEVICE_MCH_915G;
        i915_dev.mch_revision = i915_pci_read_revision(bus, slot, func);
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        i915_dev.mch_device_id = I915_DEVICE_MCH_915P;
        i915_dev.mch_revision = i915_pci_read_revision(bus, slot, func);
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915X, 0, &bus, &slot, &func)) {
        i915_dev.mch_device_id = I915_DEVICE_MCH_915X;
        i915_dev.mch_revision = i915_pci_read_revision(bus, slot, func);
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915GV, 0, &bus, &slot, &func)) {
        i915_dev.mch_device_id = I915_DEVICE_MCH_915GV;
        i915_dev.mch_revision = i915_pci_read_revision(bus, slot, func);
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915GL, 0, &bus, &slot, &func)) {
        i915_dev.mch_device_id = I915_DEVICE_MCH_915GL;
        i915_dev.mch_revision = i915_pci_read_revision(bus, slot, func);
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else {
        i915_dev.mch_device_id = 0xFFFFU;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_LPC, 0, &bus, &slot, &func)) {
        i915_dev.ich_device_id = I915_DEVICE_ICH6_LPC;
        i915_dev.ich_revision = i915_pci_read_revision(bus, slot, func);
        i915_dev.acpi_base = i915_pci_read32(bus, slot, func, I915_ICH6_REG_PMBASE) & 0xFFFFFF80U;
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else {
        i915_dev.ich_device_id = 0xFFFFU;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_GRAPHICS_915G, 0, &bus, &slot, &func)) {
        i915_dev.graphics_device_id = I915_DEVICE_GRAPHICS_915G;
        i915_dev.graphics_revision = i915_pci_read_revision(bus, slot, func);
        i915_dev.gfx_mmio_base = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR0) & 0xFFFFFFC0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else {
        i915_dev.graphics_device_id = 0xFFFFU;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_SMBUS, 0, &bus, &slot, &func)) {
        i915_dev.smbus_base = i915_pci_read32(bus, slot, func, 0x04U) & 0xFFFFFFF0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
    } else {
        i915_dev.smbus_base = 0;
    }
    i915_chipset_configure_memory();
    return 1;
}

void i915_chipset_configure_memory(void)
{
    uint8_t bus, slot, func;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915X, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915GV, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915GL, 0, &bus, &slot, &func)) {
        i915_pci_write8(bus, slot, func, I915_MCH_REG_GGC, i915_pci_read8(bus, slot, func, I915_MCH_REG_GGC) | 0x01U);
        i915_pci_write8(bus, slot, func, I915_MCH_REG_DEVEN, i915_pci_read8(bus, slot, func, I915_MCH_REG_DEVEN) | 0x03U);
    }
}

int i915_pam_read(uint8_t reg, uint8_t *value)
{
    uint8_t bus, slot, func;
    if (!value) return 0;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    *value = i915_pci_read8(bus, slot, func, reg);
    i915_stats.memory_ops++;
    return 1;
}

int i915_pam_write(uint8_t reg, uint8_t value)
{
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    i915_pci_write8(bus, slot, func, reg, value);
    i915_stats.memory_ops++;
    return 1;
}

int i915_memory_map(struct i915_dram_info *out)
{
    uint8_t bus, slot, func;
    i915_memset(out, 0, sizeof(*out));
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    uint32_t drb0 = i915_pci_read8(bus, slot, func, I915_MCH_REG_DRB0);
    uint32_t drb1 = i915_pci_read8(bus, slot, func, I915_MCH_REG_DRB1);
    uint32_t drb2 = i915_pci_read8(bus, slot, func, I915_MCH_REG_DRB2);
    uint32_t drb3 = i915_pci_read8(bus, slot, func, I915_MCH_REG_DRB3);
    uint32_t total = ((drb0 | (drb1 << 8) | (drb2 << 16) | (drb3 << 24)) & 0x3FFFFFFFU) * 32U * 1024U * 1024U;
    out->total_mb = total / (1024U * 1024U);
    out->channels = 1U;
    out->ddr2 = 1U;
    out->dual_channel = 0U;
    out->ecc = 0U;
    out->rank_count = 1U;
    out->remap_base = 0U;
    out->remap_limit = 0U;
    i915_dev.dram = *out;
    i915_stats.memory_ops++;
    return 1;
}

int i915_memory_remap_enable(uint8_t enable)
{
    (void)enable;
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    i915_stats.memory_ops++;
    return 1;
}

int i915_memory_set_timings(uint32_t tcl, uint32_t trcd, uint32_t trp, uint32_t tras)
{
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    i915_dev.dram.tcl = tcl;
    i915_dev.dram.trcd = trcd;
    i915_dev.dram.trp = trp;
    i915_dev.dram.tras = tras;
    i915_stats.memory_ops++;
    return 1;
}

int i915_memory_get_timings(uint32_t *tcl, uint32_t *trcd, uint32_t *trp, uint32_t *tras)
{
    if (!tcl || !trcd || !trp || !tras) return 0;
    *tcl = i915_dev.dram.tcl;
    *trcd = i915_dev.dram.trcd;
    *trp = i915_dev.dram.trp;
    *tras = i915_dev.dram.tras;
    return 1;
}

int i915_memory_scrub(uint32_t start, uint32_t size)
{
    volatile uint8_t *p = (volatile uint8_t *)start;
    for (uint32_t i = 0; i < size; i++) p[i] = 0;
    i915_stats.memory_ops++;
    return 1;
}

int i915_memory_interrupt_enable(uint8_t enable)
{
    (void)enable;
    return 1;
}

int i915_gme_enable(uint8_t enable)
{
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    uint8_t ggc = i915_pci_read8(bus, slot, func, I915_MCH_REG_GGC);
    if (enable) ggc |= 0x01U;
    else ggc &= (uint8_t)~0x01U;
    i915_pci_write8(bus, slot, func, I915_MCH_REG_GGC, ggc);
    return 1;
}

uint32_t i915_stolen_memory_size(void)
{
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) &&
        !i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func)) {
        return 0;
    }
    uint8_t stolen = i915_pci_read8(bus, slot, func, I915_MCH_REG_STOLEN);
    return (uint32_t)((stolen & 0x3FU) + 1U) * 32U * 1024U;
}

int i915_agp_init(void)
{
    uint8_t bus, slot, func;
    i915_dev.agp_aperture = 0;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915G, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915P, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915X, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915GV, 0, &bus, &slot, &func) ||
        i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_MCH_915GL, 0, &bus, &slot, &func)) {
        i915_dev.agp_aperture = i915_pci_read32(bus, slot, func, 0x84U) & 0xFFFFF000U;
        if (i915_dev.agp_aperture == 0U) i915_dev.agp_aperture = 64U * 1024U * 1024U;
        i915_dev.agp_available = 1;
        return 1;
    }
    return 0;
}

uint32_t i915_agp_aperture(void)
{
    return i915_dev.agp_aperture;
}

int i915_agp_bind(uint32_t gatt_index, uint32_t physical_addr)
{
    (void)gatt_index;
    (void)physical_addr;
    if (!i915_dev.agp_available) return 0;
    return 1;
}

int i915_agp_unbind(uint32_t gatt_index)
{
    (void)gatt_index;
    if (!i915_dev.agp_available) return 0;
    return 1;
}

static uint32_t i915_acpi_base_static;

int i915_acpi_available(void)
{
    i915_acpi_base_static = 0;
    i915_dev.acpi.available = 0;
    i915_dev.acpi.base = 0;
    i915_dev.acpi.sci_enabled = 0;
    i915_dev.acpi.tco_available = 0;
    if (i915_dev.acpi_base != 0U && i915_dev.acpi_base != 0xFFFFFFFFU) {
        i915_acpi_base_static = i915_dev.acpi_base;
        i915_dev.acpi.base = i915_dev.acpi_base;
        i915_outw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT, i915_inw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT) | I915_ACPI_SCI_EN);
        i915_dev.acpi.available = 1;
        i915_dev.acpi.sci_enabled = 1;
    }
    return i915_dev.acpi.available;
}

void i915_acpi_sleep(uint8_t state)
{
    if (!i915_dev.acpi.available) return;
    uint16_t val = i915_inw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT);
    val &= (uint16_t)~I915_ACPI_SLP_EN;
    val |= (uint16_t)((state & 0x07U) << 10U);
    i915_outw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT, val | I915_ACPI_SLP_EN);
    for (volatile uint32_t wait = 0; wait < 100000U; wait++);
}

void i915_acpi_reboot(void)
{
    i915_outb(0xCF9U, 0x02U);
    for (volatile uint32_t wait = 0; wait < 1000000U; wait++);
    i915_outb(0x64U, 0xFEU);
}

void i915_acpi_shutdown(void)
{
    if (i915_dev.acpi.available) {
        i915_acpi_sleep(5U);
    }
    i915_outb(0x64U, 0xFEU);
}

int i915_acpi_sci_enable(uint8_t enable)
{
    if (!i915_dev.acpi.available) return 0;
    uint16_t val = i915_inw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT);
    if (enable) val |= I915_ACPI_SCI_EN;
    else val &= (uint16_t)~I915_ACPI_SCI_EN;
    i915_outw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT, val);
    i915_dev.acpi.sci_enabled = enable;
    i915_stats.acpi_events++;
    return 1;
}

uint16_t i915_acpi_read_pm1a_cnt(void)
{
    if (!i915_dev.acpi.available) return 0;
    return i915_inw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT);
}

void i915_acpi_write_pm1a_cnt(uint16_t value)
{
    if (!i915_dev.acpi.available) return;
    i915_outw(i915_dev.acpi_base + I915_ACPI_PM1A_CNT, value);
}

int i915_tco_init(void)
{
    i915_dev.acpi.tco_available = 1;
    return 1;
}

int i915_tco_is_ready(void)
{
    return i915_dev.acpi.tco_available;
}

int i915_tco_watchdog_start(uint32_t seconds)
{
    (void)seconds;
    if (!i915_dev.acpi.available) return 0;
    i915_outw(i915_dev.acpi_base + I915_ACPI_TCO_CNT, i915_inw(i915_dev.acpi_base + I915_ACPI_TCO_CNT) | 0x0002U);
    return 1;
}

int i915_tco_watchdog_feed(void)
{
    if (!i915_dev.acpi.available) return 0;
    i915_outw(i915_dev.acpi_base + I915_ACPI_TCO_CNT, i915_inw(i915_dev.acpi_base + I915_ACPI_TCO_CNT) | 0x0001U);
    return 1;
}

int i915_tco_watchdog_stop(void)
{
    if (!i915_dev.acpi.available) return 0;
    i915_outw(i915_dev.acpi_base + I915_ACPI_TCO_CNT, i915_inw(i915_dev.acpi_base + I915_ACPI_TCO_CNT) & (uint16_t)~0x0002U);
    return 1;
}

int i915_tco_irq_status(void)
{
    if (!i915_dev.acpi.available) return 0;
    return (i915_inw(i915_dev.acpi_base + I915_ACPI_TCO_STS) & 0x0001U) != 0U;
}

int i915_gpio_read(uint8_t pin, uint8_t *value)
{
    if (!value) return 0;
    uint8_t port = 0x060U + (pin / 8U);
    uint8_t bit = pin % 8U;
    *value = (i915_inb((uint16_t)port) >> bit) & 0x01U;
    return 1;
}

int i915_gpio_write(uint8_t pin, uint8_t value)
{
    uint8_t port = 0x060U + (pin / 8U);
    uint8_t bit = pin % 8U;
    uint8_t cur = i915_inb((uint16_t)port);
    cur = value ? (uint8_t)(cur | (0x01U << bit)) : (uint8_t)(cur & (uint8_t)~(0x01U << bit));
    i915_outb((uint16_t)port, cur);
    return 1;
}

int i915_gpio_direction(uint8_t pin, uint8_t output)
{
    (void)pin;
    (void)output;
    return 1;
}

int i915_pirq_read_router(uint8_t pirq, uint8_t *route)
{
    if (!route) return 0;
    *route = i915_pci_read8(0U, 0U, 0U, 0x60U + pirq);
    return 1;
}

int i915_pirq_write_router(uint8_t pirq, uint8_t route)
{
    i915_pci_write8(0U, 0U, 0U, 0x60U + pirq, route);
    return 1;
}

int i915_smi_enable(uint8_t enable)
{
    (void)enable;
    return 1;
}

uint8_t i915_smi_status(void)
{
    return 0;
}

int i915_bios_lock(void)
{
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_LPC, 0, &bus, &slot, &func)) return 0;
    uint8_t val = i915_pci_read8(bus, slot, func, 0xDCU);
    i915_pci_write8(bus, slot, func, 0xDCU, (uint8_t)(val | 0x01U));
    return 1;
}

int i915_bios_unlock(void)
{
    uint8_t bus, slot, func;
    if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_LPC, 0, &bus, &slot, &func)) return 0;
    uint8_t val = i915_pci_read8(bus, slot, func, 0xDCU);
    i915_pci_write8(bus, slot, func, 0xDCU, (uint8_t)(val & (uint8_t)~0x01U));
    return 1;
}

static uint32_t i915_smbus_base_static;

int i915_smbus_init(void)
{
    i915_smbus_base_static = 0;
    uint8_t bus, slot, func;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_SMBUS, 0, &bus, &slot, &func)) {
        i915_smbus_base_static = i915_pci_read32(bus, slot, func, 0x04U) & 0xFFFFFFF0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
        i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_C, I915_SMBUS_HOST_KILL);
        i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_C, 0x00U);
        return 1;
    }
    return 0;
}

int i915_smbus_read_byte(uint8_t device, uint8_t offset, uint8_t *value)
{
    if (!i915_smbus_base_static || !value) return 0;
    uint8_t index = (device << 1) | 0x01U;
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_A, index);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, offset);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, I915_SMBUS_HOST_BYTE_DATA);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_C, I915_SMBUS_HOST_START);
    for (uint32_t i = 0; i < 10000U; i++) {
        if (!(i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S) & 0x01U)) {
            *value = i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_D0);
            i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S, i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S));
            i915_stats.smbus_ops++;
            return 1;
        }
    }
    i915_stats.hwmon.errors++;
    return 0;
}

int i915_smbus_write_byte(uint8_t device, uint8_t offset, uint8_t value)
{
    if (!i915_smbus_base_static) return 0;
    uint8_t index = device << 1;
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_A, index);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, offset);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_D0, value);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, I915_SMBUS_HOST_BYTE_DATA);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_C, I915_SMBUS_HOST_START);
    for (uint32_t i = 0; i < 10000U; i++) {
        if (!(i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S) & 0x01U)) {
            i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S, i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S));
            i915_stats.smbus_ops++;
            return 1;
        }
    }
    i915_stats.hwmon.errors++;
    return 0;
}

int i915_smbus_read_word(uint8_t device, uint8_t offset, uint16_t *value)
{
    if (!i915_smbus_base_static || !value) return 0;
    uint8_t index = (device << 1) | 0x01U;
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_A, index);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, offset);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, I915_SMBUS_HOST_WORD_DATA);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_C, I915_SMBUS_HOST_START);
    for (uint32_t i = 0; i < 10000U; i++) {
        if (!(i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S) & 0x01U)) {
            *value = i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_D0) | ((uint16_t)i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_D1) << 8);
            i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S, i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S));
            i915_stats.smbus_ops++;
            return 1;
        }
    }
    return 0;
}

int i915_smbus_write_word(uint8_t device, uint8_t offset, uint16_t value)
{
    if (!i915_smbus_base_static) return 0;
    uint8_t index = device << 1;
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_A, index);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, offset);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_D0, (uint8_t)(value & 0xFFU));
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_D1, (uint8_t)((value >> 8) & 0xFFU));
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_CMD, I915_SMBUS_HOST_WORD_DATA);
    i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_C, I915_SMBUS_HOST_START);
    for (uint32_t i = 0; i < 10000U; i++) {
        if (!(i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S) & 0x01U)) {
            i915_outb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S, i915_inb(i915_smbus_base_static + I915_SMBUS_REG_HOST_S));
            i915_stats.smbus_ops++;
            return 1;
        }
    }
    return 0;
}

int i915_smbus_spd_read(uint8_t offset, uint8_t *value)
{
    return i915_smbus_read_byte(0xA0U, offset, value);
}

int i915_smbus_spd_write(uint8_t offset, uint8_t value)
{
    return i915_smbus_write_byte(0xA0U, offset, value);
}

void i915_smbus_stats_get(void)
{
    i915_stats.smbus_ops = i915_stats.smbus_ops;
}

int i915_rtc_init(void)
{
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_B);
    uint8_t b = i915_inb(I915_RTC_DATA) | I915_RTC_STATUS_B_24H | I915_RTC_STATUS_B_DM_BINARY;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_B);
    i915_outb(I915_RTC_DATA, b);
    i915_dev.rtc_ready = 1;
    return 1;
}

int i915_rtc_is_ready(void)
{
    return i915_dev.rtc_ready;
}

int i915_rtc_get_time(struct i915_rtc_time *out)
{
    if (!i915_dev.rtc_ready || !out) return 0;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_A);
    while (i915_inb(I915_RTC_DATA) & 0x80U);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_SECONDS);
    out->seconds = i915_inb(I915_RTC_DATA);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_MINUTES);
    out->minutes = i915_inb(I915_RTC_DATA);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_HOURS);
    out->hours = i915_inb(I915_RTC_DATA);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_DAY_OF_WEEK);
    out->day_of_week = i915_inb(I915_RTC_DATA);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_DAY_OF_MONTH);
    out->day_of_month = i915_inb(I915_RTC_DATA);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_MONTH);
    out->month = i915_inb(I915_RTC_DATA);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_YEAR);
    out->year = i915_inb(I915_RTC_DATA);
    return 1;
}

int i915_rtc_set_time(const struct i915_rtc_time *t)
{
    if (!i915_dev.rtc_ready || !t) return 0;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_B);
    i915_outb(I915_RTC_DATA, i915_inb(I915_RTC_DATA) | 0x80U);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_SECONDS);
    i915_outb(I915_RTC_DATA, t->seconds);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_MINUTES);
    i915_outb(I915_RTC_DATA, t->minutes);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_HOURS);
    i915_outb(I915_RTC_DATA, t->hours);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_DAY_OF_WEEK);
    i915_outb(I915_RTC_DATA, t->day_of_week);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_DAY_OF_MONTH);
    i915_outb(I915_RTC_DATA, t->day_of_month);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_MONTH);
    i915_outb(I915_RTC_DATA, t->month);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_YEAR);
    i915_outb(I915_RTC_DATA, t->year);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_B);
    i915_outb(I915_RTC_DATA, i915_inb(I915_RTC_DATA) & (uint8_t)~0x80U);
    return 1;
}

int i915_rtc_nvram_read(uint8_t offset, uint8_t *value)
{
    if (!i915_dev.rtc_ready || !value) return 0;
    i915_outb(I915_RTC_INDEX, offset);
    *value = i915_inb(I915_RTC_DATA);
    return 1;
}

int i915_rtc_nvram_write(uint8_t offset, uint8_t value)
{
    if (!i915_dev.rtc_ready) return 0;
    i915_outb(I915_RTC_INDEX, offset);
    i915_outb(I915_RTC_DATA, value);
    return 1;
}

int i915_rtc_alarm_set(const struct i915_rtc_time *t)
{
    if (!i915_dev.rtc_ready || !t) return 0;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_ALARM_SECONDS);
    i915_outb(I915_RTC_DATA, t->seconds);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_ALARM_MINUTES);
    i915_outb(I915_RTC_DATA, t->minutes);
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_ALARM_HOURS);
    i915_outb(I915_RTC_DATA, t->hours);
    return 1;
}

int i915_rtc_irq_enable(uint8_t mask, uint8_t enable)
{
    if (!i915_dev.rtc_ready) return 0;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_B);
    uint8_t b = i915_inb(I915_RTC_DATA);
    if (enable) b |= mask;
    else b &= (uint8_t)~mask;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_B);
    i915_outb(I915_RTC_DATA, b);
    return 1;
}

uint8_t i915_rtc_irq_status(void)
{
    if (!i915_dev.rtc_ready) return 0;
    i915_outb(I915_RTC_INDEX, I915_RTC_REG_STATUS_C);
    return i915_inb(I915_RTC_DATA);
}

static uint16_t i915_sio_base;
static uint8_t i915_hwmon_ready;

static void i915_sio_select_ldn(uint8_t ldn)
{
    i915_outb(I915_SIO_INDEX, I915_SIO_REG_CONFIG);
    i915_outb(I915_SIO_DATA, ldn);
}

static uint8_t i915_sio_read_reg(uint8_t reg)
{
    i915_outb(I915_SIO_INDEX, reg);
    return i915_inb(I915_SIO_DATA);
}

static void i915_sio_write_reg(uint8_t reg, uint8_t value)
{
    i915_outb(I915_SIO_INDEX, reg);
    i915_outb(I915_SIO_DATA, value);
}

int i915_hwmon_init(void)
{
    i915_sio_base = I915_SIO_INDEX;
    i915_hwmon_ready = 0;
    i915_outb(I915_SIO_INDEX, I915_SIO_KEY_ENTER);
    i915_outb(I915_SIO_INDEX, I915_SIO_KEY_ENTER);
    uint8_t id = i915_sio_read_reg(I915_SIO_REG_CHIP_ID);
    i915_sio_select_ldn(I915_SIO_LDN_HWMON);
    uint8_t enable = i915_sio_read_reg(0x30U);
    i915_sio_write_reg(0x30U, (uint8_t)(enable | 0x01U));
    i915_outb(I915_SIO_INDEX, I915_SIO_KEY_EXIT);
    i915_hwmon_ready = 1;
    i915_dev.hwmon_ready = 1;
    (void)id;
    return 1;
}

int i915_hwmon_is_ready(void)
{
    return i915_hwmon_ready;
}

int i915_hwmon_read_temperature(uint8_t sensor, int32_t *millicelsius)
{
    if (!i915_hwmon_ready || !millicelsius) return 0;
    i915_sio_select_ldn(I915_SIO_LDN_HWMON);
    uint8_t base_hi = i915_sio_read_reg(0x60U);
    uint8_t base_lo = i915_sio_read_reg(0x61U);
    uint16_t base = ((uint16_t)base_hi << 8) | base_lo;
    if (base == 0) return 0;
    i915_outb(base, 0x27U + sensor);
    uint8_t raw = i915_inb(base + 1U);
    *millicelsius = (int32_t)((int8_t)raw) * 1000;
    i915_stats.hwmon.temperatures++;
    return 1;
}

int i915_hwmon_read_voltage(uint8_t sensor, int32_t *millivolt)
{
    (void)sensor;
    if (!i915_hwmon_ready || !millivolt) return 0;
    *millivolt = 0;
    i915_stats.hwmon.voltages++;
    return 1;
}

int i915_hwmon_read_fan(uint8_t fan, uint32_t *rpm)
{
    (void)fan;
    if (!i915_hwmon_ready || !rpm) return 0;
    *rpm = 0;
    i915_stats.hwmon.fans++;
    return 1;
}

void i915_hwmon_stats_get(struct i915_hwmon_stats *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_stats.hwmon, sizeof(i915_stats.hwmon));
}

static uint32_t i915_ide_base[2];
static uint32_t i915_ide_bm_base[2];
static uint32_t i915_ide_sata_base;
static uint8_t i915_ide_ready;
static uint8_t i915_sata_ready;
static uint8_t i915_ide_udma_mode[2] = {0U, 0U};

static inline uint8_t i915_wait_not_busy(uint16_t status_port, uint8_t mask, uint32_t tries)
{
    for (uint32_t i = 0; i < tries; i++) {
        if ((i915_inb(status_port) & mask) == 0U) return 1;
    }
    return 0;
}

int i915_ide_init(void)
{
    uint8_t bus, slot, func;
    i915_ide_base[0] = 0x1F0U;
    i915_ide_base[1] = 0x170U;
    i915_ide_bm_base[0] = 0;
    i915_ide_bm_base[1] = 0;
    i915_ide_ready = 0;
    i915_dev.sata_base = 0;
    i915_dev.ide.sata_ports = 0;
    i915_dev.ide.sata_mode = 0;
    i915_ide_sata_base = 0;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_IDE, 0, &bus, &slot, &func)) {
        i915_ide_bm_base[0] = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR4) & 0xFFFFFFF0U;
        i915_ide_bm_base[1] = i915_ide_bm_base[0] + 8U;
        i915_pci_set_bus_master(bus, slot, func, 1);
        i915_dev.ide_base[0] = i915_ide_bm_base[0];
        i915_dev.bm_ide_base[0] = i915_ide_bm_base[0];
        i915_ide_ready = 1;
        i915_dev.ide_ready = 1;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_SATA, 0, &bus, &slot, &func)) {
        i915_ide_sata_base = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR5) & 0xFFFFFFF0U;
        i915_dev.sata_base = i915_ide_sata_base;
        i915_dev.ide.sata_ports = 4U;
        i915_dev.ide.sata_mode = 1U;
        i915_sata_ready = 1;
        i915_pci_set_bus_master(bus, slot, func, 1);
    }
    i915_ide_udma_mode[0] = 5U;
    i915_ide_udma_mode[1] = 5U;
    i915_dev.ide.dma_available = 1;
    i915_dev.ide.ultra_dma_mode = 5U;
    return 1;
}

int i915_ide_is_ready(void)
{
    return i915_ide_ready;
}

int i915_ide_identify(uint8_t channel, uint8_t drive, uint16_t *buffer)
{
    if (!i915_ide_ready || channel > 1U || !buffer) return 0;
    uint16_t reg = i915_ide_base[channel];
    i915_outb(reg + I915_IDE_REG_DRIVE_HEAD, (uint8_t)(0xA0U | (drive << 4U)));
    i915_outb(reg + I915_IDE_REG_FEATURES, 0x00U);
    i915_outb(reg + I915_IDE_REG_COMMAND, I915_IDE_CMD_IDENTIFY);
    if (!i915_wait_not_busy(reg + I915_IDE_REG_COMMAND, 0x80U, 100000U)) return 0;
    uint8_t status = i915_inb(reg + I915_IDE_REG_COMMAND);
    if (status & 0x01U) return 0;
    i915_memset(buffer, 0, 512);
    for (uint32_t i = 0; i < 256; i++) buffer[i] = i915_inw(reg + I915_IDE_REG_DATA);
    i915_stats.ide.reads++;
    return 1;
}

int i915_ide_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)
{
    if (!i915_ide_ready || channel > 1U || !buffer || count == 0) return 0;
    uint16_t reg = i915_ide_base[channel];
    i915_outb(reg + I915_IDE_REG_SECCOUNT, count);
    i915_outb(reg + I915_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    i915_outb(reg + I915_IDE_REG_COMMAND, I915_IDE_CMD_READ);
    if (!i915_wait_not_busy(reg + I915_IDE_REG_COMMAND, 0x80U, 1000000U)) return 0;
    uint16_t *dst = (uint16_t *)buffer;
    for (uint32_t s = 0; s < count; s++) {
        if (!i915_wait_not_busy(reg + I915_IDE_REG_COMMAND, 0x88U, 1000000U)) { i915_stats.ide.errors++; return 0; }
        for (uint32_t i = 0; i < 256; i++) dst[s * 256 + i] = i915_inw(reg + I915_IDE_REG_DATA);
    }
    i915_stats.ide.reads++;
    i915_stats.ide.sectors_read += count;
    return (int)count;
}

int i915_ide_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)
{
    if (!i915_ide_ready || channel > 1U || !buffer || count == 0) return 0;
    uint16_t reg = i915_ide_base[channel];
    i915_outb(reg + I915_IDE_REG_SECCOUNT, count);
    i915_outb(reg + I915_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    i915_outb(reg + I915_IDE_REG_COMMAND, I915_IDE_CMD_WRITE);
    if (!i915_wait_not_busy(reg + I915_IDE_REG_COMMAND, 0x80U, 1000000U)) return 0;
    const uint16_t *src = (const uint16_t *)buffer;
    for (uint32_t s = 0; s < count; s++) {
        if (!i915_wait_not_busy(reg + I915_IDE_REG_COMMAND, 0x80U, 1000000U)) { i915_stats.ide.errors++; return 0; }
        for (uint32_t i = 0; i < 256; i++) i915_outw(reg + I915_IDE_REG_DATA, src[s * 256 + i]);
    }
    i915_outb(reg + I915_IDE_REG_COMMAND, I915_IDE_CMD_FLUSH);
    i915_stats.ide.writes++;
    i915_stats.ide.sectors_written += count;
    return (int)count;
}

int i915_ide_dma_read_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, void *buffer)
{
    if (!i915_ide_ready || channel > 1U || i915_ide_bm_base[channel] == 0) return 0;
    uint16_t bm = i915_ide_bm_base[channel];
    i915_outb(bm + I915_IDE_BM_REG_CMD, 0x00U);
    i915_outl(bm + I915_IDE_BM_REG_PRDT, reinterpret_cast<uint32_t>(buffer));
    i915_outb(bm + I915_IDE_BM_REG_CMD, I915_IDE_BM_CMD_START | I915_IDE_BM_CMD_READ);
    uint16_t reg = i915_ide_base[channel];
    i915_outb(reg + I915_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    i915_outb(reg + I915_IDE_REG_COMMAND, I915_IDE_CMD_READ_DMA);
    for (uint32_t i = 0; i < 1000000U; i++) {
        if (!(i915_inb(bm + I915_IDE_BM_REG_STATUS) & 0x02U)) break;
    }
    i915_outb(bm + I915_IDE_BM_REG_CMD, 0x00U);
    i915_stats.ide.dma_transfers++;
    i915_stats.ide.sectors_read += count;
    return (int)count;
}

int i915_ide_dma_write_sectors(uint8_t channel, uint8_t drive, uint32_t lba, uint8_t count, const void *buffer)
{
    if (!i915_ide_ready || channel > 1U || i915_ide_bm_base[channel] == 0) return 0;
    uint16_t bm = i915_ide_bm_base[channel];
    i915_outb(bm + I915_IDE_BM_REG_CMD, 0x00U);
    i915_outl(bm + I915_IDE_BM_REG_PRDT, reinterpret_cast<uint32_t>(buffer));
    i915_outb(bm + I915_IDE_BM_REG_CMD, I915_IDE_BM_CMD_START);
    uint16_t reg = i915_ide_base[channel];
    i915_outb(reg + I915_IDE_REG_LBA0, (uint8_t)(lba & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA1, (uint8_t)((lba >> 8) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_LBA2, (uint8_t)((lba >> 16) & 0xFFU));
    i915_outb(reg + I915_IDE_REG_DRIVE_HEAD, (uint8_t)(0xE0U | (drive << 4U) | ((lba >> 24) & 0x0FU)));
    i915_outb(reg + I915_IDE_REG_COMMAND, I915_IDE_CMD_WRITE_DMA);
    for (uint32_t i = 0; i < 1000000U; i++) {
        if (!(i915_inb(bm + I915_IDE_BM_REG_STATUS) & 0x02U)) break;
    }
    i915_outb(bm + I915_IDE_BM_REG_CMD, 0x00U);
    i915_stats.ide.dma_transfers++;
    i915_stats.ide.sectors_written += count;
    return (int)count;
}

int i915_ide_flush(uint8_t channel)
{
    if (!i915_ide_ready || channel > 1U) return 0;
    i915_outb(i915_ide_base[channel] + I915_IDE_REG_COMMAND, I915_IDE_CMD_FLUSH);
    return 1;
}

void i915_ide_reset(uint8_t channel)
{
    if (!i915_ide_ready || channel > 1U) return;
    i915_outb(i915_ide_base[channel] + 0x206U, 0x04U);
    i915_outb(i915_ide_base[channel] + 0x206U, 0x00U);
}

int i915_ide_set_dma(uint8_t channel, uint8_t drive, uint8_t mode)
{
    (void)channel;
    (void)drive;
    if (mode > 6U) return 0;
    i915_ide_udma_mode[channel] = mode;
    i915_dev.ide.ultra_dma_mode = mode;
    return 1;
}

uint8_t i915_ide_get_udma_mode(uint8_t channel)
{
    if (channel > 1U) return 0;
    return i915_ide_udma_mode[channel];
}

void i915_ide_stats_get(struct i915_ide_stats *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_stats.ide, sizeof(i915_stats.ide));
}

int i915_sata_init(void)
{
    i915_sata_ready = i915_dev.ide.sata_mode != 0U;
    return i915_sata_ready;
}

int i915_sata_is_ready(void)
{
    return i915_sata_ready;
}

uint8_t i915_sata_port_count(void)
{
    return i915_dev.ide.sata_ports;
}

int i915_sata_read_sectors(uint8_t port, uint32_t lba, uint8_t count, void *buffer)
{
    (void)port;
    (void)lba;
    (void)count;
    (void)buffer;
    if (!i915_sata_ready) return 0;
    return 0;
}

int i915_sata_write_sectors(uint8_t port, uint32_t lba, uint8_t count, const void *buffer)
{
    (void)port;
    (void)lba;
    (void)count;
    (void)buffer;
    if (!i915_sata_ready) return 0;
    return 0;
}

int i915_sata_set_mode(uint8_t port, uint8_t ahci_mode)
{
    (void)port;
    (void)ahci_mode;
    return i915_sata_ready;
}

static uint32_t i915_usb_base[4];
static uint8_t i915_usb_ready;
static uint8_t i915_usb_ports;

int i915_usb_init(void)
{
    uint8_t bus, slot, func;
    i915_usb_base[0] = 0;
    i915_usb_base[1] = 0;
    i915_usb_base[2] = 0;
    i915_usb_base[3] = 0;
    i915_usb_ready = 0;
    i915_usb_ports = 6U;
    int found = 0;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_USB1, 0, &bus, &slot, &func)) {
        i915_usb_base[0] = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR0) & 0xFFFFFFF0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_USB2, 0, &bus, &slot, &func)) {
        i915_usb_base[1] = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR0) & 0xFFFFFFF0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_USB3, 0, &bus, &slot, &func)) {
        i915_usb_base[2] = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR0) & 0xFFFFFFF0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_EHCI, 0, &bus, &slot, &func)) {
        i915_usb_base[3] = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR0) & 0xFFFFFFF0U;
        i915_dev.ehci_base = i915_usb_base[3];
        i915_pci_set_bus_master(bus, slot, func, 1);
        found = 1;
    }
    if (found) i915_usb_ready = 1;
    i915_dev.usb.ports = i915_usb_ports;
    i915_dev.usb.ehci_available = i915_usb_base[3] != 0U;
    i915_dev.usb_ready = i915_usb_ready;
    return found;
}

int i915_usb_is_ready(void)
{
    return i915_usb_ready;
}

uint8_t i915_usb_root_port_count(void)
{
    return i915_usb_ports;
}

int i915_usb_port_reset(uint8_t port)
{
    if (!i915_usb_ready || port > i915_usb_ports) return 0;
    uint32_t base = i915_usb_base[port / 2U];
    uint32_t port_reg = I915_UHC_REG_PORTSC1 + (uint32_t)(port % 2U) * 2U;
    if (base == 0) return 0;
    i915_outw(base + port_reg, i915_inw(base + port_reg) | 0x0010U);
    i915_stats.usb.resets++;
    return 1;
}

int i915_usb_port_power(uint8_t port, uint8_t on)
{
    if (!i915_usb_ready || port > i915_usb_ports) return 0;
    uint32_t base = i915_usb_base[port / 2U];
    uint32_t port_reg = I915_UHC_REG_PORTSC1 + (uint32_t)(port % 2U) * 2U;
    if (base == 0) return 0;
    uint16_t val = i915_inw(base + port_reg);
    val = on ? (val | 0x0040U) : (uint16_t)(val & ~0x0040U);
    i915_outw(base + port_reg, val);
    return 1;
}

uint32_t i915_usb_port_status(uint8_t port)
{
    if (!i915_usb_ready || port > i915_usb_ports) return 0;
    uint32_t base = i915_usb_base[port / 2U];
    uint32_t port_reg = I915_UHC_REG_PORTSC1 + (uint32_t)(port % 2U) * 2U;
    if (base == 0) return 0;
    return (uint32_t)i915_inw(base + port_reg);
}

int i915_usb_control_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length, void *data)
{
    (void)addr; (void)endpoint; (void)direction; (void)request_type; (void)request; (void)value; (void)index; (void)data;
    if (!i915_usb_ready || length == 0) return 0;
    i915_stats.usb.transfers++;
    return (int)length;
}

int i915_usb_bulk_transfer(uint8_t addr, uint8_t endpoint, uint8_t direction, uint32_t length, void *data)
{
    (void)addr; (void)endpoint; (void)direction; (void)data;
    if (!i915_usb_ready || length == 0) return 0;
    i915_stats.usb.transfers++;
    return (int)length;
}

void i915_usb_stats_get(struct i915_usb_stats *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_stats.usb, sizeof(i915_stats.usb));
}

static uint32_t i915_ac97_mixer_base;
static uint32_t i915_ac97_bm_base;
static uint8_t i915_audio_ready;
static uint8_t i915_ac97_volume[4] = {50U, 50U, 50U, 50U};
static uint8_t *i915_audio_playback_buffer;
static uint32_t i915_audio_playback_bytes;
static uint32_t i915_audio_playback_offset;
static uint8_t *i915_audio_capture_buffer;
static uint32_t i915_audio_capture_bytes;
static uint32_t i915_audio_capture_offset;

int i915_audio_init(void)
{
    uint8_t bus, slot, func;
    i915_ac97_mixer_base = 0;
    i915_ac97_bm_base = 0;
    i915_audio_ready = 0;
    i915_dev.audio.mixer_base = 0;
    i915_dev.audio.bm_base = 0;
    i915_dev.audio.codec_vendor = 0;
    i915_dev.audio.ready = 0;
    i915_dev.audio.volume[0] = 50U;
    i915_dev.audio.volume[1] = 50U;
    i915_dev.audio.volume[2] = 50U;
    i915_dev.audio.volume[3] = 50U;
    if (i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_ICH6_AC97, 0, &bus, &slot, &func)) {
        i915_ac97_mixer_base = i915_pci_read32(bus, slot, func, I915_AC97_MIXER_BAR_OFFSET) & 0xFFFFFFC0U;
        i915_ac97_bm_base = i915_pci_read32(bus, slot, func, I915_AC97_BM_BAR_OFFSET) & 0xFFFFFFC0U;
        i915_pci_set_bus_master(bus, slot, func, 1);
        i915_audio_codec_reset(1);
        i915_audio_set_volume(0U, 50U);
        i915_audio_set_volume(1U, 50U);
        i915_dev.audio.mixer_base = i915_ac97_mixer_base;
        i915_dev.audio.bm_base = i915_ac97_bm_base;
        i915_dev.audio.codec_vendor = 0x11DU;
        i915_audio_ready = 1;
        i915_dev.audio.ready = 1;
        return 1;
    }
    return 0;
}

int i915_audio_is_ready(void)
{
    return i915_audio_ready;
}

int i915_audio_codec_read(uint8_t reg, uint16_t *value)
{
    if (!i915_ac97_mixer_base || !value) return 0;
    i915_outw(i915_ac97_mixer_base + I915_AC97_REG_RESET + reg, 0x8000U);
    for (volatile uint32_t wait = 0; wait < 1000U; wait++);
    *value = i915_inw(i915_ac97_mixer_base + I915_AC97_REG_RESET + reg);
    i915_stats.audio.codec_reads++;
    return 1;
}

int i915_audio_codec_write(uint8_t reg, uint16_t value)
{
    if (!i915_ac97_mixer_base) return 0;
    i915_outw(i915_ac97_mixer_base + I915_AC97_REG_RESET + reg, value);
    i915_stats.audio.codec_writes++;
    return 1;
}

int i915_audio_codec_reset(int cold)
{
    if (!i915_ac97_mixer_base) return 0;
    if (cold) i915_outb(i915_ac97_mixer_base, 0x01U);
    else i915_outb(i915_ac97_mixer_base, 0x02U);
    for (volatile uint32_t wait = 0; wait < 10000U; wait++);
    i915_outb(i915_ac97_mixer_base, 0x00U);
    return 1;
}

int i915_audio_set_volume(uint8_t channel, uint8_t percent)
{
    if (channel > 3U) return 0;
    uint16_t att = (uint16_t)((100U - percent) * 2U);
    if (att > 0x3EU) att = 0x3EU;
    i915_ac97_volume[channel] = percent;
    i915_dev.audio.volume[channel] = percent;
    if (channel == 0U) i915_audio_codec_write(I915_AC97_REG_MASTER, (uint16_t)(att | (att << 8)));
    else if (channel == 1U) i915_audio_codec_write(I915_AC97_REG_MASTER_MONO, att);
    else if (channel == 2U) i915_audio_codec_write(I915_AC97_REG_PCM_OUT, (uint16_t)(att | (att << 8)));
    else i915_audio_codec_write(I915_AC97_REG_PCM_IN, (uint16_t)(att | (att << 8)));
    return 1;
}

uint8_t i915_audio_get_volume(uint8_t channel)
{
    if (channel > 3U) return 0;
    return i915_ac97_volume[channel];
}

int i915_audio_playback_start(const void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)
{
    (void)rate; (void)channels; (void)bits;
    if (!i915_audio_ready || !buffer || bytes == 0) return 0;
    i915_audio_playback_buffer = (uint8_t *)buffer;
    i915_audio_playback_bytes = bytes;
    i915_audio_playback_offset = 0;
    i915_outl(i915_ac97_bm_base, reinterpret_cast<uint32_t>(buffer));
    i915_outw(i915_ac97_bm_base + 4U, (uint16_t)bytes);
    i915_outb(i915_ac97_bm_base + 1U, 0x02U);
    return 1;
}

int i915_audio_playback_stop(void)
{
    if (!i915_audio_ready) return 0;
    i915_outb(i915_ac97_bm_base + 1U, 0x00U);
    i915_audio_playback_buffer = 0;
    i915_audio_playback_bytes = 0;
    i915_audio_playback_offset = 0;
    return 1;
}

int i915_audio_capture_start(void *buffer, uint32_t bytes, uint32_t rate, uint8_t channels, uint8_t bits)
{
    (void)rate; (void)channels; (void)bits;
    if (!i915_audio_ready || !buffer || bytes == 0) return 0;
    i915_audio_capture_buffer = (uint8_t *)buffer;
    i915_audio_capture_bytes = bytes;
    i915_audio_capture_offset = 0;
    i915_outl(i915_ac97_bm_base + 8U, reinterpret_cast<uint32_t>(buffer));
    i915_outw(i915_ac97_bm_base + 12U, (uint16_t)bytes);
    i915_outb(i915_ac97_bm_base + 9U, 0x02U);
    return 1;
}

int i915_audio_capture_stop(void)
{
    if (!i915_audio_ready) return 0;
    i915_outb(i915_ac97_bm_base + 9U, 0x00U);
    i915_audio_capture_buffer = 0;
    i915_audio_capture_bytes = 0;
    i915_audio_capture_offset = 0;
    return 1;
}

void i915_audio_stats_get(struct i915_audio_stats *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_stats.audio, sizeof(i915_stats.audio));
}

void i915_irq_handler(uint8_t irq)
{
    if (irq < 16U && i915_irq_handlers[irq]) {
        i915_irq_handlers[irq](irq);
        i915_stats.irq_dispatched++;
    }
}

int i915_irq_register(uint8_t irq, void (*handler)(uint8_t irq))
{
    if (irq > 15U || !handler) return 0;
    i915_irq_handlers[irq] = handler;
    return 1;
}

int i915_irq_unregister(uint8_t irq)
{
    if (irq > 15U) return 0;
    i915_irq_handlers[irq] = 0;
    return 1;
}

static inline uint32_t i915_mmio_read(uint32_t offset)
{
    return i915_gfx_mmio[offset >> 2];
}

static inline void i915_mmio_write(uint32_t offset, uint32_t value)
{
    i915_gfx_mmio[offset >> 2] = value;
}

static inline uint32_t i915_mmio_read_posted(uint32_t offset)
{
    uint32_t value = i915_mmio_read(offset);
    (void)i915_mmio_read(offset);
    return value;
}

static void i915_gfx_ring_emit(uint32_t dword)
{
    i915_gfx_ring[i915_gfx_ring_head & (I915_RING_SIZE - 1U)] = dword;
    i915_gfx_ring_head++;
}

static void i915_gfx_ring_flush(void)
{
    uint32_t tail = i915_gfx_ring_head & (I915_RING_SIZE - 1U);
    i915_mmio_write(I915_GFX_REG_RING_TAIL, tail);
    for (volatile int wait = 0; wait < 1000000; wait++) {
        if (i915_mmio_read(I915_GFX_REG_RING_HEAD) == tail) {
            break;
        }
    }
    i915_gfx_ring_tail = tail;
    i915_stats.graphics.ring_flushes++;
}

static void i915_gfx_blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (((i915_gfx_ring_head & (I915_RING_SIZE - 1U)) + 5U) > I915_RING_SIZE) {
        i915_gfx_ring_flush();
    }
    i915_gfx_ring_emit(0x78010000U);
    i915_gfx_ring_emit((y << 16) | x);
    i915_gfx_ring_emit(((h - 1U) << 16) | (w - 1U));
    i915_gfx_ring_emit(0U);
    i915_gfx_ring_emit(color);
    i915_gfx_ring_flush();
}

static void i915_gfx_blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    if (((i915_gfx_ring_head & (I915_RING_SIZE - 1U)) + 5U) > I915_RING_SIZE) {
        i915_gfx_ring_flush();
    }
    i915_gfx_ring_emit(0x74010000U);
    i915_gfx_ring_emit((dst_y << 16) | dst_x);
    i915_gfx_ring_emit(((h - 1U) << 16) | (w - 1U));
    i915_gfx_ring_emit(0U);
    i915_gfx_ring_emit((src_y << 16) | src_x);
    i915_gfx_ring_flush();
}

int i915_graphics_init(void)
{
    uint8_t bus, slot, func;
    i915_gfx_mmio = 0;
    i915_fb = (volatile uint32_t *)I915_FB_ADDR_DEFAULT;
    i915_dev.graphics.initialized = 0;
    i915_dev.graphics.mmio_base = 0;
    i915_dev.graphics.fb_addr = I915_FB_ADDR_DEFAULT;
    i915_dev.graphics.fb_size = I915_FB_WIDTH * I915_FB_HEIGHT * 4U;
    i915_dev.graphics.width = I915_FB_WIDTH;
    i915_dev.graphics.height = I915_FB_HEIGHT;
    i915_dev.graphics.pitch = I915_FB_PITCH;
    i915_dev.graphics.device_id = i915_dev.graphics_device_id;
    i915_dev.graphics.revision = i915_dev.graphics_revision;
    i915_dev.graphics.initialized = 0;
    i915_dev.graphics.overlay_available = 0;
    i915_dev.graphics.cursor_available = 0;
    i915_dev.graphics.double_buffer = 0;

    if (i915_dev.graphics_device_id == 0xFFFFU) {
        if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_GRAPHICS_915G, 0, &bus, &slot, &func)) {
            return 0;
        }
        i915_dev.graphics_device_id = I915_DEVICE_GRAPHICS_915G;
        i915_dev.graphics_revision = i915_pci_read_revision(bus, slot, func);
        i915_dev.graphics.device_id = i915_dev.graphics_device_id;
        i915_dev.graphics.revision = i915_dev.graphics_revision;
    }

    if (i915_dev.graphics_device_id != 0xFFFFU) {
        if (!i915_pci_find_bus_slot_func(I915_VENDOR_INTEL, I915_DEVICE_GRAPHICS_915G, 0, &bus, &slot, &func)) {
            return 0;
        }
        i915_dev.gfx_mmio_base = i915_pci_read32(bus, slot, func, I915_PCI_REG_BAR0) & 0xFFFFFFC0U;
        i915_dev.graphics.mmio_base = i915_dev.gfx_mmio_base;
        i915_pci_set_bus_master(bus, slot, func, 1);
    }

    if (i915_dev.gfx_mmio_base == 0U || i915_dev.gfx_mmio_base == 0xFFFFFFFFU) {
        return 0;
    }

    i915_gfx_mmio = (volatile uint32_t *)i915_dev.gfx_mmio_base;

    i915_mmio_write(I915_GFX_REG_VGA, i915_mmio_read(I915_GFX_REG_VGA) & ~0x1U);

    uint32_t dpll = i915_mmio_read(I915_GFX_REG_DPLL_A);
    i915_mmio_write(I915_GFX_REG_DPLL_A, dpll | 0x80000000U);
    for (volatile int wait = 0; wait < 100000; wait++) {
        if ((i915_mmio_read(I915_GFX_REG_DPLL_A) & 0x20000000U) != 0U) {
            break;
        }
    }

    uint32_t hblank = (((I915_FB_WIDTH + 128U) - 1U) << 16) | (I915_FB_WIDTH - 1U);
    uint32_t vblank = (((I915_FB_HEIGHT + 32U) - 1U) << 16) | (I915_FB_HEIGHT - 1U);
    uint32_t hsync = (((I915_FB_WIDTH + 64U) - 1U) << 16) | (I915_FB_WIDTH - 1U);
    uint32_t vsync = (((I915_FB_HEIGHT + 4U) - 1U) << 16) | (I915_FB_HEIGHT - 1U);

    i915_mmio_write(I915_GFX_REG_PIPEA_HBLANK, hblank);
    i915_mmio_write(I915_GFX_REG_PIPEA_VBLANK, vblank);
    i915_mmio_write(I915_GFX_REG_PIPEA_HSYNC, hsync);
    i915_mmio_write(I915_GFX_REG_PIPEA_VSYNC, vsync);
    i915_mmio_write(I915_GFX_REG_PIPEA_SRC, (I915_FB_WIDTH - 1U) | ((I915_FB_HEIGHT - 1U) << 16));
    i915_mmio_write(I915_GFX_REG_PIPEA_CONF, 0x80000000U);

    i915_mmio_write(I915_GFX_REG_DSPACNTR, 0x80000000U);
    i915_mmio_write(I915_GFX_REG_DSPABASE, I915_FB_ADDR_DEFAULT);
    i915_mmio_write(I915_GFX_REG_DSPASTRIDE, I915_FB_PITCH);

    i915_mmio_write(I915_GFX_REG_RING_CTL, 0x80000000U);
    i915_mmio_write(I915_GFX_REG_RING_START, (uint32_t)&i915_gfx_ring[0]);
    i915_mmio_write(I915_GFX_REG_RING_SIZE, sizeof(i915_gfx_ring));
    i915_mmio_write(I915_GFX_REG_RING_HEAD, 0U);
    i915_mmio_write(I915_GFX_REG_RING_TAIL, 0U);
    i915_gfx_ring_head = 0;
    i915_gfx_ring_tail = 0;

    i915_graphics_cursor_hide();

    i915_dev.graphics.initialized = 1;
    i915_dev.graphics.cursor_available = 1;
    i915_dev.graphics.double_buffer = 1;
    i915_dev.graphics_ready = 1;
    return 1;
}

int i915_graphics_is_ready(void)
{
    return i915_dev.graphics.initialized;
}

void i915_graphics_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= I915_FB_WIDTH || y >= I915_FB_HEIGHT) return;
    i915_fb[y * (I915_FB_PITCH / 4U) + x] = color;
    i915_stats.graphics.putpixels++;
}

void i915_graphics_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (i915_dev.graphics.initialized) {
        i915_gfx_blt_fill(x, y, w, h, color);
    } else {
        for (uint32_t yy = y; yy < y + h; yy++) {
            for (uint32_t xx = x; xx < x + w; xx++) {
                if (xx >= I915_FB_WIDTH || yy >= I915_FB_HEIGHT) continue;
                i915_fb[yy * (I915_FB_PITCH / 4U) + xx] = color;
            }
        }
    }
    i915_stats.graphics.fillrects++;
}

void i915_graphics_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    if (i915_dev.graphics.initialized) {
        i915_gfx_blt_copy(dst_x, dst_y, src_x, src_y, w, h);
    }
    i915_stats.graphics.blts++;
}

void i915_graphics_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    if (image) {
        i915_memcpy((void *)i915_fb, image, 64U * 64U * 4U);
    }
    i915_mmio_write(I915_GFX_REG_CURSOR_POS, (y << 16) | x);
    i915_mmio_write(I915_GFX_REG_CURSOR_BASE, I915_FB_ADDR_DEFAULT);
    i915_mmio_write(I915_GFX_REG_CURSOR_CTL, i915_mmio_read(I915_GFX_REG_CURSOR_CTL) | 0x80000000U);
    i915_stats.graphics.cursor_moves++;
}

void i915_graphics_cursor_move(uint32_t x, uint32_t y)
{
    i915_mmio_write(I915_GFX_REG_CURSOR_POS, (y << 16) | x);
    i915_stats.graphics.cursor_moves++;
}

void i915_graphics_cursor_hide(void)
{
    i915_mmio_write(I915_GFX_REG_CURSOR_CTL, i915_mmio_read(I915_GFX_REG_CURSOR_CTL) & ~0x80000000U);
}

void i915_graphics_overlay_enable(uint8_t enable)
{
    (void)enable;
    i915_dev.graphics.overlay_available = enable;
    i915_stats.graphics.overlay_updates++;
}

int i915_graphics_overlay_set_buffer(uint32_t addr, uint32_t width, uint32_t height, uint32_t pitch)
{
    (void)addr; (void)width; (void)height; (void)pitch;
    i915_mmio_write(I915_GFX_REG_OVADD, addr);
    i915_mmio_write(I915_GFX_REG_OVCTL, i915_mmio_read(I915_GFX_REG_OVCTL) | 0x00000001U);
    i915_stats.graphics.overlay_updates++;
    return 1;
}

void i915_graphics_display_timings(uint32_t width, uint32_t height, uint32_t refresh)
{
    (void)width; (void)height; (void)refresh;
}

void i915_graphics_pipe_config(uint8_t pipe, uint8_t enable)
{
    (void)pipe;
    (void)enable;
}

void i915_graphics_plane_config(uint8_t plane, uint32_t addr, uint32_t width, uint32_t height, uint32_t stride)
{
    (void)plane;
    (void)addr;
    (void)width;
    (void)height;
    (void)stride;
}

void i915_graphics_blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    i915_gfx_blt_fill(x, y, w, h, color);
}

void i915_graphics_blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    i915_gfx_blt_copy(dst_x, dst_y, src_x, src_y, w, h);
}

void *i915_graphics_get_fb(void)
{
    return (void *)i915_fb;
}

uint32_t i915_graphics_get_width(void)
{
    return I915_FB_WIDTH;
}

uint32_t i915_graphics_get_height(void)
{
    return I915_FB_HEIGHT;
}

uint32_t i915_graphics_get_pitch(void)
{
    return I915_FB_PITCH;
}

uint16_t i915_graphics_get_device_id(void)
{
    return i915_dev.graphics_device_id;
}

uint8_t i915_graphics_get_revision(void)
{
    return i915_dev.graphics_revision;
}

void i915_graphics_info_get(struct i915_graphics_info *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_dev.graphics, sizeof(i915_dev.graphics));
}

void i915_graphics_stats_get(struct i915_graphics_stats *out)
{
    if (!out) return;
    i915_memcpy(out, &i915_stats.graphics, sizeof(i915_stats.graphics));
}

void i915_graphics_vga_enable(uint8_t enable)
{
    if (enable) {
        i915_mmio_write(I915_GFX_REG_VGA, i915_mmio_read(I915_GFX_REG_VGA) | 0x1U);
    } else {
        i915_mmio_write(I915_GFX_REG_VGA, i915_mmio_read(I915_GFX_REG_VGA) & ~0x1U);
    }
}

void i915_graphics_ring_flush(void)
{
    i915_gfx_ring_flush();
}