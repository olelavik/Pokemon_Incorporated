"""Render pokefirered metatiles and map layouts to PNG (for planning map edits).

    python3 maprender.py layout LAYOUT_ID out.png        render a whole layout
    python3 maprender.py sheet PRIMARY SECONDARY out.png  every metatile, numbered
"""
import json, os, re, struct, sys
from PIL import Image, ImageDraw

ROOT = os.path.expanduser('~/pokefirered')
TS = os.path.join(ROOT, 'data/tilesets')


def tileset_dir(sym):
    # Look the real path up in headers/graphics.h (handles SSAnne -> ss_anne etc.)
    hdr = open(os.path.join(ROOT, 'src/data/tilesets/headers.h')).read()
    gfx = open(os.path.join(ROOT, 'src/data/tilesets/graphics.h')).read()
    m = re.search(r'const struct Tileset ' + re.escape(sym) + r' =\s*\{(.*?)\};', hdr, re.S)
    if m:
        tiles = re.search(r'\.tiles = (\w+)', m.group(1)).group(1)
        p = re.search(re.escape(tiles) + r'\[\] = INCBIN_U32\("(data/tilesets/[^"]+)/tiles', gfx)
        if p:
            return os.path.join(ROOT, p.group(1))
    # gTileset_ViridianCity -> viridian_city ; gTileset_Building -> building
    name = sym.replace('gTileset_', '')
    snake = ''.join('_' + c.lower() if c.isupper() else c for c in name).lstrip('_')
    for kind in ('primary', 'secondary'):
        p = os.path.join(TS, kind, snake)
        if os.path.isdir(p):
            return p
    # names with digits, e.g. GenericBuilding1 -> generic_building_1
    snake2 = re.sub(r'(\d+)', r'_\1', snake)
    for kind in ('primary', 'secondary'):
        p = os.path.join(TS, kind, snake2)
        if os.path.isdir(p):
            return p
    raise FileNotFoundError(sym)


def load_pals(d):
    pals = []
    for i in range(16):
        p = os.path.join(d, 'palettes', '%02d.pal' % i)
        cols = []
        if os.path.exists(p):
            lines = open(p).read().split('\n')[3:]
            for ln in lines[:16]:
                parts = ln.split()
                if len(parts) == 3:
                    cols.append(tuple(int(v) for v in parts))
        while len(cols) < 16:
            cols.append((255, 0, 255))
        pals.append(cols)
    return pals


class Tileset:
    def __init__(self, primary, secondary):
        self.pd, self.sd = tileset_dir(primary), tileset_dir(secondary)
        self.ptiles = Image.open(os.path.join(self.pd, 'tiles.png'))
        self.stiles = Image.open(os.path.join(self.sd, 'tiles.png'))
        pp, sp = load_pals(self.pd), load_pals(self.sd)
        self.pals = pp[:7] + sp[7:13] + pp[13:]
        self.pmeta = open(os.path.join(self.pd, 'metatiles.bin'), 'rb').read()
        self.smeta = open(os.path.join(self.sd, 'metatiles.bin'), 'rb').read()
        self.cache = {}

    def tile(self, idx, pal, hf, vf):
        img, n = (self.ptiles, idx) if idx < 640 else (self.stiles, idx - 640)
        cols = img.size[0] // 8
        if n >= cols * (img.size[1] // 8):
            return None
        t = img.crop(((n % cols) * 8, (n // cols) * 8, (n % cols) * 8 + 8, (n // cols) * 8 + 8))
        out = Image.new('RGBA', (8, 8))
        px = t.load()
        po = out.load()
        for y in range(8):
            for x in range(8):
                c = px[x, y] & 15
                po[x, y] = (0, 0, 0, 0) if c == 0 else self.pals[pal][c] + (255,)
        if hf: out = out.transpose(Image.FLIP_LEFT_RIGHT)
        if vf: out = out.transpose(Image.FLIP_TOP_BOTTOM)
        return out

    def metatile(self, mid):
        if mid in self.cache:
            return self.cache[mid]
        data, n = (self.pmeta, mid) if mid < 640 else (self.smeta, mid - 640)
        img = Image.new('RGBA', (16, 16), self.pals[0][0] + (255,))
        if n * 16 + 16 <= len(data):
            for layer in range(2):
                for i in range(4):
                    v = struct.unpack_from('<H', data, n * 16 + (layer * 4 + i) * 2)[0]
                    t = self.tile(v & 0x3FF, (v >> 12) & 15, v & 0x400, v & 0x800)
                    if t:
                        img.alpha_composite(t, ((i % 2) * 8, (i // 2) * 8))
        self.cache[mid] = img
        return img


def layouts():
    return {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}


def render_layout(lid, out, grid=None):
    l = layouts()[lid]
    ts = Tileset(l['primary_tileset'], l['secondary_tileset'])
    w, h = l['width'], l['height']
    b = grid or open(os.path.join(ROOT, l['blockdata_filepath']), 'rb').read()
    img = Image.new('RGBA', (w * 16, h * 16))
    for y in range(h):
        for x in range(w):
            v = struct.unpack_from('<H', b, (y * w + x) * 2)[0]
            img.alpha_composite(ts.metatile(v & 0x3FF), (x * 16, y * 16))
    img.save(out)


def render_sheet(primary, secondary, out, ids):
    ts = Tileset(primary, secondary)
    cols = 16
    rows = (len(ids) + cols - 1) // cols
    img = Image.new('RGB', (cols * 34, rows * 42), 'white')
    d = ImageDraw.Draw(img)
    for i, mid in enumerate(ids):
        x, y = (i % cols) * 34, (i // cols) * 42
        img.paste(ts.metatile(mid).resize((32, 32), Image.NEAREST), (x, y), ts.metatile(mid).resize((32, 32), Image.NEAREST))
        d.text((x, y + 32), str(mid), fill='black')
    img.save(out)


if __name__ == '__main__':
    if sys.argv[1] == 'layout':
        render_layout(sys.argv[2], sys.argv[3])
    elif sys.argv[1] == 'sheet':
        start, end = int(sys.argv[5]), int(sys.argv[6])
        render_sheet(sys.argv[2], sys.argv[3], sys.argv[4], list(range(start, end)))
