/* gfx_selftest.c - test patterns for a real panel */
#include "gfx_selftest.h"
#include "gfx_core.h"
#include "gfx_shapes.h"
#include "gfx_text.h"
#include "gfx_image.h"
#include "gfx_fonts.h"
#include "gfx_icons.h"

#define GFX_SELFTEST_PAGE_COUNT 9

#define GFX_ST_RED      GFX_RGB(255, 0, 0)
#define GFX_ST_GREEN    GFX_RGB(0, 255, 0)
#define GFX_ST_BLUE     GFX_RGB(0, 0, 255)
#define GFX_ST_YELLOW   GFX_RGB(255, 255, 0)
#define GFX_ST_CYAN     GFX_RGB(0, 255, 255)
#define GFX_ST_MAGENTA  GFX_RGB(255, 0, 255)

int gfx_selftest_page_count(void) { return GFX_SELFTEST_PAGE_COUNT; }

const char *gfx_selftest_page_name(int page)
{
	static const char *const names[GFX_SELFTEST_PAGE_COUNT] = {
		"Color bars", "Gray ramp", "Frame and corners", "Grid, cross, circle",
		"Nested rectangles", "Text and icons", "Solid red", "Solid green", "Solid blue"
	};
	return names[page % GFX_SELFTEST_PAGE_COUNT];
}

/* Eight vertical bars: R G B C M Y W K. Red and blue swapped is obvious. */
static void gfx_selftest_bars(void)
{
	static const gfx_color_t bar[8] = {
		GFX_ST_RED, GFX_ST_GREEN, GFX_ST_BLUE, GFX_ST_CYAN,
		GFX_ST_MAGENTA, GFX_ST_YELLOW, GFX_WHITE, GFX_BLACK
	};
	static const char *const label[8] = { "R", "G", "B", "C", "M", "Y", "W", "K" };
	for (int i = 0; i < 8; i++) {
		gfx_fill_rect(i * 30, 0, 30, 208, bar[i]);
		gfx_fill_rect(i * 30, 208, 30, 32, GFX_BLACK);
		gfx_text(i * 30 + 10, 214, label[i], &gfx_font_sans_bold_16, GFX_WHITE, GFX_BLACK);
	}
}

/* 16 flat gray steps on top, a smooth 240-column ramp below. */
static void gfx_selftest_gray(void)
{
	for (int i = 0; i < 16; i++) {
		int v = i * 17;
		gfx_fill_rect(i * 15, 0, 15, 100, GFX_RGB(v, v, v));
	}
	gfx_fill_rect(0, 100, GFX_WIDTH, 40, GFX_BLACK);
	gfx_text_box(0, 111, GFX_WIDTH, "16 steps above, smooth ramp below", &gfx_font_sans_12,
	             GFX_WHITE, GFX_BLACK, GFX_ALIGN_CENTER);
	gfx_fill_rect_hgradient(0, 140, GFX_WIDTH, 100, GFX_BLACK, GFX_WHITE);
}

/* A 1 px frame on the very edge pixels and a different colour in each corner. */
static void gfx_selftest_frame(void)
{
	gfx_fill_screen(GFX_BLACK);
	gfx_fill_rect(1, 1, 20, 20, GFX_ST_RED);                              /* top-left     */
	gfx_fill_rect(GFX_WIDTH - 21, 1, 20, 20, GFX_ST_GREEN);               /* top-right    */
	gfx_fill_rect(1, GFX_HEIGHT - 21, 20, 20, GFX_ST_BLUE);               /* bottom-left  */
	gfx_fill_rect(GFX_WIDTH - 21, GFX_HEIGHT - 21, 20, 20, GFX_ST_YELLOW);/* bottom-right */
	gfx_text_box(0, 90, GFX_WIDTH, "Frame and corners", &gfx_font_sans_bold_16,
	             GFX_WHITE, GFX_BLACK, GFX_ALIGN_CENTER);
	gfx_text_box(0, 116, GFX_WIDTH, "TL red   TR green", &gfx_font_sans_12,
	             GFX_WHITE, GFX_BLACK, GFX_ALIGN_CENTER);
	gfx_text_box(0, 134, GFX_WIDTH, "BL blue   BR yellow", &gfx_font_sans_12,
	             GFX_WHITE, GFX_BLACK, GFX_ALIGN_CENTER);
	gfx_rect(0, 0, GFX_WIDTH, GFX_HEIGHT, GFX_WHITE);                     /* drawn last: text boxes span the width */
}

/* 20 px grid, a cross through the centre and a circle: aspect ratio check. */
static void gfx_selftest_grid(void)
{
	const gfx_color_t grid = GFX_RGB(64, 64, 64);
	gfx_fill_screen(GFX_BLACK);
	for (int i = 0; i <= GFX_WIDTH; i += 20) {
		gfx_vline(i == GFX_WIDTH ? GFX_WIDTH - 1 : i, 0, GFX_HEIGHT, grid);
		gfx_hline(0, i == GFX_HEIGHT ? GFX_HEIGHT - 1 : i, GFX_WIDTH, grid);
	}
	gfx_vline(120, 0, GFX_HEIGHT, GFX_WHITE);
	gfx_hline(0, 120, GFX_WIDTH, GFX_WHITE);
	gfx_circle(120, 120, 100, GFX_ST_YELLOW);
	gfx_circle(120, 120, 50, GFX_ST_CYAN);
	gfx_line(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1, GFX_ST_MAGENTA);
	gfx_line(GFX_WIDTH - 1, 0, 0, GFX_HEIGHT - 1, GFX_ST_MAGENTA);
}

/* Twelve filled rectangles inset by 10 px each, alternating colours. */
static void gfx_selftest_nested(void)
{
	static const gfx_color_t col[4] = { GFX_WHITE, GFX_ST_RED, GFX_WHITE, GFX_ST_BLUE };
	for (int i = 0; i < 12; i++)
		gfx_fill_rect(i * 10, i * 10, GFX_WIDTH - 20 * i, GFX_HEIGHT - 20 * i, col[i % 4]);
}

/* Every font on a black background and a row of icons. */
static void gfx_selftest_text(void)
{
	static const gfx_image_a4_t *const icons[6] = {
		&gfx_icon_home, &gfx_icon_gear, &gfx_icon_wifi, &gfx_icon_battery, &gfx_icon_bell, &gfx_icon_user
	};
	gfx_fill_screen(GFX_BLACK);
	gfx_text(8, 6,   "Sans 12: The quick brown fox", &gfx_font_sans_12, GFX_WHITE, GFX_BLACK);
	gfx_text(8, 26,  "Sans 16: The quick brown fox", &gfx_font_sans_16, GFX_WHITE, GFX_BLACK);
	gfx_text(8, 50,  "Sans 24: Quick fox", &gfx_font_sans_24, GFX_WHITE, GFX_BLACK);
	gfx_text(8, 84,  "Bold 16: The quick brown fox", &gfx_font_sans_bold_16, GFX_ST_YELLOW, GFX_BLACK);
	gfx_text(8, 108, "Bold 24: Quick fox", &gfx_font_sans_bold_24, GFX_ST_CYAN, GFX_BLACK);
	gfx_text(8, 142, "12:45", &gfx_font_sans_bold_48, GFX_WHITE, GFX_BLACK);
	for (int i = 0; i < 6; i++)
		gfx_draw_icon(8 + i * 38, 202, icons[i], GFX_WHITE, GFX_BLACK);
}

static void gfx_selftest_solid(gfx_color_t color)
{
	gfx_fill_screen(color);
}

void gfx_selftest_page(int page)
{
	switch (page % GFX_SELFTEST_PAGE_COUNT) {
	case 0: gfx_selftest_bars(); break;
	case 1: gfx_selftest_gray(); break;
	case 2: gfx_selftest_frame(); break;
	case 3: gfx_selftest_grid(); break;
	case 4: gfx_selftest_nested(); break;
	case 5: gfx_selftest_text(); break;
	case 6: gfx_selftest_solid(GFX_ST_RED); break;
	case 7: gfx_selftest_solid(GFX_ST_GREEN); break;
	default: gfx_selftest_solid(GFX_ST_BLUE); break;
	}
}
