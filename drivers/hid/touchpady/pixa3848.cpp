/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/hid/touchpady/pixa3848.h>

extern "C" {
#include <graphics/rysowanie.h>
#include <graphics/okno.h>
#include <graphics/pulpit.h>
#include <graphics/menu_start.h>
}

static inline uint8_t pixa3848_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void pixa3848_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pixa3848_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (uint32_t j = 0; j < 48U; j++);
    }
}

enum pixa3848_reg {
    PIXA3848_REG_DEVICE_ID      = 0x00U,
    PIXA3848_REG_VENDOR_ID      = 0x01U,
    PIXA3848_REG_STATUS         = 0x02U,
    PIXA3848_REG_TOUCH_COUNT    = 0x03U,
    PIXA3848_REG_FINGER_BASE    = 0x04U,
    PIXA3848_REG_FINGER_STRIDE  = 0x0AU,
    PIXA3848_REG_GESTURE        = 0x64U,
    PIXA3848_REG_GESTURE_DATA   = 0x65U,
    PIXA3848_REG_CONFIG         = 0x66U,
    PIXA3848_REG_CONTROL        = 0x67U,
    PIXA3848_REG_SENSITIVITY    = 0x68U,
    PIXA3848_REG_INT_STATUS     = 0x69U,
    PIXA3848_REG_INT_MASK       = 0x6AU,
    PIXA3848_REG_STATS_TOUCH    = 0x70U,
    PIXA3848_REG_STATS_TAP      = 0x71U,
    PIXA3848_REG_STATS_SCROLL   = 0x72U,
    PIXA3848_REG_STATS_PALM     = 0x73U,
};

#define PIXA3848_VENDOR_ID  0x14E4U
#define PIXA3848_DEVICE_ID  0x3848U

#define PIXA3848_STATUS_DATA_READY   (1U << 0)
#define PIXA3848_STATUS_GESTURE      (1U << 1)
#define PIXA3848_STATUS_ERROR        (1U << 7)

#define PIXA3848_FINGER_STATE_DOWN   0x01U
#define PIXA3848_FINGER_STATE_UP     0x02U
#define PIXA3848_FINGER_STATE_MOTION 0x04U
#define PIXA3848_FINGER_STATE_PALM   0x80U

#define PIXA3848_GESTURE_NONE        0x00U
#define PIXA3848_GESTURE_TAP         0x01U
#define PIXA3848_GESTURE_DOUBLE_TAP  0x02U
#define PIXA3848_GESTURE_SCROLL      0x03U
#define PIXA3848_GESTURE_PINCH       0x04U
#define PIXA3848_GESTURE_SWIPE3      0x05U
#define PIXA3848_GESTURE_EDGE        0x06U

#define PIXA3848_EDGE_NONE      0x00U
#define PIXA3848_EDGE_LEFT      0x01U
#define PIXA3848_EDGE_RIGHT     0x02U
#define PIXA3848_EDGE_TOP       0x03U
#define PIXA3848_EDGE_BOTTOM    0x04U

#define PIXA3848_CONTROL_ENABLE     (1U << 0)
#define PIXA3848_CONTROL_INT_ENABLE (1U << 1)

#define PIXA3848_CONFIG_PALM_REJECT (1U << 0)
#define PIXA3848_CONFIG_TAP_ENABLE  (1U << 1)
#define PIXA3848_CONFIG_SCROLL2     (1U << 2)
#define PIXA3848_CONFIG_PINCH       (1U << 3)
#define PIXA3848_CONFIG_SWIPE3      (1U << 4)
#define PIXA3848_CONFIG_EDGE        (1U << 5)

#define PIXA3848_INT_DATA           (1U << 0)
#define PIXA3848_INT_GESTURE        (1U << 1)

#define PIXA3848_MAX_FINGERS 10U
#define PIXA3848_PRESSURE_PALM_THRESHOLD 200

static uint8_t pixa3848_i2c_addr = 0x24U;
static uint8_t pixa3848_ready = 0;
static uint8_t pixa3848_enabled = 0;
static uint8_t pixa3848_sensitivity = 5U;
static uint32_t pixa3848_cursor_x = 512U;
static uint32_t pixa3848_cursor_y = 384U;
static uint8_t pixa3848_left_pressed = 0;
static uint8_t pixa3848_right_pressed = 0;

static int32_t pixa3848_finger_x[PIXA3848_MAX_FINGERS];
static int32_t pixa3848_finger_y[PIXA3848_MAX_FINGERS];
static uint8_t pixa3848_finger_pressure[PIXA3848_MAX_FINGERS];
static uint8_t pixa3848_finger_width[PIXA3848_MAX_FINGERS];
static uint8_t pixa3848_finger_height[PIXA3848_MAX_FINGERS];
static uint8_t pixa3848_finger_id[PIXA3848_MAX_FINGERS];
static uint8_t pixa3848_finger_state[PIXA3848_MAX_FINGERS];
static uint32_t pixa3848_finger_last_tick[PIXA3848_MAX_FINGERS];

static uint8_t pixa3848_tap_pending = 0;
static uint32_t pixa3848_tap_last_tick = 0;
static int32_t pixa3848_tap_last_x = 0;
static int32_t pixa3848_tap_last_y = 0;

static uint32_t pixa3848_scroll_accum_x = 0;
static uint32_t pixa3848_scroll_accum_y = 0;

static struct pixa3848_stats pixa3848_stats = {};

static uint8_t pixa3848_reg_read(uint8_t reg)
{
    pixa3848_outb(0x60U, pixa3848_i2c_addr);
    pixa3848_outb(0x61U, reg);
    pixa3848_outb(0x62U, 0x01U);
    pixa3848_delay_us(10U);
    for (uint32_t wait = 0; wait < 100000U; wait++) {
        if (!(pixa3848_inb(0x62U) & 0x01U)) break;
    }
    return pixa3848_inb(0x63U);
}

static void pixa3848_reg_write(uint8_t reg, uint8_t value)
{
    pixa3848_outb(0x60U, pixa3848_i2c_addr);
    pixa3848_outb(0x61U, reg);
    pixa3848_outb(0x63U, value);
    pixa3848_outb(0x62U, 0x02U);
    pixa3848_delay_us(10U);
    for (uint32_t wait = 0; wait < 100000U; wait++) {
        if (!(pixa3848_inb(0x62U) & 0x02U)) break;
    }
}

static int pixa3848_detect_device(void)
{
    uint8_t vendor_low = pixa3848_reg_read(PIXA3848_REG_VENDOR_ID);
    uint8_t vendor_high = pixa3848_reg_read(PIXA3848_REG_VENDOR_ID + 1U);
    uint8_t dev_low = pixa3848_reg_read(PIXA3848_REG_DEVICE_ID);
    uint8_t dev_high = pixa3848_reg_read(PIXA3848_REG_DEVICE_ID + 1U);
    uint16_t vendor = (uint16_t)(vendor_low | (vendor_high << 8));
    uint16_t device = (uint16_t)(dev_low | (dev_high << 8));
    if (vendor == PIXA3848_VENDOR_ID && device == PIXA3848_DEVICE_ID) return 1;
    return 0;
}

static int pixa3848_reset(void)
{
    pixa3848_reg_write(PIXA3848_REG_CONTROL, 0x00U);
    pixa3848_delay_us(100U);
    pixa3848_reg_write(PIXA3848_REG_CONTROL, (uint8_t)PIXA3848_CONTROL_ENABLE);
    pixa3848_delay_us(100U);
    return 1;
}

static int pixa3848_read_touch_data(void)
{
    uint8_t status = pixa3848_reg_read(PIXA3848_REG_STATUS);
    if (!(status & PIXA3848_STATUS_DATA_READY)) return 0;

    uint8_t count = pixa3848_reg_read(PIXA3848_REG_TOUCH_COUNT);
    if (count > PIXA3848_MAX_FINGERS) count = PIXA3848_MAX_FINGERS;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t base = (uint8_t)(PIXA3848_REG_FINGER_BASE + i * PIXA3848_REG_FINGER_STRIDE);
        uint8_t xl = pixa3848_reg_read(base);
        uint8_t xh = pixa3848_reg_read(base + 1U);
        uint8_t yl = pixa3848_reg_read(base + 2U);
        uint8_t yh = pixa3848_reg_read(base + 3U);
        uint8_t pressure = pixa3848_reg_read(base + 4U);
        uint8_t width = pixa3848_reg_read(base + 5U);
        uint8_t height = pixa3848_reg_read(base + 6U);
        uint8_t fid = pixa3848_reg_read(base + 7U);
        uint8_t state = pixa3848_reg_read(base + 8U);

        int32_t raw_x = (int32_t)((uint16_t)(xl | (xh << 8)));
        int32_t raw_y = (int32_t)((uint16_t)(yl | (yh << 8)));

        pixa3848_finger_x[i] = raw_x;
        pixa3848_finger_y[i] = raw_y;
        pixa3848_finger_pressure[i] = pressure;
        pixa3848_finger_width[i] = width;
        pixa3848_finger_height[i] = height;
        pixa3848_finger_id[i] = fid;
        pixa3848_finger_state[i] = state;
    }

    for (uint8_t i = count; i < PIXA3848_MAX_FINGERS; i++) {
        pixa3848_finger_state[i] = PIXA3848_FINGER_STATE_UP;
    }

    pixa3848_stats.touches_detected += count;
    return 1;
}

static int pixa3848_is_palm(uint8_t index)
{
    uint8_t area = (uint8_t)(pixa3848_finger_width[index] * pixa3848_finger_height[index] / 4U);
    uint8_t pressure = pixa3848_finger_pressure[index];
    if (pressure > PIXA3848_PRESSURE_PALM_THRESHOLD || area > 64U) {
        pixa3848_stats.palm_rejections++;
        return 1;
    }
    return 0;
}

static void pixa3848_map_to_screen(int32_t raw_x, int32_t raw_y, int32_t *sx, int32_t *sy)
{
    *sx = (raw_x * (int32_t)EKRAN_SZEROKOSC) / 4096;
    *sy = (raw_y * (int32_t)EKRAN_WYSOKOSC) / 4096;
    if (*sx < 0) *sx = 0;
    if (*sy < 0) *sy = 0;
    if (*sx > (int32_t)EKRAN_SZEROKOSC) *sx = (int32_t)EKRAN_SZEROKOSC;
    if (*sy > (int32_t)EKRAN_WYSOKOSC) *sy = (int32_t)EKRAN_WYSOKOSC;
}

static void pixa3848_detect_gestures(uint8_t count)
{
    if (count == 1 && pixa3848_finger_state[0] == PIXA3848_FINGER_STATE_DOWN) {
        pixa3848_tap_pending = 1;
        pixa3848_tap_last_x = pixa3848_finger_x[0];
        pixa3848_tap_last_y = pixa3848_finger_y[0];
    }
    if (count == 2) {
        pixa3848_scroll_accum_x += (uint32_t)(pixa3848_finger_x[1] - pixa3848_finger_x[0]);
        pixa3848_scroll_accum_y += (uint32_t)(pixa3848_finger_y[1] - pixa3848_finger_y[0]);
        if (pixa3848_scroll_accum_x > 100U || pixa3848_scroll_accum_y > 100U) {
            pixa3848_stats.scrolls++;
            pixa3848_scroll_accum_x = 0;
            pixa3848_scroll_accum_y = 0;
        }
    }
    if (count == 3) {
        pixa3848_stats.swipes3++;
    }
    if (count > 0) {
        int32_t sx, sy;
        pixa3848_map_to_screen(pixa3848_finger_x[0], pixa3848_finger_y[0], &sx, &sy);
        if (sx < 20) pixa3848_stats.edge_gestures++;
        else if (sx > (int32_t)EKRAN_SZEROKOSC - 20) pixa3848_stats.edge_gestures++;
        else if (sy < 20) pixa3848_stats.edge_gestures++;
        else if (sy > (int32_t)EKRAN_WYSOKOSC - 20) pixa3848_stats.edge_gestures++;
    }
}

static void pixa3848_handle_input(uint8_t count)
{
    if (count == 0) {
        if (pixa3848_left_pressed) {
            pixa3848_left_pressed = 0;
            okno_zakoncz_przeciaganie();
            int x, y;
            kursor_graficzny_pozycja(&x, &y);
            pulp_mouse_release(x, y);
        }
        pixa3848_tap_pending = 0;
        return;
    }

    int primary_has_palm = pixa3848_is_palm(0);
    int32_t sx, sy;
    pixa3848_map_to_screen(pixa3848_finger_x[0], pixa3848_finger_y[0], &sx, &sy);

    int dx = sx - (int32_t)pixa3848_cursor_x;
    int dy = sy - (int32_t)pixa3848_cursor_y;

    if (dx != 0 || dy != 0) {
        pixa3848_cursor_x = (uint32_t)sx;
        pixa3848_cursor_y = (uint32_t)sy;
        kursor_graficzny_przesun(dx, dy);
    }

    if (pixa3848_finger_state[0] & PIXA3848_FINGER_STATE_DOWN) {
        if (!pixa3848_left_pressed && !primary_has_palm) {
            pixa3848_left_pressed = 1;
            int x, y;
            kursor_graficzny_pozycja(&x, &y);
            if (!okno_obsluz_klikniecie(x, y)) {
                if (!pulp_mouse_press(x, y)) {
                    if (y >= EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC) {
                        menu_start_mouse_click();
                    }
                }
            }
        }
    } else if (pixa3848_finger_state[0] & PIXA3848_FINGER_STATE_UP) {
        if (pixa3848_left_pressed && !primary_has_palm) {
            pixa3848_left_pressed = 0;
            okno_zakoncz_przeciaganie();
            int x, y;
            kursor_graficzny_pozycja(&x, &y);
            pulp_mouse_release(x, y);
        }
    }

    if (count >= 2 && !primary_has_palm) {
        int x, y;
        kursor_graficzny_pozycja(&x, &y);
        pulp_mouse_ruch(x, y);
    }

    pixa3848_detect_gestures(count);
}

int pixa3848_init(void)
{
    if (pixa3848_ready) return 1;
    if (!pixa3848_detect_device()) return 0;

    pixa3848_reg_write(PIXA3848_REG_INT_MASK, (uint8_t)(PIXA3848_INT_DATA | PIXA3848_INT_GESTURE));
    pixa3848_reg_write(PIXA3848_REG_CONFIG,
                       (uint8_t)(PIXA3848_CONFIG_PALM_REJECT | PIXA3848_CONFIG_TAP_ENABLE |
                                 PIXA3848_CONFIG_SCROLL2 | PIXA3848_CONFIG_PINCH |
                                 PIXA3848_CONFIG_SWIPE3 | PIXA3848_CONFIG_EDGE));
    pixa3848_reg_write(PIXA3848_REG_SENSITIVITY, pixa3848_sensitivity);

    if (!pixa3848_reset()) return 0;

    for (uint32_t i = 0; i < PIXA3848_MAX_FINGERS; i++) {
        pixa3848_finger_x[i] = 0;
        pixa3848_finger_y[i] = 0;
        pixa3848_finger_pressure[i] = 0;
        pixa3848_finger_width[i] = 0;
        pixa3848_finger_height[i] = 0;
        pixa3848_finger_id[i] = 0;
        pixa3848_finger_state[i] = PIXA3848_FINGER_STATE_UP;
        pixa3848_finger_last_tick[i] = 0;
    }

    pixa3848_cursor_x = EKRAN_SZEROKOSC / 2;
    pixa3848_cursor_y = EKRAN_WYSOKOSC / 2;
    pixa3848_left_pressed = 0;
    pixa3848_right_pressed = 0;
    pixa3848_tap_pending = 0;
    pixa3848_tap_last_tick = 0;
    pixa3848_tap_last_x = 0;
    pixa3848_tap_last_y = 0;
    pixa3848_scroll_accum_x = 0;
    pixa3848_scroll_accum_y = 0;
    pixa3848_stats = {};
    pixa3848_enabled = 1;
    pixa3848_ready = 1;
    return 1;
}

int pixa3848_poll(void)
{
    if (!pixa3848_ready || !pixa3848_enabled) return 0;
    uint8_t isr = pixa3848_reg_read(PIXA3848_REG_INT_STATUS);
    if (!(isr & (PIXA3848_INT_DATA | PIXA3848_INT_GESTURE))) return 0;

    pixa3848_reg_write(PIXA3848_REG_INT_STATUS, isr);
    pixa3848_read_touch_data();

    uint8_t count = pixa3848_reg_read(PIXA3848_REG_TOUCH_COUNT);
    if (count > PIXA3848_MAX_FINGERS) count = PIXA3848_MAX_FINGERS;

    pixa3848_handle_input(count);
    return 1;
}

int pixa3848_is_ready(void)
{
    return pixa3848_ready;
}

int pixa3848_shutdown(void)
{
    if (!pixa3848_ready) return 0;
    pixa3848_reg_write(PIXA3848_REG_CONTROL, 0x00U);
    pixa3848_enabled = 0;
    pixa3848_ready = 0;
    if (pixa3848_left_pressed) {
        pixa3848_left_pressed = 0;
        okno_zakoncz_przeciaganie();
    }
    return 1;
}

int pixa3848_enable(void)
{
    if (!pixa3848_ready) return 0;
    pixa3848_reg_write(PIXA3848_REG_CONTROL,
                       (uint8_t)(PIXA3848_CONTROL_ENABLE | PIXA3848_CONTROL_INT_ENABLE));
    pixa3848_enabled = 1;
    return 1;
}

int pixa3848_disable(void)
{
    if (!pixa3848_ready) return 0;
    pixa3848_reg_write(PIXA3848_REG_CONTROL, 0x00U);
    pixa3848_enabled = 0;
    return 1;
}

int pixa3848_get_touch_count(void)
{
    if (!pixa3848_ready) return 0;
    uint8_t count = pixa3848_reg_read(PIXA3848_REG_TOUCH_COUNT);
    if (count > PIXA3848_MAX_FINGERS) count = PIXA3848_MAX_FINGERS;
    return (int)count;
}

int pixa3848_get_finger(int index, int *x, int *y, int *pressure, int *width, int *height, int *touch_id, int *state)
{
    if (!pixa3848_ready || index < 0 || index >= (int)PIXA3848_MAX_FINGERS) return 0;
    if (x) *x = (int)pixa3848_finger_x[index];
    if (y) *y = (int)pixa3848_finger_y[index];
    if (pressure) *pressure = (int)pixa3848_finger_pressure[index];
    if (width) *width = (int)pixa3848_finger_width[index];
    if (height) *height = (int)pixa3848_finger_height[index];
    if (touch_id) *touch_id = (int)pixa3848_finger_id[index];
    if (state) *state = (int)pixa3848_finger_state[index];
    return 1;
}

int pixa3848_get_cursor(int *x, int *y)
{
    if (!pixa3848_ready) return 0;
    if (x) *x = (int)pixa3848_cursor_x;
    if (y) *y = (int)pixa3848_cursor_y;
    return 1;
}

int pixa3848_gesture_tap(void)
{
    if (!pixa3848_ready) return 0;
    if (pixa3848_tap_pending) {
        pixa3848_tap_pending = 0;
        pixa3848_stats.taps++;
        return 1;
    }
    return 0;
}

int pixa3848_gesture_double_tap(void)
{
    if (!pixa3848_ready) return 0;
    if (pixa3848_tap_pending && pixa3848_tap_last_tick != 0U) {
        pixa3848_tap_pending = 0;
        pixa3848_tap_last_tick = 0;
        pixa3848_stats.double_taps++;
        return 1;
    }
    return 0;
}

int pixa3848_gesture_scroll(int *dx, int *dy)
{
    (void)dx;
    (void)dy;
    if (!pixa3848_ready) return 0;
    if (pixa3848_stats.scrolls > 0) {
        pixa3848_stats.scrolls--;
        return 1;
    }
    return 0;
}

int pixa3848_gesture_pinch(int *scale)
{
    (void)scale;
    if (!pixa3848_ready) return 0;
    return 0;
}

int pixa3848_gesture_swipe3(int *dx, int *dy)
{
    (void)dx;
    (void)dy;
    if (!pixa3848_ready) return 0;
    if (pixa3848_stats.swipes3 > 0) {
        pixa3848_stats.swipes3--;
        return 1;
    }
    return 0;
}

int pixa3848_gesture_edge(int *edge)
{
    (void)edge;
    if (!pixa3848_ready) return 0;
    if (pixa3848_stats.edge_gestures > 0) {
        pixa3848_stats.edge_gestures--;
        return 1;
    }
    return 0;
}

int pixa3848_set_sensitivity(uint8_t level)
{
    if (!pixa3848_ready) return 0;
    if (level > 10U) level = 10U;
    pixa3848_sensitivity = level;
    pixa3848_reg_write(PIXA3848_REG_SENSITIVITY, pixa3848_sensitivity);
    return 1;
}

int pixa3848_stats_get(struct pixa3848_stats *out)
{
    if (!out) return 0;
    *out = pixa3848_stats;
    return 1;
}

void pixa3848_stats_reset(void)
{
    pixa3848_stats = {};
}
