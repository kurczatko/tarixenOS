#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const NVIDIA_VENDOR_ID: u16 = 0x10DE;
pub const GTX_1080_DEVICE_ID: u16 = 0x1B80;
pub const GTX_1070_DEVICE_ID: u16 = 0x1B81;
pub const GTX_1060_DEVICE_ID: u16 = 0x1C60;
pub const GTX_1050_TI_DEVICE_ID: u16 = 0x1C8C;
pub const GTX_1050_DEVICE_ID: u16 = 0x1C8D;
pub const GTX_1030_DEVICE_ID: u16 = 0x1D01;
pub const CHIP_GP104: u32 = 0x1B80;
pub const CHIP_GP106: u32 = 0x1C60;
pub const CHIP_GP107: u32 = 0x1C8C;
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

pub const NV_PGRAPH_CTX_CONTROL: u32 = 0x400800;
pub const NV_PGRAPH_CTX_USER: u32 = 0x400804;
pub const NV_PGRAPH_CTX_CONTROL2: u32 = 0x400808;
pub const NV_PGRAPH_FECS_HOST_NUM_CTX: u32 = 0x40080C;
pub const NV_PGRAPH_FECS_CTX_SW_SIZE: u32 = 0x400810;
pub const NV_PGRAPH_FECS_CTX_SW_ADDR: u32 = 0x400814;
pub const NV_PGRAPH_FECS_CTX_SW_MODE: u32 = 0x400818;
pub const NV_PGRAPH_FECS_CTX_SW_STATUS: u32 = 0x40081C;
pub const NV_PGRAPH_FECS_CTX_SW_TIMEOUT: u32 = 0x400820;
pub const NV_PGRAPH_FECS_CTX_SW_CONTROL: u32 = 0x400824;
pub const NV_PGRAPH_DEBUG_0: u32 = 0x400860;
pub const NV_PGRAPH_DEBUG_1: u32 = 0x400864;
pub const NV_PGRAPH_DEBUG_2: u32 = 0x400868;
pub const NV_PGRAPH_DEBUG_3: u32 = 0x40086C;
pub const NV_PGRAPH_DEBUG_4: u32 = 0x400870;
pub const NV_PGRAPH_DEBUG_5: u32 = 0x400874;
pub const NV_PGRAPH_DEBUG_6: u32 = 0x400878;
pub const NV_PGRAPH_DEBUG_7: u32 = 0x40087C;
pub const NV_PGRAPH_TRAPPED_ADDR: u32 = 0x4008E0;
pub const NV_PGRAPH_TRAPPED_DATA: u32 = 0x4008E4;
pub const NV_PGRAPH_TRAPPED_CTX: u32 = 0x4008E8;
pub const NV_PGRAPH_TRAPPED_MTHD: u32 = 0x4008EC;
pub const NV_PGRAPH_TRAPPED_CLASS: u32 = 0x4008F0;
pub const NV_PGRAPH_TRAPPED_SUBCH: u32 = 0x4008F4;
pub const NV_PGRAPH_TRAPPED_TIMESTAMP: u32 = 0x4008F8;
pub const NV_PGRAPH_TRAPPED_NOTIFY: u32 = 0x4008FC;
pub const NV_PGRAPH_TRAPPED_OFFSET: u32 = 0x400900;
pub const NV_PGRAPH_TRAPPED_SIZE: u32 = 0x400904;
pub const NV_PGRAPH_TRAPPED_FLAGS: u32 = 0x400908;
pub const NV_PGRAPH_TRAPPED_ERROR: u32 = 0x40090C;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO: u32 = 0x400910;
pub const NV_PGRAPH_INTR_0: u32 = 0x400E00;
pub const NV_PGRAPH_INTR_EN_0: u32 = 0x400E20;
pub const NV_PGRAPH_INTR_MASK_0: u32 = 0x400E40;
pub const NV_PGRAPH_INTR_STATUS_0: u32 = 0x400E60;
pub const NV_PGRAPH_INTR_SET_0: u32 = 0x400E80;

pub const NV_GPC_BASE: u32 = 0x108000;
pub const NV_GPC_CNTL: u32 = 0x108000;
pub const NV_GPC_TPC_BASE: u32 = 0x108400;
pub const NV_TPC_CNTL: u32 = 0x108400;
pub const NV_SM_BASE: u32 = 0x108800;
pub const NV_SM_CNTL: u32 = 0x108800;
pub const NV_SM_WARP_CNTL: u32 = 0x108804;
pub const NV_SM_REG_CNTL: u32 = 0x108808;
pub const NV_SM_SHARED_MEM: u32 = 0x10880C;
pub const NV_SM_TEXTURE_CNTL: u32 = 0x108810;
pub const NV_SM_CONSTANT_CNTL: u32 = 0x108814;
pub const NV_SM_LOCAL_MEM: u32 = 0x108818;
pub const NV_SM_GLOBAL_MEM: u32 = 0x10881C;

pub const NV_ROP_BASE: u32 = 0x10A000;
pub const NV_ROP_CNTL: u32 = 0x10A000;
pub const NV_ROP_DEPTH_CNTL: u32 = 0x10A004;
pub const NV_ROP_STENCIL_CNTL: u32 = 0x10A008;
pub const NV_ROP_BLEND_CNTL: u32 = 0x10A00C;
pub const NV_ROP_COLOR_CNTL: u32 = 0x10A010;
pub const NV_ROP_MASK_CNTL: u32 = 0x10A014;
pub const NV_ROP_CLEAR_CNTL: u32 = 0x10A018;

pub const NV_TEX_BASE: u32 = 0x10C000;
pub const NV_TEX_CNTL: u32 = 0x10C000;
pub const NV_TEX_FILTER: u32 = 0x10C004;
pub const NV_TEX_WRAP: u32 = 0x10C008;
pub const NV_TEX_LOD: u32 = 0x10C00C;
pub const NV_TEX_ADDR: u32 = 0x10C010;
pub const NV_TEX_DATA: u32 = 0x10C014;
pub const NV_TEX_MIP_CNTL: u32 = 0x10C018;

pub const NV_SHADER_BASE: u32 = 0x10D000;
pub const NV_SHADER_CNTL: u32 = 0x10D000;
pub const NV_SHADER_VTX_CNTL: u32 = 0x10D004;
pub const NV_SHADER_FRAG_CNTL: u32 = 0x10D008;
pub const NV_SHADER_GEOM_CNTL: u32 = 0x10D00C;
pub const NV_SHADER_COMPUTE_CNTL: u32 = 0x10D010;
pub const NV_SHADER_TESS_CNTL: u32 = 0x10D014;
pub const NV_SHADER_UNIFORM_CNTL: u32 = 0x10D018;
pub const NV_SHADER_ATTRIB_CNTL: u32 = 0x10D01C;

pub const NV_COMPUTE_BASE: u32 = 0x10E000;
pub const NV_COMPUTE_CNTL: u32 = 0x10E000;
pub const NV_COMPUTE_QUEUE: u32 = 0x10E004;
pub const NV_COMPUTE_DISPATCH: u32 = 0x10E008;
pub const NV_COMPUTE_SYNC: u32 = 0x10E00C;
pub const NV_COMPUTE_ATOMIC: u32 = 0x10E010;
pub const NV_COMPUTE_SHARED: u32 = 0x10E014;
pub const NV_COMPUTE_LOCAL: u32 = 0x10E018;

pub const NV_RASTER_BASE: u32 = 0x10F000;
pub const NV_RASTER_CNTL: u32 = 0x10F000;
pub const NV_RASTER_CLIP_CNTL: u32 = 0x10F004;
pub const NV_RASTER_CULL_CNTL: u32 = 0x10F008;
pub const NV_RASTER_SCISSOR_CNTL: u32 = 0x10F00C;
pub const NV_RASTER_MSAA_CNTL: u32 = 0x10F010;
pub const NV_RASTER_AA_CNTL: u32 = 0x10F014;
pub const NV_RASTER_OFFSET_CNTL: u32 = 0x10F018;

pub const NV_DEPTH_BASE: u32 = 0x110000;
pub const NV_DEPTH_CNTL: u32 = 0x110000;
pub const NV_DEPTH_FUNC: u32 = 0x110004;
pub const NV_DEPTH_BOUNDS: u32 = 0x110008;
pub const NV_DEPTH_WRITE_MASK: u32 = 0x11000C;
pub const NV_DEPTH_CLEAR: u32 = 0x110010;
pub const NV_DEPTH_HIZ: u32 = 0x110014;
pub const NV_DEPTH_STENCIL_CNTL: u32 = 0x110018;
pub const NV_DEPTH_STENCIL_FUNC: u32 = 0x11001C;

pub const NV_STENCIL_BASE: u32 = 0x111000;
pub const NV_STENCIL_CNTL: u32 = 0x111000;
pub const NV_STENCIL_FUNC: u32 = 0x111004;
pub const NV_STENCIL_REF: u32 = 0x111008;
pub const NV_STENCIL_MASK: u32 = 0x11100C;
pub const NV_STENCIL_OP_FAIL: u32 = 0x111010;
pub const NV_STENCIL_OP_ZFAIL: u32 = 0x111014;
pub const NV_STENCIL_OP_ZPASS: u32 = 0x111018;
pub const NV_STENCIL_CLEAR: u32 = 0x11101C;

pub const NV_BLEND_BASE: u32 = 0x112000;
pub const NV_BLEND_CNTL: u32 = 0x112000;
pub const NV_BLEND_EQUATION_RGB: u32 = 0x112004;
pub const NV_BLEND_EQUATION_ALPHA: u32 = 0x112008;
pub const NV_BLEND_FUNC_SRC_RGB: u32 = 0x11200C;
pub const NV_BLEND_FUNC_DST_RGB: u32 = 0x112010;
pub const NV_BLEND_FUNC_SRC_ALPHA: u32 = 0x112014;
pub const NV_BLEND_FUNC_DST_ALPHA: u32 = 0x112018;
pub const NV_BLEND_COLOR: u32 = 0x11201C;
pub const NV_BLEND_SEPARATE_CNTL: u32 = 0x112020;

pub const NV_COLOR0_BASE: u32 = 0x113000;
pub const NV_COLOR0_SIZE: u32 = 0x113004;
pub const NV_COLOR0_PITCH: u32 = 0x113008;
pub const NV_COLOR0_INFO: u32 = 0x11300C;
pub const NV_COLOR1_BASE: u32 = 0x113010;
pub const NV_COLOR1_SIZE: u32 = 0x113014;
pub const NV_COLOR1_PITCH: u32 = 0x113018;
pub const NV_COLOR1_INFO: u32 = 0x11301C;
pub const NV_COLOR2_BASE: u32 = 0x113020;
pub const NV_COLOR2_SIZE: u32 = 0x113024;
pub const NV_COLOR2_PITCH: u32 = 0x113028;
pub const NV_COLOR2_INFO: u32 = 0x11302C;
pub const NV_COLOR3_BASE: u32 = 0x113030;
pub const NV_COLOR3_SIZE: u32 = 0x113034;
pub const NV_COLOR3_PITCH: u32 = 0x113038;
pub const NV_COLOR3_INFO: u32 = 0x11303C;

pub const NV_FRAMEBUFFER_BASE: u32 = 0x114000;
pub const NV_FRAMEBUFFER_ADDR: u32 = 0x114000;
pub const NV_FRAMEBUFFER_SIZE: u32 = 0x114004;
pub const NV_FRAMEBUFFER_PITCH: u32 = 0x114008;
pub const NV_FRAMEBUFFER_FORMAT: u32 = 0x11400C;

pub const NV_RENDER_TARGET_BASE: u32 = 0x115000;
pub const NV_RENDER_TARGET_ADDR: u32 = 0x115000;
pub const NV_RENDER_TARGET_SIZE: u32 = 0x115004;
pub const NV_RENDER_TARGET_COUNT: u32 = 0x115008;

pub const NV_DEPTH_STENCIL_BASE: u32 = 0x116000;
pub const NV_DEPTH_STENCIL_ADDR: u32 = 0x116000;
pub const NV_DEPTH_STENCIL_SIZE: u32 = 0x116004;

pub const NV_MULTISAMPLE_BASE: u32 = 0x117000;
pub const NV_MULTISAMPLE_CNTL: u32 = 0x117000;
pub const NV_MULTISAMPLE_COUNT: u32 = 0x117004;
pub const NV_MULTISAMPLE_SAMPLE_LOCS: u32 = 0x117008;
pub const NV_MULTISAMPLE_COVERAGE: u32 = 0x11700C;
pub const NV_MULTISAMPLE_RESOLVE: u32 = 0x117010;

pub const NV_ANTIALIAS_BASE: u32 = 0x118000;
pub const NV_ANTIALIAS_CNTL: u32 = 0x118000;
pub const NV_ANTIALIAS_MODE: u32 = 0x118004;
pub const NV_ANTIALIAS_SAMPLES: u32 = 0x118008;
pub const NV_ANTIALIAS_FILTER: u32 = 0x11800C;

pub const NV_VIEWPORT_BASE: u32 = 0x119000;
pub const NV_VIEWPORT_XSCALE: u32 = 0x119000;
pub const NV_VIEWPORT_XOFFSET: u32 = 0x119004;
pub const NV_VIEWPORT_YSCALE: u32 = 0x119008;
pub const NV_VIEWPORT_YOFFSET: u32 = 0x11900C;
pub const NV_VIEWPORT_ZSCALE: u32 = 0x119010;
pub const NV_VIEWPORT_ZOFFSET: u32 = 0x119014;
pub const NV_VIEWPORT_DEPTH_RANGE: u32 = 0x119018;

pub const NV_SCISSOR_BASE: u32 = 0x11A000;
pub const NV_SCISSOR_X: u32 = 0x11A000;
pub const NV_SCISSOR_Y: u32 = 0x11A004;
pub const NV_SCISSOR_WIDTH: u32 = 0x11A008;
pub const NV_SCISSOR_HEIGHT: u32 = 0x11A00C;
pub const NV_SCISSOR_ENABLE: u32 = 0x11A010;

pub const NV_VERTEX_CACHE_BASE: u32 = 0x11B000;
pub const NV_VERTEX_CACHE_CNTL: u32 = 0x11B000;
pub const NV_VERTEX_CACHE_SIZE: u32 = 0x11B004;
pub const NV_VERTEX_CACHE_FLUSH: u32 = 0x11B008;

pub const NV_INDEX_BUFFER_BASE: u32 = 0x11C000;
pub const NV_INDEX_BUFFER_ADDR: u32 = 0x11C000;
pub const NV_INDEX_BUFFER_SIZE: u32 = 0x11C004;
pub const NV_INDEX_BUFFER_FORMAT: u32 = 0x11C008;

pub const NV_PRIMITIVE_BUFFER_BASE: u32 = 0x11C010;
pub const NV_PRIMITIVE_BUFFER_ADDR: u32 = 0x11C010;
pub const NV_PRIMITIVE_BUFFER_SIZE: u32 = 0x11C014;

pub const NV_UNIFORM_BUFFER_BASE: u32 = 0x11D000;
pub const NV_UNIFORM_BUFFER_ADDR: u32 = 0x11D000;
pub const NV_UNIFORM_BUFFER_SIZE: u32 = 0x11D004;
pub const NV_UNIFORM_BUFFER_COUNT: u32 = 0x11D008;

pub const NV_SHADER_STORAGE_BASE: u32 = 0x11E000;
pub const NV_SHADER_STORAGE_ADDR: u32 = 0x11E000;
pub const NV_SHADER_STORAGE_SIZE: u32 = 0x11E004;
pub const NV_SHADER_STORAGE_COUNT: u32 = 0x11E008;

pub const NV_ATOMIC_COUNTER_BASE: u32 = 0x11F000;
pub const NV_ATOMIC_COUNTER_ADDR: u32 = 0x11F000;
pub const NV_ATOMIC_COUNTER_COUNT: u32 = 0x11F004;

pub const NV_QUERY_OBJECT_BASE: u32 = 0x120000;
pub const NV_QUERY_OBJECT_ADDR: u32 = 0x120000;
pub const NV_QUERY_OBJECT_COUNT: u32 = 0x120004;
pub const NV_QUERY_OBJECT_TYPE: u32 = 0x120008;
pub const NV_QUERY_OBJECT_RESULT: u32 = 0x12000C;

pub const NV_PCIE_LC_CNTL: u32 = 0x9000;
pub const NV_PCIE_LC_STATUS: u32 = 0x9004;
pub const NV_PCIE_LC_CNTL2: u32 = 0x9008;
pub const NV_PCIE_LC_CNTL3: u32 = 0x900C;
pub const NV_PCIE_LC_CNTL4: u32 = 0x9010;

pub const NV_POWER_PLAY_SCLK_DPM: u32 = 0xA000;
pub const NV_POWER_PLAY_MCLK_DPM: u32 = 0xA004;
pub const NV_POWER_PLAY_VDD_CURR_DEP: u32 = 0xA008;
pub const NV_POWER_PLAY_SCLK_PW: u32 = 0xA00C;
pub const NV_POWER_PLAY_MCLK_PW: u32 = 0xA010;
pub const NV_POWER_PLAY_TEMPERATURE: u32 = 0xA014;
pub const NV_POWER_PLAY_FAN_CTRL: u32 = 0xA018;
pub const NV_POWER_PLAY_POWER_STATE: u32 = 0xA01C;
pub const NV_POWER_PLAY_GFX_CLOCK_GATING: u32 = 0xA020;
pub const NV_POWER_PLAY_CP_CLOCK_GATING: u32 = 0xA024;

pub const NV_SMU_SYSTEM_CNTL: u32 = 0xB000;
pub const NV_SMU_SYSTEM_STATUS: u32 = 0xB004;
pub const NV_SMU_SYSTEM_INTERRUPT: u32 = 0xB008;
pub const NV_SMU_SYSTEM_INTERRUPT_MASK: u32 = 0xB00C;
pub const NV_SMU_SYSTEM_PWR_CNTL: u32 = 0xB014;
pub const NV_SMU_GFX_PWR_CNTL: u32 = 0xB020;

pub const NV_SDMA0_BASE: u32 = 0x2E00;
pub const NV_SDMA1_BASE: u32 = 0x2F00;
pub const NV_SDMA0_RB_CNTL: u32 = 0x2E00;
pub const NV_SDMA0_RB_BASE: u32 = 0x2E04;
pub const NV_SDMA0_RB_RPTR: u32 = 0x2E08;
pub const NV_SDMA0_RB_WPTR: u32 = 0x2E0C;
pub const NV_SDMA0_IB_CNTL: u32 = 0x2E10;
pub const NV_SDMA0_IB_BASE: u32 = 0x2E14;
pub const NV_SDMA0_IB_RPTR: u32 = 0x2E18;
pub const NV_SDMA0_IB_WPTR: u32 = 0x2E1C;
pub const NV_SDMA1_RB_CNTL: u32 = 0x2F00;
pub const NV_SDMA1_RB_BASE: u32 = 0x2F04;
pub const NV_SDMA1_RB_RPTR: u32 = 0x2F08;
pub const NV_SDMA1_RB_WPTR: u32 = 0x2F0C;
pub const NV_SDMA1_IB_CNTL: u32 = 0x2F10;
pub const NV_SDMA1_IB_BASE: u32 = 0x2F14;
pub const NV_SDMA1_IB_RPTR: u32 = 0x2F18;
pub const NV_SDMA1_IB_WPTR: u32 = 0x2F1C;

pub const NV_CP_BASE: u32 = 0x8800;
pub const NV_CP_ME_CNTL: u32 = 0x88C8;
pub const NV_CP_ME_CNTL2: u32 = 0x88CC;
pub const NV_CP_ME_CNTL3: u32 = 0x88E4;
pub const NV_CP_INT_CNTL: u32 = 0x8908;
pub const NV_CP_INT_MASK: u32 = 0x8910;
pub const NV_CP_PFFETCH_CNTL: u32 = 0x8918;
pub const NV_CP_PFFETCH_BASE: u32 = 0x8920;
pub const NV_CP_PFFETCH_RPTR: u32 = 0x8924;
pub const NV_CP_QUEUE_THRESHOLDS: u32 = 0x8900;
pub const NV_CP_STALL_CTRL: u32 = 0x8904;
pub const NV_CP_PFFETCH_CNTL2: u32 = 0x891C;
pub const NV_CP_ALWAYS_ON_CNTL: u32 = 0x88F4;
pub const NV_CP_ALWAYS_ON_CNTL2: u32 = 0x88F8;

pub const NV_VGT_CNTL: u32 = 0x8960;
pub const NV_VGT_PRIMITIVE_TYPE: u32 = 0x8964;
pub const NV_VGT_SHADER_STAGES_EN: u32 = 0x8A80;
pub const NV_VGT_GSVS_RING_OFFSET_0: u32 = 0x8A8C;
pub const NV_VGT_GSVS_RING_SIZE: u32 = 0x8A9C;
pub const NV_VGT_CONFIG: u32 = 0x8B10;

pub const NV_PA_SC_MODE_CNTL_0: u32 = 0x8C94;
pub const NV_PA_SC_AA_CONFIG: u32 = 0x8D2C;

pub const NV_DB_DEPTH_CONTROL: u32 = 0x2F04;
pub const NV_DB_STENCIL_CONTROL: u32 = 0x2F08;
pub const NV_DB_Z_INFO: u32 = 0x2F20;

pub const NV_CB_COLOR_CONTROL: u32 = 0x16CC;
pub const NV_CB_COLOR0_BASE: u32 = 0x16D0;
pub const NV_CB_COLOR0_SIZE: u32 = 0x16D4;

pub const NV_SHADER_MODULE_BASE: u32 = 0x13E000;
pub const NV_SHADER_MODULE_CNTL: u32 = 0x13E000;
pub const NV_SHADER_MODULE_CREATE: u32 = 0x13E004;
pub const NV_SHADER_MODULE_DESTROY: u32 = 0x13E008;
pub const NV_SHADER_MODULE_SPIRV_ADDR: u32 = 0x13E00C;
pub const NV_SHADER_MODULE_SPIRV_SIZE: u32 = 0x13E010;
pub const NV_SHADER_MODULE_ENTRY_POINT: u32 = 0x13E014;

pub const NV_DESCRIPTOR_SET_BASE: u32 = 0x13F000;
pub const NV_DESCRIPTOR_SET_CNTL: u32 = 0x13F000;
pub const NV_DESCRIPTOR_SET_ALLOC: u32 = 0x13F004;
pub const NV_DESCRIPTOR_SET_FREE: u32 = 0x13F008;
pub const NV_DESCRIPTOR_SET_UPDATE: u32 = 0x13F00C;
pub const NV_DESCRIPTOR_SET_BIND: u32 = 0x13F010;
pub const NV_DESCRIPTOR_SET_BIND_ADDR: u32 = 0x13F014;
pub const NV_DESCRIPTOR_SET_BIND_SIZE: u32 = 0x13F018;

pub const NV_COMMAND_BUFFER_BASE: u32 = 0x140000;
pub const NV_COMMAND_BUFFER_CNTL: u32 = 0x140000;
pub const NV_COMMAND_BUFFER_ALLOC: u32 = 0x140004;
pub const NV_COMMAND_BUFFER_FREE: u32 = 0x140008;
pub const NV_COMMAND_BUFFER_BEGIN: u32 = 0x14000C;
pub const NV_COMMAND_BUFFER_END: u32 = 0x140010;
pub const NV_COMMAND_BUFFER_SUBMIT: u32 = 0x140014;
pub const NV_COMMAND_BUFFER_RESET: u32 = 0x140018;
pub const NV_COMMAND_BUFFER_ADDR: u32 = 0x14001C;
pub const NV_COMMAND_BUFFER_SIZE: u32 = 0x140020;

pub const NV_QUERY_POOL_BASE: u32 = 0x141000;
pub const NV_QUERY_POOL_CNTL: u32 = 0x141000;
pub const NV_QUERY_POOL_ALLOC: u32 = 0x141004;
pub const NV_QUERY_POOL_FREE: u32 = 0x141008;
pub const NV_QUERY_POOL_RESET: u32 = 0x14100C;
pub const NV_QUERY_POOL_RESULTS: u32 = 0x141010;
pub const NV_QUERY_POOL_TYPE: u32 = 0x141014;
pub const NV_QUERY_POOL_COUNT: u32 = 0x141018;

pub const NV_SWAP_CHAIN_BASE: u32 = 0x142000;
pub const NV_SWAP_CHAIN_CNTL: u32 = 0x142000;
pub const NV_SWAP_CHAIN_CREATE: u32 = 0x142004;
pub const NV_SWAP_CHAIN_DESTROY: u32 = 0x142008;
pub const NV_SWAP_CHAIN_PRESENT: u32 = 0x14200C;
pub const NV_SWAP_CHAIN_ACQUIRE: u32 = 0x142010;
pub const NV_SWAP_CHAIN_IMAGE_COUNT: u32 = 0x142014;
pub const NV_SWAP_CHAIN_IMAGE_ADDR: u32 = 0x142018;
pub const NV_SWAP_CHAIN_IMAGE_SIZE: u32 = 0x14201C;
pub const NV_SWAP_CHAIN_IMAGE_FORMAT: u32 = 0x142020;

pub const NV_GRAPHICS_PIPELINE_BASE: u32 = 0x13D000;
pub const NV_GRAPHICS_PIPELINE_CNTL: u32 = 0x13D000;
pub const NV_GRAPHICS_PIPELINE_SHADER_STAGE: u32 = 0x13D004;
pub const NV_GRAPHICS_PIPELINE_VERTEX_INPUT: u32 = 0x13D008;
pub const NV_GRAPHICS_PIPELINE_INPUT_ASSEMBLY: u32 = 0x13D00C;
pub const NV_GRAPHICS_PIPELINE_RASTERIZATION: u32 = 0x13D010;
pub const NV_GRAPHICS_PIPELINE_COLOR_BLEND: u32 = 0x13D014;
pub const NV_GRAPHICS_PIPELINE_DEPTH_STENCIL: u32 = 0x13D018;
pub const NV_GRAPHICS_PIPELINE_VIEWPORT: u32 = 0x13D01C;
pub const NV_GRAPHICS_PIPELINE_MULTISAMPLE: u32 = 0x13D020;
pub const NV_GRAPHICS_PIPELINE_DYNAMIC: u32 = 0x13D024;

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
    pub push_constant_range_count: u32,
    pub push_constant_offsets: [u32; 16],
    pub push_constant_sizes: [u32; 16],
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
pub struct GraphicsPipeline {
    pub id: u32,
    pub shader_stage_count: u32,
    pub shader_stage_addrs: [u64; 8],
    pub vertex_input_addr: u64,
    pub input_assembly_addr: u64,
    pub rasterization_addr: u64,
    pub color_blend_addr: u64,
    pub depth_stencil_addr: u64,
    pub viewport_addr: u64,
    pub multisample_addr: u64,
    pub dynamic_state_addr: u64,
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
    pub cuda_cores: u32,
    pub sm_count: u32,
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
    pub graphics_pipelines: [GraphicsPipeline; 16],
    pub graphics_pipeline_count: u32,
    pub shader_programs: [ShaderProgram; 32],
    pub shader_program_count: u32,
}

pub const GTX_1080_VRAM_SIZE: u64 = 8 * 1024 * 1024 * 1024;
pub const GTX_1070_VRAM_SIZE: u64 = 8 * 1024 * 1024 * 1024;
pub const GTX_1060_VRAM_SIZE: u64 = 6 * 1024 * 1024 * 1024;
pub const GTX_1050_TI_VRAM_SIZE: u64 = 4 * 1024 * 1024 * 1024;
pub const GTX_1050_VRAM_SIZE: u64 = 2 * 1024 * 1024 * 1024;
pub const GTX_1030_VRAM_SIZE: u64 = 2 * 1024 * 1024 * 1024;
pub const GTX_1000_GART_SIZE: u64 = 4 * 1024 * 1024 * 1024;

pub const GTX_1080_ENGINE_CLOCK: u32 = 1600000;
pub const GTX_1070_ENGINE_CLOCK: u32 = 1500000;
pub const GTX_1060_ENGINE_CLOCK: u32 = 1500000;
pub const GTX_1050_TI_ENGINE_CLOCK: u32 = 1290000;
pub const GTX_1050_ENGINE_CLOCK: u32 = 1354000;
pub const GTX_1030_ENGINE_CLOCK: u32 = 1252000;

pub const GTX_1080_MEMORY_CLOCK: u32 = 10000000;
pub const GTX_1070_MEMORY_CLOCK: u32 = 8000000;
pub const GTX_1060_MEMORY_CLOCK: u32 = 8000000;
pub const GTX_1050_TI_MEMORY_CLOCK: u32 = 7000000;
pub const GTX_1050_MEMORY_CLOCK: u32 = 7000000;
pub const GTX_1030_MEMORY_CLOCK: u32 = 6000000;

pub const GTX_1080_CUDA_CORES: u32 = 2560;
pub const GTX_1070_CUDA_CORES: u32 = 1920;
pub const GTX_1060_CUDA_CORES: u32 = 1280;
pub const GTX_1050_TI_CUDA_CORES: u32 = 768;
pub const GTX_1050_CUDA_CORES: u32 = 640;
pub const GTX_1030_CUDA_CORES: u32 = 384;

pub const GTX_1080_SM_COUNT: u32 = 20;
pub const GTX_1070_SM_COUNT: u32 = 15;
pub const GTX_1060_SM_COUNT: u32 = 10;
pub const GTX_1050_TI_SM_COUNT: u32 = 6;
pub const GTX_1050_SM_COUNT: u32 = 5;
pub const GTX_1030_SM_COUNT: u32 = 3;

pub const GTX_1080_ROP_COUNT: u32 = 64;
pub const GTX_1070_ROP_COUNT: u32 = 64;
pub const GTX_1060_ROP_COUNT: u32 = 48;
pub const GTX_1050_TI_ROP_COUNT: u32 = 32;
pub const GTX_1050_ROP_COUNT: u32 = 32;
pub const GTX_1030_ROP_COUNT: u32 = 16;

pub const GTX_1080_TEXTURE_UNITS: u32 = 160;
pub const GTX_1070_TEXTURE_UNITS: u32 = 120;
pub const GTX_1060_TEXTURE_UNITS: u32 = 80;
pub const GTX_1050_TI_TEXTURE_UNITS: u32 = 48;
pub const GTX_1050_TEXTURE_UNITS: u32 = 40;
pub const GTX_1030_TEXTURE_UNITS: u32 = 24;

pub const GTX_1080_MEMORY_BUS_WIDTH: u32 = 256;
pub const GTX_1070_MEMORY_BUS_WIDTH: u32 = 256;
pub const GTX_1060_MEMORY_BUS_WIDTH: u32 = 192;
pub const GTX_1050_TI_MEMORY_BUS_WIDTH: u32 = 128;
pub const GTX_1050_MEMORY_BUS_WIDTH: u32 = 128;
pub const GTX_1030_MEMORY_BUS_WIDTH: u32 = 64;

pub const GTX_1080_VERTEX_CACHE_SIZE: u32 = 64 * 1024;
pub const GTX_1070_VERTEX_CACHE_SIZE: u32 = 64 * 1024;
pub const GTX_1060_VERTEX_CACHE_SIZE: u32 = 48 * 1024;
pub const GTX_1050_TI_VERTEX_CACHE_SIZE: u32 = 32 * 1024;
pub const GTX_1050_VERTEX_CACHE_SIZE: u32 = 32 * 1024;
pub const GTX_1030_VERTEX_CACHE_SIZE: u32 = 16 * 1024;

pub const GTX_1000_PRIMITIVE_BUFFER_SIZE: u32 = 32 * 1024;

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new(device_id: u16) -> Self {
        let (vram_size, engine_clock, memory_clock, cuda_cores, sm_count, rop_count, texture_units, memory_bus_width, vertex_cache_size) = match device_id {
            GTX_1080_DEVICE_ID => (GTX_1080_VRAM_SIZE, GTX_1080_ENGINE_CLOCK, GTX_1080_MEMORY_CLOCK, GTX_1080_CUDA_CORES, GTX_1080_SM_COUNT, GTX_1080_ROP_COUNT, GTX_1080_TEXTURE_UNITS, GTX_1080_MEMORY_BUS_WIDTH, GTX_1080_VERTEX_CACHE_SIZE),
            GTX_1070_DEVICE_ID => (GTX_1070_VRAM_SIZE, GTX_1070_ENGINE_CLOCK, GTX_1070_MEMORY_CLOCK, GTX_1070_CUDA_CORES, GTX_1070_SM_COUNT, GTX_1070_ROP_COUNT, GTX_1070_TEXTURE_UNITS, GTX_1070_MEMORY_BUS_WIDTH, GTX_1070_VERTEX_CACHE_SIZE),
            GTX_1060_DEVICE_ID => (GTX_1060_VRAM_SIZE, GTX_1060_ENGINE_CLOCK, GTX_1060_MEMORY_CLOCK, GTX_1060_CUDA_CORES, GTX_1060_SM_COUNT, GTX_1060_ROP_COUNT, GTX_1060_TEXTURE_UNITS, GTX_1060_MEMORY_BUS_WIDTH, GTX_1060_VERTEX_CACHE_SIZE),
            GTX_1050_TI_DEVICE_ID => (GTX_1050_TI_VRAM_SIZE, GTX_1050_TI_ENGINE_CLOCK, GTX_1050_TI_MEMORY_CLOCK, GTX_1050_TI_CUDA_CORES, GTX_1050_TI_SM_COUNT, GTX_1050_TI_ROP_COUNT, GTX_1050_TI_TEXTURE_UNITS, GTX_1050_TI_MEMORY_BUS_WIDTH, GTX_1050_TI_VERTEX_CACHE_SIZE),
            GTX_1050_DEVICE_ID => (GTX_1050_VRAM_SIZE, GTX_1050_ENGINE_CLOCK, GTX_1050_MEMORY_CLOCK, GTX_1050_CUDA_CORES, GTX_1050_SM_COUNT, GTX_1050_ROP_COUNT, GTX_1050_TEXTURE_UNITS, GTX_1050_MEMORY_BUS_WIDTH, GTX_1050_VERTEX_CACHE_SIZE),
            GTX_1030_DEVICE_ID => (GTX_1030_VRAM_SIZE, GTX_1030_ENGINE_CLOCK, GTX_1030_MEMORY_CLOCK, GTX_1030_CUDA_CORES, GTX_1030_SM_COUNT, GTX_1030_ROP_COUNT, GTX_1030_TEXTURE_UNITS, GTX_1030_MEMORY_BUS_WIDTH, GTX_1030_VERTEX_CACHE_SIZE),
            _ => (GTX_1060_VRAM_SIZE, GTX_1060_ENGINE_CLOCK, GTX_1060_MEMORY_CLOCK, GTX_1060_CUDA_CORES, GTX_1060_SM_COUNT, GTX_1060_ROP_COUNT, GTX_1060_TEXTURE_UNITS, GTX_1060_MEMORY_BUS_WIDTH, GTX_1060_VERTEX_CACHE_SIZE),
        };
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size,
            vram_used: AtomicU64::new(0),
            gart_size: GTX_1000_GART_SIZE,
            engine_clock,
            memory_clock,
            cuda_cores,
            sm_count,
            rop_count,
            texture_units,
            memory_bus_width,
            vertex_cache_size,
            primitive_buffer_size: GTX_1000_PRIMITIVE_BUFFER_SIZE,
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
            descriptor_sets: unsafe { core::mem::zeroed() },
            descriptor_set_count: 0,
            command_buffers: unsafe { core::mem::zeroed() },
            command_buffer_count: 0,
            query_pools: unsafe { core::mem::zeroed() },
            query_pool_count: 0,
            swap_chains: unsafe { core::mem::zeroed() },
            swap_chain_count: 0,
            graphics_pipelines: unsafe { core::mem::zeroed() },
            graphics_pipeline_count: 0,
            shader_programs: unsafe { core::mem::zeroed() },
            shader_program_count: 0,
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
            vendor == NVIDIA_VENDOR_ID as u32 && (device == GTX_1080_DEVICE_ID as u32 || device == GTX_1070_DEVICE_ID as u32 || device == GTX_1060_DEVICE_ID as u32 || device == GTX_1050_TI_DEVICE_ID as u32 || device == GTX_1050_DEVICE_ID as u32 || device == GTX_1030_DEVICE_ID as u32) && rev != 0xFFFFFFFF
        }
    }

    pub fn device_id(&self) -> u16 {
        unsafe { self.read_mmio(0x04) as u16 }
    }

    pub fn soft_reset(&mut self) {
        self.write_mmio(NV_PMC_CNTRL, 0x00000001);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(NV_PMC_CNTRL, 0x00000000);
        self.wait_idle_inner();
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(NV_PGRAPH_INTR_STATUS_0);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
        self.error_count.fetch_add(1, Ordering::SeqCst);
    }

    pub fn wait_idle(&self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(NV_PGRAPH_INTR_STATUS_0);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn init_ring_buffers(&mut self) {
        self.write_mmio(NV_CP_PFFETCH_BASE, (REG_VRAM_BASE + 0x100000) as u32);
        self.write_mmio(NV_CP_PFFETCH_BASE + 4, ((REG_VRAM_BASE + 0x100000) >> 32) as u32);
        self.write_mmio(NV_CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(NV_CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(NV_CP_QUEUE_THRESHOLDS, 0x00000001);
        self.write_mmio(NV_CP_STALL_CTRL, 0x00000000);
        self.write_mmio(NV_CP_INT_CNTL, 0x00000001);
        self.write_mmio(NV_CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_CP_PFFETCH_RPTR, 0x00000000);
        self.write_mmio(NV_SDMA0_RB_BASE, (REG_VRAM_BASE + 0x2000000) as u32);
        self.write_mmio(NV_SDMA0_RB_BASE + 4, ((REG_VRAM_BASE + 0x2000000) >> 32) as u32);
        self.write_mmio(NV_SDMA0_RB_CNTL, 0x00000001);
        self.write_mmio(NV_SDMA0_IB_BASE, (REG_VRAM_BASE + 0x3000000) as u32);
        self.write_mmio(NV_SDMA0_IB_BASE + 4, ((REG_VRAM_BASE + 0x3000000) >> 32) as u32);
        self.write_mmio(NV_SDMA0_IB_CNTL, 0x00000001);
        self.write_mmio(NV_SDMA1_RB_BASE, (REG_VRAM_BASE + 0x4000000) as u32);
        self.write_mmio(NV_SDMA1_RB_BASE + 4, ((REG_VRAM_BASE + 0x4000000) >> 32) as u32);
        self.write_mmio(NV_SDMA1_RB_CNTL, 0x00000001);
        self.write_mmio(NV_SDMA1_IB_BASE, (REG_VRAM_BASE + 0x5000000) as u32);
        self.write_mmio(NV_SDMA1_IB_BASE + 4, ((REG_VRAM_BASE + 0x5000000) >> 32) as u32);
        self.write_mmio(NV_SDMA1_IB_CNTL, 0x00000001);
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
        self.write_mmio(NV_CP_PFFETCH_RPTR, wptr);
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
        self.setup_pmc();
        self.setup_pgraph();
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
        self.setup_pcie();
        self.setup_power_play();
        self.setup_smu();
        self.init_ring_buffers();
        self.enable_3d_acceleration();
        self.initialized.store(true, Ordering::SeqCst);
        true
    }

    fn setup_pmc(&mut self) {
        self.write_mmio(NV_PMC_ENABLE, 0x00000001);
        self.write_mmio(NV_PMC_CNTRL, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL2, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL3, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL4, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL5, 0x00000000);
    }

    fn setup_pgraph(&mut self) {
        self.write_mmio(NV_PGRAPH_CTX_CONTROL, 0x00000001);
        self.write_mmio(NV_PGRAPH_CTX_USER, 0x00000000);
        self.write_mmio(NV_PGRAPH_CTX_CONTROL2, 0x00000001);
        self.write_mmio(NV_PGRAPH_DEBUG_0, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_1, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_2, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_3, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_4, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_5, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_6, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_7, 0x00000000);
    }

    fn setup_fecs(&mut self) {
        self.write_mmio(NV_PGRAPH_FECS_HOST_NUM_CTX, 0x00000008);
        self.write_mmio(NV_PGRAPH_FECS_CTX_SW_SIZE, 0x00001000);
        self.write_mmio(NV_PGRAPH_FECS_CTX_SW_MODE, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_CTX_SW_CONTROL, 0x00000001);
        self.write_mmio(NV_PGRAPH_FECS_CTX_SW_TIMEOUT, 0x00010000);
    }

    fn setup_gpccs(&mut self) {
        self.write_mmio(NV_GPCCS_GPCCS_CNTL, 0x00000001);
        self.write_mmio(NV_GPCCS_GPCCS_PWR, 0x00000001);
        self.write_mmio(NV_GPCCS_GPCCS_CLK, 0x00000001);
    }

    fn setup_gpc(&mut self) {
        self.write_mmio(NV_GPC_CNTL, 0x00000001);
    }

    fn setup_tpc(&mut self) {
        self.write_mmio(NV_TPC_CNTL, 0x00000001);
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
        self.write_mmio(NV_TEX_LOD, 0x00000000);
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
        self.write_mmio(NV_RASTER_MSAA_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_AA_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_OFFSET_CNTL, 0x00000000);
    }

    fn setup_depth_buffer(&mut self) {
        self.write_mmio(NV_DEPTH_CNTL, 0x00000001);
        self.write_mmio(NV_DEPTH_FUNC, 0x00000003);
        self.write_mmio(NV_DEPTH_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_DEPTH_CLEAR, 0xFFFFFFFF);
        self.write_mmio(NV_DEPTH_HIZ, 0x00000001);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(NV_STENCIL_CNTL, 0x00000001);
        self.write_mmio(NV_STENCIL_FUNC, 0x00000007);
        self.write_mmio(NV_STENCIL_REF, 0x00000000);
        self.write_mmio(NV_STENCIL_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_STENCIL_OP_FAIL, 0x00000000);
        self.write_mmio(NV_STENCIL_OP_ZFAIL, 0x00000000);
        self.write_mmio(NV_STENCIL_OP_ZPASS, 0x00000000);
        self.write_mmio(NV_STENCIL_CLEAR, 0x00000000);
    }

    fn setup_blend(&mut self) {
        self.write_mmio(NV_BLEND_CNTL, 0x00000001);
        self.write_mmio(NV_BLEND_EQUATION_RGB, 0x00000000);
        self.write_mmio(NV_BLEND_EQUATION_ALPHA, 0x00000000);
        self.write_mmio(NV_BLEND_FUNC_SRC_RGB, 0x00000001);
        self.write_mmio(NV_BLEND_FUNC_DST_RGB, 0x00000000);
        self.write_mmio(NV_BLEND_FUNC_SRC_ALPHA, 0x00000001);
        self.write_mmio(NV_BLEND_FUNC_DST_ALPHA, 0x00000000);
        self.write_mmio(NV_BLEND_COLOR, 0x00000000);
        self.write_mmio(NV_BLEND_SEPARATE_CNTL, 0x00000000);
    }

    fn setup_color_buffer(&mut self) {
        self.write_mmio(NV_COLOR0_BASE, 0x00000000);
        self.write_mmio(NV_COLOR0_SIZE, 0x00000000);
        self.write_mmio(NV_COLOR0_PITCH, 0x00000000);
        self.write_mmio(NV_COLOR0_INFO, 0x00000000);
    }

    fn setup_framebuffers(&mut self) {
        self.write_mmio(NV_FRAMEBUFFER_ADDR, 0x00000000);
        self.write_mmio(NV_FRAMEBUFFER_SIZE, 0x00000000);
        self.write_mmio(NV_FRAMEBUFFER_PITCH, 0x00000000);
        self.write_mmio(NV_FRAMEBUFFER_FORMAT, 0x00000000);
    }

    fn setup_render_targets(&mut self) {
        self.write_mmio(NV_RENDER_TARGET_ADDR, 0x00000000);
        self.write_mmio(NV_RENDER_TARGET_SIZE, 0x00000000);
        self.write_mmio(NV_RENDER_TARGET_COUNT, 0x00000001);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(NV_DEPTH_STENCIL_ADDR, 0x00000000);
        self.write_mmio(NV_DEPTH_STENCIL_SIZE, 0x00000000);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(NV_MULTISAMPLE_CNTL, 0x00000001);
        self.write_mmio(NV_MULTISAMPLE_COUNT, 0x00000001);
        self.write_mmio(NV_MULTISAMPLE_RESOLVE, 0x00000001);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(NV_ANTIALIAS_CNTL, 0x00000001);
        self.write_mmio(NV_ANTIALIAS_MODE, 0x00000001);
        self.write_mmio(NV_ANTIALIAS_SAMPLES, 0x00000001);
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
        self.write_mmio(NV_INDEX_BUFFER_ADDR, 0x00000000);
        self.write_mmio(NV_INDEX_BUFFER_SIZE, 0x00000000);
        self.write_mmio(NV_INDEX_BUFFER_FORMAT, 0x00000000);
    }

    fn setup_primitive_buffer(&mut self) {
        self.write_mmio(NV_PRIMITIVE_BUFFER_ADDR, 0x00000000);
        self.write_mmio(NV_PRIMITIVE_BUFFER_SIZE, self.primitive_buffer_size);
    }

    fn setup_uniform_buffers(&mut self) {
        self.write_mmio(NV_UNIFORM_BUFFER_ADDR, 0x00000000);
        self.write_mmio(NV_UNIFORM_BUFFER_SIZE, 0x00000000);
        self.write_mmio(NV_UNIFORM_BUFFER_COUNT, 0x00000000);
    }

    fn setup_shader_storage_buffers(&mut self) {
        self.write_mmio(NV_SHADER_STORAGE_ADDR, 0x00000000);
        self.write_mmio(NV_SHADER_STORAGE_SIZE, 0x00000000);
        self.write_mmio(NV_SHADER_STORAGE_COUNT, 0x00000000);
    }

    fn setup_atomic_counters(&mut self) {
        self.write_mmio(NV_ATOMIC_COUNTER_ADDR, 0x00000000);
        self.write_mmio(NV_ATOMIC_COUNTER_COUNT, 0x00000000);
    }

    fn setup_query_objects(&mut self) {
        self.write_mmio(NV_QUERY_OBJECT_ADDR, 0x00000000);
        self.write_mmio(NV_QUERY_OBJECT_COUNT, 0x00000000);
    }

    fn setup_pcie(&mut self) {
        self.write_mmio(NV_PCIE_LC_CNTL, 0x00000001);
        self.write_mmio(NV_PCIE_LC_CNTL2, 0x00000000);
        self.write_mmio(NV_PCIE_LC_CNTL3, 0x00000000);
        self.write_mmio(NV_PCIE_LC_CNTL4, 0x00000000);
    }

    fn setup_power_play(&mut self) {
        self.write_mmio(NV_POWER_PLAY_SCLK_DPM, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_MCLK_DPM, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_SCLK_PW, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_MCLK_PW, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_GFX_CLOCK_GATING, 0x00000001);
        self.write_mmio(NV_POWER_PLAY_CP_CLOCK_GATING, 0x00000001);
    }

    fn setup_smu(&mut self) {
        self.write_mmio(NV_SMU_SYSTEM_CNTL, 0x00000001);
        self.write_mmio(NV_SMU_SYSTEM_INTERRUPT_MASK, 0xFFFFFFFF);
        self.write_mmio(NV_SMU_SYSTEM_PWR_CNTL, 0x00000001);
        self.write_mmio(NV_SMU_GFX_PWR_CNTL, 0x00000001);
    }

    pub fn enable_3d_acceleration(&mut self) {
        self.write_mmio(NV_SHADER_CNTL, 0x00000001);
        self.write_mmio(NV_RASTER_CNTL, 0x00000001);
        self.write_mmio(NV_ROP_CNTL, 0x00000001);
        self.write_mmio(NV_DEPTH_CNTL, 0x00000001);
        self.write_mmio(NV_BLEND_CNTL, 0x00000001);
        self.write_mmio(NV_TEX_CNTL, 0x00000001);
        self.write_mmio(NV_COMPUTE_CNTL, 0x00000001);
        self.write_mmio(NV_PGRAPH_CTX_CONTROL, 0x00000001);
        self.write_mmio(NV_PGRAPH_CTX_CONTROL2, 0x00000001);
        self.write_mmio(NV_GPC_CNTL, 0x00000001);
        self.write_mmio(NV_SM_CNTL, 0x00000001);
        self.wait_idle();
    }

    pub fn compile_shader(&mut self, stage: u32, spirv_addr: u64, spirv_size: u32, entry_point: u64, entry_point_size: u32) -> u32 {
        let id = self.shader_module_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.write_mmio(NV_SHADER_MODULE_CREATE, id);
        self.write_mmio_64(NV_SHADER_MODULE_SPIRV_ADDR, spirv_addr);
        self.write_mmio(NV_SHADER_MODULE_SPIRV_SIZE, spirv_size);
        self.write_mmio_64(NV_SHADER_MODULE_ENTRY_POINT, entry_point);
        self.write_mmio(NV_SHADER_MODULE_CNTL, stage);
        self.shader_modules[id as usize] = ShaderModule {
            id,
            spirv_addr,
            spirv_size,
            entry_point_addr: entry_point,
            entry_point_size,
            stage,
        };
        self.shader_module_count += 1;
        id
    }

    pub fn destroy_shader_module(&mut self, id: u32) {
        self.write_mmio(NV_SHADER_MODULE_DESTROY, id);
        if id < 32 {
            self.shader_modules[id as usize] = unsafe { core::mem::zeroed() };
        }
    }

    pub fn create_shader_program(&mut self, vs_addr: u64, fs_addr: u64, gs_addr: u64, tcs_addr: u64, tes_addr: u64, cs_addr: u64, uniform_count: u32, uniform_offsets: &[u32], descriptor_set_count: u32, descriptor_set_addrs: &[u64], push_constant_range_count: u32, push_constant_offsets: &[u32], push_constant_sizes: &[u32]) -> u32 {
        let id = self.shader_program_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        let mut offsets = [0u32; 256];
        let mut dset_addrs = [0u64; 32];
        let mut pc_offsets = [0u32; 16];
        let mut pc_sizes = [0u32; 16];
        for i in 0..uniform_offsets.len().min(256) {
            offsets[i] = uniform_offsets[i];
        }
        for i in 0..descriptor_set_addrs.len().min(32) {
            dset_addrs[i] = descriptor_set_addrs[i];
        }
        for i in 0..push_constant_offsets.len().min(16) {
            pc_offsets[i] = push_constant_offsets[i];
        }
        for i in 0..push_constant_sizes.len().min(16) {
            pc_sizes[i] = push_constant_sizes[i];
        }
        self.shader_programs[id as usize] = ShaderProgram {
            id,
            vertex_shader_addr: vs_addr,
            fragment_shader_addr: fs_addr,
            geometry_shader_addr: gs_addr,
            tessellation_ctrl_addr: tcs_addr,
            tessellation_eval_addr: tes_addr,
            compute_shader_addr: cs_addr,
            uniform_count,
            uniform_offsets: offsets,
            descriptor_set_count,
            descriptor_set_addrs: dset_addrs,
            push_constant_range_count,
            push_constant_offsets: pc_offsets,
            push_constant_sizes: pc_sizes,
        };
        self.shader_program_count += 1;
        id
    }

    pub fn bind_shader_program(&mut self, program_id: u32) {
        if program_id >= self.shader_program_count {
            return;
        }
        let prog = &self.shader_programs[program_id as usize];
        self.write_mmio(NV_SHADER_VTX_CNTL, 0x00000001);
        self.write_mmio(NV_SHADER_FRAG_CNTL, 0x00000001);
        self.write_mmio_64(NV_SHADER_MODULE_SPIRV_ADDR, prog.vertex_shader_addr);
        self.write_mmio_64(NV_SHADER_MODULE_SPIRV_ADDR + 8, prog.fragment_shader_addr);
        self.write_mmio(NV_SHADER_CNTL, 0x00000001);
    }

    pub fn set_uniform(&mut self, program_id: u32, index: u32, values: &[f32]) {
        if program_id >= self.shader_program_count || index >= 256 {
            return;
        }
        let offset = self.shader_programs[program_id as usize].uniform_offsets[index as usize];
        self.write_mmio(NV_SHADER_UNIFORM_CNTL, offset);
        for i in 0..values.len().min(4) {
            self.write_mmio(NV_SHADER_UNIFORM_CNTL + 4 + i as u32 * 4, values[i].to_bits());
        }
    }

    pub fn set_viewport(&mut self, viewport: &Viewport) {
        self.write_mmio(NV_VIEWPORT_XSCALE, viewport.width.to_bits());
        self.write_mmio(NV_VIEWPORT_XOFFSET, viewport.x.to_bits());
        self.write_mmio(NV_VIEWPORT_YSCALE, viewport.height.to_bits());
        self.write_mmio(NV_VIEWPORT_YOFFSET, viewport.y.to_bits());
        self.write_mmio(NV_VIEWPORT_ZSCALE, (viewport.max_depth - viewport.min_depth).to_bits());
        self.write_mmio(NV_VIEWPORT_ZOFFSET, viewport.min_depth.to_bits());
        self.write_mmio(NV_VIEWPORT_DEPTH_RANGE, (viewport.min_depth.to_bits() << 16) | (viewport.max_depth.to_bits() & 0xFFFF));
    }

    pub fn set_scissor(&mut self, scissor: &Scissor) {
        self.write_mmio(NV_SCISSOR_X, scissor.x as u32);
        self.write_mmio(NV_SCISSOR_Y, scissor.y as u32);
        self.write_mmio(NV_SCISSOR_WIDTH, scissor.width as u32);
        self.write_mmio(NV_SCISSOR_HEIGHT, scissor.height as u32);
        self.write_mmio(NV_SCISSOR_ENABLE, 0x00000001);
    }

    pub fn set_blend_state(&mut self, enable: bool, src_rgb: u32, dst_rgb: u32, src_alpha: u32, dst_alpha: u32, equation_rgb: u32, equation_alpha: u32) {
        self.write_mmio(NV_BLEND_CNTL, if enable { 0x00000001 } else { 0x00000000 });
        self.write_mmio(NV_BLEND_FUNC_SRC_RGB, src_rgb);
        self.write_mmio(NV_BLEND_FUNC_DST_RGB, dst_rgb);
        self.write_mmio(NV_BLEND_FUNC_SRC_ALPHA, src_alpha);
        self.write_mmio(NV_BLEND_FUNC_DST_ALPHA, dst_alpha);
        self.write_mmio(NV_BLEND_EQUATION_RGB, equation_rgb);
        self.write_mmio(NV_BLEND_EQUATION_ALPHA, equation_alpha);
    }

    pub fn set_depth_state(&mut self, enable: bool, func: u32, write_mask: u32) {
        self.write_mmio(NV_DEPTH_CNTL, if enable { 0x00000001 } else { 0x00000000 });
        self.write_mmio(NV_DEPTH_FUNC, func);
        self.write_mmio(NV_DEPTH_WRITE_MASK, write_mask);
    }

    pub fn set_stencil_state(&mut self, enable: bool, func: u32, ref_val: u32, mask: u32, op_fail: u32, op_zfail: u32, op_zpass: u32) {
        self.write_mmio(NV_STENCIL_CNTL, if enable { 0x00000001 } else { 0x00000000 });
        self.write_mmio(NV_STENCIL_FUNC, func);
        self.write_mmio(NV_STENCIL_REF, ref_val);
        self.write_mmio(NV_STENCIL_MASK, mask);
        self.write_mmio(NV_STENCIL_OP_FAIL, op_fail);
        self.write_mmio(NV_STENCIL_OP_ZFAIL, op_zfail);
        self.write_mmio(NV_STENCIL_OP_ZPASS, op_zpass);
    }

    pub fn set_rasterizer_state(&mut self, cull: u32, clip: u32, msaa: bool, aa: bool) {
        self.write_mmio(NV_RASTER_CULL_CNTL, cull);
        self.write_mmio(NV_RASTER_CLIP_CNTL, clip);
        self.write_mmio(NV_RASTER_MSAA_CNTL, if msaa { 0x00000001 } else { 0x00000000 });
        self.write_mmio(NV_RASTER_AA_CNTL, if aa { 0x00000001 } else { 0x00000000 });
    }

    pub fn create_texture(&mut self, desc: &TextureDescriptor) -> u32 {
        self.write_mmio(NV_TEX_CNTL, 0x00000001);
        self.write_mmio(NV_TEX_ADDR, desc.address as u32);
        self.write_mmio(NV_TEX_ADDR + 4, (desc.address >> 32) as u32);
        self.write_mmio(NV_TEX_FILTER, desc.format);
        self.write_mmio(NV_TEX_WRAP, desc.flags);
        self.write_mmio(NV_TEX_LOD, desc.mip_levels);
        0
    }

    pub fn bind_texture(&mut self, unit: u32, address: u64) {
        self.write_mmio(NV_TEX_ADDR + unit * 16, address as u32);
        self.write_mmio(NV_TEX_ADDR + unit * 16 + 4, (address >> 32) as u32);
    }

    pub fn create_framebuffer(&mut self, width: u32, height: u32, format: u32, address: u64) -> u32 {
        self.write_mmio(NV_FRAMEBUFFER_ADDR, address as u32);
        self.write_mmio(NV_FRAMEBUFFER_ADDR + 4, (address >> 32) as u32);
        self.write_mmio(NV_FRAMEBUFFER_SIZE, (width << 16) | height);
        self.write_mmio(NV_FRAMEBUFFER_PITCH, width * 4);
        self.write_mmio(NV_FRAMEBUFFER_FORMAT, format);
        0
    }

    pub fn bind_framebuffer(&mut self, address: u64) {
        self.write_mmio(NV_FRAMEBUFFER_ADDR, address as u32);
        self.write_mmio(NV_FRAMEBUFFER_ADDR + 4, (address >> 32) as u32);
    }

    pub fn clear(&mut self, color: u32, depth: u32, stencil: u32) {
        self.write_mmio(NV_ROP_CLEAR_CNTL, color);
        self.write_mmio(NV_DEPTH_CLEAR, depth);
        self.write_mmio(NV_STENCIL_CLEAR, stencil);
        self.write_mmio(NV_ROP_CNTL, 0x00000001);
    }

    pub fn draw_arrays(&mut self, primitive: u32, first: u32, count: u32) {
        self.write_mmio(NV_VGT_PRIMITIVE_TYPE, primitive);
        self.write_mmio(NV_VGT_CNTL, 0x00000001);
        self.write_mmio(NV_VGT_SHADER_STAGES_EN, 0x00000001);
        self.ring_write(0, (primitive << 28) | (count & 0x0FFFFFFF));
        self.ring_write(0, first);
        self.ring_submit(0);
        self.wait_idle();
    }

    pub fn draw_elements(&mut self, primitive: u32, index_count: u32, index_format: u32, index_addr: u64) {
        self.write_mmio(NV_VGT_PRIMITIVE_TYPE, primitive);
        self.write_mmio(NV_INDEX_BUFFER_FORMAT, index_format);
        self.write_mmio(NV_INDEX_BUFFER_ADDR, index_addr as u32);
        self.write_mmio(NV_INDEX_BUFFER_ADDR + 4, (index_addr >> 32) as u32);
        self.write_mmio(NV_INDEX_BUFFER_SIZE, index_count * 4);
        self.write_mmio(NV_VGT_CNTL, 0x00000001);
        self.write_mmio(NV_VGT_SHADER_STAGES_EN, 0x00000001);
        self.ring_write(0, (primitive << 28) | (index_count & 0x0FFFFFFF));
        self.ring_write(0, index_format);
        self.ring_submit(0);
        self.wait_idle();
    }

    pub fn create_command_buffer(&mut self, size: u32) -> u32 {
        let id = self.command_buffer_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.write_mmio(NV_COMMAND_BUFFER_ALLOC, id);
        self.write_mmio(NV_COMMAND_BUFFER_SIZE, size);
        self.command_buffers[id as usize] = CommandBuffer {
            id,
            addr: REG_VRAM_BASE + 0x10000000 + id as u64 * size as u64,
            size,
            offset: AtomicU32::new(0),
            submitted: AtomicBool::new(false),
        };
        self.command_buffer_count += 1;
        id
    }

    pub fn command_buffer_begin(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        self.write_mmio(NV_COMMAND_BUFFER_BEGIN, id);
        self.command_buffers[id as usize].offset.store(0, Ordering::SeqCst);
        self.command_buffers[id as usize].submitted.store(false, Ordering::SeqCst);
    }

    pub fn command_buffer_write(&mut self, id: u32, dword: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        let cb = &mut self.command_buffers[id as usize];
        let offset = cb.offset.load(Ordering::SeqCst) as usize;
        if offset + 4 > cb.size as usize {
            return;
        }
        unsafe {
            (cb.addr as *mut u32).add(offset / 4).write_volatile(dword);
        }
        cb.offset.store(offset + 4, Ordering::SeqCst);
    }

    pub fn command_buffer_end(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        self.write_mmio(NV_COMMAND_BUFFER_END, id);
    }

    pub fn command_buffer_submit(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        let cb = &self.command_buffers[id as usize];
        self.write_mmio(NV_COMMAND_BUFFER_ADDR, cb.addr as u32);
        self.write_mmio(NV_COMMAND_BUFFER_ADDR + 4, (cb.addr >> 32) as u32);
        self.write_mmio(NV_COMMAND_BUFFER_SIZE, cb.size);
        self.write_mmio(NV_COMMAND_BUFFER_SUBMIT, id);
        self.command_buffers[id as usize].submitted.store(true, Ordering::SeqCst);
        self.wait_idle();
    }

    pub fn command_buffer_reset(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        self.write_mmio(NV_COMMAND_BUFFER_RESET, id);
        self.command_buffers[id as usize].offset.store(0, Ordering::SeqCst);
        self.command_buffers[id as usize].submitted.store(false, Ordering::SeqCst);
    }

    pub fn create_swap_chain(&mut self, image_count: u32, width: u32, height: u32, format: u32) -> u32 {
        let id = self.swap_chain_count;
        if id >= 8 || image_count > 8 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.write_mmio(NV_SWAP_CHAIN_CREATE, id);
        self.write_mmio(NV_SWAP_CHAIN_IMAGE_COUNT, image_count);
        self.write_mmio(NV_SWAP_CHAIN_IMAGE_FORMAT, format);
        for i in 0..image_count as usize {
            self.write_mmio_64(NV_SWAP_CHAIN_IMAGE_ADDR + i as u32 * 8, REG_VRAM_BASE + 0x20000000 + i as u64 * (width as u64 * height as u64 * 4));
        }
        self.swap_chains[id as usize] = SwapChain {
            id,
            image_count,
            current_image: AtomicU32::new(0),
            image_addrs: [0u64; 8],
            image_size: width * height * 4,
            image_format: format,
            acquired: AtomicBool::new(false),
            presented: AtomicBool::new(false),
        };
        self.swap_chain_count += 1;
        id
    }

    pub fn swap_chain_acquire(&mut self, id: u32) -> u32 {
        if id >= self.swap_chain_count {
            return 0xFFFFFFFF;
        }
        let sc = &mut self.swap_chains[id as usize];
        let idx = sc.current_image.load(Ordering::SeqCst);
        self.write_mmio(NV_SWAP_CHAIN_ACQUIRE, id);
        sc.current_image.store((idx + 1) % sc.image_count, Ordering::SeqCst);
        sc.acquired.store(true, Ordering::SeqCst);
        idx
    }

    pub fn swap_chain_present(&mut self, id: u32) {
        if id >= self.swap_chain_count {
            return;
        }
        let sc = &self.swap_chains[id as usize];
        self.write_mmio(NV_SWAP_CHAIN_IMAGE_ADDR, sc.image_addrs[sc.current_image.load(Ordering::SeqCst) as usize]);
        self.write_mmio(NV_SWAP_CHAIN_PRESENT, id);
        self.wait_idle();
    }

    pub fn compute_dispatch(&mut self, group_x: u32, group_y: u32, group_z: u32) {
        self.write_mmio(NV_COMPUTE_DISPATCH, group_x);
        self.write_mmio(NV_COMPUTE_DISPATCH + 4, group_y);
        self.write_mmio(NV_COMPUTE_DISPATCH + 8, group_z);
        self.write_mmio(NV_COMPUTE_CNTL, 0x00000001);
        self.wait_idle();
    }

    pub fn engine_clock(&self) -> u32 {
        self.engine_clock
    }

    pub fn memory_clock(&self) -> u32 {
        self.memory_clock
    }

    pub fn cuda_cores(&self) -> u32 {
        self.cuda_cores
    }

    pub fn sm_count(&self) -> u32 {
        self.sm_count
    }

    pub fn error_count(&self) -> u32 {
        self.error_count.load(Ordering::SeqCst)
    }

    pub fn is_initialized(&self) -> bool {
        self.initialized.load(Ordering::SeqCst)
    }
}

pub fn gpu_instance() -> Option<&'static mut GpuInstance> {
    unsafe { GPU_INSTANCE.as_mut() }
}

pub fn gpu_driver_init(device_id: u16) -> bool {
    unsafe {
        GPU_INSTANCE = Some(GpuInstance::new(device_id));
        GPU_INSTANCE.as_mut().unwrap().init()
    }
}

pub fn gpu_driver_is_ready() -> bool {
    unsafe {
        match &GPU_INSTANCE {
            Some(inst) => inst.initialized.load(Ordering::SeqCst),
            None => false,
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_driver_init() -> bool {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(inst) => inst.init(),
            None => {
                GPU_INSTANCE = Some(GpuInstance::new(GTX_1060_DEVICE_ID));
                GPU_INSTANCE.as_mut().unwrap().init()
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_driver_detected() -> bool {
    unsafe {
        match &GPU_INSTANCE {
            Some(inst) => inst.detect(),
            None => false,
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_enable_3d_acceleration() {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.enable_3d_acceleration();
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_draw_arrays(primitive: u32, first: u32, count: u32) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.draw_arrays(primitive, first, count);
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_draw_elements(primitive: u32, index_count: u32, index_format: u32, index_addr: u64) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.draw_elements(primitive, index_count, index_format, index_addr);
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_clear(color: u32, depth: u32, stencil: u32) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.clear(color, depth, stencil);
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_set_viewport(x: f32, y: f32, width: f32, height: f32, min_depth: f32, max_depth: f32) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.set_viewport(&Viewport { x, y, width, height, min_depth, max_depth });
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_get_status() -> u32 {
    unsafe {
        match &GPU_INSTANCE {
            Some(inst) => inst.error_count(),
            None => 0,
        }
    }
}

#[no_mangle]
pub extern "C" fn gtx_1000_is_ready() -> bool {
    gpu_driver_is_ready()
}
