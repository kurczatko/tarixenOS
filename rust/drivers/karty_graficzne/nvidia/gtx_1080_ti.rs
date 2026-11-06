#![no_std]

use core::sync::atomic::{AtomicU32, Ordering};

pub const NVIDIA_VENDOR_ID: u16 = 0x10DE;
pub const NVIDIA_GTX_1080_TI_DEVICE_ID: u16 = 0x1B06;

pub const CHIP_GP102: u32 = 0x1B06;
pub const PASCAL_ARCH_VERSION: u32 = 1;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;

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

pub const NV_PGRAPH_TRAPPED_ADDR: u32 = 0x400880;
pub const NV_PGRAPH_TRAPPED_DATA: u32 = 0x400884;
pub const NV_PGRAPH_TRAPPED_CTX: u32 = 0x400888;
pub const NV_PGRAPH_TRAPPED_MTHD: u32 = 0x40088C;
pub const NV_PGRAPH_TRAPPED_CLASS: u32 = 0x400890;
pub const NV_PGRAPH_TRAPPED_SUBCH: u32 = 0x400894;
pub const NV_PGRAPH_TRAPPED_TIMESTAMP: u32 = 0x400898;
pub const NV_PGRAPH_TRAPPED_NOTIFY: u32 = 0x40089C;
pub const NV_PGRAPH_TRAPPED_OFFSET: u32 = 0x4008A0;
pub const NV_PGRAPH_TRAPPED_SIZE: u32 = 0x4008A4;
pub const NV_PGRAPH_TRAPPED_FLAGS: u32 = 0x4008A8;
pub const NV_PGRAPH_TRAPPED_ERROR: u32 = 0x4008AC;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO: u32 = 0x4008B0;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO2: u32 = 0x4008B4;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO3: u32 = 0x4008B8;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO4: u32 = 0x4008BC;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO5: u32 = 0x4008C0;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO6: u32 = 0x4008C4;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO7: u32 = 0x4008C8;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO8: u32 = 0x4008CC;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO9: u32 = 0x4008D0;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO10: u32 = 0x4008D4;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO11: u32 = 0x4008D8;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO12: u32 = 0x4008DC;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO13: u32 = 0x4008E0;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO14: u32 = 0x4008E4;
pub const NV_PGRAPH_TRAPPED_ERROR_INFO15: u32 = 0x4008E8;

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

static IS_INITIALIZED: AtomicU32 = AtomicU32::new(0);

#[repr(C)]
pub struct Matrix4x4 {
    pub m: [f32; 16],
}

#[repr(C)]
pub struct Vector3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}

#[repr(C)]
pub struct Vector4 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub w: f32,
}

#[repr(C)]
pub struct Vertex {
    pub position: Vector4,
    pub color: u32,
    pub tex_coord: Vector2,
    pub normal: Vector3,
}

#[repr(C)]
pub struct Vector2 {
    pub u: f32,
    pub v: f32,
}

pub struct Gtx1080Ti {
    mmio_base: *mut u32,
    vram_base: *mut u32,
    vram_size: u32,
    engine_clock: u32,
    memory_clock: u32,
    cuda_cores: u32,
    sm_count: u32,
    rop_count: u32,
    texture_units: u32,
    memory_bus_width: u32,
    vertex_cache_size: u32,
    primitive_buffer_size: u32,
}

impl Gtx1080Ti {
    pub fn new() -> Self {
        Gtx1080Ti {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            vram_size: 1024 * 1024 * 1024,
            engine_clock: 1480000,
            memory_clock: 5505000,
            cuda_cores: 3584,
            sm_count: 28,
            rop_count: 88,
            texture_units: 224,
            memory_bus_width: 352,
            vertex_cache_size: 64 * 1024,
            primitive_buffer_size: 32 * 1024,
        }
    }

    pub fn detect(&self) -> bool {
        unsafe {
            let vendor = self.read_mmio(0x00);
            let device = self.read_mmio(0x04);
            vendor == NVIDIA_VENDOR_ID as u32 && device == NVIDIA_GTX_1080_TI_DEVICE_ID as u32
        }
    }

    pub fn init(&mut self) -> bool {
        if IS_INITIALIZED.load(Ordering::SeqCst) == 1 {
            return true;
        }
        if !self.detect() {
            return false;
        }
        self.soft_reset();
        self.wait_idle_inner();
        self.setup_pmc();
        self.setup_pgraph();
        self.setup_fecs();
        self.setup_gpmu();
        self.setup_hub();
        self.setup_gr();
        self.setup_gpc();
        self.setup_tpc();
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
        IS_INITIALIZED.store(1, Ordering::SeqCst);
        true
    }

    fn soft_reset(&mut self) {
        self.write_mmio(NV_PMC_CNTRL, 0x00000001);
        for _ in 0..50000 {
            core::hint::spin_loop();
        }
        self.write_mmio(NV_PMC_CNTRL, 0x00000000);
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(NV_PGRAPH_INTR_STATUS_0);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
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

    fn setup_gpmu(&mut self) {
        self.write_mmio(0x100C00, 0x00000001);
        self.write_mmio(0x100C04, 0x00000001);
        self.write_mmio(0x100C08, 0x00000001);
        self.write_mmio(0x100C0C, 0x00000001);
        self.write_mmio(0x100C10, 0x00000001);
        self.write_mmio(0x100C14, 0x00000001);
        self.write_mmio(0x100C18, 0x00000001);
        self.write_mmio(0x100C1C, 0x00000001);
    }

    fn setup_hub(&mut self) {
        self.write_mmio(0x101000, 0x00000001);
        self.write_mmio(0x101004, 0x00000001);
        self.write_mmio(0x101008, 0x00000001);
        self.write_mmio(0x10100C, 0x00000001);
        self.write_mmio(0x101010, 0x00000001);
        self.write_mmio(0x101014, 0x00000001);
        self.write_mmio(0x101018, 0x00000001);
        self.write_mmio(0x10101C, 0x00000001);
    }

    fn setup_gr(&mut self) {
        self.write_mmio(0x102000, 0x00000001);
        self.write_mmio(0x102004, 0x00000001);
        self.write_mmio(0x102008, 0x00000001);
        self.write_mmio(0x10200C, 0x00000001);
        self.write_mmio(0x102010, 0x00000001);
        self.write_mmio(0x102014, 0x00000001);
        self.write_mmio(0x102018, 0x00000001);
        self.write_mmio(0x10201C, 0x00000001);
    }

    fn setup_gpc(&mut self) {
        self.write_mmio(0x108000, 0x00000001);
        self.write_mmio(0x108004, 0x00000001);
        self.write_mmio(0x108008, 0x00000001);
        self.write_mmio(0x10800C, 0x00000001);
        self.write_mmio(0x108010, 0x00000001);
        self.write_mmio(0x108014, 0x00000001);
        self.write_mmio(0x108018, 0x00000001);
        self.write_mmio(0x10801C, 0x00000001);
    }

    fn setup_tpc(&mut self) {
        self.write_mmio(0x108400, 0x00000001);
        self.write_mmio(0x108404, 0x00000001);
        self.write_mmio(0x108408, 0x00000001);
        self.write_mmio(0x10840C, 0x00000001);
        self.write_mmio(0x108410, 0x00000001);
        self.write_mmio(0x108414, 0x00000001);
        self.write_mmio(0x108418, 0x00000001);
        self.write_mmio(0x10841C, 0x00000001);
    }

    fn setup_rop(&mut self) {
        self.write_mmio(0x10A000, 0x00000001);
        self.write_mmio(0x10A004, 0x00000001);
        self.write_mmio(0x10A008, 0x00000001);
        self.write_mmio(0x10A00C, 0x00000001);
        self.write_mmio(0x10A010, 0x00000001);
        self.write_mmio(0x10A014, 0x00000001);
        self.write_mmio(0x10A018, 0x00000001);
        self.write_mmio(0x10A01C, 0x00000001);
    }

    fn setup_texture(&mut self) {
        self.write_mmio(0x10C000, 0x00000001);
        self.write_mmio(0x10C004, 0x00000001);
        self.write_mmio(0x10C008, 0x00000001);
        self.write_mmio(0x10C00C, 0x00000001);
        self.write_mmio(0x10C010, 0x00000001);
        self.write_mmio(0x10C014, 0x00000001);
        self.write_mmio(0x10C018, 0x00000001);
        self.write_mmio(0x10C01C, 0x00000001);
    }

    fn setup_shader(&mut self) {
        self.write_mmio(0x10D000, 0x00000001);
        self.write_mmio(0x10D004, 0x00000001);
        self.write_mmio(0x10D008, 0x00000001);
        self.write_mmio(0x10D00C, 0x00000001);
        self.write_mmio(0x10D010, 0x00000001);
        self.write_mmio(0x10D014, 0x00000001);
        self.write_mmio(0x10D018, 0x00000001);
        self.write_mmio(0x10D01C, 0x00000001);
    }

    fn setup_compute(&mut self) {
        self.write_mmio(0x10E000, 0x00000001);
        self.write_mmio(0x10E004, 0x00000001);
        self.write_mmio(0x10E008, 0x00000001);
        self.write_mmio(0x10E00C, 0x00000001);
        self.write_mmio(0x10E010, 0x00000001);
        self.write_mmio(0x10E014, 0x00000001);
        self.write_mmio(0x10E018, 0x00000001);
        self.write_mmio(0x10E01C, 0x00000001);
    }

    fn setup_rasterizer(&mut self) {
        self.write_mmio(0x10F000, 0x00000001);
        self.write_mmio(0x10F004, 0x00000001);
        self.write_mmio(0x10F008, 0x00000001);
        self.write_mmio(0x10F00C, 0x00000001);
        self.write_mmio(0x10F010, 0x00000001);
        self.write_mmio(0x10F014, 0x00000001);
        self.write_mmio(0x10F018, 0x00000001);
        self.write_mmio(0x10F01C, 0x00000001);
    }

    fn setup_depth_buffer(&mut self) {
        self.write_mmio(0x110000, 0x00000001);
        self.write_mmio(0x110004, 0x00000001);
        self.write_mmio(0x110008, 0x00000001);
        self.write_mmio(0x11000C, 0x00000001);
        self.write_mmio(0x110010, 0x00000001);
        self.write_mmio(0x110014, 0x00000001);
        self.write_mmio(0x110018, 0x00000001);
        self.write_mmio(0x11001C, 0x00000001);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(0x111000, 0x00000001);
        self.write_mmio(0x111004, 0x00000001);
        self.write_mmio(0x111008, 0x00000001);
        self.write_mmio(0x11100C, 0x00000001);
        self.write_mmio(0x111010, 0x00000001);
        self.write_mmio(0x111014, 0x00000001);
        self.write_mmio(0x111018, 0x00000001);
        self.write_mmio(0x11101C, 0x00000001);
    }

    fn setup_blend(&mut self) {
        self.write_mmio(0x112000, 0x00000001);
        self.write_mmio(0x112004, 0x00000001);
        self.write_mmio(0x112008, 0x00000001);
        self.write_mmio(0x11200C, 0x00000001);
        self.write_mmio(0x112010, 0x00000001);
        self.write_mmio(0x112014, 0x00000001);
        self.write_mmio(0x112018, 0x00000001);
        self.write_mmio(0x11201C, 0x00000001);
    }

    fn setup_color_buffer(&mut self) {
        self.write_mmio(0x113000, REG_VRAM_BASE + 0x200000);
        self.write_mmio(0x113004, self.vram_size);
        self.write_mmio(0x113008, 1024);
        self.write_mmio(0x11300C, 0x00000001);
    }

    fn setup_framebuffers(&mut self) {
        self.write_mmio(0x114000, REG_VRAM_BASE + 0x300000);
        self.write_mmio(0x114004, self.vram_size);
    }

    fn setup_render_targets(&mut self) {
        self.write_mmio(0x115000, REG_VRAM_BASE + 0x400000);
        self.write_mmio(0x115004, self.vram_size);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(0x116000, REG_VRAM_BASE + 0x500000);
        self.write_mmio(0x116004, self.vram_size);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(0x117000, 0x00000004);
        self.write_mmio(0x117004, 0x00000004);
        self.write_mmio(0x117008, 0x00000004);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(0x118000, 0x00000001);
        self.write_mmio(0x118004, 0x00000001);
        self.write_mmio(0x118008, 0x00000001);
    }

    fn setup_viewport(&mut self) {
        self.write_mmio(0x119000, 0x00000000);
        self.write_mmio(0x119004, 0x00000000);
        self.write_mmio(0x119008, 0x00000000);
        self.write_mmio(0x11900C, 0x00000000);
    }

    fn setup_scissor(&mut self) {
        self.write_mmio(0x11A000, 0x00000000);
        self.write_mmio(0x11A004, 0x00000000);
        self.write_mmio(0x11A008, 0x00000000);
        self.write_mmio(0x11A00C, 0x00000000);
    }

    fn setup_vertex_cache(&mut self) {
        self.write_mmio(0x11B000, self.vertex_cache_size);
    }

    fn setup_index_buffer(&mut self) {
        self.write_mmio(0x11C000, REG_VRAM_BASE + 0x600000);
        self.write_mmio(0x11C004, self.primitive_buffer_size);
    }

    fn setup_primitive_buffer(&mut self) {
        self.write_mmio(0x11C008, REG_VRAM_BASE + 0x700000);
        self.write_mmio(0x11C00C, self.primitive_buffer_size);
    }

    fn setup_uniform_buffers(&mut self) {
        self.write_mmio(0x11D000, REG_VRAM_BASE + 0x800000);
        self.write_mmio(0x11D004, self.vram_size);
    }

    fn setup_shader_storage_buffers(&mut self) {
        self.write_mmio(0x11E000, REG_VRAM_BASE + 0x900000);
        self.write_mmio(0x11E004, self.vram_size);
    }

    fn setup_atomic_counters(&mut self) {
        self.write_mmio(0x11F000, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(0x11F004, 0x00001000);
    }

    fn setup_query_objects(&mut self) {
        self.write_mmio(0x120000, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(0x120004, 0x00001000);
    }

    fn enable_3d_acceleration(&mut self) {
        self.write_mmio(0x0900, 0x00000001);
        self.write_mmio(0x0904, 0xFFFFFFFF);
        self.write_mmio(0x0908, 0x00000001);
    }

    fn read_mmio(&self, offset: u32) -> u32 {
        unsafe { self.mmio_base.add(offset as usize / 4).read_volatile() }
    }

    fn write_mmio(&mut self, offset: u32, value: u32) {
        unsafe { self.mmio_base.add(offset as usize / 4).write_volatile(value); }
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

    pub fn wait_idle(&self) {
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
        self.vram_size
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
}

static mut GPU_INSTANCE: Option<Gtx1080Ti> = None;

pub fn gpu_instance() -> &'static mut Gtx1080Ti {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(Gtx1080Ti::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn gtx_1080_ti_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn gtx_1080_ti_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_driver_init() -> bool {
    gtx_1080_ti_init()
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_driver_detected() -> bool {
    gtx_1080_ti_detected()
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_draw_triangle_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_triangle_3d(x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_clear_screen_3d(color: u32) {
    let gpu = gpu_instance();
    gpu.clear_screen_3d(color);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_viewport(x: u32, y: u32, width: u32, height: u32) {
    let gpu = gpu_instance();
    gpu.set_viewport(x, y, width, height);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_projection_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_projection_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_world_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_world_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_view_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_view_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_enable_depth_test(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_depth_test(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_enable_blending(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_blending(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_blend_func(src: u32, dst: u32) {
    let gpu = gpu_instance();
    gpu.set_blend_func(src, dst);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_depth_func(func: u32) {
    let gpu = gpu_instance();
    gpu.set_depth_func(func);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_cull_mode(mode: u32) {
    let gpu = gpu_instance();
    gpu.set_cull_mode(mode);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_wireframe(enabled: bool) {
    let gpu = gpu_instance();
    gpu.set_wireframe(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_set_multisample_count(count: u32) {
    let gpu = gpu_instance();
    gpu.set_multisample_count(count);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_enable_multisampling(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_multisampling(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_enable_antialiasing(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_antialiasing(enabled);
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_finish() {
    let gpu = gpu_instance();
    gpu.finish();
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_flush() {
    let gpu = gpu_instance();
    gpu.flush();
}

#[no_mangle]
pub extern "C" fn gtx_1080_ti_get_status() -> u32 {
    let gpu = gpu_instance();
    gpu.get_status()
}
