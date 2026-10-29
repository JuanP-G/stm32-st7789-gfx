#!/usr/bin/env python3
"""Check the generated site in docs/ for broken links and SEO mistakes.

Standard library only, except for the og:image size check which uses Pillow
when it is installed (the PNG header is parsed directly otherwise).

    python3 tools/check_site.py

Exit status is 0 when everything is fine, 1 otherwise.
"""
import json
import os
import re
import struct
import sys
from html.parser import HTMLParser
from urllib.parse import unquote, urldefrag, urlparse

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(ROOT, 'docs')
SITE = 'https://juanp-g.github.io/stm32-st7789-gfx/'
BASE_PATH = '/stm32-st7789-gfx/'

errors = []


def fail(page, message):
    errors.append('%s: %s' % (page, message))


class Page(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.links = []        # (attr, value) of href/src
        self.ids = []
        self.metas = {}        # name or property -> content
        self.canonicals = []
        self.alternates = []
        self.title = ''
        self.h1 = 0
        self.imgs = []
        self.jsonld = []
        self._in_title = False
        self._in_ld = False
        self._ld = ''

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        if 'id' in a:
            self.ids.append(a['id'])
        if tag == 'title':
            self._in_title = True
        elif tag == 'h1':
            self.h1 += 1
        elif tag == 'meta':
            key = a.get('name') or a.get('property')
            if key:
                self.metas[key] = a.get('content', '')
        elif tag == 'link':
            if a.get('rel') == 'canonical':
                self.canonicals.append(a.get('href', ''))
            elif a.get('rel') == 'alternate':
                self.alternates.append(a.get('href', ''))
            if a.get('href'):
                self.links.append(('href', a['href']))
        elif tag == 'script':
            if a.get('type') == 'application/ld+json':
                self._in_ld = True
                self._ld = ''
            if a.get('src'):
                self.links.append(('src', a['src']))
        elif tag == 'a':
            if a.get('href') is not None:
                self.links.append(('href', a['href']))
        elif tag == 'img':
            self.imgs.append(a)
            if a.get('src'):
                self.links.append(('src', a['src']))

    def handle_endtag(self, tag):
        if tag == 'title':
            self._in_title = False
        elif tag == 'script' and self._in_ld:
            self._in_ld = False
            self.jsonld.append(self._ld)

    def handle_data(self, data):
        if self._in_title:
            self.title += data
        if self._in_ld:
            self._ld += data


def parse(path):
    p = Page()
    with open(path, encoding='utf-8') as f:
        p.feed(f.read())
    return p


def png_size(path):
    with open(path, 'rb') as f:
        head = f.read(24)
    if head[:8] != b'\x89PNG\r\n\x1a\n':
        return None
    return struct.unpack('>II', head[16:24])


def page_files():
    out = []
    for dirpath, _, names in os.walk(DOCS):
        for n in names:
            if n.endswith('.html') and not n.startswith('google'):  # search console ownership files are not pages
                out.append(os.path.join(dirpath, n))
    return sorted(out)


def url_to_path(url):
    """Map an absolute site URL to a file in docs/, or None."""
    rel = url[len(SITE):]
    if rel == '' or rel.endswith('/'):
        rel += 'index.html'
    return os.path.join(DOCS, rel)


def resolve(page_path, target):
    """Return the file a link points to, '' for non-file targets, or None when it is external."""
    target = target.strip()
    if not target or target.startswith(('mailto:', 'tel:', 'javascript:', 'data:')):
        return ''
    u = urlparse(target)
    if u.scheme in ('http', 'https'):
        if target.startswith(SITE):
            return url_to_path(urldefrag(target)[0])
        return None
    path = unquote(urldefrag(target)[0])
    if path.startswith('/'):
        if not path.startswith(BASE_PATH):
            return os.path.join(DOCS, '__outside__', path)
        path = path[len(BASE_PATH):]
        full = os.path.join(DOCS, path)
    elif path == '':
        return page_path
    else:
        full = os.path.normpath(os.path.join(os.path.dirname(page_path), path))
    if path.endswith('/') or path == '':
        full = os.path.join(full, 'index.html')
    return full


def main():
    files = page_files()
    parsed = {f: parse(f) for f in files}
    rel = lambda f: os.path.relpath(f, DOCS).replace(os.sep, '/')
    titles, descs = {}, {}

    for f, p in parsed.items():
        name = rel(f)
        noindex = 'noindex' in p.metas.get('robots', '')

        # head metadata
        title = p.title.strip()
        desc = p.metas.get('description', '').strip()
        if not title:
            fail(name, 'missing <title>')
        elif not 25 <= len(title) <= 100:
            fail(name, 'title length %d outside 25..100' % len(title))
        if not desc:
            fail(name, 'missing meta description')
        elif not 70 <= len(desc) <= 175:
            fail(name, 'description length %d outside 70..175' % len(desc))
        if title in titles:
            fail(name, 'title duplicates %s' % titles[title])
        titles[title] = name
        if desc in descs:
            fail(name, 'description duplicates %s' % descs[desc])
        descs[desc] = name
        if p.h1 != 1:
            fail(name, 'expected exactly one <h1>, found %d' % p.h1)
        for img in p.imgs:
            if 'alt' not in img:
                fail(name, 'image without alt: %s' % img.get('src'))
            if not img.get('width') or not img.get('height'):
                fail(name, 'image without width/height: %s' % img.get('src'))
        dup = {i for i in p.ids if p.ids.count(i) > 1}
        if dup:
            fail(name, 'duplicate ids: %s' % ', '.join(sorted(dup)))

        # canonical
        if not noindex:
            expected = SITE + ('' if name == 'index.html' else
                               'es/' if name == 'es/index.html' else name)
            if p.canonicals != [expected]:
                fail(name, 'canonical %s, expected %s' % (p.canonicals, expected))
        for key in ('og:title', 'og:description', 'og:url', 'og:image', 'twitter:card', 'twitter:image'):
            if not p.metas.get(key):
                fail(name, 'missing %s' % key)
        if p.metas.get('og:image') and p.metas['og:image'].startswith(SITE):
            img_path = url_to_path(p.metas['og:image'])
            if not os.path.isfile(img_path):
                fail(name, 'og:image file missing: %s' % p.metas['og:image'])
            else:
                size = png_size(img_path)
                if size != (1200, 630):
                    fail(name, 'og:image is %s, expected (1200, 630)' % (size,))
        elif p.metas.get('og:image'):
            fail(name, 'og:image is not an absolute URL under the site')

        # JSON-LD
        for blob in p.jsonld:
            try:
                data = json.loads(blob)
            except ValueError as e:
                fail(name, 'invalid JSON-LD: %s' % e)
                continue
            for key in ('@context', '@type', 'name', 'url', 'codeRepository', 'license'):
                if key not in data:
                    fail(name, 'JSON-LD lacks %s' % key)

        # links and anchors
        for attr, value in p.links:
            target = resolve(f, value)
            if target is None or target == '':
                continue
            if not os.path.isfile(target):
                fail(name, '%s="%s" does not resolve to a file' % (attr, value))
                continue
            frag = urldefrag(value)[1]
            if frag and target.endswith('.html'):
                other = parsed.get(target) or parse(target)
                if frag not in other.ids:
                    fail(name, 'anchor #%s not found in %s' % (frag, rel(target)))

    # hreflang pairs
    for f in (os.path.join(DOCS, 'index.html'), os.path.join(DOCS, 'es', 'index.html')):
        alts = parsed[f].alternates
        for want in (SITE, SITE + 'es/'):
            if want not in alts:
                fail(rel(f), 'missing hreflang alternate %s' % want)

    # sitemap
    sm = os.path.join(DOCS, 'sitemap.xml')
    if not os.path.isfile(sm):
        fail('sitemap.xml', 'missing')
    else:
        text = open(sm, encoding='utf-8').read()
        locs = re.findall(r'<loc>([^<]+)</loc>', text)
        for loc in locs:
            if not loc.startswith(SITE) or not os.path.isfile(url_to_path(loc)):
                fail('sitemap.xml', 'loc does not map to a file: %s' % loc)
        for f, p in parsed.items():
            if 'noindex' in p.metas.get('robots', ''):
                continue
            expected = SITE + ('' if rel(f) == 'index.html' else 'es/' if rel(f) == 'es/index.html' else rel(f))
            if expected not in locs:
                fail('sitemap.xml', 'indexable page missing: %s' % expected)

    # robots
    rb = os.path.join(DOCS, 'robots.txt')
    if not os.path.isfile(rb):
        fail('robots.txt', 'missing')
    elif ('Sitemap: ' + SITE + 'sitemap.xml') not in open(rb, encoding='utf-8').read():
        fail('robots.txt', 'lacks an absolute Sitemap line')

    # 404 must work from any depth
    nf = parsed.get(os.path.join(DOCS, '404.html'))
    if nf is None:
        fail('404.html', 'missing')
    else:
        for attr, value in nf.links:
            if not value.startswith(('/', 'http', '#')):
                fail('404.html', 'relative %s="%s" breaks on nested URLs' % (attr, value))

    for needed in ('favicon.svg', '.nojekyll', os.path.join('assets', 'style.css')):
        if not os.path.exists(os.path.join(DOCS, needed)):
            fail(needed, 'missing')

    if errors:
        print('site check FAILED (%d problems)' % len(errors))
        for e in errors:
            print('  -', e)
        return 1
    print('site check OK: %d pages, %d unique titles' % (len(files), len(titles)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
