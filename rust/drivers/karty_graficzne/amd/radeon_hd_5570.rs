#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const RADEON_VENDOR_ID: u16 = 0x1002;
pub const RADEON_HD_5570_DEVICE_ID: u16 = 0x68BE;
pub const CHIP_REDWOOD: u32 = 0x68BE;
pub const EVERGREEN_ARCH_VERSION: u32 = 2;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;
pub const REG_GART_BASE: u32 = 0xD0000000;
pub const REG_DOORBELL_BASE: u32 = 0xC0000000;

pub const EVERGREEN_GRBM_CNTL: u32 = 0x0080;
pub const EVERGREEN_GRBM_READ_CNTL: u32 = 0x0084;
pub const EVERGREEN_GRBM_SOFT_RESET: u32 = 0x0088;
pub const EVERGREEN_GRBM_GUI_MASK: u32 = 0x008C;
pub const EVERGREEN_GRBM_STATUS: u32 = 0x0084;
pub const EVERGREEN_GRBM_GFX_INDEX: u32 = 0x00D0;
pub const EVERGREEN_GRBM_GFX_INDEX_INSTANCE: u32 = 0x00D4;

pub const EVERGREEN_SRBM_STATUS: u32 = 0x00E0;
pub const EVERGREEN_SRBM_GFX_CNTL: u32 = 0x00E4;

pub const EVERGREEN_CP_ME_CNTL: u32 = 0x88C8;
pub const EVERGREEN_CP_ME_CNTL2: u32 = 0x88CC;
pub const EVERGREEN_CP_ME_RAM_WORD: u32 = 0x88D0;
pub const EVERGREEN_CP_ME_RAM_RADDR: u32 = 0x88D4;
pub const EVERGREEN_CP_ME_RADDR: u32 = 0x88D8;
pub const EVERGREEN_CP_DEBUG: u32 = 0x88DC;
pub const EVERGREEN_CP_PERFMON_CNTL: u32 = 0x88E0;
pub const EVERGREEN_CP_ME_CNTL3: u32 = 0x88E4;
pub const EVERGREEN_CP_HWDP_CNTL: u32 = 0x88E8;
pub const EVERGREEN_CP_HWDP_ADDR: u32 = 0x88EC;
pub const EVERGREEN_CP_HWDP_DATA: u32 = 0x88F0;
pub const EVERGREEN_CP_ALWAYS_ON_CNTL: u32 = 0x88F4;
pub const EVERGREEN_CP_ALWAYS_ON_CNTL2: u32 = 0x88F8;
pub const EVERGREEN_CP_QUEUE_THRESHOLDS: u32 = 0x8900;
pub const EVERGREEN_CP_STALL_CTRL: u32 = 0x8904;
pub const EVERGREEN_CP_INT_CNTL: u32 = 0x8908;
pub const EVERGREEN_CP_INT_ACK: u32 = 0x890C;
pub const EVERGREEN_CP_INT_MASK: u32 = 0x8910;
pub const EVERGREEN_CP_STATUS: u32 = 0x8914;
pub const EVERGREEN_CP_PFFETCH_CNTL: u32 = 0x8918;
pub const EVERGREEN_CP_PFFETCH_CNTL2: u32 = 0x891C;
pub const EVERGREEN_CP_PFFETCH_BASE: u32 = 0x8920;
pub const EVERGREEN_CP_PFFETCH_RPTR: u32 = 0x8924;
pub const EVERGREEN_CP_PFFETCH_DATA: u32 = 0x8928;
pub const EVERGREEN_CP_PFFETCH_STATUS: u32 = 0x892C;
pub const EVERGREEN_CP_INTERRUPT_STATUS: u32 = 0x8930;
pub const EVERGREEN_CP_INTERRUPT_ACK: u32 = 0x8934;
pub const EVERGREEN_CP_INTERRUPT_MASK: u32 = 0x8938;
pub const EVERGREEN_CP_INTERRUPT_MC_STATUS: u32 = 0x893C;

pub const EVERGREEN_VGT_CNTL: u32 = 0x8960;
pub const EVERGREEN_VGT_PRIMITIVE_TYPE: u32 = 0x8964;
pub const EVERGREEN_VGT_MULTI_PRIM_IB_RESET_INDX: u32 = 0x8968;
pub const EVERGREEN_VGT_SHADER_STAGES_EN: u32 = 0x8A80;
pub const EVERGREEN_VGT_GS_OFFLOAD: u32 = 0x8A84;
pub const EVERGREEN_VGT_GS_OFFLOAD_2: u32 = 0x8A88;
pub const EVERGREEN_VGT_GSVS_RING_OFFSET_0: u32 = 0x8A8C;
pub const EVERGREEN_VGT_GSVS_RING_OFFSET_1: u32 = 0x8A90;
pub const EVERGREEN_VGT_GSVS_RING_OFFSET_2: u32 = 0x8A94;
pub const EVERGREEN_VGT_GSVS_RING_OFFSET_3: u32 = 0x8A98;
pub const EVERGREEN_VGT_GSVS_RING_SIZE: u32 = 0x8A9C;
pub const EVERGREEN_VGT_CONFIG: u32 = 0x8B10;
pub const EVERGREEN_VGT_VERTEX_REUSE_BLOCK_CNTL: u32 = 0x8B14;
pub const EVERGREEN_VGT_MULTI_PRIM_IB_RESET_INDX_2: u32 = 0x8B18;
pub const EVERGREEN_VGT_STRMOUT_VTX_STRIDE_0: u32 = 0x8E00;

pub const EVERGREEN_PA_SU_SC_MODE_CNTL: u32 = 0x8CB8;
pub const EVERGREEN_PA_SC_MODE_CNTL_0: u32 = 0x8C94;
pub const EVERGREEN_PA_SC_MODE_CNTL_1: u32 = 0x8C98;
pub const EVERGREEN_PA_SC_CLIP_CNTL: u32 = 0x8C9C;
pub const EVERGREEN_PA_SC_VPORT_ZRANGE_0: u32 = 0x8CA4;
pub const EVERGREEN_PA_SC_VPORT_ZRANGE_1: u32 = 0x8CA8;
pub const EVERGREEN_PA_SC_VPORT_XSCALE_0: u32 = 0x8CAC;
pub const EVERGREEN_PA_SC_VPORT_XOFFSET_0: u32 = 0x8CB0;
pub const EVERGREEN_PA_SC_VPORT_YSCALE_0: u32 = 0x8CB4;
pub const EVERGREEN_PA_SC_VPORT_YOFFSET_0: u32 = 0x8CB8;
pub const EVERGREEN_PA_SC_AA_CONFIG: u32 = 0x8D2C;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_0: u32 = 0x8D30;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_1: u32 = 0x8D34;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_2: u32 = 0x8D38;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_3: u32 = 0x8D3C;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_4: u32 = 0x8D40;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_5: u32 = 0x8D44;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_6: u32 = 0x8D48;
pub const EVERGREEN_PA_SC_AA_SAMPLE_LOCS_7: u32 = 0x8D4C;

pub const EVERGREEN_PA_CL_CLIP_CNTL: u32 = 0x8B2C;
pub const EVERGREEN_PA_CL_VTE_CNTL: u32 = 0x8B30;
pub const EVERGREEN_PA_CL_VPORT_XSCALE_0: u32 = 0x8B34;
pub const EVERGREEN_PA_CL_VPORT_XOFFSET_0: u32 = 0x8B38;
pub const EVERGREEN_PA_CL_VPORT_YSCALE_0: u32 = 0x8B3C;
pub const EVERGREEN_PA_CL_VPORT_YOFFSET_0: u32 = 0x8B40;
pub const EVERGREEN_PA_CL_VPORT_ZSCALE_0: u32 = 0x8B44;
pub const EVERGREEN_PA_CL_VPORT_ZOFFSET_0: u32 = 0x8B48;
pub const EVERGREEN_PA_CL_VTE_CNTL_2: u32 = 0x8B4C;
pub const EVERGREEN_PA_CL_VTE_CNTL_3: u32 = 0x8B50;
pub const EVERGREEN_PA_CL_VTE_CNTL_4: u32 = 0x8B54;
pub const EVERGREEN_PA_CL_VTE_CNTL_5: u32 = 0x8B58;
pub const EVERGREEN_PA_CL_VTE_CNTL_6: u32 = 0x8B5C;
pub const EVERGREEN_PA_CL_VTE_CNTL_7: u32 = 0x8B60;

pub const EVERGREEN_DB_SHADER_CONTROL: u32 = 0x2F00;
pub const EVERGREEN_DB_DEPTH_CONTROL: u32 = 0x2F04;
pub const EVERGREEN_DB_STENCIL_CONTROL: u32 = 0x2F08;
pub const EVERGREEN_DB_DEPTH_BOUNDS: u32 = 0x2F0C;
pub const EVERGREEN_DB_DEPTH_SLICE: u32 = 0x2F10;
pub const EVERGREEN_DB_DEPTH_VIEW: u32 = 0x2F14;
pub const EVERGREEN_DB_HTILE_SURFACE: u32 = 0x2F18;
pub const EVERGREEN_DB_DEPTH_SIZE: u32 = 0x2F1C;
pub const EVERGREEN_DB_Z_INFO: u32 = 0x2F20;
pub const EVERGREEN_DB_Z_READ_MASK: u32 = 0x2F24;
pub const EVERGREEN_DB_Z_WRITE_MASK: u32 = 0x2F28;
pub const EVERGREEN_DB_STENCIL_READ_MASK: u32 = 0x2F2C;
pub const EVERGREEN_DB_STENCIL_WRITE_MASK: u32 = 0x2F30;
pub const EVERGREEN_DB_STENCIL_REF: u32 = 0x2F34;

pub const EVERGREEN_CB_COLOR_CONTROL: u32 = 0x16CC;
pub const EVERGREEN_CB_COLOR0_BASE: u32 = 0x16D0;
pub const EVERGREEN_CB_COLOR0_SIZE: u32 = 0x16D4;
pub const EVERGREEN_CB_COLOR0_PITCH: u32 = 0x16D8;
pub const EVERGREEN_CB_COLOR0_VIEW: u32 = 0x16DC;
pub const EVERGREEN_CB_COLOR1_BASE: u32 = 0x16E0;
pub const EVERGREEN_CB_COLOR1_SIZE: u32 = 0x16E4;
pub const EVERGREEN_CB_COLOR1_PITCH: u32 = 0x16E8;
pub const EVERGREEN_CB_COLOR1_VIEW: u32 = 0x16EC;

pub const EVERGREEN_SQ_CNTL: u32 = 0x8C00;
pub const EVERGREEN_SQ_CNTL_2: u32 = 0x8C04;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ: u32 = 0x8C10;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_PS_FLUSH_ACK: u32 = 0x8C14;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_VS_FLUSH_REQ: u32 = 0x8C18;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_VS_FLUSH_ACK: u32 = 0x8C1C;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_GS_FLUSH_REQ: u32 = 0x8C20;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_GS_FLUSH_ACK: u32 = 0x8C24;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_ES_FLUSH_REQ: u32 = 0x8C28;
pub const EVERGREEN_SQ_DYN_GPR_CNTL_ES_FLUSH_ACK: u32 = 0x8C2C;
pub const EVERGREEN_SQ_DYN_GPR_CNTL: u32 = 0x8C30;
pub const EVERGREEN_SQ_GPR_RESOURCE_MGMT_1: u32 = 0x8C34;
pub const EVERGREEN_SQ_GPR_RESOURCE_MGMT_2: u32 = 0x8C38;
pub const EVERGREEN_SQ_THREAD_RESOURCE_MGMT: u32 = 0x8C3C;
pub const EVERGREEN_SQ_STACK_RESOURCE_MGMT_1: u32 = 0x8C40;
pub const EVERGREEN_SQ_STACK_RESOURCE_MGMT_2: u32 = 0x8C44;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_0_0: u32 = 0x8D80;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_0_1: u32 = 0x8D84;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_0_2: u32 = 0x8D88;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_0_3: u32 = 0x8D8C;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_1_0: u32 = 0x8D90;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_1_1: u32 = 0x8D94;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_1_2: u32 = 0x8D98;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_1_3: u32 = 0x8D9C;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_2_0: u32 = 0x8DA0;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_2_1: u32 = 0x8DA4;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_2_2: u32 = 0x8DA8;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_2_3: u32 = 0x8DAC;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_3_0: u32 = 0x8DB0;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_3_1: u32 = 0x8DB4;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_3_2: u32 = 0x8DB8;
pub const EVERGREEN_SQ_VTX_CONSTANT_WORD_3_3: u32 = 0x8DBC;
pub const EVERGREEN_SQ_PGM_RESOURCES_VS: u32 = 0x8C80;
pub const EVERGREEN_SQ_PGM_RESOURCES_PS: u32 = 0x8C84;
pub const EVERGREEN_SQ_PGM_RESOURCES_GS: u32 = 0x8C88;
pub const EVERGREEN_SQ_PGM_RESOURCES_ES: u32 = 0x8C8C;
pub const EVERGREEN_SQ_PGM_START_VS: u32 = 0x8C90;
pub const EVERGREEN_SQ_PGM_START_PS: u32 = 0x8C94;
pub const EVERGREEN_SQ_PGM_START_GS: u32 = 0x8C98;
pub const EVERGREEN_SQ_PGM_START_ES: u32 = 0x8C9C;
pub const EVERGREEN_SQ_PGM_CF_OFFSET: u32 = 0x8CA0;
pub const EVERGREEN_SQ_PGM_CF_OFFSET_END: u32 = 0x8CA4;
pub const EVERGREEN_SQ_PGM_CF_OFFSET_ALIAS: u32 = 0x8CA8;
pub const EVERGREEN_SQ_PGM_CF_OFFSET_END_ALIAS: u32 = 0x8CAC;
pub const EVERGREEN_SQ_IND_BUFFER_BASE: u32 = 0x8CA0;
pub const EVERGREEN_SQ_IND_BUFFER_SIZE: u32 = 0x8CA4;
pub const EVERGREEN_SQ_IND_BUFFER_RPTR: u32 = 0x8CA8;
pub const EVERGREEN_SQ_IND_BUFFER_WPTR: u32 = 0x8CAC;

pub const EVERGREEN_SPI_VS_OUT_CONFIG: u32 = 0x8C60;
pub const EVERGREEN_SPI_VS_OUT_CONFIG_2: u32 = 0x8C64;
pub const EVERGREEN_SPI_PS_IN_CONTROL: u32 = 0x8C68;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_0: u32 = 0x8C6C;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_1: u32 = 0x8C70;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_2: u32 = 0x8C74;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_3: u32 = 0x8C78;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_4: u32 = 0x8C7C;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_5: u32 = 0x8C80;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_6: u32 = 0x8C84;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_7: u32 = 0x8C88;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_8: u32 = 0x8C8C;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_9: u32 = 0x8C90;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_10: u32 = 0x8C94;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_11: u32 = 0x8C98;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_12: u32 = 0x8C9C;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_13: u32 = 0x8CA0;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_14: u32 = 0x8CA4;
pub const EVERGREEN_SPI_PS_INPUT_CNTL_15: u32 = 0x8CA8;

pub const EVERGREEN_GB_TILE_MODE: u32 = 0x04A0;
pub const EVERGREEN_GB_TILE_MODE_0: u32 = 0x04A0;
pub const EVERGREEN_GB_TILE_MODE_1: u32 = 0x04A4;
pub const EVERGREEN_GB_TILE_MODE_2: u32 = 0x04A8;
pub const EVERGREEN_GB_TILE_MODE_3: u32 = 0x04AC;
pub const EVERGREEN_GB_TILE_MODE_4: u32 = 0x04B0;
pub const EVERGREEN_GB_TILE_MODE_5: u32 = 0x04B4;
pub const EVERGREEN_GB_TILE_MODE_6: u32 = 0x04B8;
pub const EVERGREEN_GB_TILE_MODE_7: u32 = 0x04BC;

pub const EVERGREEN_SX_MISC: u32 = 0x9200;
pub const EVERGREEN_SX_ALPHA_TEST_CONTROL: u32 = 0x9210;
pub const EVERGREEN_SX_ALPHA_REF: u32 = 0x9214;
pub const EVERGREEN_SX_BLEND_OPT_CONTROL: u32 = 0x9218;
pub const EVERGREEN_SX_MULTI_EXIT_CTL: u32 = 0x9220;
pub const EVERGREEN_SX_POSITION_EXPORT_CONTROL: u32 = 0x9224;

pub const EVERGREEN_SDMA0_BASE: u32 = 0x2E00;
pub const EVERGREEN_SDMA1_BASE: u32 = 0x2F00;
pub const EVERGREEN_SDMA0_RB_CNTL: u32 = 0x2E00;
pub const EVERGREEN_SDMA0_RB_BASE: u32 = 0x2E04;
pub const EVERGREEN_SDMA0_RB_RPTR: u32 = 0x2E08;
pub const EVERGREEN_SDMA0_RB_WPTR: u32 = 0x2E0C;
pub const EVERGREEN_SDMA0_IB_CNTL: u32 = 0x2E10;
pub const EVERGREEN_SDMA0_IB_BASE: u32 = 0x2E14;
pub const EVERGREEN_SDMA0_IB_RPTR: u32 = 0x2E18;
pub const EVERGREEN_SDMA0_IB_WPTR: u32 = 0x2E1C;
pub const EVERGREEN_SDMA1_RB_CNTL: u32 = 0x2F00;
pub const EVERGREEN_SDMA1_RB_BASE: u32 = 0x2F04;
pub const EVERGREEN_SDMA1_RB_RPTR: u32 = 0x2F08;
pub const EVERGREEN_SDMA1_RB_WPTR: u32 = 0x2F0C;
pub const EVERGREEN_SDMA1_IB_CNTL: u32 = 0x2F10;
pub const EVERGREEN_SDMA1_IB_BASE: u32 = 0x2F14;
pub const EVERGREEN_SDMA1_IB_RPTR: u32 = 0x2F18;
pub const EVERGREEN_SDMA1_IB_WPTR: u32 = 0x2F1C;

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

pub const RADEON_HD5570_VRAM_SIZE: u64 = 1024 * 1024 * 1024;
pub const RADEON_HD5570_GART_SIZE: u64 = 512 * 1024 * 1024;
pub const RADEON_HD5570_ENGINE_CLOCK: u32 = 650000;
pub const RADEON_HD5570_MEMORY_CLOCK: u32 = 900000;
pub const RADEON_HD5570_SHADER_UNITS: u32 = 400;
pub const RADEON_HD5570_SIMD_COUNT: u32 = 5;
pub const RADEON_HD5570_ROP_COUNT: u32 = 8;
pub const RADEON_HD5570_TEXTURE_UNITS: u32 = 20;
pub const RADEON_HD5570_MEMORY_BUS_WIDTH: u32 = 128;
pub const RADEON_HD5570_VERTEX_CACHE_SIZE: u32 = 32 * 1024;
pub const RADEON_HD5570_PRIMITIVE_BUFFER_SIZE: u32 = 16 * 1024;

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new() -> Self {
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size: RADEON_HD5570_VRAM_SIZE,
            vram_used: AtomicU64::new(0),
            gart_size: RADEON_HD5570_GART_SIZE,
            engine_clock: RADEON_HD5570_ENGINE_CLOCK,
            memory_clock: RADEON_HD5570_MEMORY_CLOCK,
            shader_units: RADEON_HD5570_SHADER_UNITS,
            simd_count: RADEON_HD5570_SIMD_COUNT,
            rop_count: RADEON_HD5570_ROP_COUNT,
            texture_units: RADEON_HD5570_TEXTURE_UNITS,
            memory_bus_width: RADEON_HD5570_MEMORY_BUS_WIDTH,
            vertex_cache_size: RADEON_HD5570_VERTEX_CACHE_SIZE,
            primitive_buffer_size: RADEON_HD5570_PRIMITIVE_BUFFER_SIZE,
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
            stream_out: false,
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
            vendor == RADEON_VENDOR_ID as u32 && device == RADEON_HD_5570_DEVICE_ID as u32 && rev != 0xFFFFFFFF
        }
    }

    pub fn soft_reset(&mut self) {
        self.write_mmio(EVERGREEN_GRBM_SOFT_RESET, 0x00000001);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(EVERGREEN_GRBM_SOFT_RESET, 0x00000000);
        self.wait_idle_inner();
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(EVERGREEN_CP_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
        self.error_count.fetch_add(1, Ordering::SeqCst);
    }

    pub fn wait_idle(&self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(EVERGREEN_CP_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn init_ring_buffers(&mut self) {
        self.write_mmio(EVERGREEN_CP_PFFETCH_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(EVERGREEN_CP_PFFETCH_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(EVERGREEN_CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(EVERGREEN_CP_QUEUE_THRESHOLDS, 0x00000001);
        self.write_mmio(EVERGREEN_CP_STALL_CTRL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_INT_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_PFFETCH_RPTR, 0x00000000);
        self.write_mmio(EVERGREEN_SDMA0_RB_BASE, (REG_VRAM_BASE + 0x2000000) as u32);
        self.write_mmio(EVERGREEN_SDMA0_RB_BASE + 4, ((REG_VRAM_BASE + 0x2000000) >> 32) as u32);
        self.write_mmio(EVERGREEN_SDMA0_RB_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SDMA0_IB_BASE, (REG_VRAM_BASE + 0x3000000) as u32);
        self.write_mmio(EVERGREEN_SDMA0_IB_BASE + 4, ((REG_VRAM_BASE + 0x3000000) >> 32) as u32);
        self.write_mmio(EVERGREEN_SDMA0_IB_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SDMA1_RB_BASE, (REG_VRAM_BASE + 0x4000000) as u32);
        self.write_mmio(EVERGREEN_SDMA1_RB_BASE + 4, ((REG_VRAM_BASE + 0x4000000) >> 32) as u32);
        self.write_mmio(EVERGREEN_SDMA1_RB_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SDMA1_IB_BASE, (REG_VRAM_BASE + 0x5000000) as u32);
        self.write_mmio(EVERGREEN_SDMA1_IB_BASE + 4, ((REG_VRAM_BASE + 0x5000000) >> 32) as u32);
        self.write_mmio(EVERGREEN_SDMA1_IB_CNTL, 0x00000001);
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
        self.write_mmio(EVERGREEN_CP_PFFETCH_RPTR, wptr);
        self.submit_doorbell(doorbell_offset, wptr);
    }

    pub fn init(&mut self) -> bool {
        if self.initialized.load(Ordering::SeqCst) {
            return true;
        }
        if !self.detect() {
            return false;
        }
        self.soft_reset();
        self.setup_grbm();
        self.setup_srbm();
        self.setup_cp();
        self.setup_vgt();
        self.setup_pa();
        self.setup_db();
        self.setup_cb();
        self.setup_sq();
        self.setup_spi();
        self.setup_sx();
        self.setup_gb_tile();
        self.setup_power();
        self.init_ring_buffers();
        self.enable_3d_acceleration();
        self.initialized.store(true, Ordering::SeqCst);
        true
    }

    fn setup_grbm(&mut self) {
        self.write_mmio(EVERGREEN_GRBM_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_READ_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_SOFT_RESET, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_GUI_MASK, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_GFX_INDEX, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_GFX_INDEX_INSTANCE, 0x00000000);
    }

    fn setup_srbm(&mut self) {
        self.write_mmio(EVERGREEN_SRBM_STATUS, 0x00000000);
        self.write_mmio(EVERGREEN_SRBM_GFX_CNTL, 0x00000000);
    }

    fn setup_cp(&mut self) {
        self.write_mmio(EVERGREEN_CP_ME_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_ME_CNTL2, 0x00000000);
        self.write_mmio(EVERGREEN_CP_ME_CNTL3, 0x00000000);
        self.write_mmio(EVERGREEN_CP_DEBUG, 0x00000000);
        self.write_mmio(EVERGREEN_CP_PERFMON_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_HWDP_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_ALWAYS_ON_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_ALWAYS_ON_CNTL2, 0x00000000);
        self.write_mmio(EVERGREEN_CP_INT_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_INT_ACK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_INTERRUPT_MASK, 0x00000000);
    }

    fn setup_vgt(&mut self) {
        self.write_mmio(EVERGREEN_VGT_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_PRIMITIVE_TYPE, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_SHADER_STAGES_EN, 0x00000007);
        self.write_mmio(EVERGREEN_VGT_GSVS_RING_SIZE, 0x00004000);
        self.write_mmio(EVERGREEN_VGT_CONFIG, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_VERTEX_REUSE_BLOCK_CNTL, 0x00000000);
    }

    fn setup_pa(&mut self) {
        self.write_mmio(EVERGREEN_PA_SU_SC_MODE_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_MODE_CNTL_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_MODE_CNTL_1, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_CLIP_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_AA_CONFIG, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_CLIP_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL_2, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL_3, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL_4, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL_5, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL_6, 0x00000000);
        self.write_mmio(EVERGREEN_PA_CL_VTE_CNTL_7, 0x00000000);
    }

    fn setup_db(&mut self) {
        self.write_mmio(EVERGREEN_DB_SHADER_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_DB_DEPTH_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_DB_STENCIL_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_DB_DEPTH_BOUNDS, 0x00000000);
        self.write_mmio(EVERGREEN_DB_DEPTH_SIZE, 0x00000000);
        self.write_mmio(EVERGREEN_DB_Z_INFO, 0x00000000);
        self.write_mmio(EVERGREEN_DB_Z_READ_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_DB_Z_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_DB_STENCIL_READ_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_DB_STENCIL_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_DB_STENCIL_REF, 0x00000000);
    }

    fn setup_cb(&mut self) {
        self.write_mmio(EVERGREEN_CB_COLOR_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR0_BASE, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR0_SIZE, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR0_PITCH, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR0_VIEW, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR1_BASE, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR1_SIZE, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR1_PITCH, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR1_VIEW, 0x00000000);
    }

    fn setup_sq(&mut self) {
        self.write_mmio(EVERGREEN_SQ_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_CNTL_2, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_PS_FLUSH_ACK, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_VS_FLUSH_REQ, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_VS_FLUSH_ACK, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_GS_FLUSH_REQ, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_GS_FLUSH_ACK, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_ES_FLUSH_REQ, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL_ES_FLUSH_ACK, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_DYN_GPR_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_GPR_RESOURCE_MGMT_1, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_GPR_RESOURCE_MGMT_2, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_THREAD_RESOURCE_MGMT, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_STACK_RESOURCE_MGMT_1, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_STACK_RESOURCE_MGMT_2, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_PGM_RESOURCES_VS, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_PGM_RESOURCES_PS, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_PGM_RESOURCES_GS, 0x00000000);
        self.write_mmio(EVERGREEN_SQ_PGM_RESOURCES_ES, 0x00000000);
    }

    fn setup_spi(&mut self) {
        self.write_mmio(EVERGREEN_SPI_VS_OUT_CONFIG, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_VS_OUT_CONFIG_2, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_IN_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_0, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_1, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_2, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_3, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_4, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_5, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_6, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_7, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_8, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_9, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_10, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_11, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_12, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_13, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_14, 0x00000000);
        self.write_mmio(EVERGREEN_SPI_PS_INPUT_CNTL_15, 0x00000000);
    }

    fn setup_sx(&mut self) {
        self.write_mmio(EVERGREEN_SX_MISC, 0x00000000);
        self.write_mmio(EVERGREEN_SX_ALPHA_TEST_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_SX_ALPHA_REF, 0x00000000);
        self.write_mmio(EVERGREEN_SX_BLEND_OPT_CONTROL, 0x00000000);
        self.write_mmio(EVERGREEN_SX_MULTI_EXIT_CTL, 0x00000000);
        self.write_mmio(EVERGREEN_SX_POSITION_EXPORT_CONTROL, 0x00000000);
    }

    fn setup_gb_tile(&mut self) {
        self.write_mmio(EVERGREEN_GB_TILE_MODE_0, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_1, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_2, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_3, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_4, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_5, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_6, 0x00000000);
        self.write_mmio(EVERGREEN_GB_TILE_MODE_7, 0x00000000);
    }

    fn setup_power(&mut self) {
        self.write_mmio(EVERGREEN_CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_GRBM_CNTL, 0x00000000);
    }

    pub fn enable_3d_acceleration(&mut self) {
        self.write_mmio(EVERGREEN_VGT_SHADER_STAGES_EN, 0x00000007);
        self.write_mmio(EVERGREEN_PA_SC_MODE_CNTL_0, 0x00000001);
        self.write_mmio(EVERGREEN_DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_CB_COLOR_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_SQ_CNTL, 0x00000001);
    }

    pub fn load_shader_module(&mut self, id: u32, tgsi_addr: u64, tgsi_size: u32, stage: u32) -> bool {
        if self.shader_module_count >= 32 {
            return false;
        }
        let idx = self.shader_module_count as usize;
        self.shader_modules[idx] = ShaderModule {
            id,
            tgsi_addr,
            tgsi_size,
            entry_point_addr: 0,
            entry_point_size: 0,
            stage,
            gpr_count: 32,
            stack_count: 4,
        };
        self.shader_module_count += 1;
        true
    }

    pub fn bind_shader_stage(&mut self, stage: u32, module_id: u32) {
        let mut start_reg: u32 = 0;
        let mut resource_reg: u32 = 0;
        match stage {
            0 => {
                start_reg = EVERGREEN_SQ_PGM_START_VS;
                resource_reg = EVERGREEN_SQ_PGM_RESOURCES_VS;
            }
            1 => {
                start_reg = EVERGREEN_SQ_PGM_START_PS;
                resource_reg = EVERGREEN_SQ_PGM_RESOURCES_PS;
            }
            2 => {
                start_reg = EVERGREEN_SQ_PGM_START_GS;
                resource_reg = EVERGREEN_SQ_PGM_RESOURCES_GS;
            }
            3 => {
                start_reg = EVERGREEN_SQ_PGM_START_ES;
                resource_reg = EVERGREEN_SQ_PGM_RESOURCES_ES;
            }
            _ => return,
        }
        for i in 0..self.shader_module_count {
            if self.shader_modules[i as usize].id == module_id {
                let addr = self.shader_modules[i as usize].tgsi_addr;
                self.write_mmio(start_reg, addr as u32);
                self.write_mmio(start_reg + 4, (addr >> 32) as u32);
                self.write_mmio(resource_reg, 0x00000001);
                return;
            }
        }
    }

    pub fn set_uniform(&mut self, location: u32, values: &[f32; 4]) {
        let base = EVERGREEN_SQ_VTX_CONSTANT_WORD_0_0;
        if location >= 64 {
            return;
        }
        let reg = base + location * 16;
        unsafe {
            let ptr = self.mmio_base.add(reg as usize / 4);
            ptr.write_volatile(values[0].to_bits());
            ptr.add(1).write_volatile(values[1].to_bits());
            ptr.add(2).write_volatile(values[2].to_bits());
            ptr.add(3).write_volatile(values[3].to_bits());
        }
    }

    pub fn set_vertex_buffer(&mut self, slot: u32, descriptor: &BufferDescriptor) {
        if slot >= 32 {
            return;
        }
        self.write_mmio(EVERGREEN_SQ_IND_BUFFER_BASE, descriptor.address as u32);
        self.write_mmio(EVERGREEN_SQ_IND_BUFFER_BASE + 4, (descriptor.address >> 32) as u32);
        self.write_mmio(EVERGREEN_SQ_IND_BUFFER_SIZE, descriptor.size);
        self.write_mmio(EVERGREEN_VGT_STRMOUT_VTX_STRIDE_0 + slot * 4, descriptor.stride);
    }

    pub fn set_viewport(&mut self, viewport: &Viewport) {
        let xscale = (viewport.width / 2.0).to_bits();
        let xoffset = ((viewport.x + viewport.width / 2.0)).to_bits();
        let yscale = (viewport.height / 2.0).to_bits();
        let yoffset = ((viewport.y + viewport.height / 2.0)).to_bits();
        self.write_mmio(EVERGREEN_PA_CL_VPORT_XSCALE_0, xscale);
        self.write_mmio(EVERGREEN_PA_CL_VPORT_XOFFSET_0, xoffset);
        self.write_mmio(EVERGREEN_PA_CL_VPORT_YSCALE_0, yscale);
        self.write_mmio(EVERGREEN_PA_CL_VPORT_YOFFSET_0, yoffset);
        self.write_mmio(EVERGREEN_PA_CL_VPORT_ZSCALE_0, 0x3F800000);
        self.write_mmio(EVERGREEN_PA_CL_VPORT_ZOFFSET_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_XSCALE_0, xscale);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_XOFFSET_0, xoffset);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_YSCALE_0, yscale);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_YOFFSET_0, yoffset);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_ZRANGE_0, 0x00000000);
    }

    pub fn set_scissor(&mut self, scissor: &Scissor) {
        self.write_mmio(EVERGREEN_PA_CL_CLIP_CNTL, 0x00000001);
    }

    pub fn set_render_target(&mut self, index: u32, address: u64, size: u32, pitch: u32) {
        let base = match index {
            0 => EVERGREEN_CB_COLOR0_BASE,
            1 => EVERGREEN_CB_COLOR1_BASE,
            _ => return,
        };
        self.write_mmio(base, address as u32);
        self.write_mmio(base + 4, (address >> 32) as u32);
        self.write_mmio(base + 8, size);
        self.write_mmio(base + 12, pitch);
    }

    pub fn clear_color(&mut self, color: u32) {
        self.write_mmio(EVERGREEN_DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_CB_COLOR_CONTROL, 0x00000001);
    }

    pub fn clear_depth(&mut self, depth: u32) {
        self.write_mmio(EVERGREEN_DB_Z_INFO, depth);
        self.write_mmio(EVERGREEN_DB_DEPTH_CONTROL, 0x00000001);
    }

    pub fn draw_arrays(&mut self, primitive: u32, vertex_count: u32) {
        self.write_mmio(EVERGREEN_VGT_PRIMITIVE_TYPE, primitive);
        self.ring_write(0, 0x00000001);
        self.ring_write(0, vertex_count);
        self.ring_submit(0);
    }

    pub fn draw_indexed(&mut self, primitive: u32, index_count: u32, index_addr: u64) {
        self.write_mmio(EVERGREEN_VGT_PRIMITIVE_TYPE, primitive);
        self.ring_write(0, 0x00000002);
        self.ring_write(0, index_count);
        self.ring_write(0, index_addr as u32);
        self.ring_write(0, (index_addr >> 32) as u32);
        self.ring_submit(0);
    }

    pub fn present(&mut self, swap_chain_id: u32) {
        for i in 0..self.swap_chain_count {
            if self.swap_chains[i as usize].id == swap_chain_id {
                self.swap_chains[i as usize].presented.store(true, Ordering::SeqCst);
                self.ring_submit(0);
                return;
            }
        }
    }

    pub fn is_initialized(&self) -> bool {
        self.initialized.load(Ordering::SeqCst)
    }

    pub fn get_error_count(&self) -> u32 {
        self.error_count.load(Ordering::SeqCst)
    }

    pub fn get_shader_units(&self) -> u32 {
        self.shader_units
    }

    pub fn get_memory_size(&self) -> u64 {
        self.vram_size
    }
}

pub extern "C" fn radeon_hd5570_driver_init() -> bool {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(GpuInstance::new());
        }
        let instance = GPU_INSTANCE.as_mut().unwrap();
        instance.init()
    }
}

pub extern "C" fn radeon_hd5570_driver_detected() -> bool {
    unsafe {
        match &GPU_INSTANCE {
            Some(instance) => instance.detect(),
            None => false,
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_is_ready() -> bool {
    unsafe {
        match &GPU_INSTANCE {
            Some(instance) => instance.is_initialized(),
            None => false,
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_putpixel(x: u32, y: u32, color: u32) {
    unsafe {
        let instance = GPU_INSTANCE.as_mut().unwrap();
        let fb = instance.vram_base.add((y * 1024 + x) as usize);
        fb.write_volatile(color);
    }
}

pub extern "C" fn radeon_hd5570_driver_fillrect(x: u32, y: u32, w: u32, h: u32, color: u32) {
    unsafe {
        let instance = GPU_INSTANCE.as_mut().unwrap();
        for dy in 0..h {
            for dx in 0..w {
                let fb = instance.vram_base.add(((y + dy) * 1024 + (x + dx)) as usize);
                fb.write_volatile(color);
            }
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_get_width() -> u32 {
    1024
}

pub extern "C" fn radeon_hd5570_driver_get_height() -> u32 {
    768
}

pub extern "C" fn radeon_hd5570_driver_get_vram_size() -> u64 {
    unsafe {
        match &GPU_INSTANCE {
            Some(instance) => instance.get_memory_size(),
            None => RADEON_HD5570_VRAM_SIZE,
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_get_shader_units() -> u32 {
    unsafe {
        match &GPU_INSTANCE {
            Some(instance) => instance.get_shader_units(),
            None => RADEON_HD5570_SHADER_UNITS,
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_get_error_count() -> u32 {
    unsafe {
        match &GPU_INSTANCE {
            Some(instance) => instance.get_error_count(),
            None => 0,
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_load_shader(id: u32, addr: u64, size: u32, stage: u32) -> bool {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.load_shader_module(id, addr, size, stage),
            None => false,
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_bind_shader(stage: u32, module_id: u32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.bind_shader_stage(stage, module_id),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_set_uniform(location: u32, v0: f32, v1: f32, v2: f32, v3: f32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.set_uniform(location, &[v0, v1, v2, v3]),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_draw_arrays(primitive: u32, count: u32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.draw_arrays(primitive, count),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_draw_indexed(primitive: u32, count: u32, addr: u64) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.draw_indexed(primitive, count, addr),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_present(swap_chain_id: u32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.present(swap_chain_id),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_clear_color(color: u32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.clear_color(color),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_clear_depth(depth: u32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.clear_depth(depth),
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_set_viewport(x: f32, y: f32, width: f32, height: f32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => {
                instance.set_viewport(&Viewport {
                    x,
                    y,
                    width,
                    height,
                    min_depth: 0.0,
                    max_depth: 1.0,
                });
            }
            None => {}
        }
    }
}

pub extern "C" fn radeon_hd5570_driver_set_render_target(index: u32, address: u64, size: u32, pitch: u32) {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(instance) => instance.set_render_target(index, address, size, pitch),
            None => {}
        }
    }
}
