#include "intel_uhd.h"

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

#define INTEL_UHD_REG_VGA 0x71400U
#define INTEL_UHD_REG_DPLL_A 0x70000U
#define INTEL_UHD_REG_PIPEA_CONF 0x70008U
#define INTEL_UHD_REG_PIPEA_HBLANK 0x7000CU
#define INTEL_UHD_REG_PIPEA_VBLANK 0x70010U
#define INTEL_UHD_REG_PIPEA_SRC 0x70014U
#define INTEL_UHD_REG_PIPEA_HSYNC 0x70018U
#define INTEL_UHD_REG_PIPEA_VSYNC 0x7001CU
#define INTEL_UHD_REG_PLANE_CONF 0x70180U
#define INTEL_UHD_REG_PLANE_SURF 0x70188U
#define INTEL_UHD_REG_PLANE_SURF_HIGH 0x7018CU
#define INTEL_UHD_REG_PLANE_STRIDE 0x70190U
#define INTEL_UHD_REG_PLANE_SIZE 0x70194U
#define INTEL_UHD_REG_PLANE_POS 0x70198U
#define INTEL_UHD_REG_TRANSCONF_A 0x60010U
#define INTEL_UHD_REG_RING_CTL 0x2030U
#define INTEL_UHD_REG_RING_START 0x2034U
#define INTEL_UHD_REG_RING_SIZE 0x2038U
#define INTEL_UHD_REG_RING_HEAD 0x203CU
#define INTEL_UHD_REG_RING_TAIL 0x2040U
#define INTEL_UHD_REG_CURSOR_CTL 0x70080U
#define INTEL_UHD_REG_CURSOR_POS 0x70084U
#define INTEL_UHD_REG_CURSOR_BASE 0x70088U

#define XY_FILL_BLT_OPCODE 0x78010000U
#define XY_SRC_COPY_BLT_OPCODE 0x74010000U
#define RING_SIZE 4096U

static volatile uint32_t *g_mmio = 0;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint16_t g_device_id = 0;
static uint8_t g_revision = 0;
static int g_initialized = 0;
static volatile uint32_t *g_fb = (volatile uint32_t *)INTEL_UHD_FB_ADDR;
static uint32_t g_ring[4096];
static uint32_t g_ring_head = 0;
static uint32_t g_ring_tail = 0;

static int intel_uhd_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read_config_dword((uint8_t)bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != INTEL_UHD_VENDOR_ID) continue;
                uint32_t class_code = pci_read_config_dword((uint8_t)bus, slot, func, 0x08);
                uint8_t base_class = (uint8_t)(class_code >> 24);
                if (base_class != 0x03) continue;
                uint32_t bar = pci_read_config_dword((uint8_t)bus, slot, func, 0x10);
                if (bar == 0 || bar == 0xFFFFFFFFU) continue;
                if ((bar & 1U) != 0) continue;
                g_bus = (uint8_t)bus;
                g_slot = slot;
                g_func = func;
                g_mmio_base = bar & 0xFFFFFFF0U;
                g_device_id = (uint16_t)(id0 >> 16);
                g_revision = (uint8_t)(class_code & 0xFFU);
                return 1;
            }
        }
    }
    return 0;
}

static inline uint32_t mmio_read(uint32_t offset)
{
    return g_mmio[offset >> 2];
}

static inline void mmio_write(uint32_t offset, uint32_t value)
{
    g_mmio[offset >> 2] = value;
}

static inline uint32_t mmio_read_posted(uint32_t offset)
{
    uint32_t value = mmio_read(offset);
    (void)mmio_read(offset);
    return value;
}

static void ring_emit(uint32_t dword)
{
    g_ring[g_ring_head & (RING_SIZE - 1U)] = dword;
    g_ring_head++;
}

static void ring_flush(void)
{
    uint32_t tail = g_ring_head & (RING_SIZE - 1U);
    mmio_write(INTEL_UHD_REG_RING_TAIL, tail);
    for (volatile int wait = 0; wait < 1000000; wait++) {
        if (mmio_read(INTEL_UHD_REG_RING_HEAD) == tail) {
            break;
        }
    }
    g_ring_tail = tail;
}

static void blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (((g_ring_head & (RING_SIZE - 1U)) + 5U) > RING_SIZE) {
        ring_flush();
    }
    ring_emit(XY_FILL_BLT_OPCODE);
    ring_emit((y << 16) | x);
    ring_emit(((h - 1U) << 16) | (w - 1U));
    ring_emit(0U);
    ring_emit(color);
    ring_flush();
}

static void blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    if (((g_ring_head & (RING_SIZE - 1U)) + 5U) > RING_SIZE) {
        ring_flush();
    }
    ring_emit(XY_SRC_COPY_BLT_OPCODE);
    ring_emit((dst_y << 16) | dst_x);
    ring_emit(((h - 1U) << 16) | (w - 1U));
    ring_emit(0U);
    ring_emit((src_y << 16) | src_x);
    ring_flush();
}

int intel_uhd_init(void)
{
    if (!intel_uhd_find_device()) {
        return 0;
    }

    uint32_t command = pci_read_config_dword(g_bus, g_slot, g_func, 0x04);
    pci_write_config_dword(g_bus, g_slot, g_func, 0x04, command | 0x00000006U);

    g_mmio = (volatile uint32_t *)g_mmio_base;

    mmio_write(INTEL_UHD_REG_VGA, mmio_read(INTEL_UHD_REG_VGA) & ~0x1U);

    uint32_t dpll = mmio_read(INTEL_UHD_REG_DPLL_A);
    mmio_write(INTEL_UHD_REG_DPLL_A, dpll | 0x80000000U);
    for (volatile int wait = 0; wait < 100000; wait++) {
        if ((mmio_read(INTEL_UHD_REG_DPLL_A) & 0x20000000U) != 0U) {
            break;
        }
    }

    uint32_t hblank = (((INTEL_UHD_FB_WIDTH + 128U) - 1U) << 16) | (INTEL_UHD_FB_WIDTH - 1U);
    uint32_t vblank = (((INTEL_UHD_FB_HEIGHT + 32U) - 1U) << 16) | (INTEL_UHD_FB_HEIGHT - 1U);
    uint32_t hsync = (((INTEL_UHD_FB_WIDTH + 64U) - 1U) << 16) | (INTEL_UHD_FB_WIDTH - 1U);
    uint32_t vsync = (((INTEL_UHD_FB_HEIGHT + 4U) - 1U) << 16) | (INTEL_UHD_FB_HEIGHT - 1U);

    mmio_write(INTEL_UHD_REG_PIPEA_HBLANK, hblank);
    mmio_write(INTEL_UHD_REG_PIPEA_VBLANK, vblank);
    mmio_write(INTEL_UHD_REG_PIPEA_HSYNC, hsync);
    mmio_write(INTEL_UHD_REG_PIPEA_VSYNC, vsync);
    mmio_write(INTEL_UHD_REG_PIPEA_SRC, (INTEL_UHD_FB_WIDTH - 1U) | ((INTEL_UHD_FB_HEIGHT - 1U) << 16));
    mmio_write(INTEL_UHD_REG_PIPEA_CONF, 0x80000000U);

    mmio_write(INTEL_UHD_REG_PLANE_CONF, 0x80000000U);
    mmio_write(INTEL_UHD_REG_PLANE_SURF, INTEL_UHD_FB_ADDR);
    mmio_write(INTEL_UHD_REG_PLANE_SURF_HIGH, 0U);
    mmio_write(INTEL_UHD_REG_PLANE_STRIDE, INTEL_UHD_FB_PITCH);
    mmio_write(INTEL_UHD_REG_PLANE_SIZE, ((INTEL_UHD_FB_HEIGHT - 1U) << 16) | (INTEL_UHD_FB_WIDTH - 1U));
    mmio_write(INTEL_UHD_REG_PLANE_POS, 0U);

    mmio_write(INTEL_UHD_REG_TRANSCONF_A, 0x80000000U);

    mmio_write(INTEL_UHD_REG_RING_CTL, 0x80000000U);
    mmio_write(INTEL_UHD_REG_RING_START, (uint32_t)&g_ring[0]);
    mmio_write(INTEL_UHD_REG_RING_SIZE, sizeof(g_ring));
    mmio_write(INTEL_UHD_REG_RING_HEAD, 0U);
    mmio_write(INTEL_UHD_REG_RING_TAIL, 0U);
    g_ring_head = 0;
    g_ring_tail = 0;

    for (volatile int wait = 0; wait < 100000; wait++);

    intel_uhd_cursor_hide();

    g_initialized = 1;
    return 1;
}

void intel_uhd_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= INTEL_UHD_FB_WIDTH || y >= INTEL_UHD_FB_HEIGHT) return;
    g_fb[y * (INTEL_UHD_FB_PITCH / 4U) + x] = color;
}

void intel_uhd_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (g_initialized) {
        blt_fill(x, y, w, h, color);
        return;
    }
    uint32_t xx;
    uint32_t yy;
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= INTEL_UHD_FB_WIDTH || yy >= INTEL_UHD_FB_HEIGHT) continue;
            g_fb[yy * (INTEL_UHD_FB_PITCH / 4U) + xx] = color;
        }
    }
}

void intel_uhd_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    if (g_initialized) {
        blt_copy(dst_x, dst_y, src_x, src_y, w, h);
    }
}

void intel_uhd_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    (void)image;
    mmio_write(INTEL_UHD_REG_CURSOR_POS, (y << 16) | x);
    mmio_write(INTEL_UHD_REG_CURSOR_BASE, INTEL_UHD_FB_ADDR);
    mmio_write(INTEL_UHD_REG_CURSOR_CTL, mmio_read(INTEL_UHD_REG_CURSOR_CTL) | 0x80000000U);
}

void intel_uhd_cursor_hide(void)
{
    mmio_write(INTEL_UHD_REG_CURSOR_CTL, mmio_read(INTEL_UHD_REG_CURSOR_CTL) & ~0x80000000U);
}

void *intel_uhd_get_fb(void)
{
    return (void *)g_fb;
}

uint32_t intel_uhd_get_width(void)
{
    return INTEL_UHD_FB_WIDTH;
}

uint32_t intel_uhd_get_height(void)
{
    return INTEL_UHD_FB_HEIGHT;
}

uint16_t intel_uhd_get_device_id(void)
{
    return g_device_id;
}

uint8_t intel_uhd_get_revision(void)
{
    return g_revision;
}
