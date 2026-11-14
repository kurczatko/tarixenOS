/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const AMD_RADEON_HD_5850_VENDOR_ID: u16 = 0x1002;
pub const AMD_RADEON_HD_5850_DEVICE_ID: u16 = 0x6899;
pub const CHIP_CYPRESS: u32 = 0x6899;
pub const EVERGREEN_ARCH_VERSION: u32 = 5;

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
pub const EVERGREEN_SRBM_SOFT_RESET: u32 = 0x00EC;
pub const EVERGREEN_SRBM_GFX_CNTL2: u32 = 0x00F0;

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
pub const EVERGREEN_CP_RB_BASE: u32 = 0x8940;
pub const EVERGREEN_CP_RB_CNTL: u32 = 0x8944;
pub const EVERGREEN_CP_RB_RPTR: u32 = 0x8948;
pub const EVERGREEN_CP_RB_WPTR: u32 = 0x894C;
pub const EVERGREEN_CP_RB_WPTR_DELAY: u32 = 0x8950;
pub const EVERGREEN_CP_IB_BASE: u32 = 0x8960;
pub const EVERGREEN_CP_IB_BUFSZ: u32 = 0x8964;

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
pub const EVERGREEN_VGT_DRAW_INITIATOR: u32 = 0x8A90;

pub const EVERGREEN_PA_SC_MODE_CNTL: u32 = 0x8CB8;
pub const EVERGREEN_PA_SC_MODE_CNTL_0: u32 = 0x8C94;
pub const EVERGREEN_PA_SC_MODE_CNTL_1: u32 = 0x8C98;
pub const EVERGREEN_PA_SC_CLIP_CNTL: u32 = 0x8C9C;
pub const EVERGREEN_PA_SC_VPORT_ZRANGE_0: u32 = 0x8CA4;
pub const EVERGREEN_PA_SC_VPORT_ZRANGE_1: u32 = 0x8CA8;
pub const EVERGREEN_PA_SC_VPORT_XSCALE_0: u32 = 0x8CAC;
pub const EVERGREEN_PA_SC_VPORT_XOFFSET_0: u32 = 0x8CB0;
pub const EVERGREEN_PA_SC_VPORT_YSCALE_0: u32 = 0x8CB4;
pub const EVERGREEN_PA_SC_VPORT_YOFFSET_0: u32 = 0x8CB8;
pub const EVERGREEN_PA_SC_GENERIC_SCISSOR: u32 = 0x8CBC;
pub const EVERGREEN_PA_SC_WINDOW_OFFSET: u32 = 0x8CC4;
pub const EVERGREEN_PA_SC_AA_CONFIG: u32 = 0x8D2C;
pub const EVERGREEN_PA_SU_VTX_CNTL: u32 = 0x8B1C;
pub const EVERGREEN_PA_SU_SC_MODE_CNTL_0: u32 = 0x8CB8;
pub const EVERGREEN_PA_CLIP_VTE_CNTL: u32 = 0x8C18;
pub const EVERGREEN_PA_CLIP_VS_OUT_CNTL: u32 = 0x8C1C;
pub const EVERGREEN_PA_CLIP_CLIP_CNTL: u32 = 0x8C20;

pub const EVERGREEN_SX_EXPORT_CNTL: u32 = 0x8D74;
pub const EVERGREEN_SX_ALPHA_TEST_CNTL: u32 = 0x8D60;
pub const EVERGREEN_SX_BLEND_OPTIMIZE_CONTROL: u32 = 0x8D30;
pub const EVERGREEN_SX_MISC: u32 = 0x8D64;

pub const EVERGREEN_DB_DEPTH_CONTROL: u32 = 0x2800;
pub const EVERGREEN_DB_DEPTH_VIEW: u32 = 0x2804;
pub const EVERGREEN_DB_DEPTH_RANGE: u32 = 0x2808;
pub const EVERGREEN_DB_STENCIL_CONTROL: u32 = 0x280C;
pub const EVERGREEN_DB_STENCIL_CLEAR: u32 = 0x2810;
pub const EVERGREEN_DB_STENCILREFMASK: u32 = 0x2814;
pub const EVERGREEN_DB_STENCILREFMASK_BF: u32 = 0x2818;
pub const EVERGREEN_DB_DEPTH_CLEAR: u32 = 0x2820;
pub const EVERGREEN_DB_DEPTH_WRITE_MASK: u32 = 0x2824;
pub const EVERGREEN_DB_SHADER_CONTROL: u32 = 0x286C;
pub const EVERGREEN_DB_EQAA: u32 = 0x29E4;

pub const EVERGREEN_CB_COLOR_CONTROL: u32 = 0x16CC;
pub const EVERGREEN_CB_COLOR0_BASE: u32 = 0x16D0;
pub const EVERGREEN_CB_COLOR0_SIZE: u32 = 0x16D4;
pub const EVERGREEN_CB_COLOR0_PITCH: u32 = 0x16D8;
pub const EVERGREEN_CB_COLOR0_SLICE: u32 = 0x16DC;
pub const EVERGREEN_CB_COLOR0_VIEW: u32 = 0x16E0;
pub const EVERGREEN_CB_COLOR0_ATTRIB: u32 = 0x16E4;
pub const EVERGREEN_CB_COLOR1_BASE: u32 = 0x16E8;
pub const EVERGREEN_CB_COLOR2_BASE: u32 = 0x16F0;
pub const EVERGREEN_CB_COLOR3_BASE: u32 = 0x16F8;
pub const EVERGREEN_CB_COLOR4_BASE: u32 = 0x1700;
pub const EVERGREEN_CB_COLOR5_BASE: u32 = 0x1708;
pub const EVERGREEN_CB_COLOR6_BASE: u32 = 0x1710;
pub const EVERGREEN_CB_COLOR7_BASE: u32 = 0x1718;
pub const EVERGREEN_CB_TARGET_MASK: u32 = 0x174C;
pub const EVERGREEN_CB_SHADER_MASK: u32 = 0x1750;
pub const EVERGREEN_CB_FMASK_CONTROL: u32 = 0x1740;

pub const EVERGREEN_SQ_VTX_BASE: u32 = 0x2C000;
pub const EVERGREEN_SQ_VTX_CONSTANT_BASE: u32 = 0x2C100;
pub const EVERGREEN_SQ_VTX_CONSTANT_CNTL: u32 = 0x2C080;
pub const EVERGREEN_SQ_VTX_CONSTANT_CACHE: u32 = 0x2C090;
pub const EVERGREEN_SQ_VTX_CNTL: u32 = 0x2C0A0;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_0: u32 = 0x2884;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_1: u32 = 0x2888;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_2: u32 = 0x288C;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_3: u32 = 0x2890;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_4: u32 = 0x2894;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_5: u32 = 0x2898;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_6: u32 = 0x289C;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_7: u32 = 0x28A0;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_8: u32 = 0x28A4;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_9: u32 = 0x28A8;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_10: u32 = 0x28AC;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_11: u32 = 0x28B0;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_12: u32 = 0x28B4;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_13: u32 = 0x28B8;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_14: u32 = 0x28BC;
pub const EVERGREEN_SQ_ALU_CONST_BUFFER_SIZE_15: u32 = 0x28C0;
pub const EVERGREEN_SQ_DYN_CNTL: u32 = 0x2C0E0;
pub const EVERGREEN_SQ_GPR_CNTL: u32 = 0x2C0E4;
pub const EVERGREEN_SQ_THREAD_TRACE_CNTL: u32 = 0x2C0E8;
pub const EVERGREEN_SQ_THREAD_TRACE_BASE: u32 = 0x2C0F0;
pub const EVERGREEN_SQ_THREAD_TRACE_SIZE: u32 = 0x2C0F4;
pub const EVERGREEN_SQ_STACK_CNTL: u32 = 0x2C0E8;
pub const EVERGREEN_SQ_DYN_BASE: u32 = 0x2C000;
pub const EVERGREEN_SQ_SCRATCH_BASE: u32 = 0x2C200;

pub const EVERGREEN_SQC_IMM_CNTL: u32 = 0x2C100;
pub const EVERGREEN_SQC_IMM_BASE: u32 = 0x2C200;

pub const EVERGREEN_SPI_VS_OUT_CNTL: u32 = 0x28680;
pub const EVERGREEN_SPI_VS_OUT_CONFIG: u32 = 0x28684;
pub const EVERGREEN_SPI_VS_OUT_DECL: u32 = 0x28688;
pub const EVERGREEN_SPI_VS_OUT_IDX: u32 = 0x2868C;
pub const EVERGREEN_SPI_VS_CNTL: u32 = 0x286D0;
pub const EVERGREEN_SPI_VS_IN_CONTROL: u32 = 0x286D4;
pub const EVERGREEN_SPI_VS_INTERFACE_CNTL: u32 = 0x286E0;
pub const EVERGREEN_SPI_PS_IN_CONTROL: u32 = 0x28800;
pub const EVERGREEN_SPI_PS_INTERFACE_CNTL: u32 = 0x28810;
pub const EVERGREEN_SPI_PS_CNTL: u32 = 0x28808;
pub const EVERGREEN_SPI_PS_CNTL_1: u32 = 0x2880C;

pub const EVERGREEN_CP_COHER_SIZE: u32 = 0x8970;
pub const EVERGREEN_CP_COHER_BASE: u32 = 0x8974;
pub const EVERGREEN_CP_COHER_CNTL: u32 = 0x8978;

pub const EVERGREEN_PCIE_LC_CNTL: u32 = 0x9000;
pub const EVERGREEN_PCIE_LC_STATUS: u32 = 0x9004;
pub const EVERGREEN_PCIE_LC_CNTL2: u32 = 0x9008;
pub const EVERGREEN_PCIE_LC_CNTL3: u32 = 0x900C;
pub const EVERGREEN_PCIE_LC_CNTL4: u32 = 0x9010;
pub const EVERGREEN_PCIE_LC_CNTL5: u32 = 0x9014;
pub const EVERGREEN_PCIE_LC_CNTL6: u32 = 0x9018;
pub const EVERGREEN_PCIE_LC_CNTL7: u32 = 0x901C;
pub const EVERGREEN_PCIE_LC_CNTL8: u32 = 0x9020;
pub const EVERGREEN_PCIE_LC_CNTL9: u32 = 0x9024;
pub const EVERGREEN_PCIE_LC_CNTL10: u32 = 0x9028;
pub const EVERGREEN_PCIE_LC_CNTL11: u32 = 0x902C;

pub const EVERGREEN_POWER_PLAY_SCLK_DPM: u32 = 0xA000;
pub const EVERGREEN_POWER_PLAY_MCLK_DPM: u32 = 0xA004;
pub const EVERGREEN_POWER_PLAY_VDD_CURR_DEP: u32 = 0xA008;
pub const EVERGREEN_POWER_PLAY_SCLK_PW: u32 = 0xA00C;
pub const EVERGREEN_POWER_PLAY_MCLK_PW: u32 = 0xA010;
pub const EVERGREEN_POWER_PLAY_TEMPERATURE: u32 = 0xA014;
pub const EVERGREEN_POWER_PLAY_FAN_CTRL: u32 = 0xA018;
pub const EVERGREEN_POWER_PLAY_POWER_STATE: u32 = 0xA01C;
pub const EVERGREEN_POWER_PLAY_GFX_CLOCK_GATING: u32 = 0xA020;
pub const EVERGREEN_POWER_PLAY_CP_CLOCK_GATING: u32 = 0xA024;

pub const EVERGREEN_SMU_SYSTEM_CNTL: u32 = 0xB000;
pub const EVERGREEN_SMU_SYSTEM_STATUS: u32 = 0xB004;
pub const EVERGREEN_SMU_SYSTEM_INTERRUPT: u32 = 0xB008;
pub const EVERGREEN_SMU_SYSTEM_INTERRUPT_MASK: u32 = 0xB00C;
pub const EVERGREEN_SMU_SYSTEM_PWR_CNTL: u32 = 0xB014;
pub const EVERGREEN_SMU_GFX_PWR_CNTL: u32 = 0xB020;
pub const EVERGREEN_SMU_UVD_PWR_CNTL: u32 = 0xB02C;
pub const EVERGREEN_SMU_VCE_PWR_CNTL: u32 = 0xB038;

pub const EVERGREEN_UVD_BASE: u32 = 0xC100;
pub const EVERGREEN_VCE_BASE: u32 = 0xC200;

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

pub const EVERGREEN_SHADER_BASE: u32 = 0x10D000;
pub const EVERGREEN_SHADER_CNTL: u32 = 0x10D000;
pub const EVERGREEN_SHADER_VTX_CNTL: u32 = 0x10D004;
pub const EVERGREEN_SHADER_FRAG_CNTL: u32 = 0x10D008;
pub const EVERGREEN_SHADER_GEOM_CNTL: u32 = 0x10D00C;
pub const EVERGREEN_SHADER_COMPUTE_CNTL: u32 = 0x10D010;
pub const EVERGREEN_SHADER_TESS_CNTL: u32 = 0x10D014;
pub const EVERGREEN_SHADER_UNIFORM_CNTL: u32 = 0x10D018;
pub const EVERGREEN_SHADER_ATTRIB_CNTL: u32 = 0x10D01C;

pub const EVERGREEN_RASTER_BASE: u32 = 0x10F000;
pub const EVERGREEN_RASTER_CNTL: u32 = 0x10F000;
pub const EVERGREEN_RASTER_CLIP_CNTL: u32 = 0x10F004;
pub const EVERGREEN_RASTER_CULL_CNTL: u32 = 0x10F008;
pub const EVERGREEN_RASTER_SCISSOR_CNTL: u32 = 0x10F00C;
pub const EVERGREEN_RASTER_MSAA_CNTL: u32 = 0x10F010;
pub const EVERGREEN_RASTER_AA_CNTL: u32 = 0x10F014;
pub const EVERGREEN_RASTER_OFFSET_CNTL: u32 = 0x10F018;

pub const EVERGREEN_DEPTH_BASE: u32 = 0x110000;
pub const EVERGREEN_DEPTH_CNTL: u32 = 0x110000;
pub const EVERGREEN_DEPTH_FUNC: u32 = 0x110004;
pub const EVERGREEN_DEPTH_BOUNDS: u32 = 0x110008;
pub const EVERGREEN_DEPTH_WRITE_MASK: u32 = 0x11000C;
pub const EVERGREEN_DEPTH_CLEAR: u32 = 0x110010;
pub const EVERGREEN_DEPTH_HIZ: u32 = 0x110014;
pub const EVERGREEN_DEPTH_STENCIL_CNTL: u32 = 0x110018;
pub const EVERGREEN_DEPTH_STENCIL_FUNC: u32 = 0x11001C;

pub const EVERGREEN_STENCIL_BASE: u32 = 0x111000;
pub const EVERGREEN_STENCIL_CNTL: u32 = 0x111000;
pub const EVERGREEN_STENCIL_FUNC: u32 = 0x111004;
pub const EVERGREEN_STENCIL_REF: u32 = 0x111008;
pub const EVERGREEN_STENCIL_MASK: u32 = 0x11100C;
pub const EVERGREEN_STENCIL_OP_FAIL: u32 = 0x111010;
pub const EVERGREEN_STENCIL_OP_ZFAIL: u32 = 0x111014;
pub const EVERGREEN_STENCIL_OP_ZPASS: u32 = 0x111018;
pub const EVERGREEN_STENCIL_CLEAR: u32 = 0x11101C;

pub const EVERGREEN_BLEND_BASE: u32 = 0x112000;
pub const EVERGREEN_BLEND_CNTL: u32 = 0x112000;
pub const EVERGREEN_BLEND_EQUATION_RGB: u32 = 0x112004;
pub const EVERGREEN_BLEND_EQUATION_ALPHA: u32 = 0x112008;
pub const EVERGREEN_BLEND_FUNC_SRC_RGB: u32 = 0x11200C;
pub const EVERGREEN_BLEND_FUNC_DST_RGB: u32 = 0x112010;
pub const EVERGREEN_BLEND_FUNC_SRC_ALPHA: u32 = 0x112014;
pub const EVERGREEN_BLEND_FUNC_DST_ALPHA: u32 = 0x112018;
pub const EVERGREEN_BLEND_COLOR: u32 = 0x11201C;
pub const EVERGREEN_BLEND_SEPARATE_CNTL: u32 = 0x112020;

pub const EVERGREEN_COLOR_BUFFER_BASE: u32 = 0x113000;
pub const EVERGREEN_COLOR0_BASE: u32 = 0x113000;
pub const EVERGREEN_COLOR0_SIZE: u32 = 0x113004;
pub const EVERGREEN_COLOR0_PITCH: u32 = 0x113008;
pub const EVERGREEN_COLOR0_INFO: u32 = 0x11300C;
pub const EVERGREEN_COLOR1_BASE: u32 = 0x113010;
pub const EVERGREEN_COLOR1_SIZE: u32 = 0x113014;
pub const EVERGREEN_COLOR1_PITCH: u32 = 0x113018;
pub const EVERGREEN_COLOR1_INFO: u32 = 0x11301C;
pub const EVERGREEN_COLOR2_BASE: u32 = 0x113020;
pub const EVERGREEN_COLOR2_SIZE: u32 = 0x113024;
pub const EVERGREEN_COLOR2_PITCH: u32 = 0x113028;
pub const EVERGREEN_COLOR2_INFO: u32 = 0x11302C;
pub const EVERGREEN_COLOR3_BASE: u32 = 0x113030;
pub const EVERGREEN_COLOR3_SIZE: u32 = 0x113034;
pub const EVERGREEN_COLOR3_PITCH: u32 = 0x113038;
pub const EVERGREEN_COLOR3_INFO: u32 = 0x11303C;

pub const EVERGREEN_FRAMEBUFFER_BASE: u32 = 0x114000;
pub const EVERGREEN_FRAMEBUFFER_ADDR: u32 = 0x114000;
pub const EVERGREEN_FRAMEBUFFER_SIZE: u32 = 0x114004;
pub const EVERGREEN_FRAMEBUFFER_PITCH: u32 = 0x114008;
pub const EVERGREEN_FRAMEBUFFER_FORMAT: u32 = 0x11400C;

pub const EVERGREEN_RENDER_TARGET_BASE: u32 = 0x115000;
pub const EVERGREEN_RENDER_TARGET_ADDR: u32 = 0x115000;
pub const EVERGREEN_RENDER_TARGET_SIZE: u32 = 0x115004;
pub const EVERGREEN_RENDER_TARGET_COUNT: u32 = 0x115008;

pub const EVERGREEN_DEPTH_STENCIL_BUFFER_BASE: u32 = 0x116000;
pub const EVERGREEN_DEPTH_STENCIL_ADDR: u32 = 0x116000;
pub const EVERGREEN_DEPTH_STENCIL_SIZE: u32 = 0x116004;

pub const EVERGREEN_MULTISAMPLE_BASE: u32 = 0x117000;
pub const EVERGREEN_MULTISAMPLE_CNTL: u32 = 0x117000;
pub const EVERGREEN_MULTISAMPLE_COUNT: u32 = 0x117004;
pub const EVERGREEN_MULTISAMPLE_SAMPLE_LOCS: u32 = 0x117008;
pub const EVERGREEN_MULTISAMPLE_COVERAGE: u32 = 0x11700C;
pub const EVERGREEN_MULTISAMPLE_RESOLVE: u32 = 0x117010;

pub const EVERGREEN_ANTIALIAS_BASE: u32 = 0x118000;
pub const EVERGREEN_ANTIALIAS_CNTL: u32 = 0x118000;
pub const EVERGREEN_ANTIALIAS_MODE: u32 = 0x118004;
pub const EVERGREEN_ANTIALIAS_SAMPLES: u32 = 0x118008;
pub const EVERGREEN_ANTIALIAS_FILTER: u32 = 0x11800C;

pub const EVERGREEN_VIEWPORT_BASE: u32 = 0x119000;
pub const EVERGREEN_VIEWPORT_XSCALE: u32 = 0x119000;
pub const EVERGREEN_VIEWPORT_XOFFSET: u32 = 0x119004;
pub const EVERGREEN_VIEWPORT_YSCALE: u32 = 0x119008;
pub const EVERGREEN_VIEWPORT_YOFFSET: u32 = 0x11900C;
pub const EVERGREEN_VIEWPORT_ZSCALE: u32 = 0x119010;
pub const EVERGREEN_VIEWPORT_ZOFFSET: u32 = 0x119014;
pub const EVERGREEN_VIEWPORT_DEPTH_RANGE: u32 = 0x119018;

pub const EVERGREEN_SCISSOR_BASE: u32 = 0x11A000;
pub const EVERGREEN_SCISSOR_X: u32 = 0x11A000;
pub const EVERGREEN_SCISSOR_Y: u32 = 0x11A004;
pub const EVERGREEN_SCISSOR_WIDTH: u32 = 0x11A008;
pub const EVERGREEN_SCISSOR_HEIGHT: u32 = 0x11A00C;
pub const EVERGREEN_SCISSOR_ENABLE: u32 = 0x11A010;

pub const EVERGREEN_VERTEX_CACHE_BASE: u32 = 0x11B000;
pub const EVERGREEN_VERTEX_CACHE_CNTL: u32 = 0x11B000;
pub const EVERGREEN_VERTEX_CACHE_SIZE: u32 = 0x11B004;
pub const EVERGREEN_VERTEX_CACHE_FLUSH: u32 = 0x11B008;

pub const EVERGREEN_INDEX_BUFFER_BASE: u32 = 0x11C000;
pub const EVERGREEN_INDEX_BUFFER_ADDR: u32 = 0x11C000;
pub const EVERGREEN_INDEX_BUFFER_SIZE: u32 = 0x11C004;
pub const EVERGREEN_INDEX_BUFFER_FORMAT: u32 = 0x11C008;

pub const EVERGREEN_PRIMITIVE_BUFFER_BASE: u32 = 0x11C010;
pub const EVERGREEN_PRIMITIVE_BUFFER_ADDR: u32 = 0x11C010;
pub const EVERGREEN_PRIMITIVE_BUFFER_SIZE: u32 = 0x11C014;

pub const EVERGREEN_UNIFORM_BUFFER_BASE: u32 = 0x11D000;
pub const EVERGREEN_UNIFORM_BUFFER_ADDR: u32 = 0x11D000;
pub const EVERGREEN_UNIFORM_BUFFER_SIZE: u32 = 0x11D004;
pub const EVERGREEN_UNIFORM_BUFFER_COUNT: u32 = 0x11D008;

pub const EVERGREEN_SHADER_STORAGE_BASE: u32 = 0x11E000;
pub const EVERGREEN_SHADER_STORAGE_ADDR: u32 = 0x11E000;
pub const EVERGREEN_SHADER_STORAGE_SIZE: u32 = 0x11E004;
pub const EVERGREEN_SHADER_STORAGE_COUNT: u32 = 0x11E008;

pub const EVERGREEN_ATOMIC_COUNTER_BASE: u32 = 0x11F000;
pub const EVERGREEN_ATOMIC_COUNTER_ADDR: u32 = 0x11F000;
pub const EVERGREEN_ATOMIC_COUNTER_COUNT: u32 = 0x11F004;

pub const EVERGREEN_QUERY_OBJECT_BASE: u32 = 0x120000;
pub const EVERGREEN_QUERY_OBJECT_ADDR: u32 = 0x120000;
pub const EVERGREEN_QUERY_OBJECT_COUNT: u32 = 0x120004;
pub const EVERGREEN_QUERY_OBJECT_TYPE: u32 = 0x120008;
pub const EVERGREEN_QUERY_OBJECT_RESULT: u32 = 0x12000C;

pub const EVERGREEN_SHADER_MODULE_BASE: u32 = 0x13E000;
pub const EVERGREEN_SHADER_MODULE_CNTL: u32 = 0x13E000;
pub const EVERGREEN_SHADER_MODULE_CREATE: u32 = 0x13E004;
pub const EVERGREEN_SHADER_MODULE_DESTROY: u32 = 0x13E008;
pub const EVERGREEN_SHADER_MODULE_SPIRV_ADDR: u32 = 0x13E00C;
pub const EVERGREEN_SHADER_MODULE_SPIRV_SIZE: u32 = 0x13E010;
pub const EVERGREEN_SHADER_MODULE_ENTRY_POINT: u32 = 0x13E014;

pub const EVERGREEN_DESCRIPTOR_SET_BASE: u32 = 0x13F000;
pub const EVERGREEN_DESCRIPTOR_SET_CNTL: u32 = 0x13F000;
pub const EVERGREEN_DESCRIPTOR_SET_ALLOC: u32 = 0x13F004;
pub const EVERGREEN_DESCRIPTOR_SET_FREE: u32 = 0x13F008;
pub const EVERGREEN_DESCRIPTOR_SET_UPDATE: u32 = 0x13F00C;
pub const EVERGREEN_DESCRIPTOR_SET_BIND: u32 = 0x13F010;
pub const EVERGREEN_DESCRIPTOR_SET_BIND_ADDR: u32 = 0x13F014;
pub const EVERGREEN_DESCRIPTOR_SET_BIND_SIZE: u32 = 0x13F018;

pub const EVERGREEN_COMMAND_BUFFER_BASE: u32 = 0x140000;
pub const EVERGREEN_COMMAND_BUFFER_CNTL: u32 = 0x140000;
pub const EVERGREEN_COMMAND_BUFFER_ALLOC: u32 = 0x140004;
pub const EVERGREEN_COMMAND_BUFFER_FREE: u32 = 0x140008;
pub const EVERGREEN_COMMAND_BUFFER_BEGIN: u32 = 0x14000C;
pub const EVERGREEN_COMMAND_BUFFER_END: u32 = 0x140010;
pub const EVERGREEN_COMMAND_BUFFER_SUBMIT: u32 = 0x140014;
pub const EVERGREEN_COMMAND_BUFFER_RESET: u32 = 0x140018;
pub const EVERGREEN_COMMAND_BUFFER_ADDR: u32 = 0x14001C;
pub const EVERGREEN_COMMAND_BUFFER_SIZE: u32 = 0x140020;

pub const EVERGREEN_QUERY_POOL_BASE: u32 = 0x141000;
pub const EVERGREEN_QUERY_POOL_CNTL: u32 = 0x141000;
pub const EVERGREEN_QUERY_POOL_ALLOC: u32 = 0x141004;
pub const EVERGREEN_QUERY_POOL_FREE: u32 = 0x141008;
pub const EVERGREEN_QUERY_POOL_RESET: u32 = 0x14100C;
pub const EVERGREEN_QUERY_POOL_RESULTS: u32 = 0x141010;
pub const EVERGREEN_QUERY_POOL_TYPE: u32 = 0x141014;
pub const EVERGREEN_QUERY_POOL_COUNT: u32 = 0x141018;

pub const EVERGREEN_SWAP_CHAIN_BASE: u32 = 0x142000;
pub const EVERGREEN_SWAP_CHAIN_CNTL: u32 = 0x142000;
pub const EVERGREEN_SWAP_CHAIN_CREATE: u32 = 0x142004;
pub const EVERGREEN_SWAP_CHAIN_DESTROY: u32 = 0x142008;
pub const EVERGREEN_SWAP_CHAIN_PRESENT: u32 = 0x14200C;
pub const EVERGREEN_SWAP_CHAIN_ACQUIRE: u32 = 0x142010;
pub const EVERGREEN_SWAP_CHAIN_IMAGE_COUNT: u32 = 0x142014;
pub const EVERGREEN_SWAP_CHAIN_IMAGE_ADDR: u32 = 0x142018;
pub const EVERGREEN_SWAP_CHAIN_IMAGE_SIZE: u32 = 0x14201C;
pub const EVERGREEN_SWAP_CHAIN_IMAGE_FORMAT: u32 = 0x142020;

pub const EVERGREEN_COMPUTE_BASE: u32 = 0x10E000;
pub const EVERGREEN_COMPUTE_CNTL: u32 = 0x10E000;
pub const EVERGREEN_COMPUTE_QUEUE: u32 = 0x10E004;
pub const EVERGREEN_COMPUTE_DISPATCH: u32 = 0x10E008;
pub const EVERGREEN_COMPUTE_SYNC: u32 = 0x10E00C;
pub const EVERGREEN_COMPUTE_ATOMIC: u32 = 0x10E010;
pub const EVERGREEN_COMPUTE_SHARED: u32 = 0x10E014;
pub const EVERGREEN_COMPUTE_LOCAL: u32 = 0x10E018;

pub const HD5850_VRAM_SIZE: u64 = 1024 * 1024 * 1024;
pub const HD5850_GART_SIZE: u64 = 512 * 1024 * 1024;
pub const HD5850_ENGINE_CLOCK: u32 = 725000;
pub const HD5850_MEMORY_CLOCK: u32 = 1000000;
pub const HD5850_STREAM_CORES: u32 = 1440;
pub const HD5850_SIMD_COUNT: u32 = 18;
pub const HD5850_ROP_COUNT: u32 = 32;
pub const HD5850_TEXTURE_UNITS: u32 = 72;
pub const HD5850_MEMORY_BUS_WIDTH: u32 = 256;
pub const HD5850_VERTEX_CACHE_SIZE: u32 = 64 * 1024;
pub const HD5850_PRIMITIVE_BUFFER_SIZE: u32 = 32 * 1024;

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
    pub descriptor_set_count: u32,
    pub descriptor_set_addrs: [u64; 32],
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
pub struct ShaderPipeline {
    pub id: u32,
    pub vs_module_id: u32,
    pub fs_module_id: u32,
    pub gs_module_id: u32,
    pub tess_ctrl_module_id: u32,
    pub tess_eval_module_id: u32,
    pub compute_module_id: u32,
    pub state_addr: u64,
    pub state_size: u32,
}

#[repr(C)]
pub struct ComputePipeline {
    pub id: u32,
    pub compute_module_id: u32,
    pub state_addr: u64,
    pub state_size: u32,
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
    pub stream_cores: u32,
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
    pub shader_pipelines: [ShaderPipeline; 16],
    pub shader_pipeline_count: u32,
    pub compute_pipelines: [ComputePipeline; 16],
    pub compute_pipeline_count: u32,
    pub descriptor_sets: [DescriptorSet; 32],
    pub descriptor_set_count: u32,
    pub command_buffers: [CommandBuffer; 32],
    pub command_buffer_count: u32,
    pub query_pools: [QueryPool; 32],
    pub query_pool_count: u32,
    pub swap_chains: [SwapChain; 8],
    pub swap_chain_count: u32,
}

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new() -> Self {
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size: HD5850_VRAM_SIZE,
            vram_used: AtomicU64::new(0),
            gart_size: HD5850_GART_SIZE,
            engine_clock: HD5850_ENGINE_CLOCK,
            memory_clock: HD5850_MEMORY_CLOCK,
            stream_cores: HD5850_STREAM_CORES,
            simd_count: HD5850_SIMD_COUNT,
            rop_count: HD5850_ROP_COUNT,
            texture_units: HD5850_TEXTURE_UNITS,
            memory_bus_width: HD5850_MEMORY_BUS_WIDTH,
            vertex_cache_size: HD5850_VERTEX_CACHE_SIZE,
            primitive_buffer_size: HD5850_PRIMITIVE_BUFFER_SIZE,
            command_buffer_size: 16 * 1024 * 1024,
            initialized: AtomicBool::new(false),
            error_count: AtomicU32::new(0),
            ring_gfx: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x100000,
                size: 16 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 0,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(0) },
            },
            ring_compute: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x2000000,
                size: 8 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 1,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(1) },
            },
            ring_dma: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x4000000,
                size: 8 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 2,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(2) },
            },
            shader_modules: unsafe { core::mem::zeroed() },
            shader_module_count: 0,
            shader_pipelines: unsafe { core::mem::zeroed() },
            shader_pipeline_count: 0,
            compute_pipelines: unsafe { core::mem::zeroed() },
            compute_pipeline_count: 0,
            descriptor_sets: unsafe { core::mem::zeroed() },
            descriptor_set_count: 0,
            command_buffers: unsafe { core::mem::zeroed() },
            command_buffer_count: 0,
            query_pools: unsafe { core::mem::zeroed() },
            query_pool_count: 0,
            swap_chains: unsafe { core::mem::zeroed() },
            swap_chain_count: 0,
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
            vendor == AMD_RADEON_HD_5850_VENDOR_ID as u32 && device == AMD_RADEON_HD_5850_DEVICE_ID as u32 && rev != 0xFFFFFFFF
        }
    }

    pub fn soft_reset(&mut self) {
        self.write_mmio(EVERGREEN_GRBM_SOFT_RESET, 0x00000001);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(EVERGREEN_GRBM_SOFT_RESET, 0x00000000);
        self.write_mmio(EVERGREEN_SRBM_SOFT_RESET, 0x00000001);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(EVERGREEN_SRBM_SOFT_RESET, 0x00000000);
        self.wait_idle_inner();
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(EVERGREEN_GRBM_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
        self.error_count.fetch_add(1, Ordering::SeqCst);
    }

    pub fn wait_idle(&self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(EVERGREEN_GRBM_STATUS);
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
        self.write_mmio(EVERGREEN_CP_RB_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(EVERGREEN_CP_RB_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(EVERGREEN_CP_RB_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_RB_WPTR, 0x00000000);
        self.write_mmio(EVERGREEN_CP_RB_RPTR, 0x00000000);
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
        self.write_mmio(EVERGREEN_CP_RB_WPTR, wptr);
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
        self.setup_pa_sc();
        self.setup_sx();
        self.setup_db();
        self.setup_cb();
        self.setup_shader();
        self.setup_rasterizer();
        self.setup_depth_buffer();
        self.setup_stencil();
        self.setup_blend();
        self.setup_color_buffer();
        self.setup_framebuffers();
        self.setup_render_targets();
        self.setup_depth_stencil_buffer();
        self.setup_multisampling();
        self.setup_antialiasing();
        self.setup_viewport();
        self.setup_scissor();
        self.setup_vertex_cache();
        self.setup_index_buffer();
        self.setup_primitive_buffer();
        self.setup_uniform_buffers();
        self.setup_shader_storage_buffers();
        self.setup_atomic_counters();
        self.setup_query_objects();
        self.setup_pcie();
        self.setup_power_play();
        self.setup_smu();
        self.init_ring_buffers();
        self.enable_3d_acceleration();
        self.initialized.store(true, Ordering::SeqCst);
        true
    }

    fn setup_grbm(&mut self) {
        self.write_mmio(EVERGREEN_GRBM_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_GRBM_READ_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_SOFT_RESET, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_GUI_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_GRBM_STATUS, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_GFX_INDEX, 0x00000000);
        self.write_mmio(EVERGREEN_GRBM_GFX_INDEX_INSTANCE, 0x00000000);
    }

    fn setup_srbm(&mut self) {
        self.write_mmio(EVERGREEN_SRBM_STATUS, 0x00000000);
        self.write_mmio(EVERGREEN_SRBM_GFX_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SRBM_SOFT_RESET, 0x00000000);
        self.write_mmio(EVERGREEN_SRBM_GFX_CNTL2, 0x00000001);
    }

    fn setup_cp(&mut self) {
        self.write_mmio(EVERGREEN_CP_ME_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_ME_CNTL2, 0x00000001);
        self.write_mmio(EVERGREEN_CP_ME_CNTL3, 0x00000001);
        self.write_mmio(EVERGREEN_CP_DEBUG, 0x00000000);
        self.write_mmio(EVERGREEN_CP_PERFMON_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_HWDP_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_ALWAYS_ON_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_ALWAYS_ON_CNTL2, 0x00000001);
        self.write_mmio(EVERGREEN_CP_QUEUE_THRESHOLDS, 0x00000001);
        self.write_mmio(EVERGREEN_CP_STALL_CTRL, 0x00000000);
        self.write_mmio(EVERGREEN_CP_INT_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_INT_ACK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(EVERGREEN_CP_PFFETCH_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(EVERGREEN_CP_PFFETCH_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(EVERGREEN_CP_PFFETCH_RPTR, 0x00000000);
        self.write_mmio(EVERGREEN_CP_INTERRUPT_STATUS, 0x00000000);
        self.write_mmio(EVERGREEN_CP_INTERRUPT_ACK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_INTERRUPT_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CP_INTERRUPT_MC_STATUS, 0x00000000);
        self.write_mmio(EVERGREEN_CP_RB_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(EVERGREEN_CP_RB_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(EVERGREEN_CP_RB_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_CP_RB_RPTR, 0x00000000);
        self.write_mmio(EVERGREEN_CP_RB_WPTR, 0x00000000);
        self.write_mmio(EVERGREEN_CP_RB_WPTR_DELAY, 0x00000000);
        self.write_mmio(EVERGREEN_CP_IB_BASE, (REG_VRAM_BASE + 0x3000000) as u32);
        self.write_mmio(EVERGREEN_CP_IB_BASE + 4, ((REG_VRAM_BASE + 0x3000000) >> 32) as u32);
        self.write_mmio(EVERGREEN_CP_IB_BUFSZ, 0x00001000);
        self.write_mmio(EVERGREEN_CP_COHER_SIZE, 0x00001000);
        self.write_mmio(EVERGREEN_CP_COHER_BASE, (REG_VRAM_BASE + 0x4000000) as u32);
        self.write_mmio(EVERGREEN_CP_COHER_BASE + 4, ((REG_VRAM_BASE + 0x4000000) >> 32) as u32);
        self.write_mmio(EVERGREEN_CP_COHER_CNTL, 0x00000001);
    }

    fn setup_vgt(&mut self) {
        self.write_mmio(EVERGREEN_VGT_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_VGT_PRIMITIVE_TYPE, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_SHADER_STAGES_EN, 0x00000007);
        self.write_mmio(EVERGREEN_VGT_GSVS_RING_OFFSET_0, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_GSVS_RING_OFFSET_1, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_GSVS_RING_OFFSET_2, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_GSVS_RING_OFFSET_3, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_GSVS_RING_SIZE, 0x00001000);
        self.write_mmio(EVERGREEN_VGT_CONFIG, 0x00000001);
        self.write_mmio(EVERGREEN_VGT_VERTEX_REUSE_BLOCK_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_VGT_MULTI_PRIM_IB_RESET_INDX, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_VGT_MULTI_PRIM_IB_RESET_INDX_2, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_VGT_GS_OFFLOAD, 0x00000000);
        self.write_mmio(EVERGREEN_VGT_GS_OFFLOAD_2, 0x00000000);
    }

    fn setup_pa_sc(&mut self) {
        self.write_mmio(EVERGREEN_PA_SC_MODE_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_PA_SC_MODE_CNTL_0, 0x00000001);
        self.write_mmio(EVERGREEN_PA_SC_MODE_CNTL_1, 0x00000001);
        self.write_mmio(EVERGREEN_PA_SC_CLIP_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_ZRANGE_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_ZRANGE_1, 0x3F800000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_XSCALE_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_XOFFSET_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_YSCALE_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_VPORT_YOFFSET_0, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_GENERIC_SCISSOR, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_WINDOW_OFFSET, 0x00000000);
        self.write_mmio(EVERGREEN_PA_SC_AA_CONFIG, 0x00000001);
        self.write_mmio(EVERGREEN_PA_SU_VTX_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_PA_CLIP_VTE_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_PA_CLIP_VS_OUT_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_PA_CLIP_CLIP_CNTL, 0x00000000);
    }

    fn setup_sx(&mut self) {
        self.write_mmio(EVERGREEN_SX_EXPORT_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SX_ALPHA_TEST_CNTL, 0x00000000);
        self.write_mmio(EVERGREEN_SX_BLEND_OPTIMIZE_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_SX_MISC, 0x00000000);
    }

    fn setup_db(&mut self) {
        self.write_mmio(EVERGREEN_DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_DB_DEPTH_VIEW, 0x00000000);
        self.write_mmio(EVERGREEN_DB_DEPTH_RANGE, 0x00000000);
        self.write_mmio(EVERGREEN_DB_STENCIL_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_DB_STENCIL_CLEAR, 0x00000000);
        self.write_mmio(EVERGREEN_DB_STENCILREFMASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_DB_STENCILREFMASK_BF, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_DB_DEPTH_CLEAR, 0x00000000);
        self.write_mmio(EVERGREEN_DB_DEPTH_WRITE_MASK, 0x00000001);
        self.write_mmio(EVERGREEN_DB_SHADER_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_DB_EQAA, 0x00000001);
    }

    fn setup_cb(&mut self) {
        self.write_mmio(EVERGREEN_CB_COLOR_CONTROL, 0x00000001);
        self.write_mmio(EVERGREEN_CB_COLOR0_BASE, REG_VRAM_BASE + 0x900000);
        self.write_mmio(EVERGREEN_CB_COLOR0_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_CB_COLOR0_PITCH, 1024);
        self.write_mmio(EVERGREEN_CB_COLOR0_SLICE, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR0_VIEW, 0x00000000);
        self.write_mmio(EVERGREEN_CB_COLOR0_ATTRIB, 0x00000001);
        self.write_mmio(EVERGREEN_CB_COLOR1_BASE, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(EVERGREEN_CB_COLOR2_BASE, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(EVERGREEN_CB_COLOR3_BASE, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(EVERGREEN_CB_COLOR4_BASE, REG_VRAM_BASE + 0xD00000);
        self.write_mmio(EVERGREEN_CB_COLOR5_BASE, REG_VRAM_BASE + 0xE00000);
        self.write_mmio(EVERGREEN_CB_COLOR6_BASE, REG_VRAM_BASE + 0xF00000);
        self.write_mmio(EVERGREEN_CB_COLOR7_BASE, REG_VRAM_BASE + 0x1000000);
        self.write_mmio(EVERGREEN_CB_TARGET_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CB_SHADER_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_CB_FMASK_CONTROL, 0x00000001);
    }

    fn setup_shader(&mut self) {
        self.write_mmio(EVERGREEN_SHADER_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_VTX_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_FRAG_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_GEOM_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_COMPUTE_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_TESS_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_UNIFORM_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SHADER_ATTRIB_CNTL, 0x00000001);
    }

    fn setup_rasterizer(&mut self) {
        self.write_mmio(EVERGREEN_RASTER_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_RASTER_CLIP_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_RASTER_CULL_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_RASTER_SCISSOR_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_RASTER_MSAA_CNTL, 0x00000004);
        self.write_mmio(EVERGREEN_RASTER_AA_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_RASTER_OFFSET_CNTL, 0x00000000);
    }

    fn setup_depth_buffer(&mut self) {
        self.write_mmio(EVERGREEN_DEPTH_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_DEPTH_FUNC, 0x00000003);
        self.write_mmio(EVERGREEN_DEPTH_BOUNDS, 0x00000000);
        self.write_mmio(EVERGREEN_DEPTH_WRITE_MASK, 0x00000001);
        self.write_mmio(EVERGREEN_DEPTH_CLEAR, 0x00000000);
        self.write_mmio(EVERGREEN_DEPTH_HIZ, 0x00000001);
        self.write_mmio(EVERGREEN_DEPTH_STENCIL_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_DEPTH_STENCIL_FUNC, 0x00000003);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(EVERGREEN_STENCIL_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_STENCIL_FUNC, 0x00000003);
        self.write_mmio(EVERGREEN_STENCIL_REF, 0x00000000);
        self.write_mmio(EVERGREEN_STENCIL_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_STENCIL_OP_FAIL, 0x00000000);
        self.write_mmio(EVERGREEN_STENCIL_OP_ZFAIL, 0x00000000);
        self.write_mmio(EVERGREEN_STENCIL_OP_ZPASS, 0x00000000);
        self.write_mmio(EVERGREEN_STENCIL_CLEAR, 0x00000000);
    }

    fn setup_blend(&mut self) {
        self.write_mmio(EVERGREEN_BLEND_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_BLEND_EQUATION_RGB, 0x00000001);
        self.write_mmio(EVERGREEN_BLEND_EQUATION_ALPHA, 0x00000001);
        self.write_mmio(EVERGREEN_BLEND_FUNC_SRC_RGB, 0x00000101);
        self.write_mmio(EVERGREEN_BLEND_FUNC_DST_RGB, 0x00000101);
        self.write_mmio(EVERGREEN_BLEND_FUNC_SRC_ALPHA, 0x00000101);
        self.write_mmio(EVERGREEN_BLEND_FUNC_DST_ALPHA, 0x00000101);
        self.write_mmio(EVERGREEN_BLEND_COLOR, 0x00000000);
        self.write_mmio(EVERGREEN_BLEND_SEPARATE_CNTL, 0x00000001);
    }

    fn setup_color_buffer(&mut self) {
        self.write_mmio(EVERGREEN_COLOR0_BASE, REG_VRAM_BASE + 0x900000);
        self.write_mmio(EVERGREEN_COLOR0_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_COLOR0_PITCH, 1024);
        self.write_mmio(EVERGREEN_COLOR0_INFO, 0x00000001);
        self.write_mmio(EVERGREEN_COLOR1_BASE, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(EVERGREEN_COLOR1_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_COLOR1_PITCH, 1024);
        self.write_mmio(EVERGREEN_COLOR1_INFO, 0x00000001);
        self.write_mmio(EVERGREEN_COLOR2_BASE, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(EVERGREEN_COLOR2_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_COLOR2_PITCH, 1024);
        self.write_mmio(EVERGREEN_COLOR2_INFO, 0x00000001);
        self.write_mmio(EVERGREEN_COLOR3_BASE, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(EVERGREEN_COLOR3_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_COLOR3_PITCH, 1024);
        self.write_mmio(EVERGREEN_COLOR3_INFO, 0x00000001);
    }

    fn setup_framebuffers(&mut self) {
        self.write_mmio(EVERGREEN_FRAMEBUFFER_ADDR, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(EVERGREEN_FRAMEBUFFER_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_FRAMEBUFFER_PITCH, 1024);
        self.write_mmio(EVERGREEN_FRAMEBUFFER_FORMAT, 0x00000001);
    }

    fn setup_render_targets(&mut self) {
        self.write_mmio(EVERGREEN_RENDER_TARGET_ADDR, REG_VRAM_BASE + 0xD00000);
        self.write_mmio(EVERGREEN_RENDER_TARGET_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_RENDER_TARGET_COUNT, 0x00000008);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(EVERGREEN_DEPTH_STENCIL_ADDR, REG_VRAM_BASE + 0xE00000);
        self.write_mmio(EVERGREEN_DEPTH_STENCIL_SIZE, self.vram_size as u32);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(EVERGREEN_MULTISAMPLE_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_MULTISAMPLE_COUNT, 0x00000008);
        self.write_mmio(EVERGREEN_MULTISAMPLE_SAMPLE_LOCS, 0x00000008);
        self.write_mmio(EVERGREEN_MULTISAMPLE_COVERAGE, 0x00000001);
        self.write_mmio(EVERGREEN_MULTISAMPLE_RESOLVE, 0x00000001);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(EVERGREEN_ANTIALIAS_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_ANTIALIAS_MODE, 0x00000001);
        self.write_mmio(EVERGREEN_ANTIALIAS_SAMPLES, 0x00000008);
        self.write_mmio(EVERGREEN_ANTIALIAS_FILTER, 0x00000001);
    }

    fn setup_viewport(&mut self) {
        self.write_mmio(EVERGREEN_VIEWPORT_XSCALE, 0x00000000);
        self.write_mmio(EVERGREEN_VIEWPORT_XOFFSET, 0x00000000);
        self.write_mmio(EVERGREEN_VIEWPORT_YSCALE, 0x00000000);
        self.write_mmio(EVERGREEN_VIEWPORT_YOFFSET, 0x00000000);
        self.write_mmio(EVERGREEN_VIEWPORT_ZSCALE, 0x00000000);
        self.write_mmio(EVERGREEN_VIEWPORT_ZOFFSET, 0x00000000);
        self.write_mmio(EVERGREEN_VIEWPORT_DEPTH_RANGE, 0x00000000);
    }

    fn setup_scissor(&mut self) {
        self.write_mmio(EVERGREEN_SCISSOR_X, 0x00000000);
        self.write_mmio(EVERGREEN_SCISSOR_Y, 0x00000000);
        self.write_mmio(EVERGREEN_SCISSOR_WIDTH, 0x00000000);
        self.write_mmio(EVERGREEN_SCISSOR_HEIGHT, 0x00000000);
        self.write_mmio(EVERGREEN_SCISSOR_ENABLE, 0x00000000);
    }

    fn setup_vertex_cache(&mut self) {
        self.write_mmio(EVERGREEN_VERTEX_CACHE_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_VERTEX_CACHE_SIZE, self.vertex_cache_size);
        self.write_mmio(EVERGREEN_VERTEX_CACHE_FLUSH, 0x00000001);
    }

    fn setup_index_buffer(&mut self) {
        self.write_mmio(EVERGREEN_INDEX_BUFFER_ADDR, REG_VRAM_BASE + 0x600000);
        self.write_mmio(EVERGREEN_INDEX_BUFFER_SIZE, self.primitive_buffer_size);
        self.write_mmio(EVERGREEN_INDEX_BUFFER_FORMAT, 0x00000001);
    }

    fn setup_primitive_buffer(&mut self) {
        self.write_mmio(EVERGREEN_PRIMITIVE_BUFFER_ADDR, REG_VRAM_BASE + 0x700000);
        self.write_mmio(EVERGREEN_PRIMITIVE_BUFFER_SIZE, self.primitive_buffer_size);
    }

    fn setup_uniform_buffers(&mut self) {
        self.write_mmio(EVERGREEN_UNIFORM_BUFFER_ADDR, REG_VRAM_BASE + 0x800000);
        self.write_mmio(EVERGREEN_UNIFORM_BUFFER_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_UNIFORM_BUFFER_COUNT, 0x00000008);
    }

    fn setup_shader_storage_buffers(&mut self) {
        self.write_mmio(EVERGREEN_SHADER_STORAGE_ADDR, REG_VRAM_BASE + 0x900000);
        self.write_mmio(EVERGREEN_SHADER_STORAGE_SIZE, self.vram_size as u32);
        self.write_mmio(EVERGREEN_SHADER_STORAGE_COUNT, 0x00000008);
    }

    fn setup_atomic_counters(&mut self) {
        self.write_mmio(EVERGREEN_ATOMIC_COUNTER_ADDR, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(EVERGREEN_ATOMIC_COUNTER_COUNT, 0x00001000);
    }

    fn setup_query_objects(&mut self) {
        self.write_mmio(EVERGREEN_QUERY_OBJECT_ADDR, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(EVERGREEN_QUERY_OBJECT_COUNT, 0x00001000);
        self.write_mmio(EVERGREEN_QUERY_OBJECT_TYPE, 0x00000001);
        self.write_mmio(EVERGREEN_QUERY_OBJECT_RESULT, 0x00000000);
    }

    fn setup_pcie(&mut self) {
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_STATUS, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL2, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL3, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL4, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL5, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL6, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL7, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL8, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL9, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL10, 0x00000001);
        self.write_mmio(EVERGREEN_PCIE_LC_CNTL11, 0x00000001);
    }

    fn setup_power_play(&mut self) {
        self.write_mmio(EVERGREEN_POWER_PLAY_SCLK_DPM, 0x00000001);
        self.write_mmio(EVERGREEN_POWER_PLAY_MCLK_DPM, 0x00000001);
        self.write_mmio(EVERGREEN_POWER_PLAY_VDD_CURR_DEP, 0x00000000);
        self.write_mmio(EVERGREEN_POWER_PLAY_SCLK_PW, 0x00000000);
        self.write_mmio(EVERGREEN_POWER_PLAY_MCLK_PW, 0x00000000);
        self.write_mmio(EVERGREEN_POWER_PLAY_TEMPERATURE, 0x00000000);
        self.write_mmio(EVERGREEN_POWER_PLAY_FAN_CTRL, 0x00000001);
        self.write_mmio(EVERGREEN_POWER_PLAY_POWER_STATE, 0x00000001);
        self.write_mmio(EVERGREEN_POWER_PLAY_GFX_CLOCK_GATING, 0x00000001);
        self.write_mmio(EVERGREEN_POWER_PLAY_CP_CLOCK_GATING, 0x00000001);
    }

    fn setup_smu(&mut self) {
        self.write_mmio(EVERGREEN_SMU_SYSTEM_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SMU_SYSTEM_INTERRUPT_MASK, 0xFFFFFFFF);
        self.write_mmio(EVERGREEN_SMU_SYSTEM_PWR_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SMU_GFX_PWR_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SMU_UVD_PWR_CNTL, 0x00000001);
        self.write_mmio(EVERGREEN_SMU_VCE_PWR_CNTL, 0x00000001);
    }

    fn enable_3d_acceleration(&mut self) {
        self.write_mmio(0x0900, 0x00000001);
        self.write_mmio(0x0904, 0xFFFFFFFF);
        self.write_mmio(0x0908, 0x00000001);
        self.write_mmio(0x090C, 0x00000001);
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
            self.write_mmio(0x2000 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn set_world_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio(0x2100 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn set_view_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio(0x2200 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn enable_depth_test(&mut self, enabled: bool) {
        self.write_mmio(0x2300, if enabled { 1 } else { 0 });
    }

    pub fn enable_blending(&mut self, enabled: bool) {
        self.write_mmio(0x2400, if enabled { 1 } else { 0 });
    }

    pub fn set_blend_func(&mut self, src: u32, dst: u32) {
        self.write_mmio(0x2404, src);
        self.write_mmio(0x2408, dst);
    }

    pub fn set_depth_func(&mut self, func: u32) {
        self.write_mmio(0x2304, func);
    }

    pub fn set_cull_mode(&mut self, mode: u32) {
        self.write_mmio(0x2500, mode);
    }

    pub fn set_wireframe(&mut self, enabled: bool) {
        self.write_mmio(0x2504, if enabled { 1 } else { 0 });
    }

    pub fn set_multisample_count(&mut self, count: u32) {
        self.write_mmio(0x2508, count);
    }

    pub fn enable_multisampling(&mut self, enabled: bool) {
        self.write_mmio(0x250C, if enabled { 1 } else { 0 });
    }

    pub fn enable_antialiasing(&mut self, enabled: bool) {
        self.write_mmio(0x2510, if enabled { 1 } else { 0 });
    }

    pub fn set_depth_bounds_test(&mut self, enabled: bool, min: f32, max: f32) {
        self.write_mmio(0x2514, if enabled { 1 } else { 0 });
        self.write_mmio(0x2518, min.to_bits());
        self.write_mmio(0x251C, max.to_bits());
    }

    pub fn set_stencil_test(&mut self, enabled: bool, func: u32, ref_value: u32, mask: u32) {
        self.write_mmio(0x2520, if enabled { 1 } else { 0 });
        self.write_mmio(0x2524, func);
        self.write_mmio(0x2528, ref_value);
        self.write_mmio(0x252C, mask);
    }

    pub fn set_stencil_op(&mut self, fail: u32, zfail: u32, zpass: u32) {
        self.write_mmio(0x2530, fail);
        self.write_mmio(0x2534, zfail);
        self.write_mmio(0x2538, zpass);
    }

    pub fn set_blend_equation(&mut self, rgb: u32, alpha: u32) {
        self.write_mmio(0x2540, rgb);
        self.write_mmio(0x2544, alpha);
    }

    pub fn set_blend_color(&mut self, color: u32) {
        self.write_mmio(0x2548, color);
    }

    pub fn set_logic_op(&mut self, op: u32, enabled: bool) {
        self.write_mmio(0x254C, op);
        self.write_mmio(0x2550, if enabled { 1 } else { 0 });
    }

    pub fn set_dither(&mut self, enabled: bool) {
        self.write_mmio(0x2554, if enabled { 1 } else { 0 });
    }

    pub fn set_polygon_offset(&mut self, enabled: bool, factor: f32, units: f32) {
        self.write_mmio(0x2558, if enabled { 1 } else { 0 });
        self.write_mmio(0x255C, factor.to_bits());
        self.write_mmio(0x2560, units.to_bits());
    }

    pub fn set_sample_coverage(&mut self, value: f32, invert: bool) {
        self.write_mmio(0x2564, value.to_bits());
        self.write_mmio(0x2568, if invert { 1 } else { 0 });
    }

    pub fn set_vertex_array(&mut self, buffer: u32, stride: u32, offset: u32) {
        self.write_mmio(0x2600, buffer);
        self.write_mmio(0x2604, stride);
        self.write_mmio(0x2608, offset);
    }

    pub fn set_index_array(&mut self, buffer: u32, format: u32, offset: u32) {
        self.write_mmio(0x260C, buffer);
        self.write_mmio(0x2610, format);
        self.write_mmio(0x2614, offset);
    }

    pub fn draw_arrays(&mut self, mode: u32, first: u32, count: u32) {
        self.write_mmio(0x2618, mode);
        self.write_mmio(0x261C, first);
        self.write_mmio(0x2620, count);
        self.write_mmio(0x2624, 0x00000001);
    }

    pub fn draw_elements(&mut self, mode: u32, count: u32, type_: u32, offset: u32) {
        self.write_mmio(0x2628, mode);
        self.write_mmio(0x262C, count);
        self.write_mmio(0x2630, type_);
        self.write_mmio(0x2634, offset);
        self.write_mmio(0x2638, 0x00000001);
    }

    pub fn draw_instanced(&mut self, mode: u32, first: u32, count: u32, instance_count: u32) {
        self.write_mmio(0x263C, mode);
        self.write_mmio(0x2640, first);
        self.write_mmio(0x2644, count);
        self.write_mmio(0x2648, instance_count);
        self.write_mmio(0x264C, 0x00000001);
    }

    pub fn draw_elements_instanced(&mut self, mode: u32, count: u32, type_: u32, offset: u32, instance_count: u32) {
        self.write_mmio(0x2650, mode);
        self.write_mmio(0x2654, count);
        self.write_mmio(0x2658, type_);
        self.write_mmio(0x265C, offset);
        self.write_mmio(0x2660, instance_count);
        self.write_mmio(0x2664, 0x00000001);
    }

    pub fn set_shader_program(&mut self, program_id: u32) {
        self.write_mmio(0x2700, program_id);
    }

    pub fn set_uniform_1f(&mut self, location: u32, value: f32) {
        self.write_mmio(0x2704, location);
        self.write_mmio(0x2708, value.to_bits());
    }

    pub fn set_uniform_2f(&mut self, location: u32, x: f32, y: f32) {
        self.write_mmio(0x270C, location);
        self.write_mmio(0x2710, x.to_bits());
        self.write_mmio(0x2714, y.to_bits());
    }

    pub fn set_uniform_3f(&mut self, location: u32, x: f32, y: f32, z: f32) {
        self.write_mmio(0x2718, location);
        self.write_mmio(0x271C, x.to_bits());
        self.write_mmio(0x2720, y.to_bits());
        self.write_mmio(0x2724, z.to_bits());
    }

    pub fn set_uniform_4f(&mut self, location: u32, x: f32, y: f32, z: f32, w: f32) {
        self.write_mmio(0x2728, location);
        self.write_mmio(0x272C, x.to_bits());
        self.write_mmio(0x2730, y.to_bits());
        self.write_mmio(0x2734, z.to_bits());
        self.write_mmio(0x2738, w.to_bits());
    }

    pub fn set_uniform_matrix4fv(&mut self, location: u32, matrix: &Matrix4x4) {
        self.write_mmio(0x273C, location);
        for i in 0..16 {
            self.write_mmio(0x2740 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn bind_texture(&mut self, unit: u32, texture_id: u32) {
        self.write_mmio(0x2800, unit);
        self.write_mmio(0x2804, texture_id);
    }

    pub fn bind_framebuffer(&mut self, framebuffer_id: u32) {
        self.write_mmio(0x2810, framebuffer_id);
    }

    pub fn bind_renderbuffer(&mut self, renderbuffer_id: u32) {
        self.write_mmio(0x2820, renderbuffer_id);
    }

    pub fn create_texture(&mut self, width: u32, height: u32, format: u32) -> u32 {
        self.write_mmio(0x2900, width);
        self.write_mmio(0x2904, height);
        self.write_mmio(0x2908, format);
        self.write_mmio(0x290C, 0x00000001);
        self.read_mmio(0x2910)
    }

    pub fn delete_texture(&mut self, texture_id: u32) {
        self.write_mmio(0x2920, texture_id);
    }

    pub fn upload_texture_data(&mut self, texture_id: u32, data: *const u32, size: u32) {
        self.write_mmio(0x2930, texture_id);
        self.write_mmio(0x2934, data as u32);
        self.write_mmio(0x2938, size);
        self.write_mmio(0x293C, 0x00000001);
    }

    pub fn generate_mipmaps(&mut self, texture_id: u32) {
        self.write_mmio(0x2940, texture_id);
        self.write_mmio(0x2944, 0x00000001);
    }

    pub fn set_texture_filter(&mut self, texture_id: u32, min_filter: u32, mag_filter: u32) {
        self.write_mmio(0x2950, texture_id);
        self.write_mmio(0x2954, min_filter);
        self.write_mmio(0x2958, mag_filter);
    }

    pub fn set_texture_wrap(&mut self, texture_id: u32, wrap_s: u32, wrap_t: u32) {
        self.write_mmio(0x2960, texture_id);
        self.write_mmio(0x2964, wrap_s);
        self.write_mmio(0x2968, wrap_t);
    }

    pub fn create_renderbuffer(&mut self, width: u32, height: u32, format: u32) -> u32 {
        self.write_mmio(0x2980, width);
        self.write_mmio(0x2984, height);
        self.write_mmio(0x2988, format);
        self.write_mmio(0x298C, 0x00000001);
        self.read_mmio(0x2990)
    }

    pub fn delete_renderbuffer(&mut self, renderbuffer_id: u32) {
        self.write_mmio(0x29A0, renderbuffer_id);
    }

    pub fn create_framebuffer(&mut self) -> u32 {
        self.write_mmio(0x29B0, 0x00000001);
        self.read_mmio(0x29B4)
    }

    pub fn delete_framebuffer(&mut self, framebuffer_id: u32) {
        self.write_mmio(0x29C0, framebuffer_id);
    }

    pub fn attach_texture_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, texture_id: u32, mip_level: u32) {
        self.write_mmio(0x29D0, framebuffer_id);
        self.write_mmio(0x29D4, attachment);
        self.write_mmio(0x29D8, texture_id);
        self.write_mmio(0x29DC, mip_level);
        self.write_mmio(0x29E0, 0x00000001);
    }

    pub fn attach_renderbuffer_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, renderbuffer_id: u32) {
        self.write_mmio(0x29F0, framebuffer_id);
        self.write_mmio(0x29F4, attachment);
        self.write_mmio(0x29F8, renderbuffer_id);
        self.write_mmio(0x29FC, 0x00000001);
    }

    pub fn set_draw_buffers(&mut self, count: u32, attachments: *const u32) {
        self.write_mmio(0x2A00, count);
        self.write_mmio(0x2A04, attachments as u32);
    }

    pub fn set_read_buffer(&mut self, attachment: u32) {
        self.write_mmio(0x2A10, attachment);
    }

    pub fn blit_framebuffer(&mut self, src_x0: i32, src_y0: i32, src_x1: i32, src_y1: i32, dst_x0: i32, dst_y0: i32, dst_x1: i32, dst_y1: i32, mask: u32, filter: u32) {
        self.write_mmio(0x2A20, src_x0 as u32);
        self.write_mmio(0x2A24, src_y0 as u32);
        self.write_mmio(0x2A28, src_x1 as u32);
        self.write_mmio(0x2A2C, src_y1 as u32);
        self.write_mmio(0x2A30, dst_x0 as u32);
        self.write_mmio(0x2A34, dst_y0 as u32);
        self.write_mmio(0x2A38, dst_x1 as u32);
        self.write_mmio(0x2A3C, dst_y1 as u32);
        self.write_mmio(0x2A40, mask);
        self.write_mmio(0x2A44, filter);
        self.write_mmio(0x2A48, 0x00000001);
    }

    pub fn read_pixels(&mut self, x: i32, y: i32, width: i32, height: i32, format: u32, r#type: u32, pixels: *mut u32) {
        self.write_mmio(0x2A50, x as u32);
        self.write_mmio(0x2A54, y as u32);
        self.write_mmio(0x2A58, width as u32);
        self.write_mmio(0x2A5C, height as u32);
        self.write_mmio(0x2A60, format);
        self.write_mmio(0x2A64, r#type);
        self.write_mmio(0x2A68, pixels as u32);
        self.write_mmio(0x2A6C, 0x00000001);
    }

    pub fn finish(&mut self) {
        self.write_mmio(0x2A70, 0x00000001);
        self.wait_idle();
    }

    pub fn flush(&mut self) {
        self.write_mmio(0x2A74, 0x00000001);
    }

    pub fn get_status(&self) -> u32 {
        self.read_mmio(0xFF00)
    }

    pub fn wait_idle_pub(&self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(0xFF00);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn engine_clock(&self) -> u32 {
        self.engine_clock
    }

    pub fn memory_clock(&self) -> u32 {
        self.memory_clock
    }

    pub fn vram_size(&self) -> u32 {
        self.vram_size as u32
    }

    pub fn stream_cores(&self) -> u32 {
        self.stream_cores
    }

    pub fn simd_count(&self) -> u32 {
        self.simd_count
    }

    pub fn rop_count(&self) -> u32 {
        self.rop_count
    }

    pub fn texture_units(&self) -> u32 {
        self.texture_units
    }

    pub fn memory_bus_width(&self) -> u32 {
        self.memory_bus_width
    }

    pub fn vertex_cache_size(&self) -> u32 {
        self.vertex_cache_size
    }

    pub fn primitive_buffer_size(&self) -> u32 {
        self.primitive_buffer_size
    }

    pub fn command_buffer_size(&self) -> u32 {
        self.command_buffer_size
    }

    pub fn error_count(&self) -> u32 {
        self.error_count.load(Ordering::SeqCst)
    }

    pub fn create_shader_module(&mut self, spirv_addr: u64, spirv_size: u32, entry_point_addr: u64, entry_point_size: u32, stage: u32) -> u32 {
        let id = self.shader_module_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.shader_modules[id as usize] = ShaderModule {
            id,
            spirv_addr,
            spirv_size,
            entry_point_addr,
            entry_point_size,
            stage,
        };
        self.shader_module_count += 1;
        self.write_mmio(EVERGREEN_SHADER_MODULE_CREATE, id);
        self.write_mmio_64(EVERGREEN_SHADER_MODULE_SPIRV_ADDR, spirv_addr);
        self.write_mmio(EVERGREEN_SHADER_MODULE_SPIRV_SIZE, spirv_size);
        self.write_mmio_64(EVERGREEN_SHADER_MODULE_ENTRY_POINT, entry_point_addr);
        self.write_mmio(EVERGREEN_SHADER_MODULE_ENTRY_POINT + 4, entry_point_size);
        id
    }

    pub fn destroy_shader_module(&mut self, id: u32) {
        self.write_mmio(EVERGREEN_SHADER_MODULE_DESTROY, id);
    }

    pub fn create_descriptor_set(&mut self, addr: u64, size: u32, binding_count: u32, bindings: *const u64) -> u32 {
        let id = self.descriptor_set_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        let mut ds = DescriptorSet {
            id,
            addr,
            size,
            binding_count,
            bindings: [0; 64],
        };
        unsafe {
            for i in 0..binding_count as usize {
                ds.bindings[i] = bindings.add(i).read();
            }
        }
        self.descriptor_sets[id as usize] = ds;
        self.descriptor_set_count += 1;
        self.write_mmio(EVERGREEN_DESCRIPTOR_SET_ALLOC, id);
        self.write_mmio_64(EVERGREEN_DESCRIPTOR_SET_BIND_ADDR, addr);
        self.write_mmio(EVERGREEN_DESCRIPTOR_SET_BIND_SIZE, size);
        id
    }

    pub fn create_command_buffer(&mut self, addr: u64, size: u32) -> u32 {
        let id = self.command_buffer_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.command_buffers[id as usize] = CommandBuffer {
            id,
            addr,
            size,
            offset: AtomicU32::new(0),
            submitted: AtomicBool::new(false),
        };
        self.command_buffer_count += 1;
        self.write_mmio(EVERGREEN_COMMAND_BUFFER_ALLOC, id);
        self.write_mmio_64(EVERGREEN_COMMAND_BUFFER_ADDR, addr);
        self.write_mmio(EVERGREEN_COMMAND_BUFFER_SIZE, size);
        id
    }

    pub fn create_query_pool(&mut self, addr: u64, count: u32, query_type: u32, results_addr: u64) -> u32 {
        let id = self.query_pool_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.query_pools[id as usize] = QueryPool {
            id,
            addr,
            count,
            query_type,
            results_addr,
            results_size: count * 8,
        };
        self.query_pool_count += 1;
        self.write_mmio(EVERGREEN_QUERY_POOL_ALLOC, id);
        self.write_mmio_64(EVERGREEN_QUERY_POOL_ADDR, addr);
        self.write_mmio(EVERGREEN_QUERY_POOL_COUNT, count);
        self.write_mmio(EVERGREEN_QUERY_POOL_TYPE, query_type);
        id
    }

    pub fn create_swap_chain(&mut self, image_count: u32, image_addr: u64, image_size: u32, image_format: u32) -> u32 {
        let id = self.swap_chain_count;
        if id >= 8 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.swap_chains[id as usize] = SwapChain {
            id,
            image_count,
            current_image: AtomicU32::new(0),
            image_addrs: [0; 8],
            image_size,
            image_format,
            acquired: AtomicBool::new(false),
            presented: AtomicBool::new(false),
        };
        self.swap_chain_count += 1;
        self.write_mmio(EVERGREEN_SWAP_CHAIN_CREATE, id);
        self.write_mmio(EVERGREEN_SWAP_CHAIN_IMAGE_COUNT, image_count);
        self.write_mmio_64(EVERGREEN_SWAP_CHAIN_IMAGE_ADDR, image_addr);
        self.write_mmio(EVERGREEN_SWAP_CHAIN_IMAGE_SIZE, image_size);
        self.write_mmio(EVERGREEN_SWAP_CHAIN_IMAGE_FORMAT, image_format);
        id
    }

    pub fn create_shader_pipeline(&mut self, vs_module_id: u32, fs_module_id: u32, gs_module_id: u32, tess_ctrl_module_id: u32, tess_eval_module_id: u32, state_addr: u64, state_size: u32) -> u32 {
        let id = self.shader_pipeline_count;
        if id >= 16 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.shader_pipelines[id as usize] = ShaderPipeline {
            id,
            vs_module_id,
            fs_module_id,
            gs_module_id,
            tess_ctrl_module_id,
            tess_eval_module_id,
            compute_module_id: 0,
            state_addr,
            state_size,
        };
        self.shader_pipeline_count += 1;
        self.write_mmio(0x13D000, id);
        self.write_mmio(0x13D004, vs_module_id);
        self.write_mmio(0x13D008, fs_module_id);
        self.write_mmio(0x13D00C, gs_module_id);
        self.write_mmio(0x13D010, tess_ctrl_module_id);
        self.write_mmio(0x13D014, tess_eval_module_id);
        self.write_mmio_64(0x13D018, state_addr);
        self.write_mmio(0x13D020, state_size);
        id
    }

    pub fn create_compute_pipeline(&mut self, compute_module_id: u32, state_addr: u64, state_size: u32) -> u32 {
        let id = self.compute_pipeline_count;
        if id >= 16 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.compute_pipelines[id as usize] = ComputePipeline {
            id,
            compute_module_id,
            state_addr,
            state_size,
        };
        self.compute_pipeline_count += 1;
        self.write_mmio(0x13E000, id);
        self.write_mmio(0x13E004, compute_module_id);
        self.write_mmio_64(0x13E008, state_addr);
        self.write_mmio(0x13E00C, state_size);
        id
    }

    pub fn bind_shader_pipeline(&mut self, pipeline_id: u32) {
        self.write_mmio(0x13D000, pipeline_id);
    }

    pub fn bind_compute_pipeline(&mut self, pipeline_id: u32) {
        self.write_mmio(0x13E000, pipeline_id);
    }

    pub fn dispatch_compute(&mut self, group_x: u32, group_y: u32, group_z: u32) {
        self.write_mmio(0x13F000, group_x);
        self.write_mmio(0x13F004, group_y);
        self.write_mmio(0x13F008, group_z);
        self.write_mmio(0x13F00C, 0x00000001);
    }

    pub fn present(&mut self, swap_chain_id: u32) {
        self.write_mmio(EVERGREEN_SWAP_CHAIN_PRESENT, swap_chain_id);
    }

    pub fn acquire_image(&mut self, swap_chain_id: u32) -> u32 {
        self.write_mmio(EVERGREEN_SWAP_CHAIN_ACQUIRE, swap_chain_id);
        self.read_mmio(0x142024)
    }
}

pub fn gpu_instance() -> &'static mut GpuInstance {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(GpuInstance::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn hd_5850_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn hd_5850_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn hd_5850_driver_init() -> bool {
    hd_5850_init()
}

#[no_mangle]
pub extern "C" fn hd_5850_driver_detected() -> bool {
    hd_5850_detected()
}

#[no_mangle]
pub extern "C" fn hd_5850_draw_triangle_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_triangle_3d(x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
}

#[no_mangle]
pub extern "C" fn hd_5850_draw_line_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_line_3d(x1, y1, z1, x2, y2, z2, color);
}

#[no_mangle]
pub extern "C" fn hd_5850_clear_screen_3d(color: u32) {
    let gpu = gpu_instance();
    gpu.clear_screen_3d(color);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_viewport(x: u32, y: u32, width: u32, height: u32) {
    let gpu = gpu_instance();
    gpu.set_viewport(x, y, width, height);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_projection_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_projection_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn hd_5850_set_world_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_world_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn hd_5850_set_view_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_view_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn hd_5850_enable_depth_test(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_depth_test(enabled);
}

#[no_mangle]
pub extern "C" fn hd_5850_enable_blending(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_blending(enabled);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_blend_func(src: u32, dst: u32) {
    let gpu = gpu_instance();
    gpu.set_blend_func(src, dst);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_depth_func(func: u32) {
    let gpu = gpu_instance();
    gpu.set_depth_func(func);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_cull_mode(mode: u32) {
    let gpu = gpu_instance();
    gpu.set_cull_mode(mode);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_wireframe(enabled: bool) {
    let gpu = gpu_instance();
    gpu.set_wireframe(enabled);
}

#[no_mangle]
pub extern "C" fn hd_5850_set_multisample_count(count: u32) {
    let gpu = gpu_instance();
    gpu.set_multisample_count(count);
}

#[no_mangle]
pub extern "C" fn hd_5850_enable_multisampling(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_multisampling(enabled);
}

#[no_mangle]
pub extern "C" fn hd_5850_enable_antialiasing(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_antialiasing(enabled);
}

#[no_mangle]
pub extern "C" fn hd_5850_finish() {
    let gpu = gpu_instance();
    gpu.finish();
}

#[no_mangle]
pub extern "C" fn hd_5850_flush() {
    let gpu = gpu_instance();
    gpu.flush();
}

#[no_mangle]
pub extern "C" fn hd_5850_get_status() -> u32 {
    let gpu = gpu_instance();
    gpu.get_status()
}

#[no_mangle]
pub extern "C" fn hd_5850_engine_clock() -> u32 {
    let gpu = gpu_instance();
    gpu.engine_clock()
}

#[no_mangle]
pub extern "C" fn hd_5850_memory_clock() -> u32 {
    let gpu = gpu_instance();
    gpu.memory_clock()
}

#[no_mangle]
pub extern "C" fn hd_5850_vram_size() -> u32 {
    let gpu = gpu_instance();
    gpu.vram_size()
}

#[no_mangle]
pub extern "C" fn hd_5850_stream_cores() -> u32 {
    let gpu = gpu_instance();
    gpu.stream_cores()
}

#[no_mangle]
pub extern "C" fn hd_5850_simd_count() -> u32 {
    let gpu = gpu_instance();
    gpu.simd_count()
}

#[no_mangle]
pub extern "C" fn hd_5850_rop_count() -> u32 {
    let gpu = gpu_instance();
    gpu.rop_count()
}

#[no_mangle]
pub extern "C" fn hd_5850_texture_units() -> u32 {
    let gpu = gpu_instance();
    gpu.texture_units()
}

#[no_mangle]
pub extern "C" fn hd_5850_memory_bus_width() -> u32 {
    let gpu = gpu_instance();
    gpu.memory_bus_width()
}

#[no_mangle]
pub extern "C" fn hd_5850_error_count() -> u32 {
    let gpu = gpu_instance();
    gpu.error_count()
}

#[no_mangle]
pub extern "C" fn hd_5850_create_shader_module(spirv_addr: u64, spirv_size: u32, entry_point_addr: u64, entry_point_size: u32, stage: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_shader_module(spirv_addr, spirv_size, entry_point_addr, entry_point_size, stage)
}

#[no_mangle]
pub extern "C" fn hd_5850_destroy_shader_module(id: u32) {
    let gpu = gpu_instance();
    gpu.destroy_shader_module(id);
}

#[no_mangle]
pub extern "C" fn hd_5850_create_descriptor_set(addr: u64, size: u32, binding_count: u32, bindings: *const u64) -> u32 {
    let gpu = gpu_instance();
    gpu.create_descriptor_set(addr, size, binding_count, bindings)
}

#[no_mangle]
pub extern "C" fn hd_5850_create_command_buffer(addr: u64, size: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_command_buffer(addr, size)
}

#[no_mangle]
pub extern "C" fn hd_5850_create_query_pool(addr: u64, count: u32, query_type: u32, results_addr: u64) -> u32 {
    let gpu = gpu_instance();
    gpu.create_query_pool(addr, count, query_type, results_addr)
}

#[no_mangle]
pub extern "C" fn hd_5850_create_swap_chain(image_count: u32, image_addr: u64, image_size: u32, image_format: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_swap_chain(image_count, image_addr, image_size, image_format)
}

#[no_mangle]
pub extern "C" fn hd_5850_create_shader_pipeline(vs_module_id: u32, fs_module_id: u32, gs_module_id: u32, tess_ctrl_module_id: u32, tess_eval_module_id: u32, state_addr: u64, state_size: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_shader_pipeline(vs_module_id, fs_module_id, gs_module_id, tess_ctrl_module_id, tess_eval_module_id, state_addr, state_size)
}

#[no_mangle]
pub extern "C" fn hd_5850_create_compute_pipeline(compute_module_id: u32, state_addr: u64, state_size: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_compute_pipeline(compute_module_id, state_addr, state_size)
}

#[no_mangle]
pub extern "C" fn hd_5850_bind_shader_pipeline(pipeline_id: u32) {
    let gpu = gpu_instance();
    gpu.bind_shader_pipeline(pipeline_id);
}

#[no_mangle]
pub extern "C" fn hd_5850_bind_compute_pipeline(pipeline_id: u32) {
    let gpu = gpu_instance();
    gpu.bind_compute_pipeline(pipeline_id);
}

#[no_mangle]
pub extern "C" fn hd_5850_dispatch_compute(group_x: u32, group_y: u32, group_z: u32) {
    let gpu = gpu_instance();
    gpu.dispatch_compute(group_x, group_y, group_z);
}

#[no_mangle]
pub extern "C" fn hd_5850_present(swap_chain_id: u32) {
    let gpu = gpu_instance();
    gpu.present(swap_chain_id);
}

#[no_mangle]
pub extern "C" fn hd_5850_acquire_image(swap_chain_id: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.acquire_image(swap_chain_id);
}

pub fn gpu_instance() -> &'static mut GpuInstance {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(GpuInstance::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn hd_5850_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn hd_5850_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn hd_5850_driver_init() -> bool {
    hd_5850_init()
}

#[no_mangle]
pub extern "C" fn hd_5850_driver_detected() -> bool {
    hd_5850_detected()
}
