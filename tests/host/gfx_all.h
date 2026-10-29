/* Includes the whole library, either as separate headers (multi-file build)
 * or as the single header. The tests are compiled twice from the same sources:
 * once plain, once with -DUSE_SINGLE_HEADER. */
#ifndef GFX_ALL_H_
#define GFX_ALL_H_

#ifdef USE_SINGLE_HEADER
#include "lcd_gfx.h"
#else
#include "gfx_core.h"
#include "gfx_shapes.h"
#include "gfx_text.h"
#include "gfx_image.h"
#include "gfx_widgets.h"
#include "gfx_fonts.h"
#include "gfx_icons.h"
#include "gfx_demo.h"
#include "gfx_selftest.h"
#endif

#endif /* GFX_ALL_H_ */
