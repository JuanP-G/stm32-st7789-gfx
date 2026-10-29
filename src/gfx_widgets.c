/* gfx_widgets.c - interface elements */
#include "gfx_widgets.h"
#include "gfx_shapes.h"
#include "gfx_image.h"
#include "gfx_fonts.h"
#include "gfx_icons.h"

const gfx_theme_t gfx_theme_dark = {
	GFX_HEX(0x10131A), GFX_HEX(0x1B2030), GFX_HEX(0x2A3148), GFX_HEX(0x363F5A),
	GFX_HEX(0xE8ECF5), GFX_HEX(0x8A93A8), GFX_HEX(0x3D8BFF), GFX_HEX(0x2457B8),
	GFX_HEX(0x34C759), GFX_HEX(0xFFB020), GFX_HEX(0xFF4D4F)
};

const gfx_theme_t gfx_theme_light = {
	GFX_HEX(0xF1F3F8), GFX_HEX(0xFFFFFF), GFX_HEX(0xE3E8F2), GFX_HEX(0xCDD4E3),
	GFX_HEX(0x1A1F2B), GFX_HEX(0x6B7487), GFX_HEX(0x2F7BF5), GFX_HEX(0x1D55B8),
	GFX_HEX(0x1FA84A), GFX_HEX(0xE89A00), GFX_HEX(0xE5393B)
};

const gfx_theme_t *gfx_theme = &gfx_theme_dark;

void gfx_set_theme(const gfx_theme_t *theme) { gfx_theme = theme; }

static int gfx_clamp_percent(int p) { return p < 0 ? 0 : (p > 100 ? 100 : p); }

/* ---------- Containers ---------- */

void gfx_widget_panel(int x, int y, int w, int h, const char *title)
{
	gfx_fill_round_rect(x, y, w, h, 10, gfx_theme->surface);
	gfx_round_rect(x, y, w, h, 10, 1, gfx_theme->border);
	if (title) gfx_text(x + 12, y + 8, title, &gfx_font_sans_12, gfx_theme->text_dim, gfx_theme->surface);
}

void gfx_widget_status_bar(const char *title, int battery_percent, int wifi_on)
{
	char num[12], pct[16];
	gfx_color_t bg = gfx_theme->surface;
	gfx_fill_rect(0, 0, GFX_WIDTH, 32, bg);
	gfx_hline(0, 32, GFX_WIDTH, gfx_theme->border);
	gfx_text(10, 7, title, &gfx_font_sans_bold_16, gfx_theme->text, bg);

	int p = gfx_clamp_percent(battery_percent);
	gfx_int_to_str(p, num);
	pct[0] = '\0';
	int n = 0; while (num[n]) { pct[n] = num[n]; n++; }
	pct[n++] = '%'; pct[n] = '\0';
	int tw = gfx_text_width(pct, &gfx_font_sans_12);
	int tx = GFX_WIDTH - 8 - tw;
	gfx_text(tx, 9, pct, &gfx_font_sans_12, gfx_theme->text_dim, bg);
	gfx_color_t batt = (p <= 20) ? gfx_theme->error : gfx_theme->ok;
	gfx_draw_icon(tx - 34, 0, &gfx_icon_battery, batt, bg);
	gfx_draw_icon(tx - 68, 0, &gfx_icon_wifi, wifi_on ? gfx_theme->accent : gfx_theme->border, bg);
}

void gfx_widget_tabbar(int y, int h, const gfx_image_a4_t *const *icons, const char *const *labels,
                       int count, int selected)
{
	gfx_color_t bg = gfx_theme->surface;
	gfx_fill_rect(0, y, GFX_WIDTH, h, bg);
	gfx_hline(0, y, GFX_WIDTH, gfx_theme->border);
	int tab_w = GFX_WIDTH / count;
	for (int i = 0; i < count; i++) {
		gfx_color_t c = (i == selected) ? gfx_theme->accent : gfx_theme->text_dim;
		int cx = i * tab_w + tab_w / 2;
		gfx_draw_icon(cx - 16, y + 3, icons[i], c, bg);
		int tw = gfx_text_width(labels[i], &gfx_font_sans_12);
		gfx_text(cx - tw / 2, y + 36, labels[i], &gfx_font_sans_12, c, bg);
		if (i == selected) gfx_fill_round_rect(cx - 14, y + 1, 28, 3, 1, c);
	}
}

/* ---------- Controls ---------- */

void gfx_widget_button(int x, int y, int w, int h, const char *label, const gfx_image_a4_t *icon,
                       gfx_button_state_t state)
{
	/* Flat body: text and icon blend exactly with the body colour */
	gfx_color_t body, edge, text_color;
	if (state == GFX_BUTTON_PRESSED)       { body = gfx_theme->accent_dark; edge = gfx_darken(body, 70);  text_color = gfx_theme->text; }
	else if (state == GFX_BUTTON_DISABLED) { body = gfx_theme->surface_high; edge = gfx_theme->border;    text_color = gfx_theme->text_dim; }
	else                                   { body = gfx_theme->accent;       edge = gfx_lighten(body, 70); text_color = GFX_WHITE; }

	gfx_fill_round_rect(x, y, w, h, 10, body);
	gfx_round_rect(x, y, w, h, 10, 1, edge);
	const gfx_font_t *font = &gfx_font_sans_bold_16;
	int tw = gfx_text_width(label, font);
	if (icon && (tw + icon->width + 6 > w - 12 || icon->height > h - 4)) icon = NULL;   /* does not fit: drop the icon */
	int content = tw + (icon ? icon->width + 6 : 0);
	int cx = x + (w - content) / 2;
	if (icon) { gfx_draw_icon(cx, y + (h - icon->height) / 2, icon, text_color, body); cx += icon->width + 6; }
	gfx_text(cx, y + (h - font->height) / 2, label, font, text_color, body);
}

void gfx_widget_switch(int x, int y, int on)
{
	gfx_color_t track = on ? gfx_theme->accent : gfx_theme->surface_high;
	gfx_fill_round_rect(x, y, 48, 26, 13, track);
	if (!on) gfx_round_rect(x, y, 48, 26, 13, 1, gfx_theme->border);
	int kx = on ? x + 48 - 13 : x + 13;
	gfx_fill_circle(kx, y + 13, 9, GFX_WHITE);
}

void gfx_widget_checkbox(int x, int y, int checked, const char *label, gfx_color_t bg)
{
	if (checked) {
		gfx_fill_round_rect(x, y, 22, 22, 6, gfx_theme->accent);
		gfx_line_thick(x + 5, y + 11, x + 9, y + 15, 3, GFX_WHITE);
		gfx_line_thick(x + 9, y + 15, x + 17, y + 6, 3, GFX_WHITE);
	} else {
		gfx_fill_round_rect(x, y, 22, 22, 6, bg);
		gfx_round_rect(x, y, 22, 22, 6, 2, gfx_theme->border);
	}
	gfx_text(x + 30, y + 1, label, &gfx_font_sans_16, gfx_theme->text, bg);
}

void gfx_widget_slider(int x, int y, int w, int percent, gfx_color_t bg)
{
	int p = gfx_clamp_percent(percent);
	gfx_fill_rect(x, y, w, 24, bg);
	gfx_fill_round_rect(x + 4, y + 9, w - 8, 6, 3, gfx_theme->surface_high);
	int kx = x + 12 + ((w - 24) * p) / 100;
	if (kx > x + 12) gfx_fill_round_rect(x + 4, y + 9, kx - x - 4, 6, 3, gfx_theme->accent);
	gfx_fill_circle(kx, y + 12, 10, GFX_WHITE);
	gfx_fill_circle(kx, y + 12, 5, gfx_theme->accent);
}

void gfx_widget_badge(int x, int y, const char *text, gfx_color_t color)
{
	int tw = gfx_text_width(text, &gfx_font_sans_12);
	gfx_fill_round_rect(x, y, tw + 16, 20, 10, color);
	gfx_text(x + 8, y + 2, text, &gfx_font_sans_12, GFX_WHITE, color);
}

/* ---------- Indicators ---------- */

void gfx_widget_progress(int x, int y, int w, int h, int percent, gfx_color_t color)
{
	gfx_fill_round_rect(x, y, w, h, h / 2, gfx_theme->surface_high);
	int fill = (w * gfx_clamp_percent(percent)) / 100;
	if (fill >= h) gfx_fill_round_rect(x, y, fill, h, h / 2, color);
	else if (fill > 0) gfx_fill_round_rect(x, y, h, h, h / 2, color);   /* minimum: a circle */
}

void gfx_widget_ring(int cx, int cy, int radius, int thickness, int percent,
                     gfx_color_t color, const char *center_text)
{
	int p = gfx_clamp_percent(percent);
	gfx_fill_circle(cx, cy, radius - thickness, gfx_theme->surface);
	gfx_ring(cx, cy, radius, thickness, gfx_theme->surface_high);
	if (p > 0) gfx_arc(cx, cy, radius, thickness, 0, (p * 360) / 100, color);
	if (center_text) {
		const gfx_font_t *f = (radius >= 40) ? &gfx_font_sans_bold_24 : &gfx_font_sans_bold_16;
		int tw = gfx_text_width(center_text, f);
		gfx_text(cx - tw / 2, cy - f->height / 2, center_text, f, gfx_theme->text, gfx_theme->surface);
	}
}

void gfx_widget_chart(int x, int y, int w, int h, const int16_t *data, int count,
                      int vmin, int vmax, gfx_color_t color)
{
	gfx_color_t bg = gfx_theme->surface;
	gfx_fill_round_rect(x, y, w, h, 8, bg);
	int px = x + 8, py = y + 8, pw = w - 16, ph = h - 16;
	for (int i = 0; i <= 3; i++) gfx_hline(px, py + (ph * i) / 3, pw, gfx_blend(bg, gfx_theme->border, 150));
	if (count < 2 || vmax <= vmin) return;

	int range = vmax - vmin;
	int prev_x = 0, prev_y = 0;
	gfx_color_t fill = gfx_blend(bg, color, 70);
	for (int i = 0; i < count; i++) {
		int v = data[i]; if (v < vmin) v = vmin; if (v > vmax) v = vmax;
		int cxp = px + (pw * i) / (count - 1);
		int cyp = py + ph - (ph * (v - vmin)) / range;
		if (i > 0) {
			for (int col = prev_x; col < cxp; col++) {          /* fill under the curve */
				int yy = prev_y + ((cyp - prev_y) * (col - prev_x)) / (cxp - prev_x);
				gfx_fill_rect(col, yy, 1, py + ph - yy + 1, fill);
			}
			gfx_line_thick(prev_x, prev_y, cxp, cyp, 2, color);
		}
		prev_x = cxp; prev_y = cyp;
	}
	gfx_fill_circle(prev_x, prev_y, 4, color);
	gfx_fill_circle(prev_x, prev_y, 2, GFX_WHITE);
}

void gfx_widget_big_number(int x, int y, int w, const char *number, const char *unit,
                           gfx_color_t color, gfx_color_t bg)
{
	const gfx_font_t *big = &gfx_font_sans_bold_48, *small = &gfx_font_sans_24;
	int nw = gfx_text_width(number, big);
	int uw = unit ? gfx_text_width(unit, small) + 4 : 0;
	int left = x + (w - nw - uw) / 2;
	gfx_fill_rect(x, y, w, big->height, bg);
	gfx_text(left, y, number, big, color, bg);
	if (unit) gfx_text(left + nw + 4, y + big->baseline - small->baseline, unit, small, gfx_theme->text_dim, bg);
}

/* ---------- Lists ---------- */

void gfx_widget_list_item(int x, int y, int w, int h, const gfx_image_a4_t *icon,
                          const char *title, const char *detail, int selected)
{
	gfx_color_t bg = selected ? gfx_theme->surface_high : gfx_theme->surface;
	gfx_fill_rect(x, y, w, h, bg);
	gfx_hline(x, y + h - 1, w, gfx_theme->border);
	int tx = x + 12;
	if (icon) {
		gfx_draw_icon(x + 10, y + (h - icon->height) / 2, icon, selected ? gfx_theme->accent : gfx_theme->text_dim, bg);
		tx = x + 10 + icon->width + 10;
	}
	gfx_text(tx, y + (h - gfx_font_sans_16.height) / 2, title, &gfx_font_sans_16, gfx_theme->text, bg);
	if (detail) {
		int dw = gfx_text_width(detail, &gfx_font_sans_12);
		gfx_text(x + w - 12 - dw, y + (h - gfx_font_sans_12.height) / 2, detail, &gfx_font_sans_12, gfx_theme->text_dim, bg);
	}
}
