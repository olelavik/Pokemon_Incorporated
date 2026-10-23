"""Pokémon Incorporated: add a Pokémon Exchange building to a town map.

The Exchange is the Poké Mart building recolored brown (General palette 1)
with "CASH" on the sign. Its 16 metatiles are appended to the town's
secondary tileset (they reference the shared General tiles), and the building
is stamped onto the town layout.

    python3 exchange_building.py sign                      draw the CASH sign tiles (once)
    python3 exchange_building.py add TOWN_TILESET LAYOUT_ID X Y
        X, Y = top-left corner of the 4x4 building. Door is at (X+2, Y+3).
        Prints the metatile base id. Safe to rerun: skips if already added.
"""
import json, os, struct, sys
from PIL import Image

ROOT = os.path.expanduser('~/pokefirered')
GEN = os.path.join(ROOT, 'data/tilesets/primary/general')

# The Poké Mart exterior in the General tileset, as 4 rows of 4 metatiles.
MART = [[40, 41, 42, 43], [48, 49, 50, 51], [56, 57, 58, 59], [64, 65, 98, 99]]
SIGN_TILES = {225: 482, 226: 483}  # "MART" letter tiles -> our "CASH" tiles
EXCHANGE_PAL = 1                   # brown roof
MARKER = b'PKINC-EXCHANGE'         # not stored in data; we detect by content

# 3x5 pixel font for the sign
FONT = {
    'C': ['011', '100', '100', '100', '011'],
    'A': ['010', '101', '111', '101', '101'],
    'S': ['011', '100', '010', '001', '110'],
    'H': ['101', '101', '111', '101', '101'],
}


def draw_sign():
    im = Image.open(os.path.join(GEN, 'tiles.png'))
    px = im.load()

    def tile_xy(n):
        return (n % 16) * 8, (n // 16) * 8

    # start from the original MART tiles so border/background match exactly
    for src, dst in SIGN_TILES.items():
        sx, sy = tile_xy(src)
        dx, dy = tile_xy(dst)
        for y in range(8):
            for x in range(8):
                px[dx + x, dy + y] = px[sx + x, sy + y]
    # clear the old letters (rows 2-6) to background colour 2 (keep col 0 of the left tile)
    for dst in SIGN_TILES.values():
        dx, dy = tile_xy(dst)
        for y in range(2, 7):
            for x in range(8):
                if dst == 482 and x == 0:
                    continue
                px[dx + x, dy + y] = 2
    # row colours of the original lettering (gradient)
    row_col = [0xB, 0xC, 0xD, 0xE, 0xE]
    word = 'CASH'
    for i, ch in enumerate(word):
        x0 = 1 + i * 4  # 16 px across two tiles: letters at 1,5,9,13
        for r, bits in enumerate(FONT[ch]):
            for c, bit in enumerate(bits):
                if bit == '1':
                    gx = x0 + c
                    tile = 482 if gx < 8 else 483
                    dx, dy = tile_xy(tile)
                    px[dx + gx % 8, dy + 2 + r] = row_col[r]
    im.save(os.path.join(GEN, 'tiles.png'))
    print('drew CASH sign into tiles', list(SIGN_TILES.values()))


def read_u16s(path):
    b = open(path, 'rb').read()
    return list(struct.unpack('<%dH' % (len(b) // 2), b))


def exchange_metatiles():
    gm = open(os.path.join(GEN, 'metatiles.bin'), 'rb').read()
    ga = open(os.path.join(GEN, 'metatile_attributes.bin'), 'rb').read()
    metas, attrs = [], []
    for row in MART:
        for mid in row:
            entries = list(struct.unpack_from('<8H', gm, mid * 16))
            out = []
            for v in entries:
                tile, pal = v & 0x3FF, (v >> 12) & 15
                if pal in (2, 3):
                    pal = EXCHANGE_PAL
                tile = SIGN_TILES.get(tile, tile)
                out.append((v & 0x0C00) | tile | (pal << 12))
            metas.append(out)
            attrs.append(ga[mid * 4: mid * 4 + 4])
    return metas, attrs


def add(town_dir, layout_id, x0, y0):
    tdir = os.path.join(ROOT, 'data/tilesets/secondary', town_dir)
    mpath, apath = os.path.join(tdir, 'metatiles.bin'), os.path.join(tdir, 'metatile_attributes.bin')
    metas, attrs = exchange_metatiles()
    blob = b''.join(struct.pack('<8H', *m) for m in metas)
    cur = open(mpath, 'rb').read()
    pos = cur.find(blob)
    if pos >= 0 and pos % 16 == 0:
        base = 640 + pos // 16
        print('already in tileset at', base)
    else:
        base = 640 + len(cur) // 16
        open(mpath, 'ab').write(blob)
        open(apath, 'ab').write(b''.join(attrs))
        print('appended 16 metatiles at', base)

    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    lay = layouts[layout_id]
    bpath = os.path.join(ROOT, lay['blockdata_filepath'])
    b = bytearray(open(bpath, 'rb').read())
    w = lay['width']
    # collision/elevation copied from a real Mart: the top roof row is walkable
    # (you pass behind it), everything else is solid
    for r in range(4):
        for c in range(4):
            i = ((y0 + r) * w + (x0 + c)) * 2
            meta = base + r * 4 + c
            if MART[r][c] == 98:
                meta = 98  # keep the real sliding door so its open/close animation plays
            upper = 0xC if r == 0 else 0x1
            struct.pack_into('<H', b, i, meta | (upper << 10))
    open(bpath, 'wb').write(b)
    print('stamped building at', (x0, y0), 'door at', (x0 + 2, y0 + 3))
    return base


if __name__ == '__main__':
    if sys.argv[1] == 'sign':
        draw_sign()
    elif sys.argv[1] == 'add':
        add(sys.argv[2], sys.argv[3], int(sys.argv[4]), int(sys.argv[5]))
