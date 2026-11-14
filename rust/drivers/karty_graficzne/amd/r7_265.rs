/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

use core::sync::atomic::{AtomicU32, Ordering};

pub const AMD_RADEON_R7_265_VENDOR_ID: u16 = 0x1002;
pub const AMD_RADEON_R7_265_DEVICE_ID: u16 = 0x6610;

pub const CHIP_PITCAIRN: u32 = 0x6610;
pub const GCN_ARCH_VERSION: u32 = 1;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;

pub const SRBM_STATUS: u32 = 0x00E0;
pub const SRBM_GFX_CNTL: u32 = 0x00E4;
pub const GRBM_CNTL: u32 = 0x0080;
pub const GRBM_READ_CNTL: u32 = 0x0084;
pub const GRBM_SOFT_RESET: u32 = 0x0088;
pub const GRBM_GUI_MASK: u32 = 0x008C;
pub const GRBM_RESET: u32 = 0x0088;

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
pub const VGT_CONFIG: u32 = 0x8B10;

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

pub const PA_CL_CLIP_CNTL: u32 = 0x8B2C;
pub const PA_CL_VTE_CNTL: u32 = 0x8B30;
pub const PA_CL_VPORT_XSCALE_0: u32 = 0x8B34;
pub const PA_CL_VPORT_XOFFSET_0: u32 = 0x8B38;
pub const PA_CL_VPORT_YSCALE_0: u32 = 0x8B3C;
pub const PA_CL_VPORT_YOFFSET_0: u32 = 0x8B40;
pub const PA_CL_VPORT_ZSCALE_0: u32 = 0x8B44;
pub const PA_CL_VPORT_ZOFFSET_0: u32 = 0x8B48;

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

pub const TA_CNTL_AUX: u32 = 0x9000;

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

pub struct R7_265 {
    mmio_base: *mut u32,
    vram_base: *mut u32,
    vram_size: u32,
    engine_clock: u32,
    memory_clock: u32,
    compute_units: u32,
    shader_engines: u32,
    rasterizers: u32,
    backends: u32,
    texture_units: u32,
    vertex_cache_size: u32,
    primitive_buffer_size: u32,
}

impl R7_265 {
    pub fn new() -> Self {
        R7_265 {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            vram_size: 2048 * 1024 * 1024,
            engine_clock: 925000,
            memory_clock: 1375000,
            compute_units: 20,
            shader_engines: 2,
            rasterizers: 2,
            backends: 32,
            texture_units: 80,
            vertex_cache_size: 32 * 1024,
            primitive_buffer_size: 16 * 1024,
        }
    }

    pub fn detect(&self) -> bool {
        unsafe {
            let vendor = self.read_mmio(0x00);
            let device = self.read_mmio(0x04);
            vendor == AMD_RADEON_R7_265_VENDOR_ID as u32 && device == AMD_RADEON_R7_265_DEVICE_ID as u32
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
        self.wait_idle();
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
        self.enable_3d_acceleration();
        IS_INITIALIZED.store(1, Ordering::SeqCst);
        true
    }

    fn soft_reset(&mut self) {
        self.write_mmio(SRBM_GFX_CNTL, 0x00000001);
        self.write_mmio(GRBM_SOFT_RESET, 0xFFFFFFFF);
        for _ in 0..50000 {
            core::hint::spin_loop();
        }
        self.write_mmio(GRBM_SOFT_RESET, 0x00000000);
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(SRBM_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    fn setup_srbm(&mut self) {
        self.write_mmio(SRBM_GFX_CNTL, 0x00000000);
    }

    fn setup_grbm(&mut self) {
        self.write_mmio(GRBM_CNTL, 0x00000000);
        self.write_mmio(GRBM_READ_CNTL, 0x00000000);
        self.write_mmio(GRBM_GUI_MASK, 0xFFFFFFFF);
    }

    fn setup_command_processor(&mut self) {
        self.write_mmio(CP_ME_CNTL, 0x00000001);
        self.write_mmio(CP_ME_CNTL2, 0x00000001);
        self.write_mmio(CP_INT_CNTL, 0x00000001);
        self.write_mmio(CP_INT_MASK, 0xFFFFFFFF);
        self.write_mmio(CP_STALL_CTRL, 0x00000000);
        self.write_mmio(CP_PFFETCH_CNTL, 0x00000001);
        self.write_mmio(CP_PFFETCH_CNTL2, 0x00000001);
        self.write_mmio(CP_ALWAYS_ON_CNTL, 0x00000001);
        self.write_mmio(CP_ALWAYS_ON_CNTL2, 0x00000001);
        self.write_mmio(CP_QUEUE_THRESHOLDS, 0x00000001);
    }

    fn setup_vgt(&mut self) {
        self.write_mmio(VGT_CNTL, 0x00000001);
        self.write_mmio(VGT_PRIMITIVE_TYPE, 0x00000000);
        self.write_mmio(VGT_SHADER_STAGES_EN, 0x00000007);
        self.write_mmio(VGT_GS_OFFLOAD, 0x00000001);
        self.write_mmio(VGT_GS_OFFLOAD_2, 0x00000001);
        self.write_mmio(VGT_GSVS_RING_OFFSET_0, REG_VRAM_BASE + 0x100000);
        self.write_mmio(VGT_GSVS_RING_OFFSET_0 + 4, REG_VRAM_BASE + 0x200000);
        self.write_mmio(VGT_GSVS_RING_OFFSET_0 + 8, REG_VRAM_BASE + 0x300000);
        self.write_mmio(VGT_GSVS_RING_OFFSET_0 + 12, REG_VRAM_BASE + 0x400000);
        self.write_mmio(VGT_CONFIG, 0x00000001);
    }

    fn setup_primitive_assembly(&mut self) {
        self.write_mmio(PA_CL_CLIP_CNTL, 0x00000001);
        self.write_mmio(PA_CL_VTE_CNTL, 0x00000001);
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
    }

    fn setup_rasterizer(&mut self) {
        self.write_mmio(PA_SU_SC_MODE_CNTL, 0x00000001);
        self.write_mmio(PA_SC_MODE_CNTL_0, 0x00000001);
        self.write_mmio(PA_SC_MODE_CNTL_1, 0x00000001);
        self.write_mmio(PA_SC_CLIP_CNTL, 0x00000001);
        self.write_mmio(PA_SC_AA_CONFIG, 0x00000001);
        self.write_mmio(PA_SC_AA_SAMPLE_LOCS_0, 0x00000000);
    }

    fn setup_depth_buffer(&mut self) {
        self.write_mmio(DB_SHADER_CONTROL, 0x00000001);
        self.write_mmio(DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(DB_DEPTH_BOUNDS, 0x00000000);
        self.write_mmio(DB_DEPTH_SLICE, 0x00000000);
        self.write_mmio(DB_DEPTH_VIEW, 0x00000000);
        self.write_mmio(DB_HTILE_SURFACE, REG_VRAM_BASE + 0x800000);
        self.write_mmio(DB_DEPTH_SIZE, self.vram_size);
        self.write_mmio(DB_Z_INFO, 0x00000001);
        self.write_mmio(DB_Z_READ_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_Z_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_DEPTH_CLEAR_LO, 0x00000000);
        self.write_mmio(DB_DEPTH_CLEAR_HI, 0x00000000);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(DB_STENCIL_CONTROL, 0x00000001);
        self.write_mmio(DB_STENCIL_READ_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCIL_WRITE_MASK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCIL_REF, 0x00000000);
        self.write_mmio(DB_STENCILREFMASK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCILREFMASK_BACK, 0xFFFFFFFF);
        self.write_mmio(DB_STENCILCLEAR, 0x00000000);
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
        self.write_mmio(CB_COLOR0_SIZE, self.vram_size);
        self.write_mmio(CB_COLOR0_PITCH, 1024);
        self.write_mmio(CB_COLOR0_SLICE, 0x00000000);
        self.write_mmio(CB_COLOR0_INFO, 0x00000001);
        self.write_mmio(CB_COLOR0_VIEW, 0x00000000);
        self.write_mmio(CB_COLOR0_ATTRIB, 0x00000000);
        self.write_mmio(CB_COLOR0_CLEAR, 0x00000000);
    }

    fn setup_texture_units(&mut self) {
        for i in 0..self.texture_units {
            self.write_mmio(TA_CNTL_AUX + i * 4, 0x00000001);
        }
    }

    fn setup_samplers(&mut self) {
        for i in 0..self.texture_units {
            self.write_mmio(0x9100 + i * 4, 0x00000001);
        }
    }

    fn setup_vertex_cache(&mut self) {
        self.write_mmio(0x8B18, self.vertex_cache_size);
    }

    fn setup_index_buffer(&mut self) {
        self.write_mmio(0x8B1C, REG_VRAM_BASE + 0xA00000);
        self.write_mmio(0x8B20, self.primitive_buffer_size);
    }

    fn setup_primitive_buffer(&mut self) {
        self.write_mmio(0x8B24, REG_VRAM_BASE + 0xB00000);
        self.write_mmio(0x8B28, self.primitive_buffer_size);
    }

    fn setup_shader_registers(&mut self) {
        for i in 0..32 {
            self.write_mmio(0x8C00 + i * 4, 0x00000000);
        }
        for i in 0..32 {
            self.write_mmio(0x8D00 + i * 4, 0x00000000);
        }
    }

    fn setup_geometry_pipeline(&mut self) {
        self.write_mmio(0x8B30, 0x00000001);
        self.write_mmio(0x8B34, 0x00000001);
        self.write_mmio(0x8B38, 0x00000001);
    }

    fn setup_tessellation(&mut self) {
        self.write_mmio(0x8B40, 0x00000001);
        self.write_mmio(0x8B44, 0x00000001);
        self.write_mmio(0x8B48, 0x00000001);
        self.write_mmio(0x8B4C, 0x00000001);
        self.write_mmio(0x8B50, 0x00000001);
    }

    fn setup_compute_pipeline(&mut self) {
        self.write_mmio(0x8B54, 0x00000001);
        self.write_mmio(0x8B58, 0x00000001);
        self.write_mmio(0x8B5C, 0x00000001);
        self.write_mmio(0x8B60, 0x00000001);
        self.write_mmio(0x8B64, 0x00000001);
        self.write_mmio(0x8B68, 0x00000001);
    }

    fn setup_framebuffers(&mut self) {
        self.write_mmio(0x8B6C, REG_VRAM_BASE + 0xC00000);
        self.write_mmio(0x8B70, self.vram_size);
    }

    fn setup_render_targets(&mut self) {
        self.write_mmio(0x8B74, REG_VRAM_BASE + 0xD00000);
        self.write_mmio(0x8B78, self.vram_size);
    }

    fn setup_depth_stencil_buffer(&mut self) {
        self.write_mmio(0x8B7C, REG_VRAM_BASE + 0xE00000);
        self.write_mmio(0x8B80, self.vram_size);
    }

    fn setup_multisampling(&mut self) {
        self.write_mmio(0x8B84, 0x00000004);
        self.write_mmio(0x8B88, 0x00000004);
        self.write_mmio(0x8B8C, 0x00000004);
    }

    fn setup_antialiasing(&mut self) {
        self.write_mmio(0x8B90, 0x00000001);
        self.write_mmio(0x8B94, 0x00000001);
        self.write_mmio(0x8B98, 0x00000001);
    }

    fn setup_depth_bounds(&mut self) {
        self.write_mmio(0x8B9C, 0x00000000);
        self.write_mmio(0x8BA0, 0x00000000);
    }

    fn setup_stencil_ref(&mut self) {
        self.write_mmio(0x8BA4, 0x00000000);
    }

    fn setup_blend_constants(&mut self) {
        self.write_mmio(0x8BA8, 0x00000000);
        self.write_mmio(0x8BAC, 0x00000000);
        self.write_mmio(0x8BB0, 0x00000000);
        self.write_mmio(0x8BB4, 0x00000000);
    }

    fn setup_viewport(&mut self) {
        self.write_mmio(0x8BB8, 0x00000000);
        self.write_mmio(0x8BBC, 0x00000000);
        self.write_mmio(0x8BC0, 0x00000000);
        self.write_mmio(0x8BC4, 0x00000000);
    }

    fn setup_scissor(&mut self) {
        self.write_mmio(0x8BC8, 0x00000000);
        self.write_mmio(0x8BCC, 0x00000000);
        self.write_mmio(0x8BD0, 0x00000000);
        self.write_mmio(0x8BD4, 0x00000000);
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

    pub fn read_pixels(&mut self, x: i32, y: i32, width: i32, height: i32, format: u32, r#type: u32, pixels: *mut u32) {
        self.write_mmio(0x1B50, x as u32);
        self.write_mmio(0x1B54, y as u32);
        self.write_mmio(0x1B58, width as u32);
        self.write_mmio(0x1B5C, height as u32);
        self.write_mmio(0x1B60, format);
        self.write_mmio(0x1B64, r#type);
        self.write_mmio(0x1B68, pixels as u32);
        self.write_mmio(0x1B6C, 0x00000001);
    }

    pub fn finish(&mut self) {
        self.write_mmio(0x1B70, 0x00000001);
        self.wait_idle_inner();
    }

    pub fn flush(&mut self) {
        self.write_mmio(0x1B74, 0x00000001);
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

    pub fn compute_units(&self) -> u32 {
        self.compute_units
    }

    pub fn shader_engines(&self) -> u32 {
        self.shader_engines
    }

    pub fn rasterizers(&self) -> u32 {
        self.rasterizers
    }

    pub fn backends(&self) -> u32 {
        self.backends
    }

    pub fn texture_units(&self) -> u32 {
        self.texture_units
    }
}

static mut GPU_INSTANCE: Option<R7_265> = None;

pub fn gpu_instance() -> &'static mut R7_265 {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(R7_265::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn amd_r7_265_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn amd_r7_265_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn amd_r7_265_driver_init() -> bool {
    amd_r7_265_init()
}

#[no_mangle]
pub extern "C" fn amd_r7_265_driver_detected() -> bool {
    amd_r7_265_detected()
}

#[no_mangle]
pub extern "C" fn amd_r7_265_draw_triangle_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_triangle_3d(x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_clear_screen_3d(color: u32) {
    let gpu = gpu_instance();
    gpu.clear_screen_3d(color);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_viewport(x: u32, y: u32, width: u32, height: u32) {
    let gpu = gpu_instance();
    gpu.set_viewport(x, y, width, height);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_projection_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_projection_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_world_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_world_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_view_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_view_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn amd_r7_265_enable_depth_test(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_depth_test(enabled);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_enable_blending(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_blending(enabled);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_blend_func(src: u32, dst: u32) {
    let gpu = gpu_instance();
    gpu.set_blend_func(src, dst);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_depth_func(func: u32) {
    let gpu = gpu_instance();
    gpu.set_depth_func(func);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_cull_mode(mode: u32) {
    let gpu = gpu_instance();
    gpu.set_cull_mode(mode);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_wireframe(enabled: bool) {
    let gpu = gpu_instance();
    gpu.set_wireframe(enabled);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_set_multisample_count(count: u32) {
    let gpu = gpu_instance();
    gpu.set_multisample_count(count);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_enable_multisampling(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_multisampling(enabled);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_enable_antialiasing(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_antialiasing(enabled);
}

#[no_mangle]
pub extern "C" fn amd_r7_265_finish() {
    let gpu = gpu_instance();
    gpu.finish();
}

#[no_mangle]
pub extern "C" fn amd_r7_265_flush() {
    let gpu = gpu_instance();
    gpu.flush();
}

#[no_mangle]
pub extern "C" fn amd_r7_265_get_status() -> u32 {
    let gpu = gpu_instance();
    gpu.get_status()
}
