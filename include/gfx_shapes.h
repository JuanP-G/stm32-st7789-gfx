/**
 * @file gfx_shapes.h
 * @brief Lines, rectangles, circles, arcs and polygons.
 *
 * Everything is painted row by row (one burst per row run), never pixel by
 * pixel, and every call is clipped to the screen. Integer arithmetic only.
 */
#ifndef GFX_SHAPES_H_
#define GFX_SHAPES_H_

#include "gfx_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A point with 16-bit signed coordinates. */
typedef struct { int16_t x, y; } gfx_point_t;

/* ---- Lines ---- */

/** One-pixel line (Bresenham, grouped into runs so each run is one burst). */
void gfx_line(int x0, int y0, int x1, int y1, gfx_color_t color);

/** Thick line with round caps. */
void gfx_line_thick(int x0, int y0, int x1, int y1, int thickness, gfx_color_t color);

/* ---- Rectangles ---- */

/** One-pixel rectangle outline. */
void gfx_rect(int x, int y, int w, int h, gfx_color_t color);

/** Rectangle outline whose border grows inwards. */
void gfx_rect_thick(int x, int y, int w, int h, int thickness, gfx_color_t color);

/** Rounded rectangle outline; the border grows inwards. */
void gfx_round_rect(int x, int y, int w, int h, int radius, int thickness, gfx_color_t color);

/** Filled rounded rectangle. The radius is clamped to half the shorter side. */
void gfx_fill_round_rect(int x, int y, int w, int h, int radius, gfx_color_t color);

/** Filled rounded rectangle with a vertical gradient. */
void gfx_fill_round_rect_vgradient(int x, int y, int w, int h, int radius, gfx_color_t top, gfx_color_t bottom);

/* ---- Circles (cx, cy = centre) ---- */

/** One-pixel circle outline. */
void gfx_circle(int cx, int cy, int radius, gfx_color_t color);

/** Filled circle. */
void gfx_fill_circle(int cx, int cy, int radius, gfx_color_t color);

/** Ring; the thickness grows inwards from @p radius. */
void gfx_ring(int cx, int cy, int radius, int thickness, gfx_color_t color);

/**
 * @brief Arc: a ring segment between two angles.
 *
 * Angles are in degrees, 0 = up, positive = clockwise (like a clock).
 * Well suited to circular progress indicators.
 */
void gfx_arc(int cx, int cy, int radius, int thickness, int start_deg, int end_deg, gfx_color_t color);

/* ---- Triangles and polygons ---- */

/** Triangle outline. */
void gfx_triangle(int x0, int y0, int x1, int y1, int x2, int y2, gfx_color_t color);

/** Filled triangle. */
void gfx_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, gfx_color_t color);

/** Polygon outline. */
void gfx_polygon(const gfx_point_t *points, int count, gfx_color_t color);

/** Filled polygon (even-odd rule, concave allowed, at most 16 vertices). */
void gfx_fill_polygon(const gfx_point_t *points, int count, gfx_color_t color);

/* ---- Float-free maths ---- */

/** Integer square root (floor). */
uint32_t gfx_isqrt(uint32_t value);

/** Sine of an angle in degrees, scaled by 16384 (Q14). */
int gfx_sin_q14(int degrees);

/** Cosine of an angle in degrees, scaled by 16384 (Q14). */
int gfx_cos_q14(int degrees);

#ifdef __cplusplus
}
#endif

#endif /* GFX_SHAPES_H_ */
