#!/usr/bin/env python3
"""Generate gfx_icons.c/.h: 32x32 icons with 4-bit transparency (anti-aliased).

Each icon is drawn as a vector shape at 8x and scaled down. Usage:

    python3 gen_icons.py <output_dir>

Also writes icons_preview.png, a contact sheet of all icons. Requires Pillow."""
import sys, os, math
from PIL import Image, ImageDraw

N, S = 32, 8   # final size and supersampling factor

class Pen:
    def __init__(self): self.im = Image.new('L', (N*S, N*S), 0); self.d = ImageDraw.Draw(self.im)
    def _s(self, v): return [x*S for x in v]
    def poly(self, pts, fill=255): self.d.polygon([(x*S, y*S) for x, y in pts], fill=fill)
    def circle(self, cx, cy, r, fill=255): self.d.ellipse([(cx-r)*S, (cy-r)*S, (cx+r)*S, (cy+r)*S], fill=fill)
    def ring(self, cx, cy, r, w, fill=255):
        self.d.ellipse([(cx-r)*S, (cy-r)*S, (cx+r)*S, (cy+r)*S], outline=fill, width=int(w*S))
    def line(self, x0, y0, x1, y1, w, fill=255):
        self.d.line([x0*S, y0*S, x1*S, y1*S], fill=fill, width=int(w*S))
        for x, y in ((x0, y0), (x1, y1)): self.circle(x, y, w/2, fill)
    def poly_line(self, pts, w, fill=255):
        for a, b in zip(pts, pts[1:]): self.line(a[0], a[1], b[0], b[1], w, fill)
    def rrect(self, x0, y0, x1, y1, r, fill=255):
        self.d.rounded_rectangle([x0*S, y0*S, x1*S, y1*S], radius=r*S, fill=fill)
    def rrect_o(self, x0, y0, x1, y1, r, w, fill=255):
        self.d.rounded_rectangle([x0*S, y0*S, x1*S, y1*S], radius=r*S, outline=fill, width=int(w*S))
    def arc(self, cx, cy, r, a0, a1, w, fill=255):
        self.d.arc([(cx-r)*S, (cy-r)*S, (cx+r)*S, (cy+r)*S], a0, a1, fill=fill, width=int(w*S))
    def done(self): return self.im.resize((N, N), Image.LANCZOS)

def home():
    p = Pen(); p.poly([(16,3),(2.5,15.5),(6.5,15.5),(6.5,28),(13,28),(13,19),(19,19),(19,28),(25.5,28),(25.5,15.5),(29.5,15.5)]); return p.done()
def gear():
    p = Pen(); p.circle(16,16,9.5)
    for k in range(8):
        a = k*math.pi/4; p.line(16+8*math.cos(a),16+8*math.sin(a),16+12.6*math.cos(a),16+12.6*math.sin(a),4.6)
    p.circle(16,16,4.2,0); return p.done()
def wifi():
    p = Pen()
    for r in (21, 14, 7): p.arc(16, 26, r, 232, 308, 3)
    p.circle(16, 26, 2.6); return p.done()
def battery():
    p = Pen(); p.rrect_o(2.5,8.5,26,23.5,3,2.2); p.rrect(27.5,13,30,19,1); p.rrect(6,12,19,20,1); return p.done()
def bell():
    p = Pen(); p.circle(16,13,8); p.poly([(8,13),(24,13),(27.5,23),(4.5,23)]); p.circle(16,26.2,2.6); p.circle(16,4.8,1.9); return p.done()
def user():
    p = Pen(); p.circle(16,10,5.8); p.d.ellipse([5*S,18*S,27*S,38*S], fill=255)
    p.d.rectangle([0,30*S,N*S,N*S], fill=0); return p.done()
def lock():
    p = Pen(); p.rrect(6.5,14,25.5,28.5,2.5); p.arc(16,13,6,180,360,3.2); p.line(10,13,10,15,3.2); p.line(22,13,22,15,3.2)
    p.circle(16,20,2.3,0); p.rrect(15.1,20,16.9,24.5,0.5,0); return p.done()
def heart():
    p = Pen(); p.circle(10.6,11.6,6.6); p.circle(21.4,11.6,6.6); p.poly([(4.2,14.2),(27.8,14.2),(16,28.5)]); return p.done()
def star():
    pts = []
    for k in range(10):
        r = 14.5 if k % 2 == 0 else 6; a = -math.pi/2 + k*math.pi/5
        pts.append((16+r*math.cos(a), 17+r*math.sin(a)))
    p = Pen(); p.poly(pts); return p.done()
def sun():
    p = Pen(); p.circle(16,16,6.2)
    for k in range(8):
        a = k*math.pi/4; p.line(16+9.8*math.cos(a),16+9.8*math.sin(a),16+13.8*math.cos(a),16+13.8*math.sin(a),2.6)
    return p.done()
def moon():
    p = Pen(); p.circle(15,16,11.5); p.circle(21.5,11.5,9.5,0); return p.done()
def check():
    p = Pen(); p.poly_line([(6,17),(13,24),(26,9)], 4.2); return p.done()
def close():
    p = Pen(); p.line(8,8,24,24,4); p.line(24,8,8,24,4); return p.done()
def plus():
    p = Pen(); p.line(16,6,16,26,4); p.line(6,16,26,16,4); return p.done()
def minus():
    p = Pen(); p.line(6,16,26,16,4); return p.done()
def _arrow_right():
    p = Pen(); p.line(5,16,26,16,3.6); p.poly_line([(17,7),(26,16),(17,25)], 3.6); return p.done()
def arrow_right(): return _arrow_right()
def arrow_left(): return _arrow_right().transpose(Image.FLIP_LEFT_RIGHT)
def arrow_up(): return _arrow_right().transpose(Image.ROTATE_90)
def arrow_down(): return _arrow_right().transpose(Image.ROTATE_270)
def play():
    p = Pen(); p.poly([(9,4.5),(9,27.5),(27,16)]); return p.done()
def pause():
    p = Pen(); p.rrect(7.5,5,13.5,27,1.5); p.rrect(18.5,5,24.5,27,1.5); return p.done()
def search():
    p = Pen(); p.ring(13,13,8.5,3.2); p.line(19.5,19.5,27,27,4.2); return p.done()
def mail():
    p = Pen(); p.rrect_o(2.5,6.5,29.5,25.5,3,2.4); p.poly_line([(4.5,9.5),(16,18.5),(27.5,9.5)], 2.4); return p.done()
def clock():
    p = Pen(); p.ring(16,16,13,2.8); p.poly_line([(16,8),(16,16),(22,19.5)], 2.6); return p.done()
def bolt():
    p = Pen(); p.poly([(19.5,2),(7,18),(14.5,18),(12,30),(25,12.5),(17.5,12.5),(22,2)]); return p.done()
def power():
    p = Pen(); p.arc(16,17,10.5,300,360,3.2); p.arc(16,17,10.5,0,240,3.2); p.line(16,3.5,16,15,3.4); return p.done()
def thermometer():
    p = Pen(); p.rrect(12,3,20,21,4); p.circle(16,24,5.8); p.rrect(14.6,9,17.4,24,1.4,0); p.circle(16,24,3,0); return p.done()
def drop():
    p = Pen(); p.circle(16,20,8.2); p.poly([(16,3),(8.9,17),(23.1,17)]); return p.done()
def menu():
    p = Pen()
    for y in (7, 14.5, 22): p.rrect(5,y,27,y+3.6,1.8)
    return p.done()
def info():
    p = Pen(); p.circle(16,16,13.5); p.circle(16,9.8,2,0); p.rrect(14.1,14,17.9,24,1,0); return p.done()
def warning():
    p = Pen(); p.poly([(16,3),(30,28),(2,28)]); p.rrect(14.4,11.5,17.6,21,1.2,0); p.circle(16,24.2,1.8,0); return p.done()
def pin():
    p = Pen(); p.circle(16,12,9); p.poly([(7.7,15.5),(24.3,15.5),(16,29.5)]); p.circle(16,12,3.6,0); return p.done()

ICONS = [home, gear, wifi, battery, bell, user, lock, heart, star, sun, moon, check, close, plus, minus,
         arrow_left, arrow_right, arrow_up, arrow_down, play, pause, search, mail, clock, bolt, power,
         thermometer, drop, menu, info, warning, pin]

def main(out):
    os.makedirs(out, exist_ok=True)
    c = ['/* Generated by tools/gen_icons.py - do not edit by hand. 32x32 icons, 4 bits of opacity. */',
         '#include "gfx_icons.h"', '']
    h = ['/* Generated by tools/gen_icons.py - do not edit by hand. */', '#ifndef GFX_ICONS_H_', '#define GFX_ICONS_H_', '',
         '#include "gfx_core.h"', '', '#ifdef __cplusplus', 'extern "C" {', '#endif', '']
    sheet = Image.new('RGB', (8*40, 4*40), (32, 36, 48))
    for i, fn in enumerate(ICONS):
        im = fn(); name = fn.__name__
        blob = bytearray()
        for y in range(N):
            row = [min(15, (im.getpixel((x, y)) + 8)//17) for x in range(N)]
            for k in range(0, N, 2): blob.append((row[k] << 4) | row[k+1])
        c.append(f'static const uint8_t data_{name}[{len(blob)}] = {{')
        for k in range(0, len(blob), 32): c.append('  ' + ','.join(f'0x{b:02X}' for b in blob[k:k+32]) + ',')
        c.append('};')
        c.append(f'const gfx_image_a4_t gfx_icon_{name} = {{{N}, {N}, data_{name}}};')
        h.append(f'extern const gfx_image_a4_t gfx_icon_{name};')
        sheet.paste((235, 240, 250), (4 + (i % 8)*40, 4 + (i//8)*40), im)
    h += ['', '#ifdef __cplusplus', '}', '#endif', '', '#endif /* GFX_ICONS_H_ */', '']
    open(os.path.join(out, 'gfx_icons.c'), 'w', encoding='utf-8', newline='\n').write('\n'.join(c))
    open(os.path.join(out, 'gfx_icons.h'), 'w', encoding='utf-8', newline='\n').write('\n'.join(h))
    sheet.resize((sheet.width*3, sheet.height*3), Image.NEAREST).save(os.path.join(out, 'icons_preview.png'))
    print(len(ICONS), 'icons')

if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '.')
