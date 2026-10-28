#ifndef BROWSER_CSS_H
#define BROWSER_CSS_H

#include "../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct browser_style {
    uint32_t color;
    uint32_t background;
    uint8_t has_background;
    uint8_t bold;
    uint8_t rozmiar;
    uint32_t border_color;
    uint8_t has_border_color;
    uint16_t border_width;
    uint8_t has_border_width;
    uint16_t padding;
    uint8_t has_padding;
    uint16_t margin;
    uint8_t has_margin;
    uint8_t text_align;
    uint8_t display;
    uint16_t width;
    uint8_t has_width;
    char cursor[16];
    uint8_t has_cursor;
};

void css_init(void);
void css_parse_stylesheet(const char *text, int length);
void css_set_pseudo(const char *target, const char *pseudo, int on);
void css_compute(const char *tag, const char *class_name, const char *id,
                 const char *pseudo, struct browser_style *out);

#ifdef __cplusplus
}
#endif

#endif
