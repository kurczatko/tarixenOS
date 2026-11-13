#include <drivers/qemu_vga.h>

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

typedef struct {
    uint32_t opcode;
    uint32_t arg0;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
} qv3_cmd_t;

static qv3_cmd_t g_cmd_buffer[QEMU_VGA_3D_MAX_CMDS];
static uint32_t g_cmd_count = 0;
static qv3_vtx_attr_t g_vtx_buffer[QEMU_VGA_3D_MAX_VTX];
static uint32_t g_vtx_count = 0;
static uint16_t g_idx_buffer[QEMU_VGA_3D_MAX_IDX];
static uint32_t g_idx_count = 0;
static qv3_matrix_t g_matrix;

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

static uint32_t float_to_uint(float f)
{
    union {
        float f;
        uint32_t u;
    } u;
    u.f = f;
    return u.u;
}

static void write_matrix_block(uint32_t base, const qv3_matrix_t *m)
{
    int r;
    int c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            mmio_write(base + (uint32_t)((r * 4 + c) * 4), float_to_uint(m->m[r][c]));
        }
    }
}

static void cmd_emit(uint32_t opcode, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    if (g_cmd_count >= QEMU_VGA_3D_MAX_CMDS) {
        return;
    }
    g_cmd_buffer[g_cmd_count].opcode = opcode;
    g_cmd_buffer[g_cmd_count].arg0 = a0;
    g_cmd_buffer[g_cmd_count].arg1 = a1;
    g_cmd_buffer[g_cmd_count].arg2 = a2;
    g_cmd_buffer[g_cmd_count].arg3 = a3;
    g_cmd_count++;
}

static void cmd_flush(void)
{
    uint32_t i = 0;
    while (i < g_cmd_count) {
        qv3_cmd_t *cmd = &g_cmd_buffer[i];
        volatile int wait;
        for (wait = 0; wait < 1000000; wait++) {
            if (mmio_read(QEMU_VGA_REG_3D_STATUS) == QEMU_VGA_3D_STATUS_IDLE) {
                break;
            }
        }
        mmio_write(QEMU_VGA_REG_3D_CMD, cmd->opcode);
        switch (cmd->opcode) {
        case QEMU_VGA_3D_CMD_CLEAR:
            mmio_write(QEMU_VGA_REG_3D_CLEAR_COLOR, cmd->arg0);
            mmio_write(QEMU_VGA_REG_3D_CLEAR_DEPTH, cmd->arg1);
            break;
        case QEMU_VGA_3D_CMD_DRAW_TRIANGLES: {
            volatile uint32_t *vtx_dst = (volatile uint32_t *)QEMU_VGA_3D_VTX_BUFFER_ADDR;
            volatile uint32_t *idx_dst = (volatile uint32_t *)QEMU_VGA_3D_IDX_BUFFER_ADDR;
            uint32_t vi = 0;
            const uint32_t *src = 0;
            uint32_t k = 0;
            while (vi < g_vtx_count) {
                src = (const uint32_t *)&g_vtx_buffer[vi];
                k = 0;
                while (k < 7U) {
                    vtx_dst[vi * 7U + k] = src[k];
                    k++;
                }
                vi++;
            }
            vi = 0;
            while (vi < g_idx_count) {
                idx_dst[vi] = (uint32_t)g_idx_buffer[vi];
                vi++;
            }
            mmio_write(QEMU_VGA_REG_3D_VTX_BASE, QEMU_VGA_3D_VTX_BUFFER_ADDR);
            mmio_write(QEMU_VGA_REG_3D_VTX_COUNT, g_vtx_count);
            mmio_write(QEMU_VGA_REG_3D_IDX_BASE, QEMU_VGA_3D_IDX_BUFFER_ADDR);
            mmio_write(QEMU_VGA_REG_3D_IDX_COUNT, g_idx_count);
            mmio_write(QEMU_VGA_REG_3D_DRAW_PRIMITIVES, cmd->arg0);
            break;
        }
        case QEMU_VGA_3D_CMD_SET_MATRIX: {
            uint32_t base = QEMU_VGA_REG_3D_PROJ_MTX;
            if (cmd->arg0 == 1U) {
                base = QEMU_VGA_REG_3D_VIEW_MTX;
            } else if (cmd->arg0 == 2U) {
                base = QEMU_VGA_REG_3D_MODEL_MTX;
            }
            write_matrix_block(base, &g_matrix);
            break;
        }
        case QEMU_VGA_3D_CMD_SET_TEXTURE:
            mmio_write(QEMU_VGA_REG_3D_TEX_BASE, cmd->arg0);
            break;
        case QEMU_VGA_3D_CMD_SET_BLEND:
            mmio_write(QEMU_VGA_REG_3D_BLEND_ENABLE, cmd->arg0);
            break;
        case QEMU_VGA_3D_CMD_SET_DEPTH:
            mmio_write(QEMU_VGA_REG_3D_DEPTH_TEST, cmd->arg0);
            break;
        default:
            break;
        }
        for (wait = 0; wait < 1000000; wait++) {
            if (mmio_read(QEMU_VGA_REG_3D_STATUS) == QEMU_VGA_3D_STATUS_IDLE) {
                break;
            }
        }
        i++;
    }
    g_cmd_count = 0;
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

    uint32_t i = 0;
    while (i < QEMU_VGA_3D_MAX_CMDS) {
        g_cmd_buffer[i].opcode = 0U;
        g_cmd_buffer[i].arg0 = 0U;
        g_cmd_buffer[i].arg1 = 0U;
        g_cmd_buffer[i].arg2 = 0U;
        g_cmd_buffer[i].arg3 = 0U;
        i++;
    }
    i = 0;
    while (i < QEMU_VGA_3D_MAX_VTX) {
        g_vtx_buffer[i].position[0] = 0.0f;
        g_vtx_buffer[i].position[1] = 0.0f;
        g_vtx_buffer[i].position[2] = 0.0f;
        g_vtx_buffer[i].position[3] = 0.0f;
        g_vtx_buffer[i].color[0] = 0;
        g_vtx_buffer[i].color[1] = 0;
        g_vtx_buffer[i].color[2] = 0;
        g_vtx_buffer[i].color[3] = 0;
        g_vtx_buffer[i].texcoord[0] = 0.0f;
        g_vtx_buffer[i].texcoord[1] = 0.0f;
        i++;
    }
    i = 0;
    while (i < QEMU_VGA_3D_MAX_IDX) {
        g_idx_buffer[i] = 0;
        i++;
    }

    g_cmd_count = 0;
    g_vtx_count = 0;
    g_idx_count = 0;
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
    uint32_t xx;
    uint32_t yy;
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
    uint32_t xx;
    uint32_t yy;
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

void qemu_vga_3d_set_matrix(qv3_matrix_t *matrix)
{
    if (!matrix) return;
    g_matrix = *matrix;
    write_matrix_block(QEMU_VGA_REG_3D_PROJ_MTX, &g_matrix);
    write_matrix_block(QEMU_VGA_REG_3D_VIEW_MTX, &g_matrix);
    write_matrix_block(QEMU_VGA_REG_3D_MODEL_MTX, &g_matrix);
    cmd_emit(QEMU_VGA_3D_CMD_SET_MATRIX, 0U, 0U, 0U, 0U);
}

void qemu_vga_3d_clear(uint32_t color, float depth)
{
    cmd_emit(QEMU_VGA_3D_CMD_CLEAR, color, float_to_uint(depth), 0U, 0U);
}

void qemu_vga_3d_draw_triangle(qv3_vertex_t *v1, qv3_vertex_t *v2, qv3_vertex_t *v3,
                               qv3_color_t *c1, qv3_color_t *c2, qv3_color_t *c3)
{
    if (!v1 || !v2 || !v3 || !c1 || !c2 || !c3) return;
    if (g_vtx_count + 3U > QEMU_VGA_3D_MAX_VTX) return;
    if (g_idx_count + 3U > QEMU_VGA_3D_MAX_IDX) return;

    uint32_t i0 = g_vtx_count;
    uint32_t i1 = g_vtx_count + 1U;
    uint32_t i2 = g_vtx_count + 2U;

    qv3_vtx_attr_t a;
    a.position[0] = v1->x;
    a.position[1] = v1->y;
    a.position[2] = v1->z;
    a.position[3] = v1->w;
    a.color[0] = c1->r;
    a.color[1] = c1->g;
    a.color[2] = c1->b;
    a.color[3] = c1->a;
    a.texcoord[0] = 0.0f;
    a.texcoord[1] = 0.0f;
    g_vtx_buffer[i0] = a;

    a.position[0] = v2->x;
    a.position[1] = v2->y;
    a.position[2] = v2->z;
    a.position[3] = v2->w;
    a.color[0] = c2->r;
    a.color[1] = c2->g;
    a.color[2] = c2->b;
    a.color[3] = c2->a;
    a.texcoord[0] = 0.0f;
    a.texcoord[1] = 0.0f;
    g_vtx_buffer[i1] = a;

    a.position[0] = v3->x;
    a.position[1] = v3->y;
    a.position[2] = v3->z;
    a.position[3] = v3->w;
    a.color[0] = c3->r;
    a.color[1] = c3->g;
    a.color[2] = c3->b;
    a.color[3] = c3->a;
    a.texcoord[0] = 0.0f;
    a.texcoord[1] = 0.0f;
    g_vtx_buffer[i2] = a;

    g_vtx_count += 3U;

    g_idx_buffer[g_idx_count] = (uint16_t)i0;
    g_idx_buffer[g_idx_count + 1U] = (uint16_t)i1;
    g_idx_buffer[g_idx_count + 2U] = (uint16_t)i2;
    g_idx_count += 3U;

    cmd_emit(QEMU_VGA_3D_CMD_DRAW_TRIANGLES, 1U, 0U, 0U, 0U);
}

void qemu_vga_3d_flush(void)
{
    cmd_flush();
}

void qemu_vga_3d_begin_scene(void)
{
    cmd_emit(QEMU_VGA_3D_CMD_NOP, 0U, 0U, 0U, 0U);
}

void qemu_vga_3d_end_scene(void)
{
    cmd_emit(QEMU_VGA_3D_CMD_FLUSH, 0U, 0U, 0U, 0U);
    qemu_vga_3d_flush();
}

void qemu_vga_3d_vertex_buffer(const qv3_vtx_attr_t *vertices, uint32_t count)
{
    uint32_t i = 0;
    if (count > QEMU_VGA_3D_MAX_VTX) count = QEMU_VGA_3D_MAX_VTX;
    while (i < count) {
        g_vtx_buffer[i] = vertices[i];
        i++;
    }
    g_vtx_count = count;
}

void qemu_vga_3d_index_buffer(const uint16_t *indices, uint32_t count)
{
    uint32_t i = 0;
    if (count > QEMU_VGA_3D_MAX_IDX) count = QEMU_VGA_3D_MAX_IDX;
    while (i < count) {
        g_idx_buffer[i] = indices[i];
        i++;
    }
    g_idx_count = count;
}

void qemu_vga_3d_texture_bind(uint32_t slot, uint32_t tex_addr)
{
    (void)slot;
    mmio_write(QEMU_VGA_REG_3D_TEX_BASE, tex_addr);
    cmd_emit(QEMU_VGA_3D_CMD_SET_TEXTURE, tex_addr, 0U, 0U, 0U);
}

void qemu_vga_3d_blend_enable(uint8_t enable)
{
    mmio_write(QEMU_VGA_REG_3D_BLEND_ENABLE, (uint32_t)enable);
    cmd_emit(QEMU_VGA_3D_CMD_SET_BLEND, (uint32_t)enable, 0U, 0U, 0U);
}

void qemu_vga_3d_depth_test_enable(uint8_t enable)
{
    mmio_write(QEMU_VGA_REG_3D_DEPTH_TEST, (uint32_t)enable);
    cmd_emit(QEMU_VGA_3D_CMD_SET_DEPTH, (uint32_t)enable, 0U, 0U, 0U);
}

}
