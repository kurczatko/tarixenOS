#ifndef RADEON_HD6000_H
#define RADEON_HD6000_H

#include "../../include/printf/types.h"

#define RADEON_HD6000_VENDOR_ID 0x1002U

#define RADEON_HD6000_DEVICE_ID_BARTS_XT 0x6738U
#define RADEON_HD6000_DEVICE_ID_BARTS_PRO 0x6739U
#define RADEON_HD6000_DEVICE_ID_BARTS_LE 0x673BU
#define RADEON_HD6000_DEVICE_ID_CAYMAN_XT 0x6738U
#define RADEON_HD6000_DEVICE_ID_CAYMAN_PRO 0x673CU
#define RADEON_HD6000_DEVICE_ID_TURKS_XT 0x6740U
#define RADEON_HD6000_DEVICE_ID_TURKS_PRO 0x6741U
#define RADEON_HD6000_DEVICE_ID_TURKS_LE 0x6742U

#define RADEON_HD6000_VRAM_SIZE_BYTES ((uint64_t)2ULL * 1024ULL * 1024ULL * 1024ULL)
#define RADEON_HD6000_VRAM_PAGE_SIZE 4096U
#define RADEON_HD6000_VRAM_MAX_ALLOCS 4096U
#define RADEON_HD6000_FB_WIDTH 1920U
#define RADEON_HD6000_FB_HEIGHT 1080U
#define RADEON_HD6000_FB_PITCH (RADEON_HD6000_FB_WIDTH * 4U)
#define RADEON_HD6000_FB_ADDR 0xFE000000U

#define RADEON_HD6000_MAX_VERTEX_BUFFERS 64U
#define RADEON_HD6000_MAX_INDEX_BUFFERS 64U
#define RADEON_HD6000_MAX_TEXTURES 128U
#define RADEON_HD6000_MAX_RENDER_TARGETS 8U
#define RADEON_HD6000_MAX_DEPTH_STENCILS 8U
#define RADEON_HD6000_MAX_SHADERS 64U
#define RADEON_HD6000_MAX_COMPUTE_SHADERS 16U
#define RADEON_HD6000_MAX_SAMPLERS 16U
#define RADEON_HD6000_MAX_BLEND_STATES 16U
#define RADEON_HD6000_MAX_RASTERIZER_STATES 16U
#define RADEON_HD6000_MAX_DEPTH_STENCIL_STATES 16U
#define RADEON_HD6000_MAX_INPUT_LAYOUTS 32U
#define RADEON_HD6000_MAX_CONSTANT_BUFFERS 16U
#define RADEON_HD6000_MAX_SWAP_CHAINS 4U
#define RADEON_HD6000_MAX_FENCES 16U
#define RADEON_HD6000_MAX_SEMAPHORES 16U
#define RADEON_HD6000_COMMAND_BUFFER_SIZE 16384U
#define RADEON_HD6000_RING_BUFFER_SIZE 8192U

#define RADEON_HD6000_HANDLE_NULL 0xFFFFFFFFU
typedef uint32_t radeon_hd6000_handle_t;

enum radeon_hd6000_reg {
    REG_HD6000_CP_ME_CNTL = 0x08BCU,
    REG_HD6000_CP_ME_RB_ADDR = 0x08C4U,
    REG_HD6000_CP_ME_RB_SIZE = 0x08C8U,
    REG_HD6000_CP_ME_RB_WPTR = 0x08CCU,
    REG_HD6000_CP_ME_RB_RPTR = 0x08D0U,
    REG_HD6000_CP_ME_RB_CNTL = 0x08D4U,
    REG_HD6000_CP_ME_HALT = 0x08B8U,
    REG_HD6000_CP_ME_RESET = 0x08B4U,
    REG_HD6000_CP_PFP_CNTL = 0x08B0U,
    REG_HD6000_CP_PFP_HALT = 0x08B0U,
    REG_HD6000_INT_STAT = 0x0100U,
    REG_HD6000_INT_MASK = 0x0104U,
    REG_HD6000_INT_ACK = 0x0108U,
    REG_HD6000_GFX_CLOCK_CNTL = 0x1000U,
    REG_HD6000_MEMORY_CLOCK_CNTL = 0x1004U,
    REG_HD6000_DISPLAY_CLOCK_CNTL = 0x1008U,
    REG_HD6000_POWER_STATE = 0x1010U,
    REG_HD6000_THERMAL_TEMP = 0x1014U,
    REG_HD6000_FAN_PWM = 0x1018U,
    REG_HD6000_FAN_RPM = 0x101CU,
    REG_HD6000_RGB_CONTROL = 0x1020U,
    REG_HD6000_RGB_COLOR = 0x1024U,
    REG_HD6000_VRAM_USAGE = 0x1028U,
    REG_HD6000_VRAM_TOTAL = 0x102CU,
    REG_HD6000_TIMESTAMP_LO = 0x1030U,
    REG_HD6000_TIMESTAMP_HI = 0x1034U,
    REG_HD6000_DCN_DISP_CONTROL = 0x2000U,
    REG_HD6000_DCN_CRTC_CONTROL = 0x2004U,
    REG_HD6000_DCN_CRTC_H_TOTAL = 0x2008U,
    REG_HD6000_DCN_CRTC_V_TOTAL = 0x200CU,
    REG_HD6000_DCN_CRTC_H_BLANK = 0x2010U,
    REG_HD6000_DCN_CRTC_V_BLANK = 0x2014U,
    REG_HD6000_DCN_CRTC_H_SYNC_A = 0x2018U,
    REG_HD6000_DCN_CRTC_V_SYNC_A = 0x201CU,
    REG_HD6000_DCN_CRTC_PRIMARY_VIEWPORT_START = 0x2020U,
    REG_HD6000_DCN_CRTC_PRIMARY_VIEWPORT_END = 0x2024U,
    REG_HD6000_DCN_CRTC_PRIMARY_SURFACE_ADDRESS = 0x2028U,
    REG_HD6000_DCN_CRTC_PRIMARY_SURFACE_ADDRESS_HIGH = 0x202CU,
    REG_HD6000_DCN_CRTC_PRIMARY_SURFACE_OFFSET_X = 0x2030U,
    REG_HD6000_DCN_CRTC_PRIMARY_SURFACE_OFFSET_Y = 0x2034U,
    REG_HD6000_DCN_CRTC_PRIMARY_SURFACE_PIXEL_PITCH = 0x2038U,
    REG_HD6000_DCN_CRTC_PRIMARY_SURFACE_TILE = 0x203CU,
    REG_HD6000_DCN_PLANE_OFFSET_X = 0x2040U,
    REG_HD6000_DCN_PLANE_OFFSET_Y = 0x2044U,
    REG_HD6000_DCN_PLANE_PIXEL_PITCH = 0x2048U,
    REG_HD6000_DCN_PLANE_SURFACE_ADDRESS = 0x204CU,
    REG_HD6000_DCN_PLANE_SURFACE_ADDRESS_HIGH = 0x2050U,
    REG_HD6000_CURSOR_CONTROL = 0x2100U,
    REG_HD6000_CURSOR_POSITION = 0x2104U,
    REG_HD6000_CURSOR_COLOR0 = 0x2108U,
    REG_HD6000_CURSOR_COLOR1 = 0x210CU,
    REG_HD6000_CURSOR_BASE = 0x2110U,
    REG_HD6000_CB_TARGET0_BASE = 0x8800U,
    REG_HD6000_CB_TARGET0_PITCH = 0x8804U,
    REG_HD6000_CB_TARGET0_SLICE = 0x8808U,
    REG_HD6000_CB_TARGET0_FMASK = 0x880CU,
    REG_HD6000_CB_TARGET0_VIEW = 0x8810U,
    REG_HD6000_CB_SHADER_MASK = 0x88F0U,
    REG_HD6000_CB_CLEAR_COLOR = 0x88F4U,
    REG_HD6000_CB_CLEAR_COLOR_HIGH = 0x88F8U,
    REG_HD6000_DB_Z_INFO = 0x8A00U,
    REG_HD6000_DB_Z_READ_BASE = 0x8A04U,
    REG_HD6000_DB_Z_READ_BASE_HIGH = 0x8A08U,
    REG_HD6000_DB_Z_WRITE_BASE = 0x8A0CU,
    REG_HD6000_DB_Z_WRITE_BASE_HIGH = 0x8A10U,
    REG_HD6000_DB_Z_READ_OFF = 0x8A14U,
    REG_HD6000_DB_Z_WRITE_OFF = 0x8A18U,
    REG_HD6000_DB_CLEAR = 0x8A20U,
    REG_HD6000_DB_STENCIL_CLEAR = 0x8A24U,
    REG_HD6000_CB_BLEND_CONTROL = 0x8A50U,
    REG_HD6000_CB_BLEND_RED = 0x8A54U,
    REG_HD6000_CB_BLEND_GREEN = 0x8A58U,
    REG_HD6000_CB_BLEND_BLUE = 0x8A5CU,
    REG_HD6000_CB_BLEND_ALPHA = 0x8A60U,
    REG_HD6000_SQ_CONFIG = 0x9800U,
    REG_HD6000_SQ_VTX_BASE = 0x9810U,
    REG_HD6000_SQ_VTX_STRIDE = 0x9814U,
    REG_HD6000_SQ_VTX_NUM_FORMAT = 0x9818U,
    REG_HD6000_SQ_VTX_FMT_0 = 0x9820U,
    REG_HD6000_SQ_VTX_FMT_1 = 0x9824U,
    REG_HD6000_SQ_VTX_FMT_2 = 0x9828U,
    REG_HD6000_SQ_VTX_FMT_3 = 0x982CU,
    REG_HD6000_SQ_PROGRAM = 0x9860U,
    REG_HD6000_SQ_CONSTANT_BASE = 0x98A0U,
    REG_HD6000_SQ_CONSTANT_CACHE = 0x98A4U,
    REG_HD6000_SQ_COMPUTE_PROGRAM = 0x98B0U,
    REG_HD6000_SQ_TEX_BASE = 0x9C00U,
    REG_HD6000_SQ_SAMPLER_BASE = 0x9C80U,
    REG_HD6000_SQ_TEX_LOD = 0x9D00U,
    REG_HD6000_SQ_TEX_DIM = 0x9D80U,
    REG_HD6000_PA_SU_VTX_CNTL = 0xCA00U,
    REG_HD6000_PA_CL_VTE_CNTL = 0xCA04U,
    REG_HD6000_PA_CL_VS_OUT_CNTL = 0xCA08U,
    REG_HD6000_PA_CL_GB_ADV = 0xCA0CU,
    REG_HD6000_PA_CL_VTE_VTX_0 = 0xCA10U,
    REG_HD6000_PA_SU_POLY_OFFSET_FRONT = 0xCA30U,
    REG_HD6000_PA_SU_POLY_OFFSET_BACK = 0xCA34U,
    REG_HD6000_PA_CL_CLIP_CNTL = 0xCA40U,
    REG_HD6000_PA_SU_SC_MODE_CNTL = 0xCA44U,
    REG_HD6000_PA_SC_WINDOW_OFFSET = 0xCA48U,
    REG_HD6000_PA_SC_WINDOW_SCISSOR_TL = 0xCA4CU,
    REG_HD6000_PA_SC_WINDOW_SCISSOR_BR = 0xCA50U,
    REG_HD6000_PA_SU_PRIM_FILTER_CNTL = 0xCA54U,
    REG_HD6000_PA_SU_LINE_STIPPLE = 0xCA58U,
    REG_HD6000_PA_SC_AA_CONFIG = 0xCA5CU,
    REG_HD6000_VM_CONTEXT = 0xCC00U,
    REG_HD6000_VM_PAGE_TABLE = 0xCC04U
};

#define HD6000_CP_ME_CNTL_ME_HALT (1U << 0)
#define HD6000_CP_ME_CNTL_ME_RESET (1U << 1)
#define HD6000_CP_ME_CNTL_PFP_HALT (1U << 8)
#define HD6000_CP_ME_CNTL_PFP_RESET (1U << 9)
#define HD6000_CP_ME_RB_CNTL_RB_ENABLE (1U << 0)
#define HD6000_CP_ME_RB_CNTL_RB_NO_UPDATE (1U << 1)

#define HD6000_CRTC_CONTROL_EN (1U << 0)
#define HD6000_CRTC_CONTROL_PIXEL_WIDTH_32BPP (3U << 2)

#define HD6000_BLEND_CONTROL_SRC_BLEND_SHIFT 0U
#define HD6000_BLEND_CONTROL_DEST_BLEND_SHIFT 8U
#define HD6000_BLEND_CONTROL_BLEND_OP_SHIFT 16U
#define HD6000_BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT 20U
#define HD6000_BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT 24U
#define HD6000_BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT 28U
#define HD6000_BLEND_CONTROL_ROP_ENABLE (1U << 31U)

#define HD6000_SHADER_MASK_ENABLE (1U << 0)
#define HD6000_SHADER_MASK_CB0_ENABLE (1U << 4)

#define HD6000_DB_Z_INFO_FORMAT_SHIFT 0U
#define HD6000_DB_Z_INFO_FORMAT_16BIT 0U
#define HD6000_DB_Z_INFO_FORMAT_24BIT 1U
#define HD6000_DB_Z_INFO_FORMAT_32BIT 2U
#define HD6000_DB_Z_INFO_COMPRESSION_ENABLE (1U << 8)

#define HD6000_POWER_STATE_D0 0U
#define HD6000_POWER_STATE_D1 1U
#define HD6000_POWER_STATE_D2 2U
#define HD6000_POWER_STATE_D3 3U

#define HD6000_VERTEX_FORMAT_XYZ 0x0001U
#define HD6000_VERTEX_FORMAT_XYZW 0x0002U
#define HD6000_VERTEX_FORMAT_XYZ_COLOR 0x0004U
#define HD6000_VERTEX_FORMAT_XYZ_TEX 0x0008U
#define HD6000_VERTEX_FORMAT_XYZ_COLOR_TEX 0x0010U
#define HD6000_VERTEX_FORMAT_XYZ_COLOR_TEX2 0x0020U

#define HD6000_INDEX_FORMAT_16BIT 0x0000U
#define HD6000_INDEX_FORMAT_32BIT 0x0001U

#define HD6000_PRIM_POINTLIST 0x0000U
#define HD6000_PRIM_LINELIST 0x0001U
#define HD6000_PRIM_LINESTRIP 0x0002U
#define HD6000_PRIM_TRIANGLELIST 0x0003U
#define HD6000_PRIM_TRIANGLESTRIP 0x0004U
#define HD6000_PRIM_TRIANGLEFAN 0x0005U
#define HD6000_PRIM_QUADLIST 0x0006U
#define HD6000_PRIM_QUADSTRIP 0x0007U

#define HD6000_TEX_FORMAT_A8R8G8B8 0x0000U
#define HD6000_TEX_FORMAT_X8R8G8B8 0x0001U
#define HD6000_TEX_FORMAT_A8B8G8R8 0x0002U
#define HD6000_TEX_FORMAT_X8B8G8R8 0x0003U
#define HD6000_TEX_FORMAT_R5G6B5 0x0004U
#define HD6000_TEX_FORMAT_DXT1 0x0010U
#define HD6000_TEX_FORMAT_DXT3 0x0011U
#define HD6000_TEX_FORMAT_DXT5 0x0012U
#define HD6000_TEX_FORMAT_FLOAT32_RGBA 0x0020U

#define HD6000_SHADER_VERTEX 0x0000U
#define HD6000_SHADER_PIXEL 0x0001U
#define HD6000_SHADER_GEOMETRY 0x0002U
#define HD6000_SHADER_COMPUTE 0x0003U

#define HD6000_BLEND_ZERO 0x0000U
#define HD6000_BLEND_ONE 0x0001U
#define HD6000_BLEND_SRC_COLOR 0x0002U
#define HD6000_BLEND_DEST_COLOR 0x0003U
#define HD6000_BLEND_SRC_ALPHA 0x0004U
#define HD6000_BLEND_DEST_ALPHA 0x0005U
#define HD6000_BLEND_INV_SRC_ALPHA 0x0006U
#define HD6000_BLEND_INV_DEST_ALPHA 0x0007U
#define HD6000_BLEND_SRC_ALPHA_SAT 0x0008U
#define HD6000_BLEND_BLEND_FACTOR 0x0009U
#define HD6000_BLEND_INV_BLEND_FACTOR 0x000AU

#define HD6000_BLEND_OP_ADD 0x0000U
#define HD6000_BLEND_OP_SUBTRACT 0x0001U
#define HD6000_BLEND_OP_REV_SUBTRACT 0x0002U
#define HD6000_BLEND_OP_MIN 0x0003U
#define HD6000_BLEND_OP_MAX 0x0004U

#define HD6000_CMP_NEVER 0x0000U
#define HD6000_CMP_LESS 0x0001U
#define HD6000_CMP_EQUAL 0x0002U
#define HD6000_CMP_LEQUAL 0x0003U
#define HD6000_CMP_GREATER 0x0004U
#define HD6000_CMP_NOTEQUAL 0x0005U
#define HD6000_CMP_GEQUAL 0x0006U
#define HD6000_CMP_ALWAYS 0x0007U

#define HD6000_FILL_SOLID 0x0000U
#define HD6000_FILL_WIREFRAME 0x0001U

#define HD6000_CULL_NONE 0x0000U
#define HD6000_CULL_FRONT 0x0001U
#define HD6000_CULL_BACK 0x0002U

#define HD6000_STENCIL_KEEP 0x0000U
#define HD6000_STENCIL_ZERO 0x0001U
#define HD6000_STENCIL_REPLACE 0x0002U
#define HD6000_STENCIL_INCR 0x0003U
#define HD6000_STENCIL_DECR 0x0004U
#define HD6000_STENCIL_INVERT 0x0005U

struct radeon_hd6000_info {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint32_t mmio_base;
    uint32_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint8_t initialized;
    uint8_t architecture;
    uint64_t vram_size;
    uint64_t vram_used;
    uint32_t gfx_clock_mhz;
    uint32_t memory_clock_mhz;
    uint32_t temperature_c;
    uint32_t fan_rpm;
};

struct radeon_hd6000_vertex_buffer {
    uint8_t used;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct radeon_hd6000_index_buffer {
    uint8_t used;
    uint32_t index_count;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct radeon_hd6000_texture {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t mip_levels;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint32_t pitch;
    uint64_t vram_offset;
    uint8_t compressed;
};

struct radeon_hd6000_render_target {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t alpha_blend;
    uint64_t vram_offset;
};

struct radeon_hd6000_depth_stencil {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t compression;
    uint64_t vram_offset;
};

struct radeon_hd6000_shader {
    uint8_t used;
    uint32_t type;
    uint32_t code_size;
    uint8_t *code;
    uint32_t constant_count;
    uint8_t compiled;
};

struct radeon_hd6000_sampler_state {
    uint8_t used;
    uint32_t filter_min;
    uint32_t filter_mag;
    uint32_t filter_mip;
    uint32_t address_u;
    uint32_t address_v;
    uint32_t address_w;
    uint32_t max_anisotropy;
    uint8_t compare_enable;
    uint8_t compare_func;
};

struct radeon_hd6000_blend_state {
    uint8_t used;
    uint8_t blend_enable;
    uint32_t src_blend;
    uint32_t dest_blend;
    uint32_t blend_op;
    uint32_t src_blend_alpha;
    uint32_t dest_blend_alpha;
    uint32_t blend_op_alpha;
    uint32_t render_target_write_mask;
    uint32_t blend_factor;
};

struct radeon_hd6000_rasterizer_state {
    uint8_t used;
    uint32_t fill_mode;
    uint32_t cull_mode;
    uint8_t front_counter_clockwise;
    uint8_t depth_clip_enable;
    uint8_t scissor_enable;
    uint8_t multisample_enable;
    uint8_t antialiased_line_enable;
    uint8_t alpha_to_coverage;
};

struct radeon_hd6000_depth_stencil_state {
    uint8_t used;
    uint8_t depth_enable;
    uint32_t depth_write_mask;
    uint32_t depth_func;
    uint8_t stencil_enable;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    uint32_t front_stencil_fail_op;
    uint32_t front_stencil_depth_fail_op;
    uint32_t front_stencil_pass_op;
    uint32_t front_stencil_func;
    uint32_t back_stencil_fail_op;
    uint32_t back_stencil_depth_fail_op;
    uint32_t back_stencil_pass_op;
    uint32_t back_stencil_func;
};

struct radeon_hd6000_input_layout {
    uint8_t used;
    uint32_t element_count;
    uint32_t slot[32];
    uint32_t offset[32];
    uint32_t format[32];
};

struct radeon_hd6000_constant_buffer {
    uint8_t used;
    uint32_t size;
    uint8_t *data;
    uint64_t vram_offset;
};

struct radeon_hd6000_swap_chain {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t back_buffer_addr;
    uint32_t front_buffer_addr;
    uint8_t vsync;
    uint8_t alpha_premultiplied;
    uint64_t back_vram_offset;
    uint64_t front_vram_offset;
};

struct radeon_hd6000_fence {
    uint8_t used;
    uint32_t value;
    uint8_t signaled;
};

struct radeon_hd6000_semaphore {
    uint8_t used;
    uint32_t value;
};

struct radeon_hd6000_vram_alloc {
    uint8_t used;
    uint64_t offset;
    uint64_t size;
    uint32_t type;
};

struct radeon_hd6000_context {
    radeon_hd6000_handle_t vertex_buffer;
    radeon_hd6000_handle_t index_buffer;
    radeon_hd6000_handle_t input_layout;
    radeon_hd6000_handle_t vertex_shader;
    radeon_hd6000_handle_t pixel_shader;
    radeon_hd6000_handle_t geometry_shader;
    radeon_hd6000_handle_t compute_shader;
    radeon_hd6000_handle_t render_target;
    radeon_hd6000_handle_t depth_stencil;
    radeon_hd6000_handle_t blend_state;
    radeon_hd6000_handle_t rasterizer_state;
    radeon_hd6000_handle_t depth_stencil_state;
    radeon_hd6000_handle_t sampler[RADEON_HD6000_MAX_SAMPLERS];
    radeon_hd6000_handle_t texture[RADEON_HD6000_MAX_SAMPLERS];
    radeon_hd6000_handle_t constant_buffer[RADEON_HD6000_MAX_CONSTANT_BUFFERS];
    uint32_t viewport_x;
    uint32_t viewport_y;
    uint32_t viewport_w;
    uint32_t viewport_h;
    float viewport_min_depth;
    float viewport_max_depth;
    uint32_t scissor_left;
    uint32_t scissor_top;
    uint32_t scissor_right;
    uint32_t scissor_bottom;
    uint32_t primitive_topology;
    uint8_t alpha_blend_global;
    uint8_t in_command_buffer;
};

#ifdef __cplusplus

class RadeonHD6000 {
public:
    RadeonHD6000(void);
    int inicjalizuj(void);
    void zamknij(void);
    uint8_t czy_gotowy(void);
    void putpixel(uint32_t x, uint32_t y, uint32_t kolor);
    void fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor);
    void blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
    void cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz);
    void cursor_hide(void);
    void *get_fb(void);
    uint32_t get_szerokosc(void);
    uint32_t get_wysokosc(void);
    uint16_t get_device_id(void);
    uint8_t get_rewizja(void);
    void info_pobierz(struct radeon_hd6000_info *wyjscie);

    uint64_t vram_alokuj(uint64_t rozmiar, uint32_t typ);
    void vram_zwolnij(uint64_t uchwyt);
    uint64_t vram_uzyto(void);
    uint64_t vram_calkowicie(void);

    radeon_hd6000_handle_t bufor_wierzcholkow_stworz(uint32_t liczba_wierzcholkow, uint32_t rozmiar_wierzcholka, uint32_t format, const void *dane);
    void bufor_wierzcholkow_zniszcz(radeon_hd6000_handle_t uchwyt);
    void bufor_wierzcholkow_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

    radeon_hd6000_handle_t bufor_indeksow_stworz(uint32_t liczba_indeksow, uint32_t format, const void *dane);
    void bufor_indeksow_zniszcz(radeon_hd6000_handle_t uchwyt);
    void bufor_indeksow_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

    radeon_hd6000_handle_t tekstura_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint32_t poziomy_mip, const void *dane);
    void tekstura_zniszcz(radeon_hd6000_handle_t uchwyt);
    void tekstura_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t poziom, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *dane);
    void tekstura_wygeneruj_mipmapy(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t cel_renderowania_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t alpha_blend);
    void cel_renderowania_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t bufor_glebii_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t kompresja);
    void bufor_glebii_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t shader_stworz(uint32_t typ, const uint8_t *kod, uint32_t rozmiar_kodu);
    void shader_zniszcz(radeon_hd6000_handle_t uchwyt);
    void shader_skompiluj(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t stan_probnika_stworz(uint32_t filtr_min, uint32_t filtr_mag, uint32_t filtr_mip, uint32_t adres_u, uint32_t adres_v, uint32_t adres_w, uint32_t max_anizotropia, uint8_t wlacz_porownanie, uint8_t func_porownania);
    void stan_probnika_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t stan_mieszania_stworz(uint8_t wlacz_mieszanie, uint32_t src_blend, uint32_t dest_blend, uint32_t op_mieszania, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t op_mieszania_alpha, uint32_t maska_zapisu, uint32_t blend_factor);
    void stan_mieszania_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t stan_rasteryzacji_stworz(uint32_t tryb_wypelnienia, uint32_t tryb_odcinania, uint8_t front_ccw, uint8_t clip_glebii, uint8_t scissor, uint8_t wielopr_probowanie, uint8_t aa_linia, uint8_t alpha_do_pokrycia);
    void stan_rasteryzacji_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t stan_glebii_stencil_stworz(uint8_t wlacz_glebie, uint32_t maska_zapisu_glebii, uint32_t func_glebii, uint8_t wlacz_stencil, uint8_t maska_odczytu_stencil, uint8_t maska_zapisu_stencil, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
    void stan_glebii_stencil_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t uklad_wejsc_stworz(uint32_t liczba_elementow, const uint32_t *sloty, const uint32_t *przesuniecia, const uint32_t *formaty);
    void uklad_wejsc_zniszcz(radeon_hd6000_handle_t uchwyt);

    radeon_hd6000_handle_t bufor_stale_stworz(uint32_t rozmiar);
    void bufor_stale_zniszcz(radeon_hd6000_handle_t uchwyt);
    void bufor_stale_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

    void powiaz_bufor_wierzcholkow(radeon_hd6000_handle_t uchwyt);
    void powiaz_bufor_indeksow(radeon_hd6000_handle_t uchwyt);
    void powiaz_uklad_wejsc(radeon_hd6000_handle_t uchwyt);
    void powiaz_shader_wierzcholkow(radeon_hd6000_handle_t uchwyt);
    void powiaz_shader_pikseli(radeon_hd6000_handle_t uchwyt);
    void powiaz_shader_geometrii(radeon_hd6000_handle_t uchwyt);
    void powiaz_shader_obliczeniowy(radeon_hd6000_handle_t uchwyt);
    void powiaz_cel_renderowania(radeon_hd6000_handle_t uchwyt);
    void powiaz_bufor_glebii(radeon_hd6000_handle_t uchwyt);
    void powiaz_stan_mieszania(radeon_hd6000_handle_t uchwyt);
    void powiaz_stan_rasteryzacji(radeon_hd6000_handle_t uchwyt);
    void powiaz_stan_glebii_stencil(radeon_hd6000_handle_t uchwyt);
    void powiaz_probnik_i_teksture(uint32_t gniazdo, radeon_hd6000_handle_t probnik, radeon_hd6000_handle_t tekstura);
    void powiaz_bufor_stale(uint32_t gniazdo, radeon_hd6000_handle_t uchwyt);

    void ustaw_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_glebia, float max_glebia);
    void ustaw_scissor(uint32_t lewa, uint32_t gora, uint32_t prawa, uint32_t dol);
    void ustaw_typ_pierwotnego(uint32_t typ_pierwotnego);
    void ustaw_mieszanie_alpha_globalne(uint8_t wlacz);

    void wyczysc_cel_renderowania(radeon_hd6000_handle_t cel, uint32_t kolor);
    void wyczysc_bufor_glebii(radeon_hd6000_handle_t bufor, float glebia, uint8_t stencil);
    void rysuj(uint32_t liczba_wierzcholkow, uint32_t pierwszy_wierzcholek);
    void rysuj_indeksowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka);
    void rysuj_indeksowane_instancjonowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka, uint32_t liczba_instancji, uint32_t pierwsza_instancja);
    void rysuj_po_indirekcie(uint32_t adres_bufora, uint32_t liczba_rysowan, uint32_t krok);

    void oblicz_wyslij(uint32_t grupa_x, uint32_t grupa_y, uint32_t grupa_z);
    void oblicz_wyslij_po_indirekcie(uint32_t adres_bufora);

    void ogrodzenie_stworz(radeon_hd6000_handle_t *wyjscie);
    void ogrodzenie_sygnalizuj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc);
    int ogrodzenie_poczekaj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc, uint32_t timeout_us);
    void ogrodzenie_zniszcz(radeon_hd6000_handle_t uchwyt);

    void semafor_stworz(radeon_hd6000_handle_t *wyjscie);
    void semafor_sygnalizuj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc);
    void semafor_poczekaj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc);
    void semafor_zniszcz(radeon_hd6000_handle_t uchwyt);

    int czekaj_na_bezczynnosc(uint32_t timeout_us);
    uint64_t znacznik_czasu(void);

    void ustaw_stan_zasilania(uint8_t stan);
    uint8_t pobierz_stan_zasilania(void);
    void ustaw_zegar_graficzny(uint32_t mhz);
    void ustaw_zegar_pamieci(uint32_t mhz);
    uint32_t pobierz_zegar_graficzny(void);
    uint32_t pobierz_zegar_pamieci(void);
    uint32_t pobierz_temperature(void);
    void ustaw_predkosc_wiatraka(uint32_t obroty);
    uint32_t pobierz_predkosc_wiatraka(void);
    void ustaw_rgb(uint32_t kolor, uint8_t tryb);
    void wylacz_rgb(void);

    radeon_hd6000_handle_t lancuch_stworz(uint32_t szerokosc, uint32_t wysokosc, uint8_t vsync, uint8_t alpha_premultiplied);
    void lancuch_zniszcz(radeon_hd6000_handle_t uchwyt);
    void prezentuj(radeon_hd6000_handle_t lancuch);

    void polecenia_rozpocznij(void);
    void polecenia_zakoncz(void);
    void kontekst_pobierz(struct radeon_hd6000_context *wyjscie);
    int akceleracja_3d_dostepna(void);

private:
    struct radeon_hd6000_info dane;
    struct radeon_hd6000_vertex_buffer bufor_wierzcholkow_tab[RADEON_HD6000_MAX_VERTEX_BUFFERS];
    struct radeon_hd6000_index_buffer bufor_indeksow_tab[RADEON_HD6000_MAX_INDEX_BUFFERS];
    struct radeon_hd6000_texture tekstury_tab[RADEON_HD6000_MAX_TEXTURES];
    struct radeon_hd6000_render_target cele_tab[RADEON_HD6000_MAX_RENDER_TARGETS];
    struct radeon_hd6000_depth_stencil bufor_glebii_tab[RADEON_HD6000_MAX_DEPTH_STENCILS];
    struct radeon_hd6000_shader shadery_tab[RADEON_HD6000_MAX_SHADERS];
    struct radeon_hd6000_sampler_state probniki_tab[RADEON_HD6000_MAX_SAMPLERS];
    struct radeon_hd6000_blend_state stany_mieszania_tab[RADEON_HD6000_MAX_BLEND_STATES];
    struct radeon_hd6000_rasterizer_state stany_rasteryzacji_tab[RADEON_HD6000_MAX_RASTERIZER_STATES];
    struct radeon_hd6000_depth_stencil_state stany_glebii_tab[RADEON_HD6000_MAX_DEPTH_STENCIL_STATES];
    struct radeon_hd6000_input_layout uklady_tab[RADEON_HD6000_MAX_INPUT_LAYOUTS];
    struct radeon_hd6000_constant_buffer bufor_stale_tab[RADEON_HD6000_MAX_CONSTANT_BUFFERS];
    struct radeon_hd6000_swap_chain lancuchy_tab[RADEON_HD6000_MAX_SWAP_CHAINS];
    struct radeon_hd6000_fence ogrodzenia_tab[RADEON_HD6000_MAX_FENCES];
    struct radeon_hd6000_semaphore semafory_tab[RADEON_HD6000_MAX_SEMAPHORES];
    struct radeon_hd6000_vram_alloc alokacje_tab[RADEON_HD6000_VRAM_MAX_ALLOCS];
    struct radeon_hd6000_context kontekst;

    uint8_t inicjalizowane;
    uint32_t mmio_base;
    uint32_t pci_bus;
    uint32_t pci_slot;
    uint32_t pci_funkcja;
    uint8_t *ramka_systemowa;
    uint32_t wierzcholek_polecenia;
    uint32_t wierzcholek_pierwszy;
    uint8_t bufor_polecenia[RADEON_HD6000_COMMAND_BUFFER_SIZE];

    uint32_t odczytaj_rejestr(uint32_t rejestr);
    void zapisz_rejestr(uint32_t rejestr, uint32_t wartosc);
    static uint32_t pci_odczytaj32(uint8_t bus, uint8_t slot, uint8_t funkcja, uint8_t przesuniecie);
    static void pci_zapisz32(uint8_t bus, uint8_t slot, uint8_t funkcja, uint8_t przesuniecie, uint32_t wartosc);
    int znajdz_urzadzenie(void);
    void inicjalizuj_ring_buffer(void);
    void inicjalizuj_pamiec(void);
    void inicjalizuj_wyswietlacz(void);
    void resetuj_potok(void);
    uint64_t alokuj_vram(uint64_t rozmiar, uint32_t typ);
    void zwolnij_vram(uint64_t uchwyt);
};

RadeonHD6000 *radeon_hd6000_pobierz(void);

#endif

#ifdef __cplusplus
extern "C" {
#endif

int radeon_hd6000_init(void);
void radeon_hd6000_shutdown(void);
uint8_t radeon_hd6000_czy_gotowy(void);
void radeon_hd6000_putpixel(uint32_t x, uint32_t y, uint32_t kolor);
void radeon_hd6000_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor);
void radeon_hd6000_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void radeon_hd6000_cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz);
void radeon_hd6000_cursor_hide(void);
void *radeon_hd6000_get_fb(void);
uint32_t radeon_hd6000_get_szerokosc(void);
uint32_t radeon_hd6000_get_wysokosc(void);
uint16_t radeon_hd6000_get_device_id(void);
uint8_t radeon_hd6000_get_rewizja(void);
void radeon_hd6000_info_pobierz(struct radeon_hd6000_info *wyjscie);

uint64_t radeon_hd6000_vram_alokuj(uint64_t rozmiar, uint32_t typ);
void radeon_hd6000_vram_zwolnij(uint64_t uchwyt);
uint64_t radeon_hd6000_vram_uzyto(void);
uint64_t radeon_hd6000_vram_calkowicie(void);

radeon_hd6000_handle_t radeon_hd6000_bufor_wierzcholkow_stworz(uint32_t liczba_wierzcholkow, uint32_t rozmiar_wierzcholka, uint32_t format, const void *dane);
void radeon_hd6000_bufor_wierzcholkow_zniszcz(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_bufor_wierzcholkow_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

radeon_hd6000_handle_t radeon_hd6000_bufor_indeksow_stworz(uint32_t liczba_indeksow, uint32_t format, const void *dane);
void radeon_hd6000_bufor_indeksow_zniszcz(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_bufor_indeksow_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

radeon_hd6000_handle_t radeon_hd6000_tekstura_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint32_t poziomy_mip, const void *dane);
void radeon_hd6000_tekstura_zniszcz(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_tekstura_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t poziom, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *dane);
void radeon_hd6000_tekstura_wygeneruj_mipmapy(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_cel_renderowania_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t alpha_blend);
void radeon_hd6000_cel_renderowania_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_bufor_glebii_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t kompresja);
void radeon_hd6000_bufor_glebii_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_shader_stworz(uint32_t typ, const uint8_t *kod, uint32_t rozmiar_kodu);
void radeon_hd6000_shader_zniszcz(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_shader_skompiluj(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_stan_probnika_stworz(uint32_t filtr_min, uint32_t filtr_mag, uint32_t filtr_mip, uint32_t adres_u, uint32_t adres_v, uint32_t adres_w, uint32_t max_anizotropia, uint8_t wlacz_porownanie, uint8_t func_porownania);
void radeon_hd6000_stan_probnika_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_stan_mieszania_stworz(uint8_t wlacz_mieszanie, uint32_t src_blend, uint32_t dest_blend, uint32_t op_mieszania, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t op_mieszania_alpha, uint32_t maska_zapisu, uint32_t blend_factor);
void radeon_hd6000_stan_mieszania_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_stan_rasteryzacji_stworz(uint32_t tryb_wypelnienia, uint32_t tryb_odcinania, uint8_t front_ccw, uint8_t clip_glebii, uint8_t scissor, uint8_t wielopr_probowanie, uint8_t aa_linia, uint8_t alpha_do_pokrycia);
void radeon_hd6000_stan_rasteryzacji_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_stan_glebii_stencil_stworz(uint8_t wlacz_glebie, uint32_t maska_zapisu_glebii, uint32_t func_glebii, uint8_t wlacz_stencil, uint8_t maska_odczytu_stencil, uint8_t maska_zapisu_stencil, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void radeon_hd6000_stan_glebii_stencil_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_uklad_wejsc_stworz(uint32_t liczba_elementow, const uint32_t *sloty, const uint32_t *przesuniecia, const uint32_t *formaty);
void radeon_hd6000_uklad_wejsc_zniszcz(radeon_hd6000_handle_t uchwyt);

radeon_hd6000_handle_t radeon_hd6000_bufor_stale_stworz(uint32_t rozmiar);
void radeon_hd6000_bufor_stale_zniszcz(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_bufor_stale_zaktualizuj(radeon_hd6000_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

void radeon_hd6000_powiaz_bufor_wierzcholkow(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_bufor_indeksow(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_uklad_wejsc(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_shader_wierzcholkow(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_shader_pikseli(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_shader_geometrii(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_shader_obliczeniowy(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_cel_renderowania(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_bufor_glebii(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_stan_mieszania(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_stan_rasteryzacji(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_stan_glebii_stencil(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_powiaz_probnik_i_teksture(uint32_t gniazdo, radeon_hd6000_handle_t probnik, radeon_hd6000_handle_t tekstura);
void radeon_hd6000_powiaz_bufor_stale(uint32_t gniazdo, radeon_hd6000_handle_t uchwyt);

void radeon_hd6000_ustaw_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_glebia, float max_glebia);
void radeon_hd6000_ustaw_scissor(uint32_t lewa, uint32_t gora, uint32_t prawa, uint32_t dol);
void radeon_hd6000_ustaw_typ_pierwotnego(uint32_t typ_pierwotnego);
void radeon_hd6000_ustaw_mieszanie_alpha_globalne(uint8_t wlacz);

void radeon_hd6000_wyczysc_cel_renderowania(radeon_hd6000_handle_t cel, uint32_t kolor);
void radeon_hd6000_wyczysc_bufor_glebii(radeon_hd6000_handle_t bufor, float glebia, uint8_t stencil);
void radeon_hd6000_rysuj(uint32_t liczba_wierzcholkow, uint32_t pierwszy_wierzcholek);
void radeon_hd6000_rysuj_indeksowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka);
void radeon_hd6000_rysuj_indeksowane_instancjonowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka, uint32_t liczba_instancji, uint32_t pierwsza_instancja);
void radeon_hd6000_rysuj_po_indirekcie(uint32_t adres_bufora, uint32_t liczba_rysowan, uint32_t krok);

void radeon_hd6000_oblicz_wyslij(uint32_t grupa_x, uint32_t grupa_y, uint32_t grupa_z);
void radeon_hd6000_oblicz_wyslij_po_indirekcie(uint32_t adres_bufora);

void radeon_hd6000_ogrodzenie_stworz(radeon_hd6000_handle_t *wyjscie);
void radeon_hd6000_ogrodzenie_sygnalizuj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc);
int radeon_hd6000_ogrodzenie_poczekaj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc, uint32_t timeout_us);
void radeon_hd6000_ogrodzenie_zniszcz(radeon_hd6000_handle_t uchwyt);

void radeon_hd6000_semafor_stworz(radeon_hd6000_handle_t *wyjscie);
void radeon_hd6000_semafor_sygnalizuj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc);
void radeon_hd6000_semafor_poczekaj(radeon_hd6000_handle_t uchwyt, uint32_t wartosc);
void radeon_hd6000_semafor_zniszcz(radeon_hd6000_handle_t uchwyt);

int radeon_hd6000_czekaj_na_bezczynnosc(uint32_t timeout_us);
uint64_t radeon_hd6000_znacznik_czasu(void);

void radeon_hd6000_ustaw_stan_zasilania(uint8_t stan);
uint8_t radeon_hd6000_pobierz_stan_zasilania(void);
void radeon_hd6000_ustaw_zegar_graficzny(uint32_t mhz);
void radeon_hd6000_ustaw_zegar_pamieci(uint32_t mhz);
uint32_t radeon_hd6000_pobierz_zegar_graficzny(void);
uint32_t radeon_hd6000_pobierz_zegar_pamieci(void);
uint32_t radeon_hd6000_pobierz_temperature(void);
void radeon_hd6000_ustaw_predkosc_wiatraka(uint32_t obroty);
uint32_t radeon_hd6000_pobierz_predkosc_wiatraka(void);
void radeon_hd6000_ustaw_rgb(uint32_t kolor, uint8_t tryb);
void radeon_hd6000_wylacz_rgb(void);

radeon_hd6000_handle_t radeon_hd6000_lancuch_stworz(uint32_t szerokosc, uint32_t wysokosc, uint8_t vsync, uint8_t alpha_premultiplied);
void radeon_hd6000_lancuch_zniszcz(radeon_hd6000_handle_t uchwyt);
void radeon_hd6000_prezentuj(radeon_hd6000_handle_t lancuch);

void radeon_hd6000_polecenia_rozpocznij(void);
void radeon_hd6000_polecenia_zakoncz(void);
void radeon_hd6000_kontekst_pobierz(struct radeon_hd6000_context *wyjscie);
int radeon_hd6000_akceleracja_3d_dostepna(void);

#ifdef __cplusplus
}
#endif

#endif
