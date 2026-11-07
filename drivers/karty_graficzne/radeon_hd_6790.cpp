#include "radeon_hd_6790.h"

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

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                        uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

static volatile uint32_t *g_mmio = 0;
static volatile uint32_t *g_fb = (volatile uint32_t *)RADEON_6790_FB_ADDR;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint16_t g_device_id = 0;
static uint8_t g_revision = 0;
static int g_initialized = 0;
static uint32_t g_ring_gfx[RADEON_6790_RING_BUFFER_SIZE / 4U];
static uint32_t g_ring_gfx_head = 0;
static uint32_t g_ring_gfx_tail = 0;
static uint32_t g_ring_dma[RADEON_6790_DMA_RING_SIZE / 4U];
static uint32_t g_ring_dma_head = 0;
static uint32_t g_ring_dma_tail = 0;
static int g_cursor_visible = 0;
static uint8_t g_architecture = 1U;
static uint64_t g_vram_used = 0ULL;
static uint64_t g_vram_total = RADEON_6790_VRAM_SIZE_BYTES;
static uint32_t g_clock_gfx = 880U;
static uint32_t g_clock_memory = 1050U;
static uint32_t g_temperature = 52U;
static uint32_t g_fan_rpm = 1500U;
static uint8_t g_power_state = POWER_STATE_D0;
static uint8_t g_alpha_blend_global = 1U;

static struct radeon_6790_vram_alloc g_vram_allocs[RADEON_6790_VRAM_MAX_ALLOCS];
static uint32_t g_vram_alloc_count = 0U;

static struct radeon_6790_vertex_buffer g_vb[RADEON_6790_MAX_VERTEX_BUFFERS];
static struct radeon_6790_index_buffer g_ib[RADEON_6790_MAX_INDEX_BUFFERS];
static struct radeon_6790_texture g_tex[RADEON_6790_MAX_TEXTURES];
static struct radeon_6790_render_target g_rt[RADEON_6790_MAX_RENDER_TARGETS];
static struct radeon_6790_depth_stencil g_ds[RADEON_6790_MAX_DEPTH_STENCILS];
static struct radeon_6790_shader g_shader[RADEON_6790_MAX_SHADERS];
static struct radeon_6790_sampler_state g_sampler[RADEON_6790_MAX_SAMPLERS];
static struct radeon_6790_blend_state g_blend[RADEON_6790_MAX_BLEND_STATES];
static struct radeon_6790_rasterizer_state g_raster[RADEON_6790_MAX_RASTERIZER_STATES];
static struct radeon_6790_depth_stencil_state g_ds_state[RADEON_6790_MAX_DEPTH_STENCIL_STATES];
static struct radeon_6790_input_layout g_il[RADEON_6790_MAX_INPUT_LAYOUTS];
static struct radeon_6790_constant_buffer g_cb[RADEON_6790_MAX_CONSTANT_BUFFERS];
static struct radeon_6790_stream_output g_so[RADEON_6790_MAX_STREAM_OUTPUT];
static struct radeon_6790_tessellation g_tess[RADEON_6790_MAX_TESSELLATION];
static struct radeon_6790_swap_chain g_swap[RADEON_6790_MAX_SWAP_CHAINS];
static struct radeon_6790_fence g_fence[RADEON_6790_MAX_FENCES];
static struct radeon_6790_semaphore g_semaphore[RADEON_6790_MAX_SEMAPHORES];
static struct radeon_6790_occlusion_query g_occlusion[RADEON_6790_MAX_OCCLUSION_QUERIES];
static struct radeon_6790_pipeline_stats g_pstats[RADEON_6790_MAX_PIPELINE_STATS];

static uint8_t g_vb_used[RADEON_6790_MAX_VERTEX_BUFFERS] = {0};
static uint8_t g_ib_used[RADEON_6790_MAX_INDEX_BUFFERS] = {0};
static uint8_t g_tex_used[RADEON_6790_MAX_TEXTURES] = {0};
static uint8_t g_rt_used[RADEON_6790_MAX_RENDER_TARGETS] = {0};
static uint8_t g_ds_used[RADEON_6790_MAX_DEPTH_STENCILS] = {0};
static uint8_t g_shader_used[RADEON_6790_MAX_SHADERS] = {0};
static uint8_t g_sampler_used[RADEON_6790_MAX_SAMPLERS] = {0};
static uint8_t g_blend_used[RADEON_6790_MAX_BLEND_STATES] = {0};
static uint8_t g_raster_used[RADEON_6790_MAX_RASTERIZER_STATES] = {0};
static uint8_t g_ds_state_used[RADEON_6790_MAX_DEPTH_STENCIL_STATES] = {0};
static uint8_t g_il_used[RADEON_6790_MAX_INPUT_LAYOUTS] = {0};
static uint8_t g_cb_used[RADEON_6790_MAX_CONSTANT_BUFFERS] = {0};
static uint8_t g_so_used[RADEON_6790_MAX_STREAM_OUTPUT] = {0};
static uint8_t g_tess_used[RADEON_6790_MAX_TESSELLATION] = {0};
static uint8_t g_swap_used[RADEON_6790_MAX_SWAP_CHAINS] = {0};
static uint8_t g_fence_used[RADEON_6790_MAX_FENCES] = {0};
static uint8_t g_semaphore_used[RADEON_6790_MAX_SEMAPHORES] = {0};
static uint8_t g_occlusion_used[RADEON_6790_MAX_OCCLUSION_QUERIES] = {0};
static uint8_t g_pstats_used[RADEON_6790_MAX_PIPELINE_STATS] = {0};

static uint8_t g_cmd_recording = 0;
static uint32_t g_cmd_buf[RADEON_6790_COMMAND_BUFFER_SIZE];
static uint32_t g_cmd_count = 0;

static struct radeon_6790_context g_ctx = {};

static inline uint32_t mmio_read32(uint32_t offset)
{
    return g_mmio[offset >> 2];
}

static inline void mmio_write32(uint32_t offset, uint32_t value)
{
    g_mmio[offset >> 2] = value;
}

static inline void mmio_write32_posted(uint32_t offset, uint32_t value)
{
    mmio_write32(offset, value);
    (void)mmio_read32(offset);
}

static void radeon_6790_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

static int radeon_6790_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != RADEON_6790_VENDOR_ID) continue;
                if (device != RADEON_6790_DEVICE_ID_BARTS_PRO &&
                    device != RADEON_6790_DEVICE_ID_BARTS_XT &&
                    device != RADEON_6790_DEVICE_ID_BARTS_PRO_VAR1 &&
                    device != RADEON_6790_DEVICE_ID_BARTS_PRO_VAR2) continue;

                uint32_t bar0 = pci_read32(bus, slot, func, 0x10);
                if (bar0 == 0 || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;

                uint32_t command = pci_read32(bus, slot, func, 0x04);
                pci_write32(bus, slot, func, 0x04, command | 0x00000006);

                g_bus = bus;
                g_slot = slot;
                g_func = func;
                g_mmio_base = bar0 & 0xFFFFFFF0U;
                g_mmio = (volatile uint32_t *)g_mmio_base;
                g_device_id = device;
                g_revision = (uint8_t)(pci_read32(bus, slot, func, 0x08) & 0xFFU);
                return 1;
            }
        }
    }
    return 0;
}

static void radeon_6790_clear_interrupts(void)
{
    mmio_write32(REG_6790_INT_STAT, 0xFFFFFFFFU);
    mmio_write32(REG_6790_INT_MASK, 0x00000000U);
    mmio_write32(REG_6790_INT_ACK, 0xFFFFFFFFU);
}

static void radeon_6790_reset_cp(void)
{
    mmio_write32(REG_6790_CP_ME_CNTL, CP_ME_CNTL_ME_RESET | CP_ME_CNTL_PFP_RESET);
    radeon_6790_delay_us(100);
    mmio_write32(REG_6790_CP_ME_CNTL, 0U);
    radeon_6790_delay_us(100);
}

static void radeon_6790_cp_ring_init(void)
{
    mmio_write32(REG_6790_CP_ME_RB_ADDR, (uint32_t)&g_ring_gfx[0]);
    mmio_write32(REG_6790_CP_ME_RB_SIZE, sizeof(g_ring_gfx));
    mmio_write32(REG_6790_CP_ME_RB_WPTR, 0U);
    mmio_write32(REG_6790_CP_ME_RB_RPTR, 0U);
    mmio_write32(REG_6790_CP_ME_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_gfx_head = 0;
    g_ring_gfx_tail = 0;
}

static void radeon_6790_dma_ring_init(void)
{
    mmio_write32(REG_6790_DMA_RB_ADDR, (uint32_t)&g_ring_dma[0]);
    mmio_write32(REG_6790_DMA_RB_SIZE, sizeof(g_ring_dma));
    mmio_write32(REG_6790_DMA_RB_WPTR, 0U);
    mmio_write32(REG_6790_DMA_RB_RPTR, 0U);
    mmio_write32(REG_6790_DMA_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_dma_head = 0;
    g_ring_dma_tail = 0;
}

static void radeon_6790_cp_emit(uint32_t dword);
static void radeon_6790_cp_flush(void);

static void radeon_6790_cp_emit(uint32_t dword)
{
    if (((g_ring_gfx_head & (RADEON_6790_RING_BUFFER_SIZE / 4U - 1U)) + 5U) > (RADEON_6790_RING_BUFFER_SIZE / 4U)) {
        radeon_6790_cp_flush();
    }
    g_ring_gfx[g_ring_gfx_head & (RADEON_6790_RING_BUFFER_SIZE / 4U - 1U)] = dword;
    g_ring_gfx_head++;
}

static void radeon_6790_cp_flush(void)
{
    uint32_t tail = g_ring_gfx_head & (RADEON_6790_RING_BUFFER_SIZE / 4U - 1U);
    mmio_write32(REG_6790_CP_ME_RB_WPTR, tail);
    for (volatile uint32_t wait = 0; wait < 2000000U; wait++) {
        if (mmio_read32(REG_6790_CP_ME_RB_RPTR) == tail) {
            break;
        }
    }
    g_ring_gfx_tail = tail;
}

static void radeon_6790_dma_emit(uint32_t dword);
static void radeon_6790_dma_flush(void);

static void radeon_6790_dma_emit(uint32_t dword)
{
    if (((g_ring_dma_head & (RADEON_6790_DMA_RING_SIZE / 4U - 1U)) + 5U) > (RADEON_6790_DMA_RING_SIZE / 4U)) {
        radeon_6790_dma_flush();
    }
    g_ring_dma[g_ring_dma_head & (RADEON_6790_DMA_RING_SIZE / 4U - 1U)] = dword;
    g_ring_dma_head++;
}

static void radeon_6790_dma_flush(void)
{
    uint32_t tail = g_ring_dma_head & (RADEON_6790_DMA_RING_SIZE / 4U - 1U);
    mmio_write32(REG_6790_DMA_RB_WPTR, tail);
    for (volatile uint32_t wait = 0; wait < 2000000U; wait++) {
        if (mmio_read32(REG_6790_DMA_RB_RPTR) == tail) {
            break;
        }
    }
    g_ring_dma_tail = tail;
}

static void radeon_6790_display_init(void)
{
    mmio_write32(REG_6790_DCN_DISP_CONTROL, 0xFFFFFFFFU);

    uint32_t hblank = (((RADEON_6790_FB_WIDTH + 128U) - 1U) << 16) | (RADEON_6790_FB_WIDTH - 1U);
    uint32_t vblank = (((RADEON_6790_FB_HEIGHT + 32U) - 1U) << 16) | (RADEON_6790_FB_HEIGHT - 1U);
    uint32_t hsync = (((RADEON_6790_FB_WIDTH + 64U) - 1U) << 16) | (RADEON_6790_FB_WIDTH - 1U);
    uint32_t vsync = (((RADEON_6790_FB_HEIGHT + 4U) - 1U) << 16) | (RADEON_6790_FB_HEIGHT - 1U);

    mmio_write32(REG_6790_DCN_CRTC_H_TOTAL, hblank);
    mmio_write32(REG_6790_DCN_CRTC_V_TOTAL, vblank);
    mmio_write32(REG_6790_DCN_CRTC_H_BLANK, hblank);
    mmio_write32(REG_6790_DCN_CRTC_V_BLANK, vblank);
    mmio_write32(REG_6790_DCN_CRTC_H_SYNC_A, hsync);
    mmio_write32(REG_6790_DCN_CRTC_V_SYNC_A, vsync);

    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_VIEWPORT_START, 0U);
    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_VIEWPORT_END,
                 ((RADEON_6790_FB_HEIGHT - 1U) << 16) | (RADEON_6790_FB_WIDTH - 1U));

    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_SURFACE_ADDRESS, RADEON_6790_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_SURFACE_ADDRESS_HIGH, 0U);
    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_SURFACE_OFFSET_X, 0U);
    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_SURFACE_OFFSET_Y, 0U);
    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_SURFACE_PIXEL_PITCH, RADEON_6790_FB_PITCH);
    mmio_write32(REG_6790_DCN_CRTC_PRIMARY_SURFACE_TILE, 0U);

    mmio_write32(REG_6790_DCN_PLANE_OFFSET_X, 0U);
    mmio_write32(REG_6790_DCN_PLANE_OFFSET_Y, 0U);
    mmio_write32(REG_6790_DCN_PLANE_PIXEL_PITCH, RADEON_6790_FB_PITCH);
    mmio_write32(REG_6790_DCN_PLANE_SURFACE_ADDRESS, RADEON_6790_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_6790_DCN_PLANE_SURFACE_ADDRESS_HIGH, 0U);

    mmio_write32(REG_6790_DCN_CRTC_CONTROL, CRTC_CONTROL_EN | CRTC_CONTROL_PIXEL_WIDTH_32BPP);
    radeon_6790_delay_us(200);
}

static void radeon_6790_3d_pipeline_init(void)
{
    radeon_6790_reset_cp();
    radeon_6790_cp_ring_init();
    radeon_6790_dma_ring_init();

    mmio_write32(REG_6790_CB_TARGET0_BASE, RADEON_6790_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_6790_CB_TARGET0_PITCH, RADEON_6790_FB_PITCH);
    mmio_write32(REG_6790_CB_TARGET0_SLICE, 0U);
    mmio_write32(REG_6790_CB_TARGET0_FMASK, 0U);
    mmio_write32(REG_6790_CB_TARGET0_VIEW, 0U);

    mmio_write32(REG_6790_CB_SHADER_MASK, SHADER_MASK_ENABLE | SHADER_MASK_CB0_ENABLE);

    mmio_write32(REG_6790_CB_CLEAR_COLOR, 0x00000000U);
    mmio_write32(REG_6790_CB_CLEAR_COLOR_HIGH, 0x00000000U);

    uint32_t blend = (RADEON_6790_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_SHIFT) |
                     (RADEON_6790_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_SHIFT) |
                     (RADEON_6790_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_SHIFT) |
                     (RADEON_6790_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT) |
                     (RADEON_6790_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT) |
                     (RADEON_6790_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT);
    mmio_write32(REG_6790_CB_BLEND_CONTROL, blend);

    mmio_write32(REG_6790_DB_Z_INFO, DB_Z_INFO_FORMAT_24BIT | DB_Z_INFO_COMPRESSION_ENABLE);
    mmio_write32(REG_6790_DB_Z_READ_BASE, (RADEON_6790_FB_ADDR + 0x20000000ULL) & 0xFFFFFFFFU);
    mmio_write32(REG_6790_DB_Z_READ_BASE_HIGH, 0U);
    mmio_write32(REG_6790_DB_Z_WRITE_BASE, (RADEON_6790_FB_ADDR + 0x20000000ULL) & 0xFFFFFFFFU);
    mmio_write32(REG_6790_DB_Z_WRITE_BASE_HIGH, 0U);
    mmio_write32(REG_6790_DB_Z_READ_OFF, 0U);
    mmio_write32(REG_6790_DB_Z_WRITE_OFF, 0U);
    mmio_write32(REG_6790_DB_CLEAR, 0x00FFFFFFU);
    mmio_write32(REG_6790_DB_STENCIL_CLEAR, 0U);
    mmio_write32(REG_6790_DB_DEPTH_BOUNDS_MIN, 0U);
    mmio_write32(REG_6790_DB_DEPTH_BOUNDS_MAX, 0xFFFFFFFFU);
    mmio_write32(REG_6790_DB_DEPTH_RANGE, 0x00000000U);

    mmio_write32(REG_6790_SQ_CONFIG, 0U);
    mmio_write32(REG_6790_SQ_VTX_BASE, 0U);
    mmio_write32(REG_6790_SQ_VTX_STRIDE, 0U);
    mmio_write32(REG_6790_SQ_VTX_NUM_FORMAT, 0U);
    mmio_write32(REG_6790_SQ_VTX_FMT_0, 0U);
    mmio_write32(REG_6790_SQ_VTX_FMT_1, 0U);
    mmio_write32(REG_6790_SQ_VTX_FMT_2, 0U);
    mmio_write32(REG_6790_SQ_VTX_FMT_3, 0U);

    mmio_write32(REG_6790_SQ_COMPUTE_PROGRAM, 0U);

    mmio_write32(REG_6790_SQ_TEX_BASE, 0U);
    mmio_write32(REG_6790_SQ_SAMPLER_BASE, 0U);
    mmio_write32(REG_6790_SQ_TEX_LOD, 0U);
    mmio_write32(REG_6790_SQ_TEX_DIM, 0U);

    mmio_write32(REG_6790_SQ_STREAMOUT_BASE, 0U);
    mmio_write32(REG_6790_SQ_STREAMOUT_SIZE, 0U);

    mmio_write32(REG_6790_VGT_LS_HS_CONFIG, 0U);
    mmio_write32(REG_6790_VGT_ES_PER_GS, 0U);
    mmio_write32(REG_6790_VGT_GS_PER_VS, 0U);
    mmio_write32(REG_6790_VGT_VTX_CNTL, 0U);
    mmio_write32(REG_6790_VGT_HOS_MAX_TESS_FACTOR, 0U);
    mmio_write32(REG_6790_VGT_HOS_MIN_TESS_FACTOR, 0U);

    mmio_write32(REG_6790_PA_SU_VTX_CNTL, 0U);
    mmio_write32(REG_6790_PA_CL_VTE_CNTL, 0U);
    mmio_write32(REG_6790_PA_CL_VS_OUT_CNTL, 0U);
    mmio_write32(REG_6790_PA_CL_GB_ADV, 0U);
    mmio_write32(REG_6790_PA_CL_CLIP_CNTL, 0U);
    mmio_write32(REG_6790_PA_SU_SC_MODE_CNTL, 0U);
    mmio_write32(REG_6790_PA_SC_WINDOW_OFFSET, 0U);
    mmio_write32(REG_6790_PA_SC_WINDOW_SCISSOR_TL, 0U);
    mmio_write32(REG_6790_PA_SC_WINDOW_SCISSOR_BR,
                 ((RADEON_6790_FB_HEIGHT - 1U) << 16) | (RADEON_6790_FB_WIDTH - 1U));
    mmio_write32(REG_6790_PA_SU_PRIM_FILTER_CNTL, 0U);
    mmio_write32(REG_6790_PA_SU_LINE_STIPPLE, 0U);
    mmio_write32(REG_6790_PA_SC_AA_CONFIG, 0U);
    mmio_write32(REG_6790_PA_SU_POLY_OFFSET_FRONT, 0U);
    mmio_write32(REG_6790_PA_SU_POLY_OFFSET_BACK, 0U);

    mmio_write32(REG_6790_VM_CONTEXT, 0U);
    mmio_write32(REG_6790_VM_PAGE_TABLE, 0U);

    radeon_6790_delay_us(200);
}

static void radeon_6790_cursor_init(void)
{
    mmio_write32(REG_6790_CURSOR_CONTROL, 0U);
    mmio_write32(REG_6790_CURSOR_POSITION, 0U);
    mmio_write32(REG_6790_CURSOR_COLOR0, 0x00000000U);
    mmio_write32(REG_6790_CURSOR_COLOR1, 0x00FFFFFFU);
    mmio_write32(REG_6790_CURSOR_BASE, RADEON_6790_FB_ADDR);
    g_cursor_visible = 0;
}

static void radeon_6790_power_init(void)
{
    mmio_write32(REG_6790_POWER_STATE, POWER_STATE_D0);
    mmio_write32(REG_6790_GFX_CLOCK_CNTL, g_clock_gfx);
    mmio_write32(REG_6790_MEMORY_CLOCK_CNTL, g_clock_memory);
    mmio_write32(REG_6790_DISPLAY_CLOCK_CNTL, g_clock_gfx / 2U);
    radeon_6790_delay_us(100);
}

int radeon_6790_init(void)
{
    if (g_initialized) return 1;
    if (!radeon_6790_find_device()) return 0;

    uint32_t command = pci_read32(g_bus, g_slot, g_func, 0x04);
    pci_write32(g_bus, g_slot, g_func, 0x04, command | 0x00000006U);

    radeon_6790_clear_interrupts();

    radeon_6790_power_init();
    radeon_6790_display_init();
    radeon_6790_3d_pipeline_init();
    radeon_6790_cursor_init();

    g_initialized = 1;
    return 1;
}

int radeon_6790_is_ready(void)
{
    return g_initialized;
}

void radeon_6790_shutdown(void)
{
    if (!g_initialized) return;
    mmio_write32(REG_6790_CP_ME_CNTL, CP_ME_CNTL_ME_HALT | CP_ME_CNTL_PFP_HALT);
    mmio_write32(REG_6790_DMA_RB_CNTL, 0U);
    mmio_write32(REG_6790_POWER_STATE, POWER_STATE_D3);
    g_initialized = 0;
}

void radeon_6790_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= RADEON_6790_FB_WIDTH || y >= RADEON_6790_FB_HEIGHT) return;
    g_fb[y * (RADEON_6790_FB_PITCH / 4U) + x] = color;
}

void radeon_6790_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (g_initialized) {
        radeon_6790_cp_emit(0x40000000U | 0x01U);
        radeon_6790_cp_emit((y << 16) | x);
        radeon_6790_cp_emit(((h - 1U) << 16) | (w - 1U));
        radeon_6790_cp_emit(color);
        radeon_6790_cp_emit(0U);
        radeon_6790_cp_flush();
        return;
    }
    uint32_t xx;
    uint32_t yy;
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= RADEON_6790_FB_WIDTH || yy >= RADEON_6790_FB_HEIGHT) continue;
            g_fb[yy * (RADEON_6790_FB_PITCH / 4U) + xx] = color;
        }
    }
}

void radeon_6790_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                     uint32_t w, uint32_t h)
{
    if (!g_initialized) return;

    radeon_6790_cp_emit(0x40000000U | 0x02U);
    radeon_6790_cp_emit((dst_y << 16) | dst_x);
    radeon_6790_cp_emit(((h - 1U) << 16) | (w - 1U));
    radeon_6790_cp_emit((src_y << 16) | src_x);
    radeon_6790_cp_emit(0U);
    radeon_6790_cp_flush();
}

void radeon_6790_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    if (!g_initialized) return;

    if (image) {
        for (uint32_t i = 0; i < 64U * 64U; i++) {
            g_fb[i] = image[i];
        }
        mmio_write32(REG_6790_CURSOR_BASE, RADEON_6790_FB_ADDR);
    }

    mmio_write32(REG_6790_CURSOR_POSITION, (y << 16) | x);
    mmio_write32(REG_6790_CURSOR_CONTROL, 0x80000000U);
    g_cursor_visible = 1;
}

void radeon_6790_cursor_hide(void)
{
    if (!g_initialized) return;
    mmio_write32(REG_6790_CURSOR_CONTROL, 0U);
    g_cursor_visible = 0;
}

void *radeon_6790_get_fb(void)
{
    return (void *)g_fb;
}

uint32_t radeon_6790_get_width(void)
{
    return RADEON_6790_FB_WIDTH;
}

uint32_t radeon_6790_get_height(void)
{
    return RADEON_6790_FB_HEIGHT;
}

uint16_t radeon_6790_get_device_id(void)
{
    return g_device_id;
}

uint8_t radeon_6790_get_revision(void)
{
    return g_revision;
}

void radeon_6790_info_get(struct radeon_6790_info *out)
{
    if (!out) return;
    out->vendor_id = RADEON_6790_VENDOR_ID;
    out->device_id = g_device_id;
    out->revision_id = g_revision;
    out->mmio_base = g_mmio_base;
    out->fb_addr = RADEON_6790_FB_ADDR;
    out->fb_width = RADEON_6790_FB_WIDTH;
    out->fb_height = RADEON_6790_FB_HEIGHT;
    out->fb_pitch = RADEON_6790_FB_PITCH;
    out->initialized = (uint8_t)g_initialized;
    out->architecture = g_architecture;
    out->vram_size = g_vram_total;
    out->vram_used = g_vram_used;
    out->gfx_clock_mhz = g_clock_gfx;
    out->memory_clock_mhz = g_clock_memory;
    out->temperature_c = g_temperature;
    out->fan_rpm = g_fan_rpm;
}

uint64_t radeon_6790_vram_alloc(uint64_t size, uint32_t type)
{
    (void)type;
    if (size == 0) return 0xFFFFFFFFFFFFFFFFULL;
    if (g_vram_alloc_count >= RADEON_6790_VRAM_MAX_ALLOCS) return 0xFFFFFFFFFFFFFFFFULL;
    uint64_t align = RADEON_6790_VRAM_PAGE_SIZE;
    uint64_t offset = (g_vram_used + align - 1ULL) & ~(align - 1ULL);
    if (offset + size > g_vram_total) return 0xFFFFFFFFFFFFFFFFULL;
    uint32_t i = g_vram_alloc_count;
    g_vram_allocs[i].used = 1;
    g_vram_allocs[i].offset = offset;
    g_vram_allocs[i].size = size;
    g_vram_allocs[i].type = type;
    g_vram_alloc_count++;
    g_vram_used = offset + size;
    return offset;
}

void radeon_6790_vram_free(uint64_t handle)
{
    for (uint32_t i = 0; i < g_vram_alloc_count; i++) {
        if (g_vram_allocs[i].used && g_vram_allocs[i].offset == handle) {
            g_vram_allocs[i].used = 0;
            return;
        }
    }
}

uint64_t radeon_6790_vram_used(void)
{
    return g_vram_used;
}

uint64_t radeon_6790_vram_total(void)
{
    return g_vram_total;
}

static uint32_t find_free_slot(uint8_t *used, uint32_t max)
{
    for (uint32_t i = 0; i < max; i++) {
        if (!used[i]) return i;
    }
    return RADEON_6790_HANDLE_NULL;
}

static int handle_valid(radeon_6790_handle_t h, uint8_t *used, uint32_t max)
{
    return h != RADEON_6790_HANDLE_NULL && h < max && used[h];
}

radeon_6790_handle_t radeon_6790_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_vb_used, RADEON_6790_MAX_VERTEX_BUFFERS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_vertex_buffer *vb = &g_vb[i];
    vb->used = 1;
    vb->vertex_count = vertex_count;
    vb->vertex_size = vertex_size;
    vb->format = format;
    vb->buffer_addr = RADEON_6790_FB_ADDR + 0x40000000U + i * 0x200000U;
    vb->system_mem = 0;
    vb->vram_offset = radeon_6790_vram_alloc((uint64_t)vertex_count * vertex_size, 1U);

    if (data) {
        vb->system_mem = (uint8_t *)vb->buffer_addr;
        for (uint32_t k = 0; k < vertex_count * vertex_size; k++) {
            vb->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_6790_handle_t)i;
}

void radeon_6790_vertex_buffer_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_vb_used, RADEON_6790_MAX_VERTEX_BUFFERS)) return;
    radeon_6790_vram_free(g_vb[handle].vram_offset);
    g_vb[handle].used = 0;
}

void radeon_6790_vertex_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_vb_used, RADEON_6790_MAX_VERTEX_BUFFERS)) return;
    struct radeon_6790_vertex_buffer *vb = &g_vb[handle];
    if (!vb->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        vb->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_6790_handle_t radeon_6790_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_ib_used, RADEON_6790_MAX_INDEX_BUFFERS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_index_buffer *ib = &g_ib[i];
    ib->used = 1;
    ib->index_count = index_count;
    ib->format = format;
    ib->buffer_addr = RADEON_6790_FB_ADDR + 0x60000000U + i * 0x200000U;
    ib->system_mem = 0;
    ib->vram_offset = radeon_6790_vram_alloc((uint64_t)index_count * 4U, 2U);

    if (data) {
        ib->system_mem = (uint8_t *)ib->buffer_addr;
        uint32_t elem_size = (format == RADEON_6790_INDEX_FORMAT_32BIT) ? 4U : 2U;
        for (uint32_t k = 0; k < index_count * elem_size; k++) {
            ib->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_6790_handle_t)i;
}

void radeon_6790_index_buffer_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_ib_used, RADEON_6790_MAX_INDEX_BUFFERS)) return;
    radeon_6790_vram_free(g_ib[handle].vram_offset);
    g_ib[handle].used = 0;
}

void radeon_6790_index_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_ib_used, RADEON_6790_MAX_INDEX_BUFFERS)) return;
    struct radeon_6790_index_buffer *ib = &g_ib[handle];
    if (!ib->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        ib->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_6790_handle_t radeon_6790_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
{
    uint32_t i = find_free_slot(g_tex_used, RADEON_6790_MAX_TEXTURES);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_texture *t = &g_tex[i];
    t->used = 1;
    t->width = width;
    t->height = height;
    t->format = format;
    t->mip_levels = mip_levels;
    t->array_size = 1U;
    t->sample_count = 1U;
    t->pitch = ((width * 4U + 3U) & ~3U);
    t->buffer_addr = RADEON_6790_FB_ADDR + 0x80000000U + i * 0x400000U;
    t->system_mem = 0;
    t->vram_offset = radeon_6790_vram_alloc((uint64_t)width * height * 4ULL * mip_levels, 3U);
    t->compressed = 0;
    t->generate_mipmaps = 0;

    if (data) {
        t->system_mem = (uint8_t *)t->buffer_addr;
        uint32_t bpp = 4U;
        for (uint32_t k = 0; k < width * height * bpp; k++) {
            t->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_6790_handle_t)i;
}

void radeon_6790_texture_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, RADEON_6790_MAX_TEXTURES)) return;
    radeon_6790_vram_free(g_tex[handle].vram_offset);
    g_tex[handle].used = 0;
}

void radeon_6790_texture_update(radeon_6790_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
{
    (void)level;
    if (!handle_valid(handle, g_tex_used, RADEON_6790_MAX_TEXTURES)) return;
    struct radeon_6790_texture *t = &g_tex[handle];
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

void radeon_6790_texture_generate_mipmaps(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, RADEON_6790_MAX_TEXTURES)) return;
    struct radeon_6790_texture *t = &g_tex[handle];
    t->generate_mipmaps = 1;
    uint32_t w = t->width;
    uint32_t h = t->height;
    for (uint32_t level = 1; level < t->mip_levels && w > 1 && h > 1; level++) {
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
    }
}

radeon_6790_handle_t radeon_6790_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend)
{
    uint32_t i = find_free_slot(g_rt_used, RADEON_6790_MAX_RENDER_TARGETS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_render_target *rt = &g_rt[i];
    rt->used = 1;
    rt->width = width;
    rt->height = height;
    rt->format = format;
    rt->array_size = 1U;
    rt->sample_count = 1U;
    rt->buffer_addr = RADEON_6790_FB_ADDR + 0xC0000000ULL + (uint64_t)i * 0x10000000ULL;
    rt->alpha_blend = alpha_blend;
    rt->vram_offset = radeon_6790_vram_alloc((uint64_t)width * height * 4ULL, 4U);
    return (radeon_6790_handle_t)i;
}

void radeon_6790_render_target_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_rt_used, RADEON_6790_MAX_RENDER_TARGETS)) return;
    radeon_6790_vram_free(g_rt[handle].vram_offset);
    g_rt[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression)
{
    uint32_t i = find_free_slot(g_ds_used, RADEON_6790_MAX_DEPTH_STENCILS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_depth_stencil *ds = &g_ds[i];
    ds->used = 1;
    ds->width = width;
    ds->height = height;
    ds->format = format;
    ds->array_size = 1U;
    ds->sample_count = 1U;
    ds->buffer_addr = RADEON_6790_FB_ADDR + 0xE0000000ULL + (uint64_t)i * 0x10000000ULL;
    ds->compression = compression;
    ds->depth_bounds = 0;
    ds->vram_offset = radeon_6790_vram_alloc((uint64_t)width * height * 4ULL, 5U);
    return (radeon_6790_handle_t)i;
}

void radeon_6790_depth_stencil_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_ds_used, RADEON_6790_MAX_DEPTH_STENCILS)) return;
    radeon_6790_vram_free(g_ds[handle].vram_offset);
    g_ds[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)
{
    uint32_t i = find_free_slot(g_shader_used, RADEON_6790_MAX_SHADERS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_shader *s = &g_shader[i];
    s->used = 1;
    s->type = type;
    s->code_size = code_size;
    s->code = (uint8_t *)code;
    s->constant_count = 0;
    s->compiled = 0;
    return (radeon_6790_handle_t)i;
}

void radeon_6790_shader_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RADEON_6790_MAX_SHADERS)) return;
    g_shader[handle].used = 0;
}

void radeon_6790_shader_compile(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RADEON_6790_MAX_SHADERS)) return;
    g_shader[handle].compiled = 1;
}

radeon_6790_handle_t radeon_6790_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t max_anisotropy, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint8_t compare_enable, uint8_t compare_func, uint8_t reduction_zero)
{
    uint32_t i = find_free_slot(g_sampler_used, RADEON_6790_MAX_SAMPLERS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_sampler_state *s = &g_sampler[i];
    s->used = 1;
    s->filter_min = filter_min;
    s->filter_mag = filter_mag;
    s->filter_mip = filter_mip;
    s->max_anisotropy = max_anisotropy;
    s->address_u = address_u;
    s->address_v = address_v;
    s->address_w = address_w;
    s->compare_enable = compare_enable;
    s->compare_func = compare_func;
    s->reduction_zero = reduction_zero;
    return (radeon_6790_handle_t)i;
}

void radeon_6790_sampler_state_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_sampler_used, RADEON_6790_MAX_SAMPLERS)) return;
    g_sampler[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor, uint8_t alpha_to_coverage, uint8_t dual_source_blend)
{
    uint32_t i = find_free_slot(g_blend_used, RADEON_6790_MAX_BLEND_STATES);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_blend_state *b = &g_blend[i];
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
    b->alpha_to_coverage = alpha_to_coverage;
    b->dual_source_blend = dual_source_blend;
    return (radeon_6790_handle_t)i;
}

void radeon_6790_blend_state_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_blend_used, RADEON_6790_MAX_BLEND_STATES)) return;
    g_blend[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage, uint8_t depth_bias_enable, float depth_bias_constant, float depth_bias_slope, uint8_t conservative_raster)
{
    uint32_t i = find_free_slot(g_raster_used, RADEON_6790_MAX_RASTERIZER_STATES);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_rasterizer_state *r = &g_raster[i];
    r->used = 1;
    r->fill_mode = fill_mode;
    r->cull_mode = cull_mode;
    r->front_counter_clockwise = front_ccw;
    r->depth_clip_enable = depth_clip;
    r->scissor_enable = scissor;
    r->multisample_enable = multisample;
    r->antialiased_line_enable = aa_line;
    r->alpha_to_coverage = alpha_to_coverage;
    r->depth_bias_enable = depth_bias_enable;
    r->depth_bias_constant = depth_bias_constant;
    r->depth_bias_slope = depth_bias_slope;
    r->conservative_raster = conservative_raster;
    return (radeon_6790_handle_t)i;
}

void radeon_6790_rasterizer_state_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_raster_used, RADEON_6790_MAX_RASTERIZER_STATES)) return;
    g_raster[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func, uint8_t depth_bounds_enable)
{
    uint32_t i = find_free_slot(g_ds_state_used, RADEON_6790_MAX_DEPTH_STENCIL_STATES);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_depth_stencil_state *d = &g_ds_state[i];
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
    d->depth_bounds_enable = depth_bounds_enable;
    return (radeon_6790_handle_t)i;
}

void radeon_6790_depth_stencil_state_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_ds_state_used, RADEON_6790_MAX_DEPTH_STENCIL_STATES)) return;
    g_ds_state[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)
{
    uint32_t i = find_free_slot(g_il_used, RADEON_6790_MAX_INPUT_LAYOUTS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_input_layout *l = &g_il[i];
    l->used = 1;
    l->element_count = element_count;
    for (uint32_t k = 0; k < element_count && k < 32U; k++) {
        l->slot[k] = slots ? slots[k] : 0U;
        l->offset[k] = offsets ? offsets[k] : 0U;
        l->format[k] = formats ? formats[k] : RADEON_6790_VERTEX_FORMAT_XYZ;
    }
    return (radeon_6790_handle_t)i;
}

void radeon_6790_input_layout_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_il_used, RADEON_6790_MAX_INPUT_LAYOUTS)) return;
    g_il[handle].used = 0;
}

radeon_6790_handle_t radeon_6790_constant_buffer_create(uint32_t size)
{
    uint32_t i = find_free_slot(g_cb_used, RADEON_6790_MAX_CONSTANT_BUFFERS);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_constant_buffer *c = &g_cb[i];
    c->used = 1;
    c->size = size;
    c->data = (uint8_t *)0;
    c->vram_offset = radeon_6790_vram_alloc(size, 6U);
    return (radeon_6790_handle_t)i;
}

void radeon_6790_constant_buffer_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_cb_used, RADEON_6790_MAX_CONSTANT_BUFFERS)) return;
    radeon_6790_vram_free(g_cb[handle].vram_offset);
    g_cb[handle].used = 0;
}

void radeon_6790_constant_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_cb_used, RADEON_6790_MAX_CONSTANT_BUFFERS)) return;
    struct radeon_6790_constant_buffer *c = &g_cb[handle];
    if (!c->data) return;
    for (uint32_t k = 0; k < size; k++) {
        c->data[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_6790_handle_t radeon_6790_stream_output_create(uint32_t buffer_count, const uint64_t *sizes)
{
    uint32_t i = find_free_slot(g_so_used, RADEON_6790_MAX_STREAM_OUTPUT);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_stream_output *so = &g_so[i];
    so->used = 1;
    so->buffer_count = buffer_count > 4U ? 4U : buffer_count;
    for (uint32_t k = 0; k < so->buffer_count; k++) {
        so->vram_offset[k] = radeon_6790_vram_alloc(sizes ? sizes[k] : 0ULL, 8U);
        so->size[k] = sizes ? (uint32_t)sizes[k] : 0U;
        so->written[k] = 0U;
    }
    return (radeon_6790_handle_t)i;
}

void radeon_6790_stream_output_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_so_used, RADEON_6790_MAX_STREAM_OUTPUT)) return;
    struct radeon_6790_stream_output *so = &g_so[handle];
    for (uint32_t k = 0; k < so->buffer_count; k++) {
        radeon_6790_vram_free(so->vram_offset[k]);
    }
    g_so[handle].used = 0;
}

void radeon_6790_stream_output_update(radeon_6790_handle_t handle, uint32_t slot, const void *data, uint32_t size)
{
    (void)data;
    if (!handle_valid(handle, g_so_used, RADEON_6790_MAX_STREAM_OUTPUT)) return;
    struct radeon_6790_stream_output *so = &g_so[handle];
    if (slot >= so->buffer_count) return;
    uint32_t write_size = size > so->size[slot] ? so->size[slot] : size;
    so->written[slot] = write_size;
}

radeon_6790_handle_t radeon_6790_tessellation_state_create(uint32_t patch_vertices, const float *edge_factors, const float *inside_factors, uint8_t domain_triangles, uint8_t fractional_odd)
{
    uint32_t i = find_free_slot(g_tess_used, RADEON_6790_MAX_TESSELLATION);
    if (i == RADEON_6790_HANDLE_NULL) return RADEON_6790_HANDLE_NULL;

    struct radeon_6790_tessellation *t = &g_tess[i];
    t->used = 1;
    t->patch_vertices = patch_vertices;
    for (uint32_t k = 0; k < 4U; k++) {
        t->tess_factor_edge[k] = edge_factors ? edge_factors[k] : 1.0f;
    }
    for (uint32_t k = 0; k < 2U; k++) {
        t->tess_factor_inside[k] = inside_factors ? inside_factors[k] : 1.0f;
    }
    t->domain_is_triangles = domain_triangles;
    t->partitioning_fractional_odd = fractional_odd;
    return (radeon_6790_handle_t)i;
}

void radeon_6790_tessellation_state_destroy(radeon_6790_handle_t handle)
{
    if (!handle_valid(handle, g_tess_used, RADEON_6790_MAX_TESSELLATION)) return;
    g_tess[handle].used = 0;
}

void radeon_6790_tessellation_state_update(radeon_6790_handle_t handle, const float *edge_factors, const float *inside_factors)
{
    if (!handle_valid(handle, g_tess_used, RADEON_6790_MAX_TESSELLATION)) return;
    struct radeon_6790_tessellation *t = &g_tess[handle];
    for (uint32_t k = 0; k < 4U; k++) {
        t->tess_factor_edge[k] = edge_factors ? edge_factors[k] : t->tess_factor_edge[k];
    }
    for (uint32_t k = 0; k < 2U; k++) {
        t->tess_factor_inside[k] = inside_factors ? inside_factors[k] : t->tess_factor_inside[k];
    }
}

void radeon_6790_bind_vertex_buffer(radeon_6790_handle_t handle)
{
    g_ctx.vertex_buffer = handle;
}

void radeon_6790_bind_index_buffer(radeon_6790_handle_t handle)
{
    g_ctx.index_buffer = handle;
}

void radeon_6790_bind_input_layout(radeon_6790_handle_t handle)
{
    g_ctx.input_layout = handle;
}

void radeon_6790_bind_vertex_shader(radeon_6790_handle_t handle)
{
    g_ctx.vertex_shader = handle;
}

void radeon_6790_bind_pixel_shader(radeon_6790_handle_t handle)
{
    g_ctx.pixel_shader = handle;
}

void radeon_6790_bind_geometry_shader(radeon_6790_handle_t handle)
{
    g_ctx.geometry_shader = handle;
}

void radeon_6790_bind_compute_shader(radeon_6790_handle_t handle)
{
    g_ctx.compute_shader = handle;
}

void radeon_6790_bind_hull_shader(radeon_6790_handle_t handle)
{
    g_ctx.hull_shader = handle;
}

void radeon_6790_bind_domain_shader(radeon_6790_handle_t handle)
{
    g_ctx.domain_shader = handle;
}

void radeon_6790_bind_render_target(radeon_6790_handle_t handle)
{
    g_ctx.render_target = handle;
}

void radeon_6790_bind_depth_stencil(radeon_6790_handle_t handle)
{
    g_ctx.depth_stencil = handle;
}

void radeon_6790_bind_blend_state(radeon_6790_handle_t handle)
{
    g_ctx.blend_state = handle;
}

void radeon_6790_bind_rasterizer_state(radeon_6790_handle_t handle)
{
    g_ctx.rasterizer_state = handle;
}

void radeon_6790_bind_depth_stencil_state(radeon_6790_handle_t handle)
{
    g_ctx.depth_stencil_state = handle;
}

void radeon_6790_bind_sampler_and_texture(uint32_t slot, radeon_6790_handle_t sampler, radeon_6790_handle_t texture)
{
    if (slot >= RADEON_6790_MAX_SAMPLERS) return;
    g_ctx.sampler[slot] = sampler;
    g_ctx.texture[slot] = texture;
}

void radeon_6790_bind_constant_buffer(uint32_t slot, radeon_6790_handle_t handle)
{
    if (slot >= RADEON_6790_MAX_CONSTANT_BUFFERS) return;
    g_ctx.constant_buffer[slot] = handle;
}

void radeon_6790_bind_stream_output(uint32_t slot, radeon_6790_handle_t handle)
{
    if (slot >= RADEON_6790_MAX_STREAM_OUTPUT) return;
    g_ctx.stream_output[slot] = handle;
}

void radeon_6790_bind_tessellation(radeon_6790_handle_t handle)
{
    g_ctx.tessellation = handle;
}

void radeon_6790_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
    g_ctx.viewport_x = x;
    g_ctx.viewport_y = y;
    g_ctx.viewport_w = w;
    g_ctx.viewport_h = h;
    g_ctx.viewport_min_depth = min_depth;
    g_ctx.viewport_max_depth = max_depth;

    uint32_t packed = (x & 0xFFFFU) | ((y & 0xFFFFU) << 16U);
    mmio_write32(REG_6790_PA_SC_WINDOW_OFFSET, packed);
    packed = (w & 0xFFFFU) | ((h & 0xFFFFU) << 16U);
    mmio_write32(REG_6790_PA_SC_WINDOW_OFFSET + 4U, packed);
}

void radeon_6790_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    g_ctx.scissor_left = left;
    g_ctx.scissor_top = top;
    g_ctx.scissor_right = right;
    g_ctx.scissor_bottom = bottom;

    uint32_t packed = (left & 0xFFFFU) | ((top & 0xFFFFU) << 16U);
    mmio_write32(REG_6790_PA_SC_WINDOW_SCISSOR_TL, packed);
    packed = (right & 0xFFFFU) | ((bottom & 0xFFFFU) << 16U);
    mmio_write32(REG_6790_PA_SC_WINDOW_SCISSOR_BR, packed);
}

void radeon_6790_set_primitive_topology(uint32_t topology)
{
    g_ctx.primitive_topology = topology;
    mmio_write32(REG_6790_PA_SU_PRIM_FILTER_CNTL, topology);
}

void radeon_6790_set_alpha_blend_global(uint8_t enable)
{
    g_alpha_blend_global = enable;
}

void radeon_6790_set_depth_bounds(float min_depth, float max_depth)
{
    uint32_t min_bits = 0;
    uint32_t max_bits = 0;
    const uint8_t *min_bytes = (const uint8_t *)&min_depth;
    const uint8_t *max_bytes = (const uint8_t *)&max_depth;
    for (int k = 0; k < 4; k++) {
        ((uint8_t *)&min_bits)[k] = min_bytes[k];
        ((uint8_t *)&max_bits)[k] = max_bytes[k];
    }
    mmio_write32(REG_6790_DB_DEPTH_BOUNDS_MIN, min_bits);
    mmio_write32(REG_6790_DB_DEPTH_BOUNDS_MAX, max_bits);
}

void radeon_6790_set_render_target_array_slice(uint32_t slice)
{
    g_ctx.render_target_array_slice = slice;
    mmio_write32(REG_6790_CB_TARGET0_VIEW, slice);
}

void radeon_6790_set_msaa_samples(uint32_t samples)
{
    g_ctx.msaa_samples = samples;
    uint32_t aa = samples > 1U ? (samples << 16) | samples : 0U;
    mmio_write32(REG_6790_PA_SC_AA_CONFIG, aa);
}

void radeon_6790_clear_render_target(radeon_6790_handle_t rt, uint32_t color)
{
    if (!handle_valid(rt, g_rt_used, RADEON_6790_MAX_RENDER_TARGETS)) return;
    struct radeon_6790_render_target *target = &g_rt[rt];
    mmio_write32(REG_6790_CB_CLEAR_COLOR, color);
    mmio_write32(REG_6790_CB_CLEAR_COLOR_HIGH, 0U);
    mmio_write32(REG_6790_CB_TARGET0_BASE, target->buffer_addr);
    mmio_write32(REG_6790_CB_TARGET0_PITCH, target->width * 4U);
    radeon_6790_delay_us(100);
}

void radeon_6790_clear_depth_stencil(radeon_6790_handle_t ds, float depth, uint8_t stencil)
{
    if (!handle_valid(ds, g_ds_used, RADEON_6790_MAX_DEPTH_STENCILS)) return;
    struct radeon_6790_depth_stencil *depth_stencil = &g_ds[ds];
    uint32_t depth_bits = 0;
    const uint8_t *depth_bytes = (const uint8_t *)&depth;
    for (int k = 0; k < 4; k++) {
        ((uint8_t *)&depth_bits)[k] = depth_bytes[k];
    }
    mmio_write32(REG_6790_DB_CLEAR, depth_bits);
    mmio_write32(REG_6790_DB_STENCIL_CLEAR, (uint32_t)stencil);
    mmio_write32(REG_6790_DB_Z_INFO, depth_stencil->buffer_addr | (depth_stencil->compression ? DB_Z_INFO_COMPRESSION_ENABLE : 0U));
    radeon_6790_delay_us(100);
}

void radeon_6790_draw(uint32_t vertex_count, uint32_t start_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_6790_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 8U, g_ctx.geometry_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 12U, g_ctx.hull_shader);

    radeon_6790_cp_emit(0x80000000U | 0x10U);
    radeon_6790_cp_emit(vertex_count);
    radeon_6790_cp_emit(start_vertex);
    radeon_6790_cp_emit(g_ctx.vertex_buffer);
    radeon_6790_cp_emit(g_ctx.primitive_topology);
    radeon_6790_cp_emit(g_ctx.blend_state);
    radeon_6790_cp_emit(g_ctx.rasterizer_state);
    radeon_6790_cp_emit(g_ctx.depth_stencil_state);
    radeon_6790_cp_emit(g_ctx.tessellation);
    radeon_6790_cp_emit(g_ctx.render_target_array_slice);
    radeon_6790_cp_emit(g_ctx.msaa_samples);
    radeon_6790_cp_flush();

    radeon_6790_delay_us(200);
}

void radeon_6790_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_6790_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 8U, g_ctx.geometry_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 12U, g_ctx.hull_shader);

    radeon_6790_cp_emit(0x80000000U | 0x11U);
    radeon_6790_cp_emit(index_count);
    radeon_6790_cp_emit(start_index);
    radeon_6790_cp_emit((uint32_t)base_vertex);
    radeon_6790_cp_emit(g_ctx.index_buffer);
    radeon_6790_cp_emit(g_ctx.primitive_topology);
    radeon_6790_cp_emit(g_ctx.blend_state);
    radeon_6790_cp_emit(g_ctx.rasterizer_state);
    radeon_6790_cp_emit(g_ctx.depth_stencil_state);
    radeon_6790_cp_emit(g_ctx.tessellation);
    radeon_6790_cp_emit(g_ctx.render_target_array_slice);
    radeon_6790_cp_emit(g_ctx.msaa_samples);
    radeon_6790_cp_flush();

    radeon_6790_delay_us(200);
}

void radeon_6790_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)
{
    if (!g_initialized) return;

    mmio_write32(REG_6790_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 8U, g_ctx.geometry_shader);
    mmio_write32(REG_6790_SQ_PROGRAM + 12U, g_ctx.hull_shader);

    radeon_6790_cp_emit(0x80000000U | 0x12U);
    radeon_6790_cp_emit(index_count);
    radeon_6790_cp_emit(start_index);
    radeon_6790_cp_emit((uint32_t)base_vertex);
    radeon_6790_cp_emit(instance_count);
    radeon_6790_cp_emit(start_instance);
    radeon_6790_cp_emit(g_ctx.index_buffer);
    radeon_6790_cp_emit(g_ctx.primitive_topology);
    radeon_6790_cp_emit(g_ctx.blend_state);
    radeon_6790_cp_emit(g_ctx.rasterizer_state);
    radeon_6790_cp_emit(g_ctx.depth_stencil_state);
    radeon_6790_cp_emit(g_ctx.tessellation);
    radeon_6790_cp_emit(g_ctx.render_target_array_slice);
    radeon_6790_cp_emit(g_ctx.msaa_samples);
    radeon_6790_cp_flush();

    radeon_6790_delay_us(200);
}

void radeon_6790_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)
{
    if (!g_initialized) return;

    radeon_6790_cp_emit(0x80000000U | 0x13U);
    radeon_6790_cp_emit(buffer_addr);
    radeon_6790_cp_emit(draw_count);
    radeon_6790_cp_emit(stride);
    radeon_6790_cp_flush();

    radeon_6790_delay_us(200);
}

void radeon_6790_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
{
    if (!g_initialized) return;

    mmio_write32(REG_6790_SQ_COMPUTE_PROGRAM, g_ctx.compute_shader);

    radeon_6790_cp_emit(0x80000000U | 0x20U);
    radeon_6790_cp_emit(group_x);
    radeon_6790_cp_emit(group_y);
    radeon_6790_cp_emit(group_z);
    radeon_6790_cp_flush();

    radeon_6790_delay_us(200);
}

void radeon_6790_compute_dispatch_indirect(uint32_t buffer_addr)
{
    if (!g_initialized) return;

    radeon_6790_cp_emit(0x80000000U | 0x21U);
    radeon_6790_cp_emit(buffer_addr);
    radeon_6790_cp_flush();

    radeon_6790_delay_us(200);
}

void radeon_6790_dma_transfer(uint64_t dst, uint64_t src, uint64_t size)
{
    if (!g_initialized) return;

    radeon_6790_dma_emit(0xC0000000U | 0x30U);
    radeon_6790_dma_emit((uint32_t)(dst & 0xFFFFFFFFULL));
    radeon_6790_dma_emit((uint32_t)((dst >> 32) & 0xFFFFFFFFULL));
    radeon_6790_dma_emit((uint32_t)(src & 0xFFFFFFFFULL));
    radeon_6790_dma_emit((uint32_t)((src >> 32) & 0xFFFFFFFFULL));
    radeon_6790_dma_emit((uint32_t)(size & 0xFFFFFFFFULL));
    radeon_6790_dma_emit((uint32_t)((size >> 32) & 0xFFFFFFFFULL));
    radeon_6790_dma_flush();
}

radeon_6790_handle_t radeon_6790_occlusion_query_create(void)
{
    for (uint32_t i = 0; i < RADEON_6790_MAX_OCCLUSION_QUERIES; i++) {
        if (!g_occlusion_used[i]) {
            g_occlusion_used[i] = 1;
            g_occlusion[i].used = 1;
            g_occlusion[i].active = 0;
            g_occlusion[i].samples = 0U;
            g_occlusion[i].result = 0U;
            g_occlusion[i].result_ready = 0;
            return (radeon_6790_handle_t)i;
        }
    }
    return RADEON_6790_HANDLE_NULL;
}

void radeon_6790_occlusion_query_destroy(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_OCCLUSION_QUERIES || !g_occlusion_used[handle]) return;
    g_occlusion[handle].used = 0;
    g_occlusion_used[handle] = 0;
}

void radeon_6790_occlusion_query_begin(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_OCCLUSION_QUERIES || !g_occlusion_used[handle]) return;
    g_occlusion[handle].active = 1;
    g_occlusion[handle].samples = 0U;
    g_occlusion[handle].result_ready = 0;
    mmio_write32(REG_6790_OCCLUSION_QUERY_CNTL, (uint32_t)handle | 0x80000000U);
}

void radeon_6790_occlusion_query_end(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_OCCLUSION_QUERIES || !g_occlusion_used[handle]) return;
    g_occlusion[handle].active = 0;
    mmio_write32(REG_6790_OCCLUSION_QUERY_CNTL, (uint32_t)handle & 0x7FFFFFFFU);
    g_occlusion[handle].result_ready = 1;
    g_occlusion[handle].result = g_occlusion[handle].samples;
}

int radeon_6790_occlusion_query_get(radeon_6790_handle_t handle, uint32_t *samples)
{
    if (handle >= RADEON_6790_MAX_OCCLUSION_QUERIES || !g_occlusion_used[handle] || !samples) return 0;
    if (!g_occlusion[handle].result_ready) return 0;
    *samples = g_occlusion[handle].result;
    return 1;
}

radeon_6790_handle_t radeon_6790_pipeline_stats_create(void)
{
    for (uint32_t i = 0; i < RADEON_6790_MAX_PIPELINE_STATS; i++) {
        if (!g_pstats_used[i]) {
            g_pstats_used[i] = 1;
            g_pstats[i].used = 1;
            g_pstats[i].active = 0;
            g_pstats[i].ia_vertices = 0ULL;
            g_pstats[i].ia_primitives = 0ULL;
            g_pstats[i].vs_invocations = 0ULL;
            g_pstats[i].gs_invocations = 0ULL;
            g_pstats[i].gs_primitives = 0ULL;
            g_pstats[i].c_invocations = 0ULL;
            g_pstats[i].c_primitives = 0ULL;
            g_pstats[i].ps_invocations = 0ULL;
            return (radeon_6790_handle_t)i;
        }
    }
    return RADEON_6790_HANDLE_NULL;
}

void radeon_6790_pipeline_stats_destroy(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_PIPELINE_STATS || !g_pstats_used[handle]) return;
    g_pstats[handle].used = 0;
    g_pstats_used[handle] = 0;
}

void radeon_6790_pipeline_stats_begin(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_PIPELINE_STATS || !g_pstats_used[handle]) return;
    g_pstats[handle].active = 1;
    mmio_write32(REG_6790_SQ_PERF_COUNTER_SEL_0, (uint32_t)handle | 0x80000000U);
}

void radeon_6790_pipeline_stats_end(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_PIPELINE_STATS || !g_pstats_used[handle]) return;
    g_pstats[handle].active = 0;
    mmio_write32(REG_6790_SQ_PERF_COUNTER_SEL_0, (uint32_t)handle & 0x7FFFFFFFU);
}

int radeon_6790_pipeline_stats_get(radeon_6790_handle_t handle, struct radeon_6790_pipeline_stats *out)
{
    if (handle >= RADEON_6790_MAX_PIPELINE_STATS || !g_pstats_used[handle] || !out) return 0;
    *out = g_pstats[handle];
    return 1;
}

void radeon_6790_fence_create(radeon_6790_handle_t *out)
{
    if (!out) return;
    for (uint32_t i = 0; i < RADEON_6790_MAX_FENCES; i++) {
        if (!g_fence_used[i]) {
            g_fence_used[i] = 1;
            g_fence[i].used = 1;
            g_fence[i].value = 0U;
            g_fence[i].signaled = 0;
            *out = (radeon_6790_handle_t)i;
            return;
        }
    }
    *out = RADEON_6790_HANDLE_NULL;
}

void radeon_6790_fence_signal(radeon_6790_handle_t handle, uint32_t value)
{
    if (handle >= RADEON_6790_MAX_FENCES || !g_fence_used[handle]) return;
    g_fence[handle].value = value;
    g_fence[handle].signaled = 1;
}

int radeon_6790_fence_wait(radeon_6790_handle_t handle, uint32_t value, uint32_t timeout_us)
{
    if (handle >= RADEON_6790_MAX_FENCES || !g_fence_used[handle]) return 0;
    for (uint32_t wait = 0; wait < timeout_us; wait++) {
        if (g_fence[handle].signaled && g_fence[handle].value >= value) return 1;
        radeon_6790_delay_us(1);
    }
    return 0;
}

void radeon_6790_fence_destroy(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_FENCES || !g_fence_used[handle]) return;
    g_fence[handle].used = 0;
    g_fence_used[handle] = 0;
}

void radeon_6790_semaphore_create(radeon_6790_handle_t *out)
{
    if (!out) return;
    for (uint32_t i = 0; i < RADEON_6790_MAX_SEMAPHORES; i++) {
        if (!g_semaphore_used[i]) {
            g_semaphore_used[i] = 1;
            g_semaphore[i].used = 1;
            g_semaphore[i].value = 0U;
            *out = (radeon_6790_handle_t)i;
            return;
        }
    }
    *out = RADEON_6790_HANDLE_NULL;
}

void radeon_6790_semaphore_signal(radeon_6790_handle_t handle, uint32_t value)
{
    if (handle >= RADEON_6790_MAX_SEMAPHORES || !g_semaphore_used[handle]) return;
    g_semaphore[handle].value = value;
}

void radeon_6790_semaphore_wait(radeon_6790_handle_t handle, uint32_t value)
{
    if (handle >= RADEON_6790_MAX_SEMAPHORES || !g_semaphore_used[handle]) return;
    for (volatile uint32_t wait = 0; wait < 1000000U; wait++) {
        if (g_semaphore[handle].value >= value) break;
    }
}

void radeon_6790_semaphore_destroy(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_SEMAPHORES || !g_semaphore_used[handle]) return;
    g_semaphore[handle].used = 0;
    g_semaphore_used[handle] = 0;
}

int radeon_6790_wait_idle(uint32_t timeout_us)
{
    for (uint32_t wait = 0; wait < timeout_us; wait++) {
        uint32_t wptr = mmio_read32(REG_6790_CP_ME_RB_WPTR);
        uint32_t rptr = mmio_read32(REG_6790_CP_ME_RB_RPTR);
        if (wptr == rptr) return 1;
        radeon_6790_delay_us(1);
    }
    return 0;
}

uint64_t radeon_6790_timestamp(void)
{
    uint32_t lo = mmio_read32(REG_6790_TIMESTAMP_LO);
    uint32_t hi = mmio_read32(REG_6790_TIMESTAMP_HI);
    return ((uint64_t)hi << 32) | lo;
}

void radeon_6790_power_set_state(uint8_t state)
{
    g_power_state = state;
    mmio_write32(REG_6790_POWER_STATE, state);
    radeon_6790_delay_us(100);
}

uint8_t radeon_6790_power_get_state(void)
{
    return g_power_state;
}

void radeon_6790_set_clock_gfx(uint32_t mhz)
{
    g_clock_gfx = mhz;
    mmio_write32(REG_6790_GFX_CLOCK_CNTL, mhz);
}

void radeon_6790_set_clock_memory(uint32_t mhz)
{
    g_clock_memory = mhz;
    mmio_write32(REG_6790_MEMORY_CLOCK_CNTL, mhz);
}

uint32_t radeon_6790_get_clock_gfx(void)
{
    return g_clock_gfx;
}

uint32_t radeon_6790_get_clock_memory(void)
{
    return g_clock_memory;
}

uint32_t radeon_6790_get_temperature(void)
{
    uint32_t temp = mmio_read32(REG_6790_THERMAL_TEMP);
    if (temp != 0xFFFFFFFFU) g_temperature = temp & 0xFFU;
    return g_temperature;
}

void radeon_6790_set_fan_speed(uint32_t rpm)
{
    g_fan_rpm = rpm;
    mmio_write32(REG_6790_FAN_PWM, rpm);
    mmio_write32(REG_6790_FAN_RPM, rpm);
}

uint32_t radeon_6790_get_fan_speed(void)
{
    uint32_t rpm = mmio_read32(REG_6790_FAN_RPM);
    if (rpm != 0xFFFFFFFFU) g_fan_rpm = rpm;
    return g_fan_rpm;
}

void radeon_6790_rgb_set(uint32_t color, uint8_t mode)
{
    mmio_write32(REG_6790_RGB_CONTROL, (uint32_t)mode);
    mmio_write32(REG_6790_RGB_COLOR, color);
}

void radeon_6790_rgb_off(void)
{
    mmio_write32(REG_6790_RGB_CONTROL, 0U);
    mmio_write32(REG_6790_RGB_COLOR, 0U);
}

radeon_6790_handle_t radeon_6790_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied)
{
    for (uint32_t i = 0; i < RADEON_6790_MAX_SWAP_CHAINS; i++) {
        if (!g_swap_used[i]) {
            g_swap_used[i] = 1;
            g_swap[i].used = 1;
            g_swap[i].width = width;
            g_swap[i].height = height;
            g_swap[i].back_buffer_addr = RADEON_6790_FB_ADDR + 0x10000000U + i * 0x10000000U;
            g_swap[i].front_buffer_addr = RADEON_6790_FB_ADDR;
            g_swap[i].vsync = vsync;
            g_swap[i].alpha_premultiplied = alpha_premultiplied;
            g_swap[i].back_vram_offset = radeon_6790_vram_alloc((uint64_t)width * height * 4ULL, 7U);
            g_swap[i].front_vram_offset = 0ULL;
            return (radeon_6790_handle_t)i;
        }
    }
    return RADEON_6790_HANDLE_NULL;
}

void radeon_6790_swap_chain_destroy(radeon_6790_handle_t handle)
{
    if (handle >= RADEON_6790_MAX_SWAP_CHAINS || !g_swap_used[handle]) return;
    radeon_6790_vram_free(g_swap[handle].back_vram_offset);
    g_swap[handle].used = 0;
    g_swap_used[handle] = 0;
}

void radeon_6790_present(radeon_6790_handle_t chain)
{
    if (chain >= RADEON_6790_MAX_SWAP_CHAINS || !g_swap_used[chain]) return;

    if (g_swap[chain].vsync) {
        for (volatile uint32_t wait = 0; wait < 4000000U; wait++) {
            if (mmio_read32(REG_6790_DCN_CRTC_CONTROL) & 0x80000000U) break;
        }
    }

    radeon_6790_cp_emit(0x40000000U | 0x02U);
    radeon_6790_cp_emit((0U << 16) | 0U);
    radeon_6790_cp_emit(((RADEON_6790_FB_HEIGHT - 1U) << 16) | (RADEON_6790_FB_WIDTH - 1U));
    radeon_6790_cp_emit((0U << 16) | 0U);
    radeon_6790_cp_emit(0U);
    radeon_6790_cp_flush();
}

void radeon_6790_command_begin(void)
{
    g_cmd_recording = 1;
    g_cmd_count = 0;
}

void radeon_6790_command_end(void)
{
    if (!g_cmd_recording) return;
    g_cmd_recording = 0;
    for (uint32_t i = 0; i < g_cmd_count; i++) {
        radeon_6790_cp_emit(g_cmd_buf[i]);
    }
    radeon_6790_cp_flush();
    g_cmd_count = 0;
}

void radeon_6790_context_get(struct radeon_6790_context *out)
{
    if (!out) return;
    *out = g_ctx;
}

int radeon_6790_3d_available(void)
{
    return g_initialized ? 1 : 0;
}
