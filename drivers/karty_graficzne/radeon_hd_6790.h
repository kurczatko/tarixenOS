#ifndef RADEON_HD_6790_H
#define RADEON_HD_6790_H

#include "../../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RADEON_6790_VENDOR_ID 0x1002U
#define RADEON_6790_DEVICE_ID_BARTS_PRO 0x683FU
#define RADEON_6790_DEVICE_ID_BARTS_XT 0x6840U
#define RADEON_6790_DEVICE_ID_BARTS_PRO_VAR1 0x683EU
#define RADEON_6790_DEVICE_ID_BARTS_PRO_VAR2 0x6841U

#define RADEON_6790_VRAM_SIZE_BYTES ((uint64_t)1ULL * 1024ULL * 1024ULL * 1024ULL)
#define RADEON_6790_VRAM_PAGE_SIZE 4096U
#define RADEON_6790_VRAM_MAX_ALLOCS 8192U

#define RADEON_6790_FB_WIDTH 1920U
#define RADEON_6790_FB_HEIGHT 1080U
#define RADEON_6790_FB_PITCH (RADEON_6790_FB_WIDTH * 4U)
#define RADEON_6790_FB_ADDR 0xFE000000U

#define RADEON_6790_MAX_VERTEX_BUFFERS 128U
#define RADEON_6790_MAX_INDEX_BUFFERS 128U
#define RADEON_6790_MAX_TEXTURES 256U
#define RADEON_6790_MAX_RENDER_TARGETS 16U
#define RADEON_6790_MAX_DEPTH_STENCILS 16U
#define RADEON_6790_MAX_SHADERS 128U
#define RADEON_6790_MAX_COMPUTE_SHADERS 32U
#define RADEON_6790_MAX_SAMPLERS 32U
#define RADEON_6790_MAX_BLEND_STATES 32U
#define RADEON_6790_MAX_RASTERIZER_STATES 32U
#define RADEON_6790_MAX_DEPTH_STENCIL_STATES 32U
#define RADEON_6790_MAX_INPUT_LAYOUTS 64U
#define RADEON_6790_MAX_CONSTANT_BUFFERS 32U
#define RADEON_6790_MAX_STREAM_OUTPUT 16U
#define RADEON_6790_MAX_OCCLUSION_QUERIES 32U
#define RADEON_6790_MAX_PIPELINE_STATS 8U
#define RADEON_6790_MAX_TESSELLATION 16U
#define RADEON_6790_MAX_SWAP_CHAINS 8U
#define RADEON_6790_MAX_FENCES 32U
#define RADEON_6790_MAX_SEMAPHORES 32U
#define RADEON_6790_COMMAND_BUFFER_SIZE 65536U
#define RADEON_6790_RING_BUFFER_SIZE 16384U
#define RADEON_6790_DMA_RING_SIZE 16384U

#define RADEON_6790_HANDLE_NULL 0xFFFFFFFFU
typedef uint32_t radeon_6790_handle_t;

enum radeon_6790_reg {
    REG_6790_CP_ME_CNTL = 0x08BCU,
    REG_6790_CP_ME_RB_ADDR = 0x08C4U,
    REG_6790_CP_ME_RB_SIZE = 0x08C8U,
    REG_6790_CP_ME_RB_WPTR = 0x08CCU,
    REG_6790_CP_ME_RB_RPTR = 0x08D0U,
    REG_6790_CP_ME_RB_CNTL = 0x08D4U,
    REG_6790_CP_PFP_CNTL = 0x08B0U,
    REG_6790_INT_STAT = 0x0100U,
    REG_6790_INT_MASK = 0x0104U,
    REG_6790_INT_ACK = 0x0108U,
    REG_6790_GFX_CLOCK_CNTL = 0x1000U,
    REG_6790_MEMORY_CLOCK_CNTL = 0x1004U,
    REG_6790_DISPLAY_CLOCK_CNTL = 0x1008U,
    REG_6790_POWER_STATE = 0x1010U,
    REG_6790_THERMAL_TEMP = 0x1014U,
    REG_6790_FAN_PWM = 0x1018U,
    REG_6790_FAN_RPM = 0x101CU,
    REG_6790_RGB_CONTROL = 0x1020U,
    REG_6790_RGB_COLOR = 0x1024U,
    REG_6790_VRAM_USAGE = 0x1028U,
    REG_6790_VRAM_TOTAL = 0x102CU,
    REG_6790_TIMESTAMP_LO = 0x1030U,
    REG_6790_TIMESTAMP_HI = 0x1034U,
    REG_6790_DCN_DISP_CONTROL = 0x2000U,
    REG_6790_DCN_CRTC_CONTROL = 0x2004U,
    REG_6790_DCN_CRTC_H_TOTAL = 0x2008U,
    REG_6790_DCN_CRTC_V_TOTAL = 0x200CU,
    REG_6790_DCN_CRTC_H_BLANK = 0x2010U,
    REG_6790_DCN_CRTC_V_BLANK = 0x2014U,
    REG_6790_DCN_CRTC_H_SYNC_A = 0x2018U,
    REG_6790_DCN_CRTC_V_SYNC_A = 0x201CU,
    REG_6790_DCN_CRTC_PRIMARY_VIEWPORT_START = 0x2020U,
    REG_6790_DCN_CRTC_PRIMARY_VIEWPORT_END = 0x2024U,
    REG_6790_DCN_CRTC_PRIMARY_SURFACE_ADDRESS = 0x2028U,
    REG_6790_DCN_CRTC_PRIMARY_SURFACE_ADDRESS_HIGH = 0x202CU,
    REG_6790_DCN_CRTC_PRIMARY_SURFACE_OFFSET_X = 0x2030U,
    REG_6790_DCN_CRTC_PRIMARY_SURFACE_OFFSET_Y = 0x2034U,
    REG_6790_DCN_CRTC_PRIMARY_SURFACE_PIXEL_PITCH = 0x2038U,
    REG_6790_DCN_CRTC_PRIMARY_SURFACE_TILE = 0x203CU,
    REG_6790_DCN_PLANE_OFFSET_X = 0x2040U,
    REG_6790_DCN_PLANE_OFFSET_Y = 0x2044U,
    REG_6790_DCN_PLANE_PIXEL_PITCH = 0x2048U,
    REG_6790_DCN_PLANE_SURFACE_ADDRESS = 0x204CU,
    REG_6790_DCN_PLANE_SURFACE_ADDRESS_HIGH = 0x2050U,
    REG_6790_CURSOR_CONTROL = 0x2100U,
    REG_6790_CURSOR_POSITION = 0x2104U,
    REG_6790_CURSOR_COLOR0 = 0x2108U,
    REG_6790_CURSOR_COLOR1 = 0x210CU,
    REG_6790_CURSOR_BASE = 0x2110U,
    REG_6790_CB_TARGET0_BASE = 0x8800U,
    REG_6790_CB_TARGET0_PITCH = 0x8804U,
    REG_6790_CB_TARGET0_SLICE = 0x8808U,
    REG_6790_CB_TARGET0_FMASK = 0x880CU,
    REG_6790_CB_TARGET0_VIEW = 0x8810U,
    REG_6790_CB_SHADER_MASK = 0x88F0U,
    REG_6790_CB_CLEAR_COLOR = 0x88F4U,
    REG_6790_CB_CLEAR_COLOR_HIGH = 0x88F8U,
    REG_6790_CB_BLEND_CONTROL = 0x8A50U,
    REG_6790_CB_BLEND_RED = 0x8A54U,
    REG_6790_CB_BLEND_GREEN = 0x8A58U,
    REG_6790_CB_BLEND_BLUE = 0x8A5CU,
    REG_6790_CB_BLEND_ALPHA = 0x8A60U,
    REG_6790_DB_Z_INFO = 0x8A00U,
    REG_6790_DB_Z_READ_BASE = 0x8A04U,
    REG_6790_DB_Z_READ_BASE_HIGH = 0x8A08U,
    REG_6790_DB_Z_WRITE_BASE = 0x8A0CU,
    REG_6790_DB_Z_WRITE_BASE_HIGH = 0x8A10U,
    REG_6790_DB_Z_READ_OFF = 0x8A14U,
    REG_6790_DB_Z_WRITE_OFF = 0x8A18U,
    REG_6790_DB_CLEAR = 0x8A20U,
    REG_6790_DB_STENCIL_CLEAR = 0x8A24U,
    REG_6790_DB_DEPTH_BOUNDS_MIN = 0x8A28U,
    REG_6790_DB_DEPTH_BOUNDS_MAX = 0x8A2CU,
    REG_6790_DB_DEPTH_RANGE = 0x8A30U,
    REG_6790_SQ_CONFIG = 0x9800U,
    REG_6790_SQ_VTX_BASE = 0x9810U,
    REG_6790_SQ_VTX_STRIDE = 0x9814U,
    REG_6790_SQ_VTX_NUM_FORMAT = 0x9818U,
    REG_6790_SQ_VTX_FMT_0 = 0x9820U,
    REG_6790_SQ_VTX_FMT_1 = 0x9824U,
    REG_6790_SQ_VTX_FMT_2 = 0x9828U,
    REG_6790_SQ_VTX_FMT_3 = 0x982CU,
    REG_6790_SQ_PROGRAM = 0x9860U,
    REG_6790_SQ_CONSTANT_BASE = 0x98A0U,
    REG_6790_SQ_CONSTANT_CACHE = 0x98A4U,
    REG_6790_SQ_COMPUTE_PROGRAM = 0x98B0U,
    REG_6790_SQ_TEX_BASE = 0x9C00U,
    REG_6790_SQ_SAMPLER_BASE = 0x9C80U,
    REG_6790_SQ_TEX_LOD = 0x9D00U,
    REG_6790_SQ_TEX_DIM = 0x9D80U,
    REG_6790_SQ_STREAMOUT_BASE = 0x9E00U,
    REG_6790_SQ_STREAMOUT_SIZE = 0x9E04U,
    REG_6790_PA_SU_VTX_CNTL = 0xCA00U,
    REG_6790_PA_CL_VTE_CNTL = 0xCA04U,
    REG_6790_PA_CL_VS_OUT_CNTL = 0xCA08U,
    REG_6790_PA_CL_GB_ADV = 0xCA0CU,
    REG_6790_PA_CL_CLIP_CNTL = 0xCA40U,
    REG_6790_PA_SU_SC_MODE_CNTL = 0xCA44U,
    REG_6790_PA_SC_WINDOW_OFFSET = 0xCA48U,
    REG_6790_PA_SC_WINDOW_SCISSOR_TL = 0xCA4CU,
    REG_6790_PA_SC_WINDOW_SCISSOR_BR = 0xCA50U,
    REG_6790_PA_SU_PRIM_FILTER_CNTL = 0xCA54U,
    REG_6790_PA_SU_LINE_STIPPLE = 0xCA58U,
    REG_6790_PA_SC_AA_CONFIG = 0xCA5CU,
    REG_6790_PA_SU_POLY_OFFSET_FRONT = 0xCA30U,
    REG_6790_PA_SU_POLY_OFFSET_BACK = 0xCA34U,
    REG_6790_VGT_HOS_MAX_TESS_FACTOR = 0x89B4U,
    REG_6790_VGT_HOS_MIN_TESS_FACTOR = 0x89B8U,
    REG_6790_VGT_LS_HS_CONFIG = 0x89B0U,
    REG_6790_VGT_ES_PER_GS = 0x89F0U,
    REG_6790_VGT_GS_PER_VS = 0x89ECU,
    REG_6790_VGT_VTX_CNTL = 0x89E0U,
    REG_6790_VM_CONTEXT = 0xCC00U,
    REG_6790_VM_PAGE_TABLE = 0xCC04U,
    REG_6790_DMA_RB_ADDR = 0x3400U,
    REG_6790_DMA_RB_SIZE = 0x3404U,
    REG_6790_DMA_RB_WPTR = 0x3408U,
    REG_6790_DMA_RB_RPTR = 0x340CU,
    REG_6790_DMA_RB_CNTL = 0x3410U,
    REG_6790_DMA_SRC_ADDR = 0x3420U,
    REG_6790_DMA_DST_ADDR = 0x3424U,
    REG_6790_DMA_CMD = 0x3428U,
    REG_6790_DMA_STATUS = 0x342CU,
    REG_6790_SQ_PERF_COUNTER_SEL_0 = 0xD000U,
    REG_6790_SQ_PERF_COUNTER_SEL_1 = 0xD004U,
    REG_6790_SQ_PERF_COUNTER_SEL_2 = 0xD008U,
    REG_6790_SQ_PERF_COUNTER_SEL_3 = 0xD00CU,
    REG_6790_OCCLUSION_QUERY_CNTL = 0x8F00U,
    REG_6790_OCCLUSION_QUERY_RESULT = 0x8F04U
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

#define RADEON_6790_VERTEX_FORMAT_XYZ 0x0001U
#define RADEON_6790_VERTEX_FORMAT_XYZW 0x0002U
#define RADEON_6790_VERTEX_FORMAT_XYZ_COLOR 0x0004U
#define RADEON_6790_VERTEX_FORMAT_XYZ_TEX 0x0008U
#define RADEON_6790_VERTEX_FORMAT_XYZ_COLOR_TEX 0x0010U
#define RADEON_6790_VERTEX_FORMAT_XYZ_COLOR_TEX2 0x0020U

#define RADEON_6790_INDEX_FORMAT_16BIT 0x0000U
#define RADEON_6790_INDEX_FORMAT_32BIT 0x0001U

#define RADEON_6790_PRIM_POINTLIST 0x0000U
#define RADEON_6790_PRIM_LINELIST 0x0001U
#define RADEON_6790_PRIM_LINESTRIP 0x0002U
#define RADEON_6790_PRIM_TRIANGLELIST 0x0003U
#define RADEON_6790_PRIM_TRIANGLESTRIP 0x0004U
#define RADEON_6790_PRIM_TRIANGLEFAN 0x0005U
#define RADEON_6790_PRIM_QUADLIST 0x0006U
#define RADEON_6790_PRIM_QUADSTRIP 0x0007U

#define RADEON_6790_TEX_FORMAT_A8R8G8B8 0x0000U
#define RADEON_6790_TEX_FORMAT_X8R8G8B8 0x0001U
#define RADEON_6790_TEX_FORMAT_A8B8G8R8 0x0002U
#define RADEON_6790_TEX_FORMAT_X8B8G8R8 0x0003U
#define RADEON_6790_TEX_FORMAT_R5G6B5 0x0004U
#define RADEON_6790_TEX_FORMAT_DXT1 0x0010U
#define RADEON_6790_TEX_FORMAT_DXT3 0x0011U
#define RADEON_6790_TEX_FORMAT_DXT5 0x0012U
#define RADEON_6790_TEX_FORMAT_FLOAT32_RGBA 0x0020U

#define RADEON_6790_SHADER_VERTEX 0x0000U
#define RADEON_6790_SHADER_PIXEL 0x0001U
#define RADEON_6790_SHADER_GEOMETRY 0x0002U
#define RADEON_6790_SHADER_COMPUTE 0x0003U
#define RADEON_6790_SHADER_HULL 0x0004U
#define RADEON_6790_SHADER_DOMAIN 0x0005U

#define RADEON_6790_BLEND_ZERO 0x0000U
#define RADEON_6790_BLEND_ONE 0x0001U
#define RADEON_6790_BLEND_SRC_COLOR 0x0002U
#define RADEON_6790_BLEND_DEST_COLOR 0x0003U
#define RADEON_6790_BLEND_SRC_ALPHA 0x0004U
#define RADEON_6790_BLEND_DEST_ALPHA 0x0005U
#define RADEON_6790_BLEND_INV_SRC_ALPHA 0x0006U
#define RADEON_6790_BLEND_INV_DEST_ALPHA 0x0007U
#define RADEON_6790_BLEND_SRC_ALPHA_SAT 0x0008U
#define RADEON_6790_BLEND_BLEND_FACTOR 0x0009U
#define RADEON_6790_BLEND_INV_BLEND_FACTOR 0x000AU

#define RADEON_6790_BLEND_OP_ADD 0x0000U
#define RADEON_6790_BLEND_OP_SUBTRACT 0x0001U
#define RADEON_6790_BLEND_OP_REV_SUBTRACT 0x0002U
#define RADEON_6790_BLEND_OP_MIN 0x0003U
#define RADEON_6790_BLEND_OP_MAX 0x0004U

#define RADEON_6790_CMP_NEVER 0x0000U
#define RADEON_6790_CMP_LESS 0x0001U
#define RADEON_6790_CMP_EQUAL 0x0002U
#define RADEON_6790_CMP_LEQUAL 0x0003U
#define RADEON_6790_CMP_GREATER 0x0004U
#define RADEON_6790_CMP_NOTEQUAL 0x0005U
#define RADEON_6790_CMP_GEQUAL 0x0006U
#define RADEON_6790_CMP_ALWAYS 0x0007U

#define RADEON_6790_FILL_SOLID 0x0000U
#define RADEON_6790_FILL_WIREFRAME 0x0001U

#define RADEON_6790_CULL_NONE 0x0000U
#define RADEON_6790_CULL_FRONT 0x0001U
#define RADEON_6790_CULL_BACK 0x0002U

#define RADEON_6790_STENCIL_KEEP 0x0000U
#define RADEON_6790_STENCIL_ZERO 0x0001U
#define RADEON_6790_STENCIL_REPLACE 0x0002U
#define RADEON_6790_STENCIL_INCR 0x0003U
#define RADEON_6790_STENCIL_DECR 0x0004U
#define RADEON_6790_STENCIL_INVERT 0x0005U

#define RADEON_6790_TEX_FILTER_POINT 0x0000U
#define RADEON_6790_TEX_FILTER_LINEAR 0x0001U
#define RADEON_6790_TEX_FILTER_ANISOTROPIC 0x0002U

#define RADEON_6790_TEX_ADDR_WRAP 0x0000U
#define RADEON_6790_TEX_ADDR_CLAMP 0x0001U
#define RADEON_6790_TEX_ADDR_MIRROR 0x0002U
#define RADEON_6790_TEX_ADDR_BORDER 0x0003U

struct radeon_6790_info {
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

struct radeon_6790_vertex_buffer {
    uint8_t used;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct radeon_6790_index_buffer {
    uint8_t used;
    uint32_t index_count;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct radeon_6790_texture {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t mip_levels;
    uint32_t array_size;
    uint32_t sample_count;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint32_t pitch;
    uint64_t vram_offset;
    uint8_t compressed;
    uint8_t generate_mipmaps;
};

struct radeon_6790_render_target {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t array_size;
    uint32_t sample_count;
    uint32_t buffer_addr;
    uint8_t alpha_blend;
    uint64_t vram_offset;
};

struct radeon_6790_depth_stencil {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t array_size;
    uint32_t sample_count;
    uint32_t buffer_addr;
    uint8_t compression;
    uint8_t depth_bounds;
    uint64_t vram_offset;
};

struct radeon_6790_shader {
    uint8_t used;
    uint32_t type;
    uint32_t code_size;
    uint8_t *code;
    uint32_t constant_count;
    uint8_t compiled;
};

struct radeon_6790_sampler_state {
    uint8_t used;
    uint32_t filter_min;
    uint32_t filter_mag;
    uint32_t filter_mip;
    uint32_t max_anisotropy;
    uint32_t address_u;
    uint32_t address_v;
    uint32_t address_w;
    uint8_t compare_enable;
    uint8_t compare_func;
    uint8_t reduction_zero;
};

struct radeon_6790_blend_state {
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
    uint8_t alpha_to_coverage;
    uint8_t dual_source_blend;
};

struct radeon_6790_rasterizer_state {
    uint8_t used;
    uint32_t fill_mode;
    uint32_t cull_mode;
    uint8_t front_counter_clockwise;
    uint8_t depth_clip_enable;
    uint8_t scissor_enable;
    uint8_t multisample_enable;
    uint8_t antialiased_line_enable;
    uint8_t alpha_to_coverage;
    uint8_t depth_bias_enable;
    float depth_bias_constant;
    float depth_bias_slope;
    uint8_t conservative_raster;
};

struct radeon_6790_depth_stencil_state {
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
    uint8_t depth_bounds_enable;
};

struct radeon_6790_input_layout {
    uint8_t used;
    uint32_t element_count;
    uint32_t slot[32];
    uint32_t offset[32];
    uint32_t format[32];
};

struct radeon_6790_constant_buffer {
    uint8_t used;
    uint32_t size;
    uint8_t *data;
    uint64_t vram_offset;
};

struct radeon_6790_stream_output {
    uint8_t used;
    uint32_t buffer_count;
    uint64_t vram_offset[4];
    uint32_t size[4];
    uint32_t written[4];
};

struct radeon_6790_swap_chain {
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

struct radeon_6790_fence {
    uint8_t used;
    uint32_t value;
    uint8_t signaled;
};

struct radeon_6790_semaphore {
    uint8_t used;
    uint32_t value;
};

struct radeon_6790_vram_alloc {
    uint8_t used;
    uint64_t offset;
    uint64_t size;
    uint32_t type;
};

struct radeon_6790_occlusion_query {
    uint8_t used;
    uint8_t active;
    uint32_t samples;
    uint32_t result;
    uint8_t result_ready;
};

struct radeon_6790_pipeline_stats {
    uint8_t used;
    uint8_t active;
    uint64_t ia_vertices;
    uint64_t ia_primitives;
    uint64_t vs_invocations;
    uint64_t gs_invocations;
    uint64_t gs_primitives;
    uint64_t c_invocations;
    uint64_t c_primitives;
    uint64_t ps_invocations;
};

struct radeon_6790_tessellation {
    uint8_t used;
    uint32_t patch_vertices;
    float tess_factor_edge[4];
    float tess_factor_inside[2];
    uint8_t domain_is_triangles;
    uint8_t partitioning_fractional_odd;
};

struct radeon_6790_context {
    radeon_6790_handle_t vertex_buffer;
    radeon_6790_handle_t index_buffer;
    radeon_6790_handle_t input_layout;
    radeon_6790_handle_t vertex_shader;
    radeon_6790_handle_t pixel_shader;
    radeon_6790_handle_t geometry_shader;
    radeon_6790_handle_t compute_shader;
    radeon_6790_handle_t hull_shader;
    radeon_6790_handle_t domain_shader;
    radeon_6790_handle_t render_target;
    radeon_6790_handle_t depth_stencil;
    radeon_6790_handle_t blend_state;
    radeon_6790_handle_t rasterizer_state;
    radeon_6790_handle_t depth_stencil_state;
    radeon_6790_handle_t sampler[RADEON_6790_MAX_SAMPLERS];
    radeon_6790_handle_t texture[RADEON_6790_MAX_SAMPLERS];
    radeon_6790_handle_t constant_buffer[RADEON_6790_MAX_CONSTANT_BUFFERS];
    radeon_6790_handle_t stream_output[RADEON_6790_MAX_STREAM_OUTPUT];
    radeon_6790_handle_t tessellation;
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
    uint32_t render_target_array_slice;
    uint32_t msaa_samples;
};

int radeon_6790_init(void);
int radeon_6790_is_ready(void);
void radeon_6790_shutdown(void);

void radeon_6790_putpixel(uint32_t x, uint32_t y, uint32_t color);
void radeon_6790_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void radeon_6790_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void radeon_6790_cursor_set(uint32_t x, uint32_t y, const uint32_t *image);
void radeon_6790_cursor_hide(void);
void *radeon_6790_get_fb(void);
uint32_t radeon_6790_get_width(void);
uint32_t radeon_6790_get_height(void);
uint16_t radeon_6790_get_device_id(void);
uint8_t radeon_6790_get_revision(void);

void radeon_6790_info_get(struct radeon_6790_info *out);

uint64_t radeon_6790_vram_alloc(uint64_t size, uint32_t type);
void radeon_6790_vram_free(uint64_t handle);
uint64_t radeon_6790_vram_used(void);
uint64_t radeon_6790_vram_total(void);

radeon_6790_handle_t radeon_6790_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data);
void radeon_6790_vertex_buffer_destroy(radeon_6790_handle_t handle);
void radeon_6790_vertex_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_6790_handle_t radeon_6790_index_buffer_create(uint32_t index_count, uint32_t format, const void *data);
void radeon_6790_index_buffer_destroy(radeon_6790_handle_t handle);
void radeon_6790_index_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_6790_handle_t radeon_6790_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data);
void radeon_6790_texture_destroy(radeon_6790_handle_t handle);
void radeon_6790_texture_update(radeon_6790_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
void radeon_6790_texture_generate_mipmaps(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend);
void radeon_6790_render_target_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format, uint8_t compression);
void radeon_6790_depth_stencil_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void radeon_6790_shader_destroy(radeon_6790_handle_t handle);
void radeon_6790_shader_compile(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t max_anisotropy, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint8_t compare_enable, uint8_t compare_func, uint8_t reduction_zero);
void radeon_6790_sampler_state_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor, uint8_t alpha_to_coverage, uint8_t dual_source_blend);
void radeon_6790_blend_state_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage, uint8_t depth_bias_enable, float depth_bias_constant, float depth_bias_slope, uint8_t conservative_raster);
void radeon_6790_rasterizer_state_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func, uint8_t depth_bounds_enable);
void radeon_6790_depth_stencil_state_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void radeon_6790_input_layout_destroy(radeon_6790_handle_t handle);

radeon_6790_handle_t radeon_6790_constant_buffer_create(uint32_t size);
void radeon_6790_constant_buffer_destroy(radeon_6790_handle_t handle);
void radeon_6790_constant_buffer_update(radeon_6790_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_6790_handle_t radeon_6790_stream_output_create(uint32_t buffer_count, const uint64_t *sizes);
void radeon_6790_stream_output_destroy(radeon_6790_handle_t handle);
void radeon_6790_stream_output_update(radeon_6790_handle_t handle, uint32_t slot, const void *data, uint32_t size);

radeon_6790_handle_t radeon_6790_tessellation_state_create(uint32_t patch_vertices, const float *edge_factors, const float *inside_factors, uint8_t domain_triangles, uint8_t fractional_odd);
void radeon_6790_tessellation_state_destroy(radeon_6790_handle_t handle);
void radeon_6790_tessellation_state_update(radeon_6790_handle_t handle, const float *edge_factors, const float *inside_factors);

void radeon_6790_bind_vertex_buffer(radeon_6790_handle_t handle);
void radeon_6790_bind_index_buffer(radeon_6790_handle_t handle);
void radeon_6790_bind_input_layout(radeon_6790_handle_t handle);
void radeon_6790_bind_vertex_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_pixel_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_geometry_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_compute_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_hull_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_domain_shader(radeon_6790_handle_t handle);
void radeon_6790_bind_render_target(radeon_6790_handle_t handle);
void radeon_6790_bind_depth_stencil(radeon_6790_handle_t handle);
void radeon_6790_bind_blend_state(radeon_6790_handle_t handle);
void radeon_6790_bind_rasterizer_state(radeon_6790_handle_t handle);
void radeon_6790_bind_depth_stencil_state(radeon_6790_handle_t handle);
void radeon_6790_bind_sampler_and_texture(uint32_t slot, radeon_6790_handle_t sampler, radeon_6790_handle_t texture);
void radeon_6790_bind_constant_buffer(uint32_t slot, radeon_6790_handle_t handle);
void radeon_6790_bind_stream_output(uint32_t slot, radeon_6790_handle_t handle);
void radeon_6790_bind_tessellation(radeon_6790_handle_t handle);

void radeon_6790_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
void radeon_6790_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
void radeon_6790_set_primitive_topology(uint32_t topology);
void radeon_6790_set_alpha_blend_global(uint8_t enable);
void radeon_6790_set_depth_bounds(float min_depth, float max_depth);
void radeon_6790_set_render_target_array_slice(uint32_t slice);
void radeon_6790_set_msaa_samples(uint32_t samples);

void radeon_6790_clear_render_target(radeon_6790_handle_t rt, uint32_t color);
void radeon_6790_clear_depth_stencil(radeon_6790_handle_t ds, float depth, uint8_t stencil);
void radeon_6790_draw(uint32_t vertex_count, uint32_t start_vertex);
void radeon_6790_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex);
void radeon_6790_draw_indexed_instanced(uint32_t index_count, uint32_t start_index, int32_t base_vertex, uint32_t instance_count, uint32_t start_instance);
void radeon_6790_draw_indirect(uint32_t buffer_addr, uint32_t draw_count, uint32_t stride);

void radeon_6790_compute_dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z);
void radeon_6790_compute_dispatch_indirect(uint32_t buffer_addr);

radeon_6790_handle_t radeon_6790_occlusion_query_create(void);
void radeon_6790_occlusion_query_destroy(radeon_6790_handle_t handle);
void radeon_6790_occlusion_query_begin(radeon_6790_handle_t handle);
void radeon_6790_occlusion_query_end(radeon_6790_handle_t handle);
int radeon_6790_occlusion_query_get(radeon_6790_handle_t handle, uint32_t *samples);

radeon_6790_handle_t radeon_6790_pipeline_stats_create(void);
void radeon_6790_pipeline_stats_destroy(radeon_6790_handle_t handle);
void radeon_6790_pipeline_stats_begin(radeon_6790_handle_t handle);
void radeon_6790_pipeline_stats_end(radeon_6790_handle_t handle);
int radeon_6790_pipeline_stats_get(radeon_6790_handle_t handle, struct radeon_6790_pipeline_stats *out);

void radeon_6790_dma_transfer(uint64_t dst, uint64_t src, uint64_t size);

void radeon_6790_fence_create(radeon_6790_handle_t *out);
void radeon_6790_fence_signal(radeon_6790_handle_t handle, uint32_t value);
int radeon_6790_fence_wait(radeon_6790_handle_t handle, uint32_t value, uint32_t timeout_us);
void radeon_6790_fence_destroy(radeon_6790_handle_t handle);

void radeon_6790_semaphore_create(radeon_6790_handle_t *out);
void radeon_6790_semaphore_signal(radeon_6790_handle_t handle, uint32_t value);
void radeon_6790_semaphore_wait(radeon_6790_handle_t handle, uint32_t value);
void radeon_6790_semaphore_destroy(radeon_6790_handle_t handle);

int radeon_6790_wait_idle(uint32_t timeout_us);
uint64_t radeon_6790_timestamp(void);

void radeon_6790_power_set_state(uint8_t state);
uint8_t radeon_6790_power_get_state(void);
void radeon_6790_set_clock_gfx(uint32_t mhz);
void radeon_6790_set_clock_memory(uint32_t mhz);
uint32_t radeon_6790_get_clock_gfx(void);
uint32_t radeon_6790_get_clock_memory(void);
uint32_t radeon_6790_get_temperature(void);
void radeon_6790_set_fan_speed(uint32_t rpm);
uint32_t radeon_6790_get_fan_speed(void);
void radeon_6790_rgb_set(uint32_t color, uint8_t mode);
void radeon_6790_rgb_off(void);

radeon_6790_handle_t radeon_6790_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied);
void radeon_6790_swap_chain_destroy(radeon_6790_handle_t handle);
void radeon_6790_present(radeon_6790_handle_t chain);

void radeon_6790_command_begin(void);
void radeon_6790_command_end(void);

void radeon_6790_context_get(struct radeon_6790_context *out);
int radeon_6790_3d_available(void);

#ifdef __cplusplus
}
#endif

#endif
