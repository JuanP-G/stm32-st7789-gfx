/* gfx_core.c - window, burst, rectangles and colour */
#include "gfx_core.h"

#define GFX_CMD_CASET  0x2A
#define GFX_CMD_RASET  0x2B
#define GFX_CMD_RAMWR  0x2C

void gfx_set_window(int x0, int y0, int x1, int y1)
{
	gfx_port_cmd(GFX_CMD_CASET);
	gfx_port_data((uint16_t)(x0 >> 8)); gfx_port_data((uint16_t)(x0 & 0xFF));
	gfx_port_data((uint16_t)(x1 >> 8)); gfx_port_data((uint16_t)(x1 & 0xFF));
	gfx_port_cmd(GFX_CMD_RASET);
	gfx_port_data((uint16_t)(y0 >> 8)); gfx_port_data((uint16_t)(y0 & 0xFF));
	gfx_port_data((uint16_t)(y1 >> 8)); gfx_port_data((uint16_t)(y1 & 0xFF));
	gfx_port_cmd(GFX_CMD_RAMWR);
}

void gfx_push(gfx_color_t color)
{
	gfx_port_data(color);
}

void gfx_push_repeat(gfx_color_t color, uint32_t count)
{
	while (count--)
		gfx_port_data(color);
}

/* Clip the rectangle to the screen. Returns 0 if it ends up empty. */
int gfx_clip_rect(int *x, int *y, int *w, int *h)
{
	int x1 = *x + *w, y1 = *y + *h;      /* right and bottom limits (exclusive) */
	if (*x < 0) *x = 0;
	if (*y < 0) *y = 0;
	if (x1 > GFX_WIDTH)  x1 = GFX_WIDTH;
	if (y1 > GFX_HEIGHT) y1 = GFX_HEIGHT;
	*w = x1 - *x;
	*h = y1 - *y;
	return (*w > 0 && *h > 0);
}

void gfx_fill_rect(int x, int y, int w, int h, gfx_color_t color)
{
	if (!gfx_clip_rect(&x, &y, &w, &h)) return;
	gfx_set_window(x, y, x + w - 1, y + h - 1);
	gfx_push_repeat(color, (uint32_t)w * (uint32_t)h);
}

void gfx_pixel(int x, int y, gfx_color_t color)
{
	if (x < 0 || y < 0 || x >= GFX_WIDTH || y >= GFX_HEIGHT) return;
	gfx_set_window(x, y, x, y);
	gfx_push(color);
}

void gfx_fill_screen(gfx_color_t color)
{
	gfx_fill_rect(0, 0, GFX_WIDTH, GFX_HEIGHT, color);
}

void gfx_hline(int x, int y, int w, gfx_color_t color) { gfx_fill_rect(x, y, w, 1, color); }
void gfx_vline(int x, int y, int h, gfx_color_t color) { gfx_fill_rect(x, y, 1, h, color); }

/* ---------- Colour ---------- */

gfx_color_t gfx_blend(gfx_color_t a, gfx_color_t b, uint8_t t)
{
	uint32_t inv = 255u - t;
	uint32_t r = ((((a >> 11) & 0x1F) * inv) + (((b >> 11) & 0x1F) * t) + 127u) / 255u;
	uint32_t g = ((((a >> 5)  & 0x3F) * inv) + (((b >> 5)  & 0x3F) * t) + 127u) / 255u;
	uint32_t bl = ((((a)      & 0x1F) * inv) + (((b)       & 0x1F) * t) + 127u) / 255u;
	return (gfx_color_t)((r << 11) | (g << 5) | bl);
}

gfx_color_t gfx_lighten(gfx_color_t c, uint8_t amount) { return gfx_blend(c, GFX_WHITE, amount); }
gfx_color_t gfx_darken(gfx_color_t c, uint8_t amount)  { return gfx_blend(c, GFX_BLACK, amount); }

/* Vertical gradient: one row of each colour, mixing from top to bottom */
void gfx_fill_rect_vgradient(int x, int y, int w, int h, gfx_color_t top, gfx_color_t bottom)
{
	for (int i = 0; i < h; i++) {
		uint8_t t = (h > 1) ? (uint8_t)((i * 255) / (h - 1)) : 0;
		gfx_fill_rect(x, y + i, w, 1, gfx_blend(top, bottom, t));
	}
}

/* Horizontal gradient: one column of each colour */
void gfx_fill_rect_hgradient(int x, int y, int w, int h, gfx_color_t left, gfx_color_t right)
{
	for (int i = 0; i < w; i++) {
		uint8_t t = (w > 1) ? (uint8_t)((i * 255) / (w - 1)) : 0;
		gfx_fill_rect(x + i, y, 1, h, gfx_blend(left, right, t));
	}
}
