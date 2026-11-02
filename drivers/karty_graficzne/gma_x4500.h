#ifndef GMA_X4500_H
#define GMA_X4500_H

#include "../../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Zaawansowany sterownik Intel GMA X4500HD (Gen4 GPU, chipset G45).
 *Obsluguje potok wyswietlania, blitter 2D, akceleracje 3D, ramke graficzna, kursor oraz rejestry MMIO.
 *Integracja z graficzny/rysowanie.c przez funkcje putpixel/fillrect/blit/cursor.*/

int gma_x4500_init(void);
int gma_x4500_is_ready(void);
void gma_x4500_putpixel(uint32_t x, uint32_t y, uint32_t color);
void gma_x4500_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void gma_x4500_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void gma_x4500_cursor_set(uint32_t x, uint32_t y, const uint32_t *image);
void gma_x4500_cursor_hide(void);
void *gma_x4500_get_fb(void);
uint32_t gma_x4500_get_width(void);
uint32_t gma_x4500_get_height(void);
uint16_t gma_x4500_get_device_id(void);
uint8_t gma_x4500_get_revision(void);

/*Struktura informacji o urzadzeniu*/
struct gma_x4500_info {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint32_t mmio_base;
    uint32_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint8_t initialized;
};
void gma_x4500_info_get(struct gma_x4500_info *out);

/*================== AKCELERACJA 3D ==================*/

/*Typy formatow vertex*/
#define GMA_VERTEX_FORMAT_XYZ          0x0001U
#define GMA_VERTEX_FORMAT_XYZW         0x0002U
#define GMA_VERTEX_FORMAT_XYZ_COLOR    0x0004U
#define GMA_VERTEX_FORMAT_XYZ_TEX      0x0008U
#define GMA_VERTEX_FORMAT_XYZ_COLOR_TEX 0x0010U

/*Typy indeksow*/
#define GMA_INDEX_FORMAT_16BIT         0x0000U
#define GMA_INDEX_FORMAT_32BIT         0x0001U

/*Typy primitivow*/
#define GMA_PRIM_POINTLIST             0x0000U
#define GMA_PRIM_LINELIST              0x0001U
#define GMA_PRIM_LINESTRIP             0x0002U
#define GMA_PRIM_TRIANGLELIST          0x0003U
#define GMA_PRIM_TRIANGLESTRIP         0x0004U
#define GMA_PRIM_TRIANGLEFAN           0x0005U

/*Formaty tekstur*/
#define GMA_TEX_FORMAT_A8R8G8B8        0x0000U
#define GMA_TEX_FORMAT_X8R8G8B8        0x0001U
#define GMA_TEX_FORMAT_A8B8G8R8        0x0002U
#define GMA_TEX_FORMAT_X8B8G8R8        0x0003U
#define GMA_TEX_FORMAT_DXT1            0x0010U
#define GMA_TEX_FORMAT_DXT3            0x0011U
#define GMA_TEX_FORMAT_DXT5            0x0012U

/*Typy shaderow*/
#define GMA_SHADER_VERTEX              0x0000U
#define GMA_SHADER_PIXEL               0x0001U
#define GMA_SHADER_GEOMETRY            0x0002U

/*Stany blendowania*/
#define GMA_BLEND_ZERO                 0x0000U
#define GMA_BLEND_ONE                  0x0001U
#define GMA_BLEND_SRC_COLOR            0x0002U
#define GMA_BLEND_DEST_COLOR           0x0003U
#define GMA_BLEND_SRC_ALPHA            0x0004U
#define GMA_BLEND_DEST_ALPHA           0x0005U
#define GMA_BLEND_INV_SRC_ALPHA        0x0006U
#define GMA_BLEND_INV_DEST_ALPHA       0x0007U

/*Funkcje porownania depth*/
#define GMA_CMP_NEVER                  0x0000U
#define GMA_CMP_LESS                   0x0001U
#define GMA_CMP_EQUAL                  0x0002U
#define GMA_CMP_LEQUAL                 0x0003U
#define GMA_CMP_GREATER                0x0004U
#define GMA_CMP_NOTEQUAL               0x0005U
#define GMA_CMP_GEQUAL                 0x0006U
#define GMA_CMP_ALWAYS                 0x0007U

/*Tryby wypelnienia rasterizera*/
#define GMA_FILL_SOLID                 0x0000U
#define GMA_FILL_WIREFRAME             0x0001U

/*Tryby cullowania*/
#define GMA_CULL_NONE                  0x0000U
#define GMA_CULL_FRONT                 0x0001U
#define GMA_CULL_BACK                  0x0002U

/*Maksymalna liczba zasobow 3D (styczne alokacje, brak malloc)*/
#define GMA_MAX_VERTEX_BUFFERS         16U
#define GMA_MAX_INDEX_BUFFERS          16U
#define GMA_MAX_TEXTURES               32U
#define GMA_MAX_RENDER_TARGETS         4U
#define GMA_MAX_DEPTH_STENCILS         4U
#define GMA_MAX_SHADERS                16U
#define GMA_MAX_SAMPLERS               8U
#define GMA_MAX_BLEND_STATES           8U
#define GMA_MAX_RASTERIZER_STATES      8U
#define GMA_MAX_DEPTH_STENCIL_STATES   8U
#define GMA_MAX_INPUT_LAYOUTS          16U
#define GMA_MAX_CONSTANT_BUFFERS       8U
#define GMA_COMMAND_BUFFER_SIZE        4096U

/*Handle zasobu (indeks w tablicy lub 0xFFFFFFFF gdy wolny)*/
typedef uint32_t gma_handle_t;
#define GMA_HANDLE_NULL 0xFFFFFFFFU

/*Vertex buffer - wierzcholki w pamieci systemowej lub wideo*/
struct gma_x4500_vertex_buffer {
    uint8_t used;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t format;
    uint32_t buffer_addr;   /*adres w FB lub w pamieci systemowej*/
    uint8_t *system_mem;    /*wskaznik do pamieci systemowej (jesli nie w FB)*/
};

/*Index buffer - indeksy wierzcholkow*/
struct gma_x4500_index_buffer {
    uint8_t used;
    uint32_t index_count;
    uint32_t format;        /*GMA_INDEX_FORMAT_16BIT lub 32BIT*/
    uint32_t buffer_addr;
    uint8_t *system_mem;
};

/*Tekstura 2D*/
struct gma_x4500_texture {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t mip_levels;
    uint32_t buffer_addr;   /*adres w FB*/
    uint8_t *system_mem;    /*kopia w pamieci systemowej*/
    uint32_t pitch;
};

/*Render target (bufor koloru do renderowania)*/
struct gma_x4500_render_target {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
};

/*Depth-stencil buffer (bufor glebi i stencil)*/
struct gma_x4500_depth_stencil {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
};

/*Shader (vertex/pixel/geometry) - kod bajtowy GPU*/
struct gma_x4500_shader {
    uint8_t used;
    uint32_t type;          /*GMA_SHADER_* */
    uint32_t code_size;
    uint8_t *code;          /*kod shadera*/
    uint32_t constant_count;
};

/*Stan probki tekstury*/
struct gma_x4500_sampler_state {
    uint8_t used;
    uint32_t filter;        /*filtrowania min/mag/mip*/
    uint32_t address_u;     /*adresowanie tekstury*/
    uint32_t address_v;
    uint32_t address_w;
    uint32_t max_anisotropy;
};

/*Stan blendowania (kolor wyjsciowy vs render target)*/
struct gma_x4500_blend_state {
    uint8_t used;
    uint8_t blend_enable;
    uint32_t src_blend;
    uint32_t dest_blend;
    uint32_t blend_op;
    uint32_t src_blend_alpha;
    uint32_t dest_blend_alpha;
    uint32_t blend_op_alpha;
    uint32_t render_target_write_mask;
};

/*Stan rasterizera (wypelnienie, cullowanie, scissor, multisample)*/
struct gma_x4500_rasterizer_state {
    uint8_t used;
    uint32_t fill_mode;
    uint32_t cull_mode;
    uint8_t front_counter_clockwise;
    uint8_t depth_clip_enable;
    uint8_t scissor_enable;
    uint8_t multisample_enable;
    uint8_t antialiased_line_enable;
};

/*Stan depth-stencil (testy glebi i stencil)*/
struct gma_x4500_depth_stencil_state {
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

/*Layout wejsciowy vertex (mapowanie atrybutow do slotow)*/
struct gma_x4500_input_layout {
    uint8_t used;
    uint32_t element_count;
    uint32_t slot[16];
    uint32_t offset[16];
    uint32_t format[16];
};

/*Stale shadera (uniforms/constant buffer)*/
struct gma_x4500_constant_buffer {
    uint8_t used;
    uint32_t size;
    uint8_t *data;
};

/*Swap chain (podwojne buforowanie / present)*/
struct gma_x4500_swap_chain {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t back_buffer_addr;
    uint32_t front_buffer_addr;
    uint8_t vsync;
};

/*Biezacy kontekst 3D (stan pipeline)*/
struct gma_x4500_context {
    gma_handle_t vertex_buffer;
    gma_handle_t index_buffer;
    gma_handle_t input_layout;
    gma_handle_t vertex_shader;
    gma_handle_t pixel_shader;
    gma_handle_t geometry_shader;
    gma_handle_t render_target;
    gma_handle_t depth_stencil;
    gma_handle_t blend_state;
    gma_handle_t rasterizer_state;
    gma_handle_t depth_stencil_state;
    gma_handle_t sampler[GMA_MAX_SAMPLERS];
    gma_handle_t texture[GMA_MAX_SAMPLERS];
    gma_handle_t constant_buffer[GMA_MAX_CONSTANT_BUFFERS];
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
    uint8_t in_command_buffer;
};

/*================== FUNKCJE ZASOBOW 3D ==================*/

/*Tworzy vertex buffer. Zwraca handle lub GMA_HANDLE_NULL przy bledzie.*/
gma_handle_t gma_x4500_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data);

/*Niszczy vertex buffer o podanym handle.*/
void gma_x4500_vertex_buffer_destroy(gma_handle_t handle);

/*Aktualizuje dane vertex buffera.*/
void gma_x4500_vertex_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data);

/*Tworzy index buffer. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_index_buffer_create(uint32_t index_count, uint32_t format, const void *data);

/*Niszczy index buffer.*/
void gma_x4500_index_buffer_destroy(gma_handle_t handle);

/*Aktualizuje dane index buffera.*/
void gma_x4500_index_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data);

/*Tworzy teksture 2D. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data);

/*Niszczy teksture.*/
void gma_x4500_texture_destroy(gma_handle_t handle);

/*Aktualizuje zawartosc tekstury (upload z pamieci systemowej do FB).*/
void gma_x4500_texture_update(gma_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);

/*Tworzy render target. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_render_target_create(uint32_t width, uint32_t height, uint32_t format);

/*Niszczy render target.*/
void gma_x4500_render_target_destroy(gma_handle_t handle);

/*Tworzy depth-stencil buffer. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format);

/*Niszczy depth-stencil.*/
void gma_x4500_depth_stencil_destroy(gma_handle_t handle);

/*Tworzy shader z podanego kodu bajtowego. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);

/*Niszczy shader.*/
void gma_x4500_shader_destroy(gma_handle_t handle);

/*Tworzy stan probki tekstury. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_sampler_state_create(uint32_t filter, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy);

/*Niszczy stan probki.*/
void gma_x4500_sampler_state_destroy(gma_handle_t handle);

/*Tworzy stan blendowania. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask);

/*Niszczy stan blendowania.*/
void gma_x4500_blend_state_destroy(gma_handle_t handle);

/*Tworzy stan rasterizera. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line);

/*Niszczy stan rasterizera.*/
void gma_x4500_rasterizer_state_destroy(gma_handle_t handle);

/*Tworzy stan depth-stencil. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);

/*Niszczy stan depth-stencil.*/
void gma_x4500_depth_stencil_state_destroy(gma_handle_t handle);

/*Tworzy input layout. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);

/*Niszczy input layout.*/
void gma_x4500_input_layout_destroy(gma_handle_t handle);

/*Tworzy constant buffer. Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_constant_buffer_create(uint32_t size);

/*Niszczy constant buffer.*/
void gma_x4500_constant_buffer_destroy(gma_handle_t handle);

/*Aktualizuje dane constant buffera.*/
void gma_x4500_constant_buffer_update(gma_handle_t handle, uint32_t offset, uint32_t size, const void *data);

/*================== FUNKCJE BINDOWANIA STANOW ==================*/

/*Binduje vertex buffer do potoku.*/
void gma_x4500_bind_vertex_buffer(gma_handle_t handle);

/*Binduje index buffer do potoku.*/
void gma_x4500_bind_index_buffer(gma_handle_t handle);

/*Binduje input layout do potoku.*/
void gma_x4500_bind_input_layout(gma_handle_t handle);

/*Binduje vertex shader do potoku.*/
void gma_x4500_bind_vertex_shader(gma_handle_t handle);

/*Binduje pixel shader do potoku.*/
void gma_x4500_bind_pixel_shader(gma_handle_t handle);

/*Binduje geometry shader do potoku.*/
void gma_x4500_bind_geometry_shader(gma_handle_t handle);

/*Binduje render target do potoku.*/
void gma_x4500_bind_render_target(gma_handle_t handle);

/*Binduje depth-stencil do potoku.*/
void gma_x4500_bind_depth_stencil(gma_handle_t handle);

/*Binduje stan blendowania do potoku.*/
void gma_x4500_bind_blend_state(gma_handle_t handle);

/*Binduje stan rasterizera do potoku.*/
void gma_x4500_bind_rasterizer_state(gma_handle_t handle);

/*Binduje stan depth-stencil do potoku.*/
void gma_x4500_bind_depth_stencil_state(gma_handle_t handle);

/*Binduje probke tekstury i teksture do slotu.*/
void gma_x4500_bind_sampler_and_texture(uint32_t slot, gma_handle_t sampler, gma_handle_t texture);

/*Binduje constant buffer do slotu.*/
void gma_x4500_bind_constant_buffer(uint32_t slot, gma_handle_t handle);

/*================== FUNKCJE KONFIGURACJI POTOKU 3D ==================*/

/*Ustawia viewport (prostokat wyswietlania w NDCS do ekranu).*/
void gma_x4500_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);

/*Ustawia scissor (prostokat przyciecia renderowania).*/
void gma_x4500_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

/*Ustawia typ primitivu do rysowania.*/
void gma_x4500_set_primitive_topology(uint32_t topology);

/*================== FUNKCJE RYSOWANIA 3D ==================*/

/*Czyści render target podanym kolorem.*/
void gma_x4500_clear_render_target(gma_handle_t rt, uint32_t color);

/*Czyści depth-stencil podanymi wartosciami.*/
void gma_x4500_clear_depth_stencil(gma_handle_t ds, float depth, uint8_t stencil);

/*Rysuje primitivy z vertex buffera (bez indeksow).*/
void gma_x4500_draw(uint32_t vertex_count, uint32_t start_vertex);

/*Rysuje indeksowane primitivy z index buffera.*/
void gma_x4500_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex);

/*================== FUNKCJE COMMAND BUFFER (akceleracja 3D w strumieniu) ==================*/

/*Rozpoczyna nagrywanie polecen do command buffer.*/
void gma_x4500_command_begin(void);

/*Zakoncza nagrywanie i wykonuje wszystkie polecenia command buffer.*/
void gma_x4500_command_end(void);

/*================== FUNKCJE PRESENTATION (swap chain) ==================*/

/*Tworzy swap chain (podwojne buforowanie). Zwraca handle lub GMA_HANDLE_NULL.*/
gma_handle_t gma_x4500_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync);

/*Niszczy swap chain.*/
void gma_x4500_swap_chain_destroy(gma_handle_t handle);

/*Przebija tylny bufor na przedni (present).*/
void gma_x4500_present(gma_handle_t chain);

/*================== FUNKCJE INFORMACJI 3D ==================*/

/*Pobiera biezacy kontekst 3D.*/
void gma_x4500_context_get(struct gma_x4500_context *out);

/*Czy akceleracja 3D jest dostepna (Gen4 pipeline).*/
int gma_x4500_3d_available(void);

#ifdef __cplusplus
}
#endif

#endif
