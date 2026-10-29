/**
 * @file gfx_selftest.h
 * @brief Test patterns for bringing up a real panel.
 *
 * Each pattern makes one class of wiring or configuration fault obvious at a
 * glance. Page them with the same API shape as the demo:
 *
 * @code
 * for (int page = 0; page < gfx_selftest_page_count(); page++) {
 *     gfx_selftest_page(page);
 *     HAL_Delay(2000);
 * }
 * @endcode
 *
 * | Page | Pattern             | What it reveals                              |
 * |------|---------------------|----------------------------------------------|
 * | 0    | Colour bars         | red and blue swapped, wrong bit order        |
 * | 1    | Gray ramp           | wrong colour depth, missing data bits        |
 * | 2    | Frame and corners   | mirrored image, offset, clipped edges        |
 * | 3    | Grid, cross, circle | distorted aspect ratio, skewed rows          |
 * | 4    | Nested rectangles   | wrong window size, wrapped or lost pixels    |
 * | 5    | Text and icons      | font and icon rendering, mirrored text       |
 * | 6    | Solid red           | noise, stuck bits, dead data lines           |
 * | 7    | Solid green         | noise, stuck bits, dead data lines           |
 * | 8    | Solid blue          | noise, stuck bits, dead data lines           |
 */
#ifndef GFX_SELFTEST_H_
#define GFX_SELFTEST_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Number of test pages. */
int gfx_selftest_page_count(void);

/** Short name of a page. */
const char *gfx_selftest_page_name(int page);

/** Draw the complete test pattern number @p page. */
void gfx_selftest_page(int page);

#ifdef __cplusplus
}
#endif

#endif /* GFX_SELFTEST_H_ */
