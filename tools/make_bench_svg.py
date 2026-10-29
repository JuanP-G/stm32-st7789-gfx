#!/usr/bin/env python3
"""Draw docs/img/writes-comparison.svg from the benchmark's CSV output.

    build/bench --csv > build/bench.csv
    python3 tools/make_bench_svg.py build/bench.csv docs/img/writes-comparison.svg

The chart uses a logarithmic axis: for each primitive, one bar for the bus
writes of window + burst and one for the same pixels written one by one
(12 writes per pixel). No third-party libraries are used.
"""
import csv
import math
import sys

PICK = ['pixel', 'hline_200', 'line', 'text_12_chars', 'icon_32', 'circle_r60',
        'rect_100x60', 'image_120x80', 'screen', 'dashboard_page']
LABEL = {
    'pixel': 'One pixel', 'hline_200': 'Line, 200 px', 'line': 'Diagonal line',
    'text_12_chars': 'Text, 12 chars', 'icon_32': 'Icon 32 x 32', 'circle_r60': 'Filled circle r60',
    'rect_100x60': 'Rectangle 100 x 60', 'image_120x80': 'Image 120 x 80',
    'screen': 'Full screen', 'dashboard_page': 'Dashboard page',
}


def fmt(n):
    return '{:,}'.format(int(n))


def main(src, dst):
    with open(src, newline='', encoding='utf-8') as f:
        rows = {r['name']: r for r in csv.DictReader(f)}
    items = [rows[k] for k in PICK if k in rows]
    lo, hi = 1, 10 ** 7                      # log axis range
    left, right, top = 190, 740, 64
    row_h = 44
    width = 800
    height = top + row_h * len(items) + 70

    def x(v):
        v = max(v, lo)
        return left + (right - left) * (math.log10(v) - math.log10(lo)) / (math.log10(hi) - math.log10(lo))

    out = []
    a = out.append
    a('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %d %d" role="img" aria-labelledby="t d">' % (width, height))
    a('  <title id="t">Bus writes: window and burst compared with pixel by pixel</title>')
    a('  <desc id="d">Horizontal bars on a logarithmic axis. For every primitive, window and burst needs far fewer bus writes than pixel by pixel drawing, except for a single pixel where both cost 12.</desc>')
    a('  <style>')
    a('    :root { --bg:#ffffff; --tx:#17202e; --dim:#566379; --grid:#dfe5ef; --burst:#2f6fed; --naive:#c9d2e3; }')
    a('    @media (prefers-color-scheme: dark) { :root { --bg:#10141c; --tx:#e7ecf5; --dim:#97a3b8; --grid:#273146; --burst:#6fa3ff; --naive:#3b4a63; } }')
    a('    text { font-family: system-ui, -apple-system, "Segoe UI", Roboto, sans-serif; fill: var(--tx); font-size: 13px; }')
    a('    .bg { fill: var(--bg); } .dim { fill: var(--dim); font-size: 12px; } .h { font-weight: 600; font-size: 15px; }')
    a('    .grid { stroke: var(--grid); stroke-width: 1; } .burst { fill: var(--burst); } .naive { fill: var(--naive); }')
    a('    .r { text-anchor: end; }')
    a('  </style>')
    a('  <rect class="bg" width="%d" height="%d"/>' % (width, height))
    a('  <text class="h" x="20" y="28">Bus writes per drawing operation (logarithmic axis)</text>')
    a('  <rect class="burst" x="20" y="40" width="12" height="10"/><text class="dim" x="38" y="49">window + burst (this library)</text>')
    a('  <rect class="naive" x="240" y="40" width="12" height="10"/><text class="dim" x="258" y="49">pixel by pixel (12 writes per pixel)</text>')
    for p in range(0, 8):
        gx = x(10 ** p)
        a('  <line class="grid" x1="%.1f" y1="%d" x2="%.1f" y2="%d"/>' % (gx, top - 4, gx, top + row_h * len(items) - 4))
        label = fmt(10 ** p) if p < 7 else '10M'
        a('  <text class="dim" x="%.1f" y="%d" text-anchor="middle">%s</text>' % (gx, top + row_h * len(items) + 14, label))
    for i, r in enumerate(items):
        y = top + i * row_h
        w, n = int(r['writes']), int(r['naive_writes'])
        a('  <text class="r" x="%d" y="%d">%s</text>' % (left - 10, y + 17, LABEL.get(r['name'], r['name'])))
        a('  <rect class="burst" x="%d" y="%d" width="%.1f" height="12"/>' % (left, y + 4, max(x(w) - left, 2)))
        a('  <text class="dim" x="%.1f" y="%d">%s</text>' % (x(w) + 6, y + 14, fmt(w)))
        a('  <rect class="naive" x="%d" y="%d" width="%.1f" height="12"/>' % (left, y + 20, max(x(n) - left, 2)))
        a('  <text class="dim" x="%.1f" y="%d">%s</text>' % (x(n) + 6, y + 30, fmt(n)))
    a('</svg>')
    with open(dst, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(out) + '\n')
    print('wrote', dst)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
