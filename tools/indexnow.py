#!/usr/bin/env python3
"""Tell IndexNow-compatible search engines (Bing, Yandex, Naver, Seznam, Yep)
that the pages listed in docs/sitemap.xml are new or changed.

Standard library only. The ownership key is served from the site root, see
INDEXNOW_KEY in tools/build_site.py.

    python3 tools/indexnow.py
"""
import json
import os
import re
import sys
import urllib.error
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from build_site import INDEXNOW_KEY, SITE  # noqa: E402

ENDPOINT = 'https://api.indexnow.org/indexnow'


def main():
    with open(os.path.join(ROOT, 'docs', 'sitemap.xml'), encoding='utf-8') as f:
        urls = re.findall(r'<loc>([^<]+)</loc>', f.read())
    payload = {
        'host': SITE.split('/')[2],
        'key': INDEXNOW_KEY,
        'keyLocation': '%s%s.txt' % (SITE, INDEXNOW_KEY),
        'urlList': urls,
    }
    req = urllib.request.Request(ENDPOINT, json.dumps(payload).encode('utf-8'),
                                 {'Content-Type': 'application/json; charset=utf-8'})
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            print('IndexNow: HTTP %d for %d URLs' % (r.status, len(urls)))
    except urllib.error.HTTPError as e:
        print('IndexNow: HTTP %d %s' % (e.code, e.read().decode('utf-8', 'replace')))
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
