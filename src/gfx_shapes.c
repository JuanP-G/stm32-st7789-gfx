/* gfx_shapes.c - geometric shapes painted row by row */
#include "gfx_shapes.h"

#define GFX_MAX_POLY  16

/* ---------- Maths ---------- */

uint32_t gfx_isqrt(uint32_t value)
{
	uint32_t result = 0, bit = 1u << 30;
	while (bit > value) bit >>= 2;
	while (bit) {
		if (value >= result + bit) { value -= result + bit; result = (result >> 1) + bit; }
		else result >>= 1;
		bit >>= 2;
	}
	return result;
}

static const int16_t gfx_sin_table[91] = {   /* sine from 0 to 90 degrees, Q14 */
	0,286,572,857,1143,1428,1713,1997,2280,2563,2845,3126,3406,3686,3964,4240,4516,4790,5063,5334,
	5604,5872,6138,6402,6664,6924,7182,7438,7692,7943,8192,8438,8682,8923,9162,9397,9630,9860,10087,10311,
	10531,10749,10963,11174,11381,11585,11786,11982,12176,12365,12551,12733,12911,13085,13255,13421,13583,13741,13894,14044,
	14189,14330,14466,14598,14726,14849,14968,15082,15191,15296,15396,15491,15582,15668,15749,15826,15897,15964,16026,16083,
	16135,16182,16225,16262,16294,16322,16344,16362,16374,16382,16384
};

int gfx_sin_q14(int degrees)
{
	degrees %= 360;
	if (degrees < 0) degrees += 360;
	if (degrees <= 90)  return  gfx_sin_table[degrees];
	if (degrees <= 180) return  gfx_sin_table[180 - degrees];
	if (degrees <= 270) return -gfx_sin_table[degrees - 180];
	return -gfx_sin_table[360 - degrees];
}

int gfx_cos_q14(int degrees) { return gfx_sin_q14(degrees + 90); }

static int gfx_imin(int a, int b) { return a < b ? a : b; }
static int gfx_iabs(int a) { return a < 0 ? -a : a; }

/* ---------- Lines ---------- */

/* Bresenham grouping pixels into runs: each run is a single burst */
void gfx_line(int x0, int y0, int x1, int y1, gfx_color_t color)
{
	int dx = gfx_iabs(x1 - x0), dy = gfx_iabs(y1 - y0);
	if (dy == 0) { gfx_fill_rect(gfx_imin(x0, x1), y0, dx + 1, 1, color); return; }
	if (dx == 0) { gfx_fill_rect(x0, gfx_imin(y0, y1), 1, dy + 1, color); return; }

	int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
	if (dx >= dy) {                      /* shallow slope: horizontal runs */
		int err = dx / 2, y = y0, start = x0;
		for (int x = x0; ; x += sx) {
			if (x == x1) { gfx_fill_rect(gfx_imin(start, x), y, gfx_iabs(x - start) + 1, 1, color); break; }
			err -= dy;
			if (err < 0) {
				gfx_fill_rect(gfx_imin(start, x), y, gfx_iabs(x - start) + 1, 1, color);
				y += sy; err += dx; start = x + sx;
			}
		}
	} else {                             /* steep slope: vertical runs */
		int err = dy / 2, x = x0, start = y0;
		for (int y = y0; ; y += sy) {
			if (y == y1) { gfx_fill_rect(x, gfx_imin(start, y), 1, gfx_iabs(y - start) + 1, color); break; }
			err -= dx;
			if (err < 0) {
				gfx_fill_rect(x, gfx_imin(start, y), 1, gfx_iabs(y - start) + 1, color);
				x += sx; err += dy; start = y + sy;
			}
		}
	}
}

/* ---------- Polygon fill (even-odd, coordinates in 1/16 pixel) ---------- */

static void gfx_fill_poly_q4(const int32_t *px, const int32_t *py, int n, gfx_color_t color)
{
	int32_t ymin = py[0], ymax = py[0];
	for (int i = 1; i < n; i++) { if (py[i] < ymin) ymin = py[i]; if (py[i] > ymax) ymax = py[i]; }
	int row0 = (int)((ymin - 8 + 15) >> 4);          /* first row whose centre is inside */
	int row1 = (int)(((ymax - 8 + 15) >> 4) - 1);    /* last row */
	if (row0 < 0) row0 = 0;
	if (row1 > GFX_HEIGHT - 1) row1 = GFX_HEIGHT - 1;

	for (int row = row0; row <= row1; row++) {
		int32_t yc = (int32_t)(2 * row + 1) * 8;     /* centre of the row */
		int32_t xs[GFX_MAX_POLY];
		int k = 0;
		for (int i = 0; i < n; i++) {
			int j = (i + 1) % n;
			int32_t ya = py[i], yb = py[j], xa = px[i], xb = px[j];
			if (ya == yb) continue;
			if (ya > yb) { int32_t t = ya; ya = yb; yb = t; t = xa; xa = xb; xb = t; }
			if (yc < ya || yc >= yb) continue;
			xs[k++] = xa + ((xb - xa) * (yc - ya)) / (yb - ya);
		}
		for (int a = 1; a < k; a++) {                /* insertion sort */
			int32_t v = xs[a]; int b = a - 1;
			while (b >= 0 && xs[b] > v) { xs[b + 1] = xs[b]; b--; }
			xs[b + 1] = v;
		}
		for (int a = 0; a + 1 < k; a += 2) {
			int xstart = (int)((xs[a] - 8 + 15) >> 4);
			int xend   = (int)(((xs[a + 1] - 8 + 15) >> 4) - 1);
			if (xend >= xstart) gfx_fill_rect(xstart, row, xend - xstart + 1, 1, color);
		}
	}
}

void gfx_fill_polygon(const gfx_point_t *points, int count, gfx_color_t color)
{
	if (count < 3 || count > GFX_MAX_POLY) return;
	int32_t px[GFX_MAX_POLY], py[GFX_MAX_POLY];
	for (int i = 0; i < count; i++) { px[i] = points[i].x * 16; py[i] = points[i].y * 16; }
	gfx_fill_poly_q4(px, py, count, color);
}

void gfx_polygon(const gfx_point_t *points, int count, gfx_color_t color)
{
	for (int i = 0; i < count; i++) {
		int j = (i + 1) % count;
		gfx_line(points[i].x, points[i].y, points[j].x, points[j].y, color);
	}
}

void gfx_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, gfx_color_t color)
{
	gfx_point_t p[3] = { {(int16_t)x0, (int16_t)y0}, {(int16_t)x1, (int16_t)y1}, {(int16_t)x2, (int16_t)y2} };
	gfx_fill_polygon(p, 3, color);
}

void gfx_triangle(int x0, int y0, int x1, int y1, int x2, int y2, gfx_color_t color)
{
	gfx_line(x0, y0, x1, y1, color);
	gfx_line(x1, y1, x2, y2, color);
	gfx_line(x2, y2, x0, y0, color);
}

/* Thick line = rotated rectangle (polygon) + a circle at each end */
void gfx_line_thick(int x0, int y0, int x1, int y1, int thickness, gfx_color_t color)
{
	if (thickness <= 1) { gfx_line(x0, y0, x1, y1, color); return; }
	int dx = x1 - x0, dy = y1 - y0;
	int len = (int)gfx_isqrt((uint32_t)(dx * dx + dy * dy));
	if (len > 0) {
		int32_t ox = (int32_t)(-dy) * (thickness * 8) / len;     /* half width, in 1/16 px */
		int32_t oy = (int32_t)( dx) * (thickness * 8) / len;
		int32_t ax = x0 * 16 + 8, ay = y0 * 16 + 8, bx = x1 * 16 + 8, by = y1 * 16 + 8;
		int32_t px[4] = { ax + ox, bx + ox, bx - ox, ax - ox };
		int32_t py[4] = { ay + oy, by + oy, by - oy, ay - oy };
		gfx_fill_poly_q4(px, py, 4, color);
	}
	gfx_fill_circle(x0, y0, thickness / 2, color);
	if (len > 0) gfx_fill_circle(x1, y1, thickness / 2, color);
}

/* ---------- Rectangles ---------- */

void gfx_rect_thick(int x, int y, int w, int h, int thickness, gfx_color_t color)
{
	if (w <= 0 || h <= 0) return;
	if (thickness < 1) thickness = 1;
	if (2 * thickness >= w || 2 * thickness >= h) { gfx_fill_rect(x, y, w, h, color); return; }
	gfx_fill_rect(x, y, w, thickness, color);                                  /* top */
	gfx_fill_rect(x, y + h - thickness, w, thickness, color);                  /* bottom */
	gfx_fill_rect(x, y + thickness, thickness, h - 2 * thickness, color);      /* left */
	gfx_fill_rect(x + w - thickness, y + thickness, thickness, h - 2 * thickness, color);  /* right */
}

void gfx_rect(int x, int y, int w, int h, gfx_color_t color) { gfx_rect_thick(x, y, w, h, 1, color); }

/* How many pixels the rounded corner removes in row i (0 = border row) */
static int gfx_corner_inset(int r, int i)
{
	for (int k = 0; k < r; k++) {
		int a = 2 * r - 2 * k - 1, b = 2 * r - 2 * i - 1;
		if (a * a + b * b <= 4 * r * r) return k;
	}
	return r;
}

/* Horizontal trim of row i of a rectangle of height h with corner radius r */
static int gfx_row_inset(int h, int r, int i)
{
	if (r <= 0) return 0;
	if (i < r) return gfx_corner_inset(r, i);
	if (i >= h - r) return gfx_corner_inset(r, h - 1 - i);
	return 0;
}

static int gfx_clamp_radius(int w, int h, int r)
{
	int max_r = gfx_imin(w, h) / 2;
	return r > max_r ? max_r : (r < 0 ? 0 : r);
}

void gfx_fill_round_rect(int x, int y, int w, int h, int radius, gfx_color_t color)
{
	if (w <= 0 || h <= 0) return;
	int r = gfx_clamp_radius(w, h, radius);
	int i = 0;
	while (i < h) {                                   /* group rows with the same trim */
		int ins = gfx_row_inset(h, r, i), j = i;
		while (j + 1 < h && gfx_row_inset(h, r, j + 1) == ins) j++;
		gfx_fill_rect(x + ins, y + i, w - 2 * ins, j - i + 1, color);
		i = j + 1;
	}
}

void gfx_fill_round_rect_vgradient(int x, int y, int w, int h, int radius, gfx_color_t top, gfx_color_t bottom)
{
	if (w <= 0 || h <= 0) return;
	int r = gfx_clamp_radius(w, h, radius);
	for (int i = 0; i < h; i++) {
		int ins = gfx_row_inset(h, r, i);
		uint8_t t = (h > 1) ? (uint8_t)((i * 255) / (h - 1)) : 0;
		gfx_fill_rect(x + ins, y + i, w - 2 * ins, 1, gfx_blend(top, bottom, t));
	}
}

void gfx_round_rect(int x, int y, int w, int h, int radius, int thickness, gfx_color_t color)
{
	if (w <= 0 || h <= 0) return;
	if (thickness < 1) thickness = 1;
	if (2 * thickness >= w || 2 * thickness >= h) { gfx_fill_round_rect(x, y, w, h, radius, color); return; }
	int r = gfx_clamp_radius(w, h, radius);
	int ri = r - thickness; if (ri < 0) ri = 0;
	int ih = h - 2 * thickness;
	for (int i = 0; i < h; i++) {
		int o = gfx_row_inset(h, r, i);
		if (i < thickness || i >= h - thickness) {                    /* top / bottom band */
			gfx_fill_rect(x + o, y + i, w - 2 * o, 1, color);
			continue;
		}
		int j = thickness + gfx_row_inset(ih, ri, i - thickness);    /* inner edge of this row */
		if (j <= o) { gfx_fill_rect(x + o, y + i, w - 2 * o, 1, color); continue; }
		gfx_fill_rect(x + o, y + i, j - o, 1, color);                /* left run */
		gfx_fill_rect(x + w - j, y + i, j - o, 1, color);            /* right run */
	}
}

/* ---------- Circles ---------- */

/* Pixel (dx,dy) is inside radius r if dx^2 + dy^2 <= r^2 + r (rounder circle) */
void gfx_ring(int cx, int cy, int radius, int thickness, gfx_color_t color)
{
	if (radius < 0) return;
	if (thickness < 1) thickness = 1;
	int ri = radius - thickness;                       /* inner radius */
	uint32_t ro2 = (uint32_t)(radius * radius + radius);
	uint32_t ri2 = (ri >= 1) ? (uint32_t)(ri * ri + ri) : 0;
	for (int dy = -radius; dy <= radius; dy++) {
		uint32_t dy2 = (uint32_t)(dy * dy);
		if (dy2 > ro2) continue;
		int ho = (int)gfx_isqrt(ro2 - dy2);
		if (ri >= 1 && dy2 <= ri2) {
			int hi = (int)gfx_isqrt(ri2 - dy2);
			gfx_fill_rect(cx - ho, cy + dy, ho - hi, 1, color);          /* left run */
			gfx_fill_rect(cx + hi + 1, cy + dy, ho - hi, 1, color);      /* right run */
		} else {
			gfx_fill_rect(cx - ho, cy + dy, 2 * ho + 1, 1, color);
		}
	}
}

void gfx_circle(int cx, int cy, int radius, gfx_color_t color) { gfx_ring(cx, cy, radius, 1, color); }
void gfx_fill_circle(int cx, int cy, int radius, gfx_color_t color) { gfx_ring(cx, cy, radius, radius + 1, color); }

/* ---------- Arcs ---------- */

void gfx_arc(int cx, int cy, int radius, int thickness, int start_deg, int end_deg, gfx_color_t color)
{
	int sweep = end_deg - start_deg;
	if (sweep <= 0 || radius < 0) return;
	if (sweep >= 360) { gfx_ring(cx, cy, radius, thickness, color); return; }
	if (thickness < 1) thickness = 1;

	/* Start and end directions (screen: x right, y down; 0 degrees = up) */
	int32_t v0x = gfx_sin_q14(start_deg), v0y = -gfx_cos_q14(start_deg);
	int32_t v1x = gfx_sin_q14(end_deg),   v1y = -gfx_cos_q14(end_deg);

	int ri = radius - thickness;
	uint32_t ro2 = (uint32_t)(radius * radius + radius);
	uint32_t ri2 = (ri >= 1) ? (uint32_t)(ri * ri + ri) : 0;

	for (int dy = -radius; dy <= radius; dy++) {
		uint32_t dy2 = (uint32_t)(dy * dy);
		if (dy2 > ro2) continue;
		int ho = (int)gfx_isqrt(ro2 - dy2);
		int hi = -1;                                           /* -1 = no inner hole */
		if (ri >= 1 && dy2 <= ri2) hi = (int)gfx_isqrt(ri2 - dy2);

		int run_start = 0, in_run = 0;
		for (int dx = -ho; dx <= ho + 1; dx++) {                /* +1 closes the last run */
			int inside = 0;
			if (dx <= ho && !(hi >= 0 && dx >= -hi && dx <= hi)) {
				int32_t c0 = v0x * dy - v0y * dx;               /* cross(v0, p) */
				int32_t c1 = dx * v1y - dy * v1x;               /* cross(p, v1) */
				if (sweep <= 180) inside = (c0 >= 0 && c1 >= 0);
				else              inside = !((v1x * dy - v1y * dx) > 0 && (dx * v0y - dy * v0x) > 0);
			}
			if (inside && !in_run) { in_run = 1; run_start = dx; }
			if (!inside && in_run) { in_run = 0; gfx_fill_rect(cx + run_start, cy + dy, dx - run_start, 1, color); }
		}
	}
}
