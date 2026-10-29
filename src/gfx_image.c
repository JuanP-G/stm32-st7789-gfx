/* gfx_image.c - images, icons and bitmaps */
#include "gfx_image.h"

void gfx_draw_image(int x, int y, int w, int h, const uint16_t *pixels)
{
	int cx = x, cy = y, cw = w, ch = h;
	if (!gfx_clip_rect(&cx, &cy, &cw, &ch)) return;
	gfx_set_window(cx, cy, cx + cw - 1, cy + ch - 1);
	for (int row = cy; row < cy + ch; row++) {
		const uint16_t *line = pixels + (row - y) * w + (cx - x);
		for (int i = 0; i < cw; i++) gfx_push(line[i]);
	}
}

void gfx_draw_icon(int x, int y, const gfx_image_a4_t *icon, gfx_color_t fg, gfx_color_t bg)
{
	gfx_color_t lut[16];
	for (int a = 0; a < 16; a++) lut[a] = gfx_blend(bg, fg, (uint8_t)(a * 17));
	int cx = x, cy = y, cw = icon->width, ch = icon->height, stride = (icon->width + 1) / 2;
	if (!gfx_clip_rect(&cx, &cy, &cw, &ch)) return;
	gfx_set_window(cx, cy, cx + cw - 1, cy + ch - 1);
	for (int row = cy; row < cy + ch; row++) {
		const uint8_t *line = icon->data + (row - y) * stride;
		for (int col = cx; col < cx + cw; col++) {
			int gx = col - x;
			gfx_push(lut[(gx & 1) ? (line[gx >> 1] & 0x0F) : (line[gx >> 1] >> 4)]);
		}
	}
}

void gfx_draw_icon_t(int x, int y, const gfx_image_a4_t *icon, gfx_color_t fg)
{
	int stride = (icon->width + 1) / 2;
	for (int row = 0; row < icon->height; row++) {
		const uint8_t *line = icon->data + row * stride;
		int run = -1;
		for (int gx = 0; gx <= icon->width; gx++) {
			int on = 0;
			if (gx < icon->width) on = ((gx & 1) ? (line[gx >> 1] & 0x0F) : (line[gx >> 1] >> 4)) >= 8;
			if (on && run < 0) run = gx;
			if (!on && run >= 0) { gfx_fill_rect(x + run, y + row, gx - run, 1, fg); run = -1; }
		}
	}
}

void gfx_draw_bitmap1(int x, int y, int w, int h, const uint8_t *bits, gfx_color_t fg, gfx_color_t bg)
{
	int cx = x, cy = y, cw = w, ch = h, stride = (w + 7) / 8;
	if (!gfx_clip_rect(&cx, &cy, &cw, &ch)) return;
	gfx_set_window(cx, cy, cx + cw - 1, cy + ch - 1);
	for (int row = cy; row < cy + ch; row++) {
		const uint8_t *line = bits + (row - y) * stride;
		for (int col = cx; col < cx + cw; col++) {
			int gx = col - x;
			gfx_push((line[gx >> 3] & (0x80 >> (gx & 7))) ? fg : bg);
		}
	}
}

void gfx_draw_bitmap1_t(int x, int y, int w, int h, const uint8_t *bits, gfx_color_t fg)
{
	int stride = (w + 7) / 8;
	for (int row = 0; row < h; row++) {
		const uint8_t *line = bits + row * stride;
		int run = -1;
		for (int gx = 0; gx <= w; gx++) {
			int on = (gx < w) && (line[gx >> 3] & (0x80 >> (gx & 7)));
			if (on && run < 0) run = gx;
			if (!on && run >= 0) { gfx_fill_rect(x + run, y + row, gx - run, 1, fg); run = -1; }
		}
	}
}
