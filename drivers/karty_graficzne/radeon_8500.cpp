/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/karty_graficzne/radeon_8500.h>

static inline uint8_t inb8500(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw8500(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t inl8500(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb8500(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw8500(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outl8500(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t pci_read32_8500(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl8500(0xCF8, address);
    return inl8500(0xCFC);
}

static void pci_write32_8500(uint8_t bus, uint8_t slot, uint8_t function,
                             uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl8500(0xCF8, address);
    outl8500(0xCFC, value);
}

static volatile uint32_t *g_mmio_8500 = 0;
static volatile uint32_t *g_fb_8500 = (volatile uint32_t *)RADEON_8500_FB_ADDR;
static uint8_t g_bus_8500 = 0;
static uint8_t g_slot_8500 = 0;
static uint8_t g_func_8500 = 0;
static uint32_t g_mmio_base_8500 = 0;
static uint16_t g_device_id_8500 = 0;
static uint8_t g_revision_8500 = 0;
static int g_initialized_8500 = 0;
static uint32_t g_ring_8500[RADEON_8500_RING_BUFFER_SIZE / 4U];
static uint32_t g_ring_head_8500 = 0;
static uint32_t g_ring_tail_8500 = 0;
static int g_cursor_visible_8500 = 0;
static uint8_t g_gcn_gen_8500 = 2U;
static uint8_t g_power_state_8500 = POWER_STATE_D0;
static uint32_t g_gfx_clock_8500 = 275U;
static uint32_t g_memory_clock_8500 = 275U;
static uint32_t g_temperature_8500 = 45U;
static uint32_t g_fan_rpm_8500 = 2000U;
static uint8_t g_alpha_blend_global_8500 = 1U;

static struct radeon_8500_vram_alloc g_vram_allocs_8500[RADEON_8500_VRAM_MAX_ALLOCS];

static uint8_t g_fence_used_8500[RADEON_8500_MAX_FENCES] = {0};
static struct radeon_8500_fence g_fence_8500[RADEON_8500_MAX_FENCES];
static uint8_t g_sem_used_8500[RADEON_8500_MAX_SEMAPHORES] = {0};
static struct radeon_8500_semaphore g_sem_8500[RADEON_8500_MAX_SEMAPHORES];
static uint8_t g_swap_used_8500[RADEON_8500_MAX_SWAP_CHAINS] = {0};
static struct radeon_8500_swap_chain g_swap_8500[RADEON_8500_MAX_SWAP_CHAINS];

static inline uint32_t mmio_read32_8500(uint32_t offset)
{
    return g_mmio_8500[offset >> 2];
}

static inline void mmio_write32_8500(uint32_t offset, uint32_t value)
{
    g_mmio_8500[offset >> 2] = value;
}

static inline void mmio_write32_posted_8500(uint32_t offset, uint32_t value)
{
    mmio_write32_8500(offset, value);
    (void)mmio_read32_8500(offset);
}

static void radeon_8500_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

static int radeon_8500_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32_8500(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != RADEON_8500_VENDOR_ID) continue;
                if (device != RADEON_8500_DEVICE_ID_8500 &&
                    device != RADEON_8500_DEVICE_ID_8500_LE &&
                    device != RADEON_8500_DEVICE_ID_8500_VAR1 &&
                    device != RADEON_8500_DEVICE_ID_8500_VAR2 &&
                    device != RADEON_8500_DEVICE_ID_8500_VAR3 &&
                    device != RADEON_8500_DEVICE_ID_8500_VAR4 &&
                    device != RADEON_8500_DEVICE_ID_8500_VAR5) continue;

                uint32_t bar0 = pci_read32_8500(bus, slot, func, 0x10);
                if (bar0 == 0 || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;

                uint32_t command = pci_read32_8500(bus, slot, func, 0x04);
                pci_write32_8500(bus, slot, func, 0x04, command | 0x00000006U);

                g_bus_8500 = bus;
                g_slot_8500 = slot;
                g_func_8500 = func;
                g_mmio_base_8500 = bar0 & 0xFFFFFFF0U;
                g_mmio_8500 = (volatile uint32_t *)g_mmio_base_8500;
                g_device_id_8500 = device;
                g_revision_8500 = (uint8_t)(pci_read32_8500(bus, slot, func, 0x08) & 0xFFU);
                return 1;
            }
        }
    }
    return 0;
}

static void radeon_8500_clear_interrupts(void)
{
    mmio_write32_8500(REG_INT_STAT, 0xFFFFFFFFU);
    mmio_write32_8500(REG_INT_MASK, 0x00000000U);
}

static void radeon_8500_reset_cp(void)
{
    mmio_write32_8500(REG_CP_ME_CNTL, CP_ME_CNTL_ME_RESET | CP_ME_CNTL_PFP_RESET);
    radeon_8500_delay_us(100);
    mmio_write32_8500(REG_CP_ME_CNTL, 0U);
    radeon_8500_delay_us(100);
}

static void radeon_8500_cp_ring_init(void)
{
    mmio_write32_8500(REG_CP_ME_RB_ADDR, (uint32_t)&g_ring_8500[0]);
    mmio_write32_8500(REG_CP_ME_RB_SIZE, sizeof(g_ring_8500));
    mmio_write32_8500(REG_CP_ME_RB_WPTR, 0U);
    mmio_write32_8500(REG_CP_ME_RB_RPTR, 0U);
    mmio_write32_8500(REG_CP_ME_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_head_8500 = 0;
    g_ring_tail_8500 = 0;
}

static void radeon_8500_cp_flush(void);
static void radeon_8500_cp_emit(uint32_t dword)
{
    if (((g_ring_head_8500 & (RADEON_8500_RING_BUFFER_SIZE / 4U - 1U)) + 5U) > (RADEON_8500_RING_BUFFER_SIZE / 4U)) {
        radeon_8500_cp_flush();
    }
    g_ring_8500[g_ring_head_8500 & (RADEON_8500_RING_BUFFER_SIZE / 4U - 1U)] = dword;
    g_ring_head_8500++;
}

static void radeon_8500_cp_flush(void)
{
    uint32_t tail = g_ring_head_8500 & (RADEON_8500_RING_BUFFER_SIZE / 4U - 1U);
    mmio_write32_8500(REG_CP_ME_RB_WPTR, tail);
    for (volatile uint32_t wait = 0; wait < 1000000U; wait++) {
        if (mmio_read32_8500(REG_CP_ME_RB_RPTR) == tail) {
            break;
        }
    }
    g_ring_tail_8500 = tail;
}

static void radeon_8500_display_init(void)
{
    uint32_t hblank = (((RADEON_8500_FB_WIDTH + 128U) - 1U) << 16) | (RADEON_8500_FB_WIDTH - 1U);
    uint32_t vblank = (((RADEON_8500_FB_HEIGHT + 32U) - 1U) << 16) | (RADEON_8500_FB_HEIGHT - 1U);
    uint32_t hsync = (((RADEON_8500_FB_WIDTH + 64U) - 1U) << 16) | (RADEON_8500_FB_WIDTH - 1U);
    uint32_t vsync = (((RADEON_8500_FB_HEIGHT + 4U) - 1U) << 16) | (RADEON_8500_FB_HEIGHT - 1U);

    mmio_write32_8500(REG_CRTC_H_TOTAL, hblank);
    mmio_write32_8500(REG_CRTC_V_TOTAL, vblank);
    mmio_write32_8500(REG_CRTC_H_BLANK, hblank);
    mmio_write32_8500(REG_CRTC_V_BLANK, vblank);
    mmio_write32_8500(REG_CRTC_H_SYNC_A, hsync);
    mmio_write32_8500(REG_CRTC_V_SYNC_A, vsync);

    mmio_write32_8500(REG_CRTC_PRIMARY_VIEWPORT_START, 0U);
    mmio_write32_8500(REG_CRTC_PRIMARY_VIEWPORT_END,
                     ((RADEON_8500_FB_HEIGHT - 1U) << 16) | (RADEON_8500_FB_WIDTH - 1U));

    mmio_write32_8500(REG_CRTC_PRIMARY_SURFACE_ADDRESS, RADEON_8500_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32_8500(REG_CRTC_PRIMARY_SURFACE_ADDRESS_HIGH, 0U);
    mmio_write32_8500(REG_CRTC_PRIMARY_SURFACE_OFFSET_X, 0U);
    mmio_write32_8500(REG_CRTC_PRIMARY_SURFACE_OFFSET_Y, 0U);
    mmio_write32_8500(REG_CRTC_PRIMARY_SURFACE_PIXEL_PITCH, RADEON_8500_FB_PITCH);
    mmio_write32_8500(REG_CRTC_PRIMARY_SURFACE_TILE, 0U);

    mmio_write32_8500(REG_PRIMARY_PLANE_OFFSET_X, 0U);
    mmio_write32_8500(REG_PRIMARY_PLANE_OFFSET_Y, 0U);
    mmio_write32_8500(REG_PRIMARY_PLANE_PIXEL_PITCH, RADEON_8500_FB_PITCH);
    mmio_write32_8500(REG_PRIMARY_PLANE_SURFACE_ADDRESS, RADEON_8500_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32_8500(REG_PRIMARY_PLANE_SURFACE_ADDRESS_HIGH, 0U);

    mmio_write32_8500(REG_CRTC_CONTROL, CRTC_CONTROL_EN | CRTC_CONTROL_PIXEL_WIDTH_32BPP);
    radeon_8500_delay_us(200);
}

static void radeon_8500_3d_pipeline_init(void)
{
    radeon_8500_reset_cp();
    radeon_8500_cp_ring_init();

    mmio_write32_8500(REG_CB_TARGET0_BASE, RADEON_8500_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32_8500(REG_CB_TARGET0_PITCH, RADEON_8500_FB_PITCH);
    mmio_write32_8500(REG_CB_TARGET0_SLICE, 0U);
    mmio_write32_8500(REG_CB_TARGET0_FMASK, 0U);
    mmio_write32_8500(REG_CB_TARGET0_VIEW, 0U);

    mmio_write32_8500(REG_CB_SHADER_MASK, SHADER_MASK_ENABLE | SHADER_MASK_CB0_ENABLE);

    mmio_write32_8500(REG_CB_CLEAR_COLOR, 0x00000000U);
    mmio_write32_8500(REG_CB_CLEAR_COLOR_HIGH, 0x00000000U);

    uint32_t blend = (RADEON_8500_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_SHIFT) |
                     (RADEON_8500_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_SHIFT) |
                     (RADEON_8500_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_SHIFT) |
                     (RADEON_8500_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT) |
                     (RADEON_8500_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT) |
                     (RADEON_8500_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT);
    mmio_write32_8500(REG_CB_BLEND_CONTROL, blend);

    mmio_write32_8500(REG_DB_Z_INFO, DB_Z_INFO_FORMAT_24BIT);
    mmio_write32_8500(REG_DB_Z_READ_BASE, (RADEON_8500_FB_ADDR + 0x1000000U) & 0xFFFFFFFFU);
    mmio_write32_8500(REG_DB_Z_READ_BASE_HIGH, 0U);
    mmio_write32_8500(REG_DB_Z_WRITE_BASE, (RADEON_8500_FB_ADDR + 0x1000000U) & 0xFFFFFFFFU);
    mmio_write32_8500(REG_DB_Z_WRITE_BASE_HIGH, 0U);
    mmio_write32_8500(REG_DB_Z_READ_OFF, 0U);
    mmio_write32_8500(REG_DB_Z_WRITE_OFF, 0U);
    mmio_write32_8500(REG_DB_CLEAR, 0x00FFFFFFU);
    mmio_write32_8500(REG_DB_STENCIL_CLEAR, 0U);

    mmio_write32_8500(REG_PA_SU_VTX_CNTL, 0U);
    mmio_write32_8500(REG_PA_CL_VTE_CNTL, 0U);
    mmio_write32_8500(REG_PA_CL_VS_OUT_CNTL, 0U);
    mmio_write32_8500(REG_PA_CL_GB_ADV, 0U);
    mmio_write32_8500(REG_PA_CL_CLIP_CNTL, 0U);
    mmio_write32_8500(REG_PA_SU_SC_MODE_CNTL, 0U);
    mmio_write32_8500(REG_PA_SC_WINDOW_OFFSET, 0U);
    mmio_write32_8500(REG_PA_SC_WINDOW_SCISSOR_TL, 0U);
    mmio_write32_8500(REG_PA_SC_WINDOW_SCISSOR_BR,
                     ((RADEON_8500_FB_HEIGHT - 1U) << 16) | (RADEON_8500_FB_WIDTH - 1U));
    mmio_write32_8500(REG_PA_SU_PRIM_FILTER_CNTL, 0U);
    mmio_write32_8500(REG_PA_SU_LINE_STIPPLE, 0U);
    mmio_write32_8500(REG_PA_SC_AA_CONFIG, 0U);

    mmio_write32_8500(REG_CB_ALPHA_TO_MASK, 0U);
    mmio_write32_8500(REG_CB_ALPHA_TO_MASK_DWORD0, 0U);
    mmio_write32_8500(REG_CB_ALPHA_TO_MASK_DWORD1, 0U);
    mmio_write32_8500(REG_CB_ALPHA_TO_MASK_DWORD2, 0U);
    mmio_write32_8500(REG_CB_ALPHA_TO_MASK_DWORD3, 0U);

    mmio_write32_8500(REG_SQ_CONFIG, 0U);
    mmio_write32_8500(REG_SQ_VTX_BASE, 0U);
    mmio_write32_8500(REG_SQ_VTX_STRIDE, 0U);
    mmio_write32_8500(REG_SQ_VTX_NUM_FORMAT, 0U);
    mmio_write32_8500(REG_SQ_VTX_FMT_0, 0U);
    mmio_write32_8500(REG_SQ_VTX_FMT_1, 0U);
    mmio_write32_8500(REG_SQ_VTX_FMT_2, 0U);
    mmio_write32_8500(REG_SQ_VTX_FMT_3, 0U);

    mmio_write32_8500(REG_SQ_PROGRAM, 0U);
    mmio_write32_8500(REG_SQ_COMPUTE_PROGRAM, 0U);
    mmio_write32_8500(REG_SQ_CONSTANT_BASE, 0U);
    mmio_write32_8500(REG_SQ_CONSTANT_CACHE, 0U);

    mmio_write32_8500(REG_SQ_TEX_BASE, 0U);
    mmio_write32_8500(REG_SQ_SAMPLER_BASE, 0U);
    mmio_write32_8500(REG_SQ_TEX_LOD, 0U);
    mmio_write32_8500(REG_SQ_TEX_DIM, 0U);

    radeon_8500_delay_us(200);
}

static void radeon_8500_cursor_init(void)
{
    mmio_write32_8500(REG_CURSOR_CONTROL, 0U);
    mmio_write32_8500(REG_CURSOR_POSITION, 0U);
    mmio_write32_8500(REG_CURSOR_COLOR0, 0x00000000U);
    mmio_write32_8500(REG_CURSOR_COLOR1, 0x00FFFFFFU);
    mmio_write32_8500(REG_CURSOR_BASE, RADEON_8500_FB_ADDR);
    g_cursor_visible_8500 = 0;
}

int radeon_8500_init(void)
{
    if (g_initialized_8500) return 1;
    if (!radeon_8500_find_device()) return 0;

    uint32_t command = pci_read32_8500(g_bus_8500, g_slot_8500, g_func_8500, 0x04);
    pci_write32_8500(g_bus_8500, g_slot_8500, g_func_8500, 0x04, command | 0x00000006U);

    radeon_8500_clear_interrupts();
    radeon_8500_display_init();
    radeon_8500_3d_pipeline_init();
    radeon_8500_cursor_init();

    g_power_state_8500 = POWER_STATE_D0;
    g_gfx_clock_8500 = 275U;
    g_memory_clock_8500 = 275U;
    g_temperature_8500 = 45U;
    g_fan_rpm_8500 = 2000U;

    g_initialized_8500 = 1;
    return 1;
}

int radeon_8500_is_ready(void)
{
    return g_initialized_8500;
}

void radeon_8500_shutdown(void)
{
    mmio_write32_8500(REG_CP_ME_CNTL, CP_ME_CNTL_ME_HALT | CP_ME_CNTL_PFP_HALT);
    mmio_write32_8500(REG_POWER_STATE, POWER_STATE_D3);
    g_power_state_8500 = POWER_STATE_D3;
    g_initialized_8500 = 0;
}

void radeon_8500_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= RADEON_8500_FB_WIDTH || y >= RADEON_8500_FB_HEIGHT) return;
    g_fb_8500[y * (RADEON_8500_FB_PITCH / 4U) + x] = color;
}

void radeon_8500_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (g_initialized_8500) {
        radeon_8500_cp_emit(0x40000000U | 0x01U);
        radeon_8500_cp_emit((y << 16) | x);
        radeon_8500_cp_emit(((h - 1U) << 16) | (w - 1U));
        radeon_8500_cp_emit(color);
        radeon_8500_cp_emit(0U);
        radeon_8500_cp_flush();
        return;
    }
    uint32_t xx;
    uint32_t yy;
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= RADEON_8500_FB_WIDTH || yy >= RADEON_8500_FB_HEIGHT) continue;
            g_fb_8500[yy * (RADEON_8500_FB_PITCH / 4U) + xx] = color;
        }
    }
}

void radeon_8500_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                     uint32_t w, uint32_t h)
{
    if (!g_initialized_8500) return;
    radeon_8500_cp_emit(0x40000000U | 0x02U);
    radeon_8500_cp_emit((dst_y << 16) | dst_x);
    radeon_8500_cp_emit(((h - 1U) << 16) | (w - 1U));
    radeon_8500_cp_emit((src_y << 16) | src_x);
    radeon_8500_cp_emit(0U);
    radeon_8500_cp_flush();
}

void radeon_8500_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    if (!g_initialized_8500) return;
    if (image) {
        for (uint32_t i = 0; i < 64U * 64U; i++) {
            g_fb_8500[i] = image[i];
        }
        mmio_write32_8500(REG_CURSOR_BASE, RADEON_8500_FB_ADDR);
    }
    mmio_write32_8500(REG_CURSOR_POSITION, (y << 16) | x);
    mmio_write32_8500(REG_CURSOR_CONTROL, 0x80000000U);
    g_cursor_visible_8500 = 1;
}

void radeon_8500_cursor_hide(void)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_CURSOR_CONTROL, 0U);
    g_cursor_visible_8500 = 0;
}

void *radeon_8500_get_fb(void)
{
    return (void *)g_fb_8500;
}

uint32_t radeon_8500_get_width(void)
{
    return RADEON_8500_FB_WIDTH;
}

uint32_t radeon_8500_get_height(void)
{
    return RADEON_8500_FB_HEIGHT;
}

uint16_t radeon_8500_get_device_id(void)
{
    return g_device_id_8500;
}

uint8_t radeon_8500_get_revision(void)
{
    return g_revision_8500;
}

void radeon_8500_info_get(struct radeon_8500_info *out)
{
    if (!out) return;
    out->vendor_id = RADEON_8500_VENDOR_ID;
    out->device_id = g_device_id_8500;
    out->revision_id = g_revision_8500;
    out->mmio_base = g_mmio_base_8500;
    out->fb_addr = RADEON_8500_FB_ADDR;
    out->fb_width = RADEON_8500_FB_WIDTH;
    out->fb_height = RADEON_8500_FB_HEIGHT;
    out->fb_pitch = RADEON_8500_FB_PITCH;
    out->initialized = (uint8_t)g_initialized_8500;
    out->architecture = g_gcn_gen_8500;
    out->vram_size = RADEON_8500_VRAM_SIZE_BYTES;
    out->vram_used = radeon_8500_vram_used();
    out->gfx_clock_mhz = g_gfx_clock_8500;
    out->memory_clock_mhz = g_memory_clock_8500;
    out->temperature_c = g_temperature_8500;
    out->fan_rpm = g_fan_rpm_8500;
}

static uint64_t vram_allocate_8500(uint64_t size, uint32_t type)
{
    uint64_t aligned = (size + RADEON_8500_VRAM_PAGE_SIZE - 1U) & ~(RADEON_8500_VRAM_PAGE_SIZE - 1U);
    if (aligned == 0) return 0xFFFFFFFFFFFFFFFFULL;
    for (uint64_t i = 0; i < RADEON_8500_VRAM_MAX_ALLOCS; i++) {
        if (g_vram_allocs_8500[i].used) continue;
        uint64_t offset = i * RADEON_8500_VRAM_PAGE_SIZE;
        uint64_t end = offset + aligned;
        if (end > RADEON_8500_VRAM_SIZE_BYTES) continue;
        int conflict = 0;
        for (uint64_t k = 0; k < RADEON_8500_VRAM_MAX_ALLOCS; k++) {
            if (!g_vram_allocs_8500[k].used) continue;
            uint64_t a_start = g_vram_allocs_8500[k].offset;
            uint64_t a_end = a_start + g_vram_allocs_8500[k].size;
            if (offset < a_end && end > a_start) { conflict = 1; break; }
        }
        if (conflict) continue;
        g_vram_allocs_8500[i].used = 1;
        g_vram_allocs_8500[i].offset = offset;
        g_vram_allocs_8500[i].size = aligned;
        g_vram_allocs_8500[i].type = type;
        return offset;
    }
    return 0xFFFFFFFFFFFFFFFFULL;
}

static void vram_free_offset_8500(uint64_t offset)
{
    for (uint64_t i = 0; i < RADEON_8500_VRAM_MAX_ALLOCS; i++) {
        if (!g_vram_allocs_8500[i].used) continue;
        if (g_vram_allocs_8500[i].offset == offset) {
            g_vram_allocs_8500[i].used = 0;
            g_vram_allocs_8500[i].offset = 0;
            g_vram_allocs_8500[i].size = 0;
            g_vram_allocs_8500[i].type = 0;
            return;
        }
    }
}

uint64_t radeon_8500_vram_alloc(uint64_t size, uint32_t type)
{
    return vram_allocate_8500(size, type);
}

void radeon_8500_vram_free(uint64_t handle)
{
    vram_free_offset_8500(handle);
}

uint64_t radeon_8500_vram_used(void)
{
    uint64_t used = 0;
    for (uint64_t i = 0; i < RADEON_8500_VRAM_MAX_ALLOCS; i++) {
        if (!g_vram_allocs_8500[i].used) continue;
        used += g_vram_allocs_8500[i].size;
    }
    return used;
}

uint64_t radeon_8500_vram_total(void)
{
    return RADEON_8500_VRAM_SIZE_BYTES;
}

static uint8_t g_vb_used_8500[RADEON_8500_MAX_VERTEX_BUFFERS] = {0};
static uint8_t g_ib_used_8500[RADEON_8500_MAX_INDEX_BUFFERS] = {0};
static uint8_t g_tex_used_8500[RADEON_8500_MAX_TEXTURES] = {0};
static uint8_t g_rt_used_8500[RADEON_8500_MAX_RENDER_TARGETS] = {0};
static uint8_t g_ds_used_8500[RADEON_8500_MAX_DEPTH_STENCILS] = {0};
static uint8_t g_shader_used_8500[RADEON_8500_MAX_SHADERS] = {0};
static uint8_t g_sampler_used_8500[RADEON_8500_MAX_SAMPLERS] = {0};
static uint8_t g_blend_used_8500[RADEON_8500_MAX_BLEND_STATES] = {0};
static uint8_t g_raster_used_8500[RADEON_8500_MAX_RASTERIZER_STATES] = {0};
static uint8_t g_ds_state_used_8500[RADEON_8500_MAX_DEPTH_STENCIL_STATES] = {0};
static uint8_t g_il_used_8500[RADEON_8500_MAX_INPUT_LAYOUTS] = {0};
static uint8_t g_cb_used_8500[RADEON_8500_MAX_CONSTANT_BUFFERS] = {0};

static struct radeon_8500_vertex_buffer g_vb_8500[RADEON_8500_MAX_VERTEX_BUFFERS];
static struct radeon_8500_index_buffer g_ib_8500[RADEON_8500_MAX_INDEX_BUFFERS];
static struct radeon_8500_texture g_tex_8500[RADEON_8500_MAX_TEXTURES];
static struct radeon_8500_render_target g_rt_8500[RADEON_8500_MAX_RENDER_TARGETS];
static struct radeon_8500_depth_stencil g_ds_8500[RADEON_8500_MAX_DEPTH_STENCILS];
static struct radeon_8500_shader g_shader_8500[RADEON_8500_MAX_SHADERS];
static struct radeon_8500_sampler_state g_sampler_8500[RADEON_8500_MAX_SAMPLERS];
static struct radeon_8500_blend_state g_blend_8500[RADEON_8500_MAX_BLEND_STATES];
static struct radeon_8500_rasterizer_state g_raster_8500[RADEON_8500_MAX_RASTERIZER_STATES];
static struct radeon_8500_depth_stencil_state g_ds_state_8500[RADEON_8500_MAX_DEPTH_STENCIL_STATES];
static struct radeon_8500_input_layout g_il_8500[RADEON_8500_MAX_INPUT_LAYOUTS];
static struct radeon_8500_constant_buffer g_cb_8500[RADEON_8500_MAX_CONSTANT_BUFFERS];

static struct radeon_8500_context g_ctx_8500 = {};

static uint32_t find_free_slot_8500(uint8_t *used, uint32_t max)
{
    for (uint32_t i = 0; i < max; i++) {
        if (!used[i]) return i;
    }
    return RADEON_8500_HANDLE_NULL;
}

static int handle_valid_8500(radeon_8500_handle_t h, uint8_t *used, uint32_t max)
{
    return h != RADEON_8500_HANDLE_NULL && h < max && used[h];
}

radeon_8500_handle_t radeon_8500_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot_8500(g_vb_used_8500, RADEON_8500_MAX_VERTEX_BUFFERS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_vertex_buffer *vb = &g_vb_8500[i];
    vb->used = 1;
    vb->vertex_count = vertex_count;
    vb->vertex_size = vertex_size;
    vb->format = format;
    vb->buffer_addr = RADEON_8500_FB_ADDR + 0x2000000U + i * 0x100000U;
    vb->system_mem = 0;
    vb->vram_offset = radeon_8500_vram_alloc((uint64_t)vertex_count * (uint64_t)vertex_size, 0U);
    if (data) {
        vb->system_mem = (uint8_t *)vb->buffer_addr;
        for (uint32_t k = 0; k < vertex_count * vertex_size; k++) {
            vb->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_8500_handle_t)i;
}

void radeon_8500_vertex_buffer_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_vb_used_8500, RADEON_8500_MAX_VERTEX_BUFFERS)) return;
    g_vb_8500[handle].used = 0;
    if (g_vb_8500[handle].vram_offset != 0xFFFFFFFFFFFFFFFFULL) {
        radeon_8500_vram_free(g_vb_8500[handle].vram_offset);
    }
}

void radeon_8500_vertex_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid_8500(handle, g_vb_used_8500, RADEON_8500_MAX_VERTEX_BUFFERS)) return;
    struct radeon_8500_vertex_buffer *vb = &g_vb_8500[handle];
    if (!vb->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        vb->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_8500_handle_t radeon_8500_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot_8500(g_ib_used_8500, RADEON_8500_MAX_INDEX_BUFFERS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_index_buffer *ib = &g_ib_8500[i];
    ib->used = 1;
    ib->index_count = index_count;
    ib->format = format;
    ib->buffer_addr = RADEON_8500_FB_ADDR + 0x4000000U + i * 0x100000U;
    ib->system_mem = 0;
    ib->vram_offset = radeon_8500_vram_alloc((uint64_t)index_count * 4U, 0U);
    if (data) {
        ib->system_mem = (uint8_t *)ib->buffer_addr;
        uint32_t elem_size = (format == RADEON_8500_INDEX_FORMAT_32BIT) ? 4U : 2U;
        for (uint32_t k = 0; k < index_count * elem_size; k++) {
            ib->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_8500_handle_t)i;
}

void radeon_8500_index_buffer_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_ib_used_8500, RADEON_8500_MAX_INDEX_BUFFERS)) return;
    g_ib_8500[handle].used = 0;
    if (g_ib_8500[handle].vram_offset != 0xFFFFFFFFFFFFFFFFULL) {
        radeon_8500_vram_free(g_ib_8500[handle].vram_offset);
    }
}

void radeon_8500_index_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid_8500(handle, g_ib_used_8500, RADEON_8500_MAX_INDEX_BUFFERS)) return;
    struct radeon_8500_index_buffer *ib = &g_ib_8500[handle];
    if (!ib->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        ib->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_8500_handle_t radeon_8500_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
{
    uint32_t i = find_free_slot_8500(g_tex_used_8500, RADEON_8500_MAX_TEXTURES);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_texture *t = &g_tex_8500[i];
    t->used = 1;
    t->width = width;
    t->height = height;
    t->format = format;
    t->mip_levels = mip_levels;
    t->pitch = ((width * 4U + 3U) & ~3U);
    t->buffer_addr = RADEON_8500_FB_ADDR + 0x6000000U + i * 0x200000U;
    t->system_mem = 0;
    t->vram_offset = radeon_8500_vram_alloc((uint64_t)width * (uint64_t)height * 4U * (uint64_t)mip_levels, 0U);
    t->compressed = (format == RADEON_8500_TEX_FORMAT_DXT1 || format == RADEON_8500_TEX_FORMAT_DXT3 ||
                     format == RADEON_8500_TEX_FORMAT_DXT5) ? 1U : 0U;
    if (data) {
        t->system_mem = (uint8_t *)t->buffer_addr;
        uint32_t bpp = (t->compressed) ? 1U : 4U;
        for (uint32_t k = 0; k < width * height * bpp; k++) {
            t->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_8500_handle_t)i;
}

void radeon_8500_texture_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_tex_used_8500, RADEON_8500_MAX_TEXTURES)) return;
    g_tex_8500[handle].used = 0;
    if (g_tex_8500[handle].vram_offset != 0xFFFFFFFFFFFFFFFFULL) {
        radeon_8500_vram_free(g_tex_8500[handle].vram_offset);
    }
}

void radeon_8500_texture_update(radeon_8500_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
{
    (void)level;
    if (!handle_valid_8500(handle, g_tex_used_8500, RADEON_8500_MAX_TEXTURES)) return;
    struct radeon_8500_texture *t = &g_tex_8500[handle];
    if (!t->system_mem) return;
    for (uint32_t yy = 0; yy < h; yy++) {
        for (uint32_t xx = 0; xx < w; xx++) {
            uint32_t src = (yy * w + xx) * 4U;
            uint32_t dst = ((y + yy) * t->pitch + (x + xx)) * 4U;
            t->system_mem[dst] = ((const uint8_t *)data)[src];
            t->system_mem[dst + 1] = ((const uint8_t *)data)[src + 1];
            t->system_mem[dst + 2] = ((const uint8_t *)data)[src + 2];
            t->system_mem[dst + 3] = ((const uint8_t *)data)[src + 3];
        }
    }
}

void radeon_8500_texture_generate_mipmaps(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_tex_used_8500, RADEON_8500_MAX_TEXTURES)) return;
    struct radeon_8500_texture *t = &g_tex_8500[handle];
    if (!t->system_mem) return;
    uint32_t w = t->width;
    uint32_t h = t->height;
    for (uint32_t level = 1; level < t->mip_levels && w > 1 && h > 1; level++) {
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
    }
}

radeon_8500_handle_t radeon_8500_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend)
{
    uint32_t i = find_free_slot_8500(g_rt_used_8500, RADEON_8500_MAX_RENDER_TARGETS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_render_target *rt = &g_rt_8500[i];
    rt->used = 1;
    rt->width = width;
    rt->height = height;
    rt->format = format;
    rt->buffer_addr = RADEON_8500_FB_ADDR + 0x8000000U + i * 0x100000U;
    rt->alpha_blend = alpha_blend;
    rt->dcc = 0U;
    rt->vram_offset = radeon_8500_vram_alloc((uint64_t)width * (uint64_t)height * 4U, 0U);
    return (radeon_8500_handle_t)i;
}

void radeon_8500_render_target_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_rt_used_8500, RADEON_8500_MAX_RENDER_TARGETS)) return;
    g_rt_8500[handle].used = 0;
    if (g_rt_8500[handle].vram_offset != 0xFFFFFFFFFFFFFFFFULL) {
        radeon_8500_vram_free(g_rt_8500[handle].vram_offset);
    }
}

radeon_8500_handle_t radeon_8500_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression)
{
    uint32_t i = find_free_slot_8500(g_ds_used_8500, RADEON_8500_MAX_DEPTH_STENCILS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_depth_stencil *ds = &g_ds_8500[i];
    ds->used = 1;
    ds->width = width;
    ds->height = height;
    ds->format = format;
    ds->buffer_addr = RADEON_8500_FB_ADDR + 0xA000000U + i * 0x100000U;
    ds->compression = compression;
    ds->vram_offset = radeon_8500_vram_alloc((uint64_t)width * (uint64_t)height * 4U, 0U);
    return (radeon_8500_handle_t)i;
}

void radeon_8500_depth_stencil_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_ds_used_8500, RADEON_8500_MAX_DEPTH_STENCILS)) return;
    g_ds_8500[handle].used = 0;
    if (g_ds_8500[handle].vram_offset != 0xFFFFFFFFFFFFFFFFULL) {
        radeon_8500_vram_free(g_ds_8500[handle].vram_offset);
    }
}

radeon_8500_handle_t radeon_8500_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)
{
    uint32_t i = find_free_slot_8500(g_shader_used_8500, RADEON_8500_MAX_SHADERS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_shader *s = &g_shader_8500[i];
    s->used = 1;
    s->type = type;
    s->code_size = code_size;
    s->code = (uint8_t *)code;
    s->constant_count = 0;
    s->compiled = 0;
    return (radeon_8500_handle_t)i;
}

void radeon_8500_shader_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_shader_used_8500, RADEON_8500_MAX_SHADERS)) return;
    g_shader_8500[handle].used = 0;
}

void radeon_8500_shader_compile(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_shader_used_8500, RADEON_8500_MAX_SHADERS)) return;
    g_shader_8500[handle].compiled = 1;
    mmio_write32_8500(REG_SQ_PROGRAM, handle);
    radeon_8500_delay_us(50);
}

radeon_8500_handle_t radeon_8500_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable, uint8_t compare_func)
{
    uint32_t i = find_free_slot_8500(g_sampler_used_8500, RADEON_8500_MAX_SAMPLERS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_sampler_state *s = &g_sampler_8500[i];
    s->used = 1;
    s->filter_min = filter_min;
    s->filter_mag = filter_mag;
    s->filter_mip = filter_mip;
    s->address_u = address_u;
    s->address_v = address_v;
    s->address_w = address_w;
    s->max_anisotropy = max_anisotropy;
    s->compare_enable = compare_enable;
    s->compare_func = compare_func;
    return (radeon_8500_handle_t)i;
}

void radeon_8500_sampler_state_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_sampler_used_8500, RADEON_8500_MAX_SAMPLERS)) return;
    g_sampler_8500[handle].used = 0;
}

radeon_8500_handle_t radeon_8500_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor)
{
    uint32_t i = find_free_slot_8500(g_blend_used_8500, RADEON_8500_MAX_BLEND_STATES);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_blend_state *b = &g_blend_8500[i];
    b->used = 1;
    b->blend_enable = blend_enable;
    b->src_blend = src_blend;
    b->dest_blend = dest_blend;
    b->blend_op = blend_op;
    b->src_blend_alpha = src_blend_alpha;
    b->dest_blend_alpha = dest_blend_alpha;
    b->blend_op_alpha = blend_op_alpha;
    b->render_target_write_mask = write_mask;
    b->blend_factor = blend_factor;
    return (radeon_8500_handle_t)i;
}

void radeon_8500_blend_state_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_blend_used_8500, RADEON_8500_MAX_BLEND_STATES)) return;
    g_blend_8500[handle].used = 0;
}

radeon_8500_handle_t radeon_8500_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage)
{
    uint32_t i = find_free_slot_8500(g_raster_used_8500, RADEON_8500_MAX_RASTERIZER_STATES);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_rasterizer_state *r = &g_raster_8500[i];
    r->used = 1;
    r->fill_mode = fill_mode;
    r->cull_mode = cull_mode;
    r->front_counter_clockwise = front_ccw;
    r->depth_clip_enable = depth_clip;
    r->scissor_enable = scissor;
    r->multisample_enable = multisample;
    r->antialiased_line_enable = aa_line;
    r->alpha_to_coverage = alpha_to_coverage;
    return (radeon_8500_handle_t)i;
}

void radeon_8500_rasterizer_state_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_raster_used_8500, RADEON_8500_MAX_RASTERIZER_STATES)) return;
    g_raster_8500[handle].used = 0;
}

radeon_8500_handle_t radeon_8500_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)
{
    uint32_t i = find_free_slot_8500(g_ds_state_used_8500, RADEON_8500_MAX_DEPTH_STENCIL_STATES);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_depth_stencil_state *d = &g_ds_state_8500[i];
    d->used = 1;
    d->depth_enable = depth_enable;
    d->depth_write_mask = depth_write_mask;
    d->depth_func = depth_func;
    d->stencil_enable = stencil_enable;
    d->stencil_read_mask = stencil_read_mask;
    d->stencil_write_mask = stencil_write_mask;
    d->front_stencil_fail_op = front_fail;
    d->front_stencil_depth_fail_op = front_zfail;
    d->front_stencil_pass_op = front_pass;
    d->front_stencil_func = front_func;
    d->back_stencil_fail_op = back_fail;
    d->back_stencil_depth_fail_op = back_zfail;
    d->back_stencil_pass_op = back_pass;
    d->back_stencil_func = back_func;
    return (radeon_8500_handle_t)i;
}

void radeon_8500_depth_stencil_state_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_ds_state_used_8500, RADEON_8500_MAX_DEPTH_STENCIL_STATES)) return;
    g_ds_state_8500[handle].used = 0;
}

radeon_8500_handle_t radeon_8500_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)
{
    uint32_t i = find_free_slot_8500(g_il_used_8500, RADEON_8500_MAX_INPUT_LAYOUTS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_input_layout *l = &g_il_8500[i];
    l->used = 1;
    l->element_count = element_count;
    for (uint32_t k = 0; k < element_count && k < 32U; k++) {
        l->slot[k] = slots ? slots[k] : 0U;
        l->offset[k] = offsets ? offsets[k] : 0U;
        l->format[k] = formats ? formats[k] : RADEON_8500_VERTEX_FORMAT_XYZ;
    }
    return (radeon_8500_handle_t)i;
}

void radeon_8500_input_layout_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_il_used_8500, RADEON_8500_MAX_INPUT_LAYOUTS)) return;
    g_il_8500[handle].used = 0;
}

radeon_8500_handle_t radeon_8500_constant_buffer_create(uint32_t size)
{
    uint32_t i = find_free_slot_8500(g_cb_used_8500, RADEON_8500_MAX_CONSTANT_BUFFERS);
    if (i == RADEON_8500_HANDLE_NULL) return RADEON_8500_HANDLE_NULL;
    struct radeon_8500_constant_buffer *c = &g_cb_8500[i];
    c->used = 1;
    c->size = size;
    c->data = (uint8_t *)0;
    c->vram_offset = radeon_8500_vram_alloc(size, 0U);
    return (radeon_8500_handle_t)i;
}

void radeon_8500_constant_buffer_destroy(radeon_8500_handle_t handle)
{
    if (!handle_valid_8500(handle, g_cb_used_8500, RADEON_8500_MAX_CONSTANT_BUFFERS)) return;
    g_cb_8500[handle].used = 0;
    if (g_cb_8500[handle].vram_offset != 0xFFFFFFFFFFFFFFFFULL) {
        radeon_8500_vram_free(g_cb_8500[handle].vram_offset);
    }
}

void radeon_8500_constant_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid_8500(handle, g_cb_used_8500, RADEON_8500_MAX_CONSTANT_BUFFERS)) return;
    struct radeon_8500_constant_buffer *c = &g_cb_8500[handle];
    if (!c->data) return;
    for (uint32_t k = 0; k < size; k++) {
        c->data[offset + k] = ((const uint8_t *)data)[k];
    }
}

void radeon_8500_bind_vertex_buffer(radeon_8500_handle_t handle)
{
    g_ctx_8500.vertex_buffer = handle;
}

void radeon_8500_bind_index_buffer(radeon_8500_handle_t handle)
{
    g_ctx_8500.index_buffer = handle;
}

void radeon_8500_bind_input_layout(radeon_8500_handle_t handle)
{
    g_ctx_8500.input_layout = handle;
}

void radeon_8500_bind_vertex_shader(radeon_8500_handle_t handle)
{
    g_ctx_8500.vertex_shader = handle;
}

void radeon_8500_bind_pixel_shader(radeon_8500_handle_t handle)
{
    g_ctx_8500.pixel_shader = handle;
}

void radeon_8500_bind_geometry_shader(radeon_8500_handle_t handle)
{
    g_ctx_8500.geometry_shader = handle;
}

void radeon_8500_bind_compute_shader(radeon_8500_handle_t handle)
{
    g_ctx_8500.compute_shader = handle;
}

void radeon_8500_bind_render_target(radeon_8500_handle_t handle)
{
    g_ctx_8500.render_target = handle;
}

void radeon_8500_bind_depth_stencil(radeon_8500_handle_t handle)
{
    g_ctx_8500.depth_stencil = handle;
}

void radeon_8500_bind_blend_state(radeon_8500_handle_t handle)
{
    g_ctx_8500.blend_state = handle;
}

void radeon_8500_bind_rasterizer_state(radeon_8500_handle_t handle)
{
    g_ctx_8500.rasterizer_state = handle;
}

void radeon_8500_bind_depth_stencil_state(radeon_8500_handle_t handle)
{
    g_ctx_8500.depth_stencil_state = handle;
}

void radeon_8500_bind_sampler_and_texture(uint32_t slot, radeon_8500_handle_t sampler, radeon_8500_handle_t texture)
{
    if (slot >= RADEON_8500_MAX_SAMPLERS) return;
    g_ctx_8500.sampler[slot] = sampler;
    g_ctx_8500.texture[slot] = texture;
}

void radeon_8500_bind_constant_buffer(uint32_t slot, radeon_8500_handle_t handle)
{
    if (slot >= RADEON_8500_MAX_CONSTANT_BUFFERS) return;
    g_ctx_8500.constant_buffer[slot] = handle;
}

void radeon_8500_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
    g_ctx_8500.viewport_x = x;
    g_ctx_8500.viewport_y = y;
    g_ctx_8500.viewport_w = w;
    g_ctx_8500.viewport_h = h;
    g_ctx_8500.viewport_min_depth = min_depth;
    g_ctx_8500.viewport_max_depth = max_depth;
    uint32_t packed = (x & 0xFFFFU) | ((y & 0xFFFFU) << 16U);
    mmio_write32_8500(REG_PA_SC_WINDOW_OFFSET, packed);
    packed = (w & 0xFFFFU) | ((h & 0xFFFFU) << 16U);
    mmio_write32_8500(REG_PA_SC_WINDOW_OFFSET + 4U, packed);
}

void radeon_8500_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    g_ctx_8500.scissor_left = left;
    g_ctx_8500.scissor_top = top;
    g_ctx_8500.scissor_right = right;
    g_ctx_8500.scissor_bottom = bottom;
    uint32_t packed = (left & 0xFFFFU) | ((top & 0xFFFFU) << 16U);
    mmio_write32_8500(REG_PA_SC_WINDOW_SCISSOR_TL, packed);
    packed = (right & 0xFFFFU) | ((bottom & 0xFFFFU) << 16U);
    mmio_write32_8500(REG_PA_SC_WINDOW_SCISSOR_BR, packed);
}

void radeon_8500_set_primitive_topology(uint32_t topology)
{
    g_ctx_8500.primitive_topology = topology;
    mmio_write32_8500(REG_PA_SU_PRIM_FILTER_CNTL, topology);
}

void radeon_8500_set_alpha_blend_global(uint8_t enable)
{
    g_alpha_blend_global_8500 = enable;
}

void radeon_8500_clear_render_target(radeon_8500_handle_t rt, uint32_t color)
{
    if (!handle_valid_8500(rt, g_rt_used_8500, RADEON_8500_MAX_RENDER_TARGETS)) return;
    struct radeon_8500_render_target *target = &g_rt_8500[rt];
    mmio_write32_8500(REG_CB_CLEAR_COLOR, color);
    mmio_write32_8500(REG_CB_CLEAR_COLOR_HIGH, 0U);
    mmio_write32_8500(REG_CB_TARGET0_BASE, target->buffer_addr);
    mmio_write32_8500(REG_CB_TARGET0_PITCH, target->width * 4U);
    radeon_8500_delay_us(100);
}

void radeon_8500_clear_depth_stencil(radeon_8500_handle_t ds, float depth, uint8_t stencil)
{
    if (!handle_valid_8500(ds, g_ds_used_8500, RADEON_8500_MAX_DEPTH_STENCILS)) return;
    struct radeon_8500_depth_stencil *depth_stencil = &g_ds_8500[ds];
    uint32_t depth_bits = 0;
    const uint8_t *depth_bytes = (const uint8_t *)&depth;
    for (int k = 0; k < 4; k++) {
        ((uint8_t *)&depth_bits)[k] = depth_bytes[k];
    }
    mmio_write32_8500(REG_DB_CLEAR, depth_bits);
    mmio_write32_8500(REG_DB_STENCIL_CLEAR, (uint32_t)stencil);
    mmio_write32_8500(REG_DB_Z_INFO, depth_stencil->buffer_addr);
    radeon_8500_delay_us(100);
}

void radeon_8500_draw(uint32_t vertex_count, uint32_t start_vertex)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_SQ_PROGRAM, g_ctx_8500.vertex_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 4U, g_ctx_8500.pixel_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 8U, g_ctx_8500.geometry_shader);
    radeon_8500_cp_emit(0x80000000U | 0x10U);
    radeon_8500_cp_emit(vertex_count);
    radeon_8500_cp_emit(start_vertex);
    radeon_8500_cp_emit(g_ctx_8500.vertex_buffer);
    radeon_8500_cp_emit(g_ctx_8500.primitive_topology);
    radeon_8500_cp_emit(g_ctx_8500.blend_state);
    radeon_8500_cp_emit(g_ctx_8500.rasterizer_state);
    radeon_8500_cp_emit(g_ctx_8500.depth_stencil_state);
    radeon_8500_cp_flush();
    radeon_8500_delay_us(200);
}

void radeon_8500_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_SQ_PROGRAM, g_ctx_8500.vertex_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 4U, g_ctx_8500.pixel_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 8U, g_ctx_8500.geometry_shader);
    radeon_8500_cp_emit(0x80000000U | 0x11U);
    radeon_8500_cp_emit(index_count);
    radeon_8500_cp_emit(start_index);
    radeon_8500_cp_emit((uint32_t)base_vertex);
    radeon_8500_cp_emit(g_ctx_8500.index_buffer);
    radeon_8500_cp_emit(g_ctx_8500.primitive_topology);
    radeon_8500_cp_emit(g_ctx_8500.blend_state);
    radeon_8500_cp_emit(g_ctx_8500.rasterizer_state);
    radeon_8500_cp_emit(g_ctx_8500.depth_stencil_state);
    radeon_8500_cp_flush();
    radeon_8500_delay_us(200);
}

void radeon_8500_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_SQ_PROGRAM, g_ctx_8500.vertex_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 4U, g_ctx_8500.pixel_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 8U, g_ctx_8500.geometry_shader);
    radeon_8500_cp_emit(0x80000000U | 0x12U);
    radeon_8500_cp_emit(index_count);
    radeon_8500_cp_emit(start_index);
    radeon_8500_cp_emit((uint32_t)base_vertex);
    radeon_8500_cp_emit(instance_count);
    radeon_8500_cp_emit(start_instance);
    radeon_8500_cp_emit(g_ctx_8500.index_buffer);
    radeon_8500_cp_emit(g_ctx_8500.primitive_topology);
    radeon_8500_cp_emit(g_ctx_8500.blend_state);
    radeon_8500_cp_emit(g_ctx_8500.rasterizer_state);
    radeon_8500_cp_emit(g_ctx_8500.depth_stencil_state);
    radeon_8500_cp_flush();
    radeon_8500_delay_us(200);
}

void radeon_8500_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_SQ_PROGRAM, g_ctx_8500.vertex_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 4U, g_ctx_8500.pixel_shader);
    mmio_write32_8500(REG_SQ_PROGRAM + 8U, g_ctx_8500.geometry_shader);
    radeon_8500_cp_emit(0x80000000U | 0x13U);
    radeon_8500_cp_emit(buffer_addr);
    radeon_8500_cp_emit(draw_count);
    radeon_8500_cp_emit(stride);
    radeon_8500_cp_emit(g_ctx_8500.primitive_topology);
    radeon_8500_cp_emit(g_ctx_8500.blend_state);
    radeon_8500_cp_emit(g_ctx_8500.rasterizer_state);
    radeon_8500_cp_emit(g_ctx_8500.depth_stencil_state);
    radeon_8500_cp_flush();
    radeon_8500_delay_us(200);
}

void radeon_8500_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_SQ_COMPUTE_PROGRAM, g_ctx_8500.compute_shader);
    radeon_8500_cp_emit(0x80000000U | 0x20U);
    radeon_8500_cp_emit(group_x);
    radeon_8500_cp_emit(group_y);
    radeon_8500_cp_emit(group_z);
    radeon_8500_cp_flush();
    radeon_8500_delay_us(200);
}

void radeon_8500_compute_dispatch_indirect(uint32_t buffer_addr)
{
    if (!g_initialized_8500) return;
    mmio_write32_8500(REG_SQ_COMPUTE_PROGRAM, g_ctx_8500.compute_shader);
    radeon_8500_cp_emit(0x80000000U | 0x21U);
    radeon_8500_cp_emit(buffer_addr);
    radeon_8500_cp_flush();
    radeon_8500_delay_us(200);
}

void radeon_8500_fence_create(radeon_8500_handle_t *out)
{
    if (!out) return;
    for (uint32_t i = 0; i < RADEON_8500_MAX_FENCES; i++) {
        if (!g_fence_used_8500[i]) {
            g_fence_used_8500[i] = 1;
            g_fence_8500[i].value = 0;
            g_fence_8500[i].signaled = 0;
            *out = (radeon_8500_handle_t)i;
            return;
        }
    }
    *out = RADEON_8500_HANDLE_NULL;
}

void radeon_8500_fence_signal(radeon_8500_handle_t handle, uint32_t value)
{
    if (handle == RADEON_8500_HANDLE_NULL || handle >= RADEON_8500_MAX_FENCES || !g_fence_used_8500[handle]) return;
    g_fence_8500[handle].value = value;
    g_fence_8500[handle].signaled = 1;
}

int radeon_8500_fence_wait(radeon_8500_handle_t handle, uint32_t value, uint32_t timeout_us)
{
    if (handle == RADEON_8500_HANDLE_NULL || handle >= RADEON_8500_MAX_FENCES || !g_fence_used_8500[handle]) return 0;
    for (volatile uint32_t wait = 0; wait < timeout_us * 10U; wait++) {
        if (g_fence_8500[handle].signaled && g_fence_8500[handle].value >= value) return 1;
    }
    return 0;
}

void radeon_8500_fence_destroy(radeon_8500_handle_t handle)
{
    if (handle == RADEON_8500_HANDLE_NULL || handle >= RADEON_8500_MAX_FENCES || !g_fence_used_8500[handle]) return;
    g_fence_used_8500[handle] = 0;
}

void radeon_8500_semaphore_create(radeon_8500_handle_t *out)
{
    if (!out) return;
    for (uint32_t i = 0; i < RADEON_8500_MAX_SEMAPHORES; i++) {
        if (!g_sem_used_8500[i]) {
            g_sem_used_8500[i] = 1;
            g_sem_8500[i].value = 0;
            *out = (radeon_8500_handle_t)i;
            return;
        }
    }
    *out = RADEON_8500_HANDLE_NULL;
}

void radeon_8500_semaphore_signal(radeon_8500_handle_t handle, uint32_t value)
{
    if (handle == RADEON_8500_HANDLE_NULL || handle >= RADEON_8500_MAX_SEMAPHORES || !g_sem_used_8500[handle]) return;
    g_sem_8500[handle].value = value;
}

void radeon_8500_semaphore_wait(radeon_8500_handle_t handle, uint32_t value)
{
    if (handle == RADEON_8500_HANDLE_NULL || handle >= RADEON_8500_MAX_SEMAPHORES || !g_sem_used_8500[handle]) return;
    for (volatile uint32_t wait = 0; wait < 10000000U; wait++) {
        if (g_sem_8500[handle].value >= value) break;
    }
}

void radeon_8500_semaphore_destroy(radeon_8500_handle_t handle)
{
    if (handle == RADEON_8500_HANDLE_NULL || handle >= RADEON_8500_MAX_SEMAPHORES || !g_sem_used_8500[handle]) return;
    g_sem_used_8500[handle] = 0;
}

int radeon_8500_wait_idle(uint32_t timeout_us)
{
    for (volatile uint32_t wait = 0; wait < timeout_us * 10U; wait++) {
        uint32_t rptr = mmio_read32_8500(REG_CP_ME_RB_RPTR);
        uint32_t wptr = mmio_read32_8500(REG_CP_ME_RB_WPTR);
        if (rptr == wptr) return 1;
    }
    return 0;
}

uint64_t radeon_8500_timestamp(void)
{
    uint32_t lo = mmio_read32_8500(REG_TIMESTAMP_LO);
    uint32_t hi = mmio_read32_8500(REG_TIMESTAMP_HI);
    return ((uint64_t)hi << 32) | (uint64_t)lo;
}

void radeon_8500_power_set_state(uint8_t state)
{
    mmio_write32_8500(REG_POWER_STATE, state);
    g_power_state_8500 = state;
    radeon_8500_delay_us(100);
}

uint8_t radeon_8500_power_get_state(void)
{
    return g_power_state_8500;
}

void radeon_8500_set_clock_gfx(uint32_t mhz)
{
    mmio_write32_8500(REG_CLOCK_GFX, mhz);
    g_gfx_clock_8500 = mhz;
    radeon_8500_delay_us(50);
}

void radeon_8500_set_clock_memory(uint32_t mhz)
{
    mmio_write32_8500(REG_CLOCK_MEMORY, mhz);
    g_memory_clock_8500 = mhz;
    radeon_8500_delay_us(50);
}

uint32_t radeon_8500_get_clock_gfx(void)
{
    return g_gfx_clock_8500;
}

uint32_t radeon_8500_get_clock_memory(void)
{
    return g_memory_clock_8500;
}

uint32_t radeon_8500_get_temperature(void)
{
    uint32_t temp = mmio_read32_8500(REG_THERMAL_TEMP);
    g_temperature_8500 = temp;
    return temp;
}

void radeon_8500_set_fan_speed(uint32_t rpm)
{
    mmio_write32_8500(REG_FAN_PWM, rpm);
    g_fan_rpm_8500 = rpm;
}

uint32_t radeon_8500_get_fan_speed(void)
{
    uint32_t rpm = mmio_read32_8500(REG_FAN_RPM);
    g_fan_rpm_8500 = rpm;
    return rpm;
}

void radeon_8500_rgb_set(uint32_t color, uint8_t mode)
{
    mmio_write32_8500(REG_RGB_CONTROL, (uint32_t)mode);
    mmio_write32_8500(REG_RGB_COLOR, color);
}

void radeon_8500_rgb_off(void)
{
    mmio_write32_8500(REG_RGB_CONTROL, 0U);
}

radeon_8500_handle_t radeon_8500_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied, uint8_t triple_buffer)
{
    for (uint32_t i = 0; i < RADEON_8500_MAX_SWAP_CHAINS; i++) {
        if (g_swap_used_8500[i]) continue;
        g_swap_used_8500[i] = 1;
        g_swap_8500[i].width = width;
        g_swap_8500[i].height = height;
        g_swap_8500[i].back_buffer_addr = RADEON_8500_FB_ADDR + 0x1000000U + i * 0x200000U;
        g_swap_8500[i].front_buffer_addr = RADEON_8500_FB_ADDR;
        g_swap_8500[i].vsync = vsync;
        g_swap_8500[i].alpha_premultiplied = alpha_premultiplied;
        g_swap_8500[i].triple_buffer = triple_buffer;
        g_swap_8500[i].back_vram_offset = radeon_8500_vram_alloc((uint64_t)width * (uint64_t)height * 4U, 0U);
        g_swap_8500[i].front_vram_offset = radeon_8500_vram_alloc((uint64_t)width * (uint64_t)height * 4U, 0U);
        return (radeon_8500_handle_t)i;
    }
    return RADEON_8500_HANDLE_NULL;
}

void radeon_8500_swap_chain_destroy(radeon_8500_handle_t handle)
{
    static uint8_t g_swap_used_8500[RADEON_8500_MAX_SWAP_CHAINS] = {0};
    static struct radeon_8500_swap_chain g_swap_8500[RADEON_8500_MAX_SWAP_CHAINS];
    if (handle >= RADEON_8500_MAX_SWAP_CHAINS || !g_swap_used_8500[handle]) return;
    radeon_8500_vram_free(g_swap_8500[handle].back_vram_offset);
    radeon_8500_vram_free(g_swap_8500[handle].front_vram_offset);
    g_swap_used_8500[handle] = 0;
}

void radeon_8500_present(radeon_8500_handle_t chain)
{
    static uint8_t g_swap_used_8500[RADEON_8500_MAX_SWAP_CHAINS] = {0};
    static struct radeon_8500_swap_chain g_swap_8500[RADEON_8500_MAX_SWAP_CHAINS];
    if (chain >= RADEON_8500_MAX_SWAP_CHAINS || !g_swap_used_8500[chain]) return;
    if (g_swap_8500[chain].vsync) {
        for (volatile uint32_t wait = 0; wait < 2000000U; wait++) {
            if (mmio_read32_8500(REG_CRTC_CONTROL) & 0x80000000U) break;
        }
    }
    radeon_8500_cp_emit(0x40000000U | 0x02U);
    radeon_8500_cp_emit((0U << 16) | 0U);
    radeon_8500_cp_emit(((RADEON_8500_FB_HEIGHT - 1U) << 16) | (RADEON_8500_FB_WIDTH - 1U));
    radeon_8500_cp_emit((0U << 16) | 0U);
    radeon_8500_cp_emit(0U);
    radeon_8500_cp_flush();
}

static uint8_t g_cmd_recording_8500 = 0;
static uint32_t g_cmd_buf_8500[RADEON_8500_COMMAND_BUFFER_SIZE];
static uint32_t g_cmd_count_8500 = 0;

void radeon_8500_command_begin(void)
{
    g_cmd_recording_8500 = 1;
    g_cmd_count_8500 = 0;
}

void radeon_8500_command_end(void)
{
    if (!g_cmd_recording_8500) return;
    g_cmd_recording_8500 = 0;
    for (uint32_t i = 0; i < g_cmd_count_8500; i++) {
        radeon_8500_cp_emit(g_cmd_buf_8500[i]);
    }
    radeon_8500_cp_flush();
    g_cmd_count_8500 = 0;
}

void radeon_8500_context_get(struct radeon_8500_context *out)
{
    if (!out) return;
    *out = g_ctx_8500;
}

int radeon_8500_3d_available(void)
{
    return g_initialized_8500 ? 1 : 0;
}
