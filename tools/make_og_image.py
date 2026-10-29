#!/usr/bin/env python3
"""Compose the 1200 x 630 social preview image docs/img/og-image.png.

Needs Pillow. The screenshots come from assets/pages/ (see `make shots`).
Fonts are looked up in common system locations; any TrueType font will do.

    python3 tools/make_og_image.py
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'docs', 'img', 'og-image.png')
W, H = 1200, 630

BOLD = ['C:/Windows/Fonts/segoeuib.ttf', '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf',
        '/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf', '/Library/Fonts/Arial Bold.ttf']
REGULAR = ['C:/Windows/Fonts/segoeui.ttf', '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
           '/usr/share/fonts/dejavu/DejaVuSans.ttf', '/Library/Fonts/Arial.ttf']
MONO = ['C:/Windows/Fonts/consola.ttf', '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',
        '/usr/share/fonts/dejavu/DejaVuSansMono.ttf', '/Library/Fonts/Courier New.ttf']


def font(candidates, size):
    for path in candidates:
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def gradient():
    img = Image.new('RGB', (W, H))
    px = img.load()
    top, bottom = (14, 20, 36), (8, 11, 20)
    for y in range(H):
        t = y / (H - 1)
        row = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3))
        for x in range(W):
            px[x, y] = row
    return img


def main():
    img = gradient()
    d = ImageDraw.Draw(img)

    # screens, 2 x 2 grid on the right
    names = ['demo_3.png', 'demo_4.png', 'demo_1.png', 'demo_2.png']
    size, gap = 250, 16
    x0, y0 = W - 60 - (2 * size + gap), (H - (2 * size + gap)) // 2
    for i, name in enumerate(names):
        shot = Image.open(os.path.join(ROOT, 'assets', 'pages', name)).convert('RGB')
        shot = shot.resize((size, size), Image.NEAREST)
        x = x0 + (i % 2) * (size + gap)
        y = y0 + (i // 2) * (size + gap)
        d.rounded_rectangle((x - 6, y - 6, x + size + 6, y + size + 6), radius=12, fill=(24, 32, 52),
                            outline=(52, 66, 100), width=2)
        img.paste(shot, (x, y))

    # text on the left
    accent = (111, 163, 255)
    d.text((60, 70), 'stm32-st7789-gfx', font=font(MONO, 30), fill=accent)
    title = font(BOLD, 50)
    y = 130
    for line in ('Portable C graphics', 'for ST7789 displays', 'on STM32'):
        d.text((60, y), line, font=title, fill=(240, 244, 252))
        y += 64
    sub = font(REGULAR, 25)
    y += 14
    for line in ('FMC, SPI or 8-bit. Anti-aliased text,', 'icons and widgets. Tested on a PC.'):
        d.text((60, y), line, font=sub, fill=(168, 180, 204))
        y += 34
    tag = font(MONO, 24)
    d.text((60, H - 70), 'C99  |  integer only  |  MIT', font=tag, fill=accent)

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    img.save(OUT, optimize=True)
    print('wrote', os.path.relpath(OUT, ROOT), img.size)


if __name__ == '__main__':
    sys.exit(main())
