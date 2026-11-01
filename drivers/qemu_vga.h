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

#ifdef __cplusplus
}
#endif

#endif
