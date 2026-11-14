/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/karty_graficzne/radeon_rx_9070_xt.h>

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
static volatile uint32_t *g_fb = (volatile uint32_t *)RX_9070_XT_FB_ADDR;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint16_t g_device_id = 0;
static uint8_t g_revision = 0;
static int g_initialized = 0;
static uint32_t g_ring_gfx[RX_9070_XT_RING_BUFFER_SIZE / 4U];
static uint32_t g_ring_gfx_head = 0;
static uint32_t g_ring_gfx_tail = 0;
static uint32_t g_ring_comp[RX_9070_XT_RING_BUFFER_SIZE / 4U];
static uint32_t g_ring_comp_head = 0;
static uint32_t g_ring_comp_tail = 0;
static uint32_t g_ring_dma[RX_9070_XT_RING_BUFFER_SIZE / 4U];
static uint32_t g_ring_dma_head = 0;
static uint32_t g_ring_dma_tail = 0;
static int g_cursor_visible = 0;
static uint8_t g_architecture = 4U;
static uint64_t g_vram_used = 0ULL;
static uint64_t g_vram_total = RX_9070_XT_VRAM_SIZE_BYTES;
static uint32_t g_clock_gfx = 3100U;
static uint32_t g_clock_memory = 2250U;
static uint32_t g_temperature = 45U;
static uint32_t g_fan_rpm = 1200U;
static uint8_t g_power_state = POWER_STATE_D0;
static uint8_t g_alpha_blend_global = 1U;
static uint8_t g_ray_tracing = 0U;
static uint8_t g_mesh_shader = 0U;
static uint8_t g_variable_rate_shading = 0U;
static uint8_t g_delta_color_compression = 1U;
static uint8_t g_hdr = 0U;
static uint8_t g_freesync = 0U;

static struct rx_9070_xt_vram_alloc g_vram_allocs[RX_9070_XT_VRAM_MAX_ALLOCS];
static uint32_t g_vram_alloc_count = 0U;

static struct rx_9070_xt_vertex_buffer g_vb[RX_9070_XT_MAX_VERTEX_BUFFERS];
static struct rx_9070_xt_index_buffer g_ib[RX_9070_XT_MAX_INDEX_BUFFERS];
static struct rx_9070_xt_texture g_tex[RX_9070_XT_MAX_TEXTURES];
static struct rx_9070_xt_render_target g_rt[RX_9070_XT_MAX_RENDER_TARGETS];
static struct rx_9070_xt_depth_stencil g_ds[RX_9070_XT_MAX_DEPTH_STENCILS];
static struct rx_9070_xt_shader g_shader[RX_9070_XT_MAX_SHADERS];
static struct rx_9070_xt_sampler_state g_sampler[RX_9070_XT_MAX_SAMPLERS];
static struct rx_9070_xt_blend_state g_blend[RX_9070_XT_MAX_BLEND_STATES];
static struct rx_9070_xt_rasterizer_state g_raster[RX_9070_XT_MAX_RASTERIZER_STATES];
static struct rx_9070_xt_depth_stencil_state g_ds_state[RX_9070_XT_MAX_DEPTH_STENCIL_STATES];
static struct rx_9070_xt_input_layout g_il[RX_9070_XT_MAX_INPUT_LAYOUTS];
static struct rx_9070_xt_constant_buffer g_cb[RX_9070_XT_MAX_CONSTANT_BUFFERS];
static struct rx_9070_xt_swap_chain g_swap[RX_9070_XT_MAX_SWAP_CHAINS];
static struct rx_9070_xt_fence g_fence[RX_9070_XT_MAX_FENCES];
static struct rx_9070_xt_semaphore g_semaphore[RX_9070_XT_MAX_SEMAPHORES];

static uint8_t g_vb_used[RX_9070_XT_MAX_VERTEX_BUFFERS] = {0};
static uint8_t g_ib_used[RX_9070_XT_MAX_INDEX_BUFFERS] = {0};
static uint8_t g_tex_used[RX_9070_XT_MAX_TEXTURES] = {0};
static uint8_t g_rt_used[RX_9070_XT_MAX_RENDER_TARGETS] = {0};
static uint8_t g_ds_used[RX_9070_XT_MAX_DEPTH_STENCILS] = {0};
static uint8_t g_shader_used[RX_9070_XT_MAX_SHADERS] = {0};
static uint8_t g_sampler_used[RX_9070_XT_MAX_SAMPLERS] = {0};
static uint8_t g_blend_used[RX_9070_XT_MAX_BLEND_STATES] = {0};
static uint8_t g_raster_used[RX_9070_XT_MAX_RASTERIZER_STATES] = {0};
static uint8_t g_ds_state_used[RX_9070_XT_MAX_DEPTH_STENCIL_STATES] = {0};
static uint8_t g_il_used[RX_9070_XT_MAX_INPUT_LAYOUTS] = {0};
static uint8_t g_cb_used[RX_9070_XT_MAX_CONSTANT_BUFFERS] = {0};
static uint8_t g_swap_used[RX_9070_XT_MAX_SWAP_CHAINS] = {0};
static uint8_t g_fence_used[RX_9070_XT_MAX_FENCES] = {0};
static uint8_t g_semaphore_used[RX_9070_XT_MAX_SEMAPHORES] = {0};

static uint8_t g_cmd_recording = 0;
static uint32_t g_cmd_buf[RX_9070_XT_COMMAND_BUFFER_SIZE];
static uint32_t g_cmd_count = 0;

static struct rx_9070_xt_context g_ctx = {};

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

static void rx_9070_xt_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

static int rx_9070_xt_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != RX_9070_XT_VENDOR_ID) continue;
                if (device != RX_9070_XT_DEVICE_ID_NAVI48_XT &&
                    device != RX_9070_XT_DEVICE_ID_NAVI48_XT_VAR1 &&
                    device != RX_9070_XT_DEVICE_ID_NAVI48_XT_VAR2 &&
                    device != RX_9070_XT_DEVICE_ID_NAVI48_XT_VAR3 &&
                    device != RX_9070_XT_DEVICE_ID_NAVI48_XT_VAR4) continue;

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

static void rx_9070_xt_clear_interrupts(void)
{
    mmio_write32(REG_INT_STAT, 0xFFFFFFFFU);
    mmio_write32(REG_INT_MASK, 0x00000000U);
    mmio_write32(REG_INT_ACK, 0xFFFFFFFFU);
}

static void rx_9070_xt_reset_cp_gfx(void)
{
    mmio_write32(REG_CP_GFX_ME_CNTL, CP_ME_CNTL_ME_RESET | CP_ME_CNTL_PFP_RESET);
    rx_9070_xt_delay_us(100);
    mmio_write32(REG_CP_GFX_ME_CNTL, 0U);
    rx_9070_xt_delay_us(100);
}

static void rx_9070_xt_reset_cp_compute(void)
{
    mmio_write32(REG_CP_COMP_ME_CNTL, CP_ME_CNTL_ME_RESET | CP_ME_CNTL_PFP_RESET);
    rx_9070_xt_delay_us(100);
    mmio_write32(REG_CP_COMP_ME_CNTL, 0U);
    rx_9070_xt_delay_us(100);
}

static void rx_9070_xt_reset_cp_dma(void)
{
    mmio_write32(REG_CP_DMA_ME_CNTL, CP_ME_CNTL_ME_RESET | CP_ME_CNTL_PFP_RESET);
    rx_9070_xt_delay_us(100);
    mmio_write32(REG_CP_DMA_ME_CNTL, 0U);
    rx_9070_xt_delay_us(100);
}

static void rx_9070_xt_cp_gfx_ring_init(void)
{
    mmio_write32(REG_CP_GFX_ME_RB_ADDR, (uint32_t)&g_ring_gfx[0]);
    mmio_write32(REG_CP_GFX_ME_RB_SIZE, sizeof(g_ring_gfx));
    mmio_write32(REG_CP_GFX_ME_RB_WPTR, 0U);
    mmio_write32(REG_CP_GFX_ME_RB_RPTR, 0U);
    mmio_write32(REG_CP_GFX_ME_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_gfx_head = 0;
    g_ring_gfx_tail = 0;
}

static void rx_9070_xt_cp_gfx_emit(uint32_t dword);
static void rx_9070_xt_cp_gfx_flush(void);

static void rx_9070_xt_cp_gfx_emit(uint32_t dword)
{
    if (((g_ring_gfx_head & (RX_9070_XT_RING_BUFFER_SIZE / 4U - 1U)) + 5U) > (RX_9070_XT_RING_BUFFER_SIZE / 4U)) {
        rx_9070_xt_cp_gfx_flush();
    }
    g_ring_gfx[g_ring_gfx_head & (RX_9070_XT_RING_BUFFER_SIZE / 4U - 1U)] = dword;
    g_ring_gfx_head++;
}

static void rx_9070_xt_cp_gfx_flush(void)
{
    uint32_t tail = g_ring_gfx_head & (RX_9070_XT_RING_BUFFER_SIZE / 4U - 1U);
    mmio_write32(REG_CP_GFX_ME_RB_WPTR, tail);
    for (volatile uint32_t wait = 0; wait < 2000000U; wait++) {
        if (mmio_read32(REG_CP_GFX_ME_RB_RPTR) == tail) {
            break;
        }
    }
    g_ring_gfx_tail = tail;
}

static void rx_9070_xt_cp_comp_ring_init(void)
{
    mmio_write32(REG_CP_COMP_ME_RB_ADDR, (uint32_t)&g_ring_comp[0]);
    mmio_write32(REG_CP_COMP_ME_RB_SIZE, sizeof(g_ring_comp));
    mmio_write32(REG_CP_COMP_ME_RB_WPTR, 0U);
    mmio_write32(REG_CP_COMP_ME_RB_RPTR, 0U);
    mmio_write32(REG_CP_COMP_ME_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_comp_head = 0;
    g_ring_comp_tail = 0;
}

static void rx_9070_xt_cp_dma_ring_init(void)
{
    mmio_write32(REG_CP_DMA_ME_RB_ADDR, (uint32_t)&g_ring_dma[0]);
    mmio_write32(REG_CP_DMA_ME_RB_SIZE, sizeof(g_ring_dma));
    mmio_write32(REG_CP_DMA_ME_RB_WPTR, 0U);
    mmio_write32(REG_CP_DMA_ME_RB_RPTR, 0U);
    mmio_write32(REG_CP_DMA_ME_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_dma_head = 0;
    g_ring_dma_tail = 0;
}

static void rx_9070_xt_display_init(void)
{
    mmio_write32(REG_DCN_DISP_CONTROL, 0xFFFFFFFFU);

    uint32_t hblank = (((RX_9070_XT_FB_WIDTH + 128U) - 1U) << 16) | (RX_9070_XT_FB_WIDTH - 1U);
    uint32_t vblank = (((RX_9070_XT_FB_HEIGHT + 32U) - 1U) << 16) | (RX_9070_XT_FB_HEIGHT - 1U);
    uint32_t hsync = (((RX_9070_XT_FB_WIDTH + 64U) - 1U) << 16) | (RX_9070_XT_FB_WIDTH - 1U);
    uint32_t vsync = (((RX_9070_XT_FB_HEIGHT + 4U) - 1U) << 16) | (RX_9070_XT_FB_HEIGHT - 1U);

    mmio_write32(REG_DCN_CRTC_H_TOTAL, hblank);
    mmio_write32(REG_DCN_CRTC_V_TOTAL, vblank);
    mmio_write32(REG_DCN_CRTC_H_BLANK, hblank);
    mmio_write32(REG_DCN_CRTC_V_BLANK, vblank);
    mmio_write32(REG_DCN_CRTC_H_SYNC_A, hsync);
    mmio_write32(REG_DCN_CRTC_V_SYNC_A, vsync);

    mmio_write32(REG_DCN_CRTC_PRIMARY_VIEWPORT_START, 0U);
    mmio_write32(REG_DCN_CRTC_PRIMARY_VIEWPORT_END,
                 ((RX_9070_XT_FB_HEIGHT - 1U) << 16) | (RX_9070_XT_FB_WIDTH - 1U));

    mmio_write32(REG_DCN_CRTC_PRIMARY_SURFACE_ADDRESS, RX_9070_XT_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_DCN_CRTC_PRIMARY_SURFACE_ADDRESS_HIGH, 0U);
    mmio_write32(REG_DCN_CRTC_PRIMARY_SURFACE_OFFSET_X, 0U);
    mmio_write32(REG_DCN_CRTC_PRIMARY_SURFACE_OFFSET_Y, 0U);
    mmio_write32(REG_DCN_CRTC_PRIMARY_SURFACE_PIXEL_PITCH, RX_9070_XT_FB_PITCH);
    mmio_write32(REG_DCN_CRTC_PRIMARY_SURFACE_TILE, 0U);

    mmio_write32(REG_DCN_PLANE_OFFSET_X, 0U);
    mmio_write32(REG_DCN_PLANE_OFFSET_Y, 0U);
    mmio_write32(REG_DCN_PLANE_PIXEL_PITCH, RX_9070_XT_FB_PITCH);
    mmio_write32(REG_DCN_PLANE_SURFACE_ADDRESS, RX_9070_XT_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_DCN_PLANE_SURFACE_ADDRESS_HIGH, 0U);

    uint32_t crtc_ctl = CRTC_CONTROL_EN | CRTC_CONTROL_PIXEL_WIDTH_32BPP;
    if (g_freesync) crtc_ctl |= CRTC_CONTROL_FREESYNC;
    mmio_write32(REG_DCN_CRTC_CONTROL, crtc_ctl);
    rx_9070_xt_delay_us(200);
}

static void rx_9070_xt_3d_pipeline_init(void)
{
    rx_9070_xt_reset_cp_gfx();
    rx_9070_xt_reset_cp_compute();
    rx_9070_xt_reset_cp_dma();
    rx_9070_xt_cp_gfx_ring_init();
    rx_9070_xt_cp_comp_ring_init();
    rx_9070_xt_cp_dma_ring_init();

    mmio_write32(REG_CB_TARGET0_BASE, RX_9070_XT_FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_CB_TARGET0_PITCH, RX_9070_XT_FB_PITCH);
    mmio_write32(REG_CB_TARGET0_SLICE, 0U);
    mmio_write32(REG_CB_TARGET0_FMASK, 0U);
    mmio_write32(REG_CB_TARGET0_VIEW, 0U);

    mmio_write32(REG_CB_SHADER_MASK, SHADER_MASK_ENABLE | SHADER_MASK_CB0_ENABLE);

    mmio_write32(REG_CB_CLEAR_COLOR, 0x00000000U);
    mmio_write32(REG_CB_CLEAR_COLOR_HIGH, 0x00000000U);

    uint32_t blend = (RX_9070_XT_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_SHIFT) |
                     (RX_9070_XT_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_SHIFT) |
                     (RX_9070_XT_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_SHIFT) |
                     (RX_9070_XT_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT) |
                     (RX_9070_XT_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT) |
                     (RX_9070_XT_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT);
    mmio_write32(REG_CB_BLEND_CONTROL, blend);

    mmio_write32(REG_DB_Z_INFO, DB_Z_INFO_FORMAT_24BIT | DB_Z_INFO_COMPRESSION_ENABLE);
    mmio_write32(REG_DB_Z_READ_BASE, (RX_9070_XT_FB_ADDR + 0x20000000ULL) & 0xFFFFFFFFU);
    mmio_write32(REG_DB_Z_READ_BASE_HIGH, 0U);
    mmio_write32(REG_DB_Z_WRITE_BASE, (RX_9070_XT_FB_ADDR + 0x20000000ULL) & 0xFFFFFFFFU);
    mmio_write32(REG_DB_Z_WRITE_BASE_HIGH, 0U);
    mmio_write32(REG_DB_Z_READ_OFF, 0U);
    mmio_write32(REG_DB_Z_WRITE_OFF, 0U);
    mmio_write32(REG_DB_CLEAR, 0x00FFFFFFU);
    mmio_write32(REG_DB_STENCIL_CLEAR, 0U);

    if (g_delta_color_compression) {
        mmio_write32(REG_CB_DCC_CONTROL, DCC_ENABLE);
    }

    mmio_write32(REG_SQ_CONFIG, 0U);
    mmio_write32(REG_SQ_VTX_BASE, 0U);
    mmio_write32(REG_SQ_VTX_STRIDE, 0U);
    mmio_write32(REG_SQ_VTX_NUM_FORMAT, 0U);
    mmio_write32(REG_SQ_VTX_FMT_0, 0U);
    mmio_write32(REG_SQ_VTX_FMT_1, 0U);
    mmio_write32(REG_SQ_VTX_FMT_2, 0U);
    mmio_write32(REG_SQ_VTX_FMT_3, 0U);

    mmio_write32(REG_SQ_COMPUTE_PROGRAM, 0U);
    mmio_write32(REG_SQ_RAY_PROGRAM, 0U);
    mmio_write32(REG_SQ_MESH_PROGRAM, 0U);

    mmio_write32(REG_SQ_TEX_BASE, 0U);
    mmio_write32(REG_SQ_SAMPLER_BASE, 0U);
    mmio_write32(REG_SQ_TEX_LOD, 0U);
    mmio_write32(REG_SQ_TEX_DIM, 0U);

    mmio_write32(REG_PA_SU_VTX_CNTL, 0U);
    mmio_write32(REG_PA_CL_VTE_CNTL, 0U);
    mmio_write32(REG_PA_CL_VS_OUT_CNTL, 0U);
    mmio_write32(REG_PA_CL_GB_ADV, 0U);
    mmio_write32(REG_PA_CL_CLIP_CNTL, 0U);
    mmio_write32(REG_PA_SU_SC_MODE_CNTL, 0U);
    mmio_write32(REG_PA_SC_WINDOW_OFFSET, 0U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_TL, 0U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_BR,
                 ((RX_9070_XT_FB_HEIGHT - 1U) << 16) | (RX_9070_XT_FB_WIDTH - 1U));
    mmio_write32(REG_PA_SU_PRIM_FILTER_CNTL, 0U);
    mmio_write32(REG_PA_SU_LINE_STIPPLE, 0U);
    mmio_write32(REG_PA_SC_AA_CONFIG, 0U);

    if (g_mesh_shader) {
        mmio_write32(REG_PA_MESH_CNTL, MESH_ENABLE);
    }

    mmio_write32(REG_CB_ALPHA_TO_MASK, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD0, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD1, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD2, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD3, 0U);

    mmio_write32(REG_RT_ACCEL_STRUCT, 0U);
    mmio_write32(REG_RT_RAY_GEN, 0U);
    mmio_write32(REG_RT_ANY_HIT, 0U);
    mmio_write32(REG_RT_CLOSEST_HIT, 0U);
    mmio_write32(REG_RT_MISS, 0U);
    mmio_write32(REG_RT_CALLABLE, 0U);
    mmio_write32(REG_RT_INTERSECTION, 0U);

    mmio_write32(REG_VRS_SHADING_RATE, 0U);

    mmio_write32(REG_VM_CONTEXT, 0U);
    mmio_write32(REG_VM_PAGE_TABLE, 0U);

    rx_9070_xt_delay_us(200);
}

static void rx_9070_xt_cursor_init(void)
{
    mmio_write32(REG_CURSOR_CONTROL, 0U);
    mmio_write32(REG_CURSOR_POSITION, 0U);
    mmio_write32(REG_CURSOR_COLOR0, 0x00000000U);
    mmio_write32(REG_CURSOR_COLOR1, 0x00FFFFFFU);
    mmio_write32(REG_CURSOR_BASE, RX_9070_XT_FB_ADDR);
    g_cursor_visible = 0;
}

static void rx_9070_xt_power_init(void)
{
    mmio_write32(REG_POWER_STATE, POWER_STATE_D0);
    mmio_write32(REG_GFX_CLOCK_CNTL, g_clock_gfx);
    mmio_write32(REG_MEMORY_CLOCK_CNTL, g_clock_memory);
    mmio_write32(REG_DISPLAY_CLOCK_CNTL, g_clock_gfx / 2U);
    rx_9070_xt_delay_us(100);
}

int rx_9070_xt_init(void)
{
    if (g_initialized) return 1;
    if (!rx_9070_xt_find_device()) return 0;

    uint32_t command = pci_read32(g_bus, g_slot, g_func, 0x04);
    pci_write32(g_bus, g_slot, g_func, 0x04, command | 0x00000006U);

    rx_9070_xt_clear_interrupts();

    rx_9070_xt_power_init();
    rx_9070_xt_display_init();
    rx_9070_xt_3d_pipeline_init();
    rx_9070_xt_cursor_init();

    g_initialized = 1;
    return 1;
}

int rx_9070_xt_is_ready(void)
{
    return g_initialized;
}

void rx_9070_xt_shutdown(void)
{
    if (!g_initialized) return;
    mmio_write32(REG_CP_GFX_ME_CNTL, CP_ME_CNTL_ME_HALT | CP_ME_CNTL_PFP_HALT);
    mmio_write32(REG_CP_COMP_ME_CNTL, CP_ME_CNTL_ME_HALT | CP_ME_CNTL_PFP_HALT);
    mmio_write32(REG_CP_DMA_ME_CNTL, CP_ME_CNTL_ME_HALT | CP_ME_CNTL_PFP_HALT);
    mmio_write32(REG_POWER_STATE, POWER_STATE_D3);
    g_initialized = 0;
}

void rx_9070_xt_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= RX_9070_XT_FB_WIDTH || y >= RX_9070_XT_FB_HEIGHT) return;
    g_fb[y * (RX_9070_XT_FB_PITCH / 4U) + x] = color;
}

void rx_9070_xt_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (g_initialized) {
        rx_9070_xt_cp_gfx_emit(0x40000000U | 0x01U);
        rx_9070_xt_cp_gfx_emit((y << 16) | x);
        rx_9070_xt_cp_gfx_emit(((h - 1U) << 16) | (w - 1U));
        rx_9070_xt_cp_gfx_emit(color);
        rx_9070_xt_cp_gfx_emit(0U);
        rx_9070_xt_cp_gfx_flush();
        return;
    }
    uint32_t xx;
    uint32_t yy;
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= RX_9070_XT_FB_WIDTH || yy >= RX_9070_XT_FB_HEIGHT) continue;
            g_fb[yy * (RX_9070_XT_FB_PITCH / 4U) + xx] = color;
        }
    }
}

void rx_9070_xt_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                    uint32_t w, uint32_t h)
{
    if (!g_initialized) return;

    rx_9070_xt_cp_gfx_emit(0x40000000U | 0x02U);
    rx_9070_xt_cp_gfx_emit((dst_y << 16) | dst_x);
    rx_9070_xt_cp_gfx_emit(((h - 1U) << 16) | (w - 1U));
    rx_9070_xt_cp_gfx_emit((src_y << 16) | src_x);
    rx_9070_xt_cp_gfx_emit(0U);
    rx_9070_xt_cp_gfx_flush();
}

void rx_9070_xt_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    if (!g_initialized) return;

    if (image) {
        for (uint32_t i = 0; i < 64U * 64U; i++) {
            g_fb[i] = image[i];
        }
        mmio_write32(REG_CURSOR_BASE, RX_9070_XT_FB_ADDR);
    }

    mmio_write32(REG_CURSOR_POSITION, (y << 16) | x);
    mmio_write32(REG_CURSOR_CONTROL, 0x80000000U);
    g_cursor_visible = 1;
}

void rx_9070_xt_cursor_hide(void)
{
    if (!g_initialized) return;
    mmio_write32(REG_CURSOR_CONTROL, 0U);
    g_cursor_visible = 0;
}

void *rx_9070_xt_get_fb(void)
{
    return (void *)g_fb;
}

uint32_t rx_9070_xt_get_width(void)
{
    return RX_9070_XT_FB_WIDTH;
}

uint32_t rx_9070_xt_get_height(void)
{
    return RX_9070_XT_FB_HEIGHT;
}

uint16_t rx_9070_xt_get_device_id(void)
{
    return g_device_id;
}

uint8_t rx_9070_xt_get_revision(void)
{
    return g_revision;
}

void rx_9070_xt_info_get(struct rx_9070_xt_info *out)
{
    if (!out) return;
    out->vendor_id = RX_9070_XT_VENDOR_ID;
    out->device_id = g_device_id;
    out->revision_id = g_revision;
    out->mmio_base = g_mmio_base;
    out->fb_addr = RX_9070_XT_FB_ADDR;
    out->fb_width = RX_9070_XT_FB_WIDTH;
    out->fb_height = RX_9070_XT_FB_HEIGHT;
    out->fb_pitch = RX_9070_XT_FB_PITCH;
    out->initialized = (uint8_t)g_initialized;
    out->architecture = g_architecture;
    out->vram_size = g_vram_total;
    out->vram_used = g_vram_used;
    out->gfx_clock_mhz = g_clock_gfx;
    out->memory_clock_mhz = g_clock_memory;
    out->temperature_c = g_temperature;
    out->fan_rpm = g_fan_rpm;
}

uint64_t rx_9070_xt_vram_alloc(uint64_t size, uint32_t type)
{
    (void)type;
    if (size == 0) return 0xFFFFFFFFFFFFFFFFULL;
    if (g_vram_alloc_count >= RX_9070_XT_VRAM_MAX_ALLOCS) return 0xFFFFFFFFFFFFFFFFULL;
    uint64_t align = RX_9070_XT_VRAM_PAGE_SIZE;
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

void rx_9070_xt_vram_free(uint64_t handle)
{
    for (uint32_t i = 0; i < g_vram_alloc_count; i++) {
        if (g_vram_allocs[i].used && g_vram_allocs[i].offset == handle) {
            g_vram_allocs[i].used = 0;
            return;
        }
    }
}

uint64_t rx_9070_xt_vram_used(void)
{
    return g_vram_used;
}

uint64_t rx_9070_xt_vram_total(void)
{
    return g_vram_total;
}

static uint32_t find_free_slot(uint8_t *used, uint32_t max)
{
    for (uint32_t i = 0; i < max; i++) {
        if (!used[i]) return i;
    }
    return RX_9070_XT_HANDLE_NULL;
}

static int handle_valid(rx_9070_xt_handle_t h, uint8_t *used, uint32_t max)
{
    return h != RX_9070_XT_HANDLE_NULL && h < max && used[h];
}

rx_9070_xt_handle_t rx_9070_xt_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_vb_used, RX_9070_XT_MAX_VERTEX_BUFFERS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_vertex_buffer *vb = &g_vb[i];
    vb->used = 1;
    vb->vertex_count = vertex_count;
    vb->vertex_size = vertex_size;
    vb->format = format;
    vb->buffer_addr = RX_9070_XT_FB_ADDR + 0x40000000U + i * 0x200000U;
    vb->system_mem = 0;
    vb->vram_offset = rx_9070_xt_vram_alloc((uint64_t)vertex_count * vertex_size, 1U);

    if (data) {
        vb->system_mem = (uint8_t *)vb->buffer_addr;
        for (uint32_t k = 0; k < vertex_count * vertex_size; k++) {
            vb->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_vertex_buffer_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_vb_used, RX_9070_XT_MAX_VERTEX_BUFFERS)) return;
    rx_9070_xt_vram_free(g_vb[handle].vram_offset);
    g_vb[handle].used = 0;
}

void rx_9070_xt_vertex_buffer_update(rx_9070_xt_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_vb_used, RX_9070_XT_MAX_VERTEX_BUFFERS)) return;
    struct rx_9070_xt_vertex_buffer *vb = &g_vb[handle];
    if (!vb->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        vb->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

rx_9070_xt_handle_t rx_9070_xt_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_ib_used, RX_9070_XT_MAX_INDEX_BUFFERS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_index_buffer *ib = &g_ib[i];
    ib->used = 1;
    ib->index_count = index_count;
    ib->format = format;
    ib->buffer_addr = RX_9070_XT_FB_ADDR + 0x60000000U + i * 0x200000U;
    ib->system_mem = 0;
    ib->vram_offset = rx_9070_xt_vram_alloc((uint64_t)index_count * 4U, 2U);

    if (data) {
        ib->system_mem = (uint8_t *)ib->buffer_addr;
        uint32_t elem_size = (format == RX_9070_XT_INDEX_FORMAT_32BIT) ? 4U : 2U;
        for (uint32_t k = 0; k < index_count * elem_size; k++) {
            ib->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_index_buffer_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_ib_used, RX_9070_XT_MAX_INDEX_BUFFERS)) return;
    rx_9070_xt_vram_free(g_ib[handle].vram_offset);
    g_ib[handle].used = 0;
}

void rx_9070_xt_index_buffer_update(rx_9070_xt_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_ib_used, RX_9070_XT_MAX_INDEX_BUFFERS)) return;
    struct rx_9070_xt_index_buffer *ib = &g_ib[handle];
    if (!ib->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        ib->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

rx_9070_xt_handle_t rx_9070_xt_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
{
    uint32_t i = find_free_slot(g_tex_used, RX_9070_XT_MAX_TEXTURES);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_texture *t = &g_tex[i];
    t->used = 1;
    t->width = width;
    t->height = height;
    t->format = format;
    t->mip_levels = mip_levels;
    t->pitch = ((width * 4U + 3U) & ~3U);
    t->buffer_addr = RX_9070_XT_FB_ADDR + 0x80000000U + i * 0x400000U;
    t->system_mem = 0;
    t->vram_offset = rx_9070_xt_vram_alloc((uint64_t)width * height * 4ULL * mip_levels, 3U);
    t->compressed = 0;

    if (data) {
        t->system_mem = (uint8_t *)t->buffer_addr;
        uint32_t bpp = 4U;
        for (uint32_t k = 0; k < width * height * bpp; k++) {
            t->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_texture_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, RX_9070_XT_MAX_TEXTURES)) return;
    rx_9070_xt_vram_free(g_tex[handle].vram_offset);
    g_tex[handle].used = 0;
}

void rx_9070_xt_texture_update(rx_9070_xt_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
{
    (void)level;
    if (!handle_valid(handle, g_tex_used, RX_9070_XT_MAX_TEXTURES)) return;
    struct rx_9070_xt_texture *t = &g_tex[handle];
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

void rx_9070_xt_texture_generate_mipmaps(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, RX_9070_XT_MAX_TEXTURES)) return;
    struct rx_9070_xt_texture *t = &g_tex[handle];
    if (!t->system_mem) return;
    uint32_t w = t->width;
    uint32_t h = t->height;
    for (uint32_t level = 1; level < t->mip_levels && w > 1 && h > 1; level++) {
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
    }
}

rx_9070_xt_handle_t rx_9070_xt_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend, uint8_t dcc)
{
    uint32_t i = find_free_slot(g_rt_used, RX_9070_XT_MAX_RENDER_TARGETS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_render_target *rt = &g_rt[i];
    rt->used = 1;
    rt->width = width;
    rt->height = height;
    rt->format = format;
    rt->buffer_addr = RX_9070_XT_FB_ADDR + 0xC0000000ULL + (uint64_t)i * 0x10000000ULL;
    rt->alpha_blend = alpha_blend;
    rt->dcc = dcc;
    rt->vram_offset = rx_9070_xt_vram_alloc((uint64_t)width * height * 4ULL, 4U);
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_render_target_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_rt_used, RX_9070_XT_MAX_RENDER_TARGETS)) return;
    rx_9070_xt_vram_free(g_rt[handle].vram_offset);
    g_rt[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression)
{
    uint32_t i = find_free_slot(g_ds_used, RX_9070_XT_MAX_DEPTH_STENCILS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_depth_stencil *ds = &g_ds[i];
    ds->used = 1;
    ds->width = width;
    ds->height = height;
    ds->format = format;
    ds->buffer_addr = RX_9070_XT_FB_ADDR + 0xE0000000ULL + (uint64_t)i * 0x10000000ULL;
    ds->compression = compression;
    ds->vram_offset = rx_9070_xt_vram_alloc((uint64_t)width * height * 4ULL, 5U);
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_depth_stencil_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_ds_used, RX_9070_XT_MAX_DEPTH_STENCILS)) return;
    rx_9070_xt_vram_free(g_ds[handle].vram_offset);
    g_ds[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)
{
    uint32_t i = find_free_slot(g_shader_used, RX_9070_XT_MAX_SHADERS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_shader *s = &g_shader[i];
    s->used = 1;
    s->type = type;
    s->code_size = code_size;
    s->code = (uint8_t *)code;
    s->constant_count = 0;
    s->compiled = 0;
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_shader_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RX_9070_XT_MAX_SHADERS)) return;
    g_shader[handle].used = 0;
}

void rx_9070_xt_shader_compile(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RX_9070_XT_MAX_SHADERS)) return;
    g_shader[handle].compiled = 1;
}

rx_9070_xt_handle_t rx_9070_xt_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable, uint8_t compare_func)
{
    uint32_t i = find_free_slot(g_sampler_used, RX_9070_XT_MAX_SAMPLERS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_sampler_state *s = &g_sampler[i];
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
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_sampler_state_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_sampler_used, RX_9070_XT_MAX_SAMPLERS)) return;
    g_sampler[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor)
{
    uint32_t i = find_free_slot(g_blend_used, RX_9070_XT_MAX_BLEND_STATES);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_blend_state *b = &g_blend[i];
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
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_blend_state_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_blend_used, RX_9070_XT_MAX_BLEND_STATES)) return;
    g_blend[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage, uint8_t vrs)
{
    uint32_t i = find_free_slot(g_raster_used, RX_9070_XT_MAX_RASTERIZER_STATES);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_rasterizer_state *r = &g_raster[i];
    r->used = 1;
    r->fill_mode = fill_mode;
    r->cull_mode = cull_mode;
    r->front_counter_clockwise = front_ccw;
    r->depth_clip_enable = depth_clip;
    r->scissor_enable = scissor;
    r->multisample_enable = multisample;
    r->antialiased_line_enable = aa_line;
    r->alpha_to_coverage = alpha_to_coverage;
    r->vrs = vrs;
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_rasterizer_state_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_raster_used, RX_9070_XT_MAX_RASTERIZER_STATES)) return;
    g_raster[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)
{
    uint32_t i = find_free_slot(g_ds_state_used, RX_9070_XT_MAX_DEPTH_STENCIL_STATES);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_depth_stencil_state *d = &g_ds_state[i];
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
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_depth_stencil_state_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_ds_state_used, RX_9070_XT_MAX_DEPTH_STENCIL_STATES)) return;
    g_ds_state[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)
{
    uint32_t i = find_free_slot(g_il_used, RX_9070_XT_MAX_INPUT_LAYOUTS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_input_layout *l = &g_il[i];
    l->used = 1;
    l->element_count = element_count;
    for (uint32_t k = 0; k < element_count && k < 32U; k++) {
        l->slot[k] = slots ? slots[k] : 0U;
        l->offset[k] = offsets ? offsets[k] : 0U;
        l->format[k] = formats ? formats[k] : RX_9070_XT_VERTEX_FORMAT_XYZ;
    }
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_input_layout_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_il_used, RX_9070_XT_MAX_INPUT_LAYOUTS)) return;
    g_il[handle].used = 0;
}

rx_9070_xt_handle_t rx_9070_xt_constant_buffer_create(uint32_t size)
{
    uint32_t i = find_free_slot(g_cb_used, RX_9070_XT_MAX_CONSTANT_BUFFERS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;

    struct rx_9070_xt_constant_buffer *c = &g_cb[i];
    c->used = 1;
    c->size = size;
    c->data = (uint8_t *)0;
    c->vram_offset = rx_9070_xt_vram_alloc(size, 6U);
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_constant_buffer_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_cb_used, RX_9070_XT_MAX_CONSTANT_BUFFERS)) return;
    rx_9070_xt_vram_free(g_cb[handle].vram_offset);
    g_cb[handle].used = 0;
}

void rx_9070_xt_constant_buffer_update(rx_9070_xt_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_cb_used, RX_9070_XT_MAX_CONSTANT_BUFFERS)) return;
    struct rx_9070_xt_constant_buffer *c = &g_cb[handle];
    if (!c->data) return;
    for (uint32_t k = 0; k < size; k++) {
        c->data[offset + k] = ((const uint8_t *)data)[k];
    }
}

rx_9070_xt_handle_t rx_9070_xt_acceleration_structure_create(uint32_t geometry_count, const uint32_t *vertex_buffers, const uint32_t *index_buffers, const uint32_t *vertex_counts, const uint32_t *index_counts)
{
    (void)geometry_count;
    (void)vertex_buffers;
    (void)index_buffers;
    (void)vertex_counts;
    (void)index_counts;
    uint32_t i = find_free_slot(g_shader_used, RX_9070_XT_MAX_SHADERS);
    if (i == RX_9070_XT_HANDLE_NULL) return RX_9070_XT_HANDLE_NULL;
    struct rx_9070_xt_shader *s = &g_shader[i];
    s->used = 1;
    s->type = 0xFFFFFFFFU;
    s->code_size = 0U;
    s->code = 0;
    s->constant_count = 0;
    s->compiled = 1;
    return (rx_9070_xt_handle_t)i;
}

void rx_9070_xt_acceleration_structure_destroy(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RX_9070_XT_MAX_SHADERS)) return;
    g_shader[handle].used = 0;
}

void rx_9070_xt_acceleration_structure_build(rx_9070_xt_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RX_9070_XT_MAX_SHADERS)) return;
    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x30U);
    rx_9070_xt_cp_gfx_emit(handle);
    rx_9070_xt_cp_gfx_flush();
}

void rx_9070_xt_bind_vertex_buffer(rx_9070_xt_handle_t handle)
{
    g_ctx.vertex_buffer = handle;
}

void rx_9070_xt_bind_index_buffer(rx_9070_xt_handle_t handle)
{
    g_ctx.index_buffer = handle;
}

void rx_9070_xt_bind_input_layout(rx_9070_xt_handle_t handle)
{
    g_ctx.input_layout = handle;
}

void rx_9070_xt_bind_vertex_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.vertex_shader = handle;
}

void rx_9070_xt_bind_pixel_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.pixel_shader = handle;
}

void rx_9070_xt_bind_geometry_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.geometry_shader = handle;
}

void rx_9070_xt_bind_compute_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.compute_shader = handle;
}

void rx_9070_xt_bind_ray_gen_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.ray_gen_shader = handle;
}

void rx_9070_xt_bind_any_hit_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.any_hit_shader = handle;
}

void rx_9070_xt_bind_closest_hit_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.closest_hit_shader = handle;
}

void rx_9070_xt_bind_miss_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.miss_shader = handle;
}

void rx_9070_xt_bind_callable_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.callable_shader = handle;
}

void rx_9070_xt_bind_intersection_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.intersection_shader = handle;
}

void rx_9070_xt_bind_mesh_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.mesh_shader = handle;
}

void rx_9070_xt_bind_amplification_shader(rx_9070_xt_handle_t handle)
{
    g_ctx.amplification_shader = handle;
}

void rx_9070_xt_bind_render_target(rx_9070_xt_handle_t handle)
{
    g_ctx.render_target = handle;
}

void rx_9070_xt_bind_depth_stencil(rx_9070_xt_handle_t handle)
{
    g_ctx.depth_stencil = handle;
}

void rx_9070_xt_bind_blend_state(rx_9070_xt_handle_t handle)
{
    g_ctx.blend_state = handle;
}

void rx_9070_xt_bind_rasterizer_state(rx_9070_xt_handle_t handle)
{
    g_ctx.rasterizer_state = handle;
}

void rx_9070_xt_bind_depth_stencil_state(rx_9070_xt_handle_t handle)
{
    g_ctx.depth_stencil_state = handle;
}

void rx_9070_xt_bind_acceleration_structure(rx_9070_xt_handle_t handle)
{
    g_ctx.acceleration_structure = handle;
}

void rx_9070_xt_bind_sampler_and_texture(uint32_t slot, rx_9070_xt_handle_t sampler, rx_9070_xt_handle_t texture)
{
    if (slot >= RX_9070_XT_MAX_SAMPLERS) return;
    g_ctx.sampler[slot] = sampler;
    g_ctx.texture[slot] = texture;
}

void rx_9070_xt_bind_constant_buffer(uint32_t slot, rx_9070_xt_handle_t handle)
{
    if (slot >= RX_9070_XT_MAX_CONSTANT_BUFFERS) return;
    g_ctx.constant_buffer[slot] = handle;
}

void rx_9070_xt_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
    g_ctx.viewport_x = x;
    g_ctx.viewport_y = y;
    g_ctx.viewport_w = w;
    g_ctx.viewport_h = h;
    g_ctx.viewport_min_depth = min_depth;
    g_ctx.viewport_max_depth = max_depth;

    uint32_t packed = (x & 0xFFFFU) | ((y & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_OFFSET, packed);
    packed = (w & 0xFFFFU) | ((h & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_OFFSET + 4U, packed);
}

void rx_9070_xt_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    g_ctx.scissor_left = left;
    g_ctx.scissor_top = top;
    g_ctx.scissor_right = right;
    g_ctx.scissor_bottom = bottom;

    uint32_t packed = (left & 0xFFFFU) | ((top & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_TL, packed);
    packed = (right & 0xFFFFU) | ((bottom & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_BR, packed);
}

void rx_9070_xt_set_primitive_topology(uint32_t topology)
{
    g_ctx.primitive_topology = topology;
    mmio_write32(REG_PA_SU_PRIM_FILTER_CNTL, topology);
}

void rx_9070_xt_set_alpha_blend_global(uint8_t enable)
{
    g_alpha_blend_global = enable;
}

void rx_9070_xt_set_ray_tracing(uint8_t enable)
{
    g_ray_tracing = enable;
    g_ctx.rt_enabled = enable;
    if (enable) mmio_write32(REG_RT_ACCEL_STRUCT, RT_ENABLE);
    else mmio_write32(REG_RT_ACCEL_STRUCT, 0U);
}

void rx_9070_xt_set_mesh_shader(uint8_t enable)
{
    g_mesh_shader = enable;
    g_ctx.mesh_enabled = enable;
    if (enable) mmio_write32(REG_PA_MESH_CNTL, MESH_ENABLE);
    else mmio_write32(REG_PA_MESH_CNTL, 0U);
}

void rx_9070_xt_set_variable_rate_shading(uint8_t enable)
{
    g_variable_rate_shading = enable;
    g_ctx.vrs_enabled = enable;
    if (enable) mmio_write32(REG_VRS_SHADING_RATE, VRS_ENABLE);
    else mmio_write32(REG_VRS_SHADING_RATE, 0U);
}

void rx_9070_xt_set_delta_color_compression(uint8_t enable)
{
    g_delta_color_compression = enable;
    g_ctx.dcc_enabled = enable;
    if (enable) mmio_write32(REG_CB_DCC_CONTROL, DCC_ENABLE);
    else mmio_write32(REG_CB_DCC_CONTROL, 0U);
}

void rx_9070_xt_set_hdr(uint8_t enable)
{
    g_hdr = enable;
    g_ctx.hdr_enabled = enable;
    if (enable) mmio_write32(REG_DCN_HDR_CONTROL, HDR_ENABLE);
    else mmio_write32(REG_DCN_HDR_CONTROL, 0U);
}

void rx_9070_xt_set_freesync(uint8_t enable)
{
    g_freesync = enable;
    g_ctx.freesync_enabled = enable;
    uint32_t ctl = mmio_read32(REG_DCN_CRTC_CONTROL);
    if (enable) ctl |= CRTC_CONTROL_FREESYNC;
    else ctl &= ~CRTC_CONTROL_FREESYNC;
    mmio_write32(REG_DCN_CRTC_CONTROL, ctl);
}

void rx_9070_xt_clear_render_target(rx_9070_xt_handle_t rt, uint32_t color)
{
    if (!handle_valid(rt, g_rt_used, RX_9070_XT_MAX_RENDER_TARGETS)) return;
    struct rx_9070_xt_render_target *target = &g_rt[rt];
    mmio_write32(REG_CB_CLEAR_COLOR, color);
    mmio_write32(REG_CB_CLEAR_COLOR_HIGH, 0U);
    mmio_write32(REG_CB_TARGET0_BASE, target->buffer_addr);
    mmio_write32(REG_CB_TARGET0_PITCH, target->width * 4U);
    rx_9070_xt_delay_us(100);
}

void rx_9070_xt_clear_depth_stencil(rx_9070_xt_handle_t ds, float depth, uint8_t stencil)
{
    if (!handle_valid(ds, g_ds_used, RX_9070_XT_MAX_DEPTH_STENCILS)) return;
    struct rx_9070_xt_depth_stencil *depth_stencil = &g_ds[ds];
    uint32_t depth_bits = 0;
    const uint8_t *depth_bytes = (const uint8_t *)&depth;
    for (int k = 0; k < 4; k++) {
        ((uint8_t *)&depth_bits)[k] = depth_bytes[k];
    }
    mmio_write32(REG_DB_CLEAR, depth_bits);
    mmio_write32(REG_DB_STENCIL_CLEAR, (uint32_t)stencil);
    mmio_write32(REG_DB_Z_INFO, depth_stencil->buffer_addr | (depth_stencil->compression ? DB_Z_INFO_COMPRESSION_ENABLE : 0U));
    rx_9070_xt_delay_us(100);
}

void rx_9070_xt_draw(uint32_t vertex_count, uint32_t start_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_SQ_PROGRAM + 8U, g_ctx.geometry_shader);

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x10U);
    rx_9070_xt_cp_gfx_emit(vertex_count);
    rx_9070_xt_cp_gfx_emit(start_vertex);
    rx_9070_xt_cp_gfx_emit(g_ctx.vertex_buffer);
    rx_9070_xt_cp_gfx_emit(g_ctx.primitive_topology);
    rx_9070_xt_cp_gfx_emit(g_ctx.blend_state);
    rx_9070_xt_cp_gfx_emit(g_ctx.rasterizer_state);
    rx_9070_xt_cp_gfx_emit(g_ctx.depth_stencil_state);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_SQ_PROGRAM + 8U, g_ctx.geometry_shader);

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x11U);
    rx_9070_xt_cp_gfx_emit(index_count);
    rx_9070_xt_cp_gfx_emit(start_index);
    rx_9070_xt_cp_gfx_emit((uint32_t)base_vertex);
    rx_9070_xt_cp_gfx_emit(g_ctx.index_buffer);
    rx_9070_xt_cp_gfx_emit(g_ctx.primitive_topology);
    rx_9070_xt_cp_gfx_emit(g_ctx.blend_state);
    rx_9070_xt_cp_gfx_emit(g_ctx.rasterizer_state);
    rx_9070_xt_cp_gfx_emit(g_ctx.depth_stencil_state);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance)
{
    if (!g_initialized) return;

    mmio_write32(REG_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_SQ_PROGRAM + 8U, g_ctx.geometry_shader);

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x12U);
    rx_9070_xt_cp_gfx_emit(index_count);
    rx_9070_xt_cp_gfx_emit(start_index);
    rx_9070_xt_cp_gfx_emit((uint32_t)base_vertex);
    rx_9070_xt_cp_gfx_emit(instance_count);
    rx_9070_xt_cp_gfx_emit(start_instance);
    rx_9070_xt_cp_gfx_emit(g_ctx.index_buffer);
    rx_9070_xt_cp_gfx_emit(g_ctx.primitive_topology);
    rx_9070_xt_cp_gfx_emit(g_ctx.blend_state);
    rx_9070_xt_cp_gfx_emit(g_ctx.rasterizer_state);
    rx_9070_xt_cp_gfx_emit(g_ctx.depth_stencil_state);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride)
{
    if (!g_initialized) return;

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x13U);
    rx_9070_xt_cp_gfx_emit(buffer_addr);
    rx_9070_xt_cp_gfx_emit(draw_count);
    rx_9070_xt_cp_gfx_emit(stride);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
{
    if (!g_initialized) return;

    mmio_write32(REG_SQ_COMPUTE_PROGRAM, g_ctx.compute_shader);

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x20U);
    rx_9070_xt_cp_gfx_emit(group_x);
    rx_9070_xt_cp_gfx_emit(group_y);
    rx_9070_xt_cp_gfx_emit(group_z);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_compute_dispatch_indirect(uint32_t buffer_addr)
{
    if (!g_initialized) return;

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x21U);
    rx_9070_xt_cp_gfx_emit(buffer_addr);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_ray_trace(uint32_t width, uint32_t height, uint32_t depth, uint32_t accel_struct, uint32_t ray_gen_shader)
{
    if (!g_initialized || !g_ray_tracing) return;

    mmio_write32(REG_RT_ACCEL_STRUCT, accel_struct | RT_ENABLE);
    mmio_write32(REG_RT_RAY_GEN, ray_gen_shader);

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x40U);
    rx_9070_xt_cp_gfx_emit(width);
    rx_9070_xt_cp_gfx_emit(height);
    rx_9070_xt_cp_gfx_emit(depth);
    rx_9070_xt_cp_gfx_emit(accel_struct);
    rx_9070_xt_cp_gfx_emit(ray_gen_shader);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_mesh_draw(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    if (!g_initialized || !g_mesh_shader) return;

    mmio_write32(REG_SQ_MESH_PROGRAM, g_ctx.mesh_shader);

    rx_9070_xt_cp_gfx_emit(0x80000000U | 0x50U);
    rx_9070_xt_cp_gfx_emit(group_count_x);
    rx_9070_xt_cp_gfx_emit(group_count_y);
    rx_9070_xt_cp_gfx_emit(group_count_z);
    rx_9070_xt_cp_gfx_flush();

    rx_9070_xt_delay_us(200);
}

void rx_9070_xt_fence_create(rx_9070_xt_handle_t *out)
{
    if (!out) return;
    for (uint32_t i = 0; i < RX_9070_XT_MAX_FENCES; i++) {
        if (!g_fence_used[i]) {
            g_fence_used[i] = 1;
            g_fence[i].used = 1;
            g_fence[i].value = 0U;
            g_fence[i].signaled = 0;
            *out = (rx_9070_xt_handle_t)i;
            return;
        }
    }
    *out = RX_9070_XT_HANDLE_NULL;
}

void rx_9070_xt_fence_signal(rx_9070_xt_handle_t handle, uint32_t value)
{
    if (handle >= RX_9070_XT_MAX_FENCES || !g_fence_used[handle]) return;
    g_fence[handle].value = value;
    g_fence[handle].signaled = 1;
}

int rx_9070_xt_fence_wait(rx_9070_xt_handle_t handle, uint32_t value, uint32_t timeout_us)
{
    if (handle >= RX_9070_XT_MAX_FENCES || !g_fence_used[handle]) return 0;
    for (uint32_t wait = 0; wait < timeout_us; wait++) {
        if (g_fence[handle].signaled && g_fence[handle].value >= value) return 1;
        rx_9070_xt_delay_us(1);
    }
    return 0;
}

void rx_9070_xt_fence_destroy(rx_9070_xt_handle_t handle)
{
    if (handle >= RX_9070_XT_MAX_FENCES || !g_fence_used[handle]) return;
    g_fence[handle].used = 0;
    g_fence_used[handle] = 0;
}

void rx_9070_xt_semaphore_create(rx_9070_xt_handle_t *out)
{
    if (!out) return;
    for (uint32_t i = 0; i < RX_9070_XT_MAX_SEMAPHORES; i++) {
        if (!g_semaphore_used[i]) {
            g_semaphore_used[i] = 1;
            g_semaphore[i].used = 1;
            g_semaphore[i].value = 0U;
            *out = (rx_9070_xt_handle_t)i;
            return;
        }
    }
    *out = RX_9070_XT_HANDLE_NULL;
}

void rx_9070_xt_semaphore_signal(rx_9070_xt_handle_t handle, uint32_t value)
{
    if (handle >= RX_9070_XT_MAX_SEMAPHORES || !g_semaphore_used[handle]) return;
    g_semaphore[handle].value = value;
}

void rx_9070_xt_semaphore_wait(rx_9070_xt_handle_t handle, uint32_t value)
{
    if (handle >= RX_9070_XT_MAX_SEMAPHORES || !g_semaphore_used[handle]) return;
    for (volatile uint32_t wait = 0; wait < 1000000U; wait++) {
        if (g_semaphore[handle].value >= value) break;
    }
}

void rx_9070_xt_semaphore_destroy(rx_9070_xt_handle_t handle)
{
    if (handle >= RX_9070_XT_MAX_SEMAPHORES || !g_semaphore_used[handle]) return;
    g_semaphore[handle].used = 0;
    g_semaphore_used[handle] = 0;
}

int rx_9070_xt_wait_idle(uint32_t timeout_us)
{
    for (uint32_t wait = 0; wait < timeout_us; wait++) {
        uint32_t wptr = mmio_read32(REG_CP_GFX_ME_RB_WPTR);
        uint32_t rptr = mmio_read32(REG_CP_GFX_ME_RB_RPTR);
        if (wptr == rptr) return 1;
        rx_9070_xt_delay_us(1);
    }
    return 0;
}

uint64_t rx_9070_xt_timestamp(void)
{
    uint32_t lo = mmio_read32(REG_TIMESTAMP_LO);
    uint32_t hi = mmio_read32(REG_TIMESTAMP_HI);
    return ((uint64_t)hi << 32) | lo;
}

void rx_9070_xt_power_set_state(uint8_t state)
{
    g_power_state = state;
    mmio_write32(REG_POWER_STATE, state);
    rx_9070_xt_delay_us(100);
}

uint8_t rx_9070_xt_power_get_state(void)
{
    return g_power_state;
}

void rx_9070_xt_set_clock_gfx(uint32_t mhz)
{
    g_clock_gfx = mhz;
    mmio_write32(REG_GFX_CLOCK_CNTL, mhz);
}

void rx_9070_xt_set_clock_memory(uint32_t mhz)
{
    g_clock_memory = mhz;
    mmio_write32(REG_MEMORY_CLOCK_CNTL, mhz);
}

uint32_t rx_9070_xt_get_clock_gfx(void)
{
    return g_clock_gfx;
}

uint32_t rx_9070_xt_get_clock_memory(void)
{
    return g_clock_memory;
}

uint32_t rx_9070_xt_get_temperature(void)
{
    uint32_t temp = mmio_read32(REG_THERMAL_TEMP);
    if (temp != 0xFFFFFFFFU) g_temperature = temp & 0xFFU;
    return g_temperature;
}

void rx_9070_xt_set_fan_speed(uint32_t rpm)
{
    g_fan_rpm = rpm;
    mmio_write32(REG_FAN_PWM, rpm);
    mmio_write32(REG_FAN_RPM, rpm);
}

uint32_t rx_9070_xt_get_fan_speed(void)
{
    uint32_t rpm = mmio_read32(REG_FAN_RPM);
    if (rpm != 0xFFFFFFFFU) g_fan_rpm = rpm;
    return g_fan_rpm;
}

void rx_9070_xt_rgb_set(uint32_t color, uint8_t mode)
{
    mmio_write32(REG_RGB_CONTROL, (uint32_t)mode);
    mmio_write32(REG_RGB_COLOR, color);
}

void rx_9070_xt_rgb_off(void)
{
    mmio_write32(REG_RGB_CONTROL, 0U);
    mmio_write32(REG_RGB_COLOR, 0U);
}

int rx_9070_xt_feature_ray_tracing(void)
{
    return g_ray_tracing ? 1 : 0;
}

int rx_9070_xt_feature_mesh_shader(void)
{
    return g_mesh_shader ? 1 : 0;
}

int rx_9070_xt_feature_variable_rate_shading(void)
{
    return g_variable_rate_shading ? 1 : 0;
}

int rx_9070_xt_feature_sampler_feedback(void)
{
    return 1;
}

int rx_9070_xt_feature_hdr(void)
{
    return g_hdr ? 1 : 0;
}

int rx_9070_xt_feature_freesync(void)
{
    return g_freesync ? 1 : 0;
}

int rx_9070_xt_feature_delta_color_compression(void)
{
    return g_delta_color_compression ? 1 : 0;
}

rx_9070_xt_handle_t rx_9070_xt_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied, uint8_t hdr, uint8_t triple_buffer)
{
    (void)alpha_premultiplied;
    (void)hdr;
    (void)triple_buffer;
    for (uint32_t i = 0; i < RX_9070_XT_MAX_SWAP_CHAINS; i++) {
        if (!g_swap_used[i]) {
            g_swap_used[i] = 1;
            g_swap[i].used = 1;
            g_swap[i].width = width;
            g_swap[i].height = height;
            g_swap[i].back_buffer_addr = RX_9070_XT_FB_ADDR + 0x10000000U + i * 0x10000000U;
            g_swap[i].front_buffer_addr = RX_9070_XT_FB_ADDR;
            g_swap[i].vsync = vsync;
            g_swap[i].alpha_premultiplied = alpha_premultiplied;
            g_swap[i].hdr = hdr;
            g_swap[i].triple_buffer = triple_buffer;
            g_swap[i].back_vram_offset = rx_9070_xt_vram_alloc((uint64_t)width * height * 4ULL, 7U);
            g_swap[i].front_vram_offset = 0ULL;
            return (rx_9070_xt_handle_t)i;
        }
    }
    return RX_9070_XT_HANDLE_NULL;
}

void rx_9070_xt_swap_chain_destroy(rx_9070_xt_handle_t handle)
{
    if (handle >= RX_9070_XT_MAX_SWAP_CHAINS || !g_swap_used[handle]) return;
    rx_9070_xt_vram_free(g_swap[handle].back_vram_offset);
    g_swap[handle].used = 0;
    g_swap_used[handle] = 0;
}

void rx_9070_xt_present(rx_9070_xt_handle_t chain)
{
    if (chain >= RX_9070_XT_MAX_SWAP_CHAINS || !g_swap_used[chain]) return;

    if (g_swap[chain].vsync) {
        for (volatile uint32_t wait = 0; wait < 4000000U; wait++) {
            if (mmio_read32(REG_DCN_CRTC_CONTROL) & 0x80000000U) break;
        }
    }

    rx_9070_xt_cp_gfx_emit(0x40000000U | 0x02U);
    rx_9070_xt_cp_gfx_emit((0U << 16) | 0U);
    rx_9070_xt_cp_gfx_emit(((RX_9070_XT_FB_HEIGHT - 1U) << 16) | (RX_9070_XT_FB_WIDTH - 1U));
    rx_9070_xt_cp_gfx_emit((0U << 16) | 0U);
    rx_9070_xt_cp_gfx_emit(0U);
    rx_9070_xt_cp_gfx_flush();
}

void rx_9070_xt_command_begin(void)
{
    g_cmd_recording = 1;
    g_cmd_count = 0;
}

void rx_9070_xt_command_end(void)
{
    if (!g_cmd_recording) return;
    g_cmd_recording = 0;
    for (uint32_t i = 0; i < g_cmd_count; i++) {
        rx_9070_xt_cp_gfx_emit(g_cmd_buf[i]);
    }
    rx_9070_xt_cp_gfx_flush();
    g_cmd_count = 0;
}

void rx_9070_xt_context_get(struct rx_9070_xt_context *out)
{
    if (!out) return;
    *out = g_ctx;
}

int rx_9070_xt_3d_available(void)
{
    return g_initialized ? 1 : 0;
}
