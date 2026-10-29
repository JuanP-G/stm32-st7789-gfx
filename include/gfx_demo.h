/**
 * @file gfx_demo.h
 * @brief Demonstration screens that exercise every part of the library.
 *
 * Typical use in main.c, after the controller has been initialised:
 *
 * @code
 * for (;;) {
 *     for (int page = 0; page < gfx_demo_page_count(); page++) {
 *         gfx_demo_page(page);
 *         HAL_Delay(3000);
 *     }
 * }
 * @endcode
 */
#ifndef GFX_DEMO_H_
#define GFX_DEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Number of demo pages. */
int gfx_demo_page_count(void);

/** Name of a page: Shapes, Text, Icons, Panel, Settings, Light theme, Images. */
const char *gfx_demo_page_name(int page);

/** Draw the complete demo screen number @p page. */
void gfx_demo_page(int page);

#ifdef __cplusplus
}
#endif

#endif /* GFX_DEMO_H_ */
