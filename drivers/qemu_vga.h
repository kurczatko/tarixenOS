#ifndef QEMU_VGA_H
#define QEMU_VGA_H

#include "../include/printf/types.h"

#define QEMU_VGA_VENDOR_ID 0x1AB8
#define QEMU_STD_VGA_VENDOR_ID 0x8086
#define QEMU_VGA_FB_WIDTH 1024
#define QEMU_VGA_FB_HEIGHT 768
#define QEMU_VGA_FB_ADDR 0xFD000000U
#define QEMU_VGA_MAX_VERTICES 4096
#define QEMU_VGA_MAX_TRIANGLES 8192
#define QEMU_VGA_MAX_TEXTURES 32
#define QEMU_VGA_TEX_SIZE 64

#define QEMU_VGA_REG_3D_CMD 0x200U
#define QEMU_VGA_REG_3D_STATUS 0x204U
#define QEMU_VGA_REG_3D_VTX_BASE 0x208U
#define QEMU_VGA_REG_3D_VTX_COUNT 0x20CU
#define QEMU_VGA_REG_3D_IDX_BASE 0x210U
#define QEMU_VGA_REG_3D_IDX_COUNT 0x214U
#define QEMU_VGA_REG_3D_TEX_BASE 0x218U
#define QEMU_VGA_REG_3D_PROJ_MTX 0x220U
#define QEMU_VGA_REG_3D_VIEW_MTX 0x260U
#define QEMU_VGA_REG_3D_MODEL_MTX 0x2A0U
#define QEMU_VGA_REG_3D_CLEAR_COLOR 0x2E0U
#define QEMU_VGA_REG_3D_CLEAR_DEPTH 0x2E4U
#define QEMU_VGA_REG_3D_DRAW_PRIMITIVES 0x2E8U
#define QEMU_VGA_REG_3D_BLEND_ENABLE 0x2ECU
#define QEMU_VGA_REG_3D_DEPTH_TEST 0x2F0U

#define QEMU_VGA_3D_CMD_NOP 0x00000000U
#define QEMU_VGA_3D_CMD_CLEAR 0x00000001U
#define QEMU_VGA_3D_CMD_DRAW_TRIANGLES 0x00000002U
#define QEMU_VGA_3D_CMD_SET_MATRIX 0x00000003U
#define QEMU_VGA_3D_CMD_SET_TEXTURE 0x00000004U
#define QEMU_VGA_3D_CMD_SET_BLEND 0x00000005U
#define QEMU_VGA_3D_CMD_SET_DEPTH 0x00000006U
#define QEMU_VGA_3D_CMD_FLUSH 0x00000007U

#define QEMU_VGA_3D_STATUS_IDLE 0x00000000U
#define QEMU_VGA_3D_STATUS_BUSY 0x00000001U
#define QEMU_VGA_3D_STATUS_ERROR 0x00000002U

typedef struct {
    float position[4];
    uint8_t color[4];
    float texcoord[2];
} qv3_vtx_attr_t;

#define QEMU_VGA_3D_MAX_VTX 4096
#define QEMU_VGA_3D_MAX_IDX 8192
#define QEMU_VGA_3D_MAX_CMDS 256
#define QEMU_VGA_3D_VTX_BUFFER_ADDR 0x10000000U
#define QEMU_VGA_3D_IDX_BUFFER_ADDR 0x10001000U
#define QEMU_VGA_3D_CMD_BUFFER_ADDR 0x10002000U

typedef struct {
    float x;
    float y;
    float z;
    float w;
} qv3_vertex_t;

typedef struct {
    float m[4][4];
} qv3_matrix_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} qv3_color_t;

typedef struct {
    float u;
    float v;
} qv3_texcoord_t;

#ifdef __cplusplus
extern "C" {
#endif

int qemu_vga_init(void);
void qemu_vga_putpixel(uint32_t x, uint32_t y, uint32_t color);
void qemu_vga_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void qemu_vga_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void qemu_vga_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
void *qemu_vga_get_fb(void);
uint32_t qemu_vga_get_width(void);
uint32_t qemu_vga_get_height(void);

void qemu_vga_3d_begin_scene(void);
void qemu_vga_3d_end_scene(void);
void qemu_vga_3d_set_matrix(qv3_matrix_t *matrix);
void qemu_vga_3d_draw_triangle(qv3_vertex_t *v1, qv3_vertex_t *v2, qv3_vertex_t *v3, qv3_color_t *c1, qv3_color_t *c2, qv3_color_t *c3);
void qemu_vga_3d_clear(uint32_t color, float depth);

void qemu_vga_3d_flush(void);
void qemu_vga_3d_vertex_buffer(const qv3_vtx_attr_t *vertices, uint32_t count);
void qemu_vga_3d_index_buffer(const uint16_t *indices, uint32_t count);
void qemu_vga_3d_texture_bind(uint32_t slot, uint32_t tex_addr);
void qemu_vga_3d_blend_enable(uint8_t enable);
void qemu_vga_3d_depth_test_enable(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif

