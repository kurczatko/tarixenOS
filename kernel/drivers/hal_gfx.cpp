/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Warstwa abstrakcji HAL miedzy systemem a kartami graficznymi.
 * Pojedyncze funkcje API przekazuja rysowanie do funkcji sterownika
 * zaladowanego przez driver manager. Obsluguje KAZDY sterownik
 * kart graficznych.
 */

#include <include/printf/types.h>
#include <graphics/hal_gfx.h>

/* Wszystkie naglowki sterownikow kart graficznych */
#include <drivers/karty_graficzne/intel_uhd.h>
#include <drivers/karty_graficzne/gma_x4500.h>
#include <drivers/karty_graficzne/radeon_hd_8370d.h>
#include <drivers/karty_graficzne/radeon_hd_6870.h>
#include <drivers/karty_graficzne/radeon_rx_9070_xt.h>
#include <drivers/karty_graficzne/amd/r6000.h>
#include <drivers/karty_graficzne/nvidia/gtx3070.h>
#include <drivers/karty_graficzne/nvidia/gtx3080.h>
#include <drivers/karty_graficzne/nvidia/gtx3010.h>
#include <drivers/karty_graficzne/nvidia/gtx2010.h>
#include <drivers/karty_graficzne/nvidia/gtx1010.h>
#include <drivers/karty_graficzne/nvidia/titanx.h>
#include <drivers/karty_graficzne/nvidia/gtx760.h>
#include <drivers/karty_graficzne/nvidia/gtx660.h>
#include <drivers/karty_graficzne/nvidia/gtx360.h>
#include <drivers/karty_graficzne/nvidia/gtx560.h>
#include <drivers/video/qemu_vga.h>

/* Aktywny sterownik GPU wybrany przez HAL */
static struct hal_gfx_driver *g_hal_driver = 0;
static uint8_t g_hal_mial_sterownik = 0;

/* ------------------------------------------------------------------
 * Funkcje wrapper dla sterownikow z angielskimi nazwaniami funkcji
 * (radeon_hd_8370d, radeon_6870, rx_9070_xt) - mapowanie na polski
 * interfejs HAL.
 * ------------------------------------------------------------------ */

static uint8_t hal_wrap_8370d_czy_gotowy(void)
{
    return radeon_hd_8370d_is_ready() ? 1 : 0;
}

static uint32_t hal_wrap_8370d_get_szerokosc(void)
{
    return radeon_hd_8370d_get_width();
}

static uint32_t hal_wrap_8370d_get_wysokosc(void)
{
    return radeon_hd_8370d_get_height();
}

static uint8_t hal_wrap_6870_czy_gotowy(void)
{
    return radeon_6870_is_ready() ? 1 : 0;
}

static uint32_t hal_wrap_6870_get_szerokosc(void)
{
    return radeon_6870_get_width();
}

static uint32_t hal_wrap_6870_get_wysokosc(void)
{
    return radeon_6870_get_height();
}

static uint8_t hal_wrap_9070_czy_gotowy(void)
{
    return rx_9070_xt_is_ready() ? 1 : 0;
}

static uint32_t hal_wrap_9070_get_szerokosc(void)
{
    return rx_9070_xt_get_width();
}

static uint32_t hal_wrap_9070_get_wysokosc(void)
{
    return rx_9070_xt_get_height();
}

/* ------------------------------------------------------------------
 * Wrappery dla sterunkow bez get_szerokosc/get_wysokosc/czy_gotowy
 * (intel_uhd, gma_x4500) - zwracaja wartosci domyslne.
 * ------------------------------------------------------------------ */

static uint8_t hal_wrap_stub_czy_gotowy(void)
{
    return 1;
}

static uint32_t hal_wrap_stub_get_szerokosc(void)
{
    return 1920;
}

static uint32_t hal_wrap_stub_get_wysokosc(void)
{
    return 1080;
}

/* ------------------------------------------------------------------
 * Wrappery dla qemu_vga (brak cursor_set/cursor_hide, get_device_id,
 * oraz angielskie get_width/get_height).
 * ------------------------------------------------------------------ */

static void hal_wrap_qemu_cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz)
{
    (void)x; (void)y; (void)obraz;
}

static void hal_wrap_qemu_cursor_hide(void)
{
}

static uint8_t hal_wrap_qemu_czy_gotowy(void)
{
    return 1;
}

static uint32_t hal_wrap_qemu_get_szerokosc(void)
{
    return qemu_vga_get_width();
}

static uint32_t hal_wrap_qemu_get_wysokosc(void)
{
    return qemu_vga_get_height();
}

static uint16_t hal_wrap_qemu_get_device_id(void)
{
    return QEMU_VGA_VENDOR_ID;
}

/* ------------------------------------------------------------------
 * Struktury vtable dla kazdego wspieranego sterownika.
 * ------------------------------------------------------------------ */

static struct hal_gfx_driver hal_drv_intel_uhd = {
    intel_uhd_init,
    0,
    hal_wrap_stub_czy_gotowy,
    intel_uhd_putpixel,
    intel_uhd_fillrect,
    intel_uhd_blt,
    intel_uhd_cursor_set,
    intel_uhd_cursor_hide,
    intel_uhd_get_fb,
    hal_wrap_stub_get_szerokosc,
    hal_wrap_stub_get_wysokosc,
    intel_uhd_get_device_id,
};

static struct hal_gfx_driver hal_drv_gma_x4500 = {
    gma_x4500_init,
    0,
    hal_wrap_stub_czy_gotowy,
    gma_x4500_putpixel,
    gma_x4500_fillrect,
    gma_x4500_blt,
    gma_x4500_cursor_set,
    gma_x4500_cursor_hide,
    gma_x4500_get_fb,
    hal_wrap_stub_get_szerokosc,
    hal_wrap_stub_get_wysokosc,
    gma_x4500_get_device_id,
};

static struct hal_gfx_driver hal_drv_radeon_hd_8370d = {
    radeon_hd_8370d_init,
    0,
    hal_wrap_8370d_czy_gotowy,
    radeon_hd_8370d_putpixel,
    radeon_hd_8370d_fillrect,
    radeon_hd_8370d_blt,
    radeon_hd_8370d_cursor_set,
    radeon_hd_8370d_cursor_hide,
    radeon_hd_8370d_get_fb,
    hal_wrap_8370d_get_szerokosc,
    hal_wrap_8370d_get_wysokosc,
    radeon_hd_8370d_get_device_id,
};

static struct hal_gfx_driver hal_drv_radeon_6870 = {
    radeon_6870_init,
    0,
    hal_wrap_6870_czy_gotowy,
    radeon_6870_putpixel,
    radeon_6870_fillrect,
    radeon_6870_blt,
    radeon_6870_cursor_set,
    radeon_6870_cursor_hide,
    radeon_6870_get_fb,
    hal_wrap_6870_get_szerokosc,
    hal_wrap_6870_get_wysokosc,
    radeon_6870_get_device_id,
};

static struct hal_gfx_driver hal_drv_rx_9070_xt = {
    rx_9070_xt_init,
    rx_9070_xt_shutdown,
    hal_wrap_9070_czy_gotowy,
    rx_9070_xt_putpixel,
    rx_9070_xt_fillrect,
    rx_9070_xt_blt,
    rx_9070_xt_cursor_set,
    rx_9070_xt_cursor_hide,
    rx_9070_xt_get_fb,
    hal_wrap_9070_get_szerokosc,
    hal_wrap_9070_get_wysokosc,
    rx_9070_xt_get_device_id,
};

static struct hal_gfx_driver hal_drv_radeon_hd6000 = {
    radeon_hd6000_init,
    radeon_hd6000_shutdown,
    radeon_hd6000_czy_gotowy,
    radeon_hd6000_putpixel,
    radeon_hd6000_fillrect,
    radeon_hd6000_blt,
    radeon_hd6000_cursor_set,
    radeon_hd6000_cursor_hide,
    radeon_hd6000_get_fb,
    radeon_hd6000_get_szerokosc,
    radeon_hd6000_get_wysokosc,
    radeon_hd6000_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx3070 = {
    gtx3070_init,
    0,
    gtx3070_czy_gotowy,
    gtx3070_putpixel,
    gtx3070_fillrect,
    gtx3070_blt,
    gtx3070_cursor_set,
    gtx3070_cursor_hide,
    gtx3070_get_fb,
    gtx3070_get_szerokosc,
    gtx3070_get_wysokosc,
    gtx3070_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx3080 = {
    gtx3080_init,
    0,
    gtx3080_czy_gotowy,
    gtx3080_putpixel,
    gtx3080_fillrect,
    gtx3080_blt,
    gtx3080_cursor_set,
    gtx3080_cursor_hide,
    gtx3080_get_fb,
    gtx3080_get_szerokosc,
    gtx3080_get_wysokosc,
    gtx3080_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx3010 = {
    gtx3010_init,
    0,
    gtx3010_czy_gotowy,
    gtx3010_putpixel,
    gtx3010_fillrect,
    gtx3010_blt,
    gtx3010_cursor_set,
    gtx3010_cursor_hide,
    gtx3010_get_fb,
    gtx3010_get_szerokosc,
    gtx3010_get_wysokosc,
    gtx3010_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx2010 = {
    gtx2010_init,
    0,
    gtx2010_czy_gotowy,
    gtx2010_putpixel,
    gtx2010_fillrect,
    gtx2010_blt,
    gtx2010_cursor_set,
    gtx2010_cursor_hide,
    gtx2010_get_fb,
    gtx2010_get_szerokosc,
    gtx2010_get_wysokosc,
    gtx2010_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx1010 = {
    gtx1010_init,
    0,
    gtx1010_czy_gotowy,
    gtx1010_putpixel,
    gtx1010_fillrect,
    gtx1010_blt,
    gtx1010_cursor_set,
    gtx1010_cursor_hide,
    gtx1010_get_fb,
    gtx1010_get_szerokosc,
    gtx1010_get_wysokosc,
    gtx1010_get_device_id,
};

static struct hal_gfx_driver hal_drv_titanx = {
    titanx_init,
    0,
    titanx_czy_gotowy,
    titanx_putpixel,
    titanx_fillrect,
    titanx_blt,
    titanx_cursor_set,
    titanx_cursor_hide,
    titanx_get_fb,
    titanx_get_szerokosc,
    titanx_get_wysokosc,
    titanx_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx760 = {
    gtx760_init,
    gtx760_shutdown,
    gtx760_czy_gotowy,
    gtx760_putpixel,
    gtx760_fillrect,
    gtx760_blt,
    gtx760_cursor_set,
    gtx760_cursor_hide,
    gtx760_get_fb,
    gtx760_get_szerokosc,
    gtx760_get_wysokosc,
    gtx760_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx660 = {
    gtx660_init,
    gtx660_shutdown,
    gtx660_czy_gotowy,
    gtx660_putpixel,
    gtx660_fillrect,
    gtx660_blt,
    gtx660_cursor_set,
    gtx660_cursor_hide,
    gtx660_get_fb,
    gtx660_get_szerokosc,
    gtx660_get_wysokosc,
    gtx660_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx360 = {
    gtx360_init,
    gtx360_shutdown,
    gtx360_czy_gotowy,
    gtx360_putpixel,
    gtx360_fillrect,
    gtx360_blt,
    gtx360_cursor_set,
    gtx360_cursor_hide,
    gtx360_get_fb,
    gtx360_get_szerokosc,
    gtx360_get_wysokosc,
    gtx360_get_device_id,
};

static struct hal_gfx_driver hal_drv_gtx560 = {
    gtx560_init,
    gtx560_shutdown,
    gtx560_czy_gotowy,
    gtx560_putpixel,
    gtx560_fillrect,
    gtx560_blt,
    gtx560_cursor_set,
    gtx560_cursor_hide,
    gtx560_get_fb,
    gtx560_get_szerokosc,
    gtx560_get_wysokosc,
    gtx560_get_device_id,
};

static struct hal_gfx_driver hal_drv_qemu_vga = {
    qemu_vga_init,
    0,
    hal_wrap_qemu_czy_gotowy,
    qemu_vga_putpixel,
    qemu_vga_fillrect,
    qemu_vga_blt,
    hal_wrap_qemu_cursor_set,
    hal_wrap_qemu_cursor_hide,
    qemu_vga_get_fb,
    hal_wrap_qemu_get_szerokosc,
    hal_wrap_qemu_get_wysokosc,
    hal_wrap_qemu_get_device_id,
};

/* Lista wszystkich wspieranych sterownikow GPU (kolejnosc probowania) */
static struct hal_gfx_driver *g_hal_drivers[] = {
    &hal_drv_radeon_hd6000,
    &hal_drv_gtx1010,
    &hal_drv_gtx2010,
    &hal_drv_gtx3010,
    &hal_drv_gtx3070,
    &hal_drv_gtx3080,
    &hal_drv_gtx360,
    &hal_drv_gtx560,
    &hal_drv_gtx660,
    &hal_drv_gtx760,
    &hal_drv_titanx,
    &hal_drv_radeon_hd_8370d,
    &hal_drv_radeon_6870,
    &hal_drv_rx_9070_xt,
    &hal_drv_intel_uhd,
    &hal_drv_gma_x4500,
    &hal_drv_qemu_vga,
};

#define HAL_DRIVER_COUNT (sizeof(g_hal_drivers) / sizeof(g_hal_drivers[0]))

/* ------------------------------------------------------------------
 * Inicjalizacja HAL: probuje zainicjalizowac kazdy sterownik GPU
 * z listy i wybiera pierwszy, ktory zgłasza gotowosc.
 * ------------------------------------------------------------------ */

int hal_gfx_init(void)
{
    for (uint32_t i = 0; i < HAL_DRIVER_COUNT; i++) {
        struct hal_gfx_driver *drv = g_hal_drivers[i];
        if (!drv || !drv->init) continue;

        int rc = drv->init();
        if (rc <= 0) continue;

        if (drv->czy_gotowy && drv->czy_gotowy()) {
            g_hal_driver = drv;
            g_hal_mial_sterownik = 1;
            return 0;
        }

        /* Sterownik zainicjalizowany ale niegotowy - probujemy nastepny */
        if (drv->shutdown) drv->shutdown();
    }
    return -1;
}

void hal_gfx_shutdown(void)
{
    if (g_hal_driver && g_hal_driver->shutdown) {
        g_hal_driver->shutdown();
    }
    g_hal_driver = 0;
    g_hal_mial_sterownik = 0;
}

/* ------------------------------------------------------------------
 * Funkcje API HAL - przekazywanie wywolan do aktywnego sterownika
 * ------------------------------------------------------------------ */

uint8_t hal_gfx_czy_gotowy(void)
{
    if (!g_hal_driver || !g_hal_driver->czy_gotowy) return 0;
    return g_hal_driver->czy_gotowy();
}

void hal_gfx_putpixel(uint32_t x, uint32_t y, uint32_t kolor)
{
    if (!g_hal_driver || !g_hal_driver->putpixel) return;
    g_hal_driver->putpixel(x, y, kolor);
}

void hal_gfx_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor)
{
    if (!g_hal_driver || !g_hal_driver->fillrect) return;
    g_hal_driver->fillrect(x, y, w, h, kolor);
}

void hal_gfx_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                 uint32_t w, uint32_t h)
{
    if (!g_hal_driver || !g_hal_driver->blt) return;
    g_hal_driver->blt(dst_x, dst_y, src_x, src_y, w, h);
}

void hal_gfx_cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz)
{
    if (!g_hal_driver || !g_hal_driver->cursor_set) return;
    g_hal_driver->cursor_set(x, y, obraz);
}

void hal_gfx_cursor_hide(void)
{
    if (!g_hal_driver || !g_hal_driver->cursor_hide) return;
    g_hal_driver->cursor_hide();
}

void *hal_gfx_get_fb(void)
{
    if (!g_hal_driver || !g_hal_driver->get_fb) return 0;
    return g_hal_driver->get_fb();
}

uint32_t hal_gfx_get_szerokosc(void)
{
    if (!g_hal_driver || !g_hal_driver->get_szerokosc) return 0;
    return g_hal_driver->get_szerokosc();
}

uint32_t hal_gfx_get_wysokosc(void)
{
    if (!g_hal_driver || !g_hal_driver->get_wysokosc) return 0;
    return g_hal_driver->get_wysokosc();
}

uint16_t hal_gfx_get_device_id(void)
{
    if (!g_hal_driver || !g_hal_driver->get_device_id) return 0;
    return g_hal_driver->get_device_id();
}

uint8_t hal_gfx_mial_sterownik(void)
{
    return g_hal_mial_sterownik;
}
