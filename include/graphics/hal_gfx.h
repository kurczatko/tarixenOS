#ifndef HAL_GFX_H
#define HAL_GFX_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------
 * Warstwa abstrakcji HAL miedzy systemem a kartami graficznymi.
 * Pojedyncze funkcje API (putpixel, fillrect, blit, cursor itd.)
 * przekazuja rysowanie do funkcji sterownika zaladowanego
 * przez driver manager. Obsluguje KAZDY sterownik kart graficznej
 * ktory implementuje wspolny interfejs.
 * ------------------------------------------------------------ */

/* Wskazniki na funkcje sterownika (vtable) */
struct hal_gfx_driver {
    int   (*init)(void);
    void  (*shutdown)(void);
    uint8_t (*czy_gotowy)(void);
    void  (*putpixel)(uint32_t x, uint32_t y, uint32_t kolor);
    void  (*fillrect)(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor);
    void  (*blt)(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                 uint32_t w, uint32_t h);
    void  (*cursor_set)(uint32_t x, uint32_t y, const uint32_t *obraz);
    void  (*cursor_hide)(void);
    void *(*get_fb)(void);
    uint32_t (*get_szerokosc)(void);
    uint32_t (*get_wysokosc)(void);
    uint16_t (*get_device_id)(void);
};

/* Inicjalizacja HAL - probuje nastepujacy sterowniki GPU i wybiera
 * pierwszy, ktory zostanie pomyslnie zainicjalizowany. */
int hal_gfx_init(void);
void hal_gfx_shutdown(void);

/* Funkcje API - przekazuja wywolanie do aktywnego sterownika GPU */
uint8_t hal_gfx_czy_gotowy(void);
void hal_gfx_putpixel(uint32_t x, uint32_t y, uint32_t kolor);
void hal_gfx_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor);
void hal_gfx_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y,
                 uint32_t w, uint32_t h);
void hal_gfx_cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz);
void hal_gfx_cursor_hide(void);
void *hal_gfx_get_fb(void);
uint32_t hal_gfx_get_szerokosc(void);
uint32_t hal_gfx_get_wysokosc(void);
uint16_t hal_gfx_get_device_id(void);

/* Czy HAL ma aktywny sterownik GPU */
uint8_t hal_gfx_mial_sterownik(void);

#ifdef __cplusplus
}
#endif

#endif
