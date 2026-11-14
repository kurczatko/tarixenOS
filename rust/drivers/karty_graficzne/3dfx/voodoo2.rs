/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

use core::sync::atomic::{AtomicU32, Ordering};

pub const V3DFX_VENDOR_ID: u16 = 0x121A;
pub const V3DFX_VOODOO2_DEVICE_ID: u16 = 0x0005;

pub const CHIP_SST2: u32 = 0x0005;
pub const VOODOO2_ARCH_VERSION: u32 = 2;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_FB_BASE: u32 = 0xE0000000;

pub const SST2_STATUS: u32 = 0x0000;
pub const SST2_INTR_CTRL: u32 = 0x0004;
pub const SST2_INTR_EN: u32 = 0x0008;
pub const SST2_VGA_CONTROL: u32 = 0x000C;
pub const SST2_FBZ_MODE: u32 = 0x0010;
pub const SST2_FBZ_CLR_MODE: u32 = 0x0014;
pub const SST2_FBZ_CLR_VALUE: u32 = 0x0018;
pub const SST2_FOG_MODE: u32 = 0x001C;
pub const SST2_ALPHA_MODE: u32 = 0x0020;
pub const SST2_FBZ_CLR_PATTERN: u32 = 0x0024;
pub const SST2_CHROMA_KEY: u32 = 0x0028;
pub const SST2_CHROMA_RANGE: u32 = 0x002C;
pub const SST2_STIPPLE: u32 = 0x0030;
pub const SST2_COLOR_FORMAT: u32 = 0x0034;
pub const SST2_DITHER_MODE: u32 = 0x0038;
pub const SST2_COL_BUF_ADDR: u32 = 0x003C;
pub const SST2_COL_BUF_STRIDE: u32 = 0x0040;
pub const SST2_AUX_BUF_ADDR: u32 = 0x0044;
pub const SST2_AUX_BUF_STRIDE: u32 = 0x0048;
pub const SST2_FB_W_ADDR: u32 = 0x004C;
pub const SST2_FB_W_ADDR_SHIFT: u32 = 0x0050;
pub const SST2_FB_W_DATA: u32 = 0x0054;
pub const SST2_FB_R_ADDR: u32 = 0x0058;
pub const SST2_FB_R_DATA: u32 = 0x005C;
pub const SST2_CLFB_ADDR: u32 = 0x0060;
pub const SST2_CLFB_FORMAT: u32 = 0x0064;
pub const SST2_FBI_CTRL0: u32 = 0x0068;
pub const SST2_FBI_CTRL1: u32 = 0x006C;
pub const SST2_FOGCOLOR: u32 = 0x0070;
pub const SST2_ALPHA_REF: u32 = 0x0074;
pub const SST2_FBZ_TX_MODE: u32 = 0x0078;
pub const SST2_FBZ_TX_ADDR: u32 = 0x007C;
pub const SST2_TEXLOD_ADDR: u32 = 0x0080;
pub const SST2_TEXLOD_DATA: u32 = 0x0084;
pub const SST2_TEXBASE_ADDR: u32 = 0x0088;
pub const SST2_TEXBASE_RANGE: u32 = 0x008C;
pub const SST2_TEX_ADDR: u32 = 0x0090;
pub const SST2_TEX_DATA_0: u32 = 0x0094;
pub const SST2_TEX_DATA_1: u32 = 0x0098;
pub const SST2_TRI_CMD: u32 = 0x009C;
pub const SST2_TRI_FILL: u32 = 0x00A0;
pub const SST2_FOGPARAM: u32 = 0x00A4;
pub const SST2_TEXLOD0: u32 = 0x00A8;
pub const SST2_TEXLOD1: u32 = 0x00AC;
pub const SST2_TEXLOD2: u32 = 0x00B0;
pub const SST2_TEXLOD3: u32 = 0x00B4;
pub const SST2_TRI_OUT_REPEAT: u32 = 0x00B8;
pub const SST2_TRI_OUT_POS: u32 = 0x00BC;
pub const SST2_TRI_OUT_UV: u32 = 0x00C0;
pub const SST2_TRI_OUT_CR: u32 = 0x00C4;
pub const SST2_TRI_OUT_CC: u32 = 0x00C8;
pub const SST2_TRI_OUT_FOG: u32 = 0x00CC;
pub const SST2_BLT_CMD: u32 = 0x00D0;
pub const SST2_BLT_XY: u32 = 0x00D4;
pub const SST2_BLT_WIDTH: u32 = 0x00D8;
pub const SST2_BLT_HOSTDATA: u32 = 0x00DC;
pub const SST2_FOG_TABLE: u32 = 0x0100;
pub const SST2_RAMDAC_ADDR: u32 = 0x0200;
pub const SST2_RAMDAC_DATA: u32 = 0x0204;
pub const SST2_RAMDAC_MASK: u32 = 0x0208;
pub const SST2_DFB_PATH: u32 = 0x0300;
pub const SST2_CLEAN: u32 = 0x0304;

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

pub struct Voodoo2 {
    mmio_base: *mut u32,
    fb_base: *mut u32,
    vram_size: u32,
    engine_clock: u32,
    memory_clock: u32,
    texture_units: u32,
    texture_memory: u32,
    fragment_pixels: u32,
    vertex_cache_size: u32,
    triangle_setup_units: u32,
    trilinear_filtering: bool,
    fog_enabled: bool,
}

impl Voodoo2 {
    pub fn new() -> Self {
        Voodoo2 {
            mmio_base: REG_MMIO_BASE as *mut u32,
            fb_base: REG_FB_BASE as *mut u32,
            vram_size: 4 * 1024 * 1024,
            engine_clock: 90000000,
            memory_clock: 50000000,
            texture_units: 2,
            texture_memory: 4 * 1024 * 1024,
            fragment_pixels: 2,
            vertex_cache_size: 16 * 1024,
            triangle_setup_units: 1,
            trilinear_filtering: false,
            fog_enabled: false,
        }
    }

    pub fn detect(&self) -> bool {
        unsafe {
            let vendor = self.read_mmio(0x00);
            let device = self.read_mmio(0x04);
            vendor == V3DFX_VENDOR_ID as u32 && device == V3DFX_VOODOO2_DEVICE_ID as u32
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
        self.setup_status();
        self.setup_intr_ctrl();
        self.setup_fbz_mode();
        self.setup_alpha_mode();
        self.setup_fog_mode();
        self.setup_chroma_key();
        self.setup_stipple();
        self.setup_color_format();
        self.setup_dither_mode();
        self.setup_col_buf();
        self.setup_aux_buf();
        self.setup_fbi_ctrl();
        self.setup_fogcolor();
        self.setup_fbz_tx_mode();
        self.setup_texbase();
        self.setup_texture_units();
        self.setup_triangle_setup();
        self.setup_blending();
        self.setup_depth_test();
        self.setup_stencil();
        self.setup_fog();
        self.setup_trilinear();
        self.setup_viewport();
        self.setup_scissor();
        self.enable_3d_acceleration();
        IS_INITIALIZED.store(1, Ordering::SeqCst);
        true
    }

    fn soft_reset(&mut self) {
        self.write_mmio(SST2_FBI_CTRL0, 0x00000001);
        for _ in 0..50000 {
            core::hint::spin_loop();
        }
        self.write_mmio(SST2_FBI_CTRL0, 0x00000000);
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(SST2_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    fn setup_status(&mut self) {
        self.write_mmio(SST2_STATUS, 0x00000000);
    }

    fn setup_intr_ctrl(&mut self) {
        self.write_mmio(SST2_INTR_CTRL, 0x00000001);
        self.write_mmio(SST2_INTR_EN, 0xFFFFFFFF);
    }

    fn setup_fbz_mode(&mut self) {
        self.write_mmio(SST2_FBZ_MODE, 0x00000001);
        self.write_mmio(SST2_FBZ_CLR_MODE, 0x00000001);
        self.write_mmio(SST2_FBZ_CLR_VALUE, 0x00000000);
        self.write_mmio(SST2_FBZ_CLR_PATTERN, 0xFFFFFFFF);
    }

    fn setup_alpha_mode(&mut self) {
        self.write_mmio(SST2_ALPHA_MODE, 0x00000001);
        self.write_mmio(SST2_ALPHA_REF, 0x00000080);
    }

    fn setup_fog_mode(&mut self) {
        self.write_mmio(SST2_FOG_MODE, 0x00000001);
    }

    fn setup_chroma_key(&mut self) {
        self.write_mmio(SST2_CHROMA_KEY, 0x00000000);
        self.write_mmio(SST2_CHROMA_RANGE, 0x00000000);
    }

    fn setup_stipple(&mut self) {
        self.write_mmio(SST2_STIPPLE, 0xFFFFFFFF);
    }

    fn setup_color_format(&mut self) {
        self.write_mmio(SST2_COLOR_FORMAT, 0x00000001);
    }

    fn setup_dither_mode(&mut self) {
        self.write_mmio(SST2_DITHER_MODE, 0x00000001);
    }

    fn setup_col_buf(&mut self) {
        self.write_mmio(SST2_COL_BUF_ADDR, REG_FB_BASE);
        self.write_mmio(SST2_COL_BUF_STRIDE, 1024);
    }

    fn setup_aux_buf(&mut self) {
        self.write_mmio(SST2_AUX_BUF_ADDR, REG_FB_BASE + 0x100000);
        self.write_mmio(SST2_AUX_BUF_STRIDE, 1024);
    }

    fn setup_fbi_ctrl(&mut self) {
        self.write_mmio(SST2_FBI_CTRL0, 0x00000001);
        self.write_mmio(SST2_FBI_CTRL1, 0x00000001);
        self.write_mmio(SST2_CLEAN, 0x00000001);
    }

    fn setup_fogcolor(&mut self) {
        self.write_mmio(SST2_FOGCOLOR, 0x00000000);
    }

    fn setup_fbz_tx_mode(&mut self) {
        self.write_mmio(SST2_FBZ_TX_MODE, 0x00000001);
        self.write_mmio(SST2_FBZ_TX_ADDR, REG_FB_BASE + 0x200000);
    }

    fn setup_texbase(&mut self) {
        self.write_mmio(SST2_TEXBASE_ADDR, REG_FB_BASE + 0x300000);
        self.write_mmio(SST2_TEXBASE_RANGE, self.texture_memory);
    }

    fn setup_texture_units(&mut self) {
        for i in 0..self.texture_units {
            self.write_mmio(SST2_TEX_ADDR + i * 0x10, 0x00000001);
            self.write_mmio(SST2_TEX_DATA_0 + i * 0x10, 0x00000001);
            self.write_mmio(SST2_TEX_DATA_1 + i * 0x10, 0x00000001);
        }
    }

    fn setup_triangle_setup(&mut self) {
        self.write_mmio(SST2_TRI_CMD, 0x00000001);
        self.write_mmio(SST2_TRI_FILL, 0x00000001);
        self.write_mmio(SST2_TRI_OUT_REPEAT, 0x00000001);
        self.write_mmio(SST2_TRI_OUT_POS, 0x00000001);
        self.write_mmio(SST2_TRI_OUT_UV, 0x00000001);
        self.write_mmio(SST2_TRI_OUT_CR, 0x00000001);
        self.write_mmio(SST2_TRI_OUT_CC, 0x00000001);
        self.write_mmio(SST2_TRI_OUT_FOG, 0x00000001);
    }

    fn setup_blending(&mut self) {
        self.write_mmio(0x0100, 0x00000001);
        self.write_mmio(0x0104, 0x00000101);
        self.write_mmio(0x0108, 0x00000001);
    }

    fn setup_depth_test(&mut self) {
        self.write_mmio(0x0110, 0x00000001);
        self.write_mmio(0x0114, 0x00000002);
    }

    fn setup_stencil(&mut self) {
        self.write_mmio(0x0120, 0x00000001);
        self.write_mmio(0x0124, 0x00000001);
        self.write_mmio(0x0128, 0x00000000);
    }

    fn setup_fog(&mut self) {
        self.write_mmio(SST2_FOGPARAM, 0x00000001);
        for i in 0..256 {
            self.write_mmio(SST2_FOG_TABLE + i * 4, 0x00000000);
        }
    }

    fn setup_trilinear(&mut self) {
        self.write_mmio(0x0200, if self.trilinear_filtering { 1 } else { 0 });
        self.write_mmio(0x0204, if self.trilinear_filtering { 1 } else { 0 });
        self.write_mmio(0x0208, if self.trilinear_filtering { 1 } else { 0 });
    }

    fn setup_viewport(&mut self) {
        self.write_mmio(0x0210, 0x00000000);
        self.write_mmio(0x0214, 0x00000000);
        self.write_mmio(0x0218, 0x00000000);
        self.write_mmio(0x021C, 0x00000000);
    }

    fn setup_scissor(&mut self) {
        self.write_mmio(0x0220, 0x00000000);
        self.write_mmio(0x0224, 0x00000000);
        self.write_mmio(0x0228, 0x00000000);
        self.write_mmio(0x022C, 0x00000000);
    }

    fn enable_3d_acceleration(&mut self) {
        self.write_mmio(0x0300, 0x00000001);
        self.write_mmio(0x0304, 0xFFFFFFFF);
        self.write_mmio(0x0308, 0x00000001);
    }

    fn read_mmio(&self, offset: u32) -> u32 {
        unsafe { self.mmio_base.add(offset as usize / 4).read_volatile() }
    }

    fn write_mmio(&mut self, offset: u32, value: u32) {
        unsafe { self.mmio_base.add(offset as usize / 4).write_volatile(value); }
    }

    pub fn submit_3d_command(&mut self, command: u32, param1: u32, param2: u32, param3: u32) {
        self.write_mmio(0x0400, command);
        self.write_mmio(0x0404, param1);
        self.write_mmio(0x0408, param2);
        self.write_mmio(0x040C, param3);
        self.write_mmio(0x0410, 0x00000001);
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
            self.write_mmio(0x0500 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn set_world_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio(0x0600 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn set_view_matrix(&mut self, matrix: &Matrix4x4) {
        for i in 0..16 {
            self.write_mmio(0x0700 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn enable_depth_test(&mut self, enabled: bool) {
        self.write_mmio(0x0800, if enabled { 1 } else { 0 });
    }

    pub fn enable_blending(&mut self, enabled: bool) {
        self.write_mmio(0x0810, if enabled { 1 } else { 0 });
    }

    pub fn set_blend_func(&mut self, src: u32, dst: u32) {
        self.write_mmio(0x0814, src);
        self.write_mmio(0x0818, dst);
    }

    pub fn set_depth_func(&mut self, func: u32) {
        self.write_mmio(0x0804, func);
    }

    pub fn set_cull_mode(&mut self, mode: u32) {
        self.write_mmio(0x0820, mode);
    }

    pub fn set_wireframe(&mut self, enabled: bool) {
        self.write_mmio(0x0824, if enabled { 1 } else { 0 });
    }

    pub fn set_multisample_count(&mut self, count: u32) {
        self.write_mmio(0x0828, count);
    }

    pub fn enable_multisampling(&mut self, enabled: bool) {
        self.write_mmio(0x082C, if enabled { 1 } else { 0 });
    }

    pub fn enable_antialiasing(&mut self, enabled: bool) {
        self.write_mmio(0x0830, if enabled { 1 } else { 0 });
    }

    pub fn set_depth_bounds_test(&mut self, enabled: bool, min: f32, max: f32) {
        self.write_mmio(0x0834, if enabled { 1 } else { 0 });
        self.write_mmio(0x0838, min.to_bits());
        self.write_mmio(0x083C, max.to_bits());
    }

    pub fn set_stencil_test(&mut self, enabled: bool, func: u32, ref_value: u32, mask: u32) {
        self.write_mmio(0x0840, if enabled { 1 } else { 0 });
        self.write_mmio(0x0844, func);
        self.write_mmio(0x0848, ref_value);
        self.write_mmio(0x084C, mask);
    }

    pub fn set_stencil_op(&mut self, fail: u32, zfail: u32, zpass: u32) {
        self.write_mmio(0x0850, fail);
        self.write_mmio(0x0854, zfail);
        self.write_mmio(0x0858, zpass);
    }

    pub fn set_blend_equation(&mut self, rgb: u32, alpha: u32) {
        self.write_mmio(0x0860, rgb);
        self.write_mmio(0x0864, alpha);
    }

    pub fn set_blend_color(&mut self, color: u32) {
        self.write_mmio(0x0868, color);
    }

    pub fn set_logic_op(&mut self, op: u32, enabled: bool) {
        self.write_mmio(0x086C, op);
        self.write_mmio(0x0870, if enabled { 1 } else { 0 });
    }

    pub fn set_dither(&mut self, enabled: bool) {
        self.write_mmio(0x0874, if enabled { 1 } else { 0 });
    }

    pub fn set_polygon_offset(&mut self, enabled: bool, factor: f32, units: f32) {
        self.write_mmio(0x0878, if enabled { 1 } else { 0 });
        self.write_mmio(0x087C, factor.to_bits());
        self.write_mmio(0x0880, units.to_bits());
    }

    pub fn set_sample_coverage(&mut self, value: f32, invert: bool) {
        self.write_mmio(0x0884, value.to_bits());
        self.write_mmio(0x0888, if invert { 1 } else { 0 });
    }

    pub fn set_vertex_array(&mut self, buffer: u32, stride: u32, offset: u32) {
        self.write_mmio(0x0900, buffer);
        self.write_mmio(0x0904, stride);
        self.write_mmio(0x0908, offset);
    }

    pub fn set_index_array(&mut self, buffer: u32, format: u32, offset: u32) {
        self.write_mmio(0x090C, buffer);
        self.write_mmio(0x0910, format);
        self.write_mmio(0x0914, offset);
    }

    pub fn draw_arrays(&mut self, mode: u32, first: u32, count: u32) {
        self.write_mmio(0x0918, mode);
        self.write_mmio(0x091C, first);
        self.write_mmio(0x0920, count);
        self.write_mmio(0x0924, 0x00000001);
    }

    pub fn draw_elements(&mut self, mode: u32, count: u32, type_: u32, offset: u32) {
        self.write_mmio(0x0928, mode);
        self.write_mmio(0x092C, count);
        self.write_mmio(0x0930, type_);
        self.write_mmio(0x0934, offset);
        self.write_mmio(0x0938, 0x00000001);
    }

    pub fn draw_instanced(&mut self, mode: u32, first: u32, count: u32, instance_count: u32) {
        self.write_mmio(0x093C, mode);
        self.write_mmio(0x0940, first);
        self.write_mmio(0x0944, count);
        self.write_mmio(0x0948, instance_count);
        self.write_mmio(0x094C, 0x00000001);
    }

    pub fn draw_elements_instanced(&mut self, mode: u32, count: u32, type_: u32, offset: u32, instance_count: u32) {
        self.write_mmio(0x0950, mode);
        self.write_mmio(0x0954, count);
        self.write_mmio(0x0958, type_);
        self.write_mmio(0x095C, offset);
        self.write_mmio(0x0960, instance_count);
        self.write_mmio(0x0964, 0x00000001);
    }

    pub fn set_shader_program(&mut self, program_id: u32) {
        self.write_mmio(0x0A00, program_id);
    }

    pub fn set_uniform_1f(&mut self, location: u32, value: f32) {
        self.write_mmio(0x0A04, location);
        self.write_mmio(0x0A08, value.to_bits());
    }

    pub fn set_uniform_2f(&mut self, location: u32, x: f32, y: f32) {
        self.write_mmio(0x0A0C, location);
        self.write_mmio(0x0A10, x.to_bits());
        self.write_mmio(0x0A14, y.to_bits());
    }

    pub fn set_uniform_3f(&mut self, location: u32, x: f32, y: f32, z: f32) {
        self.write_mmio(0x0A18, location);
        self.write_mmio(0x0A1C, x.to_bits());
        self.write_mmio(0x0A20, y.to_bits());
        self.write_mmio(0x0A24, z.to_bits());
    }

    pub fn set_uniform_4f(&mut self, location: u32, x: f32, y: f32, z: f32, w: f32) {
        self.write_mmio(0x0A28, location);
        self.write_mmio(0x0A2C, x.to_bits());
        self.write_mmio(0x0A30, y.to_bits());
        self.write_mmio(0x0A34, z.to_bits());
        self.write_mmio(0x0A38, w.to_bits());
    }

    pub fn set_uniform_matrix4fv(&mut self, location: u32, matrix: &Matrix4x4) {
        self.write_mmio(0x0A3C, location);
        for i in 0..16 {
            self.write_mmio(0x0A40 + (i as u32 * 4), matrix.m[i].to_bits());
        }
    }

    pub fn bind_texture(&mut self, unit: u32, texture_id: u32) {
        self.write_mmio(0x0B00, unit);
        self.write_mmio(0x0B04, texture_id);
    }

    pub fn bind_framebuffer(&mut self, framebuffer_id: u32) {
        self.write_mmio(0x0B10, framebuffer_id);
    }

    pub fn bind_renderbuffer(&mut self, renderbuffer_id: u32) {
        self.write_mmio(0x0B20, renderbuffer_id);
    }

    pub fn create_texture(&mut self, width: u32, height: u32, format: u32) -> u32 {
        self.write_mmio(0x0C00, width);
        self.write_mmio(0x0C04, height);
        self.write_mmio(0x0C08, format);
        self.write_mmio(0x0C0C, 0x00000001);
        self.read_mmio(0x0C10)
    }

    pub fn delete_texture(&mut self, texture_id: u32) {
        self.write_mmio(0x0C20, texture_id);
    }

    pub fn upload_texture_data(&mut self, texture_id: u32, data: *const u32, size: u32) {
        self.write_mmio(0x0C30, texture_id);
        self.write_mmio(0x0C34, data as u32);
        self.write_mmio(0x0C38, size);
        self.write_mmio(0x0C3C, 0x00000001);
    }

    pub fn generate_mipmaps(&mut self, texture_id: u32) {
        self.write_mmio(0x0C40, texture_id);
        self.write_mmio(0x0C44, 0x00000001);
    }

    pub fn set_texture_filter(&mut self, texture_id: u32, min_filter: u32, mag_filter: u32) {
        self.write_mmio(0x0C50, texture_id);
        self.write_mmio(0x0C54, min_filter);
        self.write_mmio(0x0C58, mag_filter);
    }

    pub fn set_texture_wrap(&mut self, texture_id: u32, wrap_s: u32, wrap_t: u32) {
        self.write_mmio(0x0C60, texture_id);
        self.write_mmio(0x0C64, wrap_s);
        self.write_mmio(0x0C68, wrap_t);
    }

    pub fn create_renderbuffer(&mut self, width: u32, height: u32, format: u32) -> u32 {
        self.write_mmio(0x0C80, width);
        self.write_mmio(0x0C84, height);
        self.write_mmio(0x0C88, format);
        self.write_mmio(0x0C8C, 0x00000001);
        self.read_mmio(0x0C90)
    }

    pub fn delete_renderbuffer(&mut self, renderbuffer_id: u32) {
        self.write_mmio(0x0CA0, renderbuffer_id);
    }

    pub fn create_framebuffer(&mut self) -> u32 {
        self.write_mmio(0x0CB0, 0x00000001);
        self.read_mmio(0x0CB4)
    }

    pub fn delete_framebuffer(&mut self, framebuffer_id: u32) {
        self.write_mmio(0x0CC0, framebuffer_id);
    }

    pub fn attach_texture_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, texture_id: u32, mip_level: u32) {
        self.write_mmio(0x0CD0, framebuffer_id);
        self.write_mmio(0x0CD4, attachment);
        self.write_mmio(0x0CD8, texture_id);
        self.write_mmio(0x0CDC, mip_level);
        self.write_mmio(0x0CE0, 0x00000001);
    }

    pub fn attach_renderbuffer_to_framebuffer(&mut self, framebuffer_id: u32, attachment: u32, renderbuffer_id: u32) {
        self.write_mmio(0x0CF0, framebuffer_id);
        self.write_mmio(0x0CF4, attachment);
        self.write_mmio(0x0CF8, renderbuffer_id);
        self.write_mmio(0x0CFC, 0x00000001);
    }

    pub fn set_draw_buffers(&mut self, count: u32, attachments: *const u32) {
        self.write_mmio(0x0D00, count);
        self.write_mmio(0x0D04, attachments as u32);
    }

    pub fn set_read_buffer(&mut self, attachment: u32) {
        self.write_mmio(0x0D10, attachment);
    }

    pub fn blit_framebuffer(&mut self, src_x0: i32, src_y0: i32, src_x1: i32, src_y1: i32, dst_x0: i32, dst_y0: i32, dst_x1: i32, dst_y1: i32, mask: u32, filter: u32) {
        self.write_mmio(0x0D20, src_x0 as u32);
        self.write_mmio(0x0D24, src_y0 as u32);
        self.write_mmio(0x0D28, src_x1 as u32);
        self.write_mmio(0x0D2C, src_y1 as u32);
        self.write_mmio(0x0D30, dst_x0 as u32);
        self.write_mmio(0x0D34, dst_y0 as u32);
        self.write_mmio(0x0D38, dst_x1 as u32);
        self.write_mmio(0x0D3C, dst_y1 as u32);
        self.write_mmio(0x0D40, mask);
        self.write_mmio(0x0D44, filter);
        self.write_mmio(0x0D48, 0x00000001);
    }

    pub fn read_pixels(&mut self, x: i32, y: i32, width: i32, height: i32, format: u32, r#type: u32, pixels: *mut u32) {
        self.write_mmio(0x0D50, x as u32);
        self.write_mmio(0x0D54, y as u32);
        self.write_mmio(0x0D58, width as u32);
        self.write_mmio(0x0D5C, height as u32);
        self.write_mmio(0x0D60, format);
        self.write_mmio(0x0D64, r#type);
        self.write_mmio(0x0D68, pixels as u32);
        self.write_mmio(0x0D6C, 0x00000001);
    }

    pub fn finish(&mut self) {
        self.write_mmio(0x0D70, 0x00000001);
        self.wait_idle();
    }

    pub fn flush(&mut self) {
        self.write_mmio(0x0D74, 0x00000001);
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

    pub fn texture_units(&self) -> u32 {
        self.texture_units
    }

    pub fn texture_memory(&self) -> u32 {
        self.texture_memory
    }

    pub fn fragment_pixels(&self) -> u32 {
        self.fragment_pixels
    }

    pub fn vertex_cache_size(&self) -> u32 {
        self.vertex_cache_size
    }

    pub fn triangle_setup_units(&self) -> u32 {
        self.triangle_setup_units
    }

    pub fn trilinear_filtering(&self) -> bool {
        self.trilinear_filtering
    }

    pub fn fog_enabled(&self) -> bool {
        self.fog_enabled
    }
}

static mut GPU_INSTANCE: Option<Voodoo2> = None;

pub fn gpu_instance() -> &'static mut Voodoo2 {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(Voodoo2::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn voodoo2_init() -> bool {
    let gpu = gpu_instance();
    gpu.init()
}

pub fn voodoo2_detected() -> bool {
    let gpu = gpu_instance();
    gpu.detect()
}

#[no_mangle]
pub extern "C" fn voodoo2_driver_init() -> bool {
    voodoo2_init()
}

#[no_mangle]
pub extern "C" fn voodoo2_driver_detected() -> bool {
    voodoo2_detected()
}

#[no_mangle]
pub extern "C" fn voodoo2_draw_triangle_3d(x1: i32, y1: i32, z1: i32, x2: i32, y2: i32, z2: i32, x3: i32, y3: i32, z3: i32, color: u32) {
    let gpu = gpu_instance();
    gpu.draw_triangle_3d(x1, y1, z1, x2, y2, z2, x3, y3, z3, color);
}

#[no_mangle]
pub extern "C" fn voodoo2_clear_screen_3d(color: u32) {
    let gpu = gpu_instance();
    gpu.clear_screen_3d(color);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_viewport(x: u32, y: u32, width: u32, height: u32) {
    let gpu = gpu_instance();
    gpu.set_viewport(x, y, width, height);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_projection_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_projection_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn voodoo2_set_world_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_world_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn voodoo2_set_view_matrix(matrix: *const Matrix4x4) {
    let gpu = gpu_instance();
    unsafe {
        gpu.set_view_matrix(&*matrix);
    }
}

#[no_mangle]
pub extern "C" fn voodoo2_enable_depth_test(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_depth_test(enabled);
}

#[no_mangle]
pub extern "C" fn voodoo2_enable_blending(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_blending(enabled);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_blend_func(src: u32, dst: u32) {
    let gpu = gpu_instance();
    gpu.set_blend_func(src, dst);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_depth_func(func: u32) {
    let gpu = gpu_instance();
    gpu.set_depth_func(func);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_cull_mode(mode: u32) {
    let gpu = gpu_instance();
    gpu.set_cull_mode(mode);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_wireframe(enabled: bool) {
    let gpu = gpu_instance();
    gpu.set_wireframe(enabled);
}

#[no_mangle]
pub extern "C" fn voodoo2_set_multisample_count(count: u32) {
    let gpu = gpu_instance();
    gpu.set_multisample_count(count);
}

#[no_mangle]
pub extern "C" fn voodoo2_enable_multisampling(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_multisampling(enabled);
}

#[no_mangle]
pub extern "C" fn voodoo2_enable_antialiasing(enabled: bool) {
    let gpu = gpu_instance();
    gpu.enable_antialiasing(enabled);
}

#[no_mangle]
pub extern "C" fn voodoo2_finish() {
    let gpu = gpu_instance();
    gpu.finish();
}

#[no_mangle]
pub extern "C" fn voodoo2_flush() {
    let gpu = gpu_instance();
    gpu.flush();
}

#[no_mangle]
pub extern "C" fn voodoo2_get_status() -> u32 {
    let gpu = gpu_instance();
    gpu.get_status()
}
