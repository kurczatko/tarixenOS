#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const NVIDIA_VENDOR_ID: u16 = 0x10DE;
pub const NVIDIA_GTX_1030_DEVICE_ID: u16 = 0x1D01;
pub const CHIP_GP108: u32 = 0x1D01;
pub const PASCAL_ARCH_VERSION: u32 = 1;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;
pub const REG_GART_BASE: u32 = 0xD0000000;
pub const REG_DOORBELL_BASE: u32 = 0xC0000000;

pub const NV_PMC_ENABLE: u32 = 0x0000;
pub const NV_PMC_BOOT_0: u32 = 0x0000;
pub const NV_PMC_BOOT_1: u32 = 0x0004;
pub const NV_PMC_BOOT_2: u32 = 0x0008;
pub const NV_PMC_BOOT_3: u32 = 0x000C;
pub const NV_PMC_BOOT_4: u32 = 0x0010;
pub const NV_PMC_BOOT_5: u32 = 0x0014;
pub const NV_PMC_BOOT_6: u32 = 0x0018;
pub const NV_PMC_BOOT_7: u32 = 0x001C;
pub const NV_PMC_BOOT_8: u32 = 0x0020;
pub const NV_PMC_BOOT_9: u32 = 0x0024;
pub const NV_PMC_BOOT_10: u32 = 0x0028;
pub const NV_PMC_BOOT_11: u32 = 0x002C;
pub const NV_PMC_BOOT_12: u32 = 0x0030;
pub const NV_PMC_BOOT_13: u32 = 0x0034;
pub const NV_PMC_BOOT_14: u32 = 0x0038;
pub const NV_PMC_BOOT_15: u32 = 0x003C;
pub const NV_PMC_INTR_0: u32 = 0x0100;
pub const NV_PMC_INTR_EN_0: u32 = 0x0140;
pub const NV_PMC_INTR_MASK_0: u32 = 0x0180;
pub const NV_PMC_INTR_STATUS_0: u32 = 0x01C0;
pub const NV_PMC_INTR_SET_0: u32 = 0x0200;
pub const NV_PMC_CNTRL: u32 = 0x00E0;
pub const NV_PMC_CNTRL2: u32 = 0x00E4;
pub const NV_PMC_CNTRL3: u32 = 0x00E8;
pub const NV_PMC_CNTRL4: u32 = 0x00EC;
pub const NV_PMC_CNTRL5: u32 = 0x00F0;
pub const NV_PMC_CNTRL6: u32 = 0x00F4;
pub const NV_PMC_CNTRL7: u32 = 0x00F8;
pub const NV_PMC_CNTRL8: u32 = 0x00FC;
pub const NV_PMC_CNTRL9: u32 = 0x0100;
pub const NV_PMC_CNTRL10: u32 = 0x0104;
pub const NV_PMC_CNTRL11: u32 = 0x0108;
pub const NV_PMC_CNTRL12: u32 = 0x010C;
pub const NV_PMC_CNTRL13: u32 = 0x0110;
pub const NV_PMC_CNTRL14: u32 = 0x0114;
pub const NV_PMC_CNTRL15: u32 = 0x0118;
pub const NV_PMC_CNTRL16: u32 = 0x011C;
pub const NV_PMC_CNTRL17: u32 = 0x0120;
pub const NV_PMC_CNTRL18: u32 = 0x0124;
pub const NV_PMC_CNTRL19: u32 = 0x0128;
pub const NV_PMC_CNTRL20: u32 = 0x012C;
pub const NV_PMC_CNTRL21: u32 = 0x0130;
pub const NV_PMC_CNTRL22: u32 = 0x0134;
pub const NV_PMC_CNTRL23: u32 = 0x0138;
pub const NV_PMC_CNTRL24: u32 = 0x013C;
pub const NV_PMC_CNTRL25: u32 = 0x0140;
pub const NV_PMC_CNTRL26: u32 = 0x0144;
pub const NV_PMC_CNTRL27: u32 = 0x0148;
pub const NV_PMC_CNTRL28: u32 = 0x014C;
pub const NV_PMC_CNTRL29: u32 = 0x0150;
pub const NV_PMC_CNTRL30: u32 = 0x0154;
pub const NV_PMC_CNTRL31: u32 = 0x0158;
pub const NV_PMC_CNTRL32: u32 = 0x015C;
pub const NV_PMC_CNTRL33: u32 = 0x0160;
pub const NV_PMC_CNTRL34: u32 = 0x0164;
pub const NV_PMC_CNTRL35: u32 = 0x0168;
pub const NV_PMC_CNTRL36: u32 = 0x016C;
pub const NV_PMC_CNTRL37: u32 = 0x0170;
pub const NV_PMC_CNTRL38: u32 = 0x0174;
pub const NV_PMC_CNTRL39: u32 = 0x0178;
pub const NV_PMC_CNTRL40: u32 = 0x017C;
pub const NV_PMC_CNTRL41: u32 = 0x0180;
pub const NV_PMC_CNTRL42: u32 = 0x0184;
pub const NV_PMC_CNTRL43: u32 = 0x0188;
pub const NV_PMC_CNTRL44: u32 = 0x018C;
pub const NV_PMC_CNTRL45: u32 = 0x0190;
pub const NV_PMC_CNTRL46: u32 = 0x0194;
pub const NV_PMC_CNTRL47: u32 = 0x0198;
pub const NV_PMC_CNTRL48: u32 = 0x019C;
pub const NV_PMC_CNTRL49: u32 = 0x01A0;
pub const NV_PMC_CNTRL50: u32 = 0x01A4;
pub const NV_PMC_CNTRL51: u32 = 0x01A8;
pub const NV_PMC_CNTRL52: u32 = 0x01AC;
pub const NV_PMC_CNTRL53: u32 = 0x01B0;
pub const NV_PMC_CNTRL54: u32 = 0x01B4;
pub const NV_PMC_CNTRL55: u32 = 0x01B8;
pub const NV_PMC_CNTRL56: u32 = 0x01BC;
pub const NV_PMC_CNTRL57: u32 = 0x01C0;
pub const NV_PMC_CNTRL58: u32 = 0x01C4;
pub const NV_PMC_CNTRL59: u32 = 0x01C8;
pub const NV_PMC_CNTRL60: u32 = 0x01CC;
pub const NV_PMC_CNTRL61: u32 = 0x01D0;
pub const NV_PMC_CNTRL62: u32 = 0x01D4;
pub const NV_PMC_CNTRL63: u32 = 0x01D8;

pub const NV_PGRAPH_CTX_CONTROL: u32 = 0x400800;
pub const NV_PGRAPH_CTX_USER: u32 = 0x400804;
pub const NV_PGRAPH_CTX_CONTROL2: u32 = 0x400808;
pub const NV_PGRAPH_FECS_Host_Engine_Num_Contexts: u32 = 0x40080C;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Size: u32 = 0x400810;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Addr: u32 = 0x400814;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Mode: u32 = 0x400818;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Status: u32 = 0x40081C;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Timeout: u32 = 0x400820;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control: u32 = 0x400824;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control2: u32 = 0x400828;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control3: u32 = 0x40082C;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control4: u32 = 0x400830;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control5: u32 = 0x400834;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control6: u32 = 0x400838;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control7: u32 = 0x40083C;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control8: u32 = 0x400840;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control9: u32 = 0x400844;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control10: u32 = 0x400848;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control11: u32 = 0x40084C;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control12: u32 = 0x400850;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control13: u32 = 0x400854;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control14: u32 = 0x400858;
pub const NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control15: u32 = 0x40085C;
pub const NV_PGRAPH_DEBUG_0: u32 = 0x400860;
pub const NV_PGRAPH_DEBUG_1: u32 = 0x400864;
pub const NV_PGRAPH_DEBUG_2: u32 = 0x400868;
pub const NV_PGRAPH_DEBUG_3: u32 = 0x40086C;
pub const NV_PGRAPH_DEBUG_4: u32 = 0x400870;
pub const NV_PGRAPH_DEBUG_5: u32 = 0x400874;
pub const NV_PGRAPH_DEBUG_6: u32 = 0x400878;
pub const NV_PGRAPH_DEBUG_7: u32 = 0x40087C;
pub const NV_PGRAPH_DEBUG_8: u32 = 0x400880;
pub const NV_PGRAPH_DEBUG_9: u32 = 0x400884;
pub const NV_PGRAPH_DEBUG_10: u32 = 0x400888;
pub const NV_PGRAPH_DEBUG_11: u32 = 0x40088C;
pub const NV_PGRAPH_DEBUG_12: u32 = 0x400890;
pub const NV_PGRAPH_DEBUG_13: u32 = 0x400894;
pub const NV_PGRAPH_DEBUG_14: u32 = 0x400898;
pub const NV_PGRAPH_DEBUG_15: u32 = 0x40089C;
pub const NV_PGRAPH_DEBUG_16: u32 = 0x4008A0;
pub const NV_PGRAPH_DEBUG_17: u32 = 0x4008A4;
pub const NV_PGRAPH_DEBUG_18: u32 = 0x4008A8;
pub const NV_PGRAPH_DEBUG_19: u32 = 0x4008AC;
pub const NV_PGRAPH_DEBUG_20: u32 = 0x4008B0;
pub const NV_PGRAPH_DEBUG_21: u32 = 0x4008B4;
pub const NV_PGRAPH_DEBUG_22: u32 = 0x4008B8;
pub const NV_PGRAPH_DEBUG_23: u32 = 0x4008BC;
pub const NV_PGRAPH_DEBUG_24: u32 = 0x4008C0;
pub const NV_PGRAPH_DEBUG_25: u32 = 0x4008C4;
pub const NV_PGRAPH_DEBUG_26: u32 = 0x4008C8;
pub const NV_PGRAPH_DEBUG_27: u32 = 0x4008CC;
pub const NV_PGRAPH_DEBUG_28: u32 = 0x4008D0;
pub const NV_PGRAPH_DEBUG_29: u32 = 0x4008D4;
pub const NV_PGRAPH_DEBUG_30: u32 = 0x4008D8;
pub const NV_PGRAPH_DEBUG_31: u32 = 0x4008DC;
pub const NV_PGRAPH_INTR_EN_0: u32 = 0x400A00;
pub const NV_PGRAPH_INTR_EN_1: u32 = 0x400A04;
pub const NV_PGRAPH_INTR_EN_2: u32 = 0x400A08;
pub const NV_PGRAPH_INTR_EN_3: u32 = 0x400A0C;
pub const NV_PGRAPH_INTR_EN_4: u32 = 0x400A10;
pub const NV_PGRAPH_INTR_EN_5: u32 = 0x400A14;
pub const NV_PGRAPH_INTR_EN_6: u32 = 0x400A18;
pub const NV_PGRAPH_INTR_EN_7: u32 = 0x400A1C;
pub const NV_PGRAPH_INTR_MASK_0: u32 = 0x400A20;
pub const NV_PGRAPH_INTR_MASK_1: u32 = 0x400A24;
pub const NV_PGRAPH_INTR_MASK_2: u32 = 0x400A28;
pub const NV_PGRAPH_INTR_MASK_3: u32 = 0x400A2C;
pub const NV_PGRAPH_INTR_MASK_4: u32 = 0x400A30;
pub const NV_PGRAPH_INTR_MASK_5: u32 = 0x400A34;
pub const NV_PGRAPH_INTR_MASK_6: u32 = 0x400A38;
pub const NV_PGRAPH_INTR_MASK_7: u32 = 0x400A3C;

pub const NV_GPCCS_GPCCS_CNTL: u32 = 0x400C00;
pub const NV_GPCCS_GPCCS_STATUS: u32 = 0x400C04;
pub const NV_GPCCS_GPCCS_INTR: u32 = 0x400C08;
pub const NV_GPCCS_GPCCS_INTR_MASK: u32 = 0x400C0C;
pub const NV_GPCCS_GPCCS_PWR: u32 = 0x400C10;
pub const NV_GPCCS_GPCCS_CLK: u32 = 0x400C14;
pub const NV_GPCCS_GPCCS_THERM: u32 = 0x400C18;
pub const NV_GPCCS_GPCCS_FAN: u32 = 0x400C1C;
pub const NV_GPCCS_GPCCS_VOLT: u32 = 0x400C20;
pub const NV_GPCCS_GPCCS_PWR_STATE: u32 = 0x400C24;

pub const NV_GPC_CNTL: u32 = 0x401000;
pub const NV_TPC_CNTL: u32 = 0x402000;
pub const NV_SM_CNTL: u32 = 0x403000;
pub const NV_SM_WARP_CNTL: u32 = 0x403004;
pub const NV_SM_REG_CNTL: u32 = 0x403008;
pub const NV_SM_SHARED_MEM: u32 = 0x40300C;
pub const NV_SM_TEXTURE_CNTL: u32 = 0x403010;
pub const NV_SM_CONSTANT_CNTL: u32 = 0x403014;
pub const NV_SM_LOCAL_MEM: u32 = 0x403018;
pub const NV_SM_GLOBAL_MEM: u32 = 0x40301C;
pub const NV_ROP_CNTL: u32 = 0x404000;
pub const NV_ROP_DEPTH_CNTL: u32 = 0x404004;
pub const NV_ROP_STENCIL_CNTL: u32 = 0x404008;
pub const NV_ROP_BLEND_CNTL: u32 = 0x40400C;
pub const NV_ROP_COLOR_CNTL: u32 = 0x404010;
pub const NV_ROP_MASK_CNTL: u32 = 0x404014;
pub const NV_ROP_CLEAR_CNTL: u32 = 0x404018;
pub const NV_TEX_CNTL: u32 = 0x405000;
pub const NV_TEX_FILTER: u32 = 0x405004;
pub const NV_TEX_WRAP: u32 = 0x405008;
pub const NV_TEX_LOD: u32 = 0x40500C;
pub const NV_TEX_ADDR: u32 = 0x405010;
pub const NV_TEX_DATA: u32 = 0x405014;
pub const NV_TEX_MIP_CNTL: u32 = 0x405018;
pub const NV_SHADER_CNTL: u32 = 0x406000;
pub const NV_SHADER_VTX_CNTL: u32 = 0x406004;
pub const NV_SHADER_FRAG_CNTL: u32 = 0x406008;
pub const NV_SHADER_GEOM_CNTL: u32 = 0x40600C;
pub const NV_SHADER_COMPUTE_CNTL: u32 = 0x406010;
pub const NV_SHADER_TESS_CNTL: u32 = 0x406014;
pub const NV_SHADER_UNIFORM_CNTL: u32 = 0x406018;
pub const NV_SHADER_ATTRIB_CNTL: u32 = 0x40601C;
pub const NV_COMPUTE_CNTL: u32 = 0x407000;
pub const NV_COMPUTE_QUEUE: u32 = 0x407004;
pub const NV_COMPUTE_DISPATCH: u32 = 0x407008;
pub const NV_COMPUTE_SYNC: u32 = 0x40700C;
pub const NV_COMPUTE_ATOMIC: u32 = 0x407010;
pub const NV_COMPUTE_SHARED: u32 = 0x407014;
pub const NV_COMPUTE_LOCAL: u32 = 0x407018;
pub const NV_RASTER_CNTL: u32 = 0x408000;
pub const NV_RASTER_CLIP_CNTL: u32 = 0x408004;
pub const NV_RASTER_CULL_CNTL: u32 = 0x408008;
pub const NV_RASTER_SCISSOR_CNTL: u32 = 0x40800C;
pub const NV_RASTER_MSAA_CNTL: u32 = 0x408010;
pub const NV_RASTER_AA_CNTL: u32 = 0x408014;
pub const NV_RASTER_OFFSET_CNTL: u32 = 0x408018;
pub const NV_DEPTH_CNTL: u32 = 0x409000;
pub const NV_DEPTH_FUNC: u32 = 0x409004;
pub const NV_DEPTH_BOUNDS: u32 = 0x409008;
pub const NV_DEPTH_WRITE_MASK: u32 = 0x40900C;
pub const NV_DEPTH_CLEAR: u32 = 0x409010;
pub const NV_DEPTH_HIZ: u32 = 0x409014;
pub const NV_DEPTH_STENCIL_CNTL: u32 = 0x409018;
pub const NV_DEPTH_STENCIL_FUNC: u32 = 0x40901C;
pub const NV_STENCIL_CNTL: u32 = 0x40A000;
pub const NV_STENCIL_FUNC: u32 = 0x40A004;
pub const NV_STENCIL_REF: u32 = 0x40A008;
pub const NV_STENCIL_MASK: u32 = 0x40A00C;
pub const NV_STENCIL_OP_FAIL: u32 = 0x40A010;
pub const NV_STENCIL_OP_ZFAIL: u32 = 0x40A014;
pub const NV_STENCIL_OP_ZPASS: u32 = 0x40A018;
pub const NV_STENCIL_CLEAR: u32 = 0x40A01C;
pub const NV_BLEND_CNTL: u32 = 0x40B000;
pub const NV_BLEND_EQUATION_RGB: u32 = 0x40B004;
pub const NV_BLEND_EQUATION_ALPHA: u32 = 0x40B008;
pub const NV_BLEND_FUNC_SRC_RGB: u32 = 0x40B00C;
pub const NV_BLEND_FUNC_DST_RGB: u32 = 0x40B010;
pub const NV_BLEND_FUNC_SRC_ALPHA: u32 = 0x40B014;
pub const NV_BLEND_FUNC_DST_ALPHA: u32 = 0x40B018;
pub const NV_BLEND_COLOR: u32 = 0x40B01C;
pub const NV_BLEND_SEPARATE_CNTL: u32 = 0x40B020;
pub const NV_COLOR0_BASE: u32 = 0x40C000;
pub const NV_COLOR0_SIZE: u32 = 0x40C004;
pub const NV_COLOR0_PITCH: u32 = 0x40C008;
pub const NV_COLOR0_INFO: u32 = 0x40C00C;
pub const NV_COLOR1_BASE: u32 = 0x40C010;
pub const NV_COLOR1_SIZE: u32 = 0x40C014;
pub const NV_COLOR1_PITCH: u32 = 0x40C018;
pub const NV_COLOR1_INFO: u32 = 0x40C01C;
pub const NV_COLOR2_BASE: u32 = 0x40C020;
pub const NV_COLOR2_SIZE: u32 = 0x40C024;
pub const NV_COLOR2_PITCH: u32 = 0x40C028;
pub const NV_COLOR2_INFO: u32 = 0x40C02C;
pub const NV_COLOR3_BASE: u32 = 0x40C030;
pub const NV_COLOR3_SIZE: u32 = 0x40C034;
pub const NV_COLOR3_PITCH: u32 = 0x40C038;
pub const NV_COLOR3_INFO: u32 = 0x40C03C;
pub const NV_FRAMEBUFFER_ADDR: u32 = 0x40D000;
pub const NV_FRAMEBUFFER_SIZE: u32 = 0x40D004;
pub const NV_FRAMEBUFFER_PITCH: u32 = 0x40D008;
pub const NV_FRAMEBUFFER_FORMAT: u32 = 0x40D00C;
pub const NV_RENDER_TARGET_ADDR: u32 = 0x40D010;
pub const NV_RENDER_TARGET_SIZE: u32 = 0x40D014;
pub const NV_RENDER_TARGET_COUNT: u32 = 0x40D018;
pub const NV_DEPTH_STENCIL_ADDR: u32 = 0x40D01C;
pub const NV_DEPTH_STENCIL_SIZE: u32 = 0x40D020;
pub const NV_MULTISAMPLE_CNTL: u32 = 0x40E000;
pub const NV_MULTISAMPLE_COUNT: u32 = 0x40E004;
pub const NV_MULTISAMPLE_SAMPLE_LOCS: u32 = 0x40E008;
pub const NV_MULTISAMPLE_COVERAGE: u32 = 0x40E00C;
pub const NV_MULTISAMPLE_RESOLVE: u32 = 0x40E010;
pub const NV_ANTIALIAS_CNTL: u32 = 0x40F000;
pub const NV_ANTIALIAS_MODE: u32 = 0x40F004;
pub const NV_ANTIALIAS_SAMPLES: u32 = 0x40F008;
pub const NV_ANTIALIAS_FILTER: u32 = 0x40F00C;
pub const NV_VIEWPORT_XSCALE: u32 = 0x410000;
pub const NV_VIEWPORT_XOFFSET: u32 = 0x410004;
pub const NV_VIEWPORT_YSCALE: u32 = 0x410008;
pub const NV_VIEWPORT_YOFFSET: u32 = 0x41000C;
pub const NV_VIEWPORT_ZSCALE: u32 = 0x410010;
pub const NV_VIEWPORT_ZOFFSET: u32 = 0x410014;
pub const NV_VIEWPORT_DEPTH_RANGE: u32 = 0x410018;
pub const NV_SCISSOR_X: u32 = 0x411000;
pub const NV_SCISSOR_Y: u32 = 0x411004;
pub const NV_SCISSOR_WIDTH: u32 = 0x411008;
pub const NV_SCISSOR_HEIGHT: u32 = 0x41100C;
pub const NV_SCISSOR_ENABLE: u32 = 0x411010;
pub const NV_VERTEX_CACHE_CNTL: u32 = 0x412000;
pub const NV_VERTEX_CACHE_SIZE: u32 = 0x412004;
pub const NV_VERTEX_CACHE_FLUSH: u32 = 0x412008;
pub const NV_INDEX_BUFFER_ADDR: u32 = 0x413000;
pub const NV_INDEX_BUFFER_SIZE: u32 = 0x413004;
pub const NV_INDEX_BUFFER_FORMAT: u32 = 0x413008;
pub const NV_PRIMITIVE_BUFFER_ADDR: u32 = 0x414000;
pub const NV_PRIMITIVE_BUFFER_SIZE: u32 = 0x414004;
pub const NV_UNIFORM_BUFFER_ADDR: u32 = 0x415000;
pub const NV_UNIFORM_BUFFER_SIZE: u32 = 0x415004;
pub const NV_UNIFORM_BUFFER_COUNT: u32 = 0x415008;
pub const NV_SHADER_STORAGE_ADDR: u32 = 0x416000;
pub const NV_SHADER_STORAGE_SIZE: u32 = 0x416004;
pub const NV_SHADER_STORAGE_COUNT: u32 = 0x416008;
pub const NV_ATOMIC_COUNTER_ADDR: u32 = 0x417000;
pub const NV_ATOMIC_COUNTER_COUNT: u32 = 0x417004;
pub const NV_QUERY_OBJECT_ADDR: u32 = 0x418000;
pub const NV_QUERY_OBJECT_COUNT: u32 = 0x418004;
pub const NV_QUERY_OBJECT_TYPE: u32 = 0x418008;
pub const NV_QUERY_OBJECT_RESULT: u32 = 0x41800C;
pub const NV_PCIE_LC_CNTL: u32 = 0x419000;
pub const NV_PCIE_LC_CNTL2: u32 = 0x419004;
pub const NV_PCIE_LC_CNTL3: u32 = 0x419008;
pub const NV_PCIE_LC_CNTL4: u32 = 0x41900C;
pub const NV_PCIE_LC_CNTL5: u32 = 0x419010;
pub const NV_PCIE_LC_CNTL6: u32 = 0x419014;
pub const NV_PCIE_LC_CNTL7: u32 = 0x419018;
pub const NV_PCIE_LC_CNTL8: u32 = 0x41901C;
pub const NV_PCIE_LC_CNTL9: u32 = 0x419020;
pub const NV_PCIE_LC_CNTL10: u32 = 0x419024;
pub const NV_PCIE_LC_CNTL11: u32 = 0x419028;
pub const NV_POWER_PLAY_SCLK_DPM: u32 = 0x41A000;
pub const NV_POWER_PLAY_MCLK_DPM: u32 = 0x41A004;
pub const NV_POWER_PLAY_VDD_CURR_DEP: u32 = 0x41A008;
pub const NV_POWER_PLAY_SCLK_PW: u32 = 0x41A00C;
pub const NV_POWER_PLAY_MCLK_PW: u32 = 0x41A010;
pub const NV_POWER_PLAY_TEMPERATURE: u32 = 0x41A014;
pub const NV_POWER_PLAY_FAN_CTRL: u32 = 0x41A018;
pub const NV_POWER_PLAY_POWER_STATE: u32 = 0x41A01C;
pub const NV_POWER_PLAY_GFX_CLOCK_GATING: u32 = 0x41A020;
pub const NV_POWER_PLAY_CP_CLOCK_GATING: u32 = 0x41A024;
pub const NV_SMU_SYSTEM_CNTL: u32 = 0x41B000;
pub const NV_SMU_SYSTEM_INTERRUPT_MASK: u32 = 0x41B004;
pub const NV_SMU_SYSTEM_PWR_CNTL: u32 = 0x41B008;
pub const NV_SMU_GFX_PWR_CNTL: u32 = 0x41B00C;
pub const NV_SMU_UVD_PWR_CNTL: u32 = 0x41B010;
pub const NV_SMU_VCE_PWR_CNTL: u32 = 0x41B014;
pub const NV_RAY_TRACING_CNTL: u32 = 0x41C000;
pub const NV_RAY_TRACING_BVH_BUILD: u32 = 0x41C004;
pub const NV_RAY_TRACING_TRACE: u32 = 0x41C008;
pub const NV_RAY_TRACING_SHADER_TABLE: u32 = 0x41C00C;
pub const NV_RAY_TRACING_SBT_ADDR: u32 = 0x41C010;
pub const NV_RAY_TRACING_SBT_SIZE: u32 = 0x41C014;
pub const NV_RAY_TRACING_RAY_GEN_OFFSET: u32 = 0x41C018;
pub const NV_RAY_TRACING_MISS_OFFSET: u32 = 0x41C01C;
pub const NV_RAY_TRACING_HIT_GROUP_OFFSET: u32 = 0x41C020;
pub const NV_RAY_TRACING_MAX_RECURSION: u32 = 0x41C024;
pub const NV_MESH_SHADER_CNTL: u32 = 0x41D000;
pub const NV_MESH_SHADER_TASK_CNTL: u32 = 0x41D004;
pub const NV_MESH_SHADER_MESH_CNTL: u32 = 0x41D008;
pub const NV_MESH_SHADER_AMPLIFICATION_CNTL: u32 = 0x41D00C;
pub const NV_TENSOR_CORE_CNTL: u32 = 0x41E000;
pub const NV_TENSOR_CORE_MMA: u32 = 0x41E004;
pub const NV_TENSOR_CORE_DOT: u32 = 0x41E008;
pub const NV_TENSOR_CORE_ACCUM: u32 = 0x41E00C;
pub const NV_DLSS_CNTL: u32 = 0x41F000;
pub const NV_DLSS_FRAME_GEN: u32 = 0x41F004;
pub const NV_DLSS_RAY_RECON: u32 = 0x41F008;
pub const NV_DLSS_SUPER_RES: u32 = 0x41F00C;
pub const NV_VARIABLE_RATE_SHADING_CNTL: u32 = 0x420000;
pub const NV_VARIABLE_RATE_SHADING_MAP: u32 = 0x420004;
pub const NV_VARIABLE_RATE_SHADING_ATOMIC: u32 = 0x420008;
pub const NV_MESHLET_COMPRESSION_CNTL: u32 = 0x421000;
pub const NV_MESHLET_COMPRESSION_ENCODE: u32 = 0x421004;
pub const NV_MESHLET_COMPRESSION_DECODE: u32 = 0x421008;
pub const NV_DISPLACEMENT_MICRO_MAP_CNTL: u32 = 0x422000;
pub const NV_DISPLACEMENT_MICRO_MAP_BUILD: u32 = 0x422004;
pub const NV_DISPLACEMENT_MICRO_MAP_TRACE: u32 = 0x422008;
pub const NV_OPACITY_MICRO_MAP_CNTL: u32 = 0x423000;
pub const NV_OPACITY_MICRO_MAP_BUILD: u32 = 0x423004;
pub const NV_OPACITY_MICRO_MAP_TRACE: u32 = 0x423008;
pub const NV_SERIALIZED_WRITE_CNTL: u32 = 0x424000;
pub const NV_SHADER_EXECUTION_REORDER_CNTL: u32 = 0x425000;
pub const NV_SHADER_EXECUTION_REORDER_EN: u32 = 0x425004;
pub const NV_PATH_TRACING_CNTL: u32 = 0x426000;
pub const NV_PATH_TRACING_SAMPLES: u32 = 0x426004;
pub const NV_PATH_TRACING_BOUNCES: u32 = 0x426008;
pub const NV_PATH_TRACING_DENOISE: u32 = 0x42600C;
pub const NV_SHADER_MODULE_CREATE: u32 = 0x430000;
pub const NV_SHADER_MODULE_DESTROY: u32 = 0x430004;
pub const NV_SHADER_MODULE_SPIRV_ADDR: u32 = 0x430008;
pub const NV_SHADER_MODULE_SPIRV_SIZE: u32 = 0x43000C;
pub const NV_SHADER_MODULE_ENTRY_POINT: u32 = 0x430010;
pub const NV_DESCRIPTOR_SET_ALLOC: u32 = 0x431000;
pub const NV_DESCRIPTOR_SET_BIND_ADDR: u32 = 0x431004;
pub const NV_DESCRIPTOR_SET_BIND_SIZE: u32 = 0x431008;
pub const NV_COMMAND_BUFFER_ALLOC: u32 = 0x432000;
pub const NV_COMMAND_BUFFER_ADDR: u32 = 0x432004;
pub const NV_COMMAND_BUFFER_SIZE: u32 = 0x432008;
pub const NV_QUERY_POOL_ALLOC: u32 = 0x433000;
pub const NV_QUERY_POOL_ADDR: u32 = 0x433004;
pub const NV_QUERY_POOL_COUNT: u32 = 0x433008;
pub const NV_QUERY_POOL_TYPE: u32 = 0x43300C;
pub const NV_SWAP_CHAIN_CREATE: u32 = 0x434000;
pub const NV_SWAP_CHAIN_IMAGE_COUNT: u32 = 0x434004;
pub const NV_SWAP_CHAIN_IMAGE_ADDR: u32 = 0x434008;
pub const NV_SWAP_CHAIN_IMAGE_SIZE: u32 = 0x43400C;
pub const NV_SWAP_CHAIN_IMAGE_FORMAT: u32 = 0x434010;
pub const NV_RAY_TRACING_PIPELINE_CNTL: u32 = 0x435000;
pub const NV_RAY_TRACING_PIPELINE_SHADER_GROUP: u32 = 0x435004;
pub const NV_RAY_TRACING_PIPELINE_MAX_RECURSION: u32 = 0x435008;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct Matrix4x4 {
    pub m: [f32; 16],
}

impl Matrix4x4 {
    pub const fn identity() -> Self {
        Self {
            m: [
                1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0,
            ],
        }
    }
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

pub struct GpuInstance {
    pub device_id: u16,
    pub vendor_id: u16,
    pub engine_clock: u32,
    pub memory_clock: u32,
    pub vram_size: u32,
    pub cuda_cores: u32,
    pub sm_count: u32,
    pub rop_count: u32,
    pub texture_units: u32,
    pub memory_bus_width: u32,
    pub vertex_cache_size: u32,
    pub primitive_buffer_size: u32,
    pub command_buffer_size: u32,
    pub tensor_cores: u32,
    pub rt_cores: u32,
    pub dlss_units: u32,
    pub variable_rate_shading: bool,
    pub meshlet_compression: bool,
    pub displacement_micro_map: bool,
    pub opacity_micro_map: bool,
    pub serialized_write: bool,
    pub shader_execution_reorder: bool,
    pub path_tracing: bool,
    pub error_count: AtomicU32,
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
    pub ray_tracing_pipelines: [RayTracingPipeline; 16],
    pub ray_tracing_pipeline_count: u32,
    pub mesh_shaders: [MeshShader; 16],
    pub mesh_shader_count: u32,
}

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new() -> Self {
        Self {
            device_id: NVIDIA_GTX_1030_DEVICE_ID,
            vendor_id: NVIDIA_VENDOR_ID,
            engine_clock: 1227,
            memory_clock: 6000,
            vram_size: 2 * 1024 * 1024 * 1024,
            cuda_cores: 384,
            sm_count: 3,
            rop_count: 16,
            texture_units: 20,
            memory_bus_width: 64,
            vertex_cache_size: 64 * 1024,
            primitive_buffer_size: 256 * 1024,
            command_buffer_size: 4 * 1024 * 1024,
            tensor_cores: 0,
            rt_cores: 0,
            dlss_units: 0,
            variable_rate_shading: false,
            meshlet_compression: false,
            displacement_micro_map: false,
            opacity_micro_map: false,
            serialized_write: false,
            shader_execution_reorder: false,
            path_tracing: false,
            error_count: AtomicU32::new(0),
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

    pub fn detect(&self) -> bool {
        self.vendor_id == NVIDIA_VENDOR_ID && self.device_id == NVIDIA_GTX_1030_DEVICE_ID
    }

    pub fn init(&mut self) -> bool {
        if !self.detect() {
            return false;
        }
        self.reset_gpu();
        self.setup_pmc();
        self.setup_pcie();
        self.setup_power_play();
        self.setup_smu();
        self.setup_fecs();
        self.setup_gpccs();
        self.setup_gpc();
        self.setup_tpc();
        self.setup_sm();
        self.setup_rop();
        self.setup_texture();
        self.setup_shader();
        self.setup_compute();
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
        self.enable_3d_acceleration();
        true
    }

    fn read_mmio(&self, offset: u32) -> u32 {
        unsafe {
            core::ptr::read_volatile((REG_MMIO_BASE + offset) as *const u32)
        }
    }

    fn write_mmio(&self, offset: u32, value: u32) {
        unsafe {
            core::ptr::write_volatile((REG_MMIO_BASE + offset) as *mut u32, value);
        }
    }

    fn write_mmio_64(&self, offset: u32, value: u64) {
        self.write_mmio(offset, value as u32);
        self.write_mmio(offset + 4, (value >> 32) as u32);
    }

    fn reset_gpu(&mut self) {
        self.write_mmio(NV_PMC_ENABLE, 0x00000000);
        for _ in 0..1000 {
            core::hint::spin_loop();
        }
        self.write_mmio(NV_PMC_ENABLE, 0x00000001);
    }

    fn setup_pmc(&mut self) {
        self.write_mmio(NV_PMC_BOOT_0, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_1, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_2, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_3, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_4, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_5, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_6, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_7, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_8, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_9, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_10, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_11, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_12, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_13, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_14, 0x00000001);
        self.write_mmio(NV_PMC_BOOT_15, 0x00000001);
        self.write_mmio(NV_PMC_INTR_EN_0, 0xFFFFFFFF);
        self.write_mmio(NV_PMC_INTR_MASK_0, 0xFFFFFFFF);
    }

    fn setup_fecs(&mut self) {
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Num_Contexts, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Size, 0x00001000);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Addr, REG_VRAM_BASE + 0x100000);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Mode, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Status, 0x00000000);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Timeout, 0x000FFFFF);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control2, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control3, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control4, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control5, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control6, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control7, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control8, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control9, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control10, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control11, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control12, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control13, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control14, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_Host_Engine_Context_Switch_Control15, 0x00000001);
    }

    fn setup_gpccs(&mut self) {
        self.write_mmio(NV_GPCCS_GPCCS_CNTL, 0x00000001);
        self.write_mmio(NV_GPCCS_GPCCS_STATUS, 0x00000000);
        self.write_mmio(NV_GPCCS_GPCCS_INTR, 0x00000000);
        self.write_mmio(NV_GPCCS_GPCCS_INTR_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_GPCCS_GPCCS_PWR, 0x00000001);
        self.write_mmio(NV_GPCCS_GPCCS_CLK, 0x00000001);
        self.write_mmio(NV_GPCCS_GPCCS_THERM, 0x00000000);
        self.write_mmio(NV_GPCCS_GPCCS_FAN, 0x00000001);
        self.write_mmio(NV_GPCCS_GPCCS_VOLT, 0x00000000);
        self.write_mmio(NV_GPCCS_GPCCS_PWR_STATE, 0x00000001);
    }

    fn setup_gpc(&mut self) {
        self.write_mmio(NV_GPC_CNTL, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 4, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 8, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 12, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 16, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 20, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 24, 0x00000001);
        self.write_mmio(NV_GPC_CNTL + 28, 0x00000001);
    }

    fn setup_tpc(&mut self) {
        self.write_mmio(NV_TPC_CNTL, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 4, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 8, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 12, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 16, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 20, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 24, 0x00000001);
        self.write_mmio(NV_TPC_CNTL + 28, 0x00000001);
    }

    fn setup_sm(&mut self) {
        self.write_mmio(NV_SM_CNTL, 0x00000001);
        self.write_mmio(NV_SM_WARP_CNTL, 0x00000001);
        self.write_mmio(NV_SM_REG_CNTL, 0x00000001);
        self.write_mmio(NV_SM_SHARED_MEM, 0x00000001);
        self.write_mmio(NV_SM_TEXTURE_CNTL, 0x00000001);
        self.write_mmio(NV_SM_CONSTANT_CNTL, 0x00000001);
        self.write_mmio(NV_SM_LOCAL_MEM, 0x00000001);
        self.write_mmio(NV_SM_GLOBAL_MEM, 0x00000001);
    }

    fn setup_rop(&mut self) {
        self.write_mmio(NV_ROP_CNTL, 0x00000001);
        self.write_mmio(NV_ROP_DEPTH_CNTL, 0x00000001);
        self.write_mmio(NV_ROP_STENCIL_CNTL, 0x00000001);
        self.write_mmio(NV_ROP_BLEND_CNTL, 0x00000001);
        self.write_mmio(NV_ROP_COLOR_CNTL, 0x00000001);
        self.write_mmio(NV_ROP_MASK_CNTL, 0xFFFFFFFF);
        self.write_mmio(NV_ROP_CLEAR_CNTL, 0x00000000);
    }

    fn setup_texture(&mut self) {
        self.write_mmio(NV_TEX_CNTL, 0x00000001);
        self.write_mmio(NV_TEX_FILTER, 0x00000001);
        self.write_mmio(NV_TEX_WRAP, 0x00000001);
        self.write_mmio(NV_TEX_LOD, 0x00000001);
        self.write_mmio(NV_TEX_ADDR, 0x00000001);
        self.write_mmio(NV_TEX_DATA, 0x00000001);
        self.write_mmio(NV_TEX_MIP_CNTL, 0x00000001);
    }

    fn setup_shader(&mut self) {
        self.write_mmio(NV_SHADER_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_VTX_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_FRAG_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_GEOM_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_COMPUTE_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_TESS_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_UNIFORM_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_ATTRIB_CNTL, 0x00000001);
    }

    fn setup_compute(&mut self) {
        self.write_mmio(NV_COMPUTE_CNTL, 0x00000001);
        self.write_mmio(NV_COMPUTE_QUEUE, 0x00000001);
        self.write_mmio(NV_COMPUTE_DISPATCH, 0x00000001);
        self.write_mmio(NV_COMPUTE_SYNC, 0x00000001);
        self.write_mmio(NV_COMPUTE_ATOMIC, 0x00000001);
        self.write_mmio(NV_COMPUTE_SHARED, 0x00000001);
        self.write_mmio(NV_COMPUTE_LOCAL, 0x00000001);
    }

    fn setup_rasterizer(&mut self) {
        self.write_mmio(NV_RASTER_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_CLIP_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_CULL_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_SCISSOR_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_MSAA_CNTL, 0x00000004);
        self.write_mmio(NV_RASTER_AA_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_OFFSET_CNTL, 0x00000000);
    }

    fn setup_depth_buffer(&mut self) {
        self.write_mmio(NV_DEPTH_CNTL, 0x00000001);
        self.write_mmio(NV_DEPTH_FUNC, 0x00000003);
        self.write_mmio(NV_DEPTH_BOUNDS, 0x00000000);
        self.write_mmio(NV_DEPTH_WRITE_MASK, 0x00000001);
        self.write_mmio(NV_DEPTH_CLEAR, 0x00000000);
        self.write_mmio(NV_DEPTH_HIZ, 0x00000001);
        self.write_mmio(NV_DEPTH_STENCIL_CNTL, 0x00000001);
        self.write_mmio(NV_DEPTH_STENCIL_FUNC, 0x00000003);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(NV_STENCIL_CNTL, 0x00000001);
        self.write_mmio(NV_STENCIL_FUNC, 0x00000003);
        self.write_mmio(NV_STENCIL_REF, 0x00000000);
        self.write_mmio(NV_STENCIL_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_STENCIL_OP_FAIL, 0x00000000);
        self.write_mmio(NV_STENCIL_OP_ZFAIL, 0x00000000);
        self.write_mmio(NV_STENCIL_OP_ZPASS, 0x00000000);
        self.write_mmio(NV_STENCIL_CLEAR, 0x00000000);
    }

    fn setup_blend(&mut self) {
        self.write_mmio(NV_BLEND_CNTL, 0x00000001);
        self.write_mmio(NV_BLEND_EQUATION_RGB, 0x00000001);
        self.write_mmio(NV_BLEND_EQUATION_ALPHA, 0x00000001);
        self.write_mmio(NV_BLEND_FUNC_SRC_RGB, 0x00000101);
        self.write_mmio(NV_BLEND_FUNC_DST_RGB, 0x00000101);
        self.write_mmio(NV_BLEND_FUNC_SRC_ALPHA, 0x00000101);
        self.write_mmio(NV_BLEND_FUNC_DST_ALPHA, 0x00000101);
        self.write_mmio(NV_BLEND_COLOR, 0x00000000);
        self.write_mmio(NV_BLEND_SEPARATE_CNTL, 0x00000001);
    }

    fn setup_color_buffer(&mut self) {
        self.write_mmio(NV_COLOR0_BASE, REG_VRAM_BASE + 0x900000);
        self.write_mmio(NV_COLOR0_SIZE, self.vram_size as u32);
        self.write_mmio(NV_COLOR0_PITCH, 1024);
        self.write_mmio(NV_COLOR0_INFO, 0x00000001);
        self.write_mmio(NV_COLOR1_BASE, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(NV_COLOR1_SIZE, self.vram_size as u32);
        self.write_mmio(NV_COLOR1_PITCH, 1024);
        self.write_mmio(NV_COLOR1_INFO, 0x00000001);
        self.write_mmio(NV_COLOR2_BASE, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(NV_COLOR2_SIZE, self.vram_size as u32);
        self.write_mmio(NV_COLOR2_PITCH, 1024);
        self.write_mmio(NV_COLOR2_INFO, 0x00000001);
        self.write_mmio(NV_COLOR3_BASE, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(NV_COLOR3_SIZE, self.vram_size as u32);
        self.write_mmio(NV_COLOR3_PITCH, 1024);
        self.write_mmio(NV_COLOR3_INFO, 0x00000001);
    }

    fn setup_framebuffers(&mut self) {
        self.write_mmio(NV_FRAMEBUFFER_ADDR, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(NV_FRAMEBUFFER_SIZE, self.vram_size as u32);
        self.write_mmio(NV_FRAMEBUFFER_PITCH, 1024);
        self.write_mmio(NV_FRAMEBUFFER_FORMAT, 0x00000001);
    }

    fn setup_render_targets(&mut self) {
        self.write_mmio(NV_RENDER_TARGET_ADDR, REG_VRAM_BASE + 0xD00000);
        self.write_mmio(NV_RENDER_TARGET_SIZE, self.vram_size as u32);
        self.write_mmio(NV_RENDER_TARGET_COUNT, 0x00000008);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(NV_DEPTH_STENCIL_ADDR, REG_VRAM_BASE + 0xE00000);
        self.write_mmio(NV_DEPTH_STENCIL_SIZE, self.vram_size as u32);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(NV_MULTISAMPLE_CNTL, 0x00000001);
        self.write_mmio(NV_MULTISAMPLE_COUNT, 0x00000008);
        self.write_mmio(NV_MULTISAMPLE_SAMPLE_LOCS, 0x00000008);
        self.write_mmio(NV_MULTISAMPLE_COVERAGE, 0x00000001);
        self.write_mmio(NV_MULTISAMPLE_RESOLVE, 0x00000001);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(NV_ANTIALIAS_CNTL, 0x00000001);
        self.write_mmio(NV_ANTIALIAS_MODE, 0x00000001);
        self.write_mmio(NV_ANTIALIAS_SAMPLES, 0x00000008);
        self.write_mmio(NV_ANTIALIAS_FILTER, 0x00000001);
    }

    fn setup_viewport(&mut self) {
        self.write_mmio(NV_VIEWPORT_XSCALE, 0x00000000);
        self.write_mmio(NV_VIEWPORT_XOFFSET, 0x00000000);
        self.write_mmio(NV_VIEWPORT_YSCALE, 0x00000000);
        self.write_mmio(NV_VIEWPORT_YOFFSET, 0x00000000);
        self.write_mmio(NV_VIEWPORT_ZSCALE, 0x00000000);
        self.write_mmio(NV_VIEWPORT_ZOFFSET, 0x00000000);
        self.write_mmio(NV_VIEWPORT_DEPTH_RANGE, 0x00000000);
    }

    fn setup_scissor(&mut self) {
        self.write_mmio(NV_SCISSOR_X, 0x00000000);
        self.write_mmio(NV_SCISSOR_Y, 0x00000000);
        self.write_mmio(NV_SCISSOR_WIDTH, 0x00000000);
        self.write_mmio(NV_SCISSOR_HEIGHT, 0x00000000);
        self.write_mmio(NV_SCISSOR_ENABLE, 0x00000000);
    }

    fn setup_vertex_cache(&mut self) {
        self.write_mmio(NV_VERTEX_CACHE_CNTL, 0x00000001);
        self.write_mmio(NV_VERTEX_CACHE_SIZE, self.vertex_cache_size);
        self.write_mmio(NV_VERTEX_CACHE_FLUSH, 0x00000001);
    }

    fn setup_index_buffer(&mut self) {
        self.write_mmio(NV_INDEX_BUFFER_ADDR, REG_VRAM_BASE + 0x600000);
        self.write_mmio(NV_INDEX_BUFFER_SIZE, self.primitive_buffer_size);
        self.write_mmio(NV_INDEX_BUFFER_FORMAT, 0x00000001);
    }

    fn setup_primitive_buffer(&mut self) {
        self.write_mmio(NV_PRIMITIVE_BUFFER_ADDR, REG_VRAM_BASE + 0x700000);
        self.write_mmio(NV_PRIMITIVE_BUFFER_SIZE, self.primitive_buffer_size);
    }

    fn setup_uniform_buffers(&mut self) {
        self.write_mmio(NV_UNIFORM_BUFFER_ADDR, REG_VRAM_BASE + 0x800000);
        self.write_mmio(NV_UNIFORM_BUFFER_SIZE, self.vram_size as u32);
        self.write_mmio(NV_UNIFORM_BUFFER_COUNT, 0x00000008);
    }

    fn setup_shader_storage_buffers(&mut self) {
        self.write_mmio(NV_SHADER_STORAGE_ADDR, REG_VRAM_BASE + 0x900000);
        self.write_mmio(NV_SHADER_STORAGE_SIZE, self.vram_size as u32);
        self.write_mmio(NV_SHADER_STORAGE_COUNT, 0x00000008);
    }

    fn setup_atomic_counters(&mut self) {
        self.write_mmio(NV_ATOMIC_COUNTER_ADDR, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(NV_ATOMIC_COUNTER_COUNT, 0x00001000);
    }

    fn setup_query_objects(&mut self) {
        self.write_mmio(NV_QUERY_OBJECT_ADDR, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(NV_QUERY_OBJECT_COUNT, 0x00001000);
        self.write_mmio(NV_QUERY_OBJECT_TYPE, 0x00000001);
        self.write_mmio(NV_QUERY_OBJECT_RESULT, 0x00000000);
    }

    fn setup_pcie(&mut self) {
        self.write_mmio(NV_PCIE_LC_CNTL, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL2, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL3, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL4, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL5, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL6, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL7, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL8, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL9, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL10, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL11, 0x00000001);
    }

    fn setup_power_play(&mut self) {
        self.write_mmio(NV_POWER_PLAY_SCLK_DPM, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_MCLK_DPM, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_VDD_CURR_DEP, 0x00000000);
        self.write_mmio(NV_POWER_PLAY_SCLK_PW, 0x00000000);
        self.write_mmio(NV_POWER_PLAY_MCLK_PW, 0x00000000);
        self.write_mmio(NV_POWER_PLAY_TEMPERATURE, 0x00000000);
        self.write_mmio(NV_POWER_PLAY_FAN_CTRL, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_POWER_STATE, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_GFX_CLOCK_GATING, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_CP_CLOCK_GATING, 0x00000001);
    }

    fn setup_smu(&mut self) {
        self.write_mmio(NV_SMU_SYSTEM_CNTL, 0x00000001);
        self.write_mmio(NV_SMU_SYSTEM_INTERRUPT_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_SMU_SYSTEM_PWR_CNTL, 0x00000001);
        self.write_mmio(NV_SMU_GFX_PWR_CNTL, 0x00000001);
        self.write_mmio(NV_SMU_UVD_PWR_CNTL, 0x00000001);
        self.write_mmio(NV_SMU_VCE_PWR_CNTL, 0x00000001);
    }

    fn setup_ray_tracing(&mut self) {
        self.write_mmio(NV_RAY_TRACING_CNTL, 0x00000001);
        self.write_mmio(NV_RAY_TRACING_BVH_BUILD, 0x00000001);
        self.write_mmio(NV_RAY_TRACING_TRACE, 0x00000001);
        self.write_mmio(NV_RAY_TRACING_SHADER_TABLE, 0x00000001);
        self.write_mmio(NV_RAY_TRACING_SBT_ADDR, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(NV_RAY_TRACING_SBT_SIZE, 0x00100000);
        self.write_mmio(NV_RAY_TRACING_RAY_GEN_OFFSET, 0x00000000);
        self.write_mmio(NV_RAY_TRACING_MISS_OFFSET, 0x00010000);
        self.write_mmio(NV_RAY_TRACING_HIT_GROUP_OFFSET, 0x00020000);
        self.write_mmio(NV_RAY_TRACING_MAX_RECURSION, 0x00000008);
    }

    fn setup_mesh_shader(&mut self) {
        self.write_mmio(NV_MESH_SHADER_CNTL, 0x00000001);
        self.write_mmio(NV_MESH_SHADER_TASK_CNTL, 0x00000001);
        self.write_mmio(NV_MESH_SHADER_MESH_CNTL, 0x00000001);
        self.write_mmio(NV_MESH_SHADER_AMPLIFICATION_CNTL, 0x00000001);
    }

    fn setup_tensor_core(&mut self) {
        self.write_mmio(NV_TENSOR_CORE_CNTL, 0x00000001);
        self.write_mmio(NV_TENSOR_CORE_MMA, 0x00000001);
        self.write_mmio(NV_TENSOR_CORE_DOT, 0x00000001);
        self.write_mmio(NV_TENSOR_CORE_ACCUM, 0x00000001);
    }

    fn setup_dlss(&mut self) {
        self.write_mmio(NV_DLSS_CNTL, 0x00000001);
        self.write_mmio(NV_DLSS_FRAME_GEN, 0x00000001);
        self.write_mmio(NV_DLSS_RAY_RECON, 0x00000001);
        self.write_mmio(NV_DLSS_SUPER_RES, 0x00000001);
    }

    fn setup_variable_rate_shading(&mut self) {
        self.write_mmio(NV_VARIABLE_RATE_SHADING_CNTL, 0x00000001);
        self.write_mmio(NV_VARIABLE_RATE_SHADING_MAP, REG_VRAM_BASE + 0xD00000);
        self.write_mmio(NV_VARIABLE_RATE_SHADING_ATOMIC, 0x00000001);
    }

    fn setup_meshlet_compression(&mut self) {
        self.write_mmio(NV_MESHLET_COMPRESSION_CNTL, 0x00000001);
        self.write_mmio(NV_MESHLET_COMPRESSION_ENCODE, 0x00000001);
        self.write_mmio(NV_MESHLET_COMPRESSION_DECODE, 0x00000001);
    }

    fn setup_displacement_micro_map(&mut self) {
        self.write_mmio(NV_DISPLACEMENT_MICRO_MAP_CNTL, 0x00000001);
        self.write_mmio(NV_DISPLACEMENT_MICRO_MAP_BUILD, 0x00000001);
        self.write_mmio(NV_DISPLACEMENT_MICRO_MAP_TRACE, 0x00000001);
    }

    fn setup_opacity_micro_map(&mut self) {
        self.write_mmio(NV_OPACITY_MICRO_MAP_CNTL, 0x00000001);
        self.write_mmio(NV_OPACITY_MICRO_MAP_BUILD, 0x00000001);
        self.write_mmio(NV_OPACITY_MICRO_MAP_TRACE, 0x00000001);
    }

    fn setup_serialized_write(&mut self) {
        self.write_mmio(NV_SERIALIZED_WRITE_CNTL, 0x00000001);
    }

    fn setup_shader_execution_reorder(&mut self) {
        self.write_mmio(NV_SHADER_EXECUTION_REORDER_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_EXECUTION_REORDER_EN, 0x00000001);
    }

    fn setup_path_tracing(&mut self) {
        self.write_mmio(NV_PATH_TRACING_CNTL, 0x00000001);
        self.write_mmio(NV_PATH_TRACING_SAMPLES, 0x00000004);
        self.write_mmio(NV_PATH_TRACING_BOUNCES, 0x00000005);
        self.write_mmio(NV_PATH_TRACING_DENOISE, 0x00000001);
    }

    fn wait_idle(&self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(0xFF00);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
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

    pub fn cuda_cores(&self) -> u32 {
        self.cuda_cores
    }

    pub fn sm_count(&self) -> u32 {
        self.sm_count
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

    pub fn tensor_cores(&self) -> u32 {
        self.tensor_cores
    }

    pub fn rt_cores(&self) -> u32 {
        self.rt_cores
    }

    pub fn dlss_units(&self) -> u32 {
        self.dlss_units
    }

    pub fn variable_rate_shading(&self) -> bool {
        self.variable_rate_shading
    }

    pub fn meshlet_compression(&self) -> bool {
        self.meshlet_compression
    }

    pub fn displacement_micro_map(&self) -> bool {
        self.displacement_micro_map
    }

    pub fn opacity_micro_map(&self) -> bool {
        self.opacity_micro_map
    }

    pub fn serialized_write(&self) -> bool {
        self.serialized_write
    }

    pub fn shader_execution_reorder(&self) -> bool {
        self.shader_execution_reorder
    }

    pub fn path_tracing(&self) -> bool {
        self.path_tracing
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
        self.write_mmio(NV_SHADER_MODULE_CREATE, id);
        self.write_mmio_64(NV_SHADER_MODULE_SPIRV_ADDR, spirv_addr);
        self.write_mmio(NV_SHADER_MODULE_SPIRV_SIZE, spirv_size);
        self.write_mmio_64(NV_SHADER_MODULE_ENTRY_POINT, entry_point_addr);
        self.write_mmio(NV_SHADER_MODULE_ENTRY_POINT + 4, entry_point_size);
        id
    }

    pub fn destroy_shader_module(&mut self, id: u32) {
        self.write_mmio(NV_SHADER_MODULE_DESTROY, id);
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
        self.write_mmio(NV_DESCRIPTOR_SET_ALLOC, id);
        self.write_mmio_64(NV_DESCRIPTOR_SET_BIND_ADDR, addr);
        self.write_mmio(NV_DESCRIPTOR_SET_BIND_SIZE, size);
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
        self.write_mmio(NV_COMMAND_BUFFER_ALLOC, id);
        self.write_mmio_64(NV_COMMAND_BUFFER_ADDR, addr);
        self.write_mmio(NV_COMMAND_BUFFER_SIZE, size);
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
        self.write_mmio(NV_QUERY_POOL_ALLOC, id);
        self.write_mmio_64(NV_QUERY_POOL_ADDR, addr);
        self.write_mmio(NV_QUERY_POOL_COUNT, count);
        self.write_mmio(NV_QUERY_POOL_TYPE, query_type);
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
        self.write_mmio(NV_SWAP_CHAIN_CREATE, id);
        self.write_mmio(NV_SWAP_CHAIN_IMAGE_COUNT, image_count);
        self.write_mmio_64(NV_SWAP_CHAIN_IMAGE_ADDR, image_addr);
        self.write_mmio(NV_SWAP_CHAIN_IMAGE_SIZE, image_size);
        self.write_mmio(NV_SWAP_CHAIN_IMAGE_FORMAT, image_format);
        id
    }

    pub fn create_ray_tracing_pipeline(&mut self, shader_group_count: u32, shader_group_addrs: *const u64, shader_group_sizes: *const u32, max_recursion_depth: u32, traversal_addr: u64, traversal_size: u32, sbt_addr: u64, sbt_size: u32, ray_gen_offset: u32, miss_offset: u32, hit_group_offset: u32) -> u32 {
        let id = self.ray_tracing_pipeline_count;
        if id >= 16 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        let mut addrs = [0; 64];
        let mut sizes = [0; 64];
        unsafe {
            for i in 0..shader_group_count as usize {
                addrs[i] = shader_group_addrs.add(i).read();
                sizes[i] = shader_group_sizes.add(i).read();
            }
        }
        self.ray_tracing_pipelines[id as usize] = RayTracingPipeline {
            id,
            shader_group_count,
            shader_group_addrs: addrs,
            shader_group_sizes: sizes,
            max_recursion_depth,
            traversal_addr,
            traversal_size,
            sbt_addr,
            sbt_size,
            ray_gen_offset,
            miss_offset,
            hit_group_offset,
        };
        self.ray_tracing_pipeline_count += 1;
        self.write_mmio(NV_RAY_TRACING_PIPELINE_CNTL, id);
        self.write_mmio(NV_RAY_TRACING_PIPELINE_SHADER_GROUP, shader_group_count);
        self.write_mmio(NV_RAY_TRACING_PIPELINE_MAX_RECURSION, max_recursion_depth);
        self.write_mmio_64(NV_RAY_TRACING_SBT_ADDR, sbt_addr);
        self.write_mmio(NV_RAY_TRACING_SBT_SIZE, sbt_size);
        id
    }

    pub fn create_mesh_shader(&mut self, task_shader_addr: u64, task_shader_size: u32, mesh_shader_addr: u64, mesh_shader_size: u32, amplification_shader_addr: u64, amplification_shader_size: u32, vertex_attr_count: u32, vertex_attr_addrs: *const u64) -> u32 {
        let id = self.mesh_shader_count;
        if id >= 16 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        let mut va_addrs = [0; 32];
        unsafe {
            for i in 0..vertex_attr_count as usize {
                va_addrs[i] = vertex_attr_addrs.add(i).read();
            }
        }
        self.mesh_shaders[id as usize] = MeshShader {
            id,
            task_shader_addr,
            task_shader_size,
            mesh_shader_addr,
            mesh_shader_size,
            amplification_shader_addr,
            amplification_shader_size,
            vertex_attr_count,
            vertex_attr_addrs: va_addrs,
        };
        self.mesh_shader_count += 1;
        self.write_mmio(NV_MESH_SHADER_CNTL, id);
        self.write_mmio(NV_MESH_SHADER_TASK_CNTL, id);
        self.write_mmio(NV_MESH_SHADER_MESH_CNTL, id);
        self.write_mmio(NV_MESH_SHADER_AMPLIFICATION_CNTL, id);
        id
    }

    pub fn trace_rays(&mut self, pipeline_id: u32, width: u32, height: u32, depth: u32) {
        self.write_mmio(NV_RAY_TRACING_CNTL, pipeline_id);
        self.write_mmio(0x130028, width);
        self.write_mmio(0x13002C, height);
        self.write_mmio(0x130030, depth);
        self.write_mmio(NV_RAY_TRACING_TRACE, 0x00000001);
    }

    pub fn dispatch_mesh(&mut self, mesh_id: u32, task_count_x: u32, task_count_y: u32, task_count_z: u32) {
        self.write_mmio(NV_MESH_SHADER_CNTL, mesh_id);
        self.write_mmio(0x131010, task_count_x);
        self.write_mmio(0x131014, task_count_y);
        self.write_mmio(0x131018, task_count_z);
        self.write_mmio(0x13101C, 0x00000001);
    }

    pub fn tensor_op(&mut self, mma_addr: u64, dot_addr: u64, accum_addr: u64, m: u32, n: u32, k: u32) {
        self.write_mmio(NV_TENSOR_CORE_CNTL, 0x00000001);
        self.write_mmio_64(NV_TENSOR_CORE_MMA, mma_addr);
        self.write_mmio_64(NV_TENSOR_CORE_DOT, dot_addr);
        self.write_mmio_64(NV_TENSOR_CORE_ACCUM, accum_addr);
        self.write_mmio(0x132010, m);
        self.write_mmio(0x132014, n);
        self.write_mmio(0x132018, k);
        self.write_mmio(0x13201C, 0x00000001);
    }

    pub fn dlss_upscale(&mut self, input_addr: u64, output_addr: u64, input_w: u32, input_h: u32, output_w: u32, output_h: u32) {
        self.write_mmio(NV_DLSS_CNTL, 0x00000001);
        self.write_mmio_64(0x133010, input_addr);
        self.write_mmio_64(0x133018, output_addr);
        self.write_mmio(0x133020, input_w);
        self.write_mmio(0x133024, input_h);
        self.write_mmio(0x133028, output_w);
        self.write_mmio(0x13302C, output_h);
        self.write_mmio(0x133030, 0x00000001);
    }

    pub fn path_trace(&mut self, pipeline_id: u32, samples: u32, bounces: u32, denoise: bool) {
        self.write_mmio(NV_PATH_TRACING_CNTL, pipeline_id);
        self.write_mmio(NV_PATH_TRACING_SAMPLES, samples);
        self.write_mmio(NV_PATH_TRACING_BOUNCES, bounces);
        self.write_mmio(NV_PATH_TRACING_DENOISE, if denoise { 1 } else { 0 });
        self.write_mmio(0x13A010, 0x00000001);
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

pub fn gtx_1030_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn gtx_1030_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn gtx_1030_driver_init() -> bool {
    gtx_1030_init()
}

#[no_mangle]
pub extern "C" fn gtx_1030_driver_detected() -> bool {
    gtx_1030_detected()
}

#[no_mangle]
pub extern "C" fn gtx_1030_submit_3d_command(command: u32, param1: u32, param2: u32, param3: u32) {
    let gpu = gpu_instance();
    gpu.submit_3d_command(command, param1, param2, param3);
}

#[no_mangle]
pub extern "C" fn gtx_1030_draw_triangle_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_triangle_3d(x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
}

#[no_mangle]
pub extern "C" fn gtx_1030_draw_line_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_line_3d(x1, y1, z1, x2, y2, z2, color);
}

#[no_mangle]
pub extern "C" fn gtx_1030_clear_screen_3d(color: u32) {
    let gpu = gpu_instance();
    gpu.clear_screen_3d(color);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_viewport(x: u32, y: u32, width: u32, height: u32) {
    let gpu = gpu_instance();
    gpu.set_viewport(x, y, width, height);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_projection_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_projection_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_world_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_world_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_view_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_view_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1030_enable_depth_test(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_depth_test(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1030_enable_blending(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_blending(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_blend_func(src: u32, dst: u32) {
    let gpu = gpu_instance();
    gpu.set_blend_func(src, dst);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_depth_func(func: u32) {
    let gpu = gpu_instance();
    gpu.set_depth_func(func);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_cull_mode(mode: u32) {
    let gpu = gpu_instance();
    gpu.set_cull_mode(mode);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_wireframe(enabled: bool) {
    let gpu = gpu_instance();
    gpu.set_wireframe(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1030_set_multisample_count(count: u32) {
    let gpu = gpu_instance();
    gpu.set_multisample_count(count);
}

#[no_mangle]
pub extern "C" fn gtx_1030_enable_multisampling(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_multisampling(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1030_enable_antialiasing(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_antialiasing(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1030_finish() {
    let gpu = gpu_instance();
    gpu.finish();
}

#[no_mangle]
pub extern "C" fn gtx_1030_flush() {
    let gpu = gpu_instance();
    gpu.flush();
}

#[no_mangle]
pub extern "C" fn gtx_1030_get_status() -> u32 {
    let gpu = gpu_instance();
    gpu.get_status()
}

#[no_mangle]
pub extern "C" fn gtx_1030_engine_clock() -> u32 {
    let gpu = gpu_instance();
    gpu.engine_clock()
}

#[no_mangle]
pub extern "C" fn gtx_1030_memory_clock() -> u32 {
    let gpu = gpu_instance();
    gpu.memory_clock()
}

#[no_mangle]
pub extern "C" fn gtx_1030_vram_size() -> u32 {
    let gpu = gpu_instance();
    gpu.vram_size()
}

#[no_mangle]
pub extern "C" fn gtx_1030_cuda_cores() -> u32 {
    let gpu = gpu_instance();
    gpu.cuda_cores()
}

#[no_mangle]
pub extern "C" fn gtx_1030_sm_count() -> u32 {
    let gpu = gpu_instance();
    gpu.sm_count()
}

#[no_mangle]
pub extern "C" fn gtx_1030_rop_count() -> u32 {
    let gpu = gpu_instance();
    gpu.rop_count()
}

#[no_mangle]
pub extern "C" fn gtx_1030_texture_units() -> u32 {
    let gpu = gpu_instance();
    gpu.texture_units()
}

#[no_mangle]
pub extern "C" fn gtx_1030_memory_bus_width() -> u32 {
    let gpu = gpu_instance();
    gpu.memory_bus_width()
}

#[no_mangle]
pub extern "C" fn gtx_1030_tensor_cores() -> u32 {
    let gpu = gpu_instance();
    gpu.tensor_cores()
}

#[no_mangle]
pub extern "C" fn gtx_1030_rt_cores() -> u32 {
    let gpu = gpu_instance();
    gpu.rt_cores()
}

#[no_mangle]
pub extern "C" fn gtx_1030_dlss_units() -> u32 {
    let gpu = gpu_instance();
    gpu.dlss_units()
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_shader_module(spirv_addr: u64, spirv_size: u32, entry_point_addr: u64, entry_point_size: u32, stage: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_shader_module(spirv_addr, spirv_size, entry_point_addr, entry_point_size, stage)
}

#[no_mangle]
pub extern "C" fn gtx_1030_destroy_shader_module(id: u32) {
    let gpu = gpu_instance();
    gpu.destroy_shader_module(id);
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_descriptor_set(addr: u64, size: u32, binding_count: u32, bindings: *const u64) -> u32 {
    let gpu = gpu_instance();
    gpu.create_descriptor_set(addr, size, binding_count, bindings)
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_command_buffer(addr: u64, size: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_command_buffer(addr, size)
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_query_pool(addr: u64, count: u32, query_type: u32, results_addr: u64) -> u32 {
    let gpu = gpu_instance();
    gpu.create_query_pool(addr, count, query_type, results_addr)
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_swap_chain(image_count: u32, image_addr: u64, image_size: u32, image_format: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_swap_chain(image_count, image_addr, image_size, image_format)
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_ray_tracing_pipeline(shader_group_count: u32, shader_group_addrs: *const u64, shader_group_sizes: *const u32, max_recursion_depth: u32, traversal_addr: u64, traversal_size: u32, sbt_addr: u64, sbt_size: u32, ray_gen_offset: u32, miss_offset: u32, hit_group_offset: u32) -> u32 {
    let gpu = gpu_instance();
    gpu.create_ray_tracing_pipeline(shader_group_count, shader_group_addrs, shader_group_sizes, max_recursion_depth, traversal_addr, traversal_size, sbt_addr, sbt_size, ray_gen_offset, miss_offset, hit_group_offset)
}

#[no_mangle]
pub extern "C" fn gtx_1030_create_mesh_shader(task_shader_addr: u64, task_shader_size: u32, mesh_shader_addr: u64, mesh_shader_size: u32, amplification_shader_addr: u64, amplification_shader_size: u32, vertex_attr_count: u32, vertex_attr_addrs: *const u64) -> u32 {
    let gpu = gpu_instance();
    gpu.create_mesh_shader(task_shader_addr, task_shader_size, mesh_shader_addr, mesh_shader_size, amplification_shader_addr, amplification_shader_size, vertex_attr_count, vertex_attr_addrs)
}

#[no_mangle]
pub extern "C" fn gtx_1030_trace_rays(pipeline_id: u32, width: u32, height: u32, depth: u32) {
    let gpu = gpu_instance();
    gpu.trace_rays(pipeline_id, width, height, depth);
}

#[no_mangle]
pub extern "C" fn gtx_1030_dispatch_mesh(mesh_id: u32, task_count_x: u32, task_count_y: u32, task_count_z: u32) {
    let gpu = gpu_instance();
    gpu.dispatch_mesh(mesh_id, task_count_x, task_count_y, task_count_z);
}

#[no_mangle]
pub extern "C" fn gtx_1030_tensor_op(mma_addr: u64, dot_addr: u64, accum_addr: u64, m: u32, n: u32, k: u32) {
    let gpu = gpu_instance();
    gpu.tensor_op(mma_addr, dot_addr, accum_addr, m, n, k);
}

#[no_mangle]
pub extern "C" fn gtx_1030_dlss_upscale(input_addr: u64, output_addr: u64, input_w: u32, input_h: u32, output_w: u32, output_h: u32) {
    let gpu = gpu_instance();
    gpu.dlss_upscale(input_addr, output_addr, input_w, input_h, output_w, output_h);
}

#[no_mangle]
pub extern "C" fn gtx_1030_path_trace(pipeline_id: u32, samples: u32, bounces: u32, denoise: bool) {
    let gpu = gpu_instance();
    gpu.path_trace(pipeline_id, samples, bounces, denoise);
}
