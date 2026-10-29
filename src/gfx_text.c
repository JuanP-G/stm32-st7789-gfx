/* gfx_text.c - UTF-8 decoding, glyph lookup and anti-aliased painting */
#include "gfx_text.h"

/* Next character (Unicode code point) of a UTF-8 string. Only up to U+00FF. */
static uint32_t gfx_next_codepoint(const char **s)
{
	const uint8_t *p = (const uint8_t *)*s;
	uint32_t c = p[0];
	if (c < 0x80) { *s += 1; return c; }
	if ((c & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80) {       /* 2 bytes: U+0080..U+07FF */
		*s += 2;
		c = ((c & 0x1F) << 6) | (p[1] & 0x3F);
		return c <= 0xFF ? c : '?';
	}
	*s += 1;                                                  /* stray byte or long sequence */
	while ((((const uint8_t *)*s)[0] & 0xC0) == 0x80) *s += 1;
	return '?';
}

static const gfx_glyph_t *gfx_find_glyph(const gfx_font_t *f, uint32_t cp)
{
	if (cp >= f->first0 && cp <= f->last0) return &f->glyphs[cp - f->first0];
	if (cp >= f->first1 && cp <= f->last1) return &f->glyphs[(f->last0 - f->first0 + 1) + (cp - f->first1)];
	if (cp != '?') return gfx_find_glyph(f, '?');
	return &f->glyphs[0];                                     /* last resort: the first glyph (space) */
}

int gfx_text_width(const char *text, const gfx_font_t *font)
{
	int w = 0;
	while (*text) w += gfx_find_glyph(font, gfx_next_codepoint(&text))->advance;
	return w;
}

/* 16-entry colour table between background and text, indexed by pixel opacity */
static void gfx_make_lut(gfx_color_t *lut, gfx_color_t fg, gfx_color_t bg)
{
	for (int a = 0; a < 16; a++) lut[a] = gfx_blend(bg, fg, (uint8_t)(a * 17));
}

static uint8_t gfx_glyph_alpha(const gfx_font_t *f, const gfx_glyph_t *g, int gx, int row)
{
	int stride = (g->width + 1) / 2;
	const uint8_t *line = f->bitmap + g->offset + row * stride;
	return (gx & 1) ? (line[gx >> 1] & 0x0F) : (line[gx >> 1] >> 4);
}

/* One letter over a background: a window the size of the cell and one burst */
static void gfx_draw_glyph(int x, int y, const gfx_font_t *f, const gfx_glyph_t *g, const gfx_color_t *lut)
{
	int cx = x, cy = y, cw = g->advance, ch = f->height;
	if (cw == 0 || !gfx_clip_rect(&cx, &cy, &cw, &ch)) return;
	gfx_set_window(cx, cy, cx + cw - 1, cy + ch - 1);
	for (int row = cy; row < cy + ch; row++) {
		for (int col = cx; col < cx + cw; col++) {
			int gx = col - x - g->xoff;
			uint8_t a = 0;
			if (g->width && gx >= 0 && gx < g->width) a = gfx_glyph_alpha(f, g, gx, row - y);
			gfx_push(lut[a]);
		}
	}
}

int gfx_text(int x, int y, const char *text, const gfx_font_t *font, gfx_color_t fg, gfx_color_t bg)
{
	gfx_color_t lut[16];
	gfx_make_lut(lut, fg, bg);
	while (*text) {
		const gfx_glyph_t *g = gfx_find_glyph(font, gfx_next_codepoint(&text));
		gfx_draw_glyph(x, y, font, g, lut);
		x += g->advance;
	}
	return x;
}

/* Without a background nothing can be blended: only pixels at >= 50 % opacity are painted */
int gfx_text_t(int x, int y, const char *text, const gfx_font_t *font, gfx_color_t fg)
{
	while (*text) {
		const gfx_glyph_t *g = gfx_find_glyph(font, gfx_next_codepoint(&text));
		for (int row = 0; row < font->height && g->width; row++) {
			int run = -1;
			for (int gx = 0; gx <= g->width; gx++) {
				int on = (gx < g->width) && gfx_glyph_alpha(font, g, gx, row) >= 8;
				if (on && run < 0) run = gx;
				if (!on && run >= 0) { gfx_fill_rect(x + g->xoff + run, y + row, gx - run, 1, fg); run = -1; }
			}
		}
		x += g->advance;
	}
	return x;
}

void gfx_text_box(int x, int y, int w, const char *text, const gfx_font_t *font,
                  gfx_color_t fg, gfx_color_t bg, gfx_align_t align)
{
	int tw = gfx_text_width(text, font);
	int pad = w - tw;
	if (pad < 0) pad = 0;
	int left = (align == GFX_ALIGN_CENTER) ? pad / 2 : (align == GFX_ALIGN_RIGHT ? pad : 0);
	if (left > 0) gfx_fill_rect(x, y, left, font->height, bg);
	gfx_text(x + left, y, text, font, fg, bg);
	int right_start = left + tw;
	if (right_start < w) gfx_fill_rect(x + right_start, y, w - right_start, font->height, bg);
}

char *gfx_int_to_str(int32_t value, char *buf)
{
	char tmp[12];
	int n = 0, k = 0;
	uint32_t u = (value < 0) ? (uint32_t)(-(value + 1)) + 1u : (uint32_t)value;   /* safe for INT32_MIN */
	do { tmp[n++] = (char)('0' + (u % 10)); u /= 10; } while (u);
	if (value < 0) buf[k++] = '-';
	while (n) buf[k++] = tmp[--n];
	buf[k] = '\0';
	return buf;
}
