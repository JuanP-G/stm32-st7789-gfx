/**
 * @file gfx_widgets.h
 * @brief Ready-to-use interface elements.
 *
 * Widgets are plain drawing functions: they keep no state. You pass the value
 * and, when it changes, you call the function again. All of them use the
 * colours of the active theme (see gfx_set_theme). They are meant to be
 * driven by your own touch or button handling code.
 */
#ifndef GFX_WIDGETS_H_
#define GFX_WIDGETS_H_

#include "gfx_core.h"
#include "gfx_text.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Colour theme ---- */

/** A palette used by every widget. */
typedef struct {
	gfx_color_t background;     /**< screen background */
	gfx_color_t surface;        /**< cards and panels */
	gfx_color_t surface_high;   /**< tracks, switched-off controls, selected row */
	gfx_color_t border;
	gfx_color_t text;
	gfx_color_t text_dim;       /**< secondary text */
	gfx_color_t accent;
	gfx_color_t accent_dark;
	gfx_color_t ok, warn, error;
} gfx_theme_t;

extern const gfx_theme_t gfx_theme_dark;
extern const gfx_theme_t gfx_theme_light;

/** Active theme (dark by default). */
extern const gfx_theme_t *gfx_theme;

/** Select the active theme. */
void gfx_set_theme(const gfx_theme_t *theme);

/** Visual state of a button. */
typedef enum { GFX_BUTTON_NORMAL = 0, GFX_BUTTON_PRESSED = 1, GFX_BUTTON_DISABLED = 2 } gfx_button_state_t;

/* ---- Containers and headers ---- */

/** Card with an optional title (@p title may be NULL). */
void gfx_widget_panel(int x, int y, int w, int h, const char *title);

/** 32 px top bar with a title, battery level and Wi-Fi indicator. */
void gfx_widget_status_bar(const char *title, int battery_percent, int wifi_on);

/** Bottom tab bar with one icon and one label per tab. */
void gfx_widget_tabbar(int y, int h, const gfx_image_a4_t *const *icons, const char *const *labels,
                       int count, int selected);

/* ---- Controls ---- */

/** Button with a label and an optional icon (@p icon may be NULL). */
void gfx_widget_button(int x, int y, int w, int h, const char *label, const gfx_image_a4_t *icon,
                       gfx_button_state_t state);

/** On/off switch, 48 x 26 px. */
void gfx_widget_switch(int x, int y, int on);

/** Check box with a label; @p bg is the colour behind it. */
void gfx_widget_checkbox(int x, int y, int checked, const char *label, gfx_color_t bg);

/** Slider, 24 px tall; @p percent is 0..100 and @p bg the colour behind it. */
void gfx_widget_slider(int x, int y, int w, int percent, gfx_color_t bg);

/** Small rounded label. */
void gfx_widget_badge(int x, int y, const char *text, gfx_color_t color);

/* ---- Indicators ---- */

/** Horizontal progress bar. */
void gfx_widget_progress(int x, int y, int w, int h, int percent, gfx_color_t color);

/** Ring-shaped gauge with optional centre text (@p center_text may be NULL). */
void gfx_widget_ring(int cx, int cy, int radius, int thickness, int percent,
                     gfx_color_t color, const char *center_text);

/** Line chart with a filled area under the curve. */
void gfx_widget_chart(int x, int y, int w, int h, const int16_t *data, int count,
                      int vmin, int vmax, gfx_color_t color);

/** Large number with a unit (@p unit may be NULL), centred in a box of width @p w. */
void gfx_widget_big_number(int x, int y, int w, const char *number, const char *unit,
                           gfx_color_t color, gfx_color_t bg);

/* ---- Lists ---- */

/** One list row with an optional icon, a title and an optional right-aligned detail. */
void gfx_widget_list_item(int x, int y, int w, int h, const gfx_image_a4_t *icon,
                          const char *title, const char *detail, int selected);

#ifdef __cplusplus
}
#endif

#endif /* GFX_WIDGETS_H_ */
