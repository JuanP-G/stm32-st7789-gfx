# Contributing

Thank you for considering a contribution. The library is small on purpose, so
changes are easiest to review when they stay small too.

## Ground rules

- **Portable C99, integers only.** No `float`, no `malloc`, no dependency on a
  vendor HAL. The only link to the hardware is `gfx_port_cmd()` and
  `gfx_port_data()`.
- **Window + burst.** A drawing primitive sets the window once per run of pixels
  and then streams the pixels. Never call `gfx_pixel()` in a loop.
- **Clip first.** Every public drawing call must be safe for coordinates outside
  the screen.
- **English, no emojis.** Comments, identifiers, documentation and commit
  messages are in English. Public functions get a Doxygen comment in the header.
- **Match the surrounding style:** tabs in C, the existing naming (`gfx_` prefix,
  `snake_case`) and comment density.

## Workflow

1. Fork and create a branch.
2. Make the change in `include/` and `src/` (never edit `single_header/lcd_gfx.h`
   or the generated `src/gfx_fonts.c`, `src/gfx_icons.c` by hand).
3. Regenerate the single header: `make header`.
4. Run the whole suite: `make test`.
5. If you changed what a page draws on purpose, regenerate the golden hashes
   with `make golden`, and say why in the pull request. A golden change that is
   not explained is treated as a regression.
6. Open a pull request. Keep commits small, with a clear subject line.

## What `make test` runs

| Step          | What it checks                                                       |
|---------------|----------------------------------------------------------------------|
| `header-check`| the committed single header matches the sources                      |
| `host`        | library and self-test checks, multi-file build                       |
| `single`      | the same checks against the single header, linked from two TUs       |
| `equiv`       | per-page FNV-1a hashes: multi-file = single header = golden file     |
| `bench`       | every primitive costs fewer bus writes than pixel by pixel           |
| `strict`      | `-std=c99 -Wall -Wextra -pedantic -Werror` with gcc                  |
| `clang`       | the same with clang (skipped if clang is not installed)              |
| `arm`         | Cortex-M7 cross-compile (skipped without `arm-none-eabi-gcc`)        |
| `site`        | links, anchors, titles, canonical URLs and JSON-LD of `docs/`        |

CI runs every step, including the ones that may be skipped on your machine.

## Reporting a problem

Open an issue with the panel and board you use, how the port is wired (FMC,
SPI or 8-bit), what you expected and what appeared on screen. The
[troubleshooting table](docs/getting-started.html#troubleshooting) covers the
usual suspects: mirrored image, swapped red and blue, offset and noise.
