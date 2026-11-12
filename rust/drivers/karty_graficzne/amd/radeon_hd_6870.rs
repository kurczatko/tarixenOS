#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const RADEON_VENDOR_ID: u16 = 0x1002;
pub const RADEON_HD_6870_DEVICE_ID: u16 = 0x6738;
pub const CHIP_BARTS: u32 = 0x6738;
pub const NORTHERN_ISLANDS_ARCH_VERSION: u32 = 3;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;
pub const REG_GART_BASE: u32 = 0xD0000000;
pub const REG_DOORBELL_BASE: u32 = 0xC0000000;

pub const NI_GRBM_CNTL: u32 = 0x0080;
pub const NI_GRBM_READ_CNTL: u32 = 0x0084;
pub const NI_GRBM_SOFT_RESET: u32 = 0x0088;
pub const NI_GRBM_GUI_MASK: u32 = 0x008C;
pub const NI_GRBM_STATUS: u32 = 0x0084;
pub const NI_GRBM_GFX_INDEX: u32 = 0x00D0;
pub const NI_GRBM_GFX_INDEX_INSTANCE: u32 = 0x00D4;

pub const NI_SRBM_STATUS: u32 = 0x00E0;
pub const NI_SRBM_GFX_CNTL: u32 = 0x00E4;

pub const NI_CP_ME_CNTL: u32 = 0x88C8;
pub const NI_CP_ME_CNTL2: u32 = 0x88CC;
pub const NI_CP_ME_RAM_WORD: u32 = 0x88D0;
pub const NI_CP_ME_RAM_RADDR: u32 = 0x88D4;
pub const NI_CP_ME_RADDR: u32 = 0x88D8;
pub const NI_CP_DEBUG: u32 = 0x88DC;
pub const NI_CP_PERFMON_CNTL: u32 = 0x88E0;
pub const NI_CP_ME_CNTL3: u32 = 0x88E4;
pub const NI_CP_HWDP_CNTL: u32 = 0x88E8;
pub const NI_CP_HWDP_ADDR: u32 = 0x88EC;
pub const NI_CP_HWDP_DATA: u32 = 0x88F0;
pub const NI_CP_ALWAYS_ON_CNTL: u32 = 0x88F4;
pub const NI_CP_ALWAYS_ON_CNTL2: u32 = 0x88F8;
pub const NI_CP_QUEUE_THRESHOLDS: u32 = 0x8900;
pub const NI_CP_STALL_CTRL: u32 = 0x8904;
pub const NI_CP_INT_CNTL: u32 = 0x8908;
pub const NI_CP_INT_ACK: u32 = 0x890C;
pub const NI_CP_INT_MASK: u32 = 0x8910;
pub const NI_CP_STATUS: u32 = 0x8914;
pub const NI_CP_PFFETCH_CNTL: u32 = 0x8918;
pub const NI_CP_PFFETCH_CNTL2: u32 = 0x891C;
pub const NI_CP_PFFETCH_BASE: u32 = 0x8920;
pub const NI_CP_PFFETCH_RPTR: u32 = 0x8924;
pub const NI_CP_PFFETCH_DATA: u32 = 0x8928;
pub const NI_CP_PFFETCH_STATUS: u32 = 0x892C;
pub const NI_CP_INTERRUPT_STATUS: u32 = 0x8930;
pub const NI_CP_INTERRUPT_ACK: u32 = 0x8934;
pub const NI_CP_INTERRUPT_MASK: u32 = 0x8938;
pub const NI_CP_INTERRUPT_MC_STATUS: u32 = 0x893C;
pub const NI_CP_PFP_FW_VERSION: u32 = 0x8824;
pub const NI_CP_ME_FW_VERSION: u32 = 0x8828;

pub const NI_VGT_CNTL: u32 = 0x8960;
pub const NI_VGT_PRIMITIVE_TYPE: u32 = 0x8964;
pub const NI_VGT_MULTI_PRIM_IB_RESET_INDX: u32 = 0x8968;
pub const NI_VGT_SHADER_STAGES_EN: u32 = 0x8A80;
pub const NI_VGT_GS_OFFLOAD: u32 = 0x8A84;
pub const NI_VGT_GS_OFFLOAD_2: u32 = 0x8A88;
pub const NI_VGT_GSVS_RING_OFFSET_0: u32 = 0x8A8C;
pub const NI_VGT_GSVS_RING_OFFSET_1: u32 = 0x8A90;
pub const NI_VGT_GSVS_RING_OFFSET_2: u32 = 0x8A94;
pub const NI_VGT_GSVS_RING_OFFSET_3: u32 = 0x8A98;
pub const NI_VGT_GSVS_RING_SIZE: u32 = 0x8A9C;
pub const NI_VGT_CONFIG: u32 = 0x8B10;
pub const NI_VGT_VERTEX_REUSE_BLOCK_CNTL: u32 = 0x8B14;
pub const NI_VGT_MULTI_PRIM_IB_RESET_INDX_2: u32 = 0x8B18;

pub const NI_PA_SU_SC_MODE_CNTL: u32 = 0x8CB8;
pub const NI_PA_SC_MODE_CNTL_0: u32 = 0x8C94;
pub const NI_PA_SC_MODE_CNTL_1: u32 = 0x8C98;
pub const NI_PA_SC_CLIP_CNTL: u32 = 0x8C9C;
pub const NI_PA_SC_VPORT_ZRANGE_0: u32 = 0x8CA4;
pub const NI_PA_SC_VPORT_ZRANGE_1: u32 = 0x8CA8;
pub const NI_PA_SC_VPORT_XSCALE_0: u32 = 0x8CAC;
pub const NI_PA_SC_VPORT_XOFFSET_0: u32 = 0x8CB0;
pub const NI_PA_SC_VPORT_YSCALE_0: u32 = 0x8CB4;
pub const NI_PA_SC_VPORT_YOFFSET_0: u32 = 0x8CB8;
pub const NI_PA_SC_AA_CONFIG: u32 = 0x8D2C;

pub const NI_PA_CL_CLIP_CNTL: u32 = 0x8B2C;
pub const NI_PA_CL_VTE_CNTL: u32 = 0x8B30;
pub const NI_PA_CL_VPORT_XSCALE_0: u32 = 0x8B34;
pub const NI_PA_CL_VPORT_XOFFSET_0: u32 = 0x8B38;
pub const NI_PA_CL_VPORT_YSCALE_0: u32 = 0x8B3C;
pub const NI_PA_CL_VPORT_YOFFSET_0: u32 = 0x8B40;
pub const NI_PA_CL_VPORT_ZSCALE_0: u32 = 0x8B44;
pub const NI_PA_CL_VPORT_ZOFFSET_0: u32 = 0x8B48;
pub const NI_PA_CL_VTE_CNTL_2: u32 = 0x8B4C;
pub const NI_PA_CL_VTE_CNTL_3: u32 = 0x8B50;

pub const NI_DB_SHADER_CONTROL: u32 = 0x2F00;
pub const NI_DB_DEPTH_CONTROL: u32 = 0x2F04;
pub const NI_DB_STENCIL_CONTROL: u32 = 0x2F08;
pub const NI_DB_DEPTH_BOUNDS: u32 = 0x2F0C;
pub const NI_DB_DEPTH_SLICE: u32 = 0x2F10;
pub const NI_DB_DEPTH_VIEW: u32 = 0x2F14;
pub const NI_DB_HTILE_SURFACE: u32 = 0x2F18;
pub const NI_DB_DEPTH_SIZE: u32 = 0x2F1C;
pub const NI_DB_Z_INFO: u32 = 0x2F20;
pub const NI_DB_Z_READ_MASK: u32 = 0x2F24;
pub const NI_DB_Z_WRITE_MASK: u32 = 0x2F28;
pub const NI_DB_STENCIL_READ_MASK: u32 = 0x2F2C;
pub const NI_DB_STENCIL_WRITE_MASK: u32 = 0x2F30;
pub const NI_DB_STENCIL_REF: u32 = 0x2F34;

pub const NI_CB_COLOR_CONTROL: u32 = 0x16CC;
pub const NI_CB_COLOR0_BASE: u32 = 0x16D0;
pub const NI_CB_COLOR0_SIZE: u32 = 0x16D4;
pub const NI_CB_COLOR0_PITCH: u32 = 0x16D8;
pub const NI_CB_COLOR0_VIEW: u32 = 0x16DC;

pub const NI_SQ_CNTL: u32 = 0x8C00;
pub const NI_SQ_CNTL_2: u32 = 0x8C04;
pub const NI_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ: u32 = 0x8C10;
pub const NI_SQ_DYN_GPR_CNTL_PS_FLUSH_ACK: u32 = 0x8C14;
pub const NI_SQ_DYN_GPR_CNTL_VS_FLUSH_REQ: u32 = 0x8C18;
pub const NI_SQ_DYN_GPR_CNTL_VS_FLUSH_ACK: u32 = 0x8C1C;
pub const NI_SQ_DYN_GPR_CNTL_GS_FLUSH_REQ: u32 = 0x8C20;
pub const NI_SQ_DYN_GPR_CNTL_GS_FLUSH_ACK: u32 = 0x8C24;
pub const NI_SQ_DYN_GPR_CNTL_ES_FLUSH_REQ: u32 = 0x8C28;
pub const NI_SQ_DYN_GPR_CNTL_ES_FLUSH_ACK: u32 = 0x8C2C;
pub const NI_SQ_DYN_GPR_CNTL: u32 = 0x8C30;
pub const NI_SQ_GPR_RESOURCE_MGMT_1: u32 = 0x8C34;
pub const NI_SQ_GPR_RESOURCE_MGMT_2: u32 = 0x8C38;
pub const NI_SQ_THREAD_RESOURCE_MGMT: u32 = 0x8C3C;
pub const NI_SQ_STACK_RESOURCE_MGMT_1: u32 = 0x8C40;
pub const NI_SQ_STACK_RESOURCE_MGMT_2: u32 = 0x8C44;
pub const NI_SQ_VTX_CONSTANT_WORD_0_0: u32 = 0x8D80;
pub const NI_SQ_VTX_CONSTANT_WORD_0_1: u32 = 0x8D84;
pub const NI_SQ_VTX_CONSTANT_WORD_0_2: u32 = 0x8D88;
pub const NI_SQ_VTX_CONSTANT_WORD_0_3: u32 = 0x8D8C;
pub const NI_SQ_VTX_CONSTANT_WORD_1_0: u32 = 0x8D90;
pub const NI_SQ_VTX_CONSTANT_WORD_1_1: u32 = 0x8D94;
pub const NI_SQ_VTX_CONSTANT_WORD_1_2: u32 = 0x8D98;
pub const NI_SQ_VTX_CONSTANT_WORD_1_3: u32 = 0x8D9C;
pub const NI_SQ_PGM_RESOURCES_VS: u32 = 0x8C80;
pub const NI_SQ_PGM_RESOURCES_PS: u32 = 0x8C84;
pub const NI_SQ_PGM_RESOURCES_GS: u32 = 0x8C88;
pub const NI_SQ_PGM_RESOURCES_ES: u32 = 0x8C8C;
pub const NI_SQ_PGM_START_VS: u32 = 0x8C90;
pub const NI_SQ_PGM_START_PS: u32 = 0x8C94;
pub const NI_SQ_PGM_START_GS: u32 = 0x8C98;
pub const NI_SQ_PGM_START_ES: u32 = 0x8C9C;
pub const NI_SQ_PGM_CF_OFFSET: u32 = 0x8CA0;
pub const NI_SQ_PGM_CF_OFFSET_END: u32 = 0x8CA4;
pub const NI_SQ_PGM_CF_OFFSET_ALIAS: u32 = 0x8CA8;
pub const NI_SQ_PGM_CF_OFFSET_END_ALIAS: u32 = 0x8CAC;

pub const NI_SPI_VS_OUT_CONFIG: u32 = 0x8C60;
pub const NI_SPI_VS_OUT_CONFIG_2: u32 = 0x8C64;
pub const NI_SPI_PS_IN_CONTROL: u32 = 0x8C68;
pub const NI_SPI_PS_INPUT_CNTL_0: u32 = 0x8C6C;
pub const NI_SPI_PS_INPUT_CNTL_1: u32 = 0x8C70;
pub const NI_SPI_PS_INPUT_CNTL_2: u32 = 0x8C74;
pub const NI_SPI_PS_INPUT_CNTL_3: u32 = 0x8C78;
pub const NI_SPI_PS_INPUT_CNTL_4: u32 = 0x8C7C;
pub const NI_SPI_PS_INPUT_CNTL_5: u32 = 0x8C80;
pub const NI_SPI_PS_INPUT_CNTL_6: u32 = 0x8C84;
pub const NI_SPI_PS_INPUT_CNTL_7: u32 = 0x8C88;
pub const NI_SPI_PS_INPUT_CNTL_8: u32 = 0x8C8C;
pub const NI_SPI_PS_INPUT_CNTL_9: u32 = 0x8C90;
pub const NI_SPI_PS_INPUT_CNTL_10: u32 = 0x8C94;
pub const NI_SPI_PS_INPUT_CNTL_11: u32 = 0x8C98;
pub const NI_SPI_PS_INPUT_CNTL_12: u32 = 0x8C9C;
pub const NI_SPI_PS_INPUT_CNTL_13: u32 = 0x8CA0;
pub const NI_SPI_PS_INPUT_CNTL_14: u32 = 0x8CA4;
pub const NI_SPI_PS_INPUT_CNTL_15: u32 = 0x8CA8;

pub const NI_GB_TILE_MODE: u32 = 0x04A0;
pub const NI_GB_TILE_MODE_0: u32 = 0x04A0;
pub const NI_GB_TILE_MODE_1: u32 = 0x04A4;
pub const NI_GB_TILE_MODE_2: u32 = 0x04A8;
pub const NI_GB_TILE_MODE_3: u32 = 0x04AC;
pub const NI_GB_TILE_MODE_4: u32 = 0x04B0;
pub const NI_GB_TILE_MODE_5: u32 = 0x04B4;
pub const NI_GB_TILE_MODE_6: u32 = 0x04B8;
pub const NI_GB_TILE_MODE_7: u32 = 0x04BC;

pub const NI_SX_MISC: u32 = 0x9200;
pub const NI_SX_ALPHA_TEST_CONTROL: u32 = 0x9210;
pub const NI_SX_ALPHA_REF: u32 = 0x9214;
pub const NI_SX_BLEND_OPT_CONTROL: u32 = 0x9218;
pub const NI_SX_MULTI_EXIT_CTL: u32 = 0x9220;
pub const NI_SX_POSITION_EXPORT_CONTROL: u32 = 0x9224;

pub const NI_SDMA0_BASE: u32 = 0x2E00;
pub const NI_SDMA1_BASE: u32 = 0x2F00;
pub const NI_SDMA0_RB_CNTL: u32 = 0x2E00;
pub const NI_SDMA0_RB_BASE: u32 = 0x2E04;
pub const NI_SDMA0_RB_RPTR: u32 = 0x2E08;
pub const NI_SDMA0_RB_WPTR: u32 = 0x2E0C;
pub const NI_SDMA0_IB_CNTL: u32 = 0x2E10;
pub const NI_SDMA0_IB_BASE: u32 = 0x2E14;
pub const NI_SDMA0_IB_RPTR: u32 = 0x2E18;
pub const NI_SDMA0_IB_WPTR: u32 = 0x2E1C;
pub const NI_SDMA1_RB_CNTL: u32 = 0x2F00;
pub const NI_SDMA1_RB_BASE: u32 = 0x2F04;
pub const NI_SDMA1_RB_RPTR: u32 = 0x2F08;
pub const NI_SDMA1_RB_WPTR: u32 = 0x2F0C;
pub const NI_SDMA1_IB_CNTL: u32 = 0x2F10;
pub const NI_SDMA1_IB_BASE: u32 = 0x2F14;
pub const NI_SDMA1_IB_RPTR: u32 = 0x2F18;
pub const NI_SDMA1_IB_WPTR: u32 = 0x2F1C;

pub const SHADER_STAGE_VERTEX: u32 = 0;
pub const SHADER_STAGE_PIXEL: u32 = 1;
pub const SHADER_STAGE_GEOMETRY: u32 = 2;
pub const SHADER_STAGE_TESSELLATION_CTRL: u32 = 3;
pub const SHADER_STAGE_TESSELLATION_EVAL: u32 = 4;
pub const SHADER_STAGE_COMPUTE: u32 = 5;

pub const FORMAT_R8G8B8A8_UNORM: u32 = 0;
pub const FORMAT_B8G8R8A8_UNORM: u32 = 1;
pub const FORMAT_R32G32B32A32_FLOAT: u32 = 2;
pub const FORMAT_R32G32_FLOAT: u32 = 3;
pub const FORMAT_D24_UNORM_S8_UINT: u32 = 4;
pub const FORMAT_D32_FLOAT: u32 = 5;

pub const BLEND_ZERO: u32 = 0;
pub const BLEND_ONE: u32 = 1;
pub const BLEND_SRC_COLOR: u32 = 2;
pub const BLEND_INV_SRC_COLOR: u32 = 3;
pub const BLEND_SRC_ALPHA: u32 = 4;
pub const BLEND_INV_SRC_ALPHA: u32 = 5;
pub const BLEND_DEST_ALPHA: u32 = 6;
pub const BLEND_INV_DEST_ALPHA: u32 = 7;
pub const BLEND_DEST_COLOR: u32 = 8;
pub const BLEND_INV_DEST_COLOR: u32 = 9;
pub const BLEND_SRC_ALPHA_SAT: u32 = 10;
pub const BLEND_BLEND_FACTOR: u32 = 11;
pub const BLEND_INV_BLEND_FACTOR: u32 = 12;

pub const BLEND_OP_ADD: u32 = 0;
pub const BLEND_OP_SUBTRACT: u32 = 1;
pub const BLEND_OP_REV_SUBTRACT: u32 = 2;
pub const BLEND_OP_MIN: u32 = 3;
pub const BLEND_OP_MAX: u32 = 4;

pub const CULL_NONE: u32 = 0;
pub const CULL_FRONT: u32 = 1;
pub const CULL_BACK: u32 = 2;

pub const FILL_SOLID: u32 = 0;
pub const FILL_WIREFRAME: u32 = 1;

pub const PRIM_TOPOLOGY_POINT_LIST: u32 = 0;
pub const PRIM_TOPOLOGY_LINE_LIST: u32 = 1;
pub const PRIM_TOPOLOGY_LINE_STRIP: u32 = 2;
pub const PRIM_TOPOLOGY_TRIANGLE_LIST: u32 = 3;
pub const PRIM_TOPOLOGY_TRIANGLE_STRIP: u32 = 4;
pub const PRIM_TOPOLOGY_TRIANGLE_FAN: u32 = 5;

pub const RADEON_HD6870_VRAM_SIZE: u64 = 1024 * 1024 * 1024;
pub const RADEON_HD6870_GART_SIZE: u64 = 512 * 1024 * 1024;
pub const RADEON_HD6870_ENGINE_CLOCK: u32 = 900000;
pub const RADEON_HD6870_MEMORY_CLOCK: u32 = 1050000;
pub const RADEON_HD6870_SHADER_UNITS: u32 = 1120;
pub const RADEON_HD6870_SIMD_COUNT: u32 = 14;
pub const RADEON_HD6870_ROP_COUNT: u32 = 32;
pub const RADEON_HD6870_TEXTURE_UNITS: u32 = 56;
pub const RADEON_HD6870_MEMORY_BUS_WIDTH: u32 = 256;
pub const RADEON_HD6870_VERTEX_CACHE_SIZE: u32 = 32 * 1024;
pub const RADEON_HD6870_PRIMITIVE_BUFFER_SIZE: u32 = 32 * 1024;

#[repr(C)]
pub struct Matrix4x4 {
    pub m: [f32; 16],
}

#[repr(C)]
pub struct Vector4 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}

#[repr(C)]
pub struct Vector3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[repr(C)]
pub struct Vector2 {
    pub u: f32,
    pub v: f32,
}

#[repr(C)]
pub struct Vertex {
    pub position: Vector4,
    pub color: u32,
    pub tex_coord: Vector2,
    pub normal: Vector3,
    pub tangent: Vector3,
    pub bitangent: Vector3,
}

#[repr(C)]
pub struct Color {
    pub r: f32,
    pub g: f32,
    pub b: f32,
    pub a: f32,
}

#[repr(C)]
pub struct Rect {
    pub x: i32,
    pub y: i32,
    pub width: i32,
    pub height: i32,
}

#[repr(C)]
pub struct Viewport {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
    pub min_depth: f32,
    pub max_depth: f32,
}

#[repr(C)]
pub struct Scissor {
    pub x: i32,
    pub y: i32,
    pub width: i32,
    pub height: i32,
}

#[repr(C)]
pub struct BufferDescriptor {
    pub address: u64,
    pub size: u32,
    pub stride: u32,
    pub flags: u32,
}

#[repr(C)]
pub struct TextureDescriptor {
    pub width: u32,
    pub height: u32,
    pub depth: u32,
    pub mip_levels: u32,
    pub array_size: u32,
    pub format: u32,
    pub flags: u32,
    pub address: u64,
}

#[repr(C)]
pub struct ShaderProgram {
    pub id: u32,
    pub vertex_shader_addr: u64,
    pub fragment_shader_addr: u64,
    pub geometry_shader_addr: u64,
    pub tessellation_ctrl_addr: u64,
    pub tessellation_eval_addr: u64,
    pub compute_shader_addr: u64,
    pub uniform_count: u32,
    pub uniform_offsets: [u32; 128],
    pub constant_register_count: u32,
    pub constant_register_offsets: [u32; 64],
}

#[repr(C)]
pub struct RingBuffer {
    pub base_addr: u64,
    pub size: u32,
    pub rptr: AtomicU32,
    pub wptr: AtomicU32,
    pub doorbell_offset: u32,
    pub doorbell_page: *mut u32,
}

#[repr(C)]
pub struct CommandProcessor {
    pub me_cntl: u32,
    pub me_cntl2: u32,
    pub queue_thresholds: u32,
    pub stall_ctrl: u32,
    pub int_mask: u32,
    pub pffetch_cntl: u32,
    pub pffetch_base: u64,
    pub pffetch_rptr: u32,
}

#[repr(C)]
pub struct ShaderModule {
    pub id: u32,
    pub tgsi_addr: u64,
    pub tgsi_size: u32,
    pub entry_point_addr: u64,
    pub entry_point_size: u32,
    pub stage: u32,
    pub gpr_count: u32,
    pub stack_count: u32,
}

#[repr(C)]
pub struct DescriptorSet {
    pub id: u32,
    pub addr: u64,
    pub size: u32,
    pub binding_count: u32,
    pub bindings: [u64; 64],
}

#[repr(C)]
pub struct CommandBuffer {
    pub id: u32,
    pub addr: u64,
    pub size: u32,
    pub offset: AtomicU32,
    pub submitted: AtomicBool,
}

#[repr(C)]
pub struct QueryPool {
    pub id: u32,
    pub addr: u64,
    pub count: u32,
    pub query_type: u32,
    pub results_addr: u64,
    pub results_size: u32,
}

#[repr(C)]
pub struct SwapChain {
    pub id: u32,
    pub image_count: u32,
    pub current_image: AtomicU32,
    pub image_addrs: [u64; 8],
    pub image_size: u32,
    pub image_format: u32,
    pub acquired: AtomicBool,
    pub presented: AtomicBool,
}

#[repr(C)]
pub struct GpuInstance {
    pub mmio_base: *mut u32,
    pub vram_base: *mut u32,
    pub gart_base: *mut u32,
    pub doorbell_base: *mut u32,
    pub vram_size: u64,
    pub vram_used: AtomicU64,
    pub gart_size: u64,
    pub engine_clock: u32,
    pub memory_clock: u32,
    pub shader_units: u32,
    pub simd_count: u32,
    pub rop_count: u32,
    pub texture_units: u32,
    pub memory_bus_width: u32,
    pub vertex_cache_size: u32,
    pub primitive_buffer_size: u32,
    pub command_buffer_size: u32,
    pub initialized: AtomicBool,
    pub error_count: AtomicU32,
    pub ring_gfx: RingBuffer,
    pub ring_compute: RingBuffer,
    pub ring_dma: RingBuffer,
    pub shader_modules: [ShaderModule; 32],
    pub shader_module_count: u32,
    pub descriptor_sets: [DescriptorSet; 32],
    pub descriptor_set_count: u32,
    pub command_buffers: [CommandBuffer; 32],
    pub command_buffer_count: u32,
    pub query_pools: [QueryPool; 32],
    pub query_pool_count: u32,
    pub swap_chains: [SwapChain; 8],
    pub swap_chain_count: u32,
    pub tessellation: bool,
    pub geometry_shader: bool,
    pub compute_shader: bool,
    pub vertex_texture_fetch: bool,
    pub stream_out: bool,
}

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new() -> Self {
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size: RADEON_HD6870_VRAM_SIZE,
            vram_used: AtomicU64::new(0),
            gart_size: RADEON_HD6870_GART_SIZE,
            engine_clock: RADEON_HD6870_ENGINE_CLOCK,
            memory_clock: RADEON_HD6870_MEMORY_CLOCK,
            shader_units: RADEON_HD6870_SHADER_UNITS,
            simd_count: RADEON_HD6870_SIMD_COUNT,
            rop_count: RADEON_HD6870_ROP_COUNT,
            texture_units: RADEON_HD6870_TEXTURE_UNITS,
            memory_bus_width: RADEON_HD6870_MEMORY_BUS_WIDTH,
            vertex_cache_size: RADEON_HD6870_VERTEX_CACHE_SIZE,
            primitive_buffer_size: RADEON_HD6870_PRIMITIVE_BUFFER_SIZE,
            command_buffer_size: 8 * 1024 * 1024,
            initialized: AtomicBool::new(false),
            error_count: AtomicU32::new(0),
            ring_gfx: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x100000,
                size: 8 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 0,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(0) },
            },
            ring_compute: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x2000000,
                size: 4 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 1,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(1) },
            },
            ring_dma: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x4000000,
                size: 4 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 2,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(2) },
            },
            shader_modules: unsafe { core::mem::zeroed() },
            shader_module_count: 0,
            descriptor_sets: unsafe { core::mem::zeroed() },
            descriptor_set_count: 0,
            command_buffers: unsafe { core::mem::zeroed() },
            command_buffer_count: 0,
            query_pools: unsafe { core::mem::zeroed() },
            query_pool_count: 0,
            swap_chains: unsafe { core::mem::zeroed() },
            swap_chain_count: 0,
            tessellation: false,
            geometry_shader: true,
            compute_shader: true,
            vertex_texture_fetch: true,
            stream_out: true,
        }
    }

    pub fn read_mmio(&self, offset: u32) -> u32 {
        unsafe { self.mmio_base.add(offset as usize / 4).read_volatile() }
    }

    pub fn write_mmio(&mut self, offset: u32, value: u32) {
        unsafe { self.mmio_base.add(offset as usize / 4).write_volatile(value); }
    }

    pub fn read_mmio_64(&self, offset: u32) -> u64 {
        unsafe {
            let lo = self.mmio_base.add(offset as usize / 4).read_volatile() as u64;
            let hi = self.mmio_base.add((offset as usize / 4) + 1).read_volatile() as u64;
            lo | (hi << 32)
        }
    }

    pub fn write_mmio_64(&mut self, offset: u32, value: u64) {
        unsafe {
            self.mmio_base.add(offset as usize / 4).write_volatile(value as u32);
            self.mmio_base.add((offset as usize / 4) + 1).write_volatile((value >> 32) as u32);
        }
    }

    pub fn detect(&self) -> bool {
        unsafe {
            let vendor = self.read_mmio(0x00);
            let device = self.read_mmio(0x04);
            let rev = self.read_mmio(0x08);
            vendor == RADEON_VENDOR_ID as u32 && device == RADEON_HD_6870_DEVICE_ID as u32 && rev != 0xFFFFFFFF
        }
    }

    pub fn soft_reset(&mut self) {
        self.write_mmio(NI_GRBM_SOFT_RESET, 0x00000001);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(NI_GRBM_SOFT_RESET, 0x00000000);
        self.wait_idle_inner();
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(NI_CP_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
        self.error_count.fetch_add(1, Ordering::SeqCst);
    }

    pub fn wait_idle(&self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(NI_CP_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn init_ring_buffers(&mut self) {
        self.write_mmio(NI_CP_PFFETCH_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(NI_CP_PFFETCH_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(NI_CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(NI_CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(NI_CP_QUEUE_THRESHOLDS, 0x00000001);
        self.write_mmio(NI_CP_STALL_CTRL, 0x00000000);
        self.write_mmio(NI_CP_INT_CNTL, 0x00000001);
        self.write_mmio(NI_CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(NI_CP_PFFETCH_RPTR, 0x00000000);
        self.write_mmio(NI_SDMA0_RB_BASE, (REG_VRAM_BASE + 0x2000000) as u32);
        self.write_mmio(NI_SDMA0_RB_BASE + 4, ((REG_VRAM_BASE + 0x2000000) >> 32) as u32);
        self.write_mmio(NI_SDMA0_RB_CNTL, 0x00000001);
        self.write_mmio(NI_SDMA0_IB_BASE, (REG_VRAM_BASE + 0x3000000) as u32);
        self.write_mmio(NI_SDMA0_IB_BASE + 4, ((REG_VRAM_BASE + 0x3000000) >> 32) as u32);
        self.write_mmio(NI_SDMA0_IB_CNTL, 0x00000001);
        self.write_mmio(NI_SDMA1_RB_BASE, (REG_VRAM_BASE + 0x4000000) as u32);
        self.write_mmio(NI_SDMA1_RB_BASE + 4, ((REG_VRAM_BASE + 0x4000000) >> 32) as u32);
        self.write_mmio(NI_SDMA1_RB_CNTL, 0x00000001);
        self.write_mmio(NI_SDMA1_IB_BASE, (REG_VRAM_BASE + 0x5000000) as u32);
        self.write_mmio(NI_SDMA1_IB_BASE + 4, ((REG_VRAM_BASE + 0x5000000) >> 32) as u32);
        self.write_mmio(NI_SDMA1_IB_CNTL, 0x00000001);
    }

    pub fn submit_doorbell(&mut self, ring_index: u32, value: u32) {
        unsafe {
            let doorbell_page = match ring_index {
                0 => self.ring_gfx.doorbell_page,
                1 => self.ring_compute.doorbell_page,
                2 => self.ring_dma.doorbell_page,
                _ => return,
            };
            doorbell_page.add(ring_index as usize).write_volatile(value);
        }
    }

    pub fn ring_write(&mut self, ring_index: u32, dword: u32) {
        let (wptr, base, size) = match ring_index {
            0 => (
                self.ring_gfx.wptr.load(Ordering::SeqCst),
                self.ring_gfx.base_addr,
                self.ring_gfx.size,
            ),
            1 => (
                self.ring_compute.wptr.load(Ordering::SeqCst),
                self.ring_compute.base_addr,
                self.ring_compute.size,
            ),
            2 => (
                self.ring_dma.wptr.load(Ordering::SeqCst),
                self.ring_dma.base_addr,
                self.ring_dma.size,
            ),
            _ => return,
        };
        let offset = (wptr % (size / 4)) as usize;
        unsafe {
            (base as *mut u32).add(offset).write_volatile(dword);
        }
        let next_wptr = (wptr + 1) % (size / 4);
        match ring_index {
            0 => self.ring_gfx.wptr.store(next_wptr, Ordering::SeqCst),
            1 => self.ring_compute.wptr.store(next_wptr, Ordering::SeqCst),
            2 => self.ring_dma.wptr.store(next_wptr, Ordering::SeqCst),
            _ => {}
        }
    }

    pub fn ring_submit(&mut self, ring_index: u32) {
        let (wptr, doorbell_offset) = match ring_index {
            0 => (self.ring_gfx.wptr.load(Ordering::SeqCst), self.ring_gfx.doorbell_offset),
            1 => (self.ring_compute.wptr.load(Ordering::SeqCst), self.ring_compute.doorbell_offset),
            2 => (self.ring_dma.wptr.load(Ordering::SeqCst), self.ring_dma.doorbell_offset),
            _ => return,
        };
        self.write_mmio(NI_CP_PFFETCH_RPTR, wptr);
        self.submit_doorbell(doorbell_offset, wptr);
    }
}

pub fn gpu_instance() -> &'static mut GpuInstance {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(inst) => inst,
            None => {
                GPU_INSTANCE = Some(GpuInstance::new());
                GPU_INSTANCE.as_mut().unwrap()
            }
        }
    }
}

pub fn init() -> bool {
    let inst = gpu_instance();
    if !inst.detect() {
        return false;
    }
    inst.soft_reset();
    inst.init_ring_buffers();
    inst.initialized.store(true, Ordering::SeqCst);
    true
}

pub fn is_ready() -> bool {
    unsafe {
        match &GPU_INSTANCE {
            Some(inst) => inst.initialized.load(Ordering::SeqCst),
            None => false,
        }
    }
}

pub fn putpixel(x: u32, y: u32, color: u32) {
    let inst = gpu_instance();
    let fb = inst.vram_base.add(((y * 1920 + x) as usize));
    unsafe { fb.write_volatile(color); }
}

pub fn fillrect(x: u32, y: u32, w: u32, h: u32, color: u32) {
    let inst = gpu_instance();
    for cy in y..(y + h) {
        for cx in x..(x + w) {
            let fb = inst.vram_base.add(((cy * 1920 + cx) as usize));
            unsafe { fb.write_volatile(color); }
        }
    }
}

pub fn blt(dst_x: u32, dst_y: u32, src_x: u32, src_y: u32, w: u32, h: u32) {
    let inst = gpu_instance();
    for cy in 0..h {
        for cx in 0..w {
            let src = inst.vram_base.add((((src_y + cy) * 1920 + (src_x + cx)) as usize));
            let dst = inst.vram_base.add((((dst_y + cy) * 1920 + (dst_x + cx)) as usize));
            unsafe {
                let val = src.read_volatile();
                dst.write_volatile(val);
            }
        }
    }
}

pub fn get_width() -> u32 { 1920 }
pub fn get_height() -> u32 { 1080 }
pub fn get_device_id() -> u16 { RADEON_HD_6870_DEVICE_ID }
pub fn get_revision() -> u8 { 0 }

#[repr(C)]
pub struct GpuInfo {
    pub vendor_id: u16,
    pub device_id: u16,
    pub revision_id: u8,
    pub mmio_base: u32,
    pub fb_addr: u32,
    pub fb_width: u32,
    pub fb_height: u32,
    pub fb_pitch: u32,
    pub initialized: bool,
    pub gcn_generation: u32,
}

#[repr(C)]
pub struct Gpu3dContext {
    pub vertex_buffer: u32,
    pub index_buffer: u32,
    pub input_layout: u32,
    pub vertex_shader: u32,
    pub pixel_shader: u32,
    pub geometry_shader: u32,
    pub compute_shader: u32,
    pub render_target: u32,
    pub depth_stencil: u32,
    pub blend_state: u32,
    pub rasterizer_state: u32,
    pub depth_stencil_state: u32,
    pub sampler_states: [u32; 16],
    pub textures: [u32; 16],
    pub constant_buffers: [u32; 16],
    pub viewport: Viewport,
    pub scissor: Scissor,
    pub primitive_topology: u32,
}

#[repr(C)]
pub struct HandleTableEntry {
    pub used: bool,
    pub kind: u32,
    pub addr: u64,
    pub size: u32,
}

pub const HANDLE_VERTEX_BUFFER: u32 = 1;
pub const HANDLE_INDEX_BUFFER: u32 = 2;
pub const HANDLE_TEXTURE: u32 = 3;
pub const HANDLE_RENDER_TARGET: u32 = 4;
pub const HANDLE_DEPTH_STENCIL: u32 = 5;
pub const HANDLE_SHADER: u32 = 6;
pub const HANDLE_SAMPLER: u32 = 7;
pub const HANDLE_BLEND_STATE: u32 = 8;
pub const HANDLE_RASTERIZER_STATE: u32 = 9;
pub const HANDLE_DEPTH_STENCIL_STATE: u32 = 10;
pub const HANDLE_INPUT_LAYOUT: u32 = 11;
pub const HANDLE_CONSTANT_BUFFER: u32 = 12;
pub const HANDLE_SWAP_CHAIN: u32 = 13;

pub const HANDLE_NULL: u32 = 0xFFFFFFFF;

static mut HANDLE_TABLE: [HandleTableEntry; 1024] = unsafe { core::mem::zeroed() };
static mut HANDLE_COUNT: u32 = 0;

fn alloc_handle(kind: u32, addr: u64, size: u32) -> u32 {
    unsafe {
        for i in 0..1024u32 {
            if !HANDLE_TABLE[i as usize].used {
                HANDLE_TABLE[i as usize] = HandleTableEntry { used: true, kind, addr, size };
                HANDLE_COUNT += 1;
                return i;
            }
        }
        HANDLE_NULL
    }
}

fn free_handle(handle: u32) {
    unsafe {
        if handle == HANDLE_NULL || handle >= 1024 {
            return;
        }
        HANDLE_TABLE[handle as usize].used = false;
    }
}

fn handle_addr(handle: u32) -> u64 {
    unsafe {
        if handle == HANDLE_NULL || handle >= 1024 {
            return 0;
        }
        HANDLE_TABLE[handle as usize].addr
    }
}

fn handle_size(handle: u32) -> u32 {
    unsafe {
        if handle == HANDLE_NULL || handle >= 1024 {
            return 0;
        }
        HANDLE_TABLE[handle as usize].size
    }
}

pub fn vertex_buffer_create(vertex_count: u32, vertex_size: u32, format: u32, data: *const u8) -> u32 {
    let inst = gpu_instance();
    let size = vertex_count * vertex_size;
    let addr = inst.vram_used.fetch_add(size as u64, Ordering::SeqCst);
    unsafe {
        let dst = inst.vram_base.add(addr as usize);
        for i in 0..size {
            dst.add(i as usize).write_volatile(data.add(i as usize).read_volatile());
        }
    }
    alloc_handle(HANDLE_VERTEX_BUFFER, addr, size)
}

pub fn vertex_buffer_destroy(handle: u32) {
    free_handle(handle);
}

pub fn vertex_buffer_update(handle: u32, offset: u32, size: u32, data: *const u8) {
    let addr = handle_addr(handle);
    if addr == 0 {
        return;
    }
    let inst = gpu_instance();
    unsafe {
        let dst = inst.vram_base.add((addr + offset as u64) as usize);
        for i in 0..size {
            dst.add(i as usize).write_volatile(data.add(i as usize).read_volatile());
        }
    }
}

pub fn index_buffer_create(index_count: u32, format: u32, data: *const u8) -> u32 {
    let inst = gpu_instance();
    let size = index_count * if format == 2 { 4 } else { 2 };
    let addr = inst.vram_used.fetch_add(size as u64, Ordering::SeqCst);
    unsafe {
        let dst = inst.vram_base.add(addr as usize);
        for i in 0..size {
            dst.add(i as usize).write_volatile(data.add(i as usize).read_volatile());
        }
    }
    alloc_handle(HANDLE_INDEX_BUFFER, addr, size)
}

pub fn index_buffer_destroy(handle: u32) {
    free_handle(handle);
}

pub fn index_buffer_update(handle: u32, offset: u32, size: u32, data: *const u8) {
    let addr = handle_addr(handle);
    if addr == 0 {
        return;
    }
    let inst = gpu_instance();
    unsafe {
        let dst = inst.vram_base.add((addr + offset as u64) as usize);
        for i in 0..size {
            dst.add(i as usize).write_volatile(data.add(i as usize).read_volatile());
        }
    }
}

pub fn texture_create(width: u32, height: u32, format: u32, mip_levels: u32, data: *const u8) -> u32 {
    let inst = gpu_instance();
    let size = width * height * 4;
    let addr = inst.vram_used.fetch_add(size as u64, Ordering::SeqCst);
    unsafe {
        let dst = inst.vram_base.add(addr as usize);
        for i in 0..size {
            dst.add(i as usize).write_volatile(data.add(i as usize).read_volatile());
        }
    }
    alloc_handle(HANDLE_TEXTURE, addr, size)
}

pub fn texture_destroy(handle: u32) {
    free_handle(handle);
}

pub fn texture_update(handle: u32, level: u32, x: u32, y: u32, w: u32, h: u32, data: *const u8) {
    let addr = handle_addr(handle);
    if addr == 0 {
        return;
    }
    let inst = gpu_instance();
    let d = inst.vram_base.add((addr + x as u64 * 4 + y as u64 * 1920 * 4) as usize);
    unsafe {
        for cy in 0..h {
            for cx in 0..w {
                let src_off = ((cy * w + cx) * 4) as usize;
                let dst_off = ((cy * 1920 + cx) * 4) as usize;
                for c in 0..4 {
                    d.add(dst_off + c).write_volatile(data.add(src_off + c).read_volatile());
                }
            }
        }
    }
}

pub fn texture_generate_mipmaps(handle: u32) {
    let _ = handle;
}

pub fn render_target_create(width: u32, height: u32, format: u32, alpha_blend: u8) -> u32 {
    let inst = gpu_instance();
    let size = width * height * 4;
    let addr = inst.vram_used.fetch_add(size as u64, Ordering::SeqCst);
    alloc_handle(HANDLE_RENDER_TARGET, addr, size)
}

pub fn render_target_destroy(handle: u32) {
    free_handle(handle);
}

pub fn depth_stencil_create(width: u32, height: u32, format: u32) -> u32 {
    let inst = gpu_instance();
    let size = width * height * 4;
    let addr = inst.vram_used.fetch_add(size as u64, Ordering::SeqCst);
    alloc_handle(HANDLE_DEPTH_STENCIL, addr, size)
}

pub fn depth_stencil_destroy(handle: u32) {
    free_handle(handle);
}

pub fn shader_create(shader_type: u32, code: *const u8, code_size: u32) -> u32 {
    let inst = gpu_instance();
    let addr = inst.vram_used.fetch_add(code_size as u64, Ordering::SeqCst);
    unsafe {
        let dst = inst.vram_base.add(addr as usize);
        for i in 0..code_size {
            dst.add(i as usize).write_volatile(code.add(i as usize).read_volatile());
        }
    }
    alloc_handle(HANDLE_SHADER, addr, code_size)
}

pub fn shader_destroy(handle: u32) {
    free_handle(handle);
}

pub fn sampler_state_create(filter_min: u32, filter_mag: u32, filter_mip: u32, address_u: u32, address_v: u32, address_w: u32, max_anisotropy: u32, compare_enable: u8) -> u32 {
    let _ = (filter_min, filter_mag, filter_mip, address_u, address_v, address_w, max_anisotropy, compare_enable);
    alloc_handle(HANDLE_SAMPLER, 0, 0)
}

pub fn sampler_state_destroy(handle: u32) {
    free_handle(handle);
}

pub fn blend_state_create(blend_enable: u8, src_blend: u32, dest_blend: u32, blend_op: u32, src_blend_alpha: u32, dest_blend_alpha: u32, blend_op_alpha: u32, write_mask: u32, blend_factor: u32) -> u32 {
    let _ = (blend_enable, src_blend, dest_blend, blend_op, src_blend_alpha, dest_blend_alpha, blend_op_alpha, write_mask, blend_factor);
    alloc_handle(HANDLE_BLEND_STATE, 0, 0)
}

pub fn blend_state_destroy(handle: u32) {
    free_handle(handle);
}

pub fn rasterizer_state_create(fill_mode: u32, cull_mode: u32, front_ccw: u8, depth_clip: u8, scissor: u8, multisample: u8, aa_line: u8, alpha_to_coverage: u8) -> u32 {
    let _ = (fill_mode, cull_mode, front_ccw, depth_clip, scissor, multisample, aa_line, alpha_to_coverage);
    alloc_handle(HANDLE_RASTERIZER_STATE, 0, 0)
}

pub fn rasterizer_state_destroy(handle: u32) {
    free_handle(handle);
}

pub fn depth_stencil_state_create(depth_enable: u8, depth_write_mask: u32, depth_func: u32, stencil_enable: u8, stencil_read_mask: u8, stencil_write_mask: u8, front_fail: u32, front_zfail: u32, front_pass: u32, front_func: u32, back_fail: u32, back_zfail: u32, back_pass: u32, back_func: u32) -> u32 {
    let _ = (depth_enable, depth_write_mask, depth_func, stencil_enable, stencil_read_mask, stencil_write_mask, front_fail, front_zfail, front_pass, front_func, back_fail, back_zfail, back_pass, back_func);
    alloc_handle(HANDLE_DEPTH_STENCIL_STATE, 0, 0)
}

pub fn depth_stencil_state_destroy(handle: u32) {
    free_handle(handle);
}

pub fn input_layout_create(element_count: u32, slots: *const u32, offsets: *const u32, formats: *const u32) -> u32 {
    let _ = (element_count, slots, offsets, formats);
    alloc_handle(HANDLE_INPUT_LAYOUT, 0, 0)
}

pub fn input_layout_destroy(handle: u32) {
    free_handle(handle);
}

pub fn constant_buffer_create(size: u32) -> u32 {
    let inst = gpu_instance();
    let addr = inst.vram_used.fetch_add(size as u64, Ordering::SeqCst);
    alloc_handle(HANDLE_CONSTANT_BUFFER, addr, size)
}

pub fn constant_buffer_destroy(handle: u32) {
    free_handle(handle);
}

pub fn constant_buffer_update(handle: u32, offset: u32, size: u32, data: *const u8) {
    let addr = handle_addr(handle);
    if addr == 0 {
        return;
    }
    let inst = gpu_instance();
    unsafe {
        let dst = inst.vram_base.add((addr + offset as u64) as usize);
        for i in 0..size {
            dst.add(i as usize).write_volatile(data.add(i as usize).read_volatile());
        }
    }
}

pub fn bind_vertex_buffer(handle: u32) {
    let _ = handle;
}

pub fn bind_index_buffer(handle: u32) {
    let _ = handle;
}

pub fn bind_input_layout(handle: u32) {
    let _ = handle;
}

pub fn bind_vertex_shader(handle: u32) {
    let _ = handle;
}

pub fn bind_pixel_shader(handle: u32) {
    let _ = handle;
}

pub fn bind_geometry_shader(handle: u32) {
    let _ = handle;
}

pub fn bind_compute_shader(handle: u32) {
    let _ = handle;
}

pub fn bind_render_target(handle: u32) {
    let _ = handle;
}

pub fn bind_depth_stencil(handle: u32) {
    let _ = handle;
}

pub fn bind_blend_state(handle: u32) {
    let _ = handle;
}

pub fn bind_rasterizer_state(handle: u32) {
    let _ = handle;
}

pub fn bind_depth_stencil_state(handle: u32) {
    let _ = handle;
}

pub fn bind_sampler_and_texture(slot: u32, sampler: u32, texture: u32) {
    let _ = (slot, sampler, texture);
}

pub fn bind_constant_buffer(slot: u32, handle: u32) {
    let _ = (slot, handle);
}

pub fn set_viewport(x: u32, y: u32, w: u32, h: u32, min_depth: f32, max_depth: f32) {
    let _ = (x, y, w, h, min_depth, max_depth);
}

pub fn set_scissor(left: u32, top: u32, right: u32, bottom: u32) {
    let _ = (left, top, right, bottom);
}

pub fn set_primitive_topology(topology: u32) {
    let _ = topology;
}

pub fn clear_render_target(rt: u32, color: u32) {
    let addr = handle_addr(rt);
    if addr == 0 {
        return;
    }
    let size = handle_size(rt);
    let inst = gpu_instance();
    unsafe {
        let dst = inst.vram_base.add(addr as usize);
        for i in 0..(size / 4) {
            dst.add(i as usize).write_volatile(color);
        }
    }
}

pub fn clear_depth_stencil(ds: u32, depth: f32, stencil: u8) {
    let _ = (ds, depth, stencil);
}

pub fn draw(vertex_count: u32, start_vertex: u32) {
    let _ = (vertex_count, start_vertex);
}

pub fn draw_indexed(index_count: u32, start_index: u32, base_vertex: i32) {
    let _ = (index_count, start_index, base_vertex);
}

pub fn command_begin() {
}

pub fn command_end() {
    let inst = gpu_instance();
    inst.ring_submit(0);
}

pub fn swap_chain_create(width: u32, height: u32, vsync: u8) -> u32 {
    let inst = gpu_instance();
    let size = width * height * 4;
    alloc_handle(HANDLE_SWAP_CHAIN, inst.vram_used.fetch_add(size as u64 * 2, Ordering::SeqCst), size)
}

pub fn swap_chain_destroy(handle: u32) {
    free_handle(handle);
}

pub fn present(chain: u32) {
    let _ = chain;
    let inst = gpu_instance();
    inst.ring_submit(0);
}

pub fn context_get() -> Gpu3dContext {
    Gpu3dContext {
        vertex_buffer: 0,
        index_buffer: 0,
        input_layout: 0,
        vertex_shader: 0,
        pixel_shader: 0,
        geometry_shader: 0,
        compute_shader: 0,
        render_target: 0,
        depth_stencil: 0,
        blend_state: 0,
        rasterizer_state: 0,
        depth_stencil_state: 0,
        sampler_states: [0; 16],
        textures: [0; 16],
        constant_buffers: [0; 16],
        viewport: Viewport { x: 0.0, y: 0.0, width: 1920.0, height: 1080.0, min_depth: 0.0, max_depth: 1.0 },
        scissor: Scissor { x: 0, y: 0, width: 1920, height: 1080 },
        primitive_topology: PRIM_TOPOLOGY_TRIANGLE_LIST,
    }
}

pub fn _3d_available() -> bool {
    true
}

pub fn info_get() -> GpuInfo {
    GpuInfo {
        vendor_id: RADEON_VENDOR_ID,
        device_id: RADEON_HD_6870_DEVICE_ID,
        revision_id: 0,
        mmio_base: REG_MMIO_BASE,
        fb_addr: REG_VRAM_BASE,
        fb_width: 1920,
        fb_height: 1080,
        fb_pitch: 1920 * 4,
        initialized: is_ready(),
        gcn_generation: NORTHERN_ISLANDS_ARCH_VERSION,
    }
}
