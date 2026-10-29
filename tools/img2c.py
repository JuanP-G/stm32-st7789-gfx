#!/usr/bin/env python3
"""Convert an image (PNG, JPG...) into a C array for gfx_draw_image / gfx_draw_bitmap1.

  python3 img2c.py photo.png my_photo               -> my_photo.c / my_photo.h  (RGB565, uint16_t)
  python3 img2c.py logo.png my_logo --mono          -> 1-bit bitmap (MSB on the left)
  python3 img2c.py photo.png my_photo --size 120x80 -> scale before converting

The array lives in flash (const). Use it from C:
  #include "my_photo.h"
  gfx_draw_image(10, 10, MY_PHOTO_W, MY_PHOTO_H, my_photo);
Requires: pip install pillow"""
import sys, argparse
from PIL import Image

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('image'); ap.add_argument('name')
    ap.add_argument('--mono', action='store_true', help='1 bit per pixel')
    ap.add_argument('--size', help='WIDTHxHEIGHT, for example 120x80')
    a = ap.parse_args()
    im = Image.open(a.image).convert('RGB')
    if a.size:
        w, h = map(int, a.size.lower().split('x')); im = im.resize((w, h), Image.LANCZOS)
    w, h = im.size
    name, NAME = a.name, a.name.upper()
    lines = []
    if a.mono:
        g = im.convert('L'); stride = (w + 7)//8
        data = []
        for y in range(h):
            for bx in range(stride):
                b = 0
                for k in range(8):
                    x = bx*8 + k
                    if x < w and g.getpixel((x, y)) >= 128: b |= 0x80 >> k
                data.append(b)
        ctype, fmt, n = 'uint8_t', '0x%02X', len(data)
    else:
        data = []
        for y in range(h):
            for x in range(w):
                r, g_, b = im.getpixel((x, y))
                data.append(((r & 0xF8) << 8) | ((g_ & 0xFC) << 3) | (b >> 3))
        ctype, fmt, n = 'uint16_t', '0x%04X', len(data)
    per = 12 if not a.mono else 16
    lines.append('/* Generated with tools/img2c.py (%s, %dx%d) */' % ('1 bit' if a.mono else 'RGB565', w, h))
    lines.append('#include <stdint.h>')
    lines.append('const %s %s[%d] = {' % (ctype, name, n))
    for i in range(0, n, per): lines.append('  ' + ','.join(fmt % v for v in data[i:i+per]) + ',')
    lines.append('};')
    open(name + '.c', 'w', encoding='utf-8', newline='\n').write('\n'.join(lines) + '\n')
    open(name + '.h', 'w', encoding='utf-8', newline='\n').write('#ifndef %s_H_\n#define %s_H_\n#include <stdint.h>\n#define %s_W %d\n#define %s_H %d\nextern const %s %s[%d];\n#endif\n'
                                 % (NAME, NAME, NAME, w, NAME, h, ctype, name, n))
    print('%s.c and %s.h written (%d x %d)' % (name, name, w, h))

if __name__ == '__main__':
    main()
