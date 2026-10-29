/**
 * @file gfx_core.h
 * @brief Types, colour helpers and basic painting with window + burst.
 *
 * Instead of sending CASET + RASET + RAMWR for every pixel (12 bus writes
 * per pixel), the library defines the address window once and then streams
 * all pixels back to back. The controller advances through the window by
 * itself. Every drawing call is clipped to the screen first.
 *
 * Display: 240 x 240 pixels, RGB565 (5 bits red, 6 bits green, 5 bits blue).
 */
#ifndef GFX_CORE_H_
#define GFX_CORE_H_

#include <stdint.h>
#include <stddef.h>
#include "gfx_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Library version. */
#define GFX_VERSION "1.0.0"

/** Screen width in pixels. */
#define GFX_WIDTH   240
/** Screen height in pixels. */
#define GFX_HEIGHT  240

/** A colour in RGB565 format. */
typedef uint16_t gfx_color_t;

/** Build a colour from 8-bit components (0..255). */
#define GFX_RGB(r, g, b)  ((gfx_color_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xFF) >> 3)))
/** Build a colour from a 0xRRGGBB constant. */
#define GFX_HEX(rgb)      GFX_RGB((((rgb) >> 16) & 0xFF), (((rgb) >> 8) & 0xFF), ((rgb) & 0xFF))

#define GFX_BLACK  ((gfx_color_t)0x0000)
#define GFX_WHITE  ((gfx_color_t)0xFFFF)

/* ---- Font and image types (the data lives in gfx_fonts / gfx_icons) ---- */

/** One glyph of a proportional font. */
typedef struct {
	uint16_t offset;    /**< position of the glyph in the bitmap */
	uint8_t  width;     /**< width of the drawn shape (0 = blank, e.g. space) */
	uint8_t  advance;   /**< how far the cursor moves after this glyph */
	int8_t   xoff;      /**< horizontal offset of the shape inside its cell */
} gfx_glyph_t;

/** An anti-aliased bitmap font (4 bits of opacity per pixel). */
typedef struct {
	const gfx_glyph_t *glyphs;
	const uint8_t     *bitmap;       /**< 4 bits per pixel, 2 pixels per byte */
	uint8_t  height;                 /**< line height in pixels */
	uint8_t  baseline;               /**< distance from the top edge to the baseline */
	uint16_t first0, last0;          /**< first range of character codes (Unicode) */
	uint16_t first1, last1;          /**< second range (first > last means empty) */
} gfx_font_t;

/** A 4-bit opacity image (icons, glyphs). Rows are (width + 1) / 2 bytes. */
typedef struct {
	uint8_t width, height;
	const uint8_t *data;
} gfx_image_a4_t;

/* ---- Basic painting ---- */

/** Set the address window: CASET + RASET + RAMWR. Does not clip. */
void gfx_set_window(int x0, int y0, int x1, int y1);

/** Send one pixel inside the current window. */
void gfx_push(gfx_color_t color);

/** Send @p count copies of @p color inside the current window. */
void gfx_push_repeat(gfx_color_t color, uint32_t count);

/**
 * @brief Clip a rectangle to the screen.
 * @return 0 if nothing is left after clipping, non-zero otherwise.
 */
int  gfx_clip_rect(int *x, int *y, int *w, int *h);

/** Draw one pixel (clipped). */
void gfx_pixel(int x, int y, gfx_color_t color);

/** Fill a rectangle (clipped) with one window and one burst. */
void gfx_fill_rect(int x, int y, int w, int h, gfx_color_t color);

/** Fill the whole screen. */
void gfx_fill_screen(gfx_color_t color);

/** Horizontal line of width @p w. */
void gfx_hline(int x, int y, int w, gfx_color_t color);

/** Vertical line of height @p h. */
void gfx_vline(int x, int y, int h, gfx_color_t color);

/* ---- Colour utilities ---- */

/** Mix two colours: t = 0 gives @p a, t = 255 gives @p b. */
gfx_color_t gfx_blend(gfx_color_t a, gfx_color_t b, uint8_t t);

/** Mix a colour with white. */
gfx_color_t gfx_lighten(gfx_color_t c, uint8_t amount);

/** Mix a colour with black. */
gfx_color_t gfx_darken(gfx_color_t c, uint8_t amount);

/** Vertical gradient, one row per colour step. */
void gfx_fill_rect_vgradient(int x, int y, int w, int h, gfx_color_t top, gfx_color_t bottom);

/** Horizontal gradient, one column per colour step. */
void gfx_fill_rect_hgradient(int x, int y, int w, int h, gfx_color_t left, gfx_color_t right);

#ifdef __cplusplus
}
#endif

#endif /* GFX_CORE_H_ */
