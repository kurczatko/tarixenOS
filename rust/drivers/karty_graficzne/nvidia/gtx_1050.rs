/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const NVIDIA_VENDOR_ID: u16 = 0x10DE;
pub const NVIDIA_GTX_1050_DEVICE_ID: u16 = 0x1C86;
pub const CHIP_GP107: u32 = 0x1C86;
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
pub const NV_PGRAPH_TRAPPED_ERROR_INFO2: u32 = 0x400914;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO3: u32 = 0x400918;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO4: u32 = 0x40091C;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO5: u32 = 0x400920;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO6: u32 = 0x400924;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO7: u32 = 0x400928;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO8: u32 = 0x40092C;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO9: u32 = 0x400930;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO10: u32 = 0x400934;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO11: u32 = 0x400938;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO12: u32 = 0x40093C;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO13: u32 = 0x400940;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO14: u32 = 0x400944;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO15: u32 = 0x400948;

pub const NV_PGRAPH_INTR_0: u32 = 0x400E00;
pub const NV_PGRAPH_INTR_1: u32 = 0x400E04;
pub const NV_PGRAPH_INTR_2: u32 = 0x400E08;
pub const NV_PGRAPH_INTR_3: u32 = 0x400E0C;
pub const NV_PGRAPH_INTR_4: u32 = 0x400E10;
pub const NV_PGRAPH_INTR_5: u32 = 0x400E14;
pub const NV_PGRAPH_INTR_6: u32 = 0x400E18;
pub const NV_PGRAPH_INTR_7: u32 = 0x400E1C;
pub const NV_PGRAPH_INTR_EN_0: u32 = 0x400E20;
pub const NV_PGRAPH_INTR_EN_1: u32 = 0x400E24;
pub const NV_PGRAPH_INTR_EN_2: u32 = 0x400E28;
pub const NV_PGRAPH_INTR_EN_3: u32 = 0x400E2C;
pub const NV_PGRAPH_INTR_EN_4: u32 = 0x400E30;
pub const NV_PGRAPH_INTR_EN_5: u32 = 0x400E34;
pub const NV_PGRAPH_INTR_EN_6: u32 = 0x400E38;
pub const NV_PGRAPH_INTR_EN_7: u32 = 0x400E3C;
pub const NV_PGRAPH_INTR_MASK_0: u32 = 0x400E40;
pub const NV_PGRAPH_INTR_MASK_1: u32 = 0x400E44;
pub const NV_PGRAPH_INTR_MASK_2: u32 = 0x400E48;
pub const NV_PGRAPH_INTR_MASK_3: u32 = 0x400E4C;
pub const NV_PGRAPH_INTR_MASK_4: u32 = 0x400E50;
pub const NV_PGRAPH_INTR_MASK_5: u32 = 0x400E54;
pub const NV_PGRAPH_INTR_MASK_6: u32 = 0x400E58;
pub const NV_PGRAPH_INTR_MASK_7: u32 = 0x400E5C;
pub const NV_PGRAPH_INTR_STATUS_0: u32 = 0x400E60;
pub const NV_PGRAPH_INTR_STATUS_1: u32 = 0x400E64;
pub const NV_PGRAPH_INTR_STATUS_2: u32 = 0x400E68;
pub const NV_PGRAPH_INTR_STATUS_3: u32 = 0x400E6C;
pub const NV_PGRAPH_INTR_STATUS_4: u32 = 0x400E70;
pub const NV_PGRAPH_INTR_STATUS_5: u32 = 0x400E74;
pub const NV_PGRAPH_INTR_STATUS_6: u32 = 0x400E78;
pub const NV_PGRAPH_INTR_STATUS_7: u32 = 0x400E7C;
pub const NV_PGRAPH_INTR_SET_0: u32 = 0x400E80;
pub const NV_PGRAPH_INTR_SET_1: u32 = 0x400E84;
pub const NV_PGRAPH_INTR_SET_2: u32 = 0x400E88;
pub const NV_PGRAPH_INTR_SET_3: u32 = 0x400E8C;
pub const NV_PGRAPH_INTR_SET_4: u32 = 0x400E90;
pub const NV_PGRAPH_INTR_SET_5: u32 = 0x400E94;
pub const NV_PGRAPH_INTR_SET_6: u32 = 0x400E98;
pub const NV_PGRAPH_INTR_SET_7: u32 = 0x400E9C;

pub const NV_PGRAPH_CTXSW_IMAGE_SIZE: u32 = 0x400880;
pub const NV_PGRAPH_CTXSW_IMAGE_OFFSET: u32 = 0x400884;
pub const NV_PGRAPH_CTXSW_IMAGE_ADDR_HI: u32 = 0x400888;
pub const NV_PGRAPH_CTXSW_IMAGE_ADDR_LO: u32 = 0x40088C;
pub const NV_PGRAPH_CTXSW_GP_METHOD: u32 = 0x400890;
pub const NV_PGRAPH_CTXSW_GP_OFFSET: u32 = 0x400894;
pub const NV_PGRAPH_CTXSW_GP_DATA: u32 = 0x400898;
pub const NV_PGRAPH_CTXSW_GP_SIZE: u32 = 0x40089C;
pub const NV_PGRAPH_CTXSW_GP_FLAGS: u32 = 0x4008A0;
pub const NV_PGRAPH_CTXSW_GP_TIMESTAMP: u32 = 0x4008A4;
pub const NV_PGRAPH_CTXSW_GP_SUBCHANNEL: u32 = 0x4008A8;
pub const NV_PGRAPH_CTXSW_GP_CHANNEL: u32 = 0x4008AC;
pub const NV_PGRAPH_CTXSW_GP_CLASS: u32 = 0x4008B0;
pub const NV_PGRAPH_CTXSW_GP_NOTIFY: u32 = 0x4008B4;
pub const NV_PGRAPH_CTXSW_GP_ERROR: u32 = 0x4008B8;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO: u32 = 0x4008BC;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO2: u32 = 0x4008C0;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO3: u32 = 0x4008C4;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO4: u32 = 0x4008C8;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO5: u32 = 0x4008CC;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO6: u32 = 0x4008D0;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO7: u32 = 0x4008D4;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO8: u32 = 0x4008D8;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO9: u32 = 0x4008DC;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO10: u32 = 0x4008E0;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO11: u32 = 0x4008E4;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO12: u32 = 0x4008E8;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO13: u32 = 0x4008EC;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO14: u32 = 0x4008F0;
pub const NV_PGRAPH_CTXSW_GP_ERROR_INFO15: u32 = 0x4008F4;

pub const NV_GPCCS_BASE: u32 = 0x100000;
pub const NV_GPCCS_GPCCS_CNTL: u32 = 0x100000;
pub const NV_GPCCS_GPCCS_STATUS: u32 = 0x100004;
pub const NV_GPCCS_GPCCS_INTR: u32 = 0x100008;
pub const NV_GPCCS_GPCCS_INTR_MASK: u32 = 0x10000C;
pub const NV_GPCCS_GPCCS_PWR: u32 = 0x100010;
pub const NV_GPCCS_GPCCS_CLK: u32 = 0x100014;
pub const NV_GPCCS_GPCCS_THERM: u32 = 0x100018;
pub const NV_GPCCS_GPCCS_FAN: u32 = 0x10001C;
pub const NV_GPCCS_GPCCS_VOLT: u32 = 0x100020;
pub const NV_GPCCS_GPCCS_PWR_STATE: u32 = 0x100024;

pub const NV_GPC_BASE: u32 = 0x108000;
pub const NV_GPC_CNTL: u32 = 0x108000;
pub const NV_GPC_TPC_BASE: u32 = 0x108400;
pub const NV_TPC_CNTL: u32 = 0x108400;
pub const NV_TPC_SM_BASE: u32 = 0x108800;
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

pub const NV_COLOR_BUFFER_BASE: u32 = 0x113000;
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

pub const NV_DEPTH_STENCIL_BUFFER_BASE: u32 = 0x116000;
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
pub const NV_PCIE_LC_CNTL5: u32 = 0x9014;
pub const NV_PCIE_LC_CNTL6: u32 = 0x9018;
pub const NV_PCIE_LC_CNTL7: u32 = 0x901C;
pub const NV_PCIE_LC_CNTL8: u32 = 0x9020;
pub const NV_PCIE_LC_CNTL9: u32 = 0x9024;
pub const NV_PCIE_LC_CNTL10: u32 = 0x9028;
pub const NV_PCIE_LC_CNTL11: u32 = 0x902C;

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
pub const NV_SMU_UVD_PWR_CNTL: u32 = 0xB02C;
pub const NV_SMU_VCE_PWR_CNTL: u32 = 0xB038;

pub const NV_UVD_BASE: u32 = 0xC100;
pub const NV_VCE_BASE: u32 = 0xC200;

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
    pub uniform_offsets: [u32; 64],
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
}

pub const GTX1050_VRAM_SIZE: u64 = 2 * 1024 * 1024 * 1024;
pub const GTX1050_GART_SIZE: u64 = 1 * 1024 * 1024 * 1024;
pub const GTX1050_ENGINE_CLOCK: u32 = 1354000;
pub const GTX1050_MEMORY_CLOCK: u32 = 7008000;
pub const GTX1050_CUDA_CORES: u32 = 640;
pub const GTX1050_SM_COUNT: u32 = 5;
pub const GTX1050_ROP_COUNT: u32 = 32;
pub const GTX1050_TEXTURE_UNITS: u32 = 40;
pub const GTX1050_MEMORY_BUS_WIDTH: u32 = 128;
pub const GTX1050_VERTEX_CACHE_SIZE: u32 = 32 * 1024;
pub const GTX1050_PRIMITIVE_BUFFER_SIZE: u32 = 16 * 1024;

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new() -> Self {
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size: GTX1050_VRAM_SIZE,
            vram_used: AtomicU64::new(0),
            gart_size: GTX1050_GART_SIZE,
            engine_clock: GTX1050_ENGINE_CLOCK,
            memory_clock: GTX1050_MEMORY_CLOCK,
            cuda_cores: GTX1050_CUDA_CORES,
            sm_count: GTX1050_SM_COUNT,
            rop_count: GTX1050_ROP_COUNT,
            texture_units: GTX1050_TEXTURE_UNITS,
            memory_bus_width: GTX1050_MEMORY_BUS_WIDTH,
            vertex_cache_size: GTX1050_VERTEX_CACHE_SIZE,
            primitive_buffer_size: GTX1050_PRIMITIVE_BUFFER_SIZE,
            command_buffer_size: 4 * 1024 * 1024,
            initialized: AtomicBool::new(false),
            error_count: AtomicU32::new(0),
            ring_gfx: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x100000,
                size: 4 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 0,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(0) },
            },
            ring_compute: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x500000,
                size: 2 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 1,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(1) },
            },
            ring_dma: RingBuffer {
                base_addr: REG_VRAM_BASE + 0x700000,
                size: 2 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 2,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(2) },
            },
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
            vendor == NVIDIA_VENDOR_ID as u32 && device == NVIDIA_GTX_1050_DEVICE_ID as u32 && rev != 0xFFFFFFFF
        }
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
        self.write_mmio(NV_SDMA0_RB_BASE, (REG_VRAM_BASE + 0x500000) as u32);
        self.write_mmio(NV_SDMA0_RB_BASE + 4, ((REG_VRAM_BASE + 0x500000) >> 32) as u32);
        self.write_mmio(NV_SDMA0_RB_CNTL, 0x00000001);
        self.write_mmio(NV_SDMA0_IB_BASE, (REG_VRAM_BASE + 0x600000) as u32);
        self.write_mmio(NV_SDMA0_IB_BASE + 4, ((REG_VRAM_BASE + 0x600000) >> 32) as u32);
        self.write_mmio(NV_SDMA0_IB_CNTL, 0x00000001);
        self.write_mmio(NV_SDMA1_RB_BASE, (REG_VRAM_BASE + 0x700000) as u32);
        self.write_mmio(NV_SDMA1_RB_BASE + 4, ((REG_VRAM_BASE + 0x700000) >> 32) as u32);
        self.write_mmio(NV_SDMA1_RB_CNTL, 0x00000001);
        self.write_mmio(NV_SDMA1_IB_BASE, (REG_VRAM_BASE + 0x800000) as u32);
        self.write_mmio(NV_SDMA1_IB_BASE + 4, ((REG_VRAM_BASE + 0x800000) >> 32) as u32);
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
        self.write_mmio(NV_PMC_CNTRL6, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL7, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL8, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL9, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL10, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL11, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL12, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL13, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL14, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL15, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL16, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL17, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL18, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL19, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL20, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL21, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL22, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL23, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL24, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL25, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL26, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL27, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL28, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL29, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL30, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL31, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL32, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL33, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL34, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL35, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL36, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL37, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL38, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL39, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL40, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL41, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL42, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL43, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL44, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL45, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL46, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL47, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL48, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL49, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL50, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL51, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL52, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL53, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL54, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL55, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL56, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL57, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL58, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL59, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL60, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL61, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL62, 0x00000000);
        self.write_mmio(NV_PMC_CNTRL63, 0x00000000);
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
        self.write_mmio(NV_PGRAPH_DEBUG_8, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_9, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_10, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_11, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_12, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_13, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_14, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_15, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_16, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_17, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_18, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_19, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_20, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_21, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_22, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_23, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_24, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_25, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_26, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_27, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_28, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_29, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_30, 0x00000000);
        self.write_mmio(NV_PGRAPH_DEBUG_31, 0x00000000);
        self.write_mmio(NV_PGRAPH_INTR_EN_0, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_1, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_2, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_3, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_4, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_5, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_6, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_EN_7, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_0, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_1, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_2, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_3, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_4, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_5, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_6, 0xFFFFFFFF);
        self.write_mmio(NV_PGRAPH_INTR_MASK_7, 0xFFFFFFFF);
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
        self.write_mmio(NV_RENDER_TARGET_COUNT, 0x00000004);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(NV_DEPTH_STENCIL_ADDR, REG_VRAM_BASE + 0xE00000);
        self.write_mmio(NV_DEPTH_STENCIL_SIZE, self.vram_size as u32);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(NV_MULTISAMPLE_CNTL, 0x00000001);
        self.write_mmio(NV_MULTISAMPLE_COUNT, 0x00000004);
        self.write_mmio(NV_MULTISAMPLE_SAMPLE_LOCS, 0x00000004);
        self.write_mmio(NV_MULTISAMPLE_COVERAGE, 0x00000001);
        self.write_mmio(NV_MULTISAMPLE_RESOLVE, 0x00000001);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(NV_ANTIALIAS_CNTL, 0x00000001);
        self.write_mmio(NV_ANTIALIAS_MODE, 0x00000001);
        self.write_mmio(NV_ANTIALIAS_SAMPLES, 0x00000004);
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
        self.write_mmio(NV_UNIFORM_BUFFER_COUNT, 0x00000004);
    }

    fn setup_shader_storage_buffers(&mut self) {
        self.write_mmio(NV_SHADER_STORAGE_ADDR, REG_VRAM_BASE + 0x900000);
        self.write_mmio(NV_SHADER_STORAGE_SIZE, self.vram_size as u32);
        self.write_mmio(NV_SHADER_STORAGE_COUNT, 0x00000004);
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

    fn enable_3d_acceleration(&mut self) {
        self.write_mmio(0x0900, 0x00000001);
        self.write_mmio(0x0904, 0xFFFFFFFF);
        self.write_mmio(0x0908, 0x00000001);
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
}

pub fn gpu_instance() -> &'static mut GpuInstance {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(GpuInstance::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn gtx_1050_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn gtx_1050_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn gtx_1050_driver_init() -> bool {
    gtx_1050_init()
}

#[no_mangle]
pub extern "C" fn gtx_1050_driver_detected() -> bool {
    gtx_1050_detected()
}

#[no_mangle]
pub extern "C" fn gtx_1050_draw_triangle_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_triangle_3d(x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
}

#[no_mangle]
pub extern "C" fn gtx_1050_clear_screen_3d(color: u32) {
    let gpu = gpu_instance();
    gpu.clear_screen_3d(color);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_viewport(x: u32, y: u32, width: u32, height: u32) {
    let gpu = gpu_instance();
    gpu.set_viewport(x, y, width, height);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_projection_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_projection_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_world_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_world_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_view_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_view_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1050_enable_depth_test(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_depth_test(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1050_enable_blending(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_blending(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_blend_func(src: u32, dst: u32) {
    let gpu = gpu_instance();
    gpu.set_blend_func(src, dst);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_depth_func(func: u32) {
    let gpu = gpu_instance();
    gpu.set_depth_func(func);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_cull_mode(mode: u32) {
    let gpu = gpu_instance();
    gpu.set_cull_mode(mode);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_wireframe(enabled: bool) {
    let gpu = gpu_instance();
    gpu.set_wireframe(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1050_set_multisample_count(count: u32) {
    let gpu = gpu_instance();
    gpu.set_multisample_count(count);
}

#[no_mangle]
pub extern "C" fn gtx_1050_enable_multisampling(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_multisampling(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1050_enable_antialiasing(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_antialiasing(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1050_finish() {
    let gpu = gpu_instance();
    gpu.finish();
}

#[no_mangle]
pub extern "C" fn gtx_1050_flush() {
    let gpu = gpu_instance();
    gpu.flush();
}

#[no_mangle]
pub extern "C" fn gtx_1050_get_status() -> u32 {
    let gpu = gpu_instance();
    gpu.get_status()
}

#[no_mangle]
pub extern "C" fn gtx_1050_engine_clock() -> u32 {
    let gpu = gpu_instance();
    gpu.engine_clock()
}

#[no_mangle]
pub extern "C" fn gtx_1050_memory_clock() -> u32 {
    let gpu = gpu_instance();
    gpu.memory_clock()
}

#[no_mangle]
pub extern "C" fn gtx_1050_vram_size() -> u32 {
    let gpu = gpu_instance();
    gpu.vram_size()
}

#[no_mangle]
pub extern "C" fn gtx_1050_cuda_cores() -> u32 {
    let gpu = gpu_instance();
    gpu.cuda_cores()
}

#[no_mangle]
pub extern "C" fn gtx_1050_sm_count() -> u32 {
    let gpu = gpu_instance();
    gpu.sm_count()
}

#[no_mangle]
pub extern "C" fn gtx_1050_rop_count() -> u32 {
    let gpu = gpu_instance();
    gpu.rop_count()
}

#[no_mangle]
pub extern "C" fn gtx_1050_texture_units() -> u32 {
    let gpu = gpu_instance();
    gpu.texture_units()
}

#[no_mangle]
pub extern "C" fn gtx_1050_memory_bus_width() -> u32 {
    let gpu = gpu_instance();
    gpu.memory_bus_width()
}
    }