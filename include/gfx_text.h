/**
 * @file gfx_text.h
 * @brief Text with proportional anti-aliased fonts.
 *
 * - Accepts plain UTF-8 strings (accents, tildes, degree sign...) as long as
 *   your source files are saved as UTF-8. Characters up to U+00FF are
 *   supported; any other code point is drawn as "?".
 * - x, y is the top-left corner of the text line. The line height is
 *   font->height.
 * - With a background colour (@p bg) the text is smooth and fast. Without one
 *   (the _t variants) there is nothing to blend with, so no smoothing is
 *   applied.
 * - Available fonts are declared in gfx_fonts.h (sans_12, sans_16, sans_24,
 *   sans_bold_16, sans_bold_24, sans_bold_48).
 */
#ifndef GFX_TEXT_H_
#define GFX_TEXT_H_

#include "gfx_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Horizontal alignment inside a text box. */
typedef enum { GFX_ALIGN_LEFT = 0, GFX_ALIGN_CENTER = 1, GFX_ALIGN_RIGHT = 2 } gfx_align_t;

/** Width of a string in pixels. */
int  gfx_text_width(const char *text, const gfx_font_t *font);

/**
 * @brief Draw text over a known background colour.
 * @return The x coordinate right after the last glyph.
 */
int  gfx_text(int x, int y, const char *text, const gfx_font_t *font, gfx_color_t fg, gfx_color_t bg);

/**
 * @brief Draw text with a transparent background (pixels at 50 % opacity or
 *        more are painted, no smoothing).
 * @return The x coordinate right after the last glyph.
 */
int  gfx_text_t(int x, int y, const char *text, const gfx_font_t *font, gfx_color_t fg);

/**
 * @brief Draw text inside a box of width @p w.
 *
 * Aligns the text and paints the leftover background, so nothing is left
 * behind when the text changes.
 */
void gfx_text_box(int x, int y, int w, const char *text, const gfx_font_t *font,
                  gfx_color_t fg, gfx_color_t bg, gfx_align_t align);

/**
 * @brief Convert an integer to text without printf.
 * @param buf Output buffer, at least 12 bytes.
 * @return @p buf.
 */
char *gfx_int_to_str(int32_t value, char *buf);

#ifdef __cplusplus
}
#endif

#endif /* GFX_TEXT_H_ */
