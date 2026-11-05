#ifndef RADEON_8500_H
#define RADEON_8500_H

#include "../../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RADEON_8500_VENDOR_ID 0x1002U
#define RADEON_8500_DEVICE_ID_8500 0x4C48U
#define RADEON_8500_DEVICE_ID_8500_LE 0x4C4DU
#define RADEON_8500_DEVICE_ID_8500_VAR1 0x4C56U
#define RADEON_8500_DEVICE_ID_8500_VAR2 0x4C57U
#define RADEON_8500_DEVICE_ID_8500_VAR3 0x514CU
#define RADEON_8500_DEVICE_ID_8500_VAR4 0x514EU
#define RADEON_8500_DEVICE_ID_8500_VAR5 0x514FU

#define RADEON_8500_VRAM_SIZE_BYTES ((uint64_t)64ULL * 1024ULL * 1024ULL)
#define RADEON_8500_VRAM_PAGE_SIZE 4096U
#define RADEON_8500_VRAM_MAX_ALLOCS 2048U

#define RADEON_8500_FB_WIDTH 1024U
#define RADEON_8500_FB_HEIGHT 768U
#define RADEON_8500_FB_PITCH (RADEON_8500_FB_WIDTH * 4U)
#define RADEON_8500_FB_ADDR 0xFF000000U

#define RADEON_8500_MAX_VERTEX_BUFFERS 32U
#define RADEON_8500_MAX_INDEX_BUFFERS 32U
#define RADEON_8500_MAX_TEXTURES 64U
#define RADEON_8500_MAX_RENDER_TARGETS 8U
#define RADEON_8500_MAX_DEPTH_STENCILS 8U
#define RADEON_8500_MAX_SHADERS 32U
#define RADEON_8500_MAX_COMPUTE_SHADERS 8U
#define RADEON_8500_MAX_SAMPLERS 16U
#define RADEON_8500_MAX_BLEND_STATES 16U
#define RADEON_8500_MAX_RASTERIZER_STATES 16U
#define RADEON_8500_MAX_DEPTH_STENCIL_STATES 16U
#define RADEON_8500_MAX_INPUT_LAYOUTS 32U
#define RADEON_8500_MAX_CONSTANT_BUFFERS 16U
#define RADEON_8500_MAX_SWAP_CHAINS 4U
#define RADEON_8500_MAX_FENCES 16U
#define RADEON_8500_MAX_SEMAPHORES 16U
#define RADEON_8500_COMMAND_BUFFER_SIZE 8192U
#define RADEON_8500_RING_BUFFER_SIZE 4096U

#define RADEON_8500_HANDLE_NULL 0xFFFFFFFFU
typedef uint32_t radeon_8500_handle_t;

enum radeon_8500_reg {
    REG_CP_RB_BASE = 0x0700U,
    REG_CP_RB_CNTL = 0x0704U,
    REG_CP_RB_WPTR = 0x0708U,
    REG_CP_RB_RPTR = 0x070CU,
    REG_CP_RB_RPTR_ADDR = 0x0710U,
    REG_CP_ME_CNTL = 0x08BCU,
    REG_CP_ME_RB_ADDR = 0x08C4U,
    REG_CP_ME_RB_SIZE = 0x08C8U,
    REG_CP_ME_RB_WPTR = 0x08CCU,
    REG_CP_ME_RB_RPTR = 0x08D0U,
    REG_CP_ME_RB_CNTL = 0x08D4U,
    REG_CP_PFP_RB_ADDR = 0x08E0U,
    REG_CP_PFP_RB_SIZE = 0x08E4U,
    REG_CP_PFP_RB_WPTR = 0x08E8U,
    REG_CP_PFP_RB_RPTR = 0x08ECU,
    REG_INT_STAT = 0x0100U,
    REG_INT_MASK = 0x0104U,
    REG_INT_ACK = 0x0108U,
    REG_CRTC_H_TOTAL = 0x6804U,
    REG_CRTC_V_TOTAL = 0x6808U,
    REG_CRTC_H_BLANK = 0x680CU,
    REG_CRTC_V_BLANK = 0x6810U,
    REG_CRTC_H_SYNC_A = 0x6814U,
    REG_CRTC_V_SYNC_A = 0x6818U,
    REG_CRTC_CONTROL = 0x6800U,
    REG_CRTC_PRIMARY_VIEWPORT_START = 0x6820U,
    REG_CRTC_PRIMARY_VIEWPORT_END = 0x6824U,
    REG_CRTC_PRIMARY_SURFACE_ADDRESS = 0x6830U,
    REG_CRTC_PRIMARY_SURFACE_ADDRESS_HIGH = 0x6834U,
    REG_CRTC_PRIMARY_SURFACE_OFFSET_X = 0x6838U,
    REG_CRTC_PRIMARY_SURFACE_OFFSET_Y = 0x683CU,
    REG_CRTC_PRIMARY_SURFACE_PIXEL_PITCH = 0x6844U,
    REG_CRTC_PRIMARY_SURFACE_TILE = 0x684CU,
    REG_PRIMARY_PLANE_OFFSET_X = 0x6900U,
    REG_PRIMARY_PLANE_OFFSET_Y = 0x6904U,
    REG_PRIMARY_PLANE_PIXEL_PITCH = 0x6908U,
    REG_PRIMARY_PLANE_SURFACE_ADDRESS = 0x690CU,
    REG_PRIMARY_PLANE_SURFACE_ADDRESS_HIGH = 0x6910U,
    REG_CURSOR_CONTROL = 0x7000U,
    REG_CURSOR_POSITION = 0x7004U,
    REG_CURSOR_COLOR0 = 0x7008U,
    REG_CURSOR_COLOR1 = 0x700CU,
    REG_CURSOR_BASE = 0x7010U,
    REG_CB_TARGET0_BASE = 0x8800U,
    REG_CB_TARGET0_PITCH = 0x8804U,
    REG_CB_TARGET0_SLICE = 0x8808U,
    REG_CB_TARGET0_FMASK = 0x880CU,
    REG_CB_TARGET0_VIEW = 0x8810U,
    REG_CB_SHADER_MASK = 0x88F0U,
    REG_CB_CLEAR_COLOR = 0x88F4U,
    REG_CB_CLEAR_COLOR_HIGH = 0x88F8U,
    REG_CB_BLEND_CONTROL = 0x8A50U,
    REG_CB_BLEND_RED = 0x8A54U,
    REG_CB_BLEND_GREEN = 0x8A58U,
    REG_CB_BLEND_BLUE = 0x8A5CU,
    REG_CB_BLEND_ALPHA = 0x8A60U,
    REG_DB_Z_INFO = 0x8A00U,
    REG_DB_Z_READ_BASE = 0x8A04U,
    REG_DB_Z_READ_BASE_HIGH = 0x8A08U,
    REG_DB_Z_WRITE_BASE = 0x8A0CU,
    REG_DB_Z_WRITE_BASE_HIGH = 0x8A10U,
    REG_DB_Z_READ_OFF = 0x8A14U,
    REG_DB_Z_WRITE_OFF = 0x8A18U,
    REG_DB_CLEAR = 0x8A20U,
    REG_DB_STENCIL_CLEAR = 0x8A24U,
    REG_SQ_CONFIG = 0x9800U,
    REG_SQ_VTX_BASE = 0x9810U,
    REG_SQ_VTX_STRIDE = 0x9814U,
    REG_SQ_VTX_NUM_FORMAT = 0x9818U,
    REG_SQ_VTX_FMT_0 = 0x9820U,
    REG_SQ_VTX_FMT_1 = 0x9824U,
    REG_SQ_VTX_FMT_2 = 0x9828U,
    REG_SQ_VTX_FMT_3 = 0x982CU,
    REG_SQ_PROGRAM = 0x9860U,
    REG_SQ_CONSTANT_BASE = 0x98A0U,
    REG_SQ_CONSTANT_CACHE = 0x98A4U,
    REG_SQ_COMPUTE_PROGRAM = 0x98B0U,
    REG_SQ_TEX_BASE = 0x9C00U,
    REG_SQ_SAMPLER_BASE = 0x9C80U,
    REG_SQ_TEX_LOD = 0x9D00U,
    REG_SQ_TEX_DIM = 0x9D80U,
    REG_SQ_FEEDBACK_BASE = 0x9E00U,
    REG_PA_SU_VTX_CNTL = 0xCA00U,
    REG_PA_CL_VTE_CNTL = 0xCA04U,
    REG_PA_CL_VS_OUT_CNTL = 0xCA08U,
    REG_PA_CL_GB_ADV = 0xCA0CU,
    REG_PA_CL_VTE_VTX_0 = 0xCA10U,
    REG_PA_SU_POLY_OFFSET_FRONT = 0xCA30U,
    REG_PA_SU_POLY_OFFSET_BACK = 0xCA34U,
    REG_PA_CL_CLIP_CNTL = 0xCA40U,
    REG_PA_SU_SC_MODE_CNTL = 0xCA44U,
    REG_PA_SC_WINDOW_OFFSET = 0xCA48U,
    REG_PA_SC_WINDOW_SCISSOR_TL = 0xCA4CU,
    REG_PA_SC_WINDOW_SCISSOR_BR = 0xCA50U,
    REG_PA_SU_PRIM_FILTER_CNTL = 0xCA54U,
    REG_PA_SU_LINE_STIPPLE = 0xCA58U,
    REG_PA_SC_AA_CONFIG = 0xCA5CU,
    REG_CB_ALPHA_TO_MASK = 0xCA60U,
    REG_CB_ALPHA_TO_MASK_DWORD0 = 0xCA64U,
    REG_CB_ALPHA_TO_MASK_DWORD1 = 0xCA68U,
    REG_CB_ALPHA_TO_MASK_DWORD2 = 0xCA6CU,
    REG_CB_ALPHA_TO_MASK_DWORD3 = 0xCA70U,
    REG_TIMESTAMP_LO = 0x1030U,
    REG_TIMESTAMP_HI = 0x1034U,
    REG_POWER_STATE = 0x1010U,
    REG_THERMAL_TEMP = 0x1014U,
    REG_FAN_PWM = 0x1018U,
    REG_FAN_RPM = 0x101CU,
    REG_RGB_CONTROL = 0x1020U,
    REG_RGB_COLOR = 0x1024U,
    REG_VRAM_USAGE = 0x1028U,
    REG_VRAM_TOTAL = 0x102CU,
    REG_CLOCK_GFX = 0x1040U,
    REG_CLOCK_MEMORY = 0x1044U
};

#define CP_ME_CNTL_ME_HALT (1U << 0)
#define CP_ME_CNTL_ME_RESET (1U << 1)
#define CP_ME_CNTL_PFP_HALT (1U << 8)
#define CP_ME_CNTL_PFP_RESET (1U << 9)
#define CP_ME_RB_CNTL_RB_ENABLE (1U << 0)
#define CP_ME_RB_CNTL_RB_NO_UPDATE (1U << 1)

#define CRTC_CONTROL_EN (1U << 0)
#define CRTC_CONTROL_PIXEL_WIDTH_32BPP (3U << 2)

#define BLEND_CONTROL_SRC_BLEND_SHIFT 0U
#define BLEND_CONTROL_DEST_BLEND_SHIFT 8U
#define BLEND_CONTROL_BLEND_OP_SHIFT 16U
#define BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT 20U
#define BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT 24U
#define BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT 28U
#define BLEND_CONTROL_ROP_ENABLE (1U << 31U)

#define SHADER_MASK_ENABLE (1U << 0)
#define SHADER_MASK_CB0_ENABLE (1U << 4)

#define DB_Z_INFO_FORMAT_SHIFT 0U
#define DB_Z_INFO_FORMAT_16BIT 0U
#define DB_Z_INFO_FORMAT_24BIT 1U
#define DB_Z_INFO_FORMAT_32BIT 2U
#define DB_Z_INFO_COMPRESSION_ENABLE (1U << 8)

#define POWER_STATE_D0 0U
#define POWER_STATE_D1 1U
#define POWER_STATE_D2 2U
#define POWER_STATE_D3 3U

#define RADEON_8500_VERTEX_FORMAT_XYZ 0x0001U
#define RADEON_8500_VERTEX_FORMAT_XYZW 0x0002U
#define RADEON_8500_VERTEX_FORMAT_XYZ_COLOR 0x0004U
#define RADEON_8500_VERTEX_FORMAT_XYZ_TEX 0x0008U
#define RADEON_8500_VERTEX_FORMAT_XYZ_COLOR_TEX 0x0010U
#define RADEON_8500_VERTEX_FORMAT_XYZ_COLOR_TEX2 0x0020U
#define RADEON_8500_VERTEX_FORMAT_XYZ_COLOR_TEX_NORMAL 0x0040U

#define RADEON_8500_INDEX_FORMAT_16BIT 0x0000U
#define RADEON_8500_INDEX_FORMAT_32BIT 0x0001U

#define RADEON_8500_PRIM_POINTLIST 0x0000U
#define RADEON_8500_PRIM_LINELIST 0x0001U
#define RADEON_8500_PRIM_LINESTRIP 0x0002U
#define RADEON_8500_PRIM_TRIANGLELIST 0x0003U
#define RADEON_8500_PRIM_TRIANGLESTRIP 0x0004U
#define RADEON_8500_PRIM_TRIANGLEFAN 0x0005U
#define RADEON_8500_PRIM_QUADLIST 0x0006U
#define RADEON_8500_PRIM_QUADSTRIP 0x0007U

#define RADEON_8500_TEX_FORMAT_A8R8G8B8 0x0000U
#define RADEON_8500_TEX_FORMAT_X8R8G8B8 0x0001U
#define RADEON_8500_TEX_FORMAT_A8B8G8R8 0x0002U
#define RADEON_8500_TEX_FORMAT_X8B8G8R8 0x0003U
#define RADEON_8500_TEX_FORMAT_R5G6B5 0x0004U
#define RADEON_8500_TEX_FORMAT_DXT1 0x0010U
#define RADEON_8500_TEX_FORMAT_DXT3 0x0011U
#define RADEON_8500_TEX_FORMAT_DXT5 0x0012U
#define RADEON_8500_TEX_FORMAT_FLOAT32_RGBA 0x0020U
#define RADEON_8500_TEX_FORMAT_FLOAT16_RGBA 0x0021U
#define RADEON_8500_TEX_FORMAT_ATI1N 0x0030U
#define RADEON_8500_TEX_FORMAT_ATI2N 0x0031U

#define RADEON_8500_SHADER_VERTEX 0x0000U
#define RADEON_8500_SHADER_PIXEL 0x0001U
#define RADEON_8500_SHADER_GEOMETRY 0x0002U
#define RADEON_8500_SHADER_COMPUTE 0x0003U

#define RADEON_8500_BLEND_ZERO 0x0000U
#define RADEON_8500_BLEND_ONE 0x0001U
#define RADEON_8500_BLEND_SRC_COLOR 0x0002U
#define RADEON_8500_BLEND_DEST_COLOR 0x0003U
#define RADEON_8500_BLEND_SRC_ALPHA 0x0004U
#define RADEON_8500_BLEND_DEST_ALPHA 0x0005U
#define RADEON_8500_BLEND_INV_SRC_ALPHA 0x0006U
#define RADEON_8500_BLEND_INV_DEST_ALPHA 0x0007U
#define RADEON_8500_BLEND_SRC_ALPHA_SAT 0x0008U
#define RADEON_8500_BLEND_BLEND_FACTOR 0x0009U
#define RADEON_8500_BLEND_INV_BLEND_FACTOR 0x000AU

#define RADEON_8500_BLEND_OP_ADD 0x0000U
#define RADEON_8500_BLEND_OP_SUBTRACT 0x0001U
#define RADEON_8500_BLEND_OP_REV_SUBTRACT 0x0002U
#define RADEON_8500_BLEND_OP_MIN 0x0003U
#define RADEON_8500_BLEND_OP_MAX 0x0004U

#define RADEON_8500_CMP_NEVER 0x0000U
#define RADEON_8500_CMP_LESS 0x0001U
#define RADEON_8500_CMP_EQUAL 0x0002U
#define RADEON_8500_CMP_LEQUAL 0x0003U
#define RADEON_8500_CMP_GREATER 0x0004U
#define RADEON_8500_CMP_NOTEQUAL 0x0005U
#define RADEON_8500_CMP_GEQUAL 0x0006U
#define RADEON_8500_CMP_ALWAYS 0x0007U

#define RADEON_8500_FILL_SOLID 0x0000U
#define RADEON_8500_FILL_WIREFRAME 0x0001U

#define RADEON_8500_CULL_NONE 0x0000U
#define RADEON_8500_CULL_FRONT 0x0001U
#define RADEON_8500_CULL_BACK 0x0002U

#define RADEON_8500_STENCIL_KEEP 0x0000U
#define RADEON_8500_STENCIL_ZERO 0x0001U
#define RADEON_8500_STENCIL_REPLACE 0x0002U
#define RADEON_8500_STENCIL_INCR 0x0003U
#define RADEON_8500_STENCIL_DECR 0x0004U
#define RADEON_8500_STENCIL_INVERT 0x0005U

struct radeon_8500_info {
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

struct radeon_8500_vertex_buffer {
    uint8_t used;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct radeon_8500_index_buffer {
    uint8_t used;
    uint32_t index_count;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct radeon_8500_texture {
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

struct radeon_8500_render_target {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t alpha_blend;
    uint8_t dcc;
    uint64_t vram_offset;
};

struct radeon_8500_depth_stencil {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t compression;
    uint64_t vram_offset;
};

struct radeon_8500_shader {
    uint8_t used;
    uint32_t type;
    uint32_t code_size;
    uint8_t *code;
    uint32_t constant_count;
    uint8_t compiled;
};

struct radeon_8500_sampler_state {
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

struct radeon_8500_blend_state {
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

struct radeon_8500_rasterizer_state {
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

struct radeon_8500_depth_stencil_state {
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

struct radeon_8500_input_layout {
    uint8_t used;
    uint32_t element_count;
    uint32_t slot[32];
    uint32_t offset[32];
    uint32_t format[32];
};

struct radeon_8500_constant_buffer {
    uint8_t used;
    uint32_t size;
    uint8_t *data;
    uint64_t vram_offset;
};

struct radeon_8500_swap_chain {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t back_buffer_addr;
    uint32_t front_buffer_addr;
    uint8_t vsync;
    uint8_t alpha_premultiplied;
    uint8_t triple_buffer;
    uint64_t back_vram_offset;
    uint64_t front_vram_offset;
};

struct radeon_8500_fence {
    uint8_t used;
    uint32_t value;
    uint8_t signaled;
};

struct radeon_8500_semaphore {
    uint8_t used;
    uint32_t value;
};

struct radeon_8500_vram_alloc {
    uint8_t used;
    uint64_t offset;
    uint64_t size;
    uint32_t type;
};

struct radeon_8500_context {
    radeon_8500_handle_t vertex_buffer;
    radeon_8500_handle_t index_buffer;
    radeon_8500_handle_t input_layout;
    radeon_8500_handle_t vertex_shader;
    radeon_8500_handle_t pixel_shader;
    radeon_8500_handle_t geometry_shader;
    radeon_8500_handle_t compute_shader;
    radeon_8500_handle_t render_target;
    radeon_8500_handle_t depth_stencil;
    radeon_8500_handle_t blend_state;
    radeon_8500_handle_t rasterizer_state;
    radeon_8500_handle_t depth_stencil_state;
    radeon_8500_handle_t sampler[RADEON_8500_MAX_SAMPLERS];
    radeon_8500_handle_t texture[RADEON_8500_MAX_SAMPLERS];
    radeon_8500_handle_t constant_buffer[RADEON_8500_MAX_CONSTANT_BUFFERS];
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
};

int radeon_8500_init(void);
int radeon_8500_is_ready(void);
void radeon_8500_shutdown(void);

void radeon_8500_putpixel(uint32_t x, uint32_t y, uint32_t color);
void radeon_8500_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void radeon_8500_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void radeon_8500_cursor_set(uint32_t x, uint32_t y, const uint32_t *image);
void radeon_8500_cursor_hide(void);
void *radeon_8500_get_fb(void);
uint32_t radeon_8500_get_width(void);
uint32_t radeon_8500_get_height(void);
uint16_t radeon_8500_get_device_id(void);
uint8_t radeon_8500_get_revision(void);

void radeon_8500_info_get(struct radeon_8500_info *out);

uint64_t radeon_8500_vram_alloc(uint64_t size, uint32_t type);
void radeon_8500_vram_free(uint64_t handle);
uint64_t radeon_8500_vram_used(void);
uint64_t radeon_8500_vram_total(void);

radeon_8500_handle_t radeon_8500_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data);
void radeon_8500_vertex_buffer_destroy(radeon_8500_handle_t handle);
void radeon_8500_vertex_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_8500_handle_t radeon_8500_index_buffer_create(uint32_t index_count, uint32_t format, const void *data);
void radeon_8500_index_buffer_destroy(radeon_8500_handle_t handle);
void radeon_8500_index_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_8500_handle_t radeon_8500_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data);
void radeon_8500_texture_destroy(radeon_8500_handle_t handle);
void radeon_8500_texture_update(radeon_8500_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
void radeon_8500_texture_generate_mipmaps(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend);
void radeon_8500_render_target_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression);
void radeon_8500_depth_stencil_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void radeon_8500_shader_destroy(radeon_8500_handle_t handle);
void radeon_8500_shader_compile(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable, uint8_t compare_func);
void radeon_8500_sampler_state_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor);
void radeon_8500_blend_state_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage);
void radeon_8500_rasterizer_state_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void radeon_8500_depth_stencil_state_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void radeon_8500_input_layout_destroy(radeon_8500_handle_t handle);

radeon_8500_handle_t radeon_8500_constant_buffer_create(uint32_t size);
void radeon_8500_constant_buffer_destroy(radeon_8500_handle_t handle);
void radeon_8500_constant_buffer_update(radeon_8500_handle_t handle, uint32_t offset, uint32_t size, const void *data);

void radeon_8500_bind_vertex_buffer(radeon_8500_handle_t handle);
void radeon_8500_bind_index_buffer(radeon_8500_handle_t handle);
void radeon_8500_bind_input_layout(radeon_8500_handle_t handle);
void radeon_8500_bind_vertex_shader(radeon_8500_handle_t handle);
void radeon_8500_bind_pixel_shader(radeon_8500_handle_t handle);
void radeon_8500_bind_geometry_shader(radeon_8500_handle_t handle);
void radeon_8500_bind_compute_shader(radeon_8500_handle_t handle);
void radeon_8500_bind_render_target(radeon_8500_handle_t handle);
void radeon_8500_bind_depth_stencil(radeon_8500_handle_t handle);
void radeon_8500_bind_blend_state(radeon_8500_handle_t handle);
void radeon_8500_bind_rasterizer_state(radeon_8500_handle_t handle);
void radeon_8500_bind_depth_stencil_state(radeon_8500_handle_t handle);
void radeon_8500_bind_sampler_and_texture(uint32_t slot, radeon_8500_handle_t sampler, radeon_8500_handle_t texture);
void radeon_8500_bind_constant_buffer(uint32_t slot, radeon_8500_handle_t handle);

void radeon_8500_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
void radeon_8500_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
void radeon_8500_set_primitive_topology(uint32_t topology);
void radeon_8500_set_alpha_blend_global(uint8_t enable);

void radeon_8500_clear_render_target(radeon_8500_handle_t rt, uint32_t color);
void radeon_8500_clear_depth_stencil(radeon_8500_handle_t ds, float depth, uint8_t stencil);
void radeon_8500_draw(uint32_t vertex_count, uint32_t start_vertex);
void radeon_8500_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex);
void radeon_8500_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance);
void radeon_8500_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride);

void radeon_8500_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z);
void radeon_8500_compute_dispatch_indirect(uint32_t buffer_addr);

void radeon_8500_fence_create(radeon_8500_handle_t *out);
void radeon_8500_fence_signal(radeon_8500_handle_t handle, uint32_t value);
int radeon_8500_fence_wait(radeon_8500_handle_t handle, uint32_t value, uint32_t timeout_us);
void radeon_8500_fence_destroy(radeon_8500_handle_t handle);

void radeon_8500_semaphore_create(radeon_8500_handle_t *out);
void radeon_8500_semaphore_signal(radeon_8500_handle_t handle, uint32_t value);
void radeon_8500_semaphore_wait(radeon_8500_handle_t handle, uint32_t value);
void radeon_8500_semaphore_destroy(radeon_8500_handle_t handle);

int radeon_8500_wait_idle(uint32_t timeout_us);
uint64_t radeon_8500_timestamp(void);

void radeon_8500_power_set_state(uint8_t state);
uint8_t radeon_8500_power_get_state(void);
void radeon_8500_set_clock_gfx(uint32_t mhz);
void radeon_8500_set_clock_memory(uint32_t mhz);
uint32_t radeon_8500_get_clock_gfx(void);
uint32_t radeon_8500_get_clock_memory(void);
uint32_t radeon_8500_get_temperature(void);
void radeon_8500_set_fan_speed(uint32_t rpm);
uint32_t radeon_8500_get_fan_speed(void);
void radeon_8500_rgb_set(uint32_t color, uint8_t mode);
void radeon_8500_rgb_off(void);

radeon_8500_handle_t radeon_8500_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied, uint8_t triple_buffer);
void radeon_8500_swap_chain_destroy(radeon_8500_handle_t handle);
void radeon_8500_present(radeon_8500_handle_t chain);

void radeon_8500_command_begin(void);
void radeon_8500_command_end(void);

void radeon_8500_context_get(struct radeon_8500_context *out);
int radeon_8500_3d_available(void);

#ifdef __cplusplus
}
#endif

#endif
