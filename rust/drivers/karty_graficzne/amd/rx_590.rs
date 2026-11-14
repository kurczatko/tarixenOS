/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const AMD_RADEON_RX_590_VENDOR_ID: u16 = 0x1002;
pub const AMD_RADEON_RX_590_DEVICE_ID: u16 = 0x67DF;
pub const CHIP_POLARIS_30: u32 = 0x67DF;
pub const GCN_ARCH_VERSION: u32 = 4;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;
pub const REG_GART_BASE: u32 = 0xD0000000;
pub const REG_DOORBELL_BASE: u32 = 0xC0000000;

pub const GRBM_CNTL: u32 = 0x0080;
pub const GRBM_READ_CNTL: u32 = 0x0084;
pub const GRBM_SOFT_RESET: u32 = 0x0088;
pub const GRBM_GUI_MASK: u32 = 0x008C;
pub const GRBM_RESET: u32 = 0x0088;
pub const GRBM_STATUS: u32 = 0x0084;
pub const GRBM_GFX_INDEX: u32 = 0x00D0;
pub const GRBM_GFX_INDEX_INSTANCE: u32 = 0x00D4;

pub const SRBM_STATUS: u32 = 0x00E0;
pub const SRBM_GFX_CNTL: u32 = 0x00E4;

pub const CP_ME_CNTL: u32 = 0x88C8;
pub const CP_ME_CNTL2: u32 = 0x88CC;
pub const CP_ME_RAM_WORD: u32 = 0x88D0;
pub const CP_ME_RAM_RADDR: u32 = 0x88D4;
pub const CP_ME_RADDR: u32 = 0x88D8;
pub const CP_DEBUG: u32 = 0x88DC;
pub const CP_PERFMON_CNTL: u32 = 0x88E0;
pub const CP_ME_CNTL3: u32 = 0x88E4;
pub const CP_HWDP_CNTL: u32 = 0x88E8;
pub const CP_HWDP_ADDR: u32 = 0x88EC;
pub const CP_HWDP_DATA: u32 = 0x88F0;
pub const CP_ALWAYS_ON_CNTL: u32 = 0x88F4;
pub const CP_ALWAYS_ON_CNTL2: u32 = 0x88F8;
pub const CP_QUEUE_THRESHOLDS: u32 = 0x8900;
pub const CP_STALL_CTRL: u32 = 0x8904;
pub const CP_INT_CNTL: u32 = 0x8908;
pub const CP_INT_ACK: u32 = 0x890C;
pub const CP_INT_MASK: u32 = 0x8910;
pub const CP_STATUS: u32 = 0x8914;
pub const CP_PFFETCH_CNTL: u32 = 0x8918;
pub const CP_PFFETCH_CNTL2: u32 = 0x891C;
pub const CP_PFFETCH_BASE: u32 = 0x8920;
pub const CP_PFFETCH_RPTR: u32 = 0x8924;
pub const CP_PFFETCH_DATA: u32 = 0x8928;
pub const CP_PFFETCH_STATUS: u32 = 0x892C;
pub const CP_INTERRUPT_STATUS: u32 = 0x8930;
pub const CP_INTERRUPT_ACK: u32 = 0x8934;
pub const CP_INTERRUPT_MASK: u32 = 0x8938;
pub const CP_INTERRUPT_MC_STATUS: u32 = 0x893C;

pub const VGT_CNTL: u32 = 0x8960;
pub const VGT_PRIMITIVE_TYPE: u32 = 0x8964;
pub const VGT_MULTI_PRIM_IB_RESET_INDX: u32 = 0x8968;
pub const VGT_SHADER_STAGES_EN: u32 = 0x8A80;
pub const VGT_GS_OFFLOAD: u32 = 0x8A84;
pub const VGT_GS_OFFLOAD_2: u32 = 0x8A88;
pub const VGT_GSVS_RING_OFFSET_0: u32 = 0x8A8C;
pub const VGT_GSVS_RING_OFFSET_1: u32 = 0x8A90;
pub const VGT_GSVS_RING_OFFSET_2: u32 = 0x8A94;
pub const VGT_GSVS_RING_OFFSET_3: u32 = 0x8A98;
pub const VGT_GSVS_RING_SIZE: u32 = 0x8A9C;
pub const VGT_CONFIG: u32 = 0x8B10;
pub const VGT_VERTEX_REUSE_BLOCK_CNTL: u32 = 0x8B14;
pub const VGT_MULTI_PRIM_IB_RESET_INDX_2: u32 = 0x8B18;

pub const PA_SU_SC_MODE_CNTL: u32 = 0x8CB8;
pub const PA_SC_MODE_CNTL_0: u32 = 0x8C94;
pub const PA_SC_MODE_CNTL_1: u32 = 0x8C98;
pub const PA_SC_CLIP_CNTL: u32 = 0x8C9C;
pub const PA_SC_VPORT_ZRANGE_0: u32 = 0x8CA4;
pub const PA_SC_VPORT_ZRANGE_1: u32 = 0x8CA8;
pub const PA_SC_VPORT_XSCALE_0: u32 = 0x8CAC;
pub const PA_SC_VPORT_XOFFSET_0: u32 = 0x8CB0;
pub const PA_SC_VPORT_YSCALE_0: u32 = 0x8CB4;
pub const PA_SC_VPORT_YOFFSET_0: u32 = 0x8CB8;
pub const PA_SC_AA_CONFIG: u32 = 0x8D2C;
pub const PA_SC_AA_SAMPLE_LOCS_0: u32 = 0x8D30;
pub const PA_SC_AA_SAMPLE_LOCS_1: u32 = 0x8D34;
pub const PA_SC_AA_SAMPLE_LOCS_2: u32 = 0x8D38;
pub const PA_SC_AA_SAMPLE_LOCS_3: u32 = 0x8D3C;
pub const PA_SC_AA_SAMPLE_LOCS_4: u32 = 0x8D40;
pub const PA_SC_AA_SAMPLE_LOCS_5: u32 = 0x8D44;
pub const PA_SC_AA_SAMPLE_LOCS_6: u32 = 0x8D48;
pub const PA_SC_AA_SAMPLE_LOCS_7: u32 = 0x8D4C;

pub const PA_CL_CLIP_CNTL: u32 = 0x8B2C;
pub const PA_CL_VTE_CNTL: u32 = 0x8B30;
pub const PA_CL_VPORT_XSCALE_0: u32 = 0x8B34;
pub const PA_CL_VPORT_XOFFSET_0: u32 = 0x8B38;
pub const PA_CL_VPORT_YSCALE_0: u32 = 0x8B3C;
pub const PA_CL_VPORT_YOFFSET_0: u32 = 0x8B40;
pub const PA_CL_VPORT_ZSCALE_0: u32 = 0x8B44;
pub const PA_CL_VPORT_ZOFFSET_0: u32 = 0x8B48;
pub const PA_CL_VTE_CNTL_2: u32 = 0x8B4C;
pub const PA_CL_VTE_CNTL_3: u32 = 0x8B50;
pub const PA_CL_VTE_CNTL_4: u32 = 0x8B54;
pub const PA_CL_VTE_CNTL_5: u32 = 0x8B58;
pub const PA_CL_VTE_CNTL_6: u32 = 0x8B5C;
pub const PA_CL_VTE_CNTL_7: u32 = 0x8B60;

pub const DB_SHADER_CONTROL: u32 = 0x2F00;
pub const DB_DEPTH_CONTROL: u32 = 0x2F04;
pub const DB_STENCIL_CONTROL: u32 = 0x2F08;
pub const DB_DEPTH_BOUNDS: u32 = 0x2F0C;
pub const DB_DEPTH_SLICE: u32 = 0x2F10;
pub const DB_DEPTH_VIEW: u32 = 0x2F14;
pub const DB_HTILE_SURFACE: u32 = 0x2F18;
pub const DB_DEPTH_SIZE: u32 = 0x2F1C;
pub const DB_Z_INFO: u32 = 0x2F20;
pub const DB_Z_READ_MASK: u32 = 0x2F24;
pub const DB_Z_WRITE_MASK: u32 = 0x2F28;
pub const DB_STENCIL_READ_MASK: u32 = 0x2F2C;
pub const DB_STENCIL_WRITE_MASK: u32 = 0x2F30;
pub const DB_STENCIL_REF: u32 = 0x2F34;
pub const DB_STENCILREFMASK: u32 = 0x2F38;
pub const DB_STENCILREFMASK_BACK: u32 = 0x2F3C;
pub const DB_STENCILCLEAR: u32 = 0x2F40;
pub const DB_DEPTH_CLEAR: u32 = 0x2F44;
pub const DB_EQ_MASK: u32 = 0x2F48;
pub const DB_HIZ_OFFSET: u32 = 0x2F4C;
pub const DB_DEPTH_CLEAR_LO: u32 = 0x2F50;
pub const DB_DEPTH_CLEAR_HI: u32 = 0x2F54;
pub const DB_STENCIL_CLEAR: u32 = 0x2F58;
pub const DB_STENCIL_CLEAR_LO: u32 = 0x2F5C;
pub const DB_STENCIL_CLEAR_HI: u32 = 0x2F60;

pub const CB_TARGET_MASK: u32 = 0x16B0;
pub const CB_SHADER_MASK: u32 = 0x16B4;
pub const CB_BLEND_CNTL: u32 = 0x16B8;
pub const CB_BLEND_RED: u32 = 0x16BC;
pub const CB_BLEND_GREEN: u32 = 0x16C0;
pub const CB_BLEND_BLUE: u32 = 0x16C4;
pub const CB_BLEND_ALPHA: u32 = 0x16C8;
pub const CB_COLOR_CONTROL: u32 = 0x16CC;
pub const CB_COLOR0_BASE: u32 = 0x16D0;
pub const CB_COLOR0_SIZE: u32 = 0x16D4;
pub const CB_COLOR0_PITCH: u32 = 0x16D8;
pub const CB_COLOR0_SLICE: u32 = 0x16DC;
pub const CB_COLOR0_INFO: u32 = 0x16E0;
pub const CB_COLOR0_VIEW: u32 = 0x16E4;
pub const CB_COLOR0_ATTRIB: u32 = 0x16E8;
pub const CB_COLOR0_CLEAR: u32 = 0x16EC;
pub const CB_COLOR0_CLEAR_DW0: u32 = 0x16F0;
pub const CB_COLOR0_CLEAR_DW1: u32 = 0x16F4;
pub const CB_COLOR1_BASE: u32 = 0x1700;
pub const CB_COLOR1_SIZE: u32 = 0x1704;
pub const CB_COLOR1_PITCH: u32 = 0x1708;
pub const CB_COLOR1_SLICE: u32 = 0x170C;
pub const CB_COLOR1_INFO: u32 = 0x1710;
pub const CB_COLOR1_VIEW: u32 = 0x1714;
pub const CB_COLOR1_ATTRIB: u32 = 0x1718;
pub const CB_COLOR1_CLEAR: u32 = 0x171C;
pub const CB_COLOR2_BASE: u32 = 0x1720;
pub const CB_COLOR2_SIZE: u32 = 0x1724;
pub const CB_COLOR2_PITCH: u32 = 0x1728;
pub const CB_COLOR2_SLICE: u32 = 0x172C;
pub const CB_COLOR2_INFO: u32 = 0x1730;
pub const CB_COLOR2_VIEW: u32 = 0x1734;
pub const CB_COLOR2_ATTRIB: u32 = 0x1738;
pub const CB_COLOR2_CLEAR: u32 = 0x173C;
pub const CB_COLOR3_BASE: u32 = 0x1740;
pub const CB_COLOR3_SIZE: u32 = 0x1744;
pub const CB_COLOR3_PITCH: u32 = 0x1748;
pub const CB_COLOR3_SLICE: u32 = 0x174C;
pub const CB_COLOR3_INFO: u32 = 0x1750;
pub const CB_COLOR3_VIEW: u32 = 0x1754;
pub const CB_COLOR3_ATTRIB: u32 = 0x1758;
pub const CB_COLOR3_CLEAR: u32 = 0x175C;

pub const TA_CNTL_AUX: u32 = 0x9000;
pub const TA_CNTL_AUX_1: u32 = 0x9004;
pub const TA_CNTL_AUX_2: u32 = 0x9008;
pub const TA_CNTL_AUX_3: u32 = 0x900C;
pub const TA_CNTL_AUX_4: u32 = 0x9010;
pub const TA_CNTL_AUX_5: u32 = 0x9014;
pub const TA_CNTL_AUX_6: u32 = 0x9018;
pub const TA_CNTL_AUX_7: u32 = 0x901C;

pub const SDMA0_REGISTERS_BASE: u32 = 0x2E00;
pub const SDMA1_REGISTERS_BASE: u32 = 0x2F00;
pub const SDMA0_RLC0_RB_CNTL: u32 = 0x2E00;
pub const SDMA0_RLC0_RB_BASE: u32 = 0x2E04;
pub const SDMA0_RLC0_RB_RPTR: u32 = 0x2E08;
pub const SDMA0_RLC0_RB_WPTR: u32 = 0x2E0C;
pub const SDMA0_RLC0_IB_CNTL: u32 = 0x2E10;
pub const SDMA0_RLC0_IB_BASE: u32 = 0x2E14;
pub const SDMA0_RLC0_IB_RPTR: u32 = 0x2E18;
pub const SDMA0_RLC0_IB_WPTR: u32 = 0x2E1C;
pub const SDMA1_RLC0_RB_CNTL: u32 = 0x2F00;
pub const SDMA1_RLC0_RB_BASE: u32 = 0x2F04;
pub const SDMA1_RLC0_RB_RPTR: u32 = 0x2F08;
pub const SDMA1_RLC0_RB_WPTR: u32 = 0x2F0C;
pub const SDMA1_RLC0_IB_CNTL: u32 = 0x2F10;
pub const SDMA1_RLC0_IB_BASE: u32 = 0x2F14;
pub const SDMA1_RLC0_IB_RPTR: u32 = 0x2F18;
pub const SDMA1_RLC0_IB_WPTR: u32 = 0x2F1C;

pub const PCIE_REGISTERS_BASE: u32 = 0x9000;
pub const PCIE_LC_CNTL: u32 = 0x9000;
pub const PCIE_LC_STATUS: u32 = 0x9004;
pub const PCIE_LC_CNTL2: u32 = 0x9008;
pub const PCIE_LC_CNTL3: u32 = 0x900C;
pub const PCIE_LC_CNTL4: u32 = 0x9010;
pub const PCIE_LC_CNTL5: u32 = 0x9014;
pub const PCIE_LC_CNTL6: u32 = 0x9018;
pub const PCIE_LC_CNTL7: u32 = 0x901C;
pub const PCIE_LC_CNTL8: u32 = 0x9020;
pub const PCIE_LC_CNTL9: u32 = 0x9024;
pub const PCIE_LC_CNTL10: u32 = 0x9028;
pub const PCIE_LC_CNTL11: u32 = 0x902C;

pub const POWER_PLAY_BASE: u32 = 0xA000;
pub const POWER_PLAY_SCLK_DPM: u32 = 0xA000;
pub const POWER_PLAY_MCLK_DPM: u32 = 0xA004;
pub const POWER_PLAY_VDD_CURR_DEP: u32 = 0xA008;
pub const POWER_PLAY_SCLK_PW: u32 = 0xA00C;
pub const POWER_PLAY_MCLK_PW: u32 = 0xA010;
pub const POWER_PLAY_TEMPERATURE: u32 = 0xA014;
pub const POWER_PLAY_FAN_CTRL: u32 = 0xA018;
pub const POWER_PLAY_POWER_STATE: u32 = 0xA01C;
pub const POWER_PLAY_GFX_CLOCK_GATING: u32 = 0xA020;
pub const POWER_PLAY_CP_CLOCK_GATING: u32 = 0xA024;
pub const POWER_PLAY_SDMA_CLOCK_GATING: u32 = 0xA028;

pub const SMU_BASE: u32 = 0xB000;
pub const SMU_SYSTEM_CNTL: u32 = 0xB000;
pub const SMU_SYSTEM_STATUS: u32 = 0xB004;
pub const SMU_SYSTEM_INTERRUPT: u32 = 0xB008;
pub const SMU_SYSTEM_INTERRUPT_MASK: u32 = 0xB00C;
pub const SMU_SYSTEM_PWR_STATUS: u32 = 0xB010;
pub const SMU_SYSTEM_PWR_CNTL: u32 = 0xB014;
pub const SMU_SYSTEM_PWR_MASK: u32 = 0xB018;
pub const SMU_GFX_PWR_STATUS: u32 = 0xB01C;
pub const SMU_GFX_PWR_CNTL: u32 = 0xB020;
pub const SMU_GFX_PWR_MASK: u32 = 0xB024;
pub const SMU_UVD_PWR_STATUS: u32 = 0xB028;
pub const SMU_UVD_PWR_CNTL: u32 = 0xB02C;
pub const SMU_UVD_PWR_MASK: u32 = 0xB030;
pub const SMU_VCE_PWR_STATUS: u32 = 0xB034;
pub const SMU_VCE_PWR_CNTL: u32 = 0xB038;
pub const SMU_VCE_PWR_MASK: u32 = 0xB03C;

pub const UVD_BASE: u32 = 0xC100;
pub const VCE_BASE: u32 = 0xC200;

pub const SHADER_BASE: u32 = 0x8C00;
pub const SHADER_CNTL: u32 = 0x8C00;
pub const SHADER_VTX_CNTL: u32 = 0x8C04;
pub const SHADER_FRAG_CNTL: u32 = 0x8C08;
pub const SHADER_GEOM_CNTL: u32 = 0x8C0C;
pub const SHADER_COMPUTE_CNTL: u32 = 0x8C10;
pub const SHADER_TESS_CNTL: u32 = 0x8C14;
pub const SHADER_UNIFORM_CNTL: u32 = 0x8C18;
pub const SHADER_ATTRIB_CNTL: u32 = 0x8C1C;
pub const SHADER_CONSTANT_BASE: u32 = 0x8C80;
pub const SHADER_SAMPLER_BASE: u32 = 0x8D00;
pub const SHADER_RESOURCE_BASE: u32 = 0x8D80;
pub const SHADER_EXPORT_BASE: u32 = 0x8E00;
pub const SHADER_STREAM_OUT_BASE: u32 = 0x8E80;

pub const SHADER_MODULE_BASE: u32 = 0x13E000;
pub const SHADER_MODULE_CNTL: u32 = 0x13E000;
pub const SHADER_MODULE_CREATE: u32 = 0x13E004;
pub const SHADER_MODULE_DESTROY: u32 = 0x13E008;
pub const SHADER_MODULE_SPIRV_ADDR: u32 = 0x13E00C;
pub const SHADER_MODULE_SPIRV_SIZE: u32 = 0x13E010;
pub const SHADER_MODULE_ENTRY_POINT: u32 = 0x13E014;

pub const DESCRIPTOR_SET_BASE: u32 = 0x13F000;
pub const DESCRIPTOR_SET_CNTL: u32 = 0x13F000;
pub const DESCRIPTOR_SET_ALLOC: u32 = 0x13F004;
pub const DESCRIPTOR_SET_FREE: u32 = 0x13F008;
pub const DESCRIPTOR_SET_UPDATE: u32 = 0x13F00C;
pub const DESCRIPTOR_SET_BIND: u32 = 0x13F010;
pub const DESCRIPTOR_SET_BIND_ADDR: u32 = 0x13F014;
pub const DESCRIPTOR_SET_BIND_SIZE: u32 = 0x13F018;

pub const COMMAND_BUFFER_BASE: u32 = 0x140000;
pub const COMMAND_BUFFER_CNTL: u32 = 0x140000;
pub const COMMAND_BUFFER_ALLOC: u32 = 0x140004;
pub const COMMAND_BUFFER_FREE: u32 = 0x140008;
pub const COMMAND_BUFFER_BEGIN: u32 = 0x14000C;
pub const COMMAND_BUFFER_END: u32 = 0x140010;
pub const COMMAND_BUFFER_SUBMIT: u32 = 0x140014;
pub const COMMAND_BUFFER_RESET: u32 = 0x140018;
pub const COMMAND_BUFFER_ADDR: u32 = 0x14001C;
pub const COMMAND_BUFFER_SIZE: u32 = 0x140020;

pub const QUERY_POOL_BASE: u32 = 0x141000;
pub const QUERY_POOL_CNTL: u32 = 0x141000;
pub const QUERY_POOL_ALLOC: u32 = 0x141004;
pub const QUERY_POOL_FREE: u32 = 0x141008;
pub const QUERY_POOL_RESET: u32 = 0x14100C;
pub const QUERY_POOL_RESULTS: u32 = 0x141010;
pub const QUERY_POOL_TYPE: u32 = 0x141014;
pub const QUERY_POOL_COUNT: u32 = 0x141018;

pub const SWAP_CHAIN_BASE: u32 = 0x142000;
pub const SWAP_CHAIN_CNTL: u32 = 0x142000;
pub const SWAP_CHAIN_CREATE: u32 = 0x142004;
pub const SWAP_CHAIN_DESTROY: u32 = 0x142008;
pub const SWAP_CHAIN_PRESENT: u32 = 0x14200C;
pub const SWAP_CHAIN_ACQUIRE: u32 = 0x142010;
pub const SWAP_CHAIN_IMAGE_COUNT: u32 = 0x142014;
pub const SWAP_CHAIN_IMAGE_ADDR: u32 = 0x142018;
pub const SWAP_CHAIN_IMAGE_SIZE: u32 = 0x14201C;
pub const SWAP_CHAIN_IMAGE_FORMAT: u32 = 0x142020;

pub const RAY_TRACING_BASE: u32 = 0x130000;
pub const RAY_TRACING_CNTL: u32 = 0x130000;
pub const RAY_TRACING_BVH_BUILD: u32 = 0x130004;
pub const RAY_TRACING_TRACE: u32 = 0x130008;
pub const RAY_TRACING_SHADER_TABLE: u32 = 0x13000C;
pub const RAY_TRACING_SBT_ADDR: u32 = 0x130010;
pub const RAY_TRACING_SBT_SIZE: u32 = 0x130014;
pub const RAY_TRACING_RAY_GEN_OFFSET: u32 = 0x130018;
pub const RAY_TRACING_MISS_OFFSET: u32 = 0x13001C;
pub const RAY_TRACING_HIT_GROUP_OFFSET: u32 = 0x130020;
pub const RAY_TRACING_MAX_RECURSION: u32 = 0x130024;

pub const MESH_SHADER_BASE: u32 = 0x131000;
pub const MESH_SHADER_CNTL: u32 = 0x131000;
pub const MESH_SHADER_TASK_CNTL: u32 = 0x131004;
pub const MESH_SHADER_MESH_CNTL: u32 = 0x131008;
pub const MESH_SHADER_AMPLIFICATION_CNTL: u32 = 0x13100C;

pub const TESS_ENHANCED_BASE: u32 = 0x136000;
pub const TESS_ENHANCED_CNTL: u32 = 0x136000;
pub const TESS_ENHANCED_FACTOR: u32 = 0x136004;
pub const TESS_ENHANCED_MODE: u32 = 0x136008;
pub const TESS_ENHANCED_PATCH: u32 = 0x13600C;

pub const VARIABLE_RATE_SHADING_BASE: u32 = 0x137000;
pub const VARIABLE_RATE_SHADING_CNTL: u32 = 0x137000;
pub const VARIABLE_RATE_SHADING_MAP: u32 = 0x137004;
pub const VARIABLE_RATE_SHADING_ATOMIC: u32 = 0x137008;

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
    pub uniform_offsets: [u32; 256],
}

#[repr(C)]
pub struct ShaderConstant {
    pub location: u32,
    pub type_: u32,
    pub count: u32,
    pub offset: u32,
}

#[repr(C)]
pub struct ShaderResource {
    pub binding: u32,
    pub resource_type: u32,
    pub address: u64,
    pub size: u32,
}

#[repr(C)]
pub struct ShaderModule {
    pub id: u32,
    pub spirv_addr: u64,
    pub spirv_size: u32,
    pub entry_point_addr: u64,
    pub entry_point_size: u32,
    pub stage: u32,
}

#[repr(C)]
pub struct DescriptorSet {
    pub id: u32,
    pub addr: u64,
    pub size: u32,
    pub binding_count: u32,
    pub bindings: [u64; 128],
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
pub struct RayTracingPipeline {
    pub id: u32,
    pub shader_group_count: u32,
    pub shader_group_addrs: [u64; 64],
    pub shader_group_sizes: [u32; 64],
    pub max_recursion_depth: u32,
    pub traversal_addr: u64,
    pub traversal_size: u32,
    pub sbt_addr: u64,
    pub sbt_size: u32,
    pub ray_gen_offset: u32,
    pub miss_offset: u32,
    pub hit_group_offset: u32,
}

#[repr(C)]
pub struct MeshShader {
    pub id: u32,
    pub task_shader_addr: u64,
    pub task_shader_size: u32,
    pub mesh_shader_addr: u64,
    pub mesh_shader_size: u32,
    pub amplification_shader_addr: u64,
    pub amplification_shader_size: u32,
    pub vertex_attr_count: u32,
    pub vertex_attr_addrs: [u64; 32],
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
    pub compute_units: u32,
    pub shader_engines: u32,
    pub shader_arrays: u32,
    pub rasterizers: u32,
    pub backends: u32,
    pub texture_units: u32,
    pub vertex_cache_size: u32,
    pub primitive_buffer_size: u32,
    pub command_buffer_size: u32,
    pub initialized: AtomicBool,
    pub error_count: AtomicU32,
    pub ring_gfx: RingBuffer,
    pub ring_compute: RingBuffer,
    pub ring_dma: RingBuffer,
    pub shader_programs: [ShaderProgram; 32],
    pub shader_program_count: u32,
    pub shader_constants: [ShaderConstant; 512],
    pub shader_constant_count: u32,
    pub shader_resources: [ShaderResource; 128],
    pub shader_resource_count: u32,
    pub shader_modules: [ShaderModule; 64],
    pub shader_module_count: u32,
    pub descriptor_sets: [DescriptorSet; 64],
    pub descriptor_set_count: u32,
    pub command_buffers: [CommandBuffer; 64],
    pub command_buffer_count: u32,
    pub query_pools: [QueryPool; 64],
    pub query_pool_count: u32,
    pub swap_chains: [SwapChain; 8],
    pub swap_chain_count: u32,
    pub ray_tracing_pipelines: [RayTracingPipeline; 16],
    pub ray_tracing_pipeline_count: u32,
    pub mesh_shaders: [MeshShader; 16],
    pub mesh_shader_count: u32,
}

pub const RX590_VRAM_SIZE: u64 = 8 * 1024 * 1024 * 1024;
pub const RX590_GART_SIZE: u64 = 4 * 1024 * 1024 * 1024;
pub const RX590_ENGINE_CLOCK: u32 = 1545000;
pub const RX590_MEMORY_CLOCK: u32 = 8000000;
pub const RX590_COMPUTE_UNITS: u32 = 36;
pub const RX590_SHADER_ENGINES: u32 = 4;
pub const RX590_RASTERIZERS: u32 = 4;
pub const RX590_BACKENDS: u32 = 16;
pub const RX590_TEXTURE_UNITS: u32 = 72;
pub const RX590_VERTEX_CACHE_SIZE: u32 = 64 * 1024;
pub const RX590_PRIMITIVE_BUFFER_SIZE: u32 = 32 * 1024;

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new() -> Self {
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size: RX590_VRAM_SIZE,
            vram_used: AtomicU64::new(0),
            gart_size: RX590_GART_SIZE,
            engine_clock: RX590_ENGINE_CLOCK,
            memory_clock: RX590_MEMORY_CLOCK,
            compute_units: RX590_COMPUTE_UNITS,
            shader_engines: RX590_SHADER_ENGINES,
            shader_arrays: 4,
            rasterizers: RX590_RASTERIZERS,
            backends: RX590_BACKENDS,
            texture_units: RX590_TEXTURE_UNITS,
            vertex_cache_size: RX590_VERTEX_CACHE_SIZE,
            primitive_buffer_size: RX590_PRIMITIVE_BUFFER_SIZE,
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
            shader_programs: unsafe { core::mem::zeroed() },
            shader_program_count: 0,
            shader_constants: unsafe { core::mem::zeroed() },
            shader_constant_count: 0,
            shader_resources: unsafe { core::mem::zeroed() },
            shader_resource_count: 0,
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
            ray_tracing_pipelines: unsafe { core::mem::zeroed() },
            ray_tracing_pipeline_count: 0,
            mesh_shaders: unsafe { core::mem::zeroed() },
            mesh_shader_count: 0,
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
            vendor == AMD_RADEON_RX_590_VENDOR_ID as u32 && device == AMD_RADEON_RX_590_DEVICE_ID as u32 && rev != 0xFFFFFFFF
        }
    }

    pub fn soft_reset(&mut self) {
        self.write_mmio(GRBM_SOFT_RESET, 0xFFFFFFFF);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(GRBM_SOFT_RESET, 0x00000000);
        self.wait_idle_inner();
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(SRBM_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
        self.error_count.fetch_add(1, Ordering::SeqCst);
    }

    pub fn wait_idle(&self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(SRBM_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn init_ring_buffers(&mut self) {
        self.write_mmio(CP_PFFETCH_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(CP_PFFETCH_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(CP_QUEUE_THRESHOLDS, 0x00000001);
        self.write_mmio(CP_STALL_CTRL, 0x00000000);
        self.write_mmio(CP_INT_CNTL, 0x00000001);
        self.write_mmio(CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(CP_PFFETCH_RPTR, 0x00000000);
        self.write_mmio(SDMA0_RLC0_RB_BASE, (REG_VRAM_BASE + 0x2000000) as u32);
        self.write_mmio(SDMA0_RLC0_RB_BASE + 4, ((REG_VRAM_BASE + 0x2000000) >> 32) as u32);
        self.write_mmio(SDMA0_RLC0_RB_CNTL, 0x00000001);
        self.write_mmio(SDMA0_RLC0_IB_BASE, (REG_VRAM_BASE + 0x3000000) as u32);
        self.write_mmio(SDMA0_RLC0_IB_BASE + 4, ((REG_VRAM_BASE + 0x3000000) >> 32) as u32);
        self.write_mmio(SDMA0_RLC0_IB_CNTL, 0x00000001);
        self.write_mmio(SDMA1_RLC0_RB_BASE, (REG_VRAM_BASE + 0x4000000) as u32);
        self.write_mmio(SDMA1_RLC0_RB_BASE + 4, ((REG_VRAM_BASE + 0x4000000) >> 32) as u32);
        self.write_mmio(SDMA1_RLC0_RB_CNTL, 0x00000001);
        self.write_mmio(SDMA1_RLC0_IB_BASE, (REG_VRAM_BASE + 0x5000000) as u32);
        self.write_mmio(SDMA1_RLC0_IB_BASE + 4, ((REG_VRAM_BASE + 0x5000000) >> 32) as u32);
        self.write_mmio(SDMA1_RLC0_IB_CNTL, 0x00000001);
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
        self.write_mmio(CP_PFFETCH_RPTR, wptr);
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
        self.setup_srbm();
        self.setup_grbm();
        self.setup_command_processor();
        self.setup_vgt();
        self.setup_primitive_assembly();
        self.setup_shader_interpolator();
        self.setup_rasterizer();
        self.setup_depth_buffer();
        self.setup_stencil();
        self.setup_blend();
        self.setup_color_buffer();
        self.setup_texture_units();
        self.setup_samplers();
        self.setup_vertex_cache();
        self.setup_index_buffer();
        self.setup_primitive_buffer();
        self.setup_shader_registers();
        self.setup_shader_units();
        self.setup_geometry_pipeline();
        self.setup_tessellation();
        self.setup_compute_pipeline();
        self.setup_framebuffers();
        self.setup_render_targets();
        self.setup_depth_stencil_buffer();
        self.setup_multisampling();
        self.setup_antialiasing();
        self.setup_depth_bounds();
        self.setup_stencil_ref();
        self.setup_blend_constants();
        self.setup_viewport();
        self.setup_scissor();
        self.setup_pcie();
        self.setup_power_play();
        self.setup_smu();
        self.setup_shader_modules();
        self.setup_descriptor_sets();
        self.setup_command_buffers();
        self.setup_query_pools();
        self.setup_swap_chains();
        self.setup_ray_tracing();
        self.setup_mesh_shader();
        self.setup_tess_enhanced();
        self.setup_variable_rate_shading();
        self.init_ring_buffers();
        self.enable_3d_acceleration();
        self.initialized.store(true, Ordering::SeqCst);
        true
    }

    fn setup_srbm(&mut self) {
        self.write_mmio(SRBM_GFX_CNTL, 0x00000000);
        self.write_mmio(SRBM_STATUS, 0x00000000);
    }

    fn setup_grbm(&mut self) {
        self.write_mmio(GRBM_CNTL, 0x00000000);
        self.write_mmio(GRBM_READ_CNTL, 0x00000000);
        self.write_mmio(GRBM_GUI_MASK, 0xFFFFFFFF);
        self.write_mmio(GRBM_GFX_INDEX, 0x00000000);
        self.write_mmio(GRBM_GFX_INDEX_INSTANCE, 0x00000000);
    }

    fn setup_command_processor(&mut self) {
        self.write_mmio(CP_ME_CNTL, 0x00000001);
        self.write_mmio(CP_ME_CNTL2, 0x00000001);
        self.write_mmio(CP_ME_CNTL3, 0x00000001);
        self.write_mmio(CP_INT_CNTL, 0x00000001);
        self.write_mmio(CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(CP_INT_ACK, 0xFFFFFFFF);
        self.write_mmio(CP_STALL_CTRL, 0x00000000);
        self.write_mmio(CP_DEBUG, 0x00000000);
        self.write_mmio(CP_PERFMON_CNTL, 0x00000000);
        self.write_mmio(CP_HWDP_CNTL, 0x00000000);
        self.write_mmio(CP_HWDP_ADDR, 0x00000000);
        self.write_mmio(CP_HWDP_DATA, 0x00000000);
        self.write_mmio(CP_ALWAYS_ON_CNTL, 0x00000001);
        self.write_mmio(CP_ALWAYS_ON_CNTL2, 0x00000001);
        self.write_mmio(CP_QUEUE_THRESHOLDS, 0x00000001);
        self.write_mmio(CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(CP_PFFETCH_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(CP_PFFETCH_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(CP_PFFETCH_RPTR, 0x00000000);
        self.write_mmio(CP_PFFETCH_DATA, 0x00000000);
        self.write_mmio(CP_PFFETCH_STATUS, 0x00000000);
        self.write_mmio(CP_INTERRUPT_STATUS, 0xFFFFFFFF);
        self.write_mmio(CP_INTERRUPT_ACK, 0xFFFFFFFF);
        self.write_mmio(CP_INTERRUPT_MASK, 0xFFFFFFFF);
        self.write_mmio(CP_INTERRUPT_MC_STATUS, 0x00000000);
    }

    fn setup_vgt(&mut self) {
        self.write_mmio(VGT_CNTL, 0x00000001);
        self.write_mmio(VGT_PRIMITIVE_TYPE, 0x00000000);
        self.write_mmio(VGT_SHADER_STAGES_EN, 0x0000000F);
        self.write_mmio(VGT_GS_OFFLOAD, 0x00000001);
        self.write_mmio(VGT_GS_OFFLOAD_2, 0x00000001);
        self.write_mmio(VGT_GSVS_RING_OFFSET_0, REG_VRAM_BASE + 0x100000);
        self.write_mmio(VGT_GSVS_RING_OFFSET_1, REG_VRAM_BASE + 0x200000);
        self.write_mmio(VGT_GSVS_RING_OFFSET_2, REG_VRAM_BASE + 0x300000);
        self.write_mmio(VGT_GSVS_RING_OFFSET_3, REG_VRAM_BASE + 0x400000);
        self.write_mmio(VGT_GSVS_RING_SIZE, 0x00001000);
        self.write_mmio(VGT_CONFIG, 0x00000001);
        self.write_mmio(VGT_VERTEX_REUSE_BLOCK_CNTL, 0x00000010);
        self.write_mmio(VGT_MULTI_PRIM_IB_RESET_INDX, 0xFFFFFFFF);
        self.write_mmio(VGT_MULTI_PRIM_IB_RESET_INDX_2, 0xFFFFFFFF);
    }

    fn setup_primitive_assembly(&mut self) {
        self.write_mmio(PA_CL_CLIP_CNTL, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL_2, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL_3, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL_4, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL_5, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL_6, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL_7, 0x00000001);
        self.write_mmio(PA_CL_VPORT_XSCALE_0, 0x3F800000);
        self.write_mmio(PA_CL_VPORT_XOFFSET_0, 0x00000000);
        self.write_mmio(PA_CL_VPORT_YSCALE_0, 0x3F800000);
        self.write_mmio(PA_CL_VPORT_YOFFSET_0, 0x00000000);
        self.write_mmio(PA_CL_VPORT_ZSCALE_0, 0x3F800000);
        self.write_mmio(PA_CL_VPORT_ZOFFSET_0, 0x00000000);
    }

    fn setup_shader_interpolator(&mut self) {
        self.write_mmio(0x8D70, 0x00000001);
        self.write_mmio(0x8D74, 0x00000001);
        self.write_mmio(0x8D78, 0x00000001);
        self.write_mmio(0x8D7C, 0x00000001);
        self.write_mmio(0x8D80, 0x00000001);
        self.write_mmio(0x8D84, 0x00000001);
        self.write_mmio(0x8D88, 0x00000001);
        self.write_mmio(0x8D8C, 0x00000001);
        self.write_mmio(0x8D90, 0x00000001);
        self.write_mmio(0x8D94, 0x00000001);
        self.write_mmio(0x8D98, 0x00000001);
        self.write_mmio(0x8D9C, 0x00000001);
    }

    fn setup_rasterizer(&mut self) {
        self.write_mmio(PA_SU_SC_MODE_CNTL, 0x00000001);
        self.write_mmio(PA_SC_MODE_CNTL_0, 0x00000001);
        self.write_mmio(PA_SC_MODE_CNTL_1, 0x00000001);
        self.write_mmio(PA_SC_CLIP_CNTL, 0x00000001);
        self.write_mmio(PA_SC_VPORT_ZRANGE_0, 0x00000000);
        self.write_mmio(PA_SC_VPORT_ZRANGE_1, 0x3F800000);
        self.write_mmio(PA_SC_VPORT_XSCALE_0, 0x3F800000);
        self.write_mmio(PA_SC_VPORT_XOFFSET_0, 0x00000000);
        self.write_mmio(PA_SC_VPORT_YSCALE_0, 0x3F800000);
        self.write_mmio(PA_SC_VPORT_YOFFSET_0, 0x00000000);
        self.write_mmio(PA_SC_AA_CONFIG, 0x00000001);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_0, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_1, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_2, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_3, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_4, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_5, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_6, 0x00000000);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_7, 0x00000000);
    }

    fn setup_depth_buffer(&mut self) {
        self.write_mmio(DB_SHADER_CONTROL, 0x00000001);
        self.write_mmio(DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(DB_DEPTH_BOUNDS, 0x00000000);
        self.write_mmio(DB_DEPTH_SLICE, 0x00000000);
        self.write_mmio(DB_DEPTH_VIEW, 0x00000000);
        self.write_mmio(DB_HTILE_SURFACE, REG_VRAM_BASE + 0x800000);
        self.write_mmio(DB_DEPTH_SIZE, self.vram_size as u32);
        self.write_mmio(DB_Z_INFO, 0x00000001);
        self.write_mmio(DB_Z_READ_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_Z_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_DEPTH_CLEAR_LO, 0x00000000);
        self.write_mmio(DB_DEPTH_CLEAR_HI, 0x00000000);
        self.write_mmio(DB_EQ_MASK, 0x00000000);
        self.write_mmio(DB_HIZ_OFFSET, REG_VRAM_BASE + 0x880000);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(DB_STENCIL_CONTROL, 0x00000001);
        self.write_mmio(DB_STENCIL_READ_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCIL_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCIL_REF, 0x00000000);
        self.write_mmio(DB_STENCILREFMASK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCILREFMASK_BACK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCILCLEAR, 0x00000000);
        self.write_mmio(DB_STENCIL_CLEAR, 0x00000000);
        self.write_mmio(DB_STENCIL_CLEAR_LO, 0x00000000);
        self.write_mmio(DB_STENCIL_CLEAR_HI, 0x00000000);
    }

    fn setup_blend(&mut self) {
        self.write_mmio(CB_BLEND_CNTL, 0x00000001);
        self.write_mmio(CB_BLEND_RED, 0x00000101);
        self.write_mmio(CB_BLEND_GREEN, 0x00000101);
        self.write_mmio(CB_BLEND_BLUE, 0x00000101);
        self.write_mmio(CB_BLEND_ALPHA, 0x00000101);
        self.write_mmio(CB_COLOR_CONTROL, 0x00000001);
    }

    fn setup_color_buffer(&mut self) {
        self.write_mmio(CB_TARGET_MASK, 0x00000001);
        self.write_mmio(CB_SHADER_MASK, 0x00000001);
        self.write_mmio(CB_COLOR0_BASE, REG_VRAM_BASE + 0x900000);
        self.write_mmio(CB_COLOR0_SIZE, self.vram_size as u32);
        self.write_mmio(CB_COLOR0_PITCH, 1024);
        self.write_mmio(CB_COLOR0_SLICE, 0x00000000);
        self.write_mmio(CB_COLOR0_INFO, 0x00000001);
        self.write_mmio(CB_COLOR0_VIEW, 0x00000000);
        self.write_mmio(CB_COLOR0_ATTRIB, 0x00000000);
        self.write_mmio(CB_COLOR0_CLEAR, 0x00000000);
        self.write_mmio(CB_COLOR0_CLEAR_DW0, 0x00000000);
        self.write_mmio(CB_COLOR0_CLEAR_DW1, 0x00000000);
        self.write_mmio(CB_COLOR1_BASE, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(CB_COLOR1_SIZE, self.vram_size as u32);
        self.write_mmio(CB_COLOR1_PITCH, 1024);
        self.write_mmio(CB_COLOR1_SLICE, 0x00000000);
        self.write_mmio(CB_COLOR1_INFO, 0x00000001);
        self.write_mmio(CB_COLOR1_VIEW, 0x00000000);
        self.write_mmio(CB_COLOR1_ATTRIB, 0x00000000);
        self.write_mmio(CB_COLOR1_CLEAR, 0x00000000);
        self.write_mmio(CB_COLOR2_BASE, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(CB_COLOR2_SIZE, self.vram_size as u32);
        self.write_mmio(CB_COLOR2_PITCH, 1024);
        self.write_mmio(CB_COLOR2_SLICE, 0x00000000);
        self.write_mmio(CB_COLOR2_INFO, 0x00000001);
        self.write_mmio(CB_COLOR2_VIEW, 0x00000000);
        self.write_mmio(CB_COLOR2_ATTRIB, 0x00000000);
        self.write_mmio(CB_COLOR2_CLEAR, 0x00000000);
        self.write_mmio(CB_COLOR3_BASE, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(CB_COLOR3_SIZE, self.vram_size as u32);
        self.write_mmio(CB_COLOR3_PITCH, 1024);
        self.write_mmio(CB_COLOR3_SLICE, 0x00000000);
        self.write_mmio(CB_COLOR3_INFO, 0x00000001);
        self.write_mmio(CB_COLOR3_VIEW, 0x00000000);
        self.write_mmio(CB_COLOR3_ATTRIB, 0x00000000);
        self.write_mmio(CB_COLOR3_CLEAR, 0x00000000);
    }

    fn setup_texture_units(&mut self) {
        for i in 0..self.texture_units {
            self.write_mmio(TA_CNTL_AUX + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_1 + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_2 + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_3 + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_4 + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_5 + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_6 + i * 4, 0x00000001);
            self.write_mmio(TA_CNTL_AUX_7 + i * 4, 0x00000001);
        }
    }

    fn setup_samplers(&mut self) {
        for i in 0..self.texture_units {
            self.write_mmio(0x9100 + i * 4, 0x00000001);
            self.write_mmio(0x9100 + i * 4 + 0x1000, 0x00000001);
            self.write_mmio(0x9100 + i * 4 + 0x2000, 0x00000001);
            self.write_mmio(0x9100 + i * 4 + 0x3000, 0x00000001);
        }
    }

    fn setup_vertex_cache(&mut self) {
        self.write_mmio(0x8B18, self.vertex_cache_size);
        self.write_mmio(0x8B1C, self.vertex_cache_size);
        self.write_mmio(0x8B20, self.vertex_cache_size);
    }

    fn setup_index_buffer(&mut self) {
        self.write_mmio(0x8B1C, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(0x8B20, self.primitive_buffer_size);
        self.write_mmio(0x8B24, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(0x8B28, self.primitive_buffer_size);
    }

    fn setup_primitive_buffer(&mut self) {
        self.write_mmio(0x8B24, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(0x8B28, self.primitive_buffer_size);
        self.write_mmio(0x8B2C, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(0x8B30, self.primitive_buffer_size);
    }

    fn setup_shader_registers(&mut self) {
        for i in 0..64 {
            self.write_mmio(0x8C00 + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(0x8D00 + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(0x8E00 + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(0x8F00 + i * 4, 0x00000000);
        }
    }

    fn setup_shader_units(&mut self) {
        self.write_mmio(SHADER_CNTL, 0x00000001);
        self.write_mmio(SHADER_VTX_CNTL, 0x00000001);
        self.write_mmio(SHADER_FRAG_CNTL, 0x00000001);
        self.write_mmio(SHADER_GEOM_CNTL, 0x00000001);
        self.write_mmio(SHADER_COMPUTE_CNTL, 0x00000001);
        self.write_mmio(SHADER_TESS_CNTL, 0x00000001);
        self.write_mmio(SHADER_UNIFORM_CNTL, 0x00000001);
        self.write_mmio(SHADER_ATTRIB_CNTL, 0x00000001);
        for i in 0..64 {
            self.write_mmio(SHADER_CONSTANT_BASE + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(SHADER_SAMPLER_BASE + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(SHADER_RESOURCE_BASE + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(SHADER_EXPORT_BASE + i * 4, 0x00000000);
        }
        for i in 0..64 {
            self.write_mmio(SHADER_STREAM_OUT_BASE + i * 4, 0x00000000);
        }
    }

    fn setup_geometry_pipeline(&mut self) {
        self.write_mmio(0x8B30, 0x00000001);
        self.write_mmio(0x8B34, 0x00000001);
        self.write_mmio(0x8B38, 0x00000001);
        self.write_mmio(0x8B3C, 0x00000001);
        self.write_mmio(0x8B40, 0x00000001);
        self.write_mmio(0x8B44, 0x00000001);
        self.write_mmio(0x8B48, 0x00000001);
        self.write_mmio(0x8B4C, 0x00000001);
        self.write_mmio(0x8B50, 0x00000001);
        self.write_mmio(0x8B54, 0x00000001);
    }

    fn setup_tessellation(&mut self) {
        self.write_mmio(0x8B58, 0x00000001);
        self.write_mmio(0x8B5C, 0x00000001);
        self.write_mmio(0x8B60, 0x00000001);
        self.write_mmio(0x8B64, 0x00000001);
        self.write_mmio(0x8B68, 0x00000001);
        self.write_mmio(0x8B6C, 0x00000001);
        self.write_mmio(0x8B70, 0x00000001);
        self.write_mmio(0x8B74, 0x00000001);
        self.write_mmio(0x8B78, 0x00000001);
        self.write_mmio(0x8B7C, 0x00000001);
    }

    fn setup_compute_pipeline(&mut self) {
        self.write_mmio(0x8B80, 0x00000001);
        self.write_mmio(0x8B84, 0x00000001);
        self.write_mmio(0x8B88, 0x00000001);
        self.write_mmio(0x8B8C, 0x00000001);
        self.write_mmio(0x8B90, 0x00000001);
        self.write_mmio(0x8B94, 0x00000001);
        self.write_mmio(0x8B98, 0x00000001);
        self.write_mmio(0x8B9C, 0x00000001);
        self.write_mmio(0x8BA0, 0x00000001);
        self.write_mmio(0x8BA4, 0x00000001);
        self.write_mmio(0x8BA8, 0x00000001);
        self.write_mmio(0x8BAC, 0x00000001);
        self.write_mmio(0x8BB0, 0x00000001);
        self.write_mmio(0x8BB4, 0x00000001);
        self.write_mmio(0x8BB8, 0x00000001);
        self.write_mmio(0x8BBC, 0x00000001);
    }

    fn setup_framebuffers(&mut self) {
        self.write_mmio(0x8BC0, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(0x8BC4, self.vram_size as u32);
        self.write_mmio(0x8BC8, REG_VRAM_BASE + 0xD00000);
        self.write_mmio(0x8BCC, self.vram_size as u32);
        self.write_mmio(0x8BD0, REG_VRAM_BASE + 0xE00000);
        self.write_mmio(0x8BD4, self.vram_size as u32);
        self.write_mmio(0x8BD8, REG_VRAM_BASE + 0xF00000);
        self.write_mmio(0x8BDC, self.vram_size as u32);
    }

    fn setup_render_targets(&mut self) {
        self.write_mmio(0x8BE0, REG_VRAM_BASE + 0x1000000);
        self.write_mmio(0x8BE4, self.vram_size as u32);
        self.write_mmio(0x8BE8, REG_VRAM_BASE + 0x1100000);
        self.write_mmio(0x8BEC, self.vram_size as u32);
        self.write_mmio(0x8BF0, REG_VRAM_BASE + 0x1200000);
        self.write_mmio(0x8BF4, self.vram_size as u32);
        self.write_mmio(0x8BF8, REG_VRAM_BASE + 0x1300000);
        self.write_mmio(0x8BFC, self.vram_size as u32);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(0x8C00, REG_VRAM_BASE + 0x1400000);
        self.write_mmio(0x8C04, self.vram_size as u32);
        self.write_mmio(0x8C08, REG_VRAM_BASE + 0x1500000);
        self.write_mmio(0x8C0C, self.vram_size as u32);
        self.write_mmio(0x8C10, REG_VRAM_BASE + 0x1600000);
        self.write_mmio(0x8C14, self.vram_size as u32);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(0x8C18, 0x00000004);
        self.write_mmio(0x8C1C, 0x00000004);
        self.write_mmio(0x8C20, 0x00000004);
        self.write_mmio(0x8C24, 0x00000004);
        self.write_mmio(0x8C28, 0x00000004);
        self.write_mmio(0x8C2C, 0x00000004);
        self.write_mmio(0x8C30, 0x00000004);
        self.write_mmio(0x8C34, 0x00000004);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(0x8C38, 0x00000001);
        self.write_mmio(0x8C3C, 0x00000001);
        self.write_mmio(0x8C40, 0x00000001);
        self.write_mmio(0x8C44, 0x00000001);
        self.write_mmio(0x8C48, 0x00000001);
        self.write_mmio(0x8C4C, 0x00000001);
        self.write_mmio(0x8C50, 0x00000001);
        self.write_mmio(0x8C54, 0x00000001);
    }

    fn setup_depth_bounds(&mut self) {
        self.write_mmio(0x8C58, 0x00000000);
        self.write_mmio(0x8C5C, 0x00000000);
        self.write_mmio(0x8C60, 0x00000000);
        self.write_mmio(0x8C64, 0x00000000);
    }

    fn setup_stencil_ref(&mut self) {
        self.write_mmio(0x8C68, 0x00000000);
        self.write_mmio(0x8C6C, 0x00000000);
        self.write_mmio(0x8C70, 0x00000000);
        self.write_mmio(0x8C74, 0x00000000);
    }

    fn setup_blend_constants(&mut self) {
        self.write_mmio(0x8C78, 0x00000000);
        self.write_mmio(0x8C7C, 0x00000000);
        self.write_mmio(0x8C80, 0x00000000);
        self.write_mmio(0x8C84, 0x00000000);
        self.write_mmio(0x8C88, 0x00000000);
        self.write_mmio(0x8C8C, 0x00000000);
        self.write_mmio(0x8C90, 0x00000000);
        self.write_mmio(0x8C94, 0x00000000);
    }

    fn setup_viewport(&mut self) {
        self.write_mmio(0x8C98, 0x00000000);
        self.write_mmio(0x8C9C, 0x00000000);
        self.write_mmio(0x8CA0, 0x00000000);
        self.write_mmio(0x8CA4, 0x00000000);
        self.write_mmio(0x8CA8, 0x00000000);
        self.write_mmio(0x8CAC, 0x00000000);
        self.write_mmio(0x8CB0, 0x00000000);
        self.write_mmio(0x8CB4, 0x00000000);
    }

    fn setup_scissor(&mut self) {
        self.write_mmio(0x8CB8, 0x00000000);
        self.write_mmio(0x8CBC, 0x00000000);
        self.write_mmio(0x8CC0, 0x00000000);
        self.write_mmio(0x8CC4, 0x00000000);
        self.write_mmio(0x8CC8, 0x00000000);
        self.write_mmio(0x8CCC, 0x00000000);
        self.write_mmio(0x8CD0, 0x00000000);
        self.write_mmio(0x8CD4, 0x00000000);
    }

    fn setup_pcie(&mut self) {
        self.write_mmio(PCIE_LC_CNTL, 0x00000001);
        self.write_mmio(PCIE_LC_STATUS, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL2, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL3, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL4, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL5, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL6, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL7, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL8, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL9, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL10, 0x00000000);
        self.write_mmio(PCIE_LC_CNTL11, 0x00000000);
    }

    fn setup_power_play(&mut self) {
        self.write_mmio(POWER_PLAY_SCLK_DPM, 0x00000001);
        self.write_mmio(POWER_PLAY_MCLK_DPM, 0x00000001);
        self.write_mmio(POWER_PLAY_VDD_CURR_DEP, 0x00000001);
        self.write_mmio(POWER_PLAY_SCLK_PW, 0x00000001);
        self.write_mmio(POWER_PLAY_MCLK_PW, 0x00000001);
        self.write_mmio(POWER_PLAY_TEMPERATURE, 0x00000000);
        self.write_mmio(POWER_PLAY_FAN_CTRL, 0x00000001);
        self.write_mmio(POWER_PLAY_POWER_STATE, 0x00000001);
        self.write_mmio(POWER_PLAY_GFX_CLOCK_GATING, 0x00000001);
        self.write_mmio(POWER_PLAY_CP_CLOCK_GATING, 0x00000001);
        self.write_mmio(POWER_PLAY_SDMA_CLOCK_GATING, 0x00000001);
    }

    fn setup_smu(&mut self) {
        self.write_mmio(SMU_SYSTEM_CNTL, 0x00000001);
        self.write_mmio(SMU_SYSTEM_STATUS, 0x00000000);
        self.write_mmio(SMU_SYSTEM_INTERRUPT, 0x00000000);
        self.write_mmio(SMU_SYSTEM_INTERRUPT_MASK, 0xFFFFFFFF);
        self.write_mmio(SMU_SYSTEM_PWR_STATUS, 0x00000000);
        self.write_mmio(SMU_SYSTEM_PWR_CNTL, 0x00000001);
        self.write_mmio(SMU_SYSTEM_PWR_MASK, 0xFFFFFFFF);
        self.write_mmio(SMU_GFX_PWR_STATUS, 0x00000000);
        self.write_mmio(SMU_GFX_PWR_CNTL, 0x00000001);
        self.write_mmio(SMU_GFX_PWR_MASK, 0xFFFFFFFF);
        self.write_mmio(SMU_UVD_PWR_STATUS, 0x00000000);
        self.write_mmio(SMU_UVD_PWR_CNTL, 0x00000001);
        self.write_mmio(SMU_UVD_PWR_MASK, 0xFFFFFFFF);
        self.write_mmio(SMU_VCE_PWR_STATUS, 0x00000000);
        self.write_mmio(SMU_VCE_PWR_CNTL, 0x00000001);
        self.write_mmio(SMU_VCE_PWR_MASK, 0xFFFFFFFF);
    }

    fn setup_shader_modules(&mut self) {
        self.write_mmio(SHADER_MODULE_CNTL, 0x00000001);
        for i in 0..64 {
            self.write_mmio(SHADER_MODULE_CREATE + i * 4, 0x00000000);
        }
    }

    fn setup_descriptor_sets(&mut self) {
        self.write_mmio(DESCRIPTOR_SET_CNTL, 0x00000001);
        for i in 0..64 {
            self.write_mmio(DESCRIPTOR_SET_ALLOC + i * 4, 0x00000000);
        }
    }

    fn setup_command_buffers(&mut self) {
        self.write_mmio(COMMAND_BUFFER_CNTL, 0x00000001);
        for i in 0..64 {
            self.write_mmio(COMMAND_BUFFER_ALLOC + i * 4, 0x00000000);
        }
    }

    fn setup_query_pools(&mut self) {
        self.write_mmio(QUERY_POOL_CNTL, 0x00000001);
        for i in 0..64 {
            self.write_mmio(QUERY_POOL_ALLOC + i * 4, 0x00000000);
        }
    }

    fn setup_swap_chains(&mut self) {
        self.write_mmio(SWAP_CHAIN_CNTL, 0x00000001);
        for i in 0..8 {
            self.write_mmio(SWAP_CHAIN_CREATE + i * 4, 0x00000000);
        }
    }

    fn setup_ray_tracing(&mut self) {
        self.write_mmio(RAY_TRACING_CNTL, 0x00000001);
        self.write_mmio(RAY_TRACING_MAX_RECURSION, 0x00000008);
    }

    fn setup_mesh_shader(&mut self) {
        self.write_mmio(MESH_SHADER_CNTL, 0x00000001);
        self.write_mmio(MESH_SHADER_TASK_CNTL, 0x00000001);
        self.write_mmio(MESH_SHADER_MESH_CNTL, 0x00000001);
        self.write_mmio(MESH_SHADER_AMPLIFICATION_CNTL, 0x00000001);
    }

    fn setup_tess_enhanced(&mut self) {
        self.write_mmio(TESS_ENHANCED_CNTL, 0x00000001);
        self.write_mmio(TESS_ENHANCED_FACTOR, 0x00000010);
        self.write_mmio(TESS_ENHANCED_MODE, 0x00000000);
        self.write_mmio(TESS_ENHANCED_PATCH, 0x00000000);
    }

    fn setup_variable_rate_shading(&mut self) {
        self.write_mmio(VARIABLE_RATE_SHADING_CNTL, 0x00000001);
        self.write_mmio(VARIABLE_RATE_SHADING_ATOMIC, 0x00000000);
    }

    fn enable_3d_acceleration(&mut self) {
        self.write_mmio(0x0900, 0x00000001);
        self.write_mmio(0x0904, 0xFFFFFFFF);
        self.write_mmio(0x0908, 0x00000001);
        self.write_mmio(0x090C, 0x00000001);
        self.write_mmio(0x0910, 0xFFFFFFFF);
        self.write_mmio(0x0914, 0x00000001);
    }

    pub fn submit_3d_command(&mut self, command: u32, param1: u32, param2: u32, param3: u32) {
        self.write_mmio(0x1000, command);
        self.write_mmio(0x1004, param1);
        self.write_mmio(0x1008, param2);
        self.write_mmio(0x100C, param3);
        self.write_mmio(0x1010, 0x00000001);
    }

    pub fn draw_triangle_3d(&mut self, x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
        self.submit_3d_command(0x00000001, x1 as u32, y1 as u32, z1 as u32);
        self.submit_3d_command(0x00000002, x2 as u32, y2 as u32, z2 as u32);
        self.submit_3d_command(0x00000003, x3 as u32, y3 as u32, z3 as u32);
        self.submit_3d_command(0x00000004, color, 0x00000001, 0x00000000);
    }

    pub fn draw_line_3d(&mut self, x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, color: u32) {
        self.submit_3d_command(0x00000005, x1 as u32, y1 as u32, z1 as u32);
        self.submit_3d_command(0x00000006, x2 as u32, y2 as u32, z2 as u32);
        self.submit_3d_command(0x00000007, color, 0x00000001, 0x00000000);
    }

    pub fn clear_screen_3d(&mut self, color: u32) {
        self.submit_3d_command(0x00000008, color, 0x00000000, 0x00000000);
    }

    pub fn set_viewport(&mut self, x: u32, y: u32, width: u32, height: u32) {
        self.submit_3d_command(0x00000009, x, y, width);
        self.submit_3d_command(0x0000000A, height, 0x00000000, 0x00000000);
    }

    pub fn set_projection_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio((0x1100u32 + (i as u32 * 4)), matrix.m[i].to_bits());
        }
    }

    pub fn set_world_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio((0x1200u32 + (i as u32 * 4)), matrix.m[i].to_bits());
        }
    }

    pub fn set_view_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio((0x1300u32 + (i as u32 * 4)), matrix.m[i].to_bits());
        }
    }

    pub fn enable_depth_test(&mut self, enabled: bool) {
        self.write_mmio(0x1400, if enabled { 1 } else { 0 });
    }

    pub fn enable_blending(&mut self, enabled: bool) {
        self.write_mmio(0x1500, if enabled { 1 } else { 0 });
    }

    pub fn set_blend_func(&mut self, src: u32, dst: u32) {
        self.write_mmio(0x1504, src);
        self.write_mmio(0x1508, dst);
    }

    pub fn set_depth_func(&mut self, func: u32) {
        self.write_mmio(0x1404, func);
    }

    pub fn set_cull_mode(&mut self, mode: u32) {
        self.write_mmio(0x1600, mode);
    }

    pub fn set_wireframe(&mut self, enabled: bool) {
        self.write_mmio(0x1604, if enabled { 1 } else { 0 });
    }

    pub fn set_multisample_count(&mut self, count: u32) {
        self.write_mmio(0x1608, count);
    }

    pub fn enable_multisampling(&mut self, enabled: bool) {
        self.write_mmio(0x160C, if enabled { 1 } else { 0 });
    }

    pub fn enable_antialiasing(&mut self, enabled: bool) {
        self.write_mmio(0x1610, if enabled { 1 } else { 0 });
    }

    pub fn set_depth_bounds_test(&mut self, enabled: bool, min: f32, max: f32) {
        self.write_mmio(0x1614, if enabled { 1 } else { 0 });
        self.write_mmio(0x1618, min.to_bits());
        self.write_mmio(0x161C, max.to_bits());
    }

    pub fn set_stencil_test(&mut self, enabled: bool, func: u32, ref_value: u32, mask: u32) {
        self.write_mmio(0x1620, if enabled { 1 } else { 0 });
        self.write_mmio(0x1624, func);
        self.write_mmio(0x1628, ref_value);
        self.write_mmio(0x162C, mask);
    }

    pub fn set_stencil_op(&mut self, fail: u32, zfail: u32, zpass: u32) {
        self.write_mmio(0x1630, fail);
        self.write_mmio(0x1634, zfail);
        self.write_mmio(0x1638, zpass);
    }

    pub fn set_blend_equation(&mut self, rgb: u32, alpha: u32) {
        self.write_mmio(0x1640, rgb);
        self.write_mmio(0x1644, alpha);
    }

    pub fn set_blend_color(&mut self, color: u32) {
        self.write_mmio(0x1648, color);
    }

    pub fn set_logic_op(&mut self, op: u32, enabled: bool) {
        self.write_mmio(0x164C, op);
        self.write_mmio(0x1650, if enabled { 1 } else { 0 });
    }

    pub fn set_dither(&mut self, enabled: bool) {
        self.write_mmio(0x1654, if enabled { 1 } else { 0 });
    }

    pub fn set_polygon_offset(&mut self, enabled: bool, factor: f32, units: f32) {
        self.write_mmio(0x1658, if enabled { 1 } else { 0 });
        self.write_mmio(0x165C, factor.to_bits());
        self.write_mmio(0x1660, units.to_bits());
    }

    pub fn set_sample_coverage(&mut self, value: f32, invert: bool) {
        self.write_mmio(0x1664, value.to_bits());
        self.write_mmio(0x1668, if invert { 1 } else { 0 });
    }

    pub fn set_vertex_array(&mut self, buffer: u32, stride: u32, offset: u32) {
        self.write_mmio(0x1700, buffer);
        self.write_mmio(0x1704, stride);
        self.write_mmio(0x1708, offset);
    }

    pub fn set_index_array(&mut self, buffer: u32, format: u32, offset: u32) {
        self.write_mmio(0x170C, buffer);
        self.write_mmio(0x1710, format);
        self.write_mmio(0x1714, offset);
    }

    pub fn draw_arrays(&mut self, mode: u32, first: u32, count: u32) {
        self.write_mmio(0x1718, mode);
        self.write_mmio(0x171C, first);
        self.write_mmio(0x1720, count);
        self.write_mmio(0x1724, 0x00000001);
    }

    pub fn draw_elements(&mut self, mode: u32, count: u32, type_: u32, offset: u32) {
        self.write_mmio(0x1728, mode);
        self.write_mmio(0x172C, count);
        self.write_mmio(0x1730, type_);
        self.write_mmio(0x1734, offset);
        self.write_mmio(0x1738, 0x00000001);
    }

    pub fn draw_instanced(&mut self, mode: u32, first: u32, count: u32, instance_count: u32) {
        self.write_mmio(0x173C, mode);
        self.write_mmio(0x1740, first);
        self.write_mmio(0x1744, count);
        self.write_mmio(0x1748, instance_count);
        self.write_mmio(0x174C, 0x00000001);
    }

    pub fn draw_elements_instanced(&mut self, mode: u32, count: u32, type_: u32, offset: u32, instance_count: u32) {
        self.write_mmio(0x1750, mode);
        self.write_mmio(0x1754, count);
        self.write_mmio(0x1758, type_);
        self.write_mmio(0x175C, offset);
        self.write_mmio(0x1760, instance_count);
        self.write_mmio(0x1764, 0x00000001);
    }

    pub fn set_shader_program(&mut self, program_id: u32) {
        self.write_mmio(0x1800, program_id);
    }

    pub fn set_uniform_1f(&mut self, location: u32, value: f32) {
        self.write_mmio(0x1804, location);
        self.write_mmio(0x1808, value.to_bits());
    }

    pub fn set_uniform_2f(&mut self, location: u32, x: f32, y: f32) {
        self.write_mmio(0x180C, location);
        self.write_mmio(0x1810, x.to_bits());
        self.write_mmio(0x1814, y.to_bits());
    }

    pub fn set_uniform_3f(&mut self, location: u32, x: f32, y: f32, z: f32) {
        self.write_mmio(0x1818, location);
        self.write_mmio(0x181C, x.to_bits());
        self.write_mmio(0x1820, y.to_bits());
        self.write_mmio(0x1824, z.to_bits());
    }

    pub fn set_uniform_4f(&mut self, location: u32, x: f32, y: f32, z: f32, w: f32) {
        self.write_mmio(0x1828, location);
        self.write_mmio(0x182C, x.to_bits());
        self.write_mmio(0x1830, y.to_bits());
        self.write_mmio(0x1834, z.to_bits());
        self.write_mmio(0x1838, w.to_bits());
    }

    pub fn set_uniform_matrix4fv(&mut self, location: u32, matrix: &Matrix4x4) {
        self.write_mmio(0x183C, location);
        for i in 0..16 {
            self.write_mmio((0x1840u32 + (i as u32 * 4)), matrix.m[i].to_bits());
        }
    }

    pub fn bind_texture(&mut self, unit: u32, texture_id: u32) {
        self.write_mmio(0x1900, unit);
        self.write_mmio(0x1904, texture_id);
    }

    pub fn bind_framebuffer(&mut self, framebuffer_id: u32) {
        self.write_mmio(0x1910, framebuffer_id);
    }

    pub fn bind_renderbuffer(&mut self, renderbuffer_id: u32) {
        self.write_mmio(0x1920, renderbuffer_id);
    }

    pub fn create_texture(&mut self, width: u32, height: u32, format: u32) -> u32 {
        self.write_mmio(0x1A00, width);
        self.write_mmio(0x1A04, height);
        self.write_mmio(0x1A08, format);
        self.write_mmio(0x1A0C, 0x00000001);
        self.read_mmio(0x1A10)
    }

    pub fn delete_texture(&mut self, texture_id: u32) {
        self.write_mmio(0x1A20, texture_id);
    }

    pub fn upload_texture_data(&mut self, texture_id: u32, data: *const u32, size: u32) {
        self.write_mmio(0x1A30, texture_id);
        self.write_mmio(0x1A34, data as u32);
        self.write_mmio(0x1A38, size);
        self.write_mmio(0x1A3C, 0x00000001);
    }

    pub fn generate_mipmaps(&mut self, texture_id: u32) {
        self.write_mmio(0x1A40, texture_id);
        self.write_mmio(0x1A44, 0x00000001);
    }

    pub fn set_texture_filter(&mut self, texture_id: u32, min_filter: u32, mag_filter: u32) {
        self.write_mmio(0x1A50, texture_id);
        self.write_mmio(0x1A54, min_filter);
        self.write_mmio(0x1A58, mag_filter);
    }

    pub fn set_texture_wrap(&mut self, texture_id: u32, wrap_s: u32, wrap_t: u32) {
        self.write_mmio(0x1A60, texture_id);
        self.write_mmio(0x1A64, wrap_s);
        self.write_mmio(0x1A68, wrap_t);
    }

    pub fn create_renderbuffer(&mut self, width: u32, height: u32, format: u32) -> u32 {
        self.write_mmio(0x1A80, width);
        self.write_mmio(0x1A84, height);
        self.write_mmio(0x1A88, format);
        self.write_mmio(0x1A8C, 0x00000001);
        self.read_mmio(0x1A90)
    }

    pub fn delete_renderbuffer(&mut self, renderbuffer_id: u32) {
        self.write_mmio(0x1AA0, renderbuffer_id);
    }

    pub fn create_framebuffer(&mut self) -> u32 {
        self.write_mmio(0x1AB0, 0x00000001);
        self.read_mmio(0x1AB4)
    }

    pub fn delete_framebuffer(&mut self, framebuffer_id: u32) {
        self.write_mmio(0x1AC0, framebuffer_id);
    }

    pub fn attach_texture_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, texture_id: u32, mip_level: u32) {
        self.write_mmio(0x1AD0, framebuffer_id);
        self.write_mmio(0x1AD4, attachment);
        self.write_mmio(0x1AD8, texture_id);
        self.write_mmio(0x1ADC, mip_level);
        self.write_mmio(0x1AE0, 0x00000001);
    }

    pub fn attach_renderbuffer_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, renderbuffer_id: u32) {
        self.write_mmio(0x1AF0, framebuffer_id);
        self.write_mmio(0x1AF4, attachment);
        self.write_mmio(0x1AF8, renderbuffer_id);
        self.write_mmio(0x1AFC, 0x00000001);
    }

    pub fn set_draw_buffers(&mut self, count: u32, attachments: *const u32) {
        self.write_mmio(0x1B00, count);
        self.write_mmio(0x1B04, attachments as u32);
    }

    pub fn set_read_buffer(&mut self, attachment: u32) {
        self.write_mmio(0x1B10, attachment);
    }

    pub fn blit_framebuffer(&mut self, src_x0: i32, src_y0: i32, src_x1: i32, src_y1: i32, dst_x0: i32, dst_y0: i32, dst_x1: i32, dst_y1: i32, mask: u32, filter: u32) {
        self.write_mmio(0x1B20, src_x0 as u32);
        self.write_mmio(0x1B24, src_y0 as u32);
        self.write_mmio(0x1B28, src_x1 as u32);
        self.write_mmio(0x1B2C, src_y1 as u32);
        self.write_mmio(0x1B30, dst_x0 as u32);
        self.write_mmio(0x1B34, dst_y0 as u32);
        self.write_mmio(0x1B38, dst_x1 as u32);
        self.write_mmio(0x1B3C, dst_y1 as u32);
        self.write_mmio(0x1B40, mask);
        self.write_mmio(0x1B44, filter);
        self.write_mmio(0x1B48, 0x00000001);
    }

    pub fn create_shader_program(&mut self, vs_addr: u64, fs_addr: u64, gs_addr: u64, tc_addr: u64, te_addr: u64, cs_addr: u64) -> u32 {
        let id = self.shader_program_count;
        if id < 32 {
            self.shader_programs[id as usize] = ShaderProgram {
                id,
                vertex_shader_addr: vs_addr,
                fragment_shader_addr: fs_addr,
                geometry_shader_addr: gs_addr,
                tessellation_ctrl_addr: tc_addr,
                tessellation_eval_addr: te_addr,
                compute_shader_addr: cs_addr,
                uniform_count: 0,
                uniform_offsets: [0; 256],
            };
            self.shader_program_count += 1;
        }
        id
    }

    pub fn delete_shader_program(&mut self, program_id: u32) {
        if program_id < 32 {
            self.shader_programs[program_id as usize] = ShaderProgram {
                id: 0,
                vertex_shader_addr: 0,
                fragment_shader_addr: 0,
                geometry_shader_addr: 0,
                tessellation_ctrl_addr: 0,
                tessellation_eval_addr: 0,
                compute_shader_addr: 0,
                uniform_count: 0,
                uniform_offsets: [0; 256],
            };
        }
    }

    pub fn load_vertex_shader(&mut self, program_id: u32, addr: u64, size: u32) {
        if program_id < 32 {
            self.shader_programs[program_id as usize].vertex_shader_addr = addr;
            self.write_mmio(0x2000 + program_id * 4, addr as u32);
            self.write_mmio(0x2040 + program_id * 4, size);
        }
    }

    pub fn load_fragment_shader(&mut self, program_id: u32, addr: u64, size: u32) {
        if program_id < 32 {
            self.shader_programs[program_id as usize].fragment_shader_addr = addr;
            self.write_mmio(0x2080 + program_id * 4, addr as u32);
            self.write_mmio(0x20C0 + program_id * 4, size);
        }
    }

    pub fn load_geometry_shader(&mut self, program_id: u32, addr: u64, size: u32) {
        if program_id < 32 {
            self.shader_programs[program_id as usize].geometry_shader_addr = addr;
            self.write_mmio(0x2100 + program_id * 4, addr as u32);
            self.write_mmio(0x2140 + program_id * 4, size);
        }
    }

    pub fn load_compute_shader(&mut self, program_id: u32, addr: u64, size: u32) {
        if program_id < 32 {
            self.shader_programs[program_id as usize].compute_shader_addr = addr;
            self.write_mmio(0x2180 + program_id * 4, addr as u32);
            self.write_mmio(0x21C0 + program_id * 4, size);
        }
    }

    pub fn compile_shader(&mut self, program_id: u32) -> bool {
        if program_id < 32 {
            self.write_mmio(0x2200, program_id);
            self.write_mmio(0x2204, 0x00000001);
            true
        } else {
            false
        }
    }

    pub fn bind_shader_resources(&mut self, program_id: u32) {
        if program_id < 32 {
            self.write_mmio(0x2240, program_id);
            self.write_mmio(0x2244, self.shader_resource_count);
            for i in 0..self.shader_resource_count {
                self.write_mmio(0x2248 + i * 4, self.shader_resources[i as usize].binding);
                self.write_mmio(0x2248 + i * 4 + 0x100, self.shader_resources[i as usize].resource_type);
                self.write_mmio_64(0x2248 + i * 4 + 0x200, self.shader_resources[i as usize].address);
                self.write_mmio(0x2248 + i * 4 + 0x204, self.shader_resources[i as usize].size);
            }
        }
    }

    pub fn set_shader_constant(&mut self, program_id: u32, location: u32, values: &[f32]) {
        if program_id < 32 && location < 512 {
            let idx = self.shader_constant_count;
            if idx < 512 {
                self.shader_constants[idx as usize] = ShaderConstant {
                    location,
                    type_: 0,
                    count: values.len() as u32,
                    offset: idx * 4,
                };
                self.shader_constant_count += 1;
            }
            self.write_mmio(0x2280 + location * 4, values[0].to_bits());
            if values.len() > 1 {
                self.write_mmio(0x2280 + location * 4 + 4, values[1].to_bits());
            }
            if values.len() > 2 {
                self.write_mmio(0x2280 + location * 4 + 8, values[2].to_bits());
            }
            if values.len() > 3 {
                self.write_mmio(0x2280 + location * 4 + 12, values[3].to_bits());
            }
        }
    }

    pub fn set_vertex_shader(&mut self, program_id: u32) {
        if program_id < 32 {
            self.write_mmio(0x22C0, program_id);
        }
    }

    pub fn set_fragment_shader(&mut self, program_id: u32) {
        if program_id < 32 {
            self.write_mmio(0x22C4, program_id);
        }
    }

    pub fn set_geometry_shader(&mut self, program_id: u32) {
        if program_id < 32 {
            self.write_mmio(0x22C8, program_id);
        }
    }

    pub fn enable_shader_stage(&mut self, stage: u32, enabled: bool) {
        self.write_mmio(0x22D0 + stage * 4, if enabled { 1 } else { 0 });
    }

    pub fn set_uniform_block(&mut self, block_index: u32, addr: u64, size: u32) {
        self.write_mmio(0x22E0, block_index);
        self.write_mmio(0x22E4, addr as u32);
        self.write_mmio(0x22E8, size);
    }

    pub fn set_shader_storage_buffer(&mut self, binding: u32, addr: u64, size: u32) {
        self.write_mmio(0x22F0, binding);
        self.write_mmio(0x22F4, addr as u32);
        self.write_mmio(0x22F8, size);
    }

    pub fn dispatch_compute(&mut self, program_id: u32, x: u32, y: u32, z: u32) {
        if program_id < 32 {
            self.write_mmio(0x2300, program_id);
            self.write_mmio(0x2304, x);
            self.write_mmio(0x2308, y);
            self.write_mmio(0x230C, z);
            self.write_mmio(0x2310, 0x00000001);
        }
    }

    pub fn create_shader_module(&mut self, spirv_addr: u64, spirv_size: u32, entry_point_addr: u64, entry_point_size: u32, stage: u32) -> u32 {
        let id = self.shader_module_count;
        if id < 64 {
            self.shader_modules[id as usize] = ShaderModule {
                id,
                spirv_addr,
                spirv_size,
                entry_point_addr,
                entry_point_size,
                stage,
            };
            self.write_mmio(SHADER_MODULE_CREATE, id);
            self.write_mmio_64(SHADER_MODULE_SPIRV_ADDR, spirv_addr);
            self.write_mmio(SHADER_MODULE_SPIRV_SIZE, spirv_size);
            self.write_mmio_64(SHADER_MODULE_ENTRY_POINT, entry_point_addr);
            self.write_mmio(SHADER_MODULE_ENTRY_POINT + 4, entry_point_size);
            self.shader_module_count += 1;
        }
        id
    }

    pub fn destroy_shader_module(&mut self, module_id: u32) {
        if module_id < 64 {
            self.shader_modules[module_id as usize] = ShaderModule {
                id: 0,
                spirv_addr: 0,
                spirv_size: 0,
                entry_point_addr: 0,
                entry_point_size: 0,
                stage: 0,
            };
            self.write_mmio(SHADER_MODULE_DESTROY, module_id);
        }
    }

    pub fn create_descriptor_set(&mut self, addr: u64, size: u32, binding_count: u32, bindings: &[u64]) -> u32 {
        let id = self.descriptor_set_count;
        if id < 64 && binding_count <= 128 {
            self.descriptor_sets[id as usize] = DescriptorSet {
                id,
                addr,
                size,
                binding_count,
                bindings: {
                    let mut b = [0u64; 128];
                    for i in 0..binding_count as usize {
                        b[i] = bindings[i];
                    }
                    b
                },
            };
            self.write_mmio(DESCRIPTOR_SET_ALLOC, id);
            self.write_mmio_64(DESCRIPTOR_SET_BIND_ADDR, addr);
            self.write_mmio(DESCRIPTOR_SET_BIND_SIZE, size);
            self.descriptor_set_count += 1;
        }
        id
    }

    pub fn destroy_descriptor_set(&mut self, set_id: u32) {
        if set_id < 64 {
            self.descriptor_sets[set_id as usize] = DescriptorSet {
                id: 0,
                addr: 0,
                size: 0,
                binding_count: 0,
                bindings: [0; 128],
            };
            self.write_mmio(DESCRIPTOR_SET_FREE, set_id);
        }
    }

    pub fn update_descriptor_set(&mut self, set_id: u32) {
        if set_id < 64 {
            self.write_mmio(DESCRIPTOR_SET_UPDATE, set_id);
        }
    }

    pub fn bind_descriptor_set(&mut self, set_id: u32, slot: u32) {
        if set_id < 64 {
            self.write_mmio(DESCRIPTOR_SET_BIND, set_id);
            self.write_mmio(DESCRIPTOR_SET_BIND_ADDR + 4, slot);
        }
    }

    pub fn create_command_buffer(&mut self, addr: u64, size: u32) -> u32 {
        let id = self.command_buffer_count;
        if id < 64 {
            self.command_buffers[id as usize] = CommandBuffer {
                id,
                addr,
                size,
                offset: AtomicU32::new(0),
                submitted: AtomicBool::new(false),
            };
            self.write_mmio(COMMAND_BUFFER_ALLOC, id);
            self.write_mmio_64(COMMAND_BUFFER_ADDR, addr);
            self.write_mmio(COMMAND_BUFFER_SIZE, size);
            self.command_buffer_count += 1;
        }
        id
    }

    pub fn destroy_command_buffer(&mut self, cb_id: u32) {
        if cb_id < 64 {
            self.command_buffers[cb_id as usize] = CommandBuffer {
                id: 0,
                addr: 0,
                size: 0,
                offset: AtomicU32::new(0),
                submitted: AtomicBool::new(false),
            };
            self.write_mmio(COMMAND_BUFFER_FREE, cb_id);
        }
    }

    pub fn begin_command_buffer(&mut self, cb_id: u32) {
        if cb_id < 64 {
            self.write_mmio(COMMAND_BUFFER_BEGIN, cb_id);
        }
    }

    pub fn end_command_buffer(&mut self, cb_id: u32) {
        if cb_id < 64 {
            self.write_mmio(COMMAND_BUFFER_END, cb_id);
        }
    }

    pub fn submit_command_buffer(&mut self, cb_id: u32) {
        if cb_id < 64 {
            self.write_mmio(COMMAND_BUFFER_SUBMIT, cb_id);
            self.command_buffers[cb_id as usize].submitted.store(true, Ordering::SeqCst);
        }
    }

    pub fn reset_command_buffer(&mut self, cb_id: u32) {
        if cb_id < 64 {
            self.write_mmio(COMMAND_BUFFER_RESET, cb_id);
            self.command_buffers[cb_id as usize].offset.store(0, Ordering::SeqCst);
            self.command_buffers[cb_id as usize].submitted.store(false, Ordering::SeqCst);
        }
    }

    pub fn create_query_pool(&mut self, addr: u64, count: u32, query_type: u32, results_addr: u64, results_size: u32) -> u32 {
        let id = self.query_pool_count;
        if id < 64 {
            self.query_pools[id as usize] = QueryPool {
                id,
                addr,
                count,
                query_type,
                results_addr,
                results_size,
            };
            self.write_mmio(QUERY_POOL_ALLOC, id);
            self.write_mmio_64(QUERY_POOL_ALLOC + 4, addr);
            self.write_mmio(QUERY_POOL_TYPE, query_type);
            self.write_mmio(QUERY_POOL_COUNT, count);
            self.write_mmio_64(QUERY_POOL_RESULTS, results_addr);
            self.query_pool_count += 1;
        }
        id
    }

    pub fn destroy_query_pool(&mut self, pool_id: u32) {
        if pool_id < 64 {
            self.query_pools[pool_id as usize] = QueryPool {
                id: 0,
                addr: 0,
                count: 0,
                query_type: 0,
                results_addr: 0,
                results_size: 0,
            };
            self.write_mmio(QUERY_POOL_FREE, pool_id);
        }
    }

    pub fn reset_query_pool(&mut self, pool_id: u32) {
        if pool_id < 64 {
            self.write_mmio(QUERY_POOL_RESET, pool_id);
        }
    }

    pub fn get_query_pool_results(&mut self, pool_id: u32) -> u64 {
        if pool_id < 64 {
            self.write_mmio(QUERY_POOL_RESULTS, pool_id);
            self.read_mmio_64(0x141010)
        } else {
            0
        }
    }

    pub fn create_swap_chain(&mut self, image_count: u32, image_addrs: &[u64], image_size: u32, image_format: u32) -> u32 {
        let id = self.swap_chain_count;
        if id < 8 && image_count <= 8 {
            self.swap_chains[id as usize] = SwapChain {
                id,
                image_count,
                current_image: AtomicU32::new(0),
                image_addrs: {
                    let mut a = [0u64; 8];
                    for i in 0..image_count as usize {
                        a[i] = image_addrs[i];
                    }
                    a
                },
                image_size,
                image_format,
                acquired: AtomicBool::new(false),
                presented: AtomicBool::new(false),
            };
            self.write_mmio(SWAP_CHAIN_CREATE, id);
            self.write_mmio(SWAP_CHAIN_IMAGE_COUNT, image_count);
            self.write_mmio(SWAP_CHAIN_IMAGE_SIZE, image_size);
            self.write_mmio(SWAP_CHAIN_IMAGE_FORMAT, image_format);
            self.swap_chain_count += 1;
        }
        id
    }

    pub fn destroy_swap_chain(&mut self, chain_id: u32) {
        if chain_id < 8 {
            self.swap_chains[chain_id as usize] = SwapChain {
                id: 0,
                image_count: 0,
                current_image: AtomicU32::new(0),
                image_addrs: [0; 8],
                image_size: 0,
                image_format: 0,
                acquired: AtomicBool::new(false),
                presented: AtomicBool::new(false),
            };
            self.write_mmio(SWAP_CHAIN_DESTROY, chain_id);
        }
    }

    pub fn present_swap_chain(&mut self, chain_id: u32) {
        if chain_id < 8 {
            self.write_mmio(SWAP_CHAIN_PRESENT, chain_id);
            self.swap_chains[chain_id as usize].presented.store(true, Ordering::SeqCst);
        }
    }

    pub fn acquire_swap_chain_image(&mut self, chain_id: u32) -> u32 {
        if chain_id < 8 {
            let idx = self.swap_chains[chain_id as usize].current_image.load(Ordering::SeqCst);
            self.swap_chains[chain_id as usize].acquired.store(true, Ordering::SeqCst);
            idx
        } else {
            0
        }
    }

    pub fn create_ray_tracing_pipeline(&mut self, group_addrs: &[u64], group_sizes: &[u32], max_recursion: u32, sbt_addr: u64, sbt_size: u32, ray_gen: u32, miss: u32, hit_group: u32) -> u32 {
        let id = self.ray_tracing_pipeline_count;
        if id < 16 {
            let count = group_addrs.len().min(64);
            self.ray_tracing_pipelines[id as usize] = RayTracingPipeline {
                id,
                shader_group_count: count as u32,
                shader_group_addrs: {
                    let mut a = [0u64; 64];
                    for i in 0..count {
                        a[i] = group_addrs[i];
                    }
                    a
                },
                shader_group_sizes: {
                    let mut s = [0u32; 64];
                    for i in 0..count {
                        s[i] = group_sizes.get(i).copied().unwrap_or(0);
                    }
                    s
                },
                max_recursion_depth: max_recursion,
                traversal_addr: 0,
                traversal_size: 0,
                sbt_addr,
                sbt_size,
                ray_gen_offset: ray_gen,
                miss_offset: miss,
                hit_group_offset: hit_group,
            };
            self.write_mmio(RAY_TRACING_SHADER_TABLE, id);
            self.write_mmio_64(RAY_TRACING_SBT_ADDR, sbt_addr);
            self.write_mmio(RAY_TRACING_SBT_SIZE, sbt_size);
            self.write_mmio(RAY_TRACING_MAX_RECURSION, max_recursion);
            self.ray_tracing_pipeline_count += 1;
        }
        id
    }

    pub fn destroy_ray_tracing_pipeline(&mut self, rt_id: u32) {
        if rt_id < 16 {
            self.ray_tracing_pipelines[rt_id as usize] = RayTracingPipeline {
                id: 0,
                shader_group_count: 0,
                shader_group_addrs: [0; 64],
                shader_group_sizes: [0; 64],
                max_recursion_depth: 0,
                traversal_addr: 0,
                traversal_size: 0,
                sbt_addr: 0,
                sbt_size: 0,
                ray_gen_offset: 0,
                miss_offset: 0,
                hit_group_offset: 0,
            };
        }
    }

    pub fn trace_rays(&mut self, rt_id: u32, width: u32, height: u32, depth: u32) {
        if rt_id < 16 {
            self.write_mmio(RAY_TRACING_TRACE, rt_id);
            self.write_mmio(RAY_TRACING_BVH_BUILD, width);
            self.write_mmio(RAY_TRACING_BVH_BUILD + 4, height);
            self.write_mmio(RAY_TRACING_BVH_BUILD + 8, depth);
        }
    }

    pub fn create_mesh_shader(&mut self, task_addr: u64, task_size: u32, mesh_addr: u64, mesh_size: u32, amp_addr: u64, amp_size: u32, attr_count: u32, attr_addrs: &[u64]) -> u32 {
        let id = self.mesh_shader_count;
        if id < 16 {
            let count = attr_addrs.len().min(32);
            self.mesh_shaders[id as usize] = MeshShader {
                id,
                task_shader_addr: task_addr,
                task_shader_size: task_size,
                mesh_shader_addr: mesh_addr,
                mesh_shader_size: mesh_size,
                amplification_shader_addr: amp_addr,
                amplification_shader_size: amp_size,
                vertex_attr_count: count as u32,
                vertex_attr_addrs: {
                    let mut a = [0u64; 32];
                    for i in 0..count {
                        a[i] = attr_addrs[i];
                    }
                    a
                },
            };
            self.write_mmio(MESH_SHADER_CNTL, id);
            self.mesh_shader_count += 1;
        }
        id
    }

    pub fn destroy_mesh_shader(&mut self, ms_id: u32) {
        if ms_id < 16 {
            self.mesh_shaders[ms_id as usize] = MeshShader {
                id: 0,
                task_shader_addr: 0,
                task_shader_size: 0,
                mesh_shader_addr: 0,
                mesh_shader_size: 0,
                amplification_shader_addr: 0,
                amplification_shader_size: 0,
                vertex_attr_count: 0,
                vertex_attr_addrs: [0; 32],
            };
        }
    }

    pub fn dispatch_mesh(&mut self, ms_id: u32, group_x: u32, group_y: u32, group_z: u32) {
        if ms_id < 16 {
            self.write_mmio(MESH_SHADER_TASK_CNTL, ms_id);
            self.write_mmio(MESH_SHADER_TASK_CNTL + 4, group_x);
            self.write_mmio(MESH_SHADER_TASK_CNTL + 8, group_y);
            self.write_mmio(MESH_SHADER_TASK_CNTL + 12, group_z);
        }
    }

    pub fn get_error_count(&self) -> u32 {
        self.error_count.load(Ordering::SeqCst)
    }

    pub fn is_initialized(&self) -> bool {
        self.initialized.load(Ordering::SeqCst)
    }
}
