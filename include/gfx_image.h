/**
 * @file gfx_image.h
 * @brief Images, icons and bitmaps.
 *
 *  - gfx_draw_image   : full RGB565 image (array of uint16_t in flash)
 *  - gfx_draw_icon    : 4-bit opacity icon, smooth over a known background
 *  - gfx_draw_bitmap1 : 1 bit per pixel bitmap (MSB on the left), with or
 *                       without a background
 *
 * Use tools/img2c.py to turn a PNG into a C array.
 */
#ifndef GFX_IMAGE_H_
#define GFX_IMAGE_H_

#include "gfx_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Draw a w x h RGB565 image with its top-left corner at (x, y). Clipped. */
void gfx_draw_image(int x, int y, int w, int h, const uint16_t *pixels);

/** Draw a 4-bit opacity icon over a known background colour. */
void gfx_draw_icon(int x, int y, const gfx_image_a4_t *icon, gfx_color_t fg, gfx_color_t bg);

/** Draw a 4-bit opacity icon without a background (no smoothing). */
void gfx_draw_icon_t(int x, int y, const gfx_image_a4_t *icon, gfx_color_t fg);

/** Draw a 1-bit bitmap with foreground and background colours. */
void gfx_draw_bitmap1(int x, int y, int w, int h, const uint8_t *bits, gfx_color_t fg, gfx_color_t bg);

/** Draw a 1-bit bitmap, painting only the set bits. */
void gfx_draw_bitmap1_t(int x, int y, int w, int h, const uint8_t *bits, gfx_color_t fg);

#ifdef __cplusplus
}
#endif

#endif /* GFX_IMAGE_H_ */
