/* Bus-write benchmark: how many writes does each primitive cost compared with
 * drawing the same pixels one by one?
 *
 * A pixel-by-pixel driver sends CASET (1 command + 4 parameters), RASET
 * (1 command + 4 parameters) and RAMWR (1 command + 1 pixel) for every pixel:
 * 12 bus writes per pixel. The window + burst scheme pays that set-up once
 * per run of pixels.
 *
 *   bench           print a table
 *   bench --csv     print comma separated values (used to draw the SVG chart)
 *
 * The program exits with 1 if any primitive that paints more than one pixel
 * costs as much as, or more than, the pixel-by-pixel approach. */
#include "sim.h"
#include "gfx_all.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define NAIVE_WRITES_PER_PIXEL 12u

typedef struct { const char *name; const char *what; void (*run)(void); } bench_t;

static void b_pixel(void)      { gfx_pixel(120, 120, 0xFFFF); }
static void b_hline(void)      { gfx_hline(10, 100, 200, 0xFFFF); }
static void b_rect(void)       { gfx_fill_rect(20, 20, 100, 60, 0xFFFF); }
static void b_screen(void)     { gfx_fill_screen(0xFFFF); }
static void b_line(void)       { gfx_line(10, 10, 230, 90, 0xFFFF); }
static void b_circle(void)     { gfx_fill_circle(120, 120, 60, 0xFFFF); }
static void b_rrect(void)      { gfx_fill_round_rect(20, 20, 200, 80, 16, 0xFFFF); }
static void b_triangle(void)   { gfx_fill_triangle(20, 200, 220, 200, 120, 40, 0xFFFF); }
static void b_arc(void)        { gfx_arc(120, 120, 80, 14, 0, 270, 0xFFFF); }
static void b_text(void)       { gfx_text(10, 10, "Hello, world", &gfx_font_sans_16, 0xFFFF, 0); }
static void b_icon(void)       { gfx_draw_icon(100, 100, &gfx_icon_heart, 0xFFFF, 0); }
static void b_image(void)      { static uint16_t img[120 * 80]; gfx_draw_image(10, 10, 120, 80, img); }
static void b_button(void)     { gfx_widget_button(20, 20, 128, 32, "Save", &gfx_icon_check, GFX_BUTTON_NORMAL); }
static void b_ring(void)       { gfx_widget_ring(62, 88, 34, 8, 73, 0xFFFF, "73%"); }
static void b_demo0(void)      { gfx_demo_page(3); }

static const bench_t benches[] = {
	{ "pixel",           "gfx_pixel",                           b_pixel },
	{ "hline_200",       "gfx_hline, 200 px",                   b_hline },
	{ "rect_100x60",     "gfx_fill_rect, 100 x 60",             b_rect },
	{ "screen",          "gfx_fill_screen, 240 x 240",          b_screen },
	{ "line",            "gfx_line, 220 x 80",                  b_line },
	{ "circle_r60",      "gfx_fill_circle, radius 60",          b_circle },
	{ "round_rect",      "gfx_fill_round_rect, 200 x 80",       b_rrect },
	{ "triangle",        "gfx_fill_triangle, 200 x 160",        b_triangle },
	{ "arc_270",         "gfx_arc, radius 80, 270 degrees",     b_arc },
	{ "text_12_chars",   "gfx_text, 12 characters, 16 px font", b_text },
	{ "icon_32",         "gfx_draw_icon, 32 x 32",              b_icon },
	{ "image_120x80",    "gfx_draw_image, 120 x 80",            b_image },
	{ "button",          "gfx_widget_button",                   b_button },
	{ "ring_gauge",      "gfx_widget_ring",                     b_ring },
	{ "dashboard_page",  "demo page 3 (dashboard), whole page", b_demo0 },
};

int main(int argc, char **argv)
{
	int csv = (argc > 1 && !strcmp(argv[1], "--csv"));
	int bad = 0;
	if (csv) printf("name,description,pixels,writes,naive_writes,ratio\n");
	else {
		printf("%-16s %-38s %9s %10s %12s %8s\n", "name", "primitive", "pixels", "writes", "pixel-by-px", "ratio");
		printf("%-16s %-38s %9s %10s %12s %8s\n", "----", "---------", "------", "------", "-----------", "-----");
	}
	for (unsigned i = 0; i < sizeof benches / sizeof benches[0]; i++) {
		sim_reset(); sim_fill_ram(0);
		benches[i].run();
		uint64_t px = sim_pixel_writes, w = sim_bus_writes(), naive = px * NAIVE_WRITES_PER_PIXEL;
		double ratio = w ? (double)naive / (double)w : 0.0;
		if (px > 1 && w >= naive) bad = 1;
		if (sim_violations) bad = 1;
		if (csv) printf("%s,\"%s\",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.2f\n", benches[i].name, benches[i].what, px, w, naive, ratio);
		else printf("%-16s %-38s %9" PRIu64 " %10" PRIu64 " %12" PRIu64 " %7.1fx\n", benches[i].name, benches[i].what, px, w, naive, ratio);
	}
	if (!csv) printf("\n%s\n", bad ? "FAIL: a primitive costs as much as pixel by pixel" : "OK: every multi-pixel primitive beats pixel by pixel");
	return bad;
}
