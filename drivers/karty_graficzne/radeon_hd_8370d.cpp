/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/karty_graficzne/radeon_hd_8370d.h>

/*funkcje io portow*/

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

/*odczyt i zapis konfiguracji PCI*/

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                        uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

/*rejestry MMIO AMD Radeon HD 8370D (GCN Southern Islands)*/

enum radeon_hd_8370d_reg {
    /*PCI BAR0 MMIO bazowy*/
    /*Glowna kontrolka GPU*/
    REG_CP_ME_CNTL      = 0x08BCU,   /*Command Processor ME control*/
    REG_CP_ME_RB_ADDR   = 0x08C4U,   /*Command Processor ME ring buffer address*/
    REG_CP_ME_RB_SIZE   = 0x08C8U,   /*Command Processor ME ring buffer size*/
    REG_CP_ME_RB_WPTR   = 0x08CCU,   /*Command Processor ME ring buffer write pointer*/
    REG_CP_ME_RB_RPTR   = 0x08D0U,   /*Command Processor ME ring buffer read pointer*/
    REG_CP_ME_RB_CNTL   = 0x08D4U,   /*Command Processor ME ring buffer control*/

    /*Display engine*/
    REG_DC_GATE_DIS_CTRL0 = 0x0F00U, /*Display clock gating*/
    REG_CRTC0_CONTROL   = 0x6800U,   /*CRTC0 control*/
    REG_CRTC0_H_TOTAL   = 0x6804U,   /*CRTC0 horizontal total*/
    REG_CRTC0_V_TOTAL   = 0x6808U,   /*CRTC0 vertical total*/
    REG_CRTC0_H_BLANK   = 0x680CU,   /*CRTC0 horizontal blank*/
    REG_CRTC0_V_BLANK   = 0x6810U,   /*CRTC0 vertical blank*/
    REG_CRTC0_H_SYNC_A  = 0x6814U,   /*CRTC0 horizontal sync A*/
    REG_CRTC0_V_SYNC_A  = 0x6818U,   /*CRTC0 vertical sync A*/
    REG_CRTC0_PRIMARY_VIEWPORT_START = 0x6820U,
    REG_CRTC0_PRIMARY_VIEWPORT_END   = 0x6824U,
    REG_CRTC0_PRIMARY_SURFACE_ADDRESS = 0x6830U,
    REG_CRTC0_PRIMARY_SURFACE_ADDRESS_HIGH = 0x6834U,
    REG_CRTC0_PRIMARY_SURFACE_OFFSET_X = 0x6838U,
    REG_CRTC0_PRIMARY_SURFACE_OFFSET_Y = 0x683C,
    REG_CRTC0_PRIMARY_SURFACE_PIXEL_PITCH = 0x6844U,
    REG_CRTC0_PRIMARY_SURFACE_TILE = 0x684CU,

    /*Plane / surfaces*/
    REG_PRIMARY_PLANE_OFFSET_X = 0x6900U,
    REG_PRIMARY_PLANE_OFFSET_Y = 0x6904U,
    REG_PRIMARY_PLANE_PIXEL_PITCH = 0x6908U,
    REG_PRIMARY_PLANE_SURFACE_ADDRESS = 0x690CU,
    REG_PRIMARY_PLANE_SURFACE_ADDRESS_HIGH = 0x6910U,

    /*Color buffer (render target) - CB*/
    REG_CB_TARGET0_BASE = 0x8800U,   /*Color buffer target 0 base address*/
    REG_CB_TARGET0_PITCH = 0x8804U,  /*Color buffer target 0 pitch*/
    REG_CB_TARGET0_SLICE = 0x8808U,  /*Color buffer target 0 slice*/
    REG_CB_TARGET0_FMASK = 0x880CU,  /*Color buffer target 0 fragment mask*/
    REG_CB_TARGET0_VIEW = 0x8810U,   /*Color buffer target 0 view*/
    REG_CB_SHADER_MASK = 0x88F0U,    /*Color buffer shader mask*/
    REG_CB_CLEAR_COLOR = 0x88F4U,    /*Color buffer clear color*/
    REG_CB_CLEAR_COLOR_HIGH = 0x88F8U,

    /*Depth buffer - DB*/
    REG_DB_Z_INFO = 0x8A00U,         /*Depth buffer info*/
    REG_DB_Z_READ_BASE = 0x8A04U,    /*Depth buffer read base*/
    REG_DB_Z_READ_BASE_HIGH = 0x8A08U,
    REG_DB_Z_WRITE_BASE = 0x8A0CU,   /*Depth buffer write base*/
    REG_DB_Z_WRITE_BASE_HIGH = 0x8A10U,
    REG_DB_Z_READ_OFF = 0x8A14U,     /*Depth buffer read offset*/
    REG_DB_Z_WRITE_OFF = 0x8A18U,    /*Depth buffer write offset*/
    REG_DB_CLEAR = 0x8A20U,          /*Depth buffer clear*/
    REG_DB_STENCIL_CLEAR = 0x8A24U,

    /*Alpha blending - CB_BLEND*/
    REG_CB_BLEND_CONTROL = 0x8A50U,  /*Color buffer blend control*/
    REG_CB_BLEND_RED = 0x8A54U,      /*Blend red channel*/
    REG_CB_BLEND_GREEN = 0x8A58U,    /*Blend green channel*/
    REG_CB_BLEND_BLUE = 0x8A5CU,     /*Blend blue channel*/
    REG_CB_BLEND_ALPHA = 0x8A60U,    /*Blend alpha channel*/

    /*Shader registers - SQ (Shader Quad)*/
    REG_SQ_CONFIG = 0x9800U,         /*Shader config*/
    REG_SQ_VTX_BASE = 0x9810U,       /*Vertex buffer base*/
    REG_SQ_VTX_STRIDE = 0x9814U,     /*Vertex buffer stride*/
    REG_SQ_VTX_NUM_FORMAT = 0x9818U, /*Vertex number format*/
    REG_SQ_VTX_FMT_0 = 0x9820U,      /*Vertex format 0*/
    REG_SQ_VTX_FMT_1 = 0x9824U,
    REG_SQ_VTX_FMT_2 = 0x9828U,
    REG_SQ_VTX_FMT_3 = 0x982CU,
    REG_SQ_PROGRAM = 0x9860U,        /*Shader program*/
    REG_SQ_CONSTANT_BASE = 0x98A0U,  /*Shader constant base*/
    REG_SQ_CONSTANT_CACHE = 0x98A4U,

    /*Texture / sampler*/
    REG_SQ_TEX_BASE = 0x9C00U,       /*Texture base*/
    REG_SQ_SAMPLER_BASE = 0x9C80U,   /*Sampler base*/
    REG_SQ_TEX_LOD = 0x9D00U,        /*Texture LOD*/
    REG_SQ_TEX_DIM = 0x9D80U,        /*Texture dimensions*/

    /*Rasterizer - PA (Primitive Assembly)*/
    REG_PA_SU_VTX_CNTL = 0xCA00U,    /*PA SU vertex control*/
    REG_PA_CL_VTE_CNTL = 0xCA04U,    /*PA CL vertex cache control*/
    REG_PA_CL_VS_OUT_CNTL = 0xCA08U, /*PA CL VS output control*/
    REG_PA_CL_GB_ADV = 0xCA0CU,      /*PA CL GB advance*/
    REG_PA_CL_VTE_VTX_0 = 0xCA10U,   /*PA CL VTE vertex 0*/
    REG_PA_SU_POLY_OFFSET_FRONT = 0xCA30U,
    REG_PA_SU_POLY_OFFSET_BACK = 0xCA34U,
    REG_PA_CL_CLIP_CNTL = 0xCA40U,   /*PA CL clip control*/
    REG_PA_SU_SC_MODE_CNTL = 0xCA44U,/*PA SU scissor mode control*/
    REG_PA_SC_WINDOW_OFFSET = 0xCA48U,/*PA SC window offset*/
    REG_PA_SC_WINDOW_SCISSOR_TL = 0xCA4CU,/*PA SC scissor top-left*/
    REG_PA_SC_WINDOW_SCISSOR_BR = 0xCA50U,/*PA SC scissor bottom-right*/
    REG_PA_SU_PRIM_FILTER_CNTL = 0xCA54U,/*PA SU primitive filter control*/
    REG_PA_SU_LINE_STIPPLE = 0xCA58U,/*PA SU line stipple*/
    REG_PA_SC_AA_CONFIG = 0xCA5CU,   /*PA SC anti-alias config*/

    /*Alpha to coverage / transparency*/
    REG_CB_ALPHA_TO_MASK = 0xCA60U,  /*Alpha to coverage mask*/
    REG_CB_ALPHA_TO_MASK_DWORD0 = 0xCA64U,
    REG_CB_ALPHA_TO_MASK_DWORD1 = 0xCA68U,
    REG_CB_ALPHA_TO_MASK_DWORD2 = 0xCA6CU,
    REG_CB_ALPHA_TO_MASK_DWORD3 = 0xCA70U,

    /*Cursor*/
    REG_CURSOR_CONTROL = 0x7000U,    /*Cursor control*/
    REG_CURSOR_POSITION = 0x7004U,   /*Cursor position*/
    REG_CURSOR_COLOR0 = 0x7008U,     /*Cursor color 0*/
    REG_CURSOR_COLOR1 = 0x700CU,     /*Cursor color 1*/
    REG_CURSOR_BASE = 0x7010U,       /*Cursor image base address*/

    /*Interrupts*/
    REG_INT_STAT = 0x0100U,          /*Interrupt status*/
    REG_INT_MASK = 0x0104U,          /*Interrupt mask*/
    REG_INT_ACK = 0x0108U,           /*Interrupt acknowledge*/

    /*Compute*/
    REG_COMPUTE_PGM_LO = 0x8000U,    /*Compute program low*/
    REG_COMPUTE_PGM_HI = 0x8004U,    /*Compute program high*/
    REG_COMPUTE_NUM_THREAD_X = 0x8008U,/*Compute num thread X*/
    REG_COMPUTE_NUM_THREAD_Y = 0x800CU,
    REG_COMPUTE_NUM_THREAD_Z = 0x8010U,
};

/*bity CP_ME_CNTL*/

#define CP_ME_CNTL_ME_HALT      (1U << 0)
#define CP_ME_CNTL_ME_RESET     (1U << 1)
#define CP_ME_CNTL_PFP_HALT     (1U << 8)
#define CP_ME_CNTL_PFP_RESET    (1U << 9)

/*bity CP_ME_RB_CNTL*/

#define CP_ME_RB_CNTL_RB_ENABLE (1U << 0)
#define CP_ME_RB_CNTL_RB_NO_UPDATE (1U << 1)

/*bity CRTC0_CONTROL*/

#define CRTC_CONTROL_EN         (1U << 0)
#define CRTC_CONTROL_PIXEL_WIDTH_32BPP (3U << 2)

/*bity CB_BLEND_CONTROL (przezroczystosc alpha blending)*/

#define BLEND_CONTROL_SRC_BLEND_SHIFT     0U
#define BLEND_CONTROL_DEST_BLEND_SHIFT    8U
#define BLEND_CONTROL_BLEND_OP_SHIFT      16U
#define BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT 20U
#define BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT 24U
#define BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT 28U
#define BLEND_CONTROL_ROP_ENABLE          (1U << 31U)

/*bity CB_SHADER_MASK*/

#define SHADER_MASK_ENABLE                (1U << 0)
#define SHADER_MASK_CB0_ENABLE            (1U << 4)

/*bity DB_Z_INFO (depth buffer info)*/

#define DB_Z_INFO_FORMAT_SHIFT            0U
#define DB_Z_INFO_FORMAT_16BIT            0U
#define DB_Z_INFO_FORMAT_24BIT            1U
#define DB_Z_INFO_FORMAT_32BIT            2U

/*id urzadzen AMD Radeon HD 8370D (GCN Southern Islands / Richland)*/

#define AMD_VENDOR_ID   0x1002U
#define DEVICE_R8370D_1 0x9907U   /*Radeon HD 8370D (Richland)*/
#define DEVICE_R8370D_2 0x9908U   /*Radeon HD 8370D (mobile)*/
#define DEVICE_R8370D_3 0x9909U   /*Radeon HD 8370D variant*/

#define FB_ADDR  0xFE000000U
#define FB_WIDTH  1024U
#define FB_HEIGHT 768U
#define FB_PITCH  4096U
#define RING_SIZE 8192U

/*stan sterownika*/

static volatile uint32_t *g_mmio = 0;
static volatile uint32_t *g_fb = (volatile uint32_t *)FB_ADDR;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint16_t g_device_id = 0;
static uint8_t g_revision = 0;
static int g_initialized = 0;
static uint32_t g_ring[RING_SIZE / 4U];
static uint32_t g_ring_head = 0;
static uint32_t g_ring_tail = 0;
static int g_cursor_visible = 0;
static uint8_t g_gcn_gen = 3U;   /*GCN 3.0 (Southern Islands)*/
static uint8_t g_alpha_blend_global = 1U;

/*czytanie i zapis MMIO (posted write dla stabilnosci)*/

static inline uint32_t mmio_read32(uint32_t offset)
{
    return g_mmio[offset >> 2];
}

static inline void mmio_write32(uint32_t offset, uint32_t value)
{
    g_mmio[offset >> 2] = value;
}

static inline void mmio_write32_posted(uint32_t offset, uint32_t value)
{
    mmio_write32(offset, value);
    (void)mmio_read32(offset);
}

/*opoznienie mikrosekundowe (obrotowa petla CPU)*/

static void radeon_hd_8370d_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (int j = 0; j < 40; j++);
    }
}

/*wyszukanie urzadzenia AMD Radeon HD 8370D na PCI*/

static int radeon_hd_8370d_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != AMD_VENDOR_ID) continue;
                if (device != DEVICE_R8370D_1 && device != DEVICE_R8370D_2 &&
                    device != DEVICE_R8370D_3) continue;

                uint32_t bar0 = pci_read32(bus, slot, func, 0x10);
                if (bar0 == 0 || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;

                uint32_t command = pci_read32(bus, slot, func, 0x04);
                pci_write32(bus, slot, func, 0x04, command | 0x00000006);

                g_bus = bus;
                g_slot = slot;
                g_func = func;
                g_mmio_base = bar0 & 0xFFFFFFF0U;
                g_mmio = (volatile uint32_t *)g_mmio_base;
                g_device_id = device;
                g_revision = (uint8_t)(pci_read32(bus, slot, func, 0x08) & 0xFFU);
                return 1;
            }
        }
    }
    return 0;
}

/*wyczysczenie przerwan GPU*/

static void radeon_hd_8370d_clear_interrupts(void)
{
    mmio_write32(REG_INT_STAT, 0xFFFFFFFFU);
    mmio_write32(REG_INT_MASK, 0x00000000U);
}

/*reset Command Processor ME i PFP*/

static void radeon_hd_8370d_reset_cp(void)
{
    mmio_write32(REG_CP_ME_CNTL, CP_ME_CNTL_ME_RESET | CP_ME_CNTL_PFP_RESET);
    radeon_hd_8370d_delay_us(100);
    mmio_write32(REG_CP_ME_CNTL, 0U);
    radeon_hd_8370d_delay_us(100);
}

/*inicjalizacja ring buffer Command Processor*/

static void radeon_hd_8370d_cp_ring_init(void)
{
    mmio_write32(REG_CP_ME_RB_ADDR, (uint32_t)&g_ring[0]);
    mmio_write32(REG_CP_ME_RB_SIZE, sizeof(g_ring));
    mmio_write32(REG_CP_ME_RB_WPTR, 0U);
    mmio_write32(REG_CP_ME_RB_RPTR, 0U);
    mmio_write32(REG_CP_ME_RB_CNTL, CP_ME_RB_CNTL_RB_ENABLE);
    g_ring_head = 0;
    g_ring_tail = 0;
}

/*wyslanie polecenia do ring buffer CP*/

static void radeon_hd_8370d_cp_flush(void);
static void radeon_hd_8370d_cp_emit(uint32_t dword)
{
    if (((g_ring_head & (RING_SIZE / 4U - 1U)) + 5U) > (RING_SIZE / 4U)) {
        radeon_hd_8370d_cp_flush();
    }
    g_ring[g_ring_head & (RING_SIZE / 4U - 1U)] = dword;
    g_ring_head++;
}

/*wyczyscenie ring buffer CP i oczekiwanie na wykonanie*/

static void radeon_hd_8370d_cp_flush(void)
{
    uint32_t tail = g_ring_head & (RING_SIZE / 4U - 1U);
    mmio_write32(REG_CP_ME_RB_WPTR, tail);
    for (uint32_t wait = 0; wait < 1000000U; wait++) {
        if (mmio_read32(REG_CP_ME_RB_RPTR) == tail) {
            break;
        }
    }
    g_ring_tail = tail;
}

/*inicjalizacja silnika wyswietlania (CRTC, plane, surface)*/

static void radeon_hd_8370d_display_init(void)
{
    /*wlacz zegary display, wylacz gating*/
    mmio_write32(REG_DC_GATE_DIS_CTRL0, 0xFFFFFFFFU);

    /*konfiguracja CRTC0 dla 1024x768*/
    uint32_t hblank = (((FB_WIDTH + 128U) - 1U) << 16) | (FB_WIDTH - 1U);
    uint32_t vblank = (((FB_HEIGHT + 32U) - 1U) << 16) | (FB_HEIGHT - 1U);
    uint32_t hsync = (((FB_WIDTH + 64U) - 1U) << 16) | (FB_WIDTH - 1U);
    uint32_t vsync = (((FB_HEIGHT + 4U) - 1U) << 16) | (FB_HEIGHT - 1U);

    mmio_write32(REG_CRTC0_H_TOTAL, hblank);
    mmio_write32(REG_CRTC0_V_TOTAL, vblank);
    mmio_write32(REG_CRTC0_H_BLANK, hblank);
    mmio_write32(REG_CRTC0_V_BLANK, vblank);
    mmio_write32(REG_CRTC0_H_SYNC_A, hsync);
    mmio_write32(REG_CRTC0_V_SYNC_A, vsync);

    mmio_write32(REG_CRTC0_PRIMARY_VIEWPORT_START, 0U);
    mmio_write32(REG_CRTC0_PRIMARY_VIEWPORT_END,
                 ((FB_HEIGHT - 1U) << 16) | (FB_WIDTH - 1U));

    mmio_write32(REG_CRTC0_PRIMARY_SURFACE_ADDRESS, FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_CRTC0_PRIMARY_SURFACE_ADDRESS_HIGH, 0U);
    mmio_write32(REG_CRTC0_PRIMARY_SURFACE_OFFSET_X, 0U);
    mmio_write32(REG_CRTC0_PRIMARY_SURFACE_OFFSET_Y, 0U);
    mmio_write32(REG_CRTC0_PRIMARY_SURFACE_PIXEL_PITCH, FB_PITCH);
    mmio_write32(REG_CRTC0_PRIMARY_SURFACE_TILE, 0U);

    /*plane primary*/
    mmio_write32(REG_PRIMARY_PLANE_OFFSET_X, 0U);
    mmio_write32(REG_PRIMARY_PLANE_OFFSET_Y, 0U);
    mmio_write32(REG_PRIMARY_PLANE_PIXEL_PITCH, FB_PITCH);
    mmio_write32(REG_PRIMARY_PLANE_SURFACE_ADDRESS, FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_PRIMARY_PLANE_SURFACE_ADDRESS_HIGH, 0U);

    /*wlacz CRTC0*/
    mmio_write32(REG_CRTC0_CONTROL, CRTC_CONTROL_EN | CRTC_CONTROL_PIXEL_WIDTH_32BPP);
    radeon_hd_8370d_delay_us(200);
}

/*inicjalizacja potoku 3D GCN (color buffer, depth buffer, blend, rasterizer)*/

static void radeon_hd_8370d_3d_pipeline_init(void)
{
    /*wyczysc CP i ring buffer*/
    radeon_hd_8370d_reset_cp();
    radeon_hd_8370d_cp_ring_init();

    /*konfiguracja color buffer target 0 (ramka glowna)*/
    mmio_write32(REG_CB_TARGET0_BASE, FB_ADDR & 0xFFFFFFFFU);
    mmio_write32(REG_CB_TARGET0_PITCH, FB_PITCH);
    mmio_write32(REG_CB_TARGET0_SLICE, 0U);
    mmio_write32(REG_CB_TARGET0_FMASK, 0U);
    mmio_write32(REG_CB_TARGET0_VIEW, 0U);

    /*maska shader - wlaczenie celow koloru*/
    mmio_write32(REG_CB_SHADER_MASK, SHADER_MASK_ENABLE | SHADER_MASK_CB0_ENABLE);

    /*kolor czyszczenia*/
    mmio_write32(REG_CB_CLEAR_COLOR, 0x00000000U);
    mmio_write32(REG_CB_CLEAR_COLOR_HIGH, 0x00000000U);

    /*blend control - domyslne alpha blending (przezroczystosc)*/
    /*src = SRC_ALPHA, dest = INV_SRC_ALPHA, op = ADD*/
    uint32_t blend = (RADEON_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_SHIFT) |
                     (RADEON_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_SHIFT) |
                     (RADEON_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_SHIFT) |
                     (RADEON_BLEND_SRC_ALPHA << BLEND_CONTROL_SRC_BLEND_ALPHA_SHIFT) |
                     (RADEON_BLEND_INV_SRC_ALPHA << BLEND_CONTROL_DEST_BLEND_ALPHA_SHIFT) |
                     (RADEON_BLEND_OP_ADD << BLEND_CONTROL_BLEND_OP_ALPHA_SHIFT);
    mmio_write32(REG_CB_BLEND_CONTROL, blend);

    /*konfiguracja depth buffer*/
    mmio_write32(REG_DB_Z_INFO, DB_Z_INFO_FORMAT_24BIT);
    mmio_write32(REG_DB_Z_READ_BASE, (FB_ADDR + 0x1000000U) & 0xFFFFFFFFU);
    mmio_write32(REG_DB_Z_READ_BASE_HIGH, 0U);
    mmio_write32(REG_DB_Z_WRITE_BASE, (FB_ADDR + 0x1000000U) & 0xFFFFFFFFU);
    mmio_write32(REG_DB_Z_WRITE_BASE_HIGH, 0U);
    mmio_write32(REG_DB_Z_READ_OFF, 0U);
    mmio_write32(REG_DB_Z_WRITE_OFF, 0U);
    mmio_write32(REG_DB_CLEAR, 0x00FFFFFFU);   /*glębokosc 1.0*/
    mmio_write32(REG_DB_STENCIL_CLEAR, 0U);

    /*konfiguracja rasterizera PA*/
    mmio_write32(REG_PA_SU_VTX_CNTL, 0U);
    mmio_write32(REG_PA_CL_VTE_CNTL, 0U);
    mmio_write32(REG_PA_CL_VS_OUT_CNTL, 0U);
    mmio_write32(REG_PA_CL_GB_ADV, 0U);
    mmio_write32(REG_PA_CL_CLIP_CNTL, 0U);
    mmio_write32(REG_PA_SU_SC_MODE_CNTL, 0U);
    mmio_write32(REG_PA_SC_WINDOW_OFFSET, 0U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_TL, 0U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_BR,
                 ((FB_HEIGHT - 1U) << 16) | (FB_WIDTH - 1U));
    mmio_write32(REG_PA_SU_PRIM_FILTER_CNTL, 0U);
    mmio_write32(REG_PA_SU_LINE_STIPPLE, 0U);
    mmio_write32(REG_PA_SC_AA_CONFIG, 0U);

    /*alpha to coverage dla przezroczystosci*/
    mmio_write32(REG_CB_ALPHA_TO_MASK, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD0, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD1, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD2, 0U);
    mmio_write32(REG_CB_ALPHA_TO_MASK_DWORD3, 0U);

    /*konfiguracja shader SQ*/
    mmio_write32(REG_SQ_CONFIG, 0U);
    mmio_write32(REG_SQ_VTX_BASE, 0U);
    mmio_write32(REG_SQ_VTX_STRIDE, 0U);
    mmio_write32(REG_SQ_VTX_NUM_FORMAT, 0U);
    mmio_write32(REG_SQ_VTX_FMT_0, 0U);
    mmio_write32(REG_SQ_VTX_FMT_1, 0U);
    mmio_write32(REG_SQ_VTX_FMT_2, 0U);
    mmio_write32(REG_SQ_VTX_FMT_3, 0U);

    /*konfiguracja tekstury / sampler*/
    mmio_write32(REG_SQ_TEX_BASE, 0U);
    mmio_write32(REG_SQ_SAMPLER_BASE, 0U);
    mmio_write32(REG_SQ_TEX_LOD, 0U);
    mmio_write32(REG_SQ_TEX_DIM, 0U);

    radeon_hd_8370d_delay_us(200);
}

/*inicjalizacja kursora sprzetowego*/

static void radeon_hd_8370d_cursor_init(void)
{
    mmio_write32(REG_CURSOR_CONTROL, 0U);   /*ukryty domyslnie*/
    mmio_write32(REG_CURSOR_POSITION, 0U);
    mmio_write32(REG_CURSOR_COLOR0, 0x00000000U);
    mmio_write32(REG_CURSOR_COLOR1, 0x00FFFFFFU);
    mmio_write32(REG_CURSOR_BASE, FB_ADDR);
    g_cursor_visible = 0;
}

/*inicjalizacja sterownika AMD Radeon HD 8370D (2D + 3D GCN z przezroczystoscia)*/

int radeon_hd_8370d_init(void)
{
    if (g_initialized) return 1;
    if (!radeon_hd_8370d_find_device()) return 0;

    uint32_t command = pci_read32(g_bus, g_slot, g_func, 0x04);
    pci_write32(g_bus, g_slot, g_func, 0x04, command | 0x00000006U);

    radeon_hd_8370d_clear_interrupts();

    radeon_hd_8370d_display_init();

    radeon_hd_8370d_3d_pipeline_init();

    radeon_hd_8370d_cursor_init();

    g_initialized = 1;
    return 1;
}

/*czy sterownik jest zainicjowany i gotowy*/

int radeon_hd_8370d_is_ready(void)
{
    return g_initialized;
}

/*rysowanie pojedynczego piksela w ramce graficznej (z obsuga przezroczystosci alpha)*/

void radeon_hd_8370d_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= FB_WIDTH || y >= FB_HEIGHT) return;
    g_fb[y * (FB_PITCH / 4U) + x] = color;
}

/*wypelnienie prostokata (uzywa CP ring buffer gdy dostepny, fallback CPU)*/

void radeon_hd_8370d_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (g_initialized) {
        /*emit polecenie FILL_RECT do ring buffer CP*/
        radeon_hd_8370d_cp_emit(0x40000000U | 0x01U);   /*opcode FILL_RECT*/
        radeon_hd_8370d_cp_emit((y << 16) | x);
        radeon_hd_8370d_cp_emit(((h - 1U) << 16) | (w - 1U));
        radeon_hd_8370d_cp_emit(color);
        radeon_hd_8370d_cp_emit(0U);   /*reserved*/
        radeon_hd_8370d_cp_flush();
        return;
    }
    /*fallback - petla CPU*/
    uint32_t xx;
    uint32_t yy;
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= FB_WIDTH || yy >= FB_HEIGHT) continue;
            g_fb[yy * (FB_PITCH / 4U) + xx] = color;
        }
    }
}

/*kopiowanie bloku pikseli (BLT copy) - uzywa CP ring buffer sprzetowego*/

void radeon_hd_8370d_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                         uint32_t w, uint32_t h)
{
    if (!g_initialized) return;

    radeon_hd_8370d_cp_emit(0x40000000U | 0x02U);   /*opcode BLT_COPY*/
    radeon_hd_8370d_cp_emit((dst_y << 16) | dst_x);
    radeon_hd_8370d_cp_emit(((h - 1U) << 16) | (w - 1U));
    radeon_hd_8370d_cp_emit((src_y << 16) | src_x);
    radeon_hd_8370d_cp_emit(0U);
    radeon_hd_8370d_cp_flush();
}

/*ustawienie kursora sprhetowego na pozycji (x, y) z podanym obrazem 64x64 (przezroczysty)*/

void radeon_hd_8370d_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    if (!g_initialized) return;

    if (image) {
        /*kopiuj obraz kursora do pamieci ramki graficznej (kurzor zaczyna od 0xFE000000)*/
        for (uint32_t i = 0; i < 64U * 64U; i++) {
            g_fb[i] = image[i];
        }
        mmio_write32(REG_CURSOR_BASE, FB_ADDR);
    }

    mmio_write32(REG_CURSOR_POSITION, (y << 16) | x);
    mmio_write32(REG_CURSOR_CONTROL, 0x80000000U);   /*wlacz kursor*/
    g_cursor_visible = 1;
}

/*ukrycie kursora sprhetowego*/

void radeon_hd_8370d_cursor_hide(void)
{
    if (!g_initialized) return;
    mmio_write32(REG_CURSOR_CONTROL, 0U);
    g_cursor_visible = 0;
}

/*pobranie wskaznika na ramke graficzna*/

void *radeon_hd_8370d_get_fb(void)
{
    return (void *)g_fb;
}

/*pobranie szerokosci ekranu*/

uint32_t radeon_hd_8370d_get_width(void)
{
    return FB_WIDTH;
}

/*pobranie wysokosci ekranu*/

uint32_t radeon_hd_8370d_get_height(void)
{
    return FB_HEIGHT;
}

/*pobranie id urzadzenia*/

uint16_t radeon_hd_8370d_get_device_id(void)
{
    return g_device_id;
}

/*pobranie revisji urzadzenia*/

uint8_t radeon_hd_8370d_get_revision(void)
{
    return g_revision;
}

/*pobranie informacji o urzadzeniu*/

void radeon_hd_8370d_info_get(struct radeon_hd_8370d_info *out)
{
    if (!out) return;
    out->vendor_id = AMD_VENDOR_ID;
    out->device_id = g_device_id;
    out->revision_id = g_revision;
    out->mmio_base = g_mmio_base;
    out->fb_addr = FB_ADDR;
    out->fb_width = FB_WIDTH;
    out->fb_height = FB_HEIGHT;
    out->fb_pitch = FB_PITCH;
    out->initialized = (uint8_t)g_initialized;
    out->gcn_generation = g_gcn_gen;
}

/*================== ZASOBY 3D GCN - STATYCZNE TABLICE ==================*/

static struct radeon_hd_8370d_vertex_buffer g_vb[RADEON_MAX_VERTEX_BUFFERS];
static struct radeon_hd_8370d_index_buffer g_ib[RADEON_MAX_INDEX_BUFFERS];
static struct radeon_hd_8370d_texture g_tex[RADEON_MAX_TEXTURES];
static struct radeon_hd_8370d_render_target g_rt[RADEON_MAX_RENDER_TARGETS];
static struct radeon_hd_8370d_depth_stencil g_ds[RADEON_MAX_DEPTH_STENCILS];
static struct radeon_hd_8370d_shader g_shader[RADEON_MAX_SHADERS];
static struct radeon_hd_8370d_sampler_state g_sampler[RADEON_MAX_SAMPLERS];
static struct radeon_hd_8370d_blend_state g_blend[RADEON_MAX_BLEND_STATES];
static struct radeon_hd_8370d_rasterizer_state g_raster[RADEON_MAX_RASTERIZER_STATES];
static struct radeon_hd_8370d_depth_stencil_state g_ds_state[RADEON_MAX_DEPTH_STENCIL_STATES];
static struct radeon_hd_8370d_input_layout g_il[RADEON_MAX_INPUT_LAYOUTS];
static struct radeon_hd_8370d_constant_buffer g_cb[RADEON_MAX_CONSTANT_BUFFERS];
static struct radeon_hd_8370d_swap_chain g_swap[1];

static uint8_t g_cmd_recording = 0;
static uint32_t g_cmd_buf[RADEON_COMMAND_BUFFER_SIZE];
static uint32_t g_cmd_count = 0;

/*tablice uzycia zasobow (definiowane tutaj przed funkcjami je uzywajacymi)*/
static uint8_t g_vb_used[RADEON_MAX_VERTEX_BUFFERS] = {0};
static uint8_t g_ib_used[RADEON_MAX_INDEX_BUFFERS] = {0};
static uint8_t g_tex_used[RADEON_MAX_TEXTURES] = {0};
static uint8_t g_rt_used[RADEON_MAX_RENDER_TARGETS] = {0};
static uint8_t g_ds_used[RADEON_MAX_DEPTH_STENCILS] = {0};
static uint8_t g_shader_used[RADEON_MAX_SHADERS] = {0};
static uint8_t g_sampler_used[RADEON_MAX_SAMPLERS] = {0};
static uint8_t g_blend_used[RADEON_MAX_BLEND_STATES] = {0};
static uint8_t g_raster_used[RADEON_MAX_RASTERIZER_STATES] = {0};
static uint8_t g_ds_state_used[RADEON_MAX_DEPTH_STENCIL_STATES] = {0};
static uint8_t g_il_used[RADEON_MAX_INPUT_LAYOUTS] = {0};
static uint8_t g_cb_used[RADEON_MAX_CONSTANT_BUFFERS] = {0};

/*kontekst 3D GCN*/
static struct radeon_hd_8370d_context g_ctx = {};

/*Pomocnicza funkcja - znajdz wolny slot w tablicy zasobow*/
static uint32_t find_free_slot(uint8_t *used, uint32_t max)
{
    for (uint32_t i = 0; i < max; i++) {
        if (!used[i]) return i;
    }
    return RADEON_HANDLE_NULL;
}

/*Pomocnicza funkcja - walidacja handle*/
static int handle_valid(radeon_handle_t h, uint8_t *used, uint32_t max)
{
    return h != RADEON_HANDLE_NULL && h < max && used[h];
}

/*================== FUNKCJE ZASOBOW 3D ==================*/

radeon_handle_t radeon_hd_8370d_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_vb_used, RADEON_MAX_VERTEX_BUFFERS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_vertex_buffer *vb = &g_vb[i];
    vb->used = 1;
    vb->vertex_count = vertex_count;
    vb->vertex_size = vertex_size;
    vb->format = format;
    vb->buffer_addr = FB_ADDR + 0x2000000U + i * 0x100000U;
    vb->system_mem = 0;

    if (data) {
        vb->system_mem = (uint8_t *)vb->buffer_addr;
        for (uint32_t k = 0; k < vertex_count * vertex_size; k++) {
            vb->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_vertex_buffer_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_vb_used, RADEON_MAX_VERTEX_BUFFERS)) return;
    g_vb[handle].used = 0;
}

void radeon_hd_8370d_vertex_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_vb_used, RADEON_MAX_VERTEX_BUFFERS)) return;
    struct radeon_hd_8370d_vertex_buffer *vb = &g_vb[handle];
    if (!vb->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        vb->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_handle_t radeon_hd_8370d_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_ib_used, RADEON_MAX_INDEX_BUFFERS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_index_buffer *ib = &g_ib[i];
    ib->used = 1;
    ib->index_count = index_count;
    ib->format = format;
    ib->buffer_addr = FB_ADDR + 0x4000000U + i * 0x100000U;
    ib->system_mem = 0;

    if (data) {
        ib->system_mem = (uint8_t *)ib->buffer_addr;
        uint32_t elem_size = (format == RADEON_INDEX_FORMAT_32BIT) ? 4U : 2U;
        for (uint32_t k = 0; k < index_count * elem_size; k++) {
            ib->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_index_buffer_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_ib_used, RADEON_MAX_INDEX_BUFFERS)) return;
    g_ib[handle].used = 0;
}

void radeon_hd_8370d_index_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_ib_used, RADEON_MAX_INDEX_BUFFERS)) return;
    struct radeon_hd_8370d_index_buffer *ib = &g_ib[handle];
    if (!ib->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        ib->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

radeon_handle_t radeon_hd_8370d_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
{
    uint32_t i = find_free_slot(g_tex_used, RADEON_MAX_TEXTURES);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_texture *t = &g_tex[i];
    t->used = 1;
    t->width = width;
    t->height = height;
    t->format = format;
    t->mip_levels = mip_levels;
    t->pitch = ((width * 4U + 3U) & ~3U);
    t->buffer_addr = FB_ADDR + 0x6000000U + i * 0x200000U;
    t->system_mem = 0;

    if (data) {
        t->system_mem = (uint8_t *)t->buffer_addr;
        uint32_t bpp = (format == RADEON_TEX_FORMAT_DXT1) ? 1U : 2U;
        for (uint32_t k = 0; k < width * height * bpp; k++) {
            t->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_texture_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, RADEON_MAX_TEXTURES)) return;
    g_tex[handle].used = 0;
}

void radeon_hd_8370d_texture_update(radeon_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
{
    (void)level;
    if (!handle_valid(handle, g_tex_used, RADEON_MAX_TEXTURES)) return;
    struct radeon_hd_8370d_texture *t = &g_tex[handle];
    if (!t->system_mem) return;
    for (uint32_t yy = 0; yy < h; yy++) {
        for (uint32_t xx = 0; xx < w; xx++) {
            uint32_t src = (yy * w + xx) * 4U;
            uint32_t dst = ((y + yy) * t->pitch + (x + xx)) * 4U;
            t->system_mem[dst] = ((const uint8_t *)data)[src];
            t->system_mem[dst + 1] = ((const uint8_t *)data)[src + 1];
            t->system_mem[dst + 2] = ((const uint8_t *)data)[src + 2];
            t->system_mem[dst + 3] = ((const uint8_t *)data)[src + 3];
        }
    }
}

void radeon_hd_8370d_texture_generate_mipmaps(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, RADEON_MAX_TEXTURES)) return;
    struct radeon_hd_8370d_texture *t = &g_tex[handle];
    if (!t->system_mem) return;
    uint32_t w = t->width;
    uint32_t h = t->height;
    for (uint32_t level = 1; level < t->mip_levels && w > 1 && h > 1; level++) {
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
    }
}

radeon_handle_t radeon_hd_8370d_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend)
{
    uint32_t i = find_free_slot(g_rt_used, RADEON_MAX_RENDER_TARGETS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_render_target *rt = &g_rt[i];
    rt->used = 1;
    rt->width = width;
    rt->height = height;
    rt->format = format;
    rt->buffer_addr = FB_ADDR + 0x8000000U + i * 0x100000U;
    rt->alpha_blend = alpha_blend;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_render_target_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_rt_used, RADEON_MAX_RENDER_TARGETS)) return;
    g_rt[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format)
{
    uint32_t i = find_free_slot(g_ds_used, RADEON_MAX_DEPTH_STENCILS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_depth_stencil *ds = &g_ds[i];
    ds->used = 1;
    ds->width = width;
    ds->height = height;
    ds->format = format;
    ds->buffer_addr = FB_ADDR + 0xA000000U + i * 0x100000U;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_depth_stencil_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_ds_used, RADEON_MAX_DEPTH_STENCILS)) return;
    g_ds[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)
{
    uint32_t i = find_free_slot(g_shader_used, RADEON_MAX_SHADERS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_shader *s = &g_shader[i];
    s->used = 1;
    s->type = type;
    s->code_size = code_size;
    s->code = (uint8_t *)code;
    s->constant_count = 0;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_shader_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, RADEON_MAX_SHADERS)) return;
    g_shader[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable)
{
    uint32_t i = find_free_slot(g_sampler_used, RADEON_MAX_SAMPLERS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_sampler_state *s = &g_sampler[i];
    s->used = 1;
    s->filter_min = filter_min;
    s->filter_mag = filter_mag;
    s->filter_mip = filter_mip;
    s->address_u = address_u;
    s->address_v = address_v;
    s->address_w = address_w;
    s->max_anisotropy = max_anisotropy;
    s->compare_enable = compare_enable;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_sampler_state_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_sampler_used, RADEON_MAX_SAMPLERS)) return;
    g_sampler[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor)
{
    uint32_t i = find_free_slot(g_blend_used, RADEON_MAX_BLEND_STATES);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_blend_state *b = &g_blend[i];
    b->used = 1;
    b->blend_enable = blend_enable;
    b->src_blend = src_blend;
    b->dest_blend = dest_blend;
    b->blend_op = blend_op;
    b->src_blend_alpha = src_blend_alpha;
    b->dest_blend_alpha = dest_blend_alpha;
    b->blend_op_alpha = blend_op_alpha;
    b->render_target_write_mask = write_mask;
    b->blend_factor = blend_factor;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_blend_state_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_blend_used, RADEON_MAX_BLEND_STATES)) return;
    g_blend[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage)
{
    uint32_t i = find_free_slot(g_raster_used, RADEON_MAX_RASTERIZER_STATES);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_rasterizer_state *r = &g_raster[i];
    r->used = 1;
    r->fill_mode = fill_mode;
    r->cull_mode = cull_mode;
    r->front_counter_clockwise = front_ccw;
    r->depth_clip_enable = depth_clip;
    r->scissor_enable = scissor;
    r->multisample_enable = multisample;
    r->antialiased_line_enable = aa_line;
    r->alpha_to_coverage = alpha_to_coverage;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_rasterizer_state_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_raster_used, RADEON_MAX_RASTERIZER_STATES)) return;
    g_raster[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)
{
    uint32_t i = find_free_slot(g_ds_state_used, RADEON_MAX_DEPTH_STENCIL_STATES);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_depth_stencil_state *d = &g_ds_state[i];
    d->used = 1;
    d->depth_enable = depth_enable;
    d->depth_write_mask = depth_write_mask;
    d->depth_func = depth_func;
    d->stencil_enable = stencil_enable;
    d->stencil_read_mask = stencil_read_mask;
    d->stencil_write_mask = stencil_write_mask;
    d->front_stencil_fail_op = front_fail;
    d->front_stencil_depth_fail_op = front_zfail;
    d->front_stencil_pass_op = front_pass;
    d->front_stencil_func = front_func;
    d->back_stencil_fail_op = back_fail;
    d->back_stencil_depth_fail_op = back_zfail;
    d->back_stencil_pass_op = back_pass;
    d->back_stencil_func = back_func;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_depth_stencil_state_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_ds_state_used, RADEON_MAX_DEPTH_STENCIL_STATES)) return;
    g_ds_state[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)
{
    uint32_t i = find_free_slot(g_il_used, RADEON_MAX_INPUT_LAYOUTS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_input_layout *l = &g_il[i];
    l->used = 1;
    l->element_count = element_count;
    for (uint32_t k = 0; k < element_count && k < 32U; k++) {
        l->slot[k] = slots ? slots[k] : 0U;
        l->offset[k] = offsets ? offsets[k] : 0U;
        l->format[k] = formats ? formats[k] : RADEON_VERTEX_FORMAT_XYZ;
    }
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_input_layout_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_il_used, RADEON_MAX_INPUT_LAYOUTS)) return;
    g_il[handle].used = 0;
}

radeon_handle_t radeon_hd_8370d_constant_buffer_create(uint32_t size)
{
    uint32_t i = find_free_slot(g_cb_used, RADEON_MAX_CONSTANT_BUFFERS);
    if (i == RADEON_HANDLE_NULL) return RADEON_HANDLE_NULL;

    struct radeon_hd_8370d_constant_buffer *c = &g_cb[i];
    c->used = 1;
    c->size = size;
    c->data = (uint8_t *)0;
    return (radeon_handle_t)i;
}

void radeon_hd_8370d_constant_buffer_destroy(radeon_handle_t handle)
{
    if (!handle_valid(handle, g_cb_used, RADEON_MAX_CONSTANT_BUFFERS)) return;
    g_cb[handle].used = 0;
}

void radeon_hd_8370d_constant_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_cb_used, RADEON_MAX_CONSTANT_BUFFERS)) return;
    struct radeon_hd_8370d_constant_buffer *c = &g_cb[handle];
    if (!c->data) return;
    for (uint32_t k = 0; k < size; k++) {
        c->data[offset + k] = ((const uint8_t *)data)[k];
    }
}

/*================== FUNKCJE BINDOWANIA STANOW ==================*/

void radeon_hd_8370d_bind_vertex_buffer(radeon_handle_t handle)
{
    g_ctx.vertex_buffer = handle;
}

void radeon_hd_8370d_bind_index_buffer(radeon_handle_t handle)
{
    g_ctx.index_buffer = handle;
}

void radeon_hd_8370d_bind_input_layout(radeon_handle_t handle)
{
    g_ctx.input_layout = handle;
}

void radeon_hd_8370d_bind_vertex_shader(radeon_handle_t handle)
{
    g_ctx.vertex_shader = handle;
}

void radeon_hd_8370d_bind_pixel_shader(radeon_handle_t handle)
{
    g_ctx.pixel_shader = handle;
}

void radeon_hd_8370d_bind_geometry_shader(radeon_handle_t handle)
{
    g_ctx.geometry_shader = handle;
}

void radeon_hd_8370d_bind_compute_shader(radeon_handle_t handle)
{
    g_ctx.compute_shader = handle;
}

void radeon_hd_8370d_bind_render_target(radeon_handle_t handle)
{
    g_ctx.render_target = handle;
}

void radeon_hd_8370d_bind_depth_stencil(radeon_handle_t handle)
{
    g_ctx.depth_stencil = handle;
}

void radeon_hd_8370d_bind_blend_state(radeon_handle_t handle)
{
    g_ctx.blend_state = handle;
}

void radeon_hd_8370d_bind_rasterizer_state(radeon_handle_t handle)
{
    g_ctx.rasterizer_state = handle;
}

void radeon_hd_8370d_bind_depth_stencil_state(radeon_handle_t handle)
{
    g_ctx.depth_stencil_state = handle;
}

void radeon_hd_8370d_bind_sampler_and_texture(uint32_t slot, radeon_handle_t sampler, radeon_handle_t texture)
{
    if (slot >= RADEON_MAX_SAMPLERS) return;
    g_ctx.sampler[slot] = sampler;
    g_ctx.texture[slot] = texture;
}

void radeon_hd_8370d_bind_constant_buffer(uint32_t slot, radeon_handle_t handle)
{
    if (slot >= RADEON_MAX_CONSTANT_BUFFERS) return;
    g_ctx.constant_buffer[slot] = handle;
}

/*================== FUNKCJE KONFIGURACJI POTOKU 3D ==================*/

void radeon_hd_8370d_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
    g_ctx.viewport_x = x;
    g_ctx.viewport_y = y;
    g_ctx.viewport_w = w;
    g_ctx.viewport_h = h;
    g_ctx.viewport_min_depth = min_depth;
    g_ctx.viewport_max_depth = max_depth;

    uint32_t packed = (x & 0xFFFFU) | ((y & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_OFFSET, packed);
    packed = (w & 0xFFFFU) | ((h & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_OFFSET + 4U, packed);
}

void radeon_hd_8370d_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    g_ctx.scissor_left = left;
    g_ctx.scissor_top = top;
    g_ctx.scissor_right = right;
    g_ctx.scissor_bottom = bottom;

    uint32_t packed = (left & 0xFFFFU) | ((top & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_TL, packed);
    packed = (right & 0xFFFFU) | ((bottom & 0xFFFFU) << 16U);
    mmio_write32(REG_PA_SC_WINDOW_SCISSOR_BR, packed);
}

void radeon_hd_8370d_set_primitive_topology(uint32_t topology)
{
    g_ctx.primitive_topology = topology;
    mmio_write32(REG_PA_SU_PRIM_FILTER_CNTL, topology);
}

void radeon_hd_8370d_set_alpha_blend_global(uint8_t enable)
{
    g_alpha_blend_global = enable;
}

/*================== FUNKCJE RYSOWANIA 3D ==================*/

void radeon_hd_8370d_clear_render_target(radeon_handle_t rt, uint32_t color)
{
    if (!handle_valid(rt, g_rt_used, RADEON_MAX_RENDER_TARGETS)) return;
    struct radeon_hd_8370d_render_target *target = &g_rt[rt];
    mmio_write32(REG_CB_CLEAR_COLOR, color);
    mmio_write32(REG_CB_CLEAR_COLOR_HIGH, 0U);
    mmio_write32(REG_CB_TARGET0_BASE, target->buffer_addr);
    mmio_write32(REG_CB_TARGET0_PITCH, target->width * 4U);
    radeon_hd_8370d_delay_us(100);
}

void radeon_hd_8370d_clear_depth_stencil(radeon_handle_t ds, float depth, uint8_t stencil)
{
    if (!handle_valid(ds, g_ds_used, RADEON_MAX_DEPTH_STENCILS)) return;
    struct radeon_hd_8370d_depth_stencil *depth_stencil = &g_ds[ds];
    uint32_t depth_bits = 0;
    const uint8_t *depth_bytes = (const uint8_t *)&depth;
    for (int k = 0; k < 4; k++) {
        ((uint8_t *)&depth_bits)[k] = depth_bytes[k];
    }
    mmio_write32(REG_DB_CLEAR, depth_bits);
    mmio_write32(REG_DB_STENCIL_CLEAR, (uint32_t)stencil);
    mmio_write32(REG_DB_Z_INFO, depth_stencil->buffer_addr);
    radeon_hd_8370d_delay_us(100);
}

void radeon_hd_8370d_draw(uint32_t vertex_count, uint32_t start_vertex)
{
    if (!g_initialized) return;

    /*ustaw rejestry shaderow*/
    mmio_write32(REG_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_SQ_PROGRAM + 8U, g_ctx.geometry_shader);

    /*emit polecenie DRAW do ring buffer CP*/
    radeon_hd_8370d_cp_emit(0x80000000U | 0x10U);   /*opcode DRAW_VTX*/
    radeon_hd_8370d_cp_emit(vertex_count);
    radeon_hd_8370d_cp_emit(start_vertex);
    radeon_hd_8370d_cp_emit(g_ctx.vertex_buffer);
    radeon_hd_8370d_cp_emit(g_ctx.primitive_topology);
    radeon_hd_8370d_cp_emit(g_ctx.blend_state);
    radeon_hd_8370d_cp_emit(g_ctx.rasterizer_state);
    radeon_hd_8370d_cp_emit(g_ctx.depth_stencil_state);
    radeon_hd_8370d_cp_flush();

    radeon_hd_8370d_delay_us(200);
}

void radeon_hd_8370d_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_SQ_PROGRAM, g_ctx.vertex_shader);
    mmio_write32(REG_SQ_PROGRAM + 4U, g_ctx.pixel_shader);
    mmio_write32(REG_SQ_PROGRAM + 8U, g_ctx.geometry_shader);

    radeon_hd_8370d_cp_emit(0x80000000U | 0x11U);   /*opcode DRAW_IDX*/
    radeon_hd_8370d_cp_emit(index_count);
    radeon_hd_8370d_cp_emit(start_index);
    radeon_hd_8370d_cp_emit((uint32_t)base_vertex);
    radeon_hd_8370d_cp_emit(g_ctx.index_buffer);
    radeon_hd_8370d_cp_emit(g_ctx.primitive_topology);
    radeon_hd_8370d_cp_emit(g_ctx.blend_state);
    radeon_hd_8370d_cp_emit(g_ctx.rasterizer_state);
    radeon_hd_8370d_cp_emit(g_ctx.depth_stencil_state);
    radeon_hd_8370d_cp_flush();

    radeon_hd_8370d_delay_us(200);
}

/*================== FUNKCJE COMMAND BUFFER ==================*/

void radeon_hd_8370d_command_begin(void)
{
    g_cmd_recording = 1;
    g_cmd_count = 0;
}

void radeon_hd_8370d_command_end(void)
{
    if (!g_cmd_recording) return;
    g_cmd_recording = 0;
    for (uint32_t i = 0; i < g_cmd_count; i++) {
        radeon_hd_8370d_cp_emit(g_cmd_buf[i]);
    }
    radeon_hd_8370d_cp_flush();
    g_cmd_count = 0;
}

/*================== FUNKCJE PRESENTATION ==================*/

radeon_handle_t radeon_hd_8370d_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied)
{
    if (g_swap[0].used) return RADEON_HANDLE_NULL;
    g_swap[0].used = 1;
    g_swap[0].width = width;
    g_swap[0].height = height;
    g_swap[0].back_buffer_addr = FB_ADDR + 0x1000000U;
    g_swap[0].front_buffer_addr = FB_ADDR;
    g_swap[0].vsync = vsync;
    g_swap[0].alpha_premultiplied = alpha_premultiplied;
    return 0;
}

void radeon_hd_8370d_swap_chain_destroy(radeon_handle_t handle)
{
    if (handle != 0 || !g_swap[0].used) return;
    g_swap[0].used = 0;
}

void radeon_hd_8370d_present(radeon_handle_t chain)
{
    (void)chain;
    if (!g_swap[0].used) return;

    if (g_swap[0].vsync) {
        for (uint32_t wait = 0; wait < 2000000U; wait++) {
            if (mmio_read32(REG_CRTC0_CONTROL) & 0x80000000U) break;
        }
    }

    /*kopiuj tylny bufor na przedni przez CP ring buffer (BLT z alpha blendingiem jesli premultiplied)*/
    radeon_hd_8370d_cp_emit(0x40000000U | 0x02U);   /*opcode BLT_COPY*/
    radeon_hd_8370d_cp_emit((0U << 16) | 0U);
    radeon_hd_8370d_cp_emit(((FB_HEIGHT - 1U) << 16) | (FB_WIDTH - 1U));
    radeon_hd_8370d_cp_emit((0U << 16) | 0U);
    radeon_hd_8370d_cp_emit(0U);
    radeon_hd_8370d_cp_flush();
}

/*================== FUNKCJE INFORMACJI 3D ==================*/

void radeon_hd_8370d_context_get(struct radeon_hd_8370d_context *out)
{
    if (!out) return;
    *out = g_ctx;
}

int radeon_hd_8370d_3d_available(void)
{
    return g_initialized ? 1 : 0;
}
