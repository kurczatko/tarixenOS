#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, AtomicU64, Ordering};

pub const AMD_VENDOR_ID: u16 = 0x1002;
pub const R9_280X_DEVICE_ID: u16 = 0x6798;
pub const CHIP_TAHITI: u32 = 0x6798;
pub const GCN_ARCH_VERSION: u32 = 1;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_VRAM_BASE: u32 = 0xE0000000;
pub const REG_GART_BASE: u32 = 0xD0000000;
pub const REG_DOORBELL_BASE: u32 = 0xC0000000;

pub const AMD_GRBM_CNTL: u32 = 0x8000;
pub const AMD_GRBM_STATUS: u32 = 0x8004;
pub const AMD_GRBM_SOFT_RESET: u32 = 0x8040;
pub const AMD_GRBM_GFX_INDEX: u32 = 0x8400;
pub const AMD_GRBM_GFX_INDEX_INSTANCE: u32 = 0x8404;
pub const AMD_GRBM_TIMESTAMP: u32 = 0x89C0;

pub const AMD_CP_BASE: u32 = 0x8C00;
pub const AMD_CP_RB_CNTL: u32 = 0x8C00;
pub const AMD_CP_RB_BASE: u32 = 0x8C04;
pub const AMD_CP_RB_RPTR: u32 = 0x8C08;
pub const AMD_CP_RB_WPTR: u32 = 0x8C0C;
pub const AMD_CP_RB_WPTR_DELAY: u32 = 0x8C10;
pub const AMD_CP_RB_RPTR_WPTR_ADDR: u32 = 0x8C14;
pub const AMD_CP_ME_CNTL: u32 = 0x8C38;
pub const AMD_CP_ME_CNTL2: u32 = 0x8C3C;
pub const AMD_CP_MEC_CNTL: u32 = 0x8D30;
pub const AMD_CP_INT_CNTL: u32 = 0x8C70;
pub const AMD_CP_INT_STATUS: u32 = 0x8C74;
pub const AMD_CP_STALL_CTRL: u32 = 0x8C94;
pub const AMD_CP_PERFMON_CNTL: u32 = 0x8C98;
pub const AMD_CP_HW_DEBUG_CNTL: u32 = 0x8C9C;

pub const AMD_VGT_BASE: u32 = 0x8800;
pub const AMD_VGT_CNTL: u32 = 0x8800;
pub const AMD_VGT_PRIMITIVE_TYPE: u32 = 0x8804;
pub const AMD_VGT_MULTI_PRIM_IBRESET_INDX: u32 = 0x8808;
pub const AMD_VGT_MULTI_PRIM_IBRESET_INDX1: u32 = 0x880C;
pub const AMD_VGT_SHADER_STAGES_EN: u32 = 0x8A80;
pub const AMD_VGT_CONFIG: u32 = 0x8B10;
pub const AMD_VGT_GSVS_RING_OFFSET_0: u32 = 0x8A88;
pub const AMD_VGT_GSVS_RING_SIZE: u32 = 0x8A9C;

pub const AMD_PA_BASE: u32 = 0x8C80;
pub const AMD_PA_SC_MODE_CNTL_0: u32 = 0x8C94;
pub const AMD_PA_SC_AA_CONFIG: u32 = 0x8D2C;
pub const AMD_PA_CL_VTE_CNTL: u32 = 0x8990;
pub const AMD_PA_CL_VS_OUT_CNTL: u32 = 0x8994;
pub const AMD_PA_CL_VS_OUT_CNTL_0: u32 = 0x8994;
pub const AMD_PA_SU_SC_MODE_CNTL: u32 = 0x8920;
pub const AMD_PA_SU_HARDWARE_INDX: u32 = 0x8924;
pub const AMD_PA_SC_LINE_STIPPLE: u32 = 0x8B30;

pub const AMD_SC_BASE: u32 = 0x8D00;
pub const AMD_SC_PRIM_VPORT_XSCALE: u32 = 0x8D10;
pub const AMD_SC_PRIM_VPORT_XOFFSET: u32 = 0x8D14;
pub const AMD_SC_PRIM_VPORT_YSCALE: u32 = 0x8D18;
pub const AMD_SC_PRIM_VPORT_YOFFSET: u32 = 0x8D1C;
pub const AMD_SC_PRIM_VPORT_ZSCALE: u32 = 0x8D20;
pub const AMD_SC_PRIM_VPORT_ZOFFSET: u32 = 0x8D24;
pub const AMD_SC_SCISSOR_0: u32 = 0x8D30;
pub const AMD_SC_SCISSOR_1: u32 = 0x8D34;

pub const AMD_DB_BASE: u32 = 0x2800;
pub const AMD_DB_DEPTH_CONTROL: u32 = 0x2800;
pub const AMD_DB_STENCIL_CONTROL: u32 = 0x2804;
pub const AMD_DB_Z_INFO: u32 = 0x2808;
pub const AMD_DB_STENCIL_INFO: u32 = 0x280C;
pub const AMD_DB_SHADER_CONTROL: u32 = 0x2810;
pub const AMD_DB_DEPTH_SIZE: u32 = 0x2814;
pub const AMD_DB_DEPTH_VIEW: u32 = 0x2818;
pub const AMD_DB_DEPTH_SLICE: u32 = 0x281C;
pub const AMD_DB_DEPTH_CLEAR: u32 = 0x2820;
pub const AMD_DB_STENCIL_CLEAR: u32 = 0x2824;
pub const AMD_DB_DEPTH_RANGE: u32 = 0x2830;

pub const AMD_CB_BASE: u32 = 0x1600;
pub const AMD_CB_COLOR_CONTROL: u32 = 0x16C8;
pub const AMD_CB_COLOR0_BASE: u32 = 0x16CC;
pub const AMD_CB_COLOR0_SIZE: u32 = 0x16D0;
pub const AMD_CB_COLOR0_PITCH: u32 = 0x16D4;
pub const AMD_CB_COLOR0_SLICE: u32 = 0x16D8;
pub const AMD_CB_COLOR0_VIEW: u32 = 0x16DC;
pub const AMD_CB_COLOR0_ATTRIB: u32 = 0x16E0;
pub const AMD_CB_COLOR1_BASE: u32 = 0x16E4;
pub const AMD_CB_COLOR1_SIZE: u32 = 0x16E8;
pub const AMD_CB_COLOR2_BASE: u32 = 0x16EC;
pub const AMD_CB_COLOR2_SIZE: u32 = 0x16F0;
pub const AMD_CB_COLOR3_BASE: u32 = 0x16F4;
pub const AMD_CB_COLOR3_SIZE: u32 = 0x16F8;
pub const AMD_CB_TARGET_MASK: u32 = 0x1714;
pub const AMD_CB_SHADER_MASK: u32 = 0x1718;
pub const AMD_CB_CLEAR: u32 = 0x171C;
pub const AMD_CB_FMASK0_BASE: u32 = 0x1720;
pub const AMD_CB_FMASK0_SIZE: u32 = 0x1724;
pub const AMD_CB_CMASK0_BASE: u32 = 0x1728;
pub const AMD_CB_CMASK0_SIZE: u32 = 0x172C;

pub const AMD_SX_BASE: u32 = 0x9C00;
pub const AMD_SX_MISC: u32 = 0x9C00;
pub const AMD_SX_EXPORT_BUFFER_SIZES: u32 = 0x9C04;
pub const AMD_SX_COLOR_BUFFER_SIZES: u32 = 0x9C08;
pub const AMD_SX_POSITION_EXPORT: u32 = 0x9C0C;
pub const AMD_SX_DEPTH_BUFFER_SIZES: u32 = 0x9C10;
pub const AMD_SX_ALPHA_REF: u32 = 0x9C14;
pub const AMD_SX_MRT0: u32 = 0x9C18;
pub const AMD_SX_MRT1: u32 = 0x9C1C;
pub const AMD_SX_MRT2: u32 = 0x9C20;
pub const AMD_SX_MRT3: u32 = 0x9C24;

pub const AMD_TA_BASE: u32 = 0xCA00;
pub const AMD_TA_CNTL_AUX: u32 = 0xCA00;
pub const AMD_TA_CNTL: u32 = 0xCA04;
pub const AMD_TA_CNTL_AUX_0: u32 = 0xCA08;
pub const AMD_TA_CNTL_AUX_1: u32 = 0xCA0C;
pub const AMD_TA_STATUS: u32 = 0xCA10;

pub const AMD_TC_BASE: u32 = 0xCC00;
pub const AMD_TC_CNTL: u32 = 0xCC00;
pub const AMD_TC_CNTL_0: u32 = 0xCC04;
pub const AMD_TC_CNTL_1: u32 = 0xCC08;
pub const AMD_TC_CNTL_2: u32 = 0xCC0C;

pub const AMD_SPI_BASE: u32 = 0x9C40;
pub const AMD_SPI_CONFIG_CNTL: u32 = 0x9C40;
pub const AMD_SPI_CONFIG_CNTL_0: u32 = 0x9C44;
pub const AMD_SPI_CONFIG_CNTL_1: u32 = 0x9C48;
pub const AMD_SPI_INPUT_Z: u32 = 0x9C4C;
pub const AMD_SPI_PS_IN_CONTROL: u32 = 0x9C80;
pub const AMD_SPI_PS_INPUT_ADDR: u32 = 0x9C84;
pub const AMD_SPI_PS_INPUT_CNTL: u32 = 0x9C88;

pub const AMD_SQC_BASE: u32 = 0x9CC0;
pub const AMD_SQC_CNTL: u32 = 0x9CC0;
pub const AMD_SQC_CNTL_0: u32 = 0x9CC4;

pub const AMD_SDMA0_BASE: u32 = 0x2E00;
pub const AMD_SDMA1_BASE: u32 = 0x2F00;
pub const AMD_SDMA0_RB_CNTL: u32 = 0x2E00;
pub const AMD_SDMA0_RB_BASE: u32 = 0x2E04;
pub const AMD_SDMA0_RB_RPTR: u32 = 0x2E08;
pub const AMD_SDMA0_RB_WPTR: u32 = 0x2E0C;
pub const AMD_SDMA1_RB_CNTL: u32 = 0x2F00;
pub const AMD_SDMA1_RB_BASE: u32 = 0x2F04;
pub const AMD_SDMA1_RB_RPTR: u32 = 0x2F08;
pub const AMD_SDMA1_RB_WPTR: u32 = 0x2F0C;

pub const AMD_SMU_BASE: u32 = 0xA000;
pub const AMD_SMU_CNTL: u32 = 0xA000;
pub const AMD_SMU_STATUS: u32 = 0xA004;
pub const AMD_SMU_GFX_CLOCK_CNTL: u32 = 0xA010;
pub const AMD_SMU_MEM_CLOCK_CNTL: u32 = 0xA014;
pub const AMD_SMU_POWER_STATE: u32 = 0xA018;

pub const R9_280X_VRAM_SIZE: u64 = 3 * 1024 * 1024 * 1024;
pub const R9_280X_GART_SIZE: u64 = 4 * 1024 * 1024 * 1024;
pub const R9_280X_ENGINE_CLOCK: u32 = 1000000;
pub const R9_280X_MEMORY_CLOCK: u32 = 6000000;
pub const R9_280X_COMPUTE_UNITS: u32 = 32;
pub const R9_280X_STREAM_PROCESSORS: u32 = 2048;
pub const R9_280X_ROP_COUNT: u32 = 32;
pub const R9_280X_TEXTURE_UNITS: u32 = 128;
pub const R9_280X_MEMORY_BUS_WIDTH: u32 = 384;
pub const R9_280X_VERTEX_CACHE_SIZE: u32 = 32 * 1024;
pub const R9_280X_PRIMITIVE_BUFFER_SIZE: u32 = 32 * 1024;

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
    pub compute_units: u32,
    pub stream_processors: u32,
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

static mut GPU_INSTANCE: Option<GpuInstance> = None;

impl GpuInstance {
    pub fn new(device_id: u16) -> Self {
        let (vram_size, engine_clock, memory_clock, compute_units, stream_processors, rop_count, texture_units, memory_bus_width, vertex_cache_size) = match device_id {
            R9_280X_DEVICE_ID => (R9_280X_VRAM_SIZE, R9_280X_ENGINE_CLOCK, R9_280X_MEMORY_CLOCK, R9_280X_COMPUTE_UNITS, R9_280X_STREAM_PROCESSORS, R9_280X_ROP_COUNT, R9_280X_TEXTURE_UNITS, R9_280X_MEMORY_BUS_WIDTH, R9_280X_VERTEX_CACHE_SIZE),
            _ => (R9_280X_VRAM_SIZE, R9_280X_ENGINE_CLOCK, R9_280X_MEMORY_CLOCK, R9_280X_COMPUTE_UNITS, R9_280X_STREAM_PROCESSORS, R9_280X_ROP_COUNT, R9_280X_TEXTURE_UNITS, R9_280X_MEMORY_BUS_WIDTH, R9_280X_VERTEX_CACHE_SIZE),
        };
        GpuInstance {
            mmio_base: REG_MMIO_BASE as *mut u32,
            vram_base: REG_VRAM_BASE as *mut u32,
            gart_base: REG_GART_BASE as *mut u32,
            doorbell_base: REG_DOORBELL_BASE as *mut u32,
            vram_size,
            vram_used: AtomicU64::new(0),
            gart_size: R9_280X_GART_SIZE,
            engine_clock,
            memory_clock,
            compute_units,
            stream_processors,
            rop_count,
            texture_units,
            memory_bus_width,
            vertex_cache_size,
            primitive_buffer_size: R9_280X_PRIMITIVE_BUFFER_SIZE,
            command_buffer_size: 16 * 1024 * 1024,
            initialized: AtomicBool::new(false),
            error_count: AtomicU32::new(0),
            ring_gfx: RingBuffer {
                base_addr: (REG_VRAM_BASE as u64) + 0x100000,
                size: 16 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 0,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(0) },
            },
            ring_compute: RingBuffer {
                base_addr: (REG_VRAM_BASE as u64) + 0x2000000,
                size: 8 * 1024 * 1024,
                rptr: AtomicU32::new(0),
                wptr: AtomicU32::new(0),
                doorbell_offset: 1,
                doorbell_page: unsafe { (REG_DOORBELL_BASE as *mut u32).add(1) },
            },
            ring_dma: RingBuffer {
                base_addr: (REG_VRAM_BASE as u64) + 0x4000000,
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
            vendor == AMD_VENDOR_ID as u32 && device == R9_280X_DEVICE_ID as u32 && rev != 0xFFFFFFFF
        }
    }

    pub fn device_id(&self) -> u16 {
        unsafe { self.read_mmio(0x04) as u16 }
    }

    pub fn soft_reset(&mut self) {
        self.write_mmio(AMD_GRBM_SOFT_RESET, 0x00000001);
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        self.write_mmio(AMD_GRBM_SOFT_RESET, 0x00000000);
        self.wait_idle_inner();
    }

    fn wait_idle_inner(&mut self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(AMD_GRBM_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
        self.error_count.fetch_add(1, Ordering::SeqCst);
    }

    pub fn wait_idle(&self) {
        for _ in 0..2000000 {
            let status = self.read_mmio(AMD_GRBM_STATUS);
            if status & 0x00000001 == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn init_ring_buffers(&mut self) {
        self.write_mmio(AMD_CP_RB_CNTL, 0x00000001);
        self.write_mmio_64(AMD_CP_RB_BASE, self.ring_gfx.base_addr);
        self.write_mmio(AMD_CP_RB_RPTR, 0x00000000);
        self.write_mmio(AMD_CP_RB_WPTR, 0x00000000);
        self.write_mmio(AMD_CP_ME_CNTL, 0x00000001);
        self.write_mmio(AMD_CP_ME_CNTL2, 0x00000001);
        self.write_mmio(AMD_CP_INT_CNTL, 0x00000001);
        self.write_mmio(AMD_CP_STALL_CTRL, 0x00000000);
        self.write_mmio(AMD_SDMA0_RB_CNTL, 0x00000001);
        self.write_mmio_64(AMD_SDMA0_RB_BASE, self.ring_compute.base_addr);
        self.write_mmio(AMD_SDMA0_RB_RPTR, 0x00000000);
        self.write_mmio(AMD_SDMA0_RB_WPTR, 0x00000000);
        self.write_mmio(AMD_SDMA1_RB_CNTL, 0x00000001);
        self.write_mmio_64(AMD_SDMA1_RB_BASE, self.ring_dma.base_addr);
        self.write_mmio(AMD_SDMA1_RB_RPTR, 0x00000000);
        self.write_mmio(AMD_SDMA1_RB_WPTR, 0x00000000);
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
        self.write_mmio(AMD_CP_RB_WPTR, wptr);
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
        self.setup_cp();
        self.setup_vgt();
        self.setup_pa();
        self.setup_sc();
        self.setup_db();
        self.setup_cb();
        self.setup_sx();
        self.setup_ta();
        self.setup_tc();
        self.setup_spi();
        self.setup_sqc();
        self.setup_sdma();
        self.setup_smu();
        self.init_ring_buffers();
        self.enable_3d_acceleration();
        self.initialized.store(true, Ordering::SeqCst);
        true
    }

    fn setup_grbm(&mut self) {
        self.write_mmio(AMD_GRBM_CNTL, 0x00000001);
        self.write_mmio(AMD_GRBM_GFX_INDEX, 0x00000000);
        self.write_mmio(AMD_GRBM_TIMESTAMP, 0x00000000);
    }

    fn setup_cp(&mut self) {
        self.write_mmio(AMD_CP_ME_CNTL, 0x00000001);
        self.write_mmio(AMD_CP_ME_CNTL2, 0x00000001);
        self.write_mmio(AMD_CP_INT_CNTL, 0x00000001);
        self.write_mmio(AMD_CP_INT_STATUS, 0xFFFFFFFF);
        self.write_mmio(AMD_CP_STALL_CTRL, 0x00000000);
    }

    fn setup_vgt(&mut self) {
        self.write_mmio(AMD_VGT_CNTL, 0x00000001);
        self.write_mmio(AMD_VGT_PRIMITIVE_TYPE, 0x00000000);
        self.write_mmio(AMD_VGT_SHADER_STAGES_EN, 0x00000001);
        self.write_mmio(AMD_VGT_CONFIG, 0x00000001);
    }

    fn setup_pa(&mut self) {
        self.write_mmio(AMD_PA_SC_MODE_CNTL_0, 0x00000001);
        self.write_mmio(AMD_PA_SC_AA_CONFIG, 0x00000001);
        self.write_mmio(AMD_PA_CL_VTE_CNTL, 0x00000001);
        self.write_mmio(AMD_PA_CL_VS_OUT_CNTL_0, 0x00000001);
        self.write_mmio(AMD_PA_SU_SC_MODE_CNTL, 0x00000001);
        self.write_mmio(AMD_PA_SU_HARDWARE_INDX, 0x00000001);
    }

    fn setup_sc(&mut self) {
        self.write_mmio(AMD_SC_PRIM_VPORT_XSCALE, 0x00000000);
        self.write_mmio(AMD_SC_PRIM_VPORT_XOFFSET, 0x00000000);
        self.write_mmio(AMD_SC_PRIM_VPORT_YSCALE, 0x00000000);
        self.write_mmio(AMD_SC_PRIM_VPORT_YOFFSET, 0x00000000);
        self.write_mmio(AMD_SC_PRIM_VPORT_ZSCALE, 0x00000000);
        self.write_mmio(AMD_SC_PRIM_VPORT_ZOFFSET, 0x00000000);
        self.write_mmio(AMD_SC_SCISSOR_0, 0x00000000);
        self.write_mmio(AMD_SC_SCISSOR_1, 0x00000000);
    }

    fn setup_db(&mut self) {
        self.write_mmio(AMD_DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(AMD_DB_STENCIL_CONTROL, 0x00000001);
        self.write_mmio(AMD_DB_Z_INFO, 0x00000000);
        self.write_mmio(AMD_DB_SHADER_CONTROL, 0x00000001);
        self.write_mmio(AMD_DB_DEPTH_CLEAR, 0x00000000);
        self.write_mmio(AMD_DB_STENCIL_CLEAR, 0x00000000);
        self.write_mmio(AMD_DB_DEPTH_RANGE, 0x00000000);
    }

    fn setup_cb(&mut self) {
        self.write_mmio(AMD_CB_COLOR_CONTROL, 0x00000001);
        self.write_mmio(AMD_CB_TARGET_MASK, 0xFFFFFFFF);
        self.write_mmio(AMD_CB_SHADER_MASK, 0xFFFFFFFF);
        self.write_mmio(AMD_CB_CLEAR, 0x00000000);
    }

    fn setup_sx(&mut self) {
        self.write_mmio(AMD_SX_MISC, 0x00000001);
        self.write_mmio(AMD_SX_EXPORT_BUFFER_SIZES, 0x00000001);
        self.write_mmio(AMD_SX_COLOR_BUFFER_SIZES, 0x00000001);
        self.write_mmio(AMD_SX_POSITION_EXPORT, 0x00000001);
        self.write_mmio(AMD_SX_DEPTH_BUFFER_SIZES, 0x00000001);
        self.write_mmio(AMD_SX_ALPHA_REF, 0x00000000);
        self.write_mmio(AMD_SX_MRT0, 0x00000001);
        self.write_mmio(AMD_SX_MRT1, 0x00000001);
        self.write_mmio(AMD_SX_MRT2, 0x00000001);
        self.write_mmio(AMD_SX_MRT3, 0x00000001);
    }

    fn setup_ta(&mut self) {
        self.write_mmio(AMD_TA_CNTL_AUX, 0x00000001);
        self.write_mmio(AMD_TA_CNTL, 0x00000001);
        self.write_mmio(AMD_TA_CNTL_AUX_0, 0x00000001);
        self.write_mmio(AMD_TA_CNTL_AUX_1, 0x00000001);
    }

    fn setup_tc(&mut self) {
        self.write_mmio(AMD_TC_CNTL, 0x00000001);
        self.write_mmio(AMD_TC_CNTL_0, 0x00000001);
        self.write_mmio(AMD_TC_CNTL_1, 0x00000001);
        self.write_mmio(AMD_TC_CNTL_2, 0x00000001);
    }

    fn setup_spi(&mut self) {
        self.write_mmio(AMD_SPI_CONFIG_CNTL, 0x00000001);
        self.write_mmio(AMD_SPI_CONFIG_CNTL_0, 0x00000001);
        self.write_mmio(AMD_SPI_CONFIG_CNTL_1, 0x00000001);
        self.write_mmio(AMD_SPI_INPUT_Z, 0x00000000);
        self.write_mmio(AMD_SPI_PS_IN_CONTROL, 0x00000001);
        self.write_mmio(AMD_SPI_PS_INPUT_ADDR, 0x00000000);
        self.write_mmio(AMD_SPI_PS_INPUT_CNTL, 0x00000001);
    }

    fn setup_sqc(&mut self) {
        self.write_mmio(AMD_SQC_CNTL, 0x00000001);
        self.write_mmio(AMD_SQC_CNTL_0, 0x00000001);
    }

    fn setup_sdma(&mut self) {
        self.write_mmio(AMD_SDMA0_RB_CNTL, 0x00000001);
        self.write_mmio(AMD_SDMA1_RB_CNTL, 0x00000001);
    }

    fn setup_smu(&mut self) {
        self.write_mmio(AMD_SMU_CNTL, 0x00000001);
        self.write_mmio(AMD_SMU_GFX_CLOCK_CNTL, 0x00000001);
        self.write_mmio(AMD_SMU_MEM_CLOCK_CNTL, 0x00000001);
        self.write_mmio(AMD_SMU_POWER_STATE, 0x00000001);
    }

    pub fn enable_3d_acceleration(&mut self) {
        self.write_mmio(AMD_VGT_CNTL, 0x00000001);
        self.write_mmio(AMD_PA_SC_MODE_CNTL_0, 0x00000001);
        self.write_mmio(AMD_DB_DEPTH_CONTROL, 0x00000001);
        self.write_mmio(AMD_CB_COLOR_CONTROL, 0x00000001);
        self.write_mmio(AMD_SX_MISC, 0x00000001);
        self.write_mmio(AMD_TA_CNTL, 0x00000001);
        self.write_mmio(AMD_CP_ME_CNTL, 0x00000001);
        self.write_mmio(AMD_SPI_CONFIG_CNTL, 0x00000001);
        self.wait_idle();
    }

    pub fn compile_shader(&mut self, stage: u32, spirv_addr: u64, spirv_size: u32, entry_point: u64, entry_point_size: u32) -> u32 {
        let id = self.shader_module_count;
        if id >= 32 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.write_mmio(0x9D00, id);
        self.write_mmio_64(0x9D04, spirv_addr);
        self.write_mmio(0x9D08, spirv_size);
        self.write_mmio_64(0x9D0C, entry_point);
        self.write_mmio(0x9D10, entry_point_size);
        self.write_mmio(0x9D14, stage);
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
        self.write_mmio(0x9D18, id);
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
        let vs_addr = prog.vertex_shader_addr;
        let fs_addr = prog.fragment_shader_addr;
        self.write_mmio(0x9D20, 0x00000001);
        self.write_mmio(0x9D24, 0x00000001);
        self.write_mmio_64(0x9D28, vs_addr);
        self.write_mmio_64(0x9D30, fs_addr);
        self.write_mmio(0x9D38, 0x00000001);
    }

    pub fn set_uniform(&mut self, program_id: u32, index: u32, values: &[f32]) {
        if program_id >= self.shader_program_count || index >= 256 {
            return;
        }
        let offset = self.shader_programs[program_id as usize].uniform_offsets[index as usize];
        self.write_mmio(0x9D40, offset);
        for i in 0..values.len().min(4) {
            self.write_mmio(0x9D44 + i as u32 * 4, values[i].to_bits());
        }
    }

    pub fn set_viewport(&mut self, viewport: &Viewport) {
        self.write_mmio(AMD_SC_PRIM_VPORT_XSCALE, viewport.width.to_bits());
        self.write_mmio(AMD_SC_PRIM_VPORT_XOFFSET, viewport.x.to_bits());
        self.write_mmio(AMD_SC_PRIM_VPORT_YSCALE, viewport.height.to_bits());
        self.write_mmio(AMD_SC_PRIM_VPORT_YOFFSET, viewport.y.to_bits());
        self.write_mmio(AMD_SC_PRIM_VPORT_ZSCALE, (viewport.max_depth - viewport.min_depth).to_bits());
        self.write_mmio(AMD_SC_PRIM_VPORT_ZOFFSET, viewport.min_depth.to_bits());
    }

    pub fn set_scissor(&mut self, scissor: &Scissor) {
        self.write_mmio(AMD_SC_SCISSOR_0, scissor.x as u32);
        self.write_mmio(AMD_SC_SCISSOR_1, scissor.y as u32);
        self.write_mmio(0x8D38, scissor.width as u32);
        self.write_mmio(0x8D3C, scissor.height as u32);
        self.write_mmio(0x8D40, 0x00000001);
    }

    pub fn set_blend_state(&mut self, enable: bool, src_rgb: u32, dst_rgb: u32, src_alpha: u32, dst_alpha: u32, equation_rgb: u32, equation_alpha: u32) {
        self.write_mmio(AMD_CB_COLOR_CONTROL, if enable { 0x00000001 } else { 0x00000000 });
        self.write_mmio(0x1700, src_rgb);
        self.write_mmio(0x1704, dst_rgb);
        self.write_mmio(0x1708, src_alpha);
        self.write_mmio(0x170C, dst_alpha);
        self.write_mmio(0x1710, equation_rgb);
        self.write_mmio(0x1714, equation_alpha);
    }

    pub fn set_depth_state(&mut self, enable: bool, func: u32, write_mask: u32) {
        self.write_mmio(AMD_DB_DEPTH_CONTROL, if enable { 0x00000001 } else { 0x00000000 });
        self.write_mmio(AMD_DB_DEPTH_CONTROL + 4, func);
        self.write_mmio(AMD_DB_DEPTH_CONTROL + 8, write_mask);
    }

    pub fn set_stencil_state(&mut self, enable: bool, func: u32, ref_val: u32, mask: u32, op_fail: u32, op_zfail: u32, op_zpass: u32) {
        self.write_mmio(AMD_DB_STENCIL_CONTROL, if enable { 0x00000001 } else { 0x00000000 });
        self.write_mmio(AMD_DB_STENCIL_CONTROL + 4, func);
        self.write_mmio(AMD_DB_STENCIL_CONTROL + 8, ref_val);
        self.write_mmio(AMD_DB_STENCIL_CONTROL + 12, mask);
        self.write_mmio(AMD_DB_STENCIL_CONTROL + 16, op_fail);
        self.write_mmio(AMD_DB_STENCIL_CONTROL + 20, op_zfail);
        self.write_mmio(AMD_DB_STENCIL_CONTROL + 24, op_zpass);
    }

    pub fn set_rasterizer_state(&mut self, cull: u32, clip: u32, msaa: bool, aa: bool) {
        self.write_mmio(0x8928, cull);
        self.write_mmio(0x892C, clip);
        self.write_mmio(0x8930, if msaa { 0x00000001 } else { 0x00000000 });
        self.write_mmio(0x8934, if aa { 0x00000001 } else { 0x00000000 });
    }

    pub fn create_texture(&mut self, desc: &TextureDescriptor) -> u32 {
        self.write_mmio(AMD_TA_CNTL, 0x00000001);
        self.write_mmio(0xCC10, desc.address as u32);
        self.write_mmio(0xCC14, (desc.address >> 32) as u32);
        self.write_mmio(AMD_TC_CNTL_0, desc.format);
        self.write_mmio(AMD_TC_CNTL_1, desc.flags);
        self.write_mmio(AMD_TC_CNTL_2, desc.mip_levels);
        0
    }

    pub fn bind_texture(&mut self, unit: u32, address: u64) {
        self.write_mmio(0xCC20 + unit * 16, address as u32);
        self.write_mmio(0xCC24 + unit * 16, (address >> 32) as u32);
    }

    pub fn create_framebuffer(&mut self, width: u32, height: u32, format: u32, address: u64) -> u32 {
        self.write_mmio(AMD_CB_COLOR0_BASE, address as u32);
        self.write_mmio(AMD_CB_COLOR0_BASE + 4, (address >> 32) as u32);
        self.write_mmio(AMD_CB_COLOR0_SIZE, (width << 16) | height);
        self.write_mmio(AMD_CB_COLOR0_PITCH, width * 4);
        self.write_mmio(AMD_CB_COLOR0_ATTRIB, format);
        0
    }

    pub fn bind_framebuffer(&mut self, address: u64) {
        self.write_mmio(AMD_CB_COLOR0_BASE, address as u32);
        self.write_mmio(AMD_CB_COLOR0_BASE + 4, (address >> 32) as u32);
    }

    pub fn clear(&mut self, color: u32, depth: u32, stencil: u32) {
        self.write_mmio(AMD_CB_CLEAR, color);
        self.write_mmio(AMD_DB_DEPTH_CLEAR, depth);
        self.write_mmio(AMD_DB_STENCIL_CLEAR, stencil);
        self.write_mmio(AMD_CB_COLOR_CONTROL, 0x00000001);
    }

    pub fn draw_arrays(&mut self, primitive: u32, first: u32, count: u32) {
        self.write_mmio(AMD_VGT_PRIMITIVE_TYPE, primitive);
        self.write_mmio(AMD_VGT_CNTL, 0x00000001);
        self.write_mmio(AMD_VGT_SHADER_STAGES_EN, 0x00000001);
        self.ring_write(0, (primitive << 28) | (count & 0x0FFFFFFF));
        self.ring_write(0, first);
        self.ring_submit(0);
        self.wait_idle();
    }

    pub fn draw_elements(&mut self, primitive: u32, index_count: u32, index_format: u32, index_addr: u64) {
        self.write_mmio(AMD_VGT_PRIMITIVE_TYPE, primitive);
        self.write_mmio(0x8810, index_format);
        self.write_mmio(0x8814, index_addr as u32);
        self.write_mmio(0x8818, (index_addr >> 32) as u32);
        self.write_mmio(0x881C, index_count * 4);
        self.write_mmio(AMD_VGT_CNTL, 0x00000001);
        self.write_mmio(AMD_VGT_SHADER_STAGES_EN, 0x00000001);
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
        self.write_mmio(0x9E00, id);
        self.write_mmio(0x9E04, size);
        self.command_buffers[id as usize] = CommandBuffer {
            id,
            addr: (REG_VRAM_BASE as u64) + 0x10000000 + id as u64 * size as u64,
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
        self.write_mmio(0x9E08, id);
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
        cb.offset.store((offset + 4) as u32, Ordering::SeqCst);
    }

    pub fn command_buffer_end(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        self.write_mmio(0x9E0C, id);
    }

    pub fn command_buffer_submit(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        let cb = &self.command_buffers[id as usize];
        let addr = cb.addr;
        let size = cb.size;
        self.write_mmio(0x9E10, addr as u32);
        self.write_mmio(0x9E14, (addr >> 32) as u32);
        self.write_mmio(0x9E18, size);
        self.write_mmio(0x9E1C, id);
        self.command_buffers[id as usize].submitted.store(true, Ordering::SeqCst);
        self.wait_idle();
    }

    pub fn command_buffer_reset(&mut self, id: u32) {
        if id >= self.command_buffer_count {
            return;
        }
        self.write_mmio(0x9E20, id);
        self.command_buffers[id as usize].offset.store(0, Ordering::SeqCst);
        self.command_buffers[id as usize].submitted.store(false, Ordering::SeqCst);
    }

    pub fn create_swap_chain(&mut self, image_count: u32, width: u32, height: u32, format: u32) -> u32 {
        let id = self.swap_chain_count;
        if id >= 8 || image_count > 8 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
            return 0xFFFFFFFF;
        }
        self.write_mmio(0x9F00, id);
        self.write_mmio(0x9F04, image_count);
        self.write_mmio(0x9F08, format);
        for i in 0..image_count as usize {
            self.write_mmio_64(0x9F0C + i as u32 * 8, (REG_VRAM_BASE as u64) + 0x20000000 + i as u64 * (width as u64 * height as u64 * 4));
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
        let idx = self.swap_chains[id as usize].current_image.load(Ordering::SeqCst);
        self.write_mmio(0x9F2C, id);
        self.swap_chains[id as usize].current_image.store((idx + 1) % self.swap_chains[id as usize].image_count, Ordering::SeqCst);
        self.swap_chains[id as usize].acquired.store(true, Ordering::SeqCst);
        idx
    }

    pub fn swap_chain_present(&mut self, id: u32) {
        if id >= self.swap_chain_count {
            return;
        }
        let sc = &self.swap_chains[id as usize];
        self.write_mmio(0x9F2C, sc.image_addrs[sc.current_image.load(Ordering::SeqCst) as usize] as u32);
        self.write_mmio(0x9F2C + 4, (sc.image_addrs[sc.current_image.load(Ordering::SeqCst) as usize] >> 32) as u32);
        self.write_mmio(0x9F30, id);
        self.wait_idle();
    }

    pub fn compute_dispatch(&mut self, group_x: u32, group_y: u32, group_z: u32) {
        self.write_mmio(0x9D50, group_x);
        self.write_mmio(0x9D54, group_y);
        self.write_mmio(0x9D58, group_z);
        self.write_mmio(0x9D5C, 0x00000001);
        self.wait_idle();
    }

    pub fn engine_clock(&self) -> u32 {
        self.engine_clock
    }

    pub fn memory_clock(&self) -> u32 {
        self.memory_clock
    }

    pub fn compute_units(&self) -> u32 {
        self.compute_units
    }

    pub fn stream_processors(&self) -> u32 {
        self.stream_processors
    }

    pub fn rop_count(&self) -> u32 {
        self.rop_count
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
pub extern "C" fn r9_280x_driver_init() -> bool {
    unsafe {
        match &mut GPU_INSTANCE {
            Some(inst) => inst.init(),
            None => {
                GPU_INSTANCE = Some(GpuInstance::new(R9_280X_DEVICE_ID));
                GPU_INSTANCE.as_mut().unwrap().init()
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_driver_detected() -> bool {
    unsafe {
        match &GPU_INSTANCE {
            Some(inst) => inst.detect(),
            None => false,
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_enable_3d_acceleration() {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.enable_3d_acceleration();
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_draw_arrays(primitive: u32, first: u32, count: u32) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.draw_arrays(primitive, first, count);
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_draw_elements(primitive: u32, index_count: u32, index_format: u32, index_addr: u64) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.draw_elements(primitive, index_count, index_format, index_addr);
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_clear(color: u32, depth: u32, stencil: u32) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.clear(color, depth, stencil);
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_set_viewport(x: f32, y: f32, width: f32, height: f32, min_depth: f32, max_depth: f32) {
    unsafe {
        if let Some(inst) = &mut GPU_INSTANCE {
            inst.set_viewport(&Viewport { x, y, width, height, min_depth, max_depth });
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_get_status() -> u32 {
    unsafe {
        match &GPU_INSTANCE {
            Some(inst) => inst.error_count(),
            None => 0,
        }
    }
}

#[no_mangle]
pub extern "C" fn r9_280x_is_ready() -> bool {
    gpu_driver_is_ready()
}
