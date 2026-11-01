#include "qemu_vga.h"

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

static uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t function,
                                   uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

#define QEMU_VGA_REG_MODE 0x00U
#define QEMU_VGA_REG_CTRL 0x04U
#define QEMU_VGA_REG_STATUS 0x08U
#define QEMU_VGA_REG_2D_FILL 0x10U
#define QEMU_VGA_REG_2D_COPY 0x14U
#define QEMU_VGA_REG_2D_LINE 0x18U

#define QEMU_VGA_MODE_GRAPHICS 0x00000001U
#define QEMU_VGA_CTRL_ENABLE 0x80000000U

#define QEMU_VGA_OP_FILL 0x01000000U
#define QEMU_VGA_OP_COPY 0x02000000U

#define QEMU_VGA_RING_SIZE 4096U

static volatile uint32_t *g_mmio = 0;
static uint8_t g_bus = 0;
static uint8_t g_slot = 0;
static uint8_t g_func = 0;
static uint32_t g_mmio_base = 0;
static uint16_t g_device_id = 0;
static uint8_t g_revision = 0;
static int g_initialized = 0;
static volatile uint32_t *g_fb = (volatile uint32_t *)QEMU_VGA_FB_ADDR;
static uint32_t g_ring[QEMU_VGA_RING_SIZE];
static uint32_t g_ring_head = 0;
static uint32_t g_ring_tail = 0;

static float g_zbuffer[QEMU_VGA_FB_WIDTH * QEMU_VGA_FB_HEIGHT];
static qv3_matrix_t g_matrix;
static int g_scene_begun = 0;

static int qemu_vga_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t id0 = pci_read_config_dword((uint8_t)bus, slot, func, 0x00);
                uint16_t vendor = (uint16_t)id0;
                if (vendor == 0xFFFF || vendor == 0x0000) continue;
                if (vendor != QEMU_VGA_VENDOR_ID && vendor != QEMU_STD_VGA_VENDOR_ID) continue;
                uint32_t class_code = pci_read_config_dword((uint8_t)bus, slot, func, 0x08);
                uint8_t base_class = (uint8_t)(class_code >> 24);
                if (base_class != 0x03) continue;
                uint32_t bar = pci_read_config_dword((uint8_t)bus, slot, func, 0x10);
                if (bar == 0 || bar == 0xFFFFFFFFU) continue;
                if ((bar & 1U) != 0) continue;
                g_bus = (uint8_t)bus;
                g_slot = slot;
                g_func = func;
                g_mmio_base = bar & 0xFFFFFFF0U;
                g_device_id = (uint16_t)(id0 >> 16);
                g_revision = (uint8_t)(class_code & 0xFFU);
                return 1;
            }
        }
    }
    return 0;
}

static inline uint32_t mmio_read(uint32_t offset)
{
    return g_mmio[offset >> 2];
}

static inline void mmio_write(uint32_t offset, uint32_t value)
{
    g_mmio[offset >> 2] = value;
}

static void ring_emit(uint32_t dword)
{
    g_ring[g_ring_head & (QEMU_VGA_RING_SIZE - 1U)] = dword;
    g_ring_head++;
}

static void ring_flush(void)
{
    uint32_t tail = g_ring_head & (QEMU_VGA_RING_SIZE - 1U);
    mmio_write(QEMU_VGA_REG_CTRL, mmio_read(QEMU_VGA_REG_CTRL));
    for (volatile int wait = 0; wait < 1000000; wait++) {
        if ((g_ring_head & (QEMU_VGA_RING_SIZE - 1U)) == tail) {
            break;
        }
    }
    g_ring_tail = tail;
}

static void blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (((g_ring_head & (QEMU_VGA_RING_SIZE - 1U)) + 5U) > QEMU_VGA_RING_SIZE) {
        ring_flush();
    }
    ring_emit(QEMU_VGA_OP_FILL);
    ring_emit((y << 16) | x);
    ring_emit(((h - 1U) << 16) | (w - 1U));
    ring_emit(0U);
    ring_emit(color);
    ring_flush();
}

static void blt_copy(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    if (((g_ring_head & (QEMU_VGA_RING_SIZE - 1U)) + 5U) > QEMU_VGA_RING_SIZE) {
        ring_flush();
    }
    ring_emit(QEMU_VGA_OP_COPY);
    ring_emit((dst_y << 16) | dst_x);
    ring_emit(((h - 1U) << 16) | (w - 1U));
    ring_emit(0U);
    ring_emit((src_y << 16) | src_x);
    ring_flush();
}

static float qv3_sin(float x)
{
    while (x > 3.14159265358979323846f * 2.0f) x -= 3.14159265358979323846f * 2.0f;
    while (x < -3.14159265358979323846f * 2.0f) x += 3.14159265358979323846f * 2.0f;
    float x2 = x * x;
    return x * (1.0f - x2 / 6.0f + x2 * x2 / 120.0f - x2 * x2 * x2 / 5040.0f);
}

static float qv3_cos(float x)
{
    while (x > 3.14159265358979323846f * 2.0f) x -= 3.14159265358979323846f * 2.0f;
    while (x < -3.14159265358979323846f * 2.0f) x += 3.14159265358979323846f * 2.0f;
    float x2 = x * x;
    return 1.0f - x2 / 2.0f + x2 * x2 / 24.0f - x2 * x2 * x2 / 720.0f;
}

static float qv3_tan(float x)
{
    float c = qv3_cos(x);
    if (c == 0.0f) c = 0.0001f;
    return qv3_sin(x) / c;
}

static qv3_matrix_t matrix_identity(void)
{
    qv3_matrix_t m;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m.m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    return m;
}

static qv3_matrix_t matrix_mul(const qv3_matrix_t *a, const qv3_matrix_t *b)
{
    qv3_matrix_t r = matrix_identity();
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (k = 0; k < 4; k++) {
                sum += a->m[i][k] * b->m[k][j];
            }
            r.m[i][j] = sum;
        }
    }
    return r;
}

static qv3_matrix_t matrix_perspective(float fov, float aspect, float znear, float zfar)
{
    qv3_matrix_t m = matrix_identity();
    float f = 1.0f / qv3_tan(fov * 0.5f);
    float nf = 1.0f / (znear - zfar);
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (zfar + znear) * nf;
    m.m[2][3] = (2.0f * zfar * znear) * nf;
    m.m[3][2] = -1.0f;
    m.m[3][3] = 0.0f;
    return m;
}

static float vec_dot(float ax, float ay, float az, float bx, float by, float bz)
{
    return ax * bx + ay * by + az * bz;
}

static void vec_cross(float ax, float ay, float az, float bx, float by, float bz,
                      float *ox, float *oy, float *oz)
{
    *ox = ay * bz - az * by;
    *oy = az * bx - ax * bz;
    *oz = ax * by - ay * bx;
}

static void vec_norm(float *x, float *y, float *z)
{
    float temp = (*x) * (*x) + (*y) * (*y) + (*z) * (*z);
    float inv = 1.0f;
    if (temp > 0.0f) {
        float guess = 1.0f;
        int i;
        for (i = 0; i < 6; i++) {
            guess = 0.5f * (guess + temp / guess);
        }
        inv = guess;
    }
    *x *= inv;
    *y *= inv;
    *z *= inv;
}

static qv3_matrix_t matrix_lookat(qv3_vertex_t *eye, qv3_vertex_t *center, qv3_vertex_t *up)
{
    qv3_matrix_t m = matrix_identity();
    float fx = center->x - eye->x;
    float fy = center->y - eye->y;
    float fz = center->z - eye->z;
    vec_norm(&fx, &fy, &fz);
    float sx, sy, sz;
    vec_cross(fx, fy, fz, up->x, up->y, up->z, &sx, &sy, &sz);
    vec_norm(&sx, &sy, &sz);
    float ux, uy, uz;
    vec_cross(sx, sy, sz, fx, fy, fz, &ux, &uy, &uz);
    m.m[0][0] = sx; m.m[0][1] = ux; m.m[0][2] = -fx; m.m[0][3] = 0.0f;
    m.m[1][0] = sy; m.m[1][1] = uy; m.m[1][2] = -fy; m.m[1][3] = 0.0f;
    m.m[2][0] = sz; m.m[2][1] = uz; m.m[2][2] = -fz; m.m[2][3] = 0.0f;
    m.m[3][0] = -vec_dot(sx, sy, sz, eye->x, eye->y, eye->z);
    m.m[3][1] = -vec_dot(ux, uy, uz, eye->x, eye->y, eye->z);
    m.m[3][2] = vec_dot(fx, fy, fz, eye->x, eye->y, eye->z);
    m.m[3][3] = 1.0f;
    return m;
}

static qv3_matrix_t matrix_model(float tx, float ty, float tz,
                                 float rx, float ry, float rz,
                                 float sx, float sy, float sz)
{
    qv3_matrix_t m = matrix_identity();
    float cr = qv3_cos(rx), sr = qv3_sin(rx);
    float cp = qv3_cos(ry), sp = qv3_sin(ry);
    float cz = qv3_cos(rz), szr = qv3_sin(rz);
    m.m[0][0] = cp * cz * sx;
    m.m[0][1] = (sr * sp * cz - cr * szr) * sy;
    m.m[0][2] = (cr * sp * cz + sr * szr) * sz;
    m.m[0][3] = tx;
    m.m[1][0] = cp * szr * sx;
    m.m[1][1] = (sr * sp * szr + cr * cz) * sy;
    m.m[1][2] = (cr * sp * szr - sr * cz) * sz;
    m.m[1][3] = ty;
    m.m[2][0] = -sp * sx;
    m.m[2][1] = sr * cp * szr;
    m.m[2][2] = cr * cp * sz;
    m.m[2][3] = tz;
    return m;
}

extern "C" {

int qemu_vga_init(void)
{
    if (!qemu_vga_find_device()) {
        return 0;
    }

    uint32_t command = pci_read_config_dword(g_bus, g_slot, g_func, 0x04);
    pci_write_config_dword(g_bus, g_slot, g_func, 0x04, command | 0x00000006U);

    g_mmio = (volatile uint32_t *)g_mmio_base;

    mmio_write(QEMU_VGA_REG_CTRL, QEMU_VGA_CTRL_ENABLE);
    mmio_write(QEMU_VGA_REG_MODE, QEMU_VGA_MODE_GRAPHICS);
    mmio_write(QEMU_VGA_REG_2D_FILL, 0U);
    mmio_write(QEMU_VGA_REG_2D_COPY, 0U);

    g_ring_head = 0;
    g_ring_tail = 0;
    g_initialized = 1;
    return 1;
}

void qemu_vga_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= QEMU_VGA_FB_WIDTH || y >= QEMU_VGA_FB_HEIGHT) return;
    g_fb[y * QEMU_VGA_FB_WIDTH + x] = color;
}

void qemu_vga_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    uint32_t xx, yy;
    if (g_initialized) {
        blt_fill(x, y, w, h, color);
        return;
    }
    for (yy = y; yy < y + h; yy++) {
        for (xx = x; xx < x + w; xx++) {
            if (xx >= QEMU_VGA_FB_WIDTH || yy >= QEMU_VGA_FB_HEIGHT) continue;
            g_fb[yy * QEMU_VGA_FB_WIDTH + xx] = color;
        }
    }
}

void qemu_vga_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    uint32_t xx, yy;
    if (g_initialized) {
        blt_copy(dst_x, dst_y, src_x, src_y, w, h);
        return;
    }
    for (yy = 0; yy < h; yy++) {
        uint32_t sy = src_y + yy;
        uint32_t dy = dst_y + yy;
        if (dy >= QEMU_VGA_FB_HEIGHT || sy >= QEMU_VGA_FB_HEIGHT) continue;
        for (xx = 0; xx < w; xx++) {
            uint32_t sx = src_x + xx;
            uint32_t dx = dst_x + xx;
            if (dx >= QEMU_VGA_FB_WIDTH || sx >= QEMU_VGA_FB_WIDTH) continue;
            g_fb[dy * QEMU_VGA_FB_WIDTH + dx] = g_fb[sy * QEMU_VGA_FB_WIDTH + sx];
        }
    }
}

void qemu_vga_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color)
{
    int x = (int)x0;
    int y = (int)y0;
    int dx = (int)x1 - x;
    int dy = (int)y1 - y;
    int sx = dx < 0 ? -1 : 1;
    int sy = dy < 0 ? -1 : 1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int err = dx - dy;
    for (;;) {
        if (x >= 0 && x < (int)QEMU_VGA_FB_WIDTH && y >= 0 && y < (int)QEMU_VGA_FB_HEIGHT) {
            g_fb[y * QEMU_VGA_FB_WIDTH + x] = color;
        }
        if (x == (int)x1 && y == (int)y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
}

void *qemu_vga_get_fb(void)
{
    return (void *)g_fb;
}

uint32_t qemu_vga_get_width(void)
{
    return QEMU_VGA_FB_WIDTH;
}

uint32_t qemu_vga_get_height(void)
{
    return QEMU_VGA_FB_HEIGHT;
}

void qemu_vga_3d_begin_scene(void)
{
    g_scene_begun = 1;
}

void qemu_vga_3d_end_scene(void)
{
    g_scene_begun = 0;
}

void qemu_vga_3d_set_matrix(qv3_matrix_t *matrix)
{
    if (matrix) {
        g_matrix = *matrix;
    }
}

void qemu_vga_3d_clear(uint32_t color, float depth)
{
    uint32_t count = QEMU_VGA_FB_WIDTH * QEMU_VGA_FB_HEIGHT;
    uint32_t i;
    for (i = 0; i < count; i++) {
        g_fb[i] = color;
        g_zbuffer[i] = depth;
    }
}

static float edge_func(float x, float y, float x0, float y0, float x1, float y1)
{
    return (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
}

static void transform_vertex(const qv3_matrix_t *m, const qv3_vertex_t *v,
                             float *sx, float *sy, float *sz)
{
    float x = m->m[0][0] * v->x + m->m[0][1] * v->y + m->m[0][2] * v->z + m->m[0][3] * v->w;
    float y = m->m[1][0] * v->x + m->m[1][1] * v->y + m->m[1][2] * v->z + m->m[1][3] * v->w;
    float z = m->m[2][0] * v->x + m->m[2][1] * v->y + m->m[2][2] * v->z + m->m[2][3] * v->w;
    float w = m->m[3][0] * v->x + m->m[3][1] * v->y + m->m[3][2] * v->z + m->m[3][3] * v->w;
    if (w == 0.0f) w = 0.0001f;
    float invw = 1.0f / w;
    float ndc_x = x * invw;
    float ndc_y = y * invw;
    float ndc_z = z * invw;
    *sx = (ndc_x + 1.0f) * 0.5f * (float)QEMU_VGA_FB_WIDTH;
    *sy = (1.0f - ndc_y) * 0.5f * (float)QEMU_VGA_FB_HEIGHT;
    *sz = ndc_z;
}

static uint32_t color_pack(const qv3_color_t *c)
{
    return ((uint32_t)c->a << 24) | ((uint32_t)c->r << 16) |
           ((uint32_t)c->g << 8) | (uint32_t)c->b;
}

void qemu_vga_3d_draw_triangle(qv3_vertex_t *v1, qv3_vertex_t *v2, qv3_vertex_t *v3,
                               qv3_color_t *c1, qv3_color_t *c2, qv3_color_t *c3)
{
    if (!g_scene_begun) return;
    if (!v1 || !v2 || !v3 || !c1 || !c2 || !c3) return;

    float x0, y0, z0;
    float x1, y1, z1;
    float x2, y2, z2;
    transform_vertex(&g_matrix, v1, &x0, &y0, &z0);
    transform_vertex(&g_matrix, v2, &x1, &y1, &z1);
    transform_vertex(&g_matrix, v3, &x2, &y2, &z2);

    float area = edge_func(x0, y0, x1, y1, x2, y2);
    if (area == 0.0f) return;

    int min_x = (int)x0; if ((int)x1 < min_x) min_x = (int)x1; if ((int)x2 < min_x) min_x = (int)x2;
    int min_y = (int)y0; if ((int)y1 < min_y) min_y = (int)y1; if ((int)y2 < min_y) min_y = (int)y2;
    int max_x = (int)x0; if ((int)x1 > max_x) max_x = (int)x1; if ((int)x2 > max_x) max_x = (int)x2;
    int max_y = (int)y0; if ((int)y1 > max_y) max_y = (int)y1; if ((int)y2 > max_y) max_y = (int)y2;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x > (int)QEMU_VGA_FB_WIDTH - 1) max_x = (int)QEMU_VGA_FB_WIDTH - 1;
    if (max_y > (int)QEMU_VGA_FB_HEIGHT - 1) max_y = (int)QEMU_VGA_FB_HEIGHT - 1;

    uint32_t col0 = color_pack(c1);
    uint32_t col1 = color_pack(c2);
    uint32_t col2 = color_pack(c3);
    (void)col0; (void)col1; (void)col2;

    int y;
    for (y = min_y; y <= max_y; y++) {
        int x;
        for (x = min_x; x <= max_x; x++) {
            float e0 = edge_func((float)x + 0.5f, (float)y + 0.5f, x1, y1, x2, y2);
            float e1 = edge_func((float)x + 0.5f, (float)y + 0.5f, x2, y2, x0, y0);
            float e2 = edge_func((float)x + 0.5f, (float)y + 0.5f, x0, y0, x1, y1);
            if (e0 >= 0.0f && e1 >= 0.0f && e2 >= 0.0f) {
                float w0 = e0 / area;
                float w1 = e1 / area;
                float w2 = e2 / area;
                float depth = w0 * z0 + w1 * z1 + w2 * z2;
                uint32_t idx = (uint32_t)(y * QEMU_VGA_FB_WIDTH + x);
                if (depth < g_zbuffer[idx]) {
                    g_zbuffer[idx] = depth;
                    uint32_t r = (uint32_t)(w0 * (float)c1->r + w1 * (float)c2->r + w2 * (float)c3->r);
                    uint32_t g = (uint32_t)(w0 * (float)c1->g + w1 * (float)c2->g + w2 * (float)c3->g);
                    uint32_t b = (uint32_t)(w0 * (float)c1->b + w1 * (float)c2->b + w2 * (float)c3->b);
                    uint32_t a = (uint32_t)(w0 * (float)c1->a + w1 * (float)c2->a + w2 * (float)c3->a);
                    g_fb[idx] = (a << 24) | (r << 16) | (g << 8) | b;
                }
            }
        }
    }
}

}
