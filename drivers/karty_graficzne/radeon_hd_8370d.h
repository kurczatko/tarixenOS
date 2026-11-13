#ifndef RADEON_HD_8370D_H
#define RADEON_HD_8370D_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*Zaawansowany sterownik AMD Radeon HD 8370D (GCN, Graphics Core Next, chipset Richland) napisany w C++.
 *Obsluguje pelny potok graficzny 2D i 3D, akceleracje sprzetowa, przezroczystosc (alpha blending),
 *tekstury, shadery, render targety, depth-stencil, command buffer oraz presentation.
 *Integracja z graficzny/rysowanie.c przez funkcje putpixel/fillrect/blit/cursor.*/

int radeon_hd_8370d_init(void);
int radeon_hd_8370d_is_ready(void);
void radeon_hd_8370d_putpixel(uint32_t x, uint32_t y, uint32_t color);
void radeon_hd_8370d_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void radeon_hd_8370d_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void radeon_hd_8370d_cursor_set(uint32_t x, uint32_t y, const uint32_t *image);
void radeon_hd_8370d_cursor_hide(void);
void *radeon_hd_8370d_get_fb(void);
uint32_t radeon_hd_8370d_get_width(void);
uint32_t radeon_hd_8370d_get_height(void);
uint16_t radeon_hd_8370d_get_device_id(void);
uint8_t radeon_hd_8370d_get_revision(void);

/*Struktura informacji o urzadzeniu*/
struct radeon_hd_8370d_info {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint32_t mmio_base;
    uint32_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint8_t initialized;
    uint8_t gcn_generation;
};
void radeon_hd_8370d_info_get(struct radeon_hd_8370d_info *out);

/*================== AKCELERACJA 3D GCN ==================*/

/*Typy formatow vertex*/
#define RADEON_VERTEX_FORMAT_XYZ          0x0001U
#define RADEON_VERTEX_FORMAT_XYZW         0x0002U
#define RADEON_VERTEX_FORMAT_XYZ_COLOR    0x0004U
#define RADEON_VERTEX_FORMAT_XYZ_TEX      0x0008U
#define RADEON_VERTEX_FORMAT_XYZ_COLOR_TEX 0x0010U
#define RADEON_VERTEX_FORMAT_XYZ_COLOR_TEX2 0x0020U

/*Typy indeksow*/
#define RADEON_INDEX_FORMAT_16BIT         0x0000U
#define RADEON_INDEX_FORMAT_32BIT         0x0001U

/*Typy primitivow*/
#define RADEON_PRIM_POINTLIST             0x0000U
#define RADEON_PRIM_LINELIST              0x0001U
#define RADEON_PRIM_LINESTRIP             0x0002U
#define RADEON_PRIM_TRIANGLELIST          0x0003U
#define RADEON_PRIM_TRIANGLESTRIP         0x0004U
#define RADEON_PRIM_TRIANGLEFAN           0x0005U
#define RADEON_PRIM_QUADLIST              0x0006U
#define RADEON_PRIM_QUADSTRIP             0x0007U

/*Formaty tekstur*/
#define RADEON_TEX_FORMAT_A8R8G8B8        0x0000U
#define RADEON_TEX_FORMAT_X8R8G8B8        0x0001U
#define RADEON_TEX_FORMAT_A8B8G8R8        0x0002U
#define RADEON_TEX_FORMAT_X8B8G8R8        0x0003U
#define RADEON_TEX_FORMAT_R5G6B5          0x0004U
#define RADEON_TEX_FORMAT_DXT1            0x0010U
#define RADEON_TEX_FORMAT_DXT3            0x0011U
#define RADEON_TEX_FORMAT_DXT5            0x0012U
#define RADEON_TEX_FORMAT_FLOAT32_RGBA    0x0020U

/*Typy shaderow GCN*/
#define RADEON_SHADER_VERTEX              0x0000U
#define RADEON_SHADER_PIXEL               0x0001U
#define RADEON_SHADER_GEOMETRY            0x0002U
#define RADEON_SHADER_COMPUTE             0x0003U

/*Stany blendowania (przezroczystosc)*/
#define RADEON_BLEND_ZERO                 0x0000U
#define RADEON_BLEND_ONE                  0x0001U
#define RADEON_BLEND_SRC_COLOR            0x0002U
#define RADEON_BLEND_DEST_COLOR           0x0003U
#define RADEON_BLEND_SRC_ALPHA            0x0004U
#define RADEON_BLEND_DEST_ALPHA           0x0005U
#define RADEON_BLEND_INV_SRC_ALPHA        0x0006U
#define RADEON_BLEND_INV_DEST_ALPHA       0x0007U
#define RADEON_BLEND_SRC_ALPHA_SAT        0x0008U
#define RADEON_BLEND_BLEND_FACTOR         0x0009U
#define RADEON_BLEND_INV_BLEND_FACTOR     0x000AU

/*Operacje blendowania*/
#define RADEON_BLEND_OP_ADD               0x0000U
#define RADEON_BLEND_OP_SUBTRACT          0x0001U
#define RADEON_BLEND_OP_REV_SUBTRACT      0x0002U
#define RADEON_BLEND_OP_MIN               0x0003U
#define RADEON_BLEND_OP_MAX               0x0004U

/*Funkcje porownania depth*/
#define RADEON_CMP_NEVER                  0x0000U
#define RADEON_CMP_LESS                   0x0001U
#define RADEON_CMP_EQUAL                  0x0002U
#define RADEON_CMP_LEQUAL                 0x0003U
#define RADEON_CMP_GREATER                0x0004U
#define RADEON_CMP_NOTEQUAL               0x0005U
#define RADEON_CMP_GEQUAL                 0x0006U
#define RADEON_CMP_ALWAYS                 0x0007U

/*Tryby wypelnienia rasterizera*/
#define RADEON_FILL_SOLID                 0x0000U
#define RADEON_FILL_WIREFRAME             0x0001U

/*Tryby cullowania*/
#define RADEON_CULL_NONE                  0x0000U
#define RADEON_CULL_FRONT                 0x0001U
#define RADEON_CULL_BACK                  0x0002U

/*Stencil operacje*/
#define RADEON_STENCIL_KEEP               0x0000U
#define RADEON_STENCIL_ZERO               0x0001U
#define RADEON_STENCIL_REPLACE            0x0002U
#define RADEON_STENCIL_INCR               0x0003U
#define RADEON_STENCIL_DECR               0x0004U
#define RADEON_STENCIL_INVERT             0x0005U

/*Maksymalna liczba zasobow 3D (styczne alokacje, bez malloc)*/
#define RADEON_MAX_VERTEX_BUFFERS         32U
#define RADEON_MAX_INDEX_BUFFERS          32U
#define RADEON_MAX_TEXTURES               64U
#define RADEON_MAX_RENDER_TARGETS         8U
#define RADEON_MAX_DEPTH_STENCILS         8U
#define RADEON_MAX_SHADERS                32U
#define RADEON_MAX_SAMPLERS               16U
#define RADEON_MAX_BLEND_STATES           16U
#define RADEON_MAX_RASTERIZER_STATES      16U
#define RADEON_MAX_DEPTH_STENCIL_STATES   16U
#define RADEON_MAX_INPUT_LAYOUTS          32U
#define RADEON_MAX_CONSTANT_BUFFERS       16U
#define RADEON_COMMAND_BUFFER_SIZE        8192U

/*Handle zasobu*/
typedef uint32_t radeon_handle_t;
#define RADEON_HANDLE_NULL 0xFFFFFFFFU

/*Vertex buffer - wierzcholki w pamieci wideo lub systemowej*/
struct radeon_hd_8370d_vertex_buffer {
    uint8_t used;
    uint32_t vertex_count;
    uint32_t vertex_size;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
};

/*Index buffer - indeksy wierzcholkow*/
struct radeon_hd_8370d_index_buffer {
    uint8_t used;
    uint32_t index_count;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t *system_mem;
};

/*Tekstura 2D z obsuga mipmap*/
struct radeon_hd_8370d_texture {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t mip_levels;
    uint32_t buffer_addr;
    uint8_t *system_mem;
    uint32_t pitch;
};

/*Render target (bufor koloru do renderowania 3D z przezroczystoscia)*/
struct radeon_hd_8370d_render_target {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
    uint8_t alpha_blend;   /*wlaczenie blendowania alfa dla tego celu*/
};

/*Depth-stencil buffer (bufor glebi i stencil do przezroczystosci)*/
struct radeon_hd_8370d_depth_stencil {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t buffer_addr;
};

/*Shader GCN (vertex/pixel/geometry/compute) - kod bajtowy ISA GPU*/
struct radeon_hd_8370d_shader {
    uint8_t used;
    uint32_t type;
    uint32_t code_size;
    uint8_t *code;
    uint32_t constant_count;
};

/*Stan probki tekstury*/
struct radeon_hd_8370d_sampler_state {
    uint8_t used;
    uint32_t filter_min;
    uint32_t filter_mag;
    uint32_t filter_mip;
    uint32_t address_u;
    uint32_t address_v;
    uint32_t address_w;
    uint32_t max_anisotropy;
    uint8_t compare_enable;
};

/*Stan blendowania (przezroczystosc alpha)*/
struct radeon_hd_8370d_blend_state {
    uint8_t used;
    uint8_t blend_enable;
    uint32_t src_blend;
    uint32_t dest_blend;
    uint32_t blend_op;
    uint32_t src_blend_alpha;
    uint32_t dest_blend_alpha;
    uint32_t blend_op_alpha;
    uint32_t render_target_write_mask;
    uint32_t blend_factor;   /*staly kolor blendowania dla przezroczystosci*/
};

/*Stan rasterizera*/
struct radeon_hd_8370d_rasterizer_state {
    uint8_t used;
    uint32_t fill_mode;
    uint32_t cull_mode;
    uint8_t front_counter_clockwise;
    uint8_t depth_clip_enable;
    uint8_t scissor_enable;
    uint8_t multisample_enable;
    uint8_t antialiased_line_enable;
    uint8_t alpha_to_coverage;   /*przezroczystosc do pokrycia multisample*/
};

/*Stan depth-stencil*/
struct radeon_hd_8370d_depth_stencil_state {
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

/*Layout wejsciowy vertex*/
struct radeon_hd_8370d_input_layout {
    uint8_t used;
    uint32_t element_count;
    uint32_t slot[32];
    uint32_t offset[32];
    uint32_t format[32];
};

/*Stale shadera (uniforms/constant buffer)*/
struct radeon_hd_8370d_constant_buffer {
    uint8_t used;
    uint32_t size;
    uint8_t *data;
};

/*Swap chain (podwojne buforowanie / present z przezroczystoscia)*/
struct radeon_hd_8370d_swap_chain {
    uint8_t used;
    uint32_t width;
    uint32_t height;
    uint32_t back_buffer_addr;
    uint32_t front_buffer_addr;
    uint8_t vsync;
    uint8_t alpha_premultiplied;   /*premultiplied alpha dla przezroczystosci*/
};

/*Biezacy kontekst 3D (stan pipeline GCN)*/
struct radeon_hd_8370d_context {
    radeon_handle_t vertex_buffer;
    radeon_handle_t index_buffer;
    radeon_handle_t input_layout;
    radeon_handle_t vertex_shader;
    radeon_handle_t pixel_shader;
    radeon_handle_t geometry_shader;
    radeon_handle_t compute_shader;
    radeon_handle_t render_target;
    radeon_handle_t depth_stencil;
    radeon_handle_t blend_state;
    radeon_handle_t rasterizer_state;
    radeon_handle_t depth_stencil_state;
    radeon_handle_t sampler[RADEON_MAX_SAMPLERS];
    radeon_handle_t texture[RADEON_MAX_SAMPLERS];
    radeon_handle_t constant_buffer[RADEON_MAX_CONSTANT_BUFFERS];
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
    uint8_t alpha_blend_global;   /*globalne wlaczenie przezroczystosci alpha*/
    uint8_t in_command_buffer;
};

/*================== FUNKCJE ZASOBOW 3D ==================*/

radeon_handle_t radeon_hd_8370d_vertex_buffer_create(uint32_t vertex_count, uint32_t vertex_size, uint32_t format, const void *data);
void radeon_hd_8370d_vertex_buffer_destroy(radeon_handle_t handle);
void radeon_hd_8370d_vertex_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_handle_t radeon_hd_8370d_index_buffer_create(uint32_t index_count, uint32_t format, const void *data);
void radeon_hd_8370d_index_buffer_destroy(radeon_handle_t handle);
void radeon_hd_8370d_index_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data);

radeon_handle_t radeon_hd_8370d_texture_create(uint32_t width, uint32_t height, uint32_t format, uint32_t mip_levels, const void *data);
void radeon_hd_8370d_texture_destroy(radeon_handle_t handle);
void radeon_hd_8370d_texture_update(radeon_handle_t handle, uint32_t level, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *data);
void radeon_hd_8370d_texture_generate_mipmaps(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_render_target_create(uint32_t width, uint32_t height, uint32_t format, uint8_t alpha_blend);
void radeon_hd_8370d_render_target_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_depth_stencil_create(uint32_t width, uint32_t height, uint32_t format);
void radeon_hd_8370d_depth_stencil_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_shader_create(uint32_t type, const uint8_t *code, uint32_t code_size);
void radeon_hd_8370d_shader_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_sampler_state_create(uint32_t filter_min, uint32_t filter_mag, uint32_t filter_mip, uint32_t address_u, uint32_t address_v, uint32_t address_w, uint32_t max_anisotropy, uint8_t compare_enable);
void radeon_hd_8370d_sampler_state_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_blend_state_create(uint8_t blend_enable, uint32_t src_blend, uint32_t dest_blend, uint32_t blend_op, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t blend_op_alpha, uint32_t write_mask, uint32_t blend_factor);
void radeon_hd_8370d_blend_state_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_rasterizer_state_create(uint32_t fill_mode, uint32_t cull_mode, uint8_t front_ccw, uint8_t depth_clip, uint8_t scissor, uint8_t multisample, uint8_t aa_line, uint8_t alpha_to_coverage);
void radeon_hd_8370d_rasterizer_state_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_depth_stencil_state_create(uint8_t depth_enable, uint32_t depth_write_mask, uint32_t depth_func, uint8_t stencil_enable, uint8_t stencil_read_mask, uint8_t stencil_write_mask, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func);
void radeon_hd_8370d_depth_stencil_state_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_input_layout_create(uint32_t element_count, const uint32_t *slots, const uint32_t *offsets, const uint32_t *formats);
void radeon_hd_8370d_input_layout_destroy(radeon_handle_t handle);

radeon_handle_t radeon_hd_8370d_constant_buffer_create(uint32_t size);
void radeon_hd_8370d_constant_buffer_destroy(radeon_handle_t handle);
void radeon_hd_8370d_constant_buffer_update(radeon_handle_t handle, uint32_t offset, uint32_t size, const void *data);

/*================== FUNKCJE BINDOWANIA STANOW ==================*/

void radeon_hd_8370d_bind_vertex_buffer(radeon_handle_t handle);
void radeon_hd_8370d_bind_index_buffer(radeon_handle_t handle);
void radeon_hd_8370d_bind_input_layout(radeon_handle_t handle);
void radeon_hd_8370d_bind_vertex_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_pixel_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_geometry_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_compute_shader(radeon_handle_t handle);
void radeon_hd_8370d_bind_render_target(radeon_handle_t handle);
void radeon_hd_8370d_bind_depth_stencil(radeon_handle_t handle);
void radeon_hd_8370d_bind_blend_state(radeon_handle_t handle);
void radeon_hd_8370d_bind_rasterizer_state(radeon_handle_t handle);
void radeon_hd_8370d_bind_depth_stencil_state(radeon_handle_t handle);
void radeon_hd_8370d_bind_sampler_and_texture(uint32_t slot, radeon_handle_t sampler, radeon_handle_t texture);
void radeon_hd_8370d_bind_constant_buffer(uint32_t slot, radeon_handle_t handle);

/*================== FUNKCJE KONFIGURACJI POTOKU 3D ==================*/

void radeon_hd_8370d_set_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_depth, float max_depth);
void radeon_hd_8370d_set_scissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
void radeon_hd_8370d_set_primitive_topology(uint32_t topology);
void radeon_hd_8370d_set_alpha_blend_global(uint8_t enable);

/*================== FUNKCJE RYSOWANIA 3D ==================*/

void radeon_hd_8370d_clear_render_target(radeon_handle_t rt, uint32_t color);
void radeon_hd_8370d_clear_depth_stencil(radeon_handle_t ds, float depth, uint8_t stencil);
void radeon_hd_8370d_draw(uint32_t vertex_count, uint32_t start_vertex);
void radeon_hd_8370d_draw_indexed(uint32_t index_count, uint32_t start_index, int32_t base_vertex);

/*================== FUNKCJE COMMAND BUFFER ==================*/

void radeon_hd_8370d_command_begin(void);
void radeon_hd_8370d_command_end(void);

/*================== FUNKCJE PRESENTATION ==================*/

radeon_handle_t radeon_hd_8370d_swap_chain_create(uint32_t width, uint32_t height, uint8_t vsync, uint8_t alpha_premultiplied);
void radeon_hd_8370d_swap_chain_destroy(radeon_handle_t handle);
void radeon_hd_8370d_present(radeon_handle_t chain);

/*================== FUNKCJE INFORMACJI 3D ==================*/

void radeon_hd_8370d_context_get(struct radeon_hd_8370d_context *out);
int radeon_hd_8370d_3d_available(void);

#ifdef __cplusplus
}
#endif

#endif
