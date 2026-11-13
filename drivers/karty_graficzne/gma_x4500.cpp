#include <drivers/karty_graficzne/gma_x4500.h>

/*funkcje we/wy portow*/

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

/*rejestry MMIO Intel GMA X4500HD (Gen4)*/

enum gma_x4500_reg {
    REG_VGA             = 0x71400U,
    REG_DPLL_A          = 0x70000U,
    REG_PIPEA_CONF      = 0x70008U,
    REG_PIPEA_HBLANK    = 0x7000CU,
    REG_PIPEA_VBLANK    = 0x70010U,
    REG_PIPEA_SRC       = 0x70014U,
    REG_PIPEA_HSYNC     = 0x70018U,
    REG_PIPEA_VSYNC     = 0x7001CU,
    REG_PLANE_A_CONF    = 0x70180U,
    REG_PLANE_A_SURF    = 0x70188U,
    REG_PLANE_A_SURF_HIGH = 0x7018CU,
    REG_PLANE_A_STRIDE  = 0x70190U,
    REG_PLANE_A_SIZE    = 0x70194U,
    REG_PLANE_A_POS     = 0x70198U,
    REG_TRANSCONF_A     = 0x60010U,
    REG_BLT_CTL         = 0x2030U,
    REG_BLT_START       = 0x2034U,
    REG_BLT_SIZE        = 0x2038U,
    REG_BLT_HEAD        = 0x203CU,
    REG_BLT_TAIL        = 0x2040U,
    REG_CURSOR_CTL      = 0x70080U,
    REG_CURSOR_POS      = 0x70084U,
    REG_CURSOR_BASE     = 0x70088U,
    REG_OVERLAY_CTL     = 0x78000U,
    REG_OVERLAY_SURF    = 0x78004U,
    REG_INSTDMI         = 0x61100U,
    REG_DEIER           = 0x61110U,
    /*Rejestry potoku 3D Gen4*/
    REG_3D_CTL          = 0x7000U,
    REG_3D_STATE_VS     = 0x7004U,
    REG_3D_STATE_GS     = 0x7008U,
    REG_3D_STATE_CLIP   = 0x700CU,
    REG_3D_STATE_SF     = 0x7010U,
    REG_3D_STATE_WM     = 0x7014U,
    REG_3D_STATE_CC     = 0x7018U,
    REG_3D_STATE_SAMPLE_MASK = 0x701CU,
    REG_3D_STATE_CONSTANT_VS = 0x7020U,
    REG_3D_STATE_CONSTANT_PS = 0x7024U,
    REG_3D_STATE_CONSTANT_GS = 0x7028U,
    REG_3D_PRIMITIVE    = 0x702CU,
    REG_3D_DRAW_VTX     = 0x7030U,
    REG_3D_DRAW_IDX     = 0x7034U,
    REG_3D_CLEAR_RT     = 0x7038U,
    REG_3D_CLEAR_DS     = 0x703CU,
    REG_3D_VIEWPORT     = 0x7040U,
    REG_3D_SCISSOR      = 0x7044U,
    REG_3D_BLEND_STATE  = 0x7048U,
    REG_3D_RASTER_STATE = 0x704CU,
    REG_3D_DS_STATE     = 0x7050U,
    REG_3D_SAMPLER_STATE = 0x7054U,
    REG_3D_TEXTURE_STATE = 0x7058U,
};

/*bity VGA*/

#define VGA_DISP_DISABLE  0x1U

/*bity DPLL_A*/

#define DPLL_VCO_ENABLE   (1U << 31)
#define DPLL_PIPEA_MASK   0x00FF0000U

/*bity PIPEA_CONF*/

#define PIPE_CONF_ENABLE  (1U << 31)
#define PIPE_CONF_DISABLE (0U)

/*bity PLANE_CONF*/

#define PLANE_CONF_ENABLE  (1U << 31)
#define PLANE_FORMAT_XRGB8888  (0x0U << 26)

/*bity CURSOR_CTL*/

#define CURSOR_ENABLE     (1U << 31)

/*bity 3D_CTL*/

#define CTL_3D_ENABLE     (1U << 31)
#define CTL_3D_RESET      (1U << 30)

/*id urzadzen Intel GMA X4500HD (rodzina G45, Gen4)*/

#define INTEL_VENDOR_ID   0x8086U
#define DEVICE_X4500HD_1  0x2E32U
#define DEVICE_X4500HD_2  0x2E42U
#define DEVICE_X4500HD_3  0x2E12U
#define DEVICE_X4500HD_4  0x2E22U
#define DEVICE_X4500HD_5  0x2E02U
#define DEVICE_X4500HD_6  0x2E3AU
#define DEVICE_X4500HD_7  0x2E4AU

#define FB_ADDR  0xFD000000U
#define FB_WIDTH  1024U
#define FB_HEIGHT 768U
#define FB_PITCH  4096U
#define RING_SIZE 4096U

#define BLT_OP_FILL       0x78010000U
#define BLT_OP_COPY       0x74010000U

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

/*Tablice użycia zasobow 3D (musza byc zdefiniowane przed funkcjami je uzywajacymi)*/
static uint8_t g_vb_used[GMA_MAX_VERTEX_BUFFERS] = {0};
static uint8_t g_ib_used[GMA_MAX_INDEX_BUFFERS] = {0};
static uint8_t g_tex_used[GMA_MAX_TEXTURES] = {0};
static uint8_t g_rt_used[GMA_MAX_RENDER_TARGETS] = {0};
static uint8_t g_ds_used[GMA_MAX_DEPTH_STENCILS] = {0};
static uint8_t g_shader_used[GMA_MAX_SHADERS] = {0};
static uint8_t g_sampler_used[GMA_MAX_SAMPLERS] = {0};
static uint8_t g_blend_used[GMA_MAX_BLEND_STATES] = {0};
static uint8_t g_raster_used[GMA_MAX_RASTERIZER_STATES] = {0};
static uint8_t g_ds_state_used[GMA_MAX_DEPTH_STENCIL_STATES] = {0};
static uint8_t g_il_used[GMA_MAX_INPUT_LAYOUTS] = {0};
static uint8_t g_cb_used[GMA_MAX_CONSTANT_BUFFERS] = {0};

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

static void gma_x4500_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

/*wyszukanie urzadzenia Intel GMA X4500HD na PCI*/

static int gma_x4500_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read32(bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != INTEL_VENDOR_ID) continue;
                if (device != DEVICE_X4500HD_1 && device != DEVICE_X4500HD_2 &&
                    device != DEVICE_X4500HD_3 && device != DEVICE_X4500HD_4 &&
                    device != DEVICE_X4500HD_5 && device != DEVICE_X4500HD_6 &&
                    device != DEVICE_X4500HD_7) continue;

                uint32_t bar = pci_read32(bus, slot, func, 0x10);
                if (bar == 0 || bar == 0xFFFFFFFFU || (bar & 1U)) continue;

                uint32_t command = pci_read32(bus, slot, func, 0x04);
                pci_write32(bus, slot, func, 0x04, command | 0x00000006);

                g_bus = bus;
                g_slot = slot;
                g_func = func;
                g_mmio_base = bar & 0xFFFFFFF0U;
                g_mmio = (volatile uint32_t *)g_mmio_base;
                g_device_id = device;
                g_revision = (uint8_t)(pci_read32(bus, slot, func, 0x08) & 0xFFU);
                return 1;
            }
        }
    }
    return 0;
}

/*wyczysczenie przerwan silnika display*/

static void gma_x4500_clear_interrupts(void)
{
    mmio_write32(REG_INSTDMI, 0xFFFFFFFFU);
    mmio_write32(REG_DEIER, 0x00000000U);
}

/*wylaczenie wyswietlania przed zmiana konfiguracji*/

static void gma_x4500_disable_display(void)
{
    uint32_t vga = mmio_read32(REG_VGA);
    mmio_write32(REG_VGA, vga | VGA_DISP_DISABLE);
    gma_x4500_delay_us(100);

    mmio_write32(REG_PIPEA_CONF, mmio_read32(REG_PIPEA_CONF) & ~PIPE_CONF_ENABLE);
    mmio_write32(REG_PLANE_A_CONF, mmio_read32(REG_PLANE_A_CONF) & ~PLANE_CONF_ENABLE);
    mmio_write32(REG_TRANSCONF_A, mmio_read32(REG_TRANSCONF_A) & ~0x80000000U);
    gma_x4500_delay_us(100);
}

/*konfiguracja DPLL (zegar potoku) dla 1024x768@60Hz*/

static int gma_x4500_setup_dpll(void)
{
    uint32_t dpll = mmio_read32(REG_DPLL_A);
    dpll |= DPLL_VCO_ENABLE;
    mmio_write32(REG_DPLL_A, dpll);

    for (uint32_t wait = 0; wait < 500000U; wait++) {
        if ((mmio_read32(REG_DPLL_A) & 0x20000000U) != 0U) {
            return 1;
        }
    }
    return 0;
}

/*konfiguracja pionowych i poziomych pustkowan oraz synchronizacji*/

static void gma_x4500_setup_timing(void)
{
    uint32_t hblank = (((FB_WIDTH + 128U) - 1U) << 16) | (FB_WIDTH - 1U);
    uint32_t vblank = (((FB_HEIGHT + 32U) - 1U) << 16) | (FB_HEIGHT - 1U);
    uint32_t hsync = (((FB_WIDTH + 64U) - 1U) << 16) | (FB_WIDTH - 1U);
    uint32_t vsync = (((FB_HEIGHT + 4U) - 1U) << 16) | (FB_HEIGHT - 1U);

    mmio_write32(REG_PIPEA_HBLANK, hblank);
    mmio_write32(REG_PIPEA_VBLANK, vblank);
    mmio_write32(REG_PIPEA_HSYNC, hsync);
    mmio_write32(REG_PIPEA_VSYNC, vsync);

    mmio_write32(REG_PIPEA_SRC,
                 ((FB_WIDTH - 1U) << 16) | (FB_HEIGHT - 1U));
}

/*wlaczenie potoku wyswietlania A*/

static void gma_x4500_enable_pipe(void)
{
    mmio_write32(REG_PIPEA_CONF, PIPE_CONF_ENABLE);
    gma_x4500_delay_us(200);
}

/*konfiguracja powierzchni A (ramka graficzna)*/

static void gma_x4500_setup_plane(void)
{
    mmio_write32(REG_PLANE_A_CONF, PLANE_CONF_ENABLE | PLANE_FORMAT_XRGB8888);
    mmio_write32(REG_PLANE_A_SURF, FB_ADDR);
    mmio_write32(REG_PLANE_A_SURF_HIGH, 0U);
    mmio_write32(REG_PLANE_A_STRIDE, FB_PITCH);
    mmio_write32(REG_PLANE_A_SIZE,
                 ((FB_HEIGHT - 1U) << 16) | (FB_WIDTH - 1U));
    mmio_write32(REG_PLANE_A_POS, 0U);
}

/*wlaczenie transkodera A (wyjscie na ekran)*/

static void gma_x4500_enable_transcoder(void)
{
    mmio_write32(REG_TRANSCONF_A, 0x80000000U);
    gma_x4500_delay_us(200);
}

/*inicjalizacja ring buffer blitera*/

static void gma_x4500_blt_ring_init(void)
{
    mmio_write32(REG_BLT_CTL, 0x80000000U);
    mmio_write32(REG_BLT_START, (uint32_t)&g_ring[0]);
    mmio_write32(REG_BLT_SIZE, sizeof(g_ring));
    mmio_write32(REG_BLT_HEAD, 0U);
    mmio_write32(REG_BLT_TAIL, 0U);
    g_ring_head = 0;
    g_ring_tail = 0;
}

/*wyslanie polecenia do ring buffer blitera i oczekiwanie na wykonanie*/

static void gma_x4500_blt_flush(void);
static void gma_x4500_blt_emit(uint32_t dword);

/*wyczyscenie ring buffer blitera i oczekiwanie na wykonanie wszystkich polecen*/

static void gma_x4500_blt_flush(void)
{
    uint32_t tail = g_ring_head & (RING_SIZE / 4U - 1U);
    mmio_write32(REG_BLT_TAIL, tail);
    for (volatile uint32_t wait = 0; wait < 1000000U; wait++) {
        if (mmio_read32(REG_BLT_HEAD) == tail) {
            break;
        }
    }
    g_ring_tail = tail;
}

/*wyslanie polecenia do ring buffer blitera i oczekiwanie na wykonanie*/

static void gma_x4500_blt_emit(uint32_t dword)
{
    if (((g_ring_head & (RING_SIZE / 4U - 1U)) + 5U) > (RING_SIZE / 4U)) {
        gma_x4500_blt_flush();
    }
    g_ring[g_ring_head & (RING_SIZE / 4U - 1U)] = dword;
    g_ring_head++;
}

/*inicjalizacja kursora sprzetowego (ukryty domyslnie)*/

static void gma_x4500_cursor_init(void)
{
    mmio_write32(REG_CURSOR_CTL, mmio_read32(REG_CURSOR_CTL) & ~CURSOR_ENABLE);
    mmio_write32(REG_CURSOR_BASE, FB_ADDR);
    mmio_write32(REG_CURSOR_POS, 0U);
    g_cursor_visible = 0;
}

/*inicjalizacja potoku 3D Gen4 (reset, wlaczenie, stan domyslny)*/

static void gma_x4500_3d_pipeline_init(void)
{
    mmio_write32(REG_3D_CTL, CTL_3D_RESET);
    gma_x4500_delay_us(50);
    mmio_write32(REG_3D_CTL, CTL_3D_ENABLE);

    mmio_write32(REG_3D_STATE_VS, 0U);
    mmio_write32(REG_3D_STATE_GS, 0U);
    mmio_write32(REG_3D_STATE_CLIP, 0U);
    mmio_write32(REG_3D_STATE_SF, 0U);
    mmio_write32(REG_3D_STATE_WM, 0U);
    mmio_write32(REG_3D_STATE_CC, 0U);
    mmio_write32(REG_3D_STATE_SAMPLE_MASK, 0xFFFFFFFFU);
    mmio_write32(REG_3D_STATE_CONSTANT_VS, 0U);
    mmio_write32(REG_3D_STATE_CONSTANT_PS, 0U);
    mmio_write32(REG_3D_STATE_CONSTANT_GS, 0U);

    mmio_write32(REG_3D_PRIMITIVE, GMA_PRIM_TRIANGLELIST);
    mmio_write32(REG_3D_VIEWPORT, 0U);
    mmio_write32(REG_3D_SCISSOR, 0U);
    gma_x4500_delay_us(100);
}

/*inicjalizacja sterownika Intel GMA X4500HD (2D + 3D)*/

int gma_x4500_init(void)
{
    if (g_initialized) return 1;
    if (!gma_x4500_find_device()) return 0;

    uint32_t command = pci_read32(g_bus, g_slot, g_func, 0x04);
    pci_write32(g_bus, g_slot, g_func, 0x04, command | 0x00000006U);

    gma_x4500_clear_interrupts();
    gma_x4500_disable_display();

    if (!gma_x4500_setup_dpll()) {
        return 0;
    }

    gma_x4500_setup_timing();
    gma_x4500_setup_plane();
    gma_x4500_enable_pipe();
    gma_x4500_enable_transcoder();

    gma_x4500_blt_ring_init();
    gma_x4500_cursor_init();
    gma_x4500_3d_pipeline_init();

    g_initialized = 1;
    return 1;
}

/*czy sterownik jest zainicjowany i gotowy*/

int gma_x4500_is_ready(void)
{
    return g_initialized;
}

/*rysowanie pojedynczego piksela w ramce graficznej*/

void gma_x4500_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= FB_WIDTH || y >= FB_HEIGHT) return;
    g_fb[y * (FB_PITCH / 4U) + x] = color;
}

/*wypelnienie prostokata (uzywa blitera gdy dostepny, w przeciwnym razie petla CPU)*/

void gma_x4500_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (g_initialized) {
        gma_x4500_blt_emit(BLT_OP_FILL);
        gma_x4500_blt_emit((y << 16) | x);
        gma_x4500_blt_emit(((h - 1U) << 16) | (w - 1U));
        gma_x4500_blt_emit(0U);
        gma_x4500_blt_emit(color);
        gma_x4500_blt_flush();
        return;
    }
    uint32_t xx;
    uint32_t yy;
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= FB_WIDTH || yy >= FB_HEIGHT) continue;
            g_fb[yy * (FB_PITCH / 4U) + xx] = color;
        }
    }
}

/*kopiowanie bloku pikseli (BLT copy) - uzywa blitera sprzetowego*/

void gma_x4500_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                   uint32_t w, uint32_t h)
{
    if (!g_initialized) return;

    gma_x4500_blt_emit(BLT_OP_COPY);
    gma_x4500_blt_emit((dst_y << 16) | dst_x);
    gma_x4500_blt_emit(((h - 1U) << 16) | (w - 1U));
    gma_x4500_blt_emit(0U);
    gma_x4500_blt_emit((src_y << 16) | src_x);
    gma_x4500_blt_flush();
}

/*ustawienie kursora sprhetowego na pozycji (x, y) z podanym obrazem 64x64*/

void gma_x4500_cursor_set(uint32_t x, uint32_t y, const uint32_t *image)
{
    if (!g_initialized) return;

    if (image) {
        for (uint32_t i = 0; i < 64U * 64U; i++) {
            g_fb[i] = image[i];
        }
        mmio_write32(REG_CURSOR_BASE, FB_ADDR);
    }

    mmio_write32(REG_CURSOR_POS, (y << 16) | x);
    mmio_write32(REG_CURSOR_CTL, mmio_read32(REG_CURSOR_CTL) | CURSOR_ENABLE);
    g_cursor_visible = 1;
}

/*ukrycie kursora sprhetowego*/

void gma_x4500_cursor_hide(void)
{
    if (!g_initialized) return;
    mmio_write32(REG_CURSOR_CTL, mmio_read32(REG_CURSOR_CTL) & ~CURSOR_ENABLE);
    g_cursor_visible = 0;
}

/*pobranie wskaznika na ramke graficzna*/

void *gma_x4500_get_fb(void)
{
    return (void *)g_fb;
}

/*pobranie szerokosci ekranu*/

uint32_t gma_x4500_get_width(void)
{
    return FB_WIDTH;
}

/*pobranie wysokosci ekranu*/

uint32_t gma_x4500_get_height(void)
{
    return FB_HEIGHT;
}

/*pobranie id urzadzenia*/

uint16_t gma_x4500_get_device_id(void)
{
    return g_device_id;
}

/*pobranie revisji urzadzenia*/

uint8_t gma_x4500_get_revision(void)
{
    return g_revision;
}

/*pobranie informacji o urzadzeniu*/

void gma_x4500_info_get(struct gma_x4500_info *out)
{
    if (!out) return;
    out->vendor_id = INTEL_VENDOR_ID;
    out->device_id = g_device_id;
    out->revision_id = g_revision;
    out->mmio_base = g_mmio_base;
    out->fb_addr = FB_ADDR;
    out->fb_width = FB_WIDTH;
    out->fb_height = FB_HEIGHT;
    out->fb_pitch = FB_PITCH;
    out->initialized = (uint8_t)g_initialized;
}

/*================== ZASOBY 3D - STATYCZNE TABLICE ==================*/

static struct gma_x4500_vertex_buffer g_vb[GMA_MAX_VERTEX_BUFFERS];
static struct gma_x4500_index_buffer g_ib[GMA_MAX_INDEX_BUFFERS];
static struct gma_x4500_texture g_tex[GMA_MAX_TEXTURES];
static struct gma_x4500_render_target g_rt[GMA_MAX_RENDER_TARGETS];
static struct gma_x4500_depth_stencil g_ds[GMA_MAX_DEPTH_STENCILS];
static struct gma_x4500_shader g_shader[GMA_MAX_SHADERS];
static struct gma_x4500_sampler_state g_sampler[GMA_MAX_SAMPLERS];
static struct gma_x4500_blend_state g_blend[GMA_MAX_BLEND_STATES];
static struct gma_x4500_rasterizer_state g_raster[GMA_MAX_RASTERIZER_STATES];
static struct gma_x4500_depth_stencil_state g_ds_state[GMA_MAX_DEPTH_STENCIL_STATES];
static struct gma_x4500_input_layout g_il[GMA_MAX_INPUT_LAYOUTS];
static struct gma_x4500_constant_buffer g_cb[GMA_MAX_CONSTANT_BUFFERS];
static struct gma_x4500_swap_chain g_swap[1];

static uint8_t g_cmd_recording = 0;
static uint32_t g_cmd_buf[GMA_COMMAND_BUFFER_SIZE];
static uint32_t g_cmd_count = 0;

/*Pomocnicza funkcja - znajdz wolny slot w tablicy zasobow*/
static uint32_t find_free_slot(uint8_t *used, uint32_t max)
{
    for (uint32_t i = 0; i < max; i++) {
        if (!used[i]) return i;
    }
    return GMA_HANDLE_NULL;
}

/*Pomocnicza funkcja - walidacja handle*/
static int handle_valid(gma_handle_t h, uint8_t *used, uint32_t max)
{
    return h != GMA_HANDLE_NULL && h < max && used[h];
}

/*================== FUNKCJE ZASOBOW 3D ==================*/

gma_handle_t gma_x4500_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_vb_used, GMA_MAX_VERTEX_BUFFERS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_vertex_buffer *vb = &g_vb[i];
    vb->used = 1;
    vb->vertex_count = vertex_count;
    vb->vertex_size = vertex_size;
    vb->format = format;
    vb->buffer_addr = FB_ADDR + 0x100000U + i * 0x10000U;
    vb->system_mem = 0;

    if (data) {
        vb->system_mem = (uint8_t *)vb->buffer_addr;
        for (uint32_t k = 0; k < vertex_count * vertex_size; k++) {
            vb->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (gma_handle_t)i;
}

void gma_x4500_vertex_buffer_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_vb_used, GMA_MAX_VERTEX_BUFFERS)) return;
    g_vb[handle].used = 0;
}

void gma_x4500_vertex_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_vb_used, GMA_MAX_VERTEX_BUFFERS)) return;
    struct gma_x4500_vertex_buffer *vb = &g_vb[handle];
    if (!vb->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        vb->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

gma_handle_t gma_x4500_index_buffer_create(uint32_t index_count, uint32_t format, const void *data)
{
    uint32_t i = find_free_slot(g_ib_used, GMA_MAX_INDEX_BUFFERS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_index_buffer *ib = &g_ib[i];
    ib->used = 1;
    ib->index_count = index_count;
    ib->format = format;
    ib->buffer_addr = FB_ADDR + 0x200000U + i * 0x10000U;
    ib->system_mem = 0;

    if (data) {
        ib->system_mem = (uint8_t *)ib->buffer_addr;
        uint32_t elem_size = (format == GMA_INDEX_FORMAT_32BIT) ? 4U : 2U;
        for (uint32_t k = 0; k < index_count * elem_size; k++) {
            ib->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (gma_handle_t)i;
}

void gma_x4500_index_buffer_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_ib_used, GMA_MAX_INDEX_BUFFERS)) return;
    g_ib[handle].used = 0;
}

void gma_x4500_index_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_ib_used, GMA_MAX_INDEX_BUFFERS)) return;
    struct gma_x4500_index_buffer *ib = &g_ib[handle];
    if (!ib->system_mem) return;
    for (uint32_t k = 0; k < size; k++) {
        ib->system_mem[offset + k] = ((const uint8_t *)data)[k];
    }
}

gma_handle_t gma_x4500_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data)
{
    uint32_t i = find_free_slot(g_tex_used, GMA_MAX_TEXTURES);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_texture *t = &g_tex[i];
    t->used = 1;
    t->width = width;
    t->height = height;
    t->format = format;
    t->mip_levels = mip_levels;
    t->pitch = ((width * 4U + 3U) & ~3U);
    t->buffer_addr = FB_ADDR + 0x400000U + i * 0x20000U;
    t->system_mem = 0;

    if (data) {
        t->system_mem = (uint8_t *)t->buffer_addr;
        uint32_t bpp = (format == GMA_TEX_FORMAT_DXT1) ? 1U : 2U;
        for (uint32_t k = 0; k < width * height * bpp; k++) {
            t->system_mem[k] = ((const uint8_t *)data)[k];
        }
    }
    return (gma_handle_t)i;
}

void gma_x4500_texture_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_tex_used, GMA_MAX_TEXTURES)) return;
    g_tex[handle].used = 0;
}

void gma_x4500_texture_update(gma_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data)
{
    (void)level;
    if (!handle_valid(handle, g_tex_used, GMA_MAX_TEXTURES)) return;
    struct gma_x4500_texture *t = &g_tex[handle];
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

gma_handle_t gma_x4500_render_target_create(uint32_t width, uint32_t height, uint32_t format)
{
    uint32_t i = find_free_slot(g_rt_used, GMA_MAX_RENDER_TARGETS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_render_target *rt = &g_rt[i];
    rt->used = 1;
    rt->width = width;
    rt->height = height;
    rt->format = format;
    rt->buffer_addr = FB_ADDR + 0x800000U + i * 0x100000U;
    return (gma_handle_t)i;
}

void gma_x4500_render_target_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_rt_used, GMA_MAX_RENDER_TARGETS)) return;
    g_rt[handle].used = 0;
}

gma_handle_t gma_x4500_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format)
{
    uint32_t i = find_free_slot(g_ds_used, GMA_MAX_DEPTH_STENCILS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_depth_stencil *ds = &g_ds[i];
    ds->used = 1;
    ds->width = width;
    ds->height = height;
    ds->format = format;
    ds->buffer_addr = FB_ADDR + 0xC00000U + i * 0x100000U;
    return (gma_handle_t)i;
}

void gma_x4500_depth_stencil_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_ds_used, GMA_MAX_DEPTH_STENCILS)) return;
    g_ds[handle].used = 0;
}

gma_handle_t gma_x4500_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size)
{
    uint32_t i = find_free_slot(g_shader_used, GMA_MAX_SHADERS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_shader *s = &g_shader[i];
    s->used = 1;
    s->type = type;
    s->code_size = code_size;
    s->code = (uint8_t *)code;
    s->constant_count = 0;
    return (gma_handle_t)i;
}

void gma_x4500_shader_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_shader_used, GMA_MAX_SHADERS)) return;
    g_shader[handle].used = 0;
}

gma_handle_t gma_x4500_sampler_state_create(uint32_t filter, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy)
{
    uint32_t i = find_free_slot(g_sampler_used, GMA_MAX_SAMPLERS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_sampler_state *s = &g_sampler[i];
    s->used = 1;
    s->filter = filter;
    s->address_u = address_u;
    s->address_v = address_v;
    s->address_w = address_w;
    s->max_anisotropy = max_anisotropy;
    return (gma_handle_t)i;
}

void gma_x4500_sampler_state_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_sampler_used, GMA_MAX_SAMPLERS)) return;
    g_sampler[handle].used = 0;
}

gma_handle_t gma_x4500_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask)
{
    uint32_t i = find_free_slot(g_blend_used, GMA_MAX_BLEND_STATES);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_blend_state *b = &g_blend[i];
    b->used = 1;
    b->blend_enable = blend_enable;
    b->src_blend = src_blend;
    b->dest_blend = dest_blend;
    b->blend_op = blend_op;
    b->src_blend_alpha = src_blend_alpha;
    b->dest_blend_alpha = dest_blend_alpha;
    b->blend_op_alpha = blend_op_alpha;
    b->render_target_write_mask = write_mask;
    return (gma_handle_t)i;
}

void gma_x4500_blend_state_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_blend_used, GMA_MAX_BLEND_STATES)) return;
    g_blend[handle].used = 0;
}

gma_handle_t gma_x4500_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line)
{
    uint32_t i = find_free_slot(g_raster_used, GMA_MAX_RASTERIZER_STATES);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_rasterizer_state *r = &g_raster[i];
    r->used = 1;
    r->fill_mode = fill_mode;
    r->cull_mode = cull_mode;
    r->front_counter_clockwise = front_ccw;
    r->depth_clip_enable = depth_clip;
    r->scissor_enable = scissor;
    r->multisample_enable = multisample;
    r->antialiased_line_enable = aa_line;
    return (gma_handle_t)i;
}

void gma_x4500_rasterizer_state_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_raster_used, GMA_MAX_RASTERIZER_STATES)) return;
    g_raster[handle].used = 0;
}

gma_handle_t gma_x4500_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)
{
    uint32_t i = find_free_slot(g_ds_state_used, GMA_MAX_DEPTH_STENCIL_STATES);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_depth_stencil_state *d = &g_ds_state[i];
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
    return (gma_handle_t)i;
}

void gma_x4500_depth_stencil_state_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_ds_state_used, GMA_MAX_DEPTH_STENCIL_STATES)) return;
    g_ds_state[handle].used = 0;
}

gma_handle_t gma_x4500_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats)
{
    uint32_t i = find_free_slot(g_il_used, GMA_MAX_INPUT_LAYOUTS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_input_layout *l = &g_il[i];
    l->used = 1;
    l->element_count = element_count;
    for (uint32_t k = 0; k < element_count && k < 16U; k++) {
        l->slot[k] = slots ? slots[k] : 0U;
        l->offset[k] = offsets ? offsets[k] : 0U;
        l->format[k] = formats ? formats[k] : GMA_VERTEX_FORMAT_XYZ;
    }
    return (gma_handle_t)i;
}

void gma_x4500_input_layout_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_il_used, GMA_MAX_INPUT_LAYOUTS)) return;
    g_il[handle].used = 0;
}

gma_handle_t gma_x4500_constant_buffer_create(uint32_t size)
{
    uint32_t i = find_free_slot(g_cb_used, GMA_MAX_CONSTANT_BUFFERS);
    if (i == GMA_HANDLE_NULL) return GMA_HANDLE_NULL;

    struct gma_x4500_constant_buffer *c = &g_cb[i];
    c->used = 1;
    c->size = size;
    c->data = (uint8_t *)0;
    return (gma_handle_t)i;
}

void gma_x4500_constant_buffer_destroy(gma_handle_t handle)
{
    if (!handle_valid(handle, g_cb_used, GMA_MAX_CONSTANT_BUFFERS)) return;
    g_cb[handle].used = 0;
}

void gma_x4500_constant_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data)
{
    if (!handle_valid(handle, g_cb_used, GMA_MAX_CONSTANT_BUFFERS)) return;
    struct gma_x4500_constant_buffer *c = &g_cb[handle];
    if (!c->data) return;
    for (uint32_t k = 0; k < size; k++) {
        c->data[offset + k] = ((const uint8_t *)data)[k];
    }
}

/*================== FUNKCJE BINDOWANIA STANOW ==================*/

static struct gma_x4500_context g_ctx = {};

void gma_x4500_bind_vertex_buffer(gma_handle_t handle)
{
    g_ctx.vertex_buffer = handle;
}

void gma_x4500_bind_index_buffer(gma_handle_t handle)
{
    g_ctx.index_buffer = handle;
}

void gma_x4500_bind_input_layout(gma_handle_t handle)
{
    g_ctx.input_layout = handle;
}

void gma_x4500_bind_vertex_shader(gma_handle_t handle)
{
    g_ctx.vertex_shader = handle;
}

void gma_x4500_bind_pixel_shader(gma_handle_t handle)
{
    g_ctx.pixel_shader = handle;
}

void gma_x4500_bind_geometry_shader(gma_handle_t handle)
{
    g_ctx.geometry_shader = handle;
}

void gma_x4500_bind_render_target(gma_handle_t handle)
{
    g_ctx.render_target = handle;
}

void gma_x4500_bind_depth_stencil(gma_handle_t handle)
{
    g_ctx.depth_stencil = handle;
}

void gma_x4500_bind_blend_state(gma_handle_t handle)
{
    g_ctx.blend_state = handle;
}

void gma_x4500_bind_rasterizer_state(gma_handle_t handle)
{
    g_ctx.rasterizer_state = handle;
}

void gma_x4500_bind_depth_stencil_state(gma_handle_t handle)
{
    g_ctx.depth_stencil_state = handle;
}

void gma_x4500_bind_sampler_and_texture(uint32_t slot, gma_handle_t sampler, gma_handle_t texture)
{
    if (slot >= GMA_MAX_SAMPLERS) return;
    g_ctx.sampler[slot] = sampler;
    g_ctx.texture[slot] = texture;
}

void gma_x4500_bind_constant_buffer(uint32_t slot, gma_handle_t handle)
{
    if (slot >= GMA_MAX_CONSTANT_BUFFERS) return;
    g_ctx.constant_buffer[slot] = handle;
}

/*================== FUNKCJE KONFIGURACJI POTOKU 3D ==================*/

void gma_x4500_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth)
{
    g_ctx.viewport_x = x;
    g_ctx.viewport_y = y;
    g_ctx.viewport_w = w;
    g_ctx.viewport_h = h;
    g_ctx.viewport_min_depth = min_depth;
    g_ctx.viewport_max_depth = max_depth;

    uint32_t packed = (x & 0xFFFFU) | ((y & 0xFFFFU) << 16U);
    mmio_write32(REG_3D_VIEWPORT, packed);
    packed = (w & 0xFFFFU) | ((h & 0xFFFFU) << 16U);
    mmio_write32(REG_3D_VIEWPORT + 4U, packed);
}

void gma_x4500_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    g_ctx.scissor_left = left;
    g_ctx.scissor_top = top;
    g_ctx.scissor_right = right;
    g_ctx.scissor_bottom = bottom;

    uint32_t packed = (left & 0xFFFFU) | ((top & 0xFFFFU) << 16U);
    mmio_write32(REG_3D_SCISSOR, packed);
    packed = (right & 0xFFFFU) | ((bottom & 0xFFFFU) << 16U);
    mmio_write32(REG_3D_SCISSOR + 4U, packed);
}

void gma_x4500_set_primitive_topology(uint32_t topology)
{
    g_ctx.primitive_topology = topology;
    mmio_write32(REG_3D_PRIMITIVE, topology);
}

/*================== FUNKCJE RYSOWANIA 3D ==================*/

void gma_x4500_clear_render_target(gma_handle_t rt, uint32_t color)
{
    if (!handle_valid(rt, g_rt_used, GMA_MAX_RENDER_TARGETS)) return;
    struct gma_x4500_render_target *target = &g_rt[rt];
    mmio_write32(REG_3D_CLEAR_RT, color);
    mmio_write32(REG_3D_CLEAR_RT + 4U, target->buffer_addr);
    mmio_write32(REG_3D_CLEAR_RT + 8U, target->width);
    mmio_write32(REG_3D_CLEAR_RT + 12U, target->height);
    gma_x4500_delay_us(100);
}

void gma_x4500_clear_depth_stencil(gma_handle_t ds, float depth, uint8_t stencil)
{
    if (!handle_valid(ds, g_ds_used, GMA_MAX_DEPTH_STENCILS)) return;
    struct gma_x4500_depth_stencil *depth_stencil = &g_ds[ds];
    uint32_t depth_bits = 0;
    const uint8_t *depth_bytes = (const uint8_t *)&depth;
    for (int k = 0; k < 4; k++) {
        ((uint8_t *)&depth_bits)[k] = depth_bytes[k];
    }
    mmio_write32(REG_3D_CLEAR_DS, depth_stencil->buffer_addr);
    mmio_write32(REG_3D_CLEAR_DS + 4U, depth_bits);
    mmio_write32(REG_3D_CLEAR_DS + 8U, (uint32_t)stencil);
    gma_x4500_delay_us(100);
}

void gma_x4500_draw(uint32_t vertex_count, uint32_t start_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_3D_STATE_VS, g_ctx.vertex_shader);
    mmio_write32(REG_3D_STATE_GS, g_ctx.geometry_shader);
    mmio_write32(REG_3D_STATE_WM, g_ctx.pixel_shader);
    mmio_write32(REG_3D_DRAW_VTX, vertex_count);
    mmio_write32(REG_3D_DRAW_VTX + 4U, start_vertex);
    mmio_write32(REG_3D_DRAW_VTX + 8U, g_ctx.vertex_buffer);
    gma_x4500_delay_us(200);
}

void gma_x4500_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex)
{
    if (!g_initialized) return;

    mmio_write32(REG_3D_STATE_VS, g_ctx.vertex_shader);
    mmio_write32(REG_3D_STATE_GS, g_ctx.geometry_shader);
    mmio_write32(REG_3D_STATE_WM, g_ctx.pixel_shader);
    mmio_write32(REG_3D_DRAW_IDX, index_count);
    mmio_write32(REG_3D_DRAW_IDX + 4U, start_index);
    mmio_write32(REG_3D_DRAW_IDX + 8U, (uint32_t)base_vertex);
    mmio_write32(REG_3D_DRAW_IDX + 12U, g_ctx.index_buffer);
    gma_x4500_delay_us(200);
}

/*================== FUNKCJE COMMAND BUFFER ==================*/

void gma_x4500_command_begin(void)
{
    g_cmd_recording = 1;
    g_cmd_count = 0;
}

void gma_x4500_command_end(void)
{
    if (!g_cmd_recording) return;
    g_cmd_recording = 0;
    for (uint32_t i = 0; i < g_cmd_count; i++) {
        gma_x4500_blt_emit(g_cmd_buf[i]);
    }
    gma_x4500_blt_flush();
    g_cmd_count = 0;
}

/*================== FUNKCJE PRESENTATION ==================*/

gma_handle_t gma_x4500_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync)
{
    if (g_swap[0].used) return GMA_HANDLE_NULL;
    g_swap[0].used = 1;
    g_swap[0].width = width;
    g_swap[0].height = height;
    g_swap[0].back_buffer_addr = FB_ADDR + 0x1000000U;
    g_swap[0].front_buffer_addr = FB_ADDR;
    g_swap[0].vsync = vsync;
    return 0;
}

void gma_x4500_swap_chain_destroy(gma_handle_t handle)
{
    if (handle != 0 || !g_swap[0].used) return;
    g_swap[0].used = 0;
}

void gma_x4500_present(gma_handle_t chain)
{
    (void)chain;
    if (!g_swap[0].used) return;
    if (g_swap[0].vsync) {
        for (volatile uint32_t wait = 0; wait < 2000000U; wait++) {
            if (mmio_read32(REG_PIPEA_CONF) & 0x80000000U) break;
        }
    }
    gma_x4500_blt_emit(BLT_OP_COPY);
    gma_x4500_blt_emit((0U << 16) | 0U);
    gma_x4500_blt_emit(((FB_HEIGHT - 1U) << 16) | (FB_WIDTH - 1U));
    gma_x4500_blt_emit(0U);
    gma_x4500_blt_emit((0U << 16) | 0U);
    gma_x4500_blt_flush();
}

/*================== FUNKCJE INFORMACJI 3D ==================*/

void gma_x4500_context_get(struct gma_x4500_context *out)
{
    if (!out) return;
    *out = g_ctx;
}

int gma_x4500_3d_available(void)
{
    return g_initialized ? 1 : 0;
}
