#!/usr/bin/env python3
"""Convert the PPM captures written by tests/host/pages.c into PNG files.

    python3 tools/ppm2png.py <ppm_dir> <png_dir>

Writes <set>_<n>.png for every page, plus <set>_sheet.png, a contact sheet of
all pages of a set. Requires Pillow.
"""
import glob
import os
import re
import sys

from PIL import Image


def main(src, dst):
    os.makedirs(dst, exist_ok=True)
    sets = {}
    for path in sorted(glob.glob(os.path.join(src, '*.ppm'))):
        m = re.match(r'(\w+?)_(\d+)\.ppm$', os.path.basename(path))
        if not m:
            continue
        img = Image.open(path).convert('RGB')
        img.save(os.path.join(dst, '%s_%s.png' % (m.group(1), m.group(2))), optimize=True)
        sets.setdefault(m.group(1), []).append((int(m.group(2)), img))
    for name, pages in sets.items():
        pages.sort(key=lambda p: p[0])
        cols = 4 if len(pages) > 4 else len(pages)
        rows = (len(pages) + cols - 1) // cols
        gap = 12
        sheet = Image.new('RGB', (cols * 240 + (cols + 1) * gap, rows * 240 + (rows + 1) * gap), (24, 28, 36))
        for i, (_, img) in enumerate(pages):
            sheet.paste(img, (gap + (i % cols) * (240 + gap), gap + (i // cols) * (240 + gap)))
        sheet.save(os.path.join(dst, '%s_sheet.png' % name), optimize=True)
    print('wrote %d pages and %d sheets to %s' % (sum(len(v) for v in sets.values()), len(sets), dst))


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
