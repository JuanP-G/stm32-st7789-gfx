/* gfx_demo.c - one demo screen per area of the library */
#include "gfx_demo.h"
#include "gfx_core.h"
#include "gfx_shapes.h"
#include "gfx_text.h"
#include "gfx_image.h"
#include "gfx_widgets.h"
#include "gfx_fonts.h"
#include "gfx_icons.h"
#include "gfx_sample_scene.h"
#include "gfx_sample_smile.h"

#define GFX_DEMO_BLUE    GFX_HEX(0x3D8BFF)
#define GFX_DEMO_GREEN   GFX_HEX(0x34C759)
#define GFX_DEMO_ORANGE  GFX_HEX(0xFF9500)
#define GFX_DEMO_RED     GFX_HEX(0xFF453A)
#define GFX_DEMO_PURPLE  GFX_HEX(0xBF5AF2)
#define GFX_DEMO_TEAL    GFX_HEX(0x30D5C8)
#define GFX_DEMO_YELLOW  GFX_HEX(0xFFD60A)

#define GFX_DEMO_PAGE_COUNT 7

int gfx_demo_page_count(void) { return GFX_DEMO_PAGE_COUNT; }

const char *gfx_demo_page_name(int page)
{
	static const char *const names[GFX_DEMO_PAGE_COUNT] = { "Shapes", "Text", "Icons", "Panel", "Settings", "Light theme", "Images" };
	return names[page % GFX_DEMO_PAGE_COUNT];
}

/* Header shared by the demo pages */
static void gfx_demo_header(const char *title)
{
	gfx_fill_rect(0, 0, GFX_WIDTH, 28, gfx_theme->surface);
	gfx_hline(0, 28, GFX_WIDTH, gfx_theme->border);
	gfx_text(10, 4, title, &gfx_font_sans_bold_16, gfx_theme->text, gfx_theme->surface);
}

/* ---------- Page 0: shapes ---------- */
static void gfx_demo_page_shapes(void)
{
	gfx_set_theme(&gfx_theme_dark);
	gfx_fill_rect_vgradient(0, 0, GFX_WIDTH, GFX_HEIGHT, GFX_HEX(0x141A33), GFX_HEX(0x070A14));
	gfx_demo_header("Shapes");

	/* 3 x 3 cells of 66 x 56 */
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			gfx_fill_round_rect(10 + col * 76, 36 + row * 66, 66, 56, 8, gfx_theme->surface);

	/* 1) fan of lines */
	for (int i = 0; i < 7; i++)
		gfx_line(14, 40, 72, 40 + 8 * i, gfx_blend(GFX_DEMO_BLUE, GFX_DEMO_PURPLE, (uint8_t)(i * 42)));
	gfx_line_thick(16, 86, 70, 70, 4, GFX_DEMO_YELLOW);

	/* 2) rectangles */
	gfx_fill_rect(94, 42, 30, 20, GFX_DEMO_ORANGE);
	gfx_rect(122, 42, 22, 20, GFX_DEMO_TEAL);
	gfx_rect_thick(94, 66, 50, 22, 4, GFX_DEMO_GREEN);

	/* 3) rounded rectangles */
	gfx_fill_round_rect(170, 42, 30, 20, 8, GFX_DEMO_RED);
	gfx_round_rect(204, 42, 28, 20, 8, 2, GFX_DEMO_PURPLE);
	gfx_fill_round_rect_vgradient(170, 66, 62, 22, 11, GFX_DEMO_BLUE, GFX_HEX(0x1B3A8A));

	/* 4) circles */
	gfx_fill_circle(30, 130, 14, GFX_DEMO_ORANGE);
	gfx_circle(62, 130, 14, GFX_DEMO_TEAL);
	gfx_ring(46, 130, 9, 3, GFX_DEMO_YELLOW);

	/* 5) triangles */
	gfx_fill_triangle(98, 156, 118, 104, 138, 156, GFX_DEMO_GREEN);
	gfx_triangle(104, 150, 118, 112, 132, 150, GFX_BLACK);

	/* 6) star (concave polygon) */
	{
		gfx_point_t star[10];
		for (int i = 0; i < 10; i++) {
			int r = (i % 2) ? 11 : 26;
			int a = -90 + i * 36;
			star[i].x = (int16_t)(201 + (r * gfx_cos_q14(a)) / 16384);
			star[i].y = (int16_t)(136 + (r * gfx_sin_q14(a)) / 16384);
		}
		gfx_fill_polygon(star, 10, GFX_DEMO_YELLOW);
	}

	/* 7) concentric arcs */
	gfx_arc(44, 204, 26, 5, 0, 270, GFX_DEMO_BLUE);
	gfx_arc(44, 204, 18, 5, 0, 180, GFX_DEMO_TEAL);
	gfx_arc(44, 204, 10, 4, 0, 90, GFX_DEMO_ORANGE);

	/* 8) thick zigzag line */
	gfx_line_thick(90, 220, 106, 178, 6, GFX_DEMO_RED);
	gfx_line_thick(106, 178, 122, 220, 6, GFX_DEMO_ORANGE);
	gfx_line_thick(122, 220, 138, 178, 6, GFX_DEMO_YELLOW);

	/* 9) hexagon */
	{
		gfx_point_t hex[6];
		for (int i = 0; i < 6; i++) {
			hex[i].x = (int16_t)(201 + (24 * gfx_cos_q14(i * 60)) / 16384);
			hex[i].y = (int16_t)(204 + (24 * gfx_sin_q14(i * 60)) / 16384);
		}
		gfx_fill_polygon(hex, 6, GFX_DEMO_PURPLE);
		gfx_polygon(hex, 6, gfx_lighten(GFX_DEMO_PURPLE, 120));
	}
}

/* ---------- Page 1: text ---------- */
static void gfx_demo_page_text(void)
{
	gfx_set_theme(&gfx_theme_dark);
	gfx_color_t bg = gfx_theme->background;
	gfx_fill_screen(bg);
	gfx_demo_header("Text");

	gfx_text(10, 34, "Small 12 px: Café, señor, naïve", &gfx_font_sans_12, gfx_theme->text_dim, bg);
	gfx_text(10, 52, "Regular 16: Résumé", &gfx_font_sans_16, gfx_theme->text, bg);
	gfx_text(10, 74, "Large 24: Ñandú", &gfx_font_sans_24, GFX_DEMO_TEAL, bg);
	gfx_text(10, 106, "Bold 16: Year 2026", &gfx_font_sans_bold_16, GFX_DEMO_YELLOW, bg);
	gfx_text(10, 128, "Bold 24", &gfx_font_sans_bold_24, GFX_DEMO_ORANGE, bg);
	gfx_text(10, 158, "12:45", &gfx_font_sans_bold_48, GFX_WHITE, bg);
	gfx_text(172, 176, "25 °C", &gfx_font_sans_24, GFX_DEMO_BLUE, bg);

	/* Alignment inside a box */
	gfx_color_t box = gfx_theme->surface_high;
	gfx_fill_round_rect(8, 214, 224, 22, 6, box);
	gfx_text_box(12, 216, 66, "Left", &gfx_font_sans_16, gfx_theme->text, box, GFX_ALIGN_LEFT);
	gfx_text_box(86, 216, 68, "Center", &gfx_font_sans_16, gfx_theme->text, box, GFX_ALIGN_CENTER);
	gfx_text_box(162, 216, 66, "Right", &gfx_font_sans_16, gfx_theme->text, box, GFX_ALIGN_RIGHT);
}

/* ---------- Page 2: icons ---------- */
static void gfx_demo_page_icons(void)
{
	gfx_set_theme(&gfx_theme_dark);
	gfx_color_t bg = gfx_theme->background;
	gfx_fill_screen(bg);
	gfx_demo_header("Icons (32 x 32)");

	static const gfx_image_a4_t *const icons[] = {
		&gfx_icon_home, &gfx_icon_gear, &gfx_icon_wifi, &gfx_icon_battery, &gfx_icon_bell, &gfx_icon_user,
		&gfx_icon_lock, &gfx_icon_heart, &gfx_icon_star, &gfx_icon_sun, &gfx_icon_moon, &gfx_icon_check,
		&gfx_icon_close, &gfx_icon_plus, &gfx_icon_minus, &gfx_icon_arrow_left, &gfx_icon_arrow_right, &gfx_icon_arrow_up,
		&gfx_icon_arrow_down, &gfx_icon_play, &gfx_icon_pause, &gfx_icon_search, &gfx_icon_mail, &gfx_icon_clock,
		&gfx_icon_bolt, &gfx_icon_power, &gfx_icon_thermometer, &gfx_icon_drop, &gfx_icon_menu, &gfx_icon_info,
		&gfx_icon_warning, &gfx_icon_pin
	};
	static const gfx_color_t colors[6] = { GFX_DEMO_BLUE, GFX_DEMO_TEAL, GFX_DEMO_GREEN, GFX_DEMO_YELLOW, GFX_DEMO_ORANGE, GFX_DEMO_RED };
	for (int i = 0; i < 32; i++) {
		int col = i % 6, row = i / 6;
		gfx_draw_icon(9 + col * 38, 36 + row * 34, icons[i], colors[(col + row) % 6], bg);
	}
}

/* ---------- Pages 3 and 5: dashboard (dark / light) ---------- */
static void gfx_demo_page_dashboard(const gfx_theme_t *theme, int cpu, int battery)
{
	gfx_set_theme(theme);
	gfx_fill_screen(theme->background);
	gfx_widget_status_bar("Dashboard", battery, 1);

	gfx_widget_panel(8, 38, 108, 92, "CPU");
	gfx_widget_ring(62, 88, 34, 8, cpu, theme->accent, "73%");

	gfx_widget_panel(124, 38, 108, 92, "Temperature");
	gfx_widget_big_number(124, 58, 108, "23", "°C", theme->text, theme->surface);
	gfx_widget_badge(150, 108, "Normal", theme->ok);

	static const int16_t samples[] = { 12, 18, 15, 28, 22, 35, 30, 41, 33, 46, 38, 44 };
	gfx_widget_chart(8, 136, 224, 48, samples, 12, 0, 50, theme->accent);

	static const gfx_image_a4_t *const tab_icons[] = { &gfx_icon_home, &gfx_icon_search, &gfx_icon_bell, &gfx_icon_gear };
	static const char *const tab_labels[] = { "Home", "Search", "Alerts", "Settings" };
	gfx_widget_tabbar(188, 52, tab_icons, tab_labels, 4, 0);
}

/* ---------- Page 4: settings ---------- */
static void gfx_demo_page_settings(void)
{
	gfx_set_theme(&gfx_theme_dark);
	gfx_fill_screen(gfx_theme->background);
	gfx_widget_status_bar("Settings", 62, 1);

	gfx_widget_list_item(0, 38, GFX_WIDTH, 34, &gfx_icon_wifi, "Wi-Fi", "Home_5G", 1);
	gfx_widget_list_item(0, 72, GFX_WIDTH, 34, &gfx_icon_bell, "Alerts", NULL, 0);
	gfx_widget_switch(184, 76, 1);
	gfx_widget_list_item(0, 106, GFX_WIDTH, 34, &gfx_icon_sun, "Brightness", NULL, 0);
	gfx_widget_slider(112, 111, 118, 65, gfx_theme->surface);
	gfx_widget_list_item(0, 140, GFX_WIDTH, 34, &gfx_icon_moon, "Night mode", NULL, 0);
	gfx_widget_switch(184, 144, 0);

	gfx_widget_checkbox(14, 178, 1, "Save", gfx_theme->background);
	gfx_widget_checkbox(124, 178, 0, "Notify", gfx_theme->background);
	gfx_widget_button(8, 204, 88, 32, "Cancel", NULL, GFX_BUTTON_PRESSED);
	gfx_widget_button(104, 204, 128, 32, "Save", &gfx_icon_check, GFX_BUTTON_NORMAL);
}

/* ---------- Page 6: images ---------- */
static void gfx_demo_page_images(void)
{
	gfx_set_theme(&gfx_theme_dark);
	gfx_color_t bg = gfx_theme->background;
	gfx_fill_screen(bg);
	gfx_demo_header("Images");

	gfx_text(10, 36, "RGB565 (120 x 80)", &gfx_font_sans_12, gfx_theme->text_dim, bg);
	gfx_draw_image(10, 54, GFX_SAMPLE_SCENE_W, GFX_SAMPLE_SCENE_H, gfx_sample_scene);
	gfx_round_rect(8, 52, GFX_SAMPLE_SCENE_W + 4, GFX_SAMPLE_SCENE_H + 4, 2, 2, gfx_theme->border);

	gfx_text(150, 36, "1 bit", &gfx_font_sans_12, gfx_theme->text_dim, bg);
	gfx_draw_bitmap1(150, 54, GFX_SAMPLE_SMILE_W, GFX_SAMPLE_SMILE_H, gfx_sample_smile, GFX_DEMO_YELLOW, gfx_theme->surface);
	gfx_draw_bitmap1_t(190, 54, GFX_SAMPLE_SMILE_W, GFX_SAMPLE_SMILE_H, gfx_sample_smile, GFX_DEMO_GREEN);

	gfx_text(10, 142, "Anti-aliased icons", &gfx_font_sans_12, gfx_theme->text_dim, bg);
	gfx_fill_round_rect(8, 160, 224, 70, 10, gfx_theme->surface);
	gfx_draw_icon(24, 170, &gfx_icon_heart, GFX_DEMO_RED, gfx_theme->surface);
	gfx_draw_icon(70, 170, &gfx_icon_star, GFX_DEMO_YELLOW, gfx_theme->surface);
	gfx_draw_icon(116, 170, &gfx_icon_bolt, GFX_DEMO_ORANGE, gfx_theme->surface);
	gfx_draw_icon(162, 170, &gfx_icon_drop, GFX_DEMO_BLUE, gfx_theme->surface);
	gfx_text(24, 208, "Love", &gfx_font_sans_12, gfx_theme->text_dim, gfx_theme->surface);
	gfx_text(70, 208, "Star", &gfx_font_sans_12, gfx_theme->text_dim, gfx_theme->surface);
	gfx_text(116, 208, "Fast", &gfx_font_sans_12, gfx_theme->text_dim, gfx_theme->surface);
	gfx_text(162, 208, "Water", &gfx_font_sans_12, gfx_theme->text_dim, gfx_theme->surface);
}

void gfx_demo_page(int page)
{
	switch (page % GFX_DEMO_PAGE_COUNT) {
	case 0: gfx_demo_page_shapes(); break;
	case 1: gfx_demo_page_text(); break;
	case 2: gfx_demo_page_icons(); break;
	case 3: gfx_demo_page_dashboard(&gfx_theme_dark, 73, 82); break;
	case 4: gfx_demo_page_settings(); break;
	case 5: gfx_demo_page_dashboard(&gfx_theme_light, 48, 15); break;
	default: gfx_demo_page_images(); break;
	}
}
