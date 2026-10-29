/*
 * Quick start on the PC: draw a first screen against the controller model.
 *
 * This is the code from the "Draw the first screen" guide. It builds with the
 * host model instead of real hardware, counts the bus writes and checks that
 * nothing was written outside the address window.
 *
 *     make examples
 *     build/host_quickstart [out.ppm]
 */
#include <stdio.h>

#include "gfx_core.h"
#include "gfx_fonts.h"
#include "gfx_shapes.h"
#include "gfx_text.h"
#include "gfx_widgets.h"
#include "sim.h"

static void ui_demo(void)
{
    gfx_set_theme(&gfx_theme_dark);
    gfx_fill_screen(gfx_theme->background);
    gfx_widget_status_bar("Hello", 82, 1);
    gfx_widget_ring(62, 110, 40, 9, 73, gfx_theme->accent, "73%");
    gfx_text(120, 100, "CPU load", &gfx_font_sans_16, gfx_theme->text, gfx_theme->background);
    gfx_text(10, 190, "Hello, ST7789", &gfx_font_sans_bold_24, GFX_WHITE, gfx_theme->background);
}

int main(int argc, char **argv)
{
    sim_reset();
    ui_demo();

    printf("bus writes : %llu (%llu commands, %llu data words)\n",
           (unsigned long long)sim_bus_writes(), (unsigned long long)sim_cmd_count,
           (unsigned long long)sim_data_count);
    printf("violations : %llu\n", (unsigned long long)sim_violations);
    if (argc > 1) {
        sim_save_ppm(argv[1]);
        printf("saved      : %s\n", argv[1]);
    }
    return sim_violations == 0 ? 0 : 1;
}
