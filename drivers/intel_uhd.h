#ifndef INTEL_UHD_H
#define INTEL_UHD_H

#include "../include/printf/types.h"

#define INTEL_UHD_VENDOR_ID 0x8086
#define INTEL_UHD_FB_ADDR 0xFD000000U
#define INTEL_UHD_FB_WIDTH 1024U
#define INTEL_UHD_FB_HEIGHT 768U
#define INTEL_UHD_FB_PITCH 4096U

#ifdef __cplusplus
extern "C" {
#endif

int intel_uhd_init(void);
void intel_uhd_putpixel(uint32_t x, uint32_t y, uint32_t color);
void intel_uhd_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void intel_uhd_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h);
void intel_uhd_cursor_set(uint32_t x, uint32_t y, const uint32_t *image);
void intel_uhd_cursor_hide(void);
void *intel_uhd_get_fb(void);
uint32_t intel_uhd_get_width(void);
uint32_t intel_uhd_get_height(void);
uint16_t intel_uhd_get_device_id(void);
uint8_t intel_uhd_get_revision(void);

#ifdef __cplusplus
}
#endif

#endif
