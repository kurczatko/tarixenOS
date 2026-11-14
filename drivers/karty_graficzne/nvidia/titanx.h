/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#ifndef TITANX_H
#define TITANX_H

#include <include/printf/types.h>

#define TITANX_VENDOR_ID 0x10DEU
#define TITANX_DEVICE_ID_GP102 0x10D0U

#define TITANX_VRAM_SIZE_BYTES ((uint64_t)12ULL * 1024ULL * 1024ULL * 1024ULL)
#define TITANX_VRAM_PAGE_SIZE 4096U
#define TITANX_VRAM_MAX_ALLOCS 4096U
#define TITANX_FB_WIDTH 1920U
#define TITANX_FB_HEIGHT 1080U
#define TITANX_FB_PITCH (TITANX_FB_WIDTH * 4U)
#define TITANX_FB_ADDR 0xFE000000U

#define TITANX_MAX_VERTEX_BUFFERS 64U
#define TITANX_MAX_INDEX_BUFFERS 64U
#define TITANX_MAX_TEXTURES 128U
#define TITANX_MAX_RENDER_TARGETS 8U
#define TITANX_MAX_DEPTH_STENCILS 8U
#define TITANX_MAX_SHADERS 64U
#define TITANX_MAX_COMPUTE_SHADERS 16U
#define TITANX_MAX_SAMPLERS 16U
#define TITANX_MAX_BLEND_STATES 16U
#define TITANX_MAX_RASTERIZER_STATES 16U
#define TITANX_MAX_DEPTH_STENCIL_STATES 16U
#define TITANX_MAX_INPUT_LAYOUTS 32U
#define TITANX_MAX_CONSTANT_BUFFERS 16U
#define TITANX_MAX_SWAP_CHAINS 4U
#define TITANX_MAX_FENCES 16U
#define TITANX_MAX_SEMAPHORES 16U
#define TITANX_COMMAND_BUFFER_SIZE 16384U
#define TITANX_RING_BUFFER_SIZE 8192U

#define TITANX_HANDLE_NULL 0xFFFFFFFFU
typedef uint32_t titanx_handle_t;

enum titanx_reg {
    REG_TITANX_PGRAPH_CTX_CONTROL = 0x400800U,
    REG_TITANX_PGRAPH_TRAPPED = 0x400804U,
    REG_TITANX_PFIFO_CACHE_DMA_PUT = 0x100200U,
    REG_TITANX_PFIFO_CACHE_DMA_GET = 0x100204U,
    REG_TITANX_PFIFO_CACHE_DMA_CTRL = 0x100208U,
    REG_TITANX_PFIFO_INTR = 0x100210U,
    REG_TITANX_PFIFO_INTR_EN = 0x100214U,
    REG_TITANX_PBUS_PWR = 0x000200U,
    REG_TITANX_PBUS_PWR_GATE = 0x000204U,
    REG_TITANX_PCLOCK_GFX = 0x000210U,
    REG_TITANX_PCLOCK_MEM = 0x000214U,
    REG_TITANX_PCOUNTER_TEMP = 0x000220U,
    REG_TITANX_PFAN_PWM = 0x000224U,
    REG_TITANX_PFAN_RPM = 0x000228U,
    REG_TITANX_PRGB_CONTROL = 0x000230U,
    REG_TITANX_PRGB_COLOR = 0x000234U,
    REG_TITANX_DISP_DISP_CONTROL = 0x600000U,
    REG_TITANX_DISP_CRTC_CONTROL = 0x600004U,
    REG_TITANX_DISP_CRTC_H_TOTAL = 0x600008U,
    REG_TITANX_DISP_CRTC_V_TOTAL = 0x60000CU,
    REG_TITANX_DISP_CRTC_SURFACE = 0x600010U,
    REG_TITANX_DISP_CRTC_PITCH = 0x600014U,
    REG_TITANX_CURSOR_CONTROL = 0x600100U,
    REG_TITANX_CURSOR_POSITION = 0x600104U,
    REG_TITANX_CURSOR_BASE = 0x600108U,
    REG_TITANX_RT_CONTROL = 0x800000U,
    REG_TITANX_RT_BASE0 = 0x800004U,
    REG_TITANX_RT_PITCH0 = 0x800008U,
    REG_TITANX_RT_FORMAT0 = 0x80000CU,
    REG_TITANX_DS_CONTROL = 0x800010U,
    REG_TITANX_DS_BASE = 0x800014U,
    REG_TITANX_DS_FORMAT = 0x800018U,
    REG_TITANX_BLEND_CONTROL = 0x800020U,
    REG_TITANX_BLEND_SRC = 0x800024U,
    REG_TITANX_BLEND_DST = 0x800028U,
    REG_TITANX_BLEND_OP = 0x80002CU,
    REG_TITANX_RASTER_CONTROL = 0x800030U,
    REG_TITANX_RASTER_CULL = 0x800034U,
    REG_TITANX_RASTER_FILL = 0x800038U,
    REG_TITANX_ZS_CONTROL = 0x800040U,
    REG_TITANX_ZS_FUNC = 0x800044U,
    REG_TITANX_SHADER_PROGRAM = 0x900000U,
    REG_TITANX_SHADER_CONSTANT = 0x900004U,
    REG_TITANX_SHADER_VTX_BASE = 0x900008U,
    REG_TITANX_SHADER_VTX_STRIDE = 0x90000CU,
    REG_TITANX_TEX_BASE = 0xA00000U,
    REG_TITANX_TEX_SAMPLER = 0xA00004U,
    REG_TITANX_COMPUTE_PROGRAM = 0xB00000U,
    REG_TITANX_COMPUTE_DIM = 0xB00004U,
    REG_TITANX_TESS_CONTROL = 0xC00000U,
    REG_TITANX_TESS_EVAL = 0xC00004U,
    REG_TITANX_TIMESTAMP_LO = 0xD00000U,
    REG_TITANX_TIMESTAMP_HI = 0xD00004U,
    REG_TITANX_VRAM_USAGE = 0xD00008U,
    REG_TITANX_VRAM_TOTAL = 0xD0000CU
};

#define TITANX_PWR_D0 0U
#define TITANX_PWR_D1 1U
#define TITANX_PWR_D2 2U
#define TITANX_PWR_D3 3U

#define TITANX_VTX_XYZ 0x0001U
#define TITANX_VTX_XYZW 0x0002U
#define TITANX_VTX_XYZ_COLOR 0x0004U
#define TITANX_VTX_XYZ_TEX 0x0008U
#define TITANX_VTX_XYZ_COLOR_TEX 0x0010U

#define TITANX_IDX_16BIT 0x0000U
#define TITANX_IDX_32BIT 0x0001U

#define TITANX_PRIM_POINT 0x0000U
#define TITANX_PRIM_LINE 0x0001U
#define TITANX_PRIM_TRIANGLE 0x0002U
#define TITANX_PRIM_TRIANGLE_STRIP 0x0003U
#define TITANX_PRIM_TRIANGLE_FAN 0x0004U
#define TITANX_PRIM_LINE_LOOP 0x0005U
#define TITANX_PRIM_LINE_STRIP 0x0006U

#define TITANX_TESS_NONE 0x0000U
#define TITANX_TESS_ISOLINES 0x0001U
#define TITANX_TESS_QUADS 0x0002U
#define TITANX_TESS_TRIANGLES 0x0003U

#define TITANX_TEX_RGBA8 0x0000U
#define TITANX_TEX_RGB8 0x0001U
#define TITANX_TEX_DXT1 0x0010U
#define TITANX_TEX_DXT3 0x0011U
#define TITANX_TEX_DXT5 0x0012U
#define TITANX_TEX_FLOAT_RGBA 0x0020U

#define TITANX_SHADER_VERTEX 0x0000U
#define TITANX_SHADER_PIXEL 0x0001U
#define TITANX_SHADER_GEOMETRY 0x0002U
#define TITANX_SHADER_COMPUTE 0x0003U
#define TITANX_SHADER_TESS_CONTROL 0x0004U
#define TITANX_SHADER_TESS_EVAL 0x0005U

#define TITANX_BLEND_ZERO 0x0000U
#define TITANX_BLEND_ONE 0x0001U
#define TITANX_BLEND_SRC_ALPHA 0x0004U
#define TITANX_BLEND_INV_SRC_ALPHA 0x0006U
#define TITANX_BLEND_OP_ADD 0x0000U
#define TITANX_BLEND_OP_SUB 0x0001U
#define TITANX_BLEND_OP_REV_SUB 0x0002U
#define TITANX_BLEND_OP_MIN 0x0003U
#define TITANX_BLEND_OP_MAX 0x0004U

#define TITANX_CMP_NEVER 0x0000U
#define TITANX_CMP_LESS 0x0001U
#define TITANX_CMP_EQUAL 0x0002U
#define TITANX_CMP_LEQUAL 0x0003U
#define TITANX_CMP_GREATER 0x0004U
#define TITANX_CMP_NOTEQUAL 0x0005U
#define TITANX_CMP_GEQUAL 0x0006U
#define TITANX_CMP_ALWAYS 0x0007U

#define TITANX_FILL_SOLID 0x0000U
#define TITANX_FILL_WIREFRAME 0x0001U
#define TITANX_CULL_NONE 0x0000U
#define TITANX_CULL_FRONT 0x0001U
#define TITANX_CULL_BACK 0x0002U

struct titanx_info {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint32_t mmio_base;
    uint32_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint8_t initialized;
    uint8_t architecture;
    uint64_t vram_size;
    uint64_t vram_used;
    uint32_t gfx_clock_mhz;
    uint32_t memory_clock_mhz;
    uint32_t temperature_c;
    uint32_t fan_rpm;
};

struct titanx_vertex_buffer {
    uint8_t used;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct titanx_index_buffer {
    uint8_t used;
    uint32_t index_count;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint64_t vram_offset;
};

struct titanx_texture {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t mip_levels;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint32_t pitch;
    uint64_t vram_offset;
    uint8_t compressed;
};

struct titanx_render_target {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t alpha_blend;
    uint64_t vram_offset;
};

struct titanx_depth_stencil {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t compression;
    uint64_t vram_offset;
};

struct titanx_shader {
    uint8_t used;
    uint32_t type;
    uint32_t code_size;
    uint8_t *code;
    uint32_t constant_count;
    uint8_t compiled;
};

struct titanx_sampler_state {
    uint8_t used;
    uint32_t filter_min;
    uint32_t filter_mag;
    uint32_t filter_mip;
    uint32_t address_u;
    uint32_t address_v;
    uint32_t address_w;
    uint32_t max_anisotropy;
    uint8_t compare_enable;
    uint8_t compare_func;
};

struct titanx_blend_state {
    uint8_t used;
    uint8_t blend_enable;
    uint32_t src_blend;
    uint32_t dest_blend;
    uint32_t blend_op;
    uint32_t src_blend_alpha;
    uint32_t dest_blend_alpha;
    uint32_t blend_op_alpha;
    uint32_t render_target_write_mask;
    uint32_t blend_factor;
};

struct titanx_rasterizer_state {
    uint8_t used;
    uint32_t fill_mode;
    uint32_t cull_mode;
    uint8_t front_counter_clockwise;
    uint8_t depth_clip_enable;
    uint8_t scissor_enable;
    uint8_t multisample_enable;
    uint8_t antialiased_line_enable;
    uint8_t alpha_to_coverage;
};

struct titanx_depth_stencil_state {
    uint8_t used;
    uint8_t depth_enable;
    uint32_t depth_write_mask;
    uint32_t depth_func;
    uint8_t stencil_enable;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    uint32_t front_stencil_fail_op;
    uint32_t front_stencil_depth_fail_op;
    uint32_t front_stencil_pass_op;
    uint32_t front_stencil_func;
    uint32_t back_stencil_fail_op;
    uint32_t back_stencil_depth_fail_op;
    uint32_t back_stencil_pass_op;
    uint32_t back_stencil_func;
};

struct titanx_input_layout {
    uint8_t used;
    uint32_t element_count;
    uint32_t slot[32];
    uint32_t offset[32];
    uint32_t format[32];
};

struct titanx_constant_buffer {
    uint8_t used;
    uint32_t size;
    uint8_t *data;
    uint64_t vram_offset;
};

struct titanx_swap_chain {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t back_buffer_addr;
    uint32_t front_buffer_addr;
    uint8_t vsync;
    uint8_t alpha_premultiplied;
    uint64_t back_vram_offset;
    uint64_t front_vram_offset;
};

struct titanx_fence {
    uint8_t used;
    uint32_t value;
    uint8_t signaled;
};

struct titanx_semaphore {
    uint8_t used;
    uint32_t value;
};

struct titanx_vram_alloc {
    uint8_t used;
    uint64_t offset;
    uint64_t size;
    uint32_t type;
};

struct titanx_context {
    titanx_handle_t vertex_buffer;
    titanx_handle_t index_buffer;
    titanx_handle_t input_layout;
    titanx_handle_t vertex_shader;
    titanx_handle_t pixel_shader;
    titanx_handle_t geometry_shader;
    titanx_handle_t tess_control_shader;
    titanx_handle_t tess_eval_shader;
    titanx_handle_t compute_shader;
    titanx_handle_t render_target;
    titanx_handle_t depth_stencil;
    titanx_handle_t blend_state;
    titanx_handle_t rasterizer_state;
    titanx_handle_t depth_stencil_state;
    titanx_handle_t sampler[TITANX_MAX_SAMPLERS];
    titanx_handle_t texture[TITANX_MAX_SAMPLERS];
    titanx_handle_t constant_buffer[TITANX_MAX_CONSTANT_BUFFERS];
    uint32_t viewport_x;
    uint32_t viewport_y;
    uint32_t viewport_w;
    uint32_t viewport_h;
    float viewport_min_depth;
    float viewport_max_depth;
    uint32_t scissor_left;
    uint32_t scissor_top;
    uint32_t scissor_right;
    uint32_t scissor_bottom;
    uint32_t primitive_topology;
    uint32_t tessellation_mode;
    uint32_t tessellation_factor;
    uint8_t alpha_blend_global;
    uint8_t in_command_buffer;
};

#ifdef __cplusplus

class TitanX {
public:
    TitanX(void);
    int inicjalizuj(void);
    void zamknij(void);
    uint8_t czy_gotowy(void);
    void putpixel(uint32_t x, uint32_t y, uint32_t kolor);
    void fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor);
    void blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
    void cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz);
    void cursor_hide(void);
    void *get_fb(void);
    uint32_t get_szerokosc(void);
    uint32_t get_wysokosc(void);
    uint16_t get_device_id(void);
    uint8_t get_rewizja(void);
    void info_pobierz(struct titanx_info *wyjscie);

    uint64_t vram_alokuj(uint64_t rozmiar, uint32_t typ);
    void vram_zwolnij(uint64_t uchwyt);
    uint64_t vram_uzyto(void);
    uint64_t vram_calkowicie(void);

    titanx_handle_t bufor_wierzcholkow_stworz(uint32_t liczba_wierzcholkow, uint32_t rozmiar_wierzcholka, uint32_t format, const void *dane);
    void bufor_wierzcholkow_zniszcz(titanx_handle_t uchwyt);
    void bufor_wierzcholkow_zaktualizuj(titanx_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

    titanx_handle_t bufor_indeksow_stworz(uint32_t liczba_indeksow, uint32_t format, const void *dane);
    void bufor_indeksow_zniszcz(titanx_handle_t uchwyt);
    void bufor_indeksow_zaktualizuj(titanx_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

    titanx_handle_t tekstura_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint32_t poziomy_mip, const void *dane);
    void tekstura_zniszcz(titanx_handle_t uchwyt);
    void tekstura_zaktualizuj(titanx_handle_t uchwyt, uint32_t poziom, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *dane);
    void tekstura_wygeneruj_mipmapy(titanx_handle_t uchwyt);

    titanx_handle_t cel_renderowania_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t alpha_blend);
    void cel_renderowania_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t bufor_glebii_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t kompresja);
    void bufor_glebii_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t shader_stworz(uint32_t typ, const uint8_t *kod, uint32_t rozmiar_kodu);
    void shader_zniszcz(titanx_handle_t uchwyt);
    void shader_skompiluj(titanx_handle_t uchwyt);

    titanx_handle_t stan_probnika_stworz(uint32_t filtr_min, uint32_t filtr_mag, uint32_t filtr_mip, uint32_t adres_u, uint32_t adres_v, uint32_t adres_w, uint32_t max_anizotropia, uint8_t wlacz_porownanie, uint8_t func_porownania);
    void stan_probnika_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t stan_mieszania_stworz(uint8_t wlacz_mieszanie, uint32_t src_blend, uint32_t dest_blend, uint32_t op_mieszania, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t op_mieszania_alpha, uint32_t maska_zapisu, uint32_t blend_factor);
    void stan_mieszania_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t stan_rasteryzacji_stworz(uint32_t tryb_wypelnienia, uint32_t tryb_odcinania, uint8_t front_ccw, uint8_t clip_glebii, uint8_t scissor, uint8_t wielopr_probowanie, uint8_t aa_linia, uint8_t alpha_do_pokrycia);
    void stan_rasteryzacji_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t stan_glebii_stencil_stworz(uint8_t wlacz_glebie, uint32_t maska_zapisu_glebii, uint32_t func_glebii, uint8_t wlacz_stencil, uint8_t maska_odczytu_stencil, uint8_t maska_zapisu_stencil, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
    void stan_glebii_stencil_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t uklad_wejsc_stworz(uint32_t liczba_elementow, const uint32_t *sloty, const uint32_t *przesuniecia, const uint32_t *formaty);
    void uklad_wejsc_zniszcz(titanx_handle_t uchwyt);

    titanx_handle_t bufor_stale_stworz(uint32_t rozmiar);
    void bufor_stale_zniszcz(titanx_handle_t uchwyt);
    void bufor_stale_zaktualizuj(titanx_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

    void powiaz_bufor_wierzcholkow(titanx_handle_t uchwyt);
    void powiaz_bufor_indeksow(titanx_handle_t uchwyt);
    void powiaz_uklad_wejsc(titanx_handle_t uchwyt);
    void powiaz_shader_wierzcholkow(titanx_handle_t uchwyt);
    void powiaz_shader_pikseli(titanx_handle_t uchwyt);
    void powiaz_shader_geometrii(titanx_handle_t uchwyt);
    void powiaz_shader_tess_kontrola(titanx_handle_t uchwyt);
    void powiaz_shader_tess_evaluacja(titanx_handle_t uchwyt);
    void powiaz_shader_obliczeniowy(titanx_handle_t uchwyt);
    void powiaz_cel_renderowania(titanx_handle_t uchwyt);
    void powiaz_bufor_glebii(titanx_handle_t uchwyt);
    void powiaz_stan_mieszania(titanx_handle_t uchwyt);
    void powiaz_stan_rasteryzacji(titanx_handle_t uchwyt);
    void powiaz_stan_glebii_stencil(titanx_handle_t uchwyt);
    void powiaz_probnik_i_teksture(uint32_t gniazdo, titanx_handle_t probnik, titanx_handle_t tekstura);
    void powiaz_bufor_stale(uint32_t gniazdo, titanx_handle_t uchwyt);

    void ustaw_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_glebia, float max_glebia);
    void ustaw_scissor(uint32_t lewa, uint32_t gora, uint32_t prawa, uint32_t dol);
    void ustaw_typ_pierwotnego(uint32_t typ_pierwotnego);
    void ustaw_tessellation(uint32_t tryb, uint32_t factor);
    void ustaw_mieszanie_alpha_globalne(uint8_t wlacz);

    void wyczysc_cel_renderowania(titanx_handle_t cel, uint32_t kolor);
    void wyczysc_bufor_glebii(titanx_handle_t bufor, float glebia, uint8_t stencil);
    void rysuj(uint32_t liczba_wierzcholkow, uint32_t pierwszy_wierzcholek);
    void rysuj_indeksowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka);
    void rysuj_indeksowane_instancjonowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka, uint32_t liczba_instancji, uint32_t pierwsza_instancja);
    void rysuj_po_indirekcie(uint32_t adres_bufora, uint32_t liczba_rysowan, uint32_t krok);

    void oblicz_wyslij(uint32_t grupa_x, uint32_t grupa_y, uint32_t grupa_z);
    void oblicz_wyslij_po_indirekcie(uint32_t adres_bufora);

    void ogrodzenie_stworz(titanx_handle_t *wyjscie);
    void ogrodzenie_sygnalizuj(titanx_handle_t uchwyt, uint32_t wartosc);
    int ogrodzenie_poczekaj(titanx_handle_t uchwyt, uint32_t wartosc, uint32_t timeout_us);
    void ogrodzenie_zniszcz(titanx_handle_t uchwyt);

    void semafor_stworz(titanx_handle_t *wyjscie);
    void semafor_sygnalizuj(titanx_handle_t uchwyt, uint32_t wartosc);
    void semafor_poczekaj(titanx_handle_t uchwyt, uint32_t wartosc);
    void semafor_zniszcz(titanx_handle_t uchwyt);

    int czekaj_na_bezczynnosc(uint32_t timeout_us);
    uint64_t znacznik_czasu(void);

    void ustaw_stan_zasilania(uint8_t stan);
    uint8_t pobierz_stan_zasilania(void);
    void ustaw_zegar_graficzny(uint32_t mhz);
    void ustaw_zegar_pamieci(uint32_t mhz);
    uint32_t pobierz_zegar_graficzny(void);
    uint32_t pobierz_zegar_pamieci(void);
    uint32_t pobierz_temperature(void);
    void ustaw_predkosc_wiatraka(uint32_t obroty);
    uint32_t pobierz_predkosc_wiatraka(void);
    void ustaw_rgb(uint32_t kolor, uint8_t tryb);
    void wylacz_rgb(void);

    titanx_handle_t lancuch_stworz(uint32_t szerokosc, uint32_t wysokosc, uint8_t vsync, uint8_t alpha_premultiplied);
    void lancuch_zniszcz(titanx_handle_t uchwyt);
    void prezentuj(titanx_handle_t lancuch);

    void polecenia_rozpocznij(void);
    void polecenia_zakoncz(void);
    void kontekst_pobierz(struct titanx_context *wyjscie);
    int akceleracja_3d_dostepna(void);

private:
    struct titanx_info dane;
    struct titanx_vertex_buffer bufor_wierzcholkow_tab[TITANX_MAX_VERTEX_BUFFERS];
    struct titanx_index_buffer bufor_indeksow_tab[TITANX_MAX_INDEX_BUFFERS];
    struct titanx_texture tekstury_tab[TITANX_MAX_TEXTURES];
    struct titanx_render_target cele_tab[TITANX_MAX_RENDER_TARGETS];
    struct titanx_depth_stencil bufor_glebii_tab[TITANX_MAX_DEPTH_STENCILS];
    struct titanx_shader shadery_tab[TITANX_MAX_SHADERS];
    struct titanx_sampler_state probniki_tab[TITANX_MAX_SAMPLERS];
    struct titanx_blend_state stany_mieszania_tab[TITANX_MAX_BLEND_STATES];
    struct titanx_rasterizer_state stany_rasteryzacji_tab[TITANX_MAX_RASTERIZER_STATES];
    struct titanx_depth_stencil_state stany_glebii_tab[TITANX_MAX_DEPTH_STENCIL_STATES];
    struct titanx_input_layout uklady_tab[TITANX_MAX_INPUT_LAYOUTS];
    struct titanx_constant_buffer bufor_stale_tab[TITANX_MAX_CONSTANT_BUFFERS];
    struct titanx_swap_chain lancuchy_tab[TITANX_MAX_SWAP_CHAINS];
    struct titanx_fence ogrodzenia_tab[TITANX_MAX_FENCES];
    struct titanx_semaphore semafory_tab[TITANX_MAX_SEMAPHORES];
    struct titanx_vram_alloc alokacje_tab[TITANX_VRAM_MAX_ALLOCS];
    struct titanx_context kontekst;

    uint8_t inicjalizowane;
    uint32_t mmio_base;
    uint32_t pci_bus;
    uint32_t pci_slot;
    uint32_t pci_funkcja;
    uint8_t *ramka_systemowa;
    uint32_t wierzcholek_polecenia;
    uint32_t wierzcholek_pierwszy;
    uint8_t bufor_polecenia[TITANX_COMMAND_BUFFER_SIZE];

    static uint32_t pci_odczytaj32(uint8_t bus, uint8_t slot, uint8_t funkcja, uint8_t przesuniecie);
    static void pci_zapisz32(uint8_t bus, uint8_t slot, uint8_t funkcja, uint8_t przesuniecie, uint32_t wartosc);
    uint32_t odczytaj_rejestr(uint32_t rejestr);
    void zapisz_rejestr(uint32_t rejestr, uint32_t wartosc);
    int znajdz_urzadzenie(void);
    void inicjalizuj_ring_buffer(void);
    void inicjalizuj_pamiec(void);
    void inicjalizuj_wyswietlacz(void);
    void resetuj_potok(void);
    uint64_t alokuj_vram(uint64_t rozmiar, uint32_t typ);
    void zwolnij_vram(uint64_t uchwyt);
};

TitanX *titanx_pobierz(void);

#endif

#ifdef __cplusplus
extern "C" {
#endif

int titanx_init(void);
void titanx_shutdown(void);
uint8_t titanx_czy_gotowy(void);
void titanx_putpixel(uint32_t x, uint32_t y, uint32_t kolor);
void titanx_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor);
void titanx_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void titanx_cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz);
void titanx_cursor_hide(void);
void *titanx_get_fb(void);
uint32_t titanx_get_szerokosc(void);
uint32_t titanx_get_wysokosc(void);
uint16_t titanx_get_device_id(void);
uint8_t titanx_get_rewizja(void);
void titanx_info_pobierz(struct titanx_info *wyjscie);

uint64_t titanx_vram_alokuj(uint64_t rozmiar, uint32_t typ);
void titanx_vram_zwolnij(uint64_t uchwyt);
uint64_t titanx_vram_uzyto(void);
uint64_t titanx_vram_calkowicie(void);

titanx_handle_t titanx_bufor_wierzcholkow_stworz(uint32_t liczba_wierzcholkow, uint32_t rozmiar_wierzcholka, uint32_t format, const void *dane);
void titanx_bufor_wierzcholkow_zniszcz(titanx_handle_t uchwyt);
void titanx_bufor_wierzcholkow_zaktualizuj(titanx_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

titanx_handle_t titanx_bufor_indeksow_stworz(uint32_t liczba_indeksow, uint32_t format, const void *dane);
void titanx_bufor_indeksow_zniszcz(titanx_handle_t uchwyt);
void titanx_bufor_indeksow_zaktualizuj(titanx_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

titanx_handle_t titanx_tekstura_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint32_t poziomy_mip, const void *dane);
void titanx_tekstura_zniszcz(titanx_handle_t uchwyt);
void titanx_tekstura_zaktualizuj(titanx_handle_t uchwyt, uint32_t poziom, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *dane);
void titanx_tekstura_wygeneruj_mipmapy(titanx_handle_t uchwyt);

titanx_handle_t titanx_cel_renderowania_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t alpha_blend);
void titanx_cel_renderowania_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_bufor_glebii_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t kompresja);
void titanx_bufor_glebii_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_shader_stworz(uint32_t typ, const uint8_t *kod, uint32_t rozmiar_kodu);
void titanx_shader_zniszcz(titanx_handle_t uchwyt);
void titanx_shader_skompiluj(titanx_handle_t uchwyt);

titanx_handle_t titanx_stan_probnika_stworz(uint32_t filtr_min, uint32_t filtr_mag, uint32_t filtr_mip, uint32_t adres_u, uint32_t adres_v, uint32_t adres_w, uint32_t max_anizotropia, uint8_t wlacz_porownanie, uint8_t func_porownania);
void titanx_stan_probnika_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_stan_mieszania_stworz(uint8_t wlacz_mieszanie, uint32_t src_blend, uint32_t dest_blend, uint32_t op_mieszania, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t op_mieszania_alpha, uint32_t maska_zapisu, uint32_t blend_factor);
void titanx_stan_mieszania_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_stan_rasteryzacji_stworz(uint32_t tryb_wypelnienia, uint32_t tryb_odcinania, uint8_t front_ccw, uint8_t clip_glebii, uint8_t scissor, uint8_t wielopr_probowanie, uint8_t aa_linia, uint8_t alpha_do_pokrycia);
void titanx_stan_rasteryzacji_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_stan_glebii_stencil_stworz(uint8_t wlacz_glebie, uint32_t maska_zapisu_glebii, uint32_t func_glebii, uint8_t wlacz_stencil, uint8_t maska_odczytu_stencil, uint8_t maska_zapisu_stencil, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void titanx_stan_glebii_stencil_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_uklad_wejsc_stworz(uint32_t liczba_elementow, const uint32_t *sloty, const uint32_t *przesuniecia, const uint32_t *formaty);
void titanx_uklad_wejsc_zniszcz(titanx_handle_t uchwyt);

titanx_handle_t titanx_bufor_stale_stworz(uint32_t rozmiar);
void titanx_bufor_stale_zniszcz(titanx_handle_t uchwyt);
void titanx_bufor_stale_zaktualizuj(titanx_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane);

void titanx_powiaz_bufor_wierzcholkow(titanx_handle_t uchwyt);
void titanx_powiaz_bufor_indeksow(titanx_handle_t uchwyt);
void titanx_powiaz_uklad_wejsc(titanx_handle_t uchwyt);
void titanx_powiaz_shader_wierzcholkow(titanx_handle_t uchwyt);
void titanx_powiaz_shader_pikseli(titanx_handle_t uchwyt);
void titanx_powiaz_shader_geometrii(titanx_handle_t uchwyt);
void titanx_powiaz_shader_tess_kontrola(titanx_handle_t uchwyt);
void titanx_powiaz_shader_tess_evaluacja(titanx_handle_t uchwyt);
void titanx_powiaz_shader_obliczeniowy(titanx_handle_t uchwyt);
void titanx_powiaz_cel_renderowania(titanx_handle_t uchwyt);
void titanx_powiaz_bufor_glebii(titanx_handle_t uchwyt);
void titanx_powiaz_stan_mieszania(titanx_handle_t uchwyt);
void titanx_powiaz_stan_rasteryzacji(titanx_handle_t uchwyt);
void titanx_powiaz_stan_glebii_stencil(titanx_handle_t uchwyt);
void titanx_powiaz_probnik_i_teksture(uint32_t gniazdo, titanx_handle_t probnik, titanx_handle_t tekstura);
void titanx_powiaz_bufor_stale(uint32_t gniazdo, titanx_handle_t uchwyt);

void titanx_ustaw_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_glebia, float max_glebia);
void titanx_ustaw_scissor(uint32_t lewa, uint32_t gora, uint32_t prawa, uint32_t dol);
void titanx_ustaw_typ_pierwotnego(uint32_t typ_pierwotnego);
void titanx_ustaw_tessellation(uint32_t tryb, uint32_t factor);
void titanx_ustaw_mieszanie_alpha_globalne(uint8_t wlacz);

void titanx_wyczysc_cel_renderowania(titanx_handle_t cel, uint32_t kolor);
void titanx_wyczysc_bufor_glebii(titanx_handle_t bufor, float glebia, uint8_t stencil);
void titanx_rysuj(uint32_t liczba_wierzcholkow, uint32_t pierwszy_wierzcholek);
void titanx_rysuj_indeksowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka);
void titanx_rysuj_indeksowane_instancjonowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka, uint32_t liczba_instancji, uint32_t pierwsza_instancja);
void titanx_rysuj_po_indirekcie(uint32_t adres_bufora, uint32_t liczba_rysowan, uint32_t krok);

void titanx_oblicz_wyslij(uint32_t grupa_x, uint32_t grupa_y, uint32_t grupa_z);
void titanx_oblicz_wyslij_po_indirekcie(uint32_t adres_bufora);

void titanx_ogrodzenie_stworz(titanx_handle_t *wyjscie);
void titanx_ogrodzenie_sygnalizuj(titanx_handle_t uchwyt, uint32_t wartosc);
int titanx_ogrodzenie_poczekaj(titanx_handle_t uchwyt, uint32_t wartosc, uint32_t timeout_us);
void titanx_ogrodzenie_zniszcz(titanx_handle_t uchwyt);

void titanx_semafor_stworz(titanx_handle_t *wyjscie);
void titanx_semafor_sygnalizuj(titanx_handle_t uchwyt, uint32_t wartosc);
void titanx_semafor_poczekaj(titanx_handle_t uchwyt, uint32_t wartosc);
void titanx_semafor_zniszcz(titanx_handle_t uchwyt);

int titanx_czekaj_na_bezczynnosc(uint32_t timeout_us);
uint64_t titanx_znacznik_czasu(void);

void titanx_ustaw_stan_zasilania(uint8_t stan);
uint8_t titanx_pobierz_stan_zasilania(void);
void titanx_ustaw_zegar_graficzny(uint32_t mhz);
void titanx_ustaw_zegar_pamieci(uint32_t mhz);
uint32_t titanx_pobierz_zegar_graficzny(void);
uint32_t titanx_pobierz_zegar_pamieci(void);
uint32_t titanx_pobierz_temperature(void);
void titanx_ustaw_predkosc_wiatraka(uint32_t obroty);
uint32_t titanx_pobierz_predkosc_wiatraka(void);
void titanx_ustaw_rgb(uint32_t kolor, uint8_t tryb);
void titanx_wylacz_rgb(void);

titanx_handle_t titanx_lancuch_stworz(uint32_t szerokosc, uint32_t wysokosc, uint8_t vsync, uint8_t alpha_premultiplied);
void titanx_lancuch_zniszcz(titanx_handle_t uchwyt);
void titanx_prezentuj(titanx_handle_t lancuch);

void titanx_polecenia_rozpocznij(void);
void titanx_polecenia_zakoncz(void);
void titanx_kontekst_pobierz(struct titanx_context *wyjscie);
int titanx_akceleracja_3d_dostepna(void);

#ifdef __cplusplus
}
#endif

#endif
