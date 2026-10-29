/* Library tests against the controller model.
 * Part A: the original 56 checks (geometry, text, images, widgets).
 * Part B: checks on the board self-test patterns. */
#include "sim.h"
#include "gfx_all.h"
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int fails = 0, checks = 0;
#define CHECK(cond, ...) do { checks++; if (!(cond)) { fails++; printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static int px(int x, int y) { return sim_ram[y][x]; }
static int count_color(uint16_t c) { int n = 0; for (int y = 0; y < 240; y++) for (int x = 0; x < 240; x++) n += (sim_ram[y][x] == c); return n; }
static void clear(void) { sim_reset(); sim_fill_ram(0); }

static void part_a(void)
{
	const uint16_t W = 0xFFFF;

	/* 1. exact fill_rect and clipping */
	clear(); gfx_fill_rect(10, 20, 30, 15, W);
	CHECK(count_color(W) == 450, "area=%d", count_color(W));
	CHECK(px(10,20) == W && px(39,34) == W && px(40,34) == 0 && px(9,20) == 0 && px(10,35) == 0, "rect edges");
	clear(); gfx_fill_rect(-10, -10, 20, 20, W);  CHECK(count_color(W) == 100, "clip top-left %d", count_color(W));
	clear(); gfx_fill_rect(230, 230, 50, 50, W);  CHECK(count_color(W) == 100, "clip bottom-right %d", count_color(W));
	clear(); gfx_fill_rect(300, 10, 5, 5, W);     CHECK(count_color(W) == 0, "fully outside");
	clear(); gfx_fill_screen(W);                  CHECK(count_color(W) == 240*240, "full screen");
	CHECK(sim_violations == 0, "violations in rects: %" PRIu64, sim_violations);

	/* 2. lines */
	clear(); gfx_line(10, 10, 100, 40, W);
	CHECK(count_color(W) == 91, "shallow line pixels=%d", count_color(W));
	CHECK(px(10,10) == W && px(100,40) == W, "end points");
	clear(); gfx_line(10, 10, 40, 100, W);
	CHECK(count_color(W) == 91, "steep line pixels=%d", count_color(W));
	clear(); gfx_line(100, 100, 10, 60, W);
	CHECK(px(100,100) == W && px(10,60) == W && count_color(W) == 91, "reversed line %d", count_color(W));
	clear(); gfx_line(5, 5, 5, 50, W); CHECK(count_color(W) == 46, "vertical");
	clear(); gfx_line(5, 5, 50, 5, W); CHECK(count_color(W) == 46, "horizontal");
	clear(); gfx_line(-50, -50, 300, 300, W); CHECK(sim_violations == 0 && count_color(W) == 240, "clipped diagonal %d", count_color(W));

	/* 3. circles: symmetry and area */
	clear(); gfx_fill_circle(120, 120, 50, W);
	int area = count_color(W); double ideal = M_PI * 50.5 * 50.5;
	CHECK(fabs(area - ideal) / ideal < 0.01, "circle area %d vs %.0f", area, ideal);
	int sym = 1;
	for (int y = 0; y < 240; y++) for (int x = 0; x < 240; x++) {
		int rx = 240 - x, ry = 240 - y;
		if (rx < 240 && ry < 240 && px(x,y) != px(rx,ry)) sym = 0;
	}
	CHECK(sym, "circle symmetry");
	clear(); gfx_circle(120, 120, 50, W);
	CHECK(px(70,120) == W && px(170,120) == W && px(120,70) == W && px(120,170) == W && px(120,120) == 0, "1 px outline");
	clear(); gfx_ring(120, 120, 60, 10, W);
	CHECK(px(120,120) == 0 && px(120,65) == W && px(120,75) == 0, "ring");
	clear(); gfx_fill_circle(0, 0, 40, W); CHECK(sim_violations == 0, "circle in the corner");

	/* 4. rounded rectangle */
	clear(); gfx_fill_round_rect(20, 30, 100, 60, 16, W);
	CHECK(px(20,30) == 0 && px(119,30) == 0 && px(20,89) == 0 && px(119,89) == 0, "trimmed corners");
	CHECK(px(60,30) == W && px(20,60) == W && px(119,60) == W && px(60,89) == W, "full sides");
	sym = 1;
	for (int y = 0; y < 60; y++) for (int x = 0; x < 100; x++)
		if (px(20+x,30+y) != px(20+99-x,30+y) || px(20+x,30+y) != px(20+x,30+59-y)) sym = 0;
	CHECK(sym, "rounded rectangle symmetry");
	int full = count_color(W);
	clear(); gfx_round_rect(20, 30, 100, 60, 16, 3, W);
	CHECK(px(60,30) == W && px(60,32) == W && px(60,33) == 0 && px(60,60) == 0 && px(20,60) == W && px(22,60) == W && px(23,60) == 0, "3 px outline");
	CHECK(count_color(W) < full && count_color(W) > 600, "outline area %d", count_color(W));
	clear(); gfx_fill_round_rect(-30, -30, 100, 100, 20, W); CHECK(sim_violations == 0, "clipped rounded rectangle");

	/* 5. polygons */
	clear(); { gfx_point_t r[4] = {{10,10},{50,10},{50,40},{10,40}}; gfx_fill_polygon(r, 4, W); }
	CHECK(count_color(W) == 40*30 && px(10,10) == W && px(49,39) == W && px(50,39) == 0, "polygon = exact rectangle (%d)", count_color(W));
	clear(); gfx_fill_triangle(100, 100, 200, 100, 150, 180, W);
	CHECK(abs(count_color(W) - 4000) < 120, "triangle area %d", count_color(W));
	clear(); { gfx_point_t s[10]; for (int i = 0; i < 10; i++) { double r = (i % 2) ? 20 : 50, a = -M_PI/2 + i*M_PI/5; s[i].x = (int16_t)(120 + r*cos(a)); s[i].y = (int16_t)(120 + r*sin(a)); }
		gfx_fill_polygon(s, 10, W); }
	CHECK(count_color(W) > 1500 && px(120,120) == W, "concave star %d", count_color(W));
	clear(); gfx_line_thick(20, 20, 200, 120, 8, W);
	CHECK(px(110,70) == W && sim_violations == 0, "thick line");

	/* 6. arc against an atan2 reference */
	{
		int cases[][3] = { {0,90,0}, {0,180,0}, {45,300,0}, {270,360,0}, {-90,90,0}, {10,350,0}, {180,200,0}, {0,359,0} };
		for (unsigned k = 0; k < sizeof cases / sizeof cases[0]; k++) {
			int a0 = cases[k][0], a1 = cases[k][1];
			clear(); gfx_arc(120, 120, 80, 14, a0, a1, W);
			int diff = 0, total = 0;
			for (int y = 0; y < 240; y++) for (int x = 0; x < 240; x++) {
				int dx = x - 120, dy = y - 120; double d2 = dx*dx + dy*dy;
				int in_ring = (d2 <= 80*80+80) && (d2 > 66*66+66);
				double ang = atan2(dx, -dy) * 180.0 / M_PI; if (ang < 0) ang += 360;
				double rel = fmod(ang - a0 + 720.0, 360.0);
				int in_arc = in_ring && (a1 - a0 >= 360 || rel <= (a1 - a0));
				if (in_arc) total++;
				if ((px(x,y) == W) != in_arc) diff++;
			}
			CHECK(diff <= total / 50 + 3, "arc %d..%d: %d pixels differ out of %d", a0, a1, diff, total);
		}
	}

	/* 7. text */
	CHECK(gfx_text_width("", &gfx_font_sans_16) == 0, "empty width");
	int w_a = gfx_text_width("Hola", &gfx_font_sans_16), w_b = gfx_text_width("Hola Hola", &gfx_font_sans_16);
	CHECK(w_a > 20 && w_b > w_a * 2 - 1 && w_b < w_a * 2 + 10, "widths %d %d", w_a, w_b);
	CHECK(gfx_text_width("\xC3\xB1" "a", &gfx_font_sans_16) == gfx_text_width("\xC3\xB1", &gfx_font_sans_16) + gfx_text_width("a", &gfx_font_sans_16), "n-tilde in UTF-8");
	{
		/* the n-tilde has its own glyph: it must not render like the "?" fallback */
		int differs = 0;
		clear(); gfx_text(10, 10, "\xC3\xB1", &gfx_font_sans_16, W, 0);
		static uint16_t a[19 * 8];
		for (int y = 0; y < 19; y++) for (int x = 0; x < 8; x++) a[y * 8 + x] = (uint16_t)px(10 + x, 10 + y);
		clear(); gfx_text(10, 10, "?", &gfx_font_sans_16, W, 0);
		for (int y = 0; y < 19; y++) for (int x = 0; x < 8; x++) if (a[y * 8 + x] != (uint16_t)px(10 + x, 10 + y)) differs = 1;
		CHECK(differs, "n-tilde renders like the '?' fallback");
	}
	CHECK(gfx_text_width("\xE2\x82\xAC", &gfx_font_sans_16) == gfx_text_width("?", &gfx_font_sans_16), "euro falls back to ?");
	clear(); gfx_fill_screen(0x0000);
	int end = gfx_text(10, 10, "Hola Mundo", &gfx_font_sans_24, W, 0);
	CHECK(end == 10 + gfx_text_width("Hola Mundo", &gfx_font_sans_24), "final x");
	CHECK(count_color(W) > 100 && sim_violations == 0, "text paints white pixels %d", count_color(W));
	int lit = 0; for (int y = 10; y < 10 + 29; y++) for (int x = 10; x < end; x++) lit += (px(x,y) != 0);
	CHECK(lit > 300, "text is visible");
	clear(); gfx_text(230, 230, "Clipped", &gfx_font_sans_24, W, 0); CHECK(sim_violations == 0, "text at the edge");
	clear(); gfx_text(-30, -10, "Clipped", &gfx_font_sans_24, W, 0); CHECK(sim_violations == 0, "text half outside");
	clear(); gfx_text_t(10, 10, "Transparent", &gfx_font_sans_16, W);
	CHECK(count_color(W) > 80 && sim_violations == 0, "transparent text %d", count_color(W));
	{ char b[16]; CHECK(!strcmp(gfx_int_to_str(0, b), "0") && !strcmp(gfx_int_to_str(-123, b), "-123") && !strcmp(gfx_int_to_str(2147483647, b), "2147483647") && !strcmp(gfx_int_to_str(-2147483647-1, b), "-2147483648"), "int to string"); }
	clear(); gfx_fill_screen(0x1234); gfx_text_box(20, 20, 100, "Hi", &gfx_font_sans_16, W, 0, GFX_ALIGN_RIGHT);
	CHECK(px(20, 20) == 0 && px(119, 20 + 18) == 0 && px(120, 20) == 0x1234 && px(19, 20) == 0x1234, "text_box clears exactly its box");

	/* 8. images and icons */
	clear(); { static uint16_t img[4*3]; for (int i = 0; i < 12; i++) img[i] = (uint16_t)(i + 1); gfx_draw_image(5, 7, 4, 3, img); }
	CHECK(px(5,7) == 1 && px(8,7) == 4 && px(5,9) == 9 && px(8,9) == 12, "image");
	clear(); { static uint16_t img[4*3]; for (int i = 0; i < 12; i++) img[i] = (uint16_t)(i + 1); gfx_draw_image(-2, -1, 4, 3, img); }
	CHECK(px(0,0) == 7 && px(1,0) == 8 && px(0,1) == 11 && sim_violations == 0, "clipped image");
	clear(); { const uint8_t bits[2] = {0xA0, 0x50}; gfx_draw_bitmap1(10, 10, 4, 2, bits, W, 0x0001); }
	CHECK(px(10,10) == W && px(11,10) == 1 && px(12,10) == W && px(13,10) == 1 && px(10,11) == 1 && px(11,11) == W, "1-bit bitmap");
	clear(); gfx_draw_icon(50, 50, &gfx_icon_heart, W, 0);
	CHECK(count_color(W) > 100 && px(50, 50) == 0 && sim_violations == 0, "icon (empty corner, filled centre)");
	clear(); gfx_draw_icon(-10, 220, &gfx_icon_home, W, 0); CHECK(sim_violations == 0, "clipped icon");

	/* 9. widgets: none of them violates the window in any state */
	clear(); gfx_fill_screen(gfx_theme->background);
	gfx_widget_panel(10, 40, 220, 100, "Panel");
	gfx_widget_button(20, 60, 100, 40, "Accept", &gfx_icon_check, GFX_BUTTON_NORMAL);
	gfx_widget_button(130, 60, 90, 40, "No", NULL, GFX_BUTTON_PRESSED);
	gfx_widget_button(20, 110, 90, 30, "Off", NULL, GFX_BUTTON_DISABLED);
	gfx_widget_switch(150, 112, 1); gfx_widget_switch(150, 112, 0);
	gfx_widget_checkbox(20, 150, 1, "Option", gfx_theme->background); gfx_widget_checkbox(20, 180, 0, "Other", gfx_theme->background);
	gfx_widget_slider(100, 150, 120, 0, gfx_theme->background); gfx_widget_slider(100, 150, 120, 50, gfx_theme->background); gfx_widget_slider(100, 150, 120, 100, gfx_theme->background);
	gfx_widget_progress(100, 190, 120, 12, 0, gfx_theme->ok); gfx_widget_progress(100, 190, 120, 12, 3, gfx_theme->ok); gfx_widget_progress(100, 190, 120, 12, 100, gfx_theme->ok);
	gfx_widget_ring(60, 190, 40, 8, 0, gfx_theme->accent, "0%"); gfx_widget_ring(60, 190, 40, 8, 73, gfx_theme->accent, "73%"); gfx_widget_ring(60, 190, 40, 8, 100, gfx_theme->accent, "100%");
	{ int16_t d[8] = {5, 20, 12, 40, 33, 25, 50, 44}; gfx_widget_chart(10, 140, 220, 90, d, 8, 0, 50, gfx_theme->accent); }
	gfx_widget_big_number(0, 40, 240, "23.5", "\xC2\xB0" "C", gfx_theme->text, gfx_theme->background);
	gfx_widget_badge(10, 10, "NEW", gfx_theme->error);
	gfx_widget_list_item(0, 100, 240, 40, &gfx_icon_gear, "Settings", "v1.0", 1);
	gfx_widget_status_bar("Dashboard", 15, 0); gfx_widget_status_bar("Dashboard", 100, 1);
	{ const gfx_image_a4_t *ic[3] = { &gfx_icon_home, &gfx_icon_search, &gfx_icon_gear }; const char *lb[3] = { "Home", "Search", "Settings" };
	  gfx_widget_tabbar(192, 48, ic, lb, 3, 1); }
	gfx_set_theme(&gfx_theme_light); gfx_widget_panel(0, 0, 240, 240, "Light"); gfx_set_theme(&gfx_theme_dark);
	CHECK(sim_violations == 0, "widgets: %" PRIu64 " violations", sim_violations);

	/* 10. cost: bus writes of a full-screen fill */
	clear(); gfx_fill_screen(W);
	printf("fill_screen: %" PRIu64 " commands + %" PRIu64 " data = %" PRIu64 " writes (pixel by pixel would be %d)\n",
	       sim_cmd_count, sim_data_count, sim_bus_writes(), 240*240*12);
}

/* ---------------- Part B: self-test patterns ---------------- */

static void part_b(void)
{
	const uint16_t RED = GFX_RGB(255, 0, 0), GREEN = GFX_RGB(0, 255, 0), BLUE = GFX_RGB(0, 0, 255);
	const uint16_t YELLOW = GFX_RGB(255, 255, 0), WHITE = 0xFFFF;

	CHECK(gfx_selftest_page_count() == 9, "selftest page count %d", gfx_selftest_page_count());

	int all_named = 1;
	for (int p = 0; p < gfx_selftest_page_count(); p++) {
		const char *n = gfx_selftest_page_name(p);
		if (!n || !n[0]) all_named = 0;
	}
	CHECK(all_named, "every selftest page has a name");

	uint64_t viol = 0;
	for (int p = 0; p < gfx_selftest_page_count(); p++) {
		clear(); gfx_selftest_page(p);
		viol += sim_violations;
	}
	CHECK(viol == 0, "selftest pages: %" PRIu64 " violations", viol);

	clear(); gfx_selftest_page(6);
	CHECK(count_color(RED) == 240 * 240, "solid red covers exactly %d pixels", count_color(RED));
	clear(); gfx_selftest_page(7);
	CHECK(count_color(GREEN) == 240 * 240, "solid green covers exactly %d pixels", count_color(GREEN));
	clear(); gfx_selftest_page(8);
	CHECK(count_color(BLUE) == 240 * 240, "solid blue covers exactly %d pixels", count_color(BLUE));

	clear(); gfx_selftest_page(0);
	CHECK(px(15, 100) == RED && px(45, 100) == GREEN && px(75, 100) == BLUE && px(225, 100) == 0 && px(195, 100) == WHITE,
	      "colour bar order R G B ... W K");

	clear(); gfx_selftest_page(1);
	{
		int mono = 1;
		for (int x = 1; x < 240; x++) if (px(x, 200) < px(x - 1, 200)) mono = 0;   /* smooth ramp rises */
		CHECK(mono && px(0, 200) == 0 && px(239, 200) == WHITE, "gray ramp is monotonic from black to white");
	}

	clear(); gfx_selftest_page(2);
	CHECK(px(0, 0) == WHITE && px(239, 0) == WHITE && px(0, 239) == WHITE && px(239, 239) == WHITE && px(120, 0) == WHITE && px(0, 120) == WHITE,
	      "frame sits on the edge pixels");
	CHECK(px(5, 5) == RED && px(234, 5) == GREEN && px(5, 234) == BLUE && px(234, 234) == YELLOW, "corner colours are all different");

	clear(); gfx_selftest_page(3);
	CHECK(px(120, 30) == WHITE && px(30, 120) == WHITE && px(220, 120) == YELLOW && px(120, 20) == YELLOW, "grid cross and circle");

	clear(); gfx_selftest_page(4);
	CHECK(px(0, 0) == WHITE && px(10, 10) == RED && px(20, 20) == WHITE && px(30, 30) == BLUE && px(119, 119) != 0, "nested rectangles");

	clear(); gfx_selftest_page(5);
	{
		int lit = 0;
		for (int y = 0; y < 240; y++) for (int x = 0; x < 240; x++) lit += (px(x, y) != 0);
		CHECK(lit > 2500, "text and icons page is not empty (%d lit pixels)", lit);
	}
}

int main(void)
{
	part_a();
	int checks_a = checks;
	printf("part A: %d checks\n", checks_a);
	part_b();
	printf("part B: %d checks\n", checks - checks_a);
	printf("%d checks, %d failures\n", checks, fails);
	return fails ? 1 : 0;
}
