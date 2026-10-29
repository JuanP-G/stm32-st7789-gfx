#!/usr/bin/env python3
"""Build the static documentation site into docs/.

Page bodies live in site_src/<lang>/<page>.html as HTML fragments. This script
wraps them with the shared head (SEO tags), navigation and footer, copies the
static assets and writes sitemap.xml and robots.txt. The generated docs/ is
committed so GitHub Pages can serve it as it is.

    python3 tools/build_site.py
"""
import json
import os
import re
import shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'site_src')
OUT = os.path.join(ROOT, 'docs')
SITE = 'https://juanp-g.github.io/stm32-st7789-gfx/'
# IndexNow ownership key. It is public by design: search engines fetch it from
# the site root to confirm that the submitted URLs belong to this host.
INDEXNOW_KEY = '42bc971095ba7d2f1bab2abd2f46cddf'
REPO = 'https://github.com/JuanP-G/stm32-st7789-gfx'
OG_IMAGE = SITE + 'img/og-image.png'
KEYWORDS = ('STM32 ST7789 library, FMC LCD driver, STM32F723E-DISCO, RGB565, embedded GUI in C, '
            'anti-aliased fonts for microcontrollers, host-testable')

EN_NAV = [('index.html', 'Home'), ('getting-started.html', 'Getting started'),
          ('architecture.html', 'Architecture'), ('api.html', 'API'), ('testing.html', 'Testing')]

PAGES = [
    dict(src='en/index.html', out='index.html', url='', lang='en', nav='index.html', jsonld=True,
         title='STM32 ST7789 Library: Portable C Graphics for FMC LCDs | stm32-st7789-gfx',
         desc='Portable C library for ST7789 displays on STM32: shapes, anti-aliased fonts, icons and widgets over FMC, '
              'SPI or 8-bit. Integer only, tested on a PC.'),
    dict(src='en/getting-started.html', out='getting-started.html', url='getting-started.html', lang='en',
         nav='getting-started.html',
         title='Getting Started: Add the ST7789 Library to an STM32CubeIDE Project',
         desc='Add stm32-st7789-gfx to an STM32CubeIDE project and write the port for FMC, SPI or an 8-bit bus. '
              'Includes a troubleshooting table and known limits.'),
    dict(src='en/architecture.html', out='architecture.html', url='architecture.html', lang='en',
         nav='architecture.html',
         title='Architecture: Window and Burst Drawing for the ST7789 on STM32',
         desc='How the library draws: layered modules, one address window per run of pixels, a pixel burst, '
              'clipping, and a two-function hardware port.'),
    dict(src='en/api.html', out='api.html', url='api.html', lang='en', nav='api.html',
         title='API Reference: stm32-st7789-gfx Functions, Fonts, Icons and Widgets',
         desc='Reference for every public function of the ST7789 graphics library: core, shapes, text, images, '
              'widgets, fonts, icons, demo and self-test.'),
    dict(src='en/testing.html', out='testing.html', url='testing.html', lang='en', nav='testing.html',
         title='Testing: A Host-Testable Embedded GUI Library in C',
         desc='Three levels of testing: a PC controller model with golden pixel hashes, board test patterns, '
              'and integration. Includes the make test pipeline.'),
    dict(src='es/index.html', out='es/index.html', url='es/', lang='es', nav=None, jsonld=True,
         title='Biblioteca ST7789 para STM32: gráficos en C portables con FMC | stm32-st7789-gfx',
         desc='Biblioteca en C para pantallas ST7789 en STM32: formas, fuentes suavizadas, iconos y widgets '
              'por FMC, SPI o bus de 8 bits. Solo enteros y probada en el PC.'),
]

SITE_JS = '''(function () {
  var root = document.documentElement;
  var key = "theme";
  try { var saved = localStorage.getItem(key); if (saved) root.setAttribute("data-theme", saved); } catch (e) {}
  var btn = document.getElementById("theme-toggle");
  if (!btn) return;
  btn.addEventListener("click", function () {
    var current = root.getAttribute("data-theme");
    if (!current) current = window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
    var next = current === "dark" ? "light" : "dark";
    root.setAttribute("data-theme", next);
    try { localStorage.setItem(key, next); } catch (e) {}
  });
})();
'''

FAVICON = '''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" role="img" aria-label="stm32-st7789-gfx">
  <rect x="4" y="4" width="56" height="56" rx="12" fill="#10192e"/>
  <rect x="12" y="12" width="40" height="40" rx="4" fill="#0a0e16" stroke="#6fa3ff" stroke-width="3"/>
  <path d="M18 44 L28 30 L35 38 L41 26 L48 44 Z" fill="#6fa3ff"/>
  <circle cx="42" cy="19" r="4" fill="#ffd60a"/>
</svg>
'''


def read(path):
    with open(path, encoding='utf-8', newline='') as f:
        return f.read().replace('\r\n', '\n')


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)


def esc(s):
    return s.replace('&', '&amp;').replace('"', '&quot;').replace('<', '&lt;')


def jsonld(page):
    es = page['lang'] == 'es'
    data = {
        '@context': 'https://schema.org',
        '@type': 'SoftwareSourceCode',
        'name': 'stm32-st7789-gfx',
        'description': page['desc'],
        'url': SITE + page['url'],
        'codeRepository': REPO,
        'programmingLanguage': {'@type': 'ComputerLanguage', 'name': 'C'},
        'runtimePlatform': 'STM32 microcontrollers',
        'license': 'https://opensource.org/licenses/MIT',
        'version': '1.0.0',
        'datePublished': '2026-10-29',
        'inLanguage': page['lang'],
        'keywords': KEYWORDS,
        'image': OG_IMAGE,
        'author': {'@type': 'Person', 'name': 'Juan Pastrana', 'url': 'https://github.com/JuanP-G'},
        'about': 'Graphics library for ST7789 displays on STM32 (FMC, RGB565, 240 x 240)' if not es else
                 'Biblioteca gráfica para pantallas ST7789 en STM32 (FMC, RGB565, 240 x 240)',
    }
    return json.dumps(data, indent=2, ensure_ascii=False)


def build_toc(body, label):
    items = re.findall(r'<h2 id="([^"]+)">(.*?)</h2>', body, flags=re.S)
    if len(items) < 3:
        return ''
    links = ''.join('<a href="#%s">%s</a>' % (i, re.sub(r'<[^>]+>', '', t)) for i, t in items)
    return '<nav class="toc" aria-label="%s"><b>%s</b>%s</nav>' % (label, label, links)


def render(page, root, absolute=False):
    """root is the path prefix to the site root ('' or '../' or an absolute path)."""
    es = page['lang'] == 'es'
    body = read(os.path.join(SRC, page['src']))
    body = body.replace('{{root}}', root).replace('{{site}}', SITE).replace('{{repo}}', REPO)
    canonical = SITE + page['url']
    head = ['<meta charset="utf-8">',
            '<meta name="viewport" content="width=device-width, initial-scale=1">',
            '<title>%s</title>' % esc(page['title']),
            '<meta name="description" content="%s">' % esc(page['desc']),
            '<meta name="keywords" content="%s">' % esc(KEYWORDS),
            '<meta name="author" content="Juan Pastrana">',
            '<meta name="color-scheme" content="light dark">',
            '<meta name="theme-color" content="#2458d6">']
    if page.get('noindex'):
        head.append('<meta name="robots" content="noindex">')
    else:
        head.append('<meta name="robots" content="index, follow, max-image-preview:large">')
        head.append('<link rel="canonical" href="%s">' % canonical)
    if page['out'] in ('index.html', 'es/index.html'):
        head.append('<link rel="alternate" hreflang="en" href="%s">' % SITE)
        head.append('<link rel="alternate" hreflang="es" href="%ses/">' % SITE)
        head.append('<link rel="alternate" hreflang="x-default" href="%s">' % SITE)
    head += ['<link rel="icon" href="%sfavicon.svg" type="image/svg+xml">' % root,
             '<link rel="stylesheet" href="%sassets/style.css">' % root,
             '<meta property="og:type" content="website">',
             '<meta property="og:site_name" content="stm32-st7789-gfx">',
             '<meta property="og:title" content="%s">' % esc(page['title']),
             '<meta property="og:description" content="%s">' % esc(page['desc']),
             '<meta property="og:url" content="%s">' % canonical,
             '<meta property="og:locale" content="%s">' % ('es_ES' if es else 'en_US'),
             '<meta property="og:image" content="%s">' % OG_IMAGE,
             '<meta property="og:image:type" content="image/png">',
             '<meta property="og:image:width" content="1200">',
             '<meta property="og:image:height" content="630">',
             '<meta property="og:image:alt" content="stm32-st7789-gfx: graphics for ST7789 displays on STM32, with demo screens">',
             '<meta name="twitter:card" content="summary_large_image">',
             '<meta name="twitter:title" content="%s">' % esc(page['title']),
             '<meta name="twitter:description" content="%s">' % esc(page['desc']),
             '<meta name="twitter:image" content="%s">' % OG_IMAGE,
             '<meta name="twitter:image:alt" content="stm32-st7789-gfx: graphics for ST7789 displays on STM32, with demo screens">']
    if page.get('jsonld'):
        head.append('<script type="application/ld+json">\n%s\n</script>' % jsonld(page))

    if es:
        nav = ('<a href="./" aria-current="page">Inicio</a>'
               '<a href="%sgetting-started.html" hreflang="en" lang="en">Docs (EN)</a>'
               '<a href="%s">GitHub</a>'
               '<a class="lang" href="%sindex.html" hreflang="en" lang="en">English</a>') % (root, REPO, root)
    else:
        items = []
        for href, label in EN_NAV:
            cur = ' aria-current="page"' if page['nav'] == href else ''
            items.append('<a href="%s%s"%s>%s</a>' % (root, href, cur, label))
        items.append('<a href="%s">GitHub</a>' % REPO)
        items.append('<a class="lang" href="%ses/" hreflang="es" lang="es">Español</a>' % root)
        nav = ''.join(items)

    landing = page['out'] in ('index.html', 'es/index.html')
    toc = '' if landing else build_toc(body, 'Contenido' if es else 'On this page')
    main = ('<div class="doc"><article>%s</article>%s</div>' % (body, toc)) if toc else '<article>%s</article>' % body
    foot = (('<span>MIT License. Fuentes derivadas de DejaVu Sans: ver <a href="%s/blob/main/THIRD_PARTY_NOTICES.md">avisos de terceros</a>.</span>'
             '<a href="%s">GitHub</a><a href="%s/blob/main/CHANGELOG.md">Cambios</a>') if es else
            ('<span>MIT License. Fonts derived from DejaVu Sans: see <a href="%s/blob/main/THIRD_PARTY_NOTICES.md">third-party notices</a>.</span>'
             '<a href="%s">GitHub</a><a href="%s/blob/main/CHANGELOG.md">Changelog</a>')) % (REPO, REPO, REPO)
    brand_href = ('./' if es else root + 'index.html') if not absolute else root
    html = '''<!doctype html>
<html lang="%(lang)s">
<head>
%(head)s
</head>
<body>
<a class="skip" href="#main">%(skip)s</a>
<header class="site-header">
  <div class="wrap">
    <a class="brand" href="%(brand)s"><img src="%(root)sfavicon.svg" alt="" width="26" height="26">stm32-st7789-gfx</a>
    <nav class="nav" aria-label="%(navlabel)s">%(nav)s<button class="theme-btn" id="theme-toggle" type="button" aria-label="%(themelabel)s">%(theme)s</button></nav>
  </div>
</header>
<main id="main">
  <div class="wrap">
%(main)s
  </div>
</main>
<footer class="site-footer">
  <div class="wrap">%(foot)s</div>
</footer>
<script src="%(root)sassets/site.js" defer></script>
</body>
</html>
''' % dict(lang=page['lang'], head='\n'.join(head), skip='Saltar al contenido' if es else 'Skip to content',
           brand=brand_href, root=root, nav=nav, navlabel='Principal' if es else 'Main',
           themelabel='Cambiar tema claro u oscuro' if es else 'Toggle light or dark theme',
           theme='Tema' if es else 'Theme', main=main, foot=foot)
    return html


def sitemap():
    urls = [p for p in PAGES if not p.get('noindex')]
    lines = ['<?xml version="1.0" encoding="UTF-8"?>',
             '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9" xmlns:xhtml="http://www.w3.org/1999/xhtml">']
    for p in urls:
        lines.append('  <url>')
        lines.append('    <loc>%s%s</loc>' % (SITE, p['url']))
        lines.append('    <lastmod>2026-10-29</lastmod>')
        if p['out'] in ('index.html', 'es/index.html'):
            lines.append('    <xhtml:link rel="alternate" hreflang="en" href="%s"/>' % SITE)
            lines.append('    <xhtml:link rel="alternate" hreflang="es" href="%ses/"/>' % SITE)
        lines.append('  </url>')
    lines.append('</urlset>')
    return '\n'.join(lines) + '\n'


def main():
    os.makedirs(OUT, exist_ok=True)
    write(os.path.join(OUT, 'assets', 'style.css'), read(os.path.join(SRC, 'style.css')))
    write(os.path.join(OUT, 'assets', 'site.js'), SITE_JS)
    write(os.path.join(OUT, 'favicon.svg'), FAVICON)
    write(os.path.join(OUT, '.nojekyll'), '')
    write(os.path.join(OUT, INDEXNOW_KEY + '.txt'), INDEXNOW_KEY)
    for p in PAGES:
        root = '../' if p['out'].startswith('es/') else ''
        write(os.path.join(OUT, p['out']), render(p, root))
    notfound = dict(src='en/404.html', out='404.html', url='404.html', lang='en', nav=None, noindex=True,
                    title='Page not found | stm32-st7789-gfx',
                    desc='The page you asked for does not exist. Go back to the stm32-st7789-gfx documentation.')
    write(os.path.join(OUT, '404.html'), render(notfound, '/stm32-st7789-gfx/', absolute=True))
    write(os.path.join(OUT, 'sitemap.xml'), sitemap())
    write(os.path.join(OUT, 'robots.txt'), 'User-agent: *\nAllow: /\n\nSitemap: %ssitemap.xml\n' % SITE)
    # screenshots used by the pages
    shots = os.path.join(OUT, 'img', 'screens')
    os.makedirs(shots, exist_ok=True)
    pages_dir = os.path.join(ROOT, 'assets', 'pages')
    for name in os.listdir(pages_dir):
        if name.endswith('.png'):
            shutil.copyfile(os.path.join(pages_dir, name), os.path.join(shots, name))
    print('site built in docs/ (%d pages)' % (len(PAGES) + 1))


if __name__ == '__main__':
    main()
