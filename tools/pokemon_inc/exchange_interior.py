"""Pokémon Incorporated: the Pokémon Exchange interior tileset and layout.

Creates data/tilesets/secondary/pokemon_exchange (a copy of the Poké Mart
tileset plus cage tiles) and data/layouts/PokemonExchange (the Mart floor plan
with the shelves replaced by cages). Cage bars live on the top tile layer with
the NORMAL layer type, so the game draws them above the caged Pokémon.

    python3 exchange_interior.py
"""
import os, shutil, struct, json
from PIL import Image

ROOT = os.path.expanduser('~/pokefirered')
MART = os.path.join(ROOT, 'data/tilesets/secondary/mart')
DST = os.path.join(ROOT, 'data/tilesets/secondary/pokemon_exchange')
CAGE_PAL = 10

# palette 10: index -> colour
PAL = [
    (255, 0, 255),   # 0 transparent
    (41, 41, 49),    # 1 iron shadow
    (82, 82, 98),    # 2 iron dark
    (139, 139, 156), # 3 iron mid
    (205, 205, 222), # 4 iron light
    (98, 66, 41),    # 5 wood dark
    (148, 98, 57),   # 6 wood
    (139, 106, 49),  # 7 straw shadow
    (197, 156, 74),  # 8 straw dark
    (230, 197, 106), # 9 straw
    (246, 230, 156), # 10 straw light
    (57, 41, 32),    # 11 back wall dark
    (205, 82, 65),   # 12 price tag red
    (255, 255, 255), # 13 white
    (0, 0, 0), (0, 0, 0),
]


def cage_pixels():
    """Return (bottom, top) 16x16 index grids for one cage.

    Dark inside so the Pokémon stands out, a strip of straw at the bottom,
    and only two thin bars so you can actually see who's in there.
    """
    bottom = [[11] * 16 for _ in range(16)]
    straw = [9, 8, 10, 9, 7, 9, 10, 8]
    for y in range(12, 16):
        for x in range(16):
            bottom[y][x] = straw[(x * 3 + y) % len(straw)]
    top = [[0] * 16 for _ in range(16)]
    for x in range(16):                      # frame top and bottom
        top[0][x] = 2
        top[15][x] = 1
    for y in range(16):                      # frame sides
        top[y][0] = 3
        top[y][15] = 1
    for bx in (5, 10):                       # two thin bars
        for y in range(1, 15):
            top[y][bx] = 4
    for y in range(1, 3):                    # tiny red price tag
        for x in range(12, 15):
            top[y][x] = 12
    return bottom, top


def build_tileset():
    if os.path.isdir(DST):
        shutil.rmtree(DST)
    shutil.copytree(MART, DST)
    for f in os.listdir(DST):
        if f.endswith('.4bpp') or f.endswith('.lz'):
            os.remove(os.path.join(DST, f))

    with open(os.path.join(DST, 'palettes', '%02d.pal' % CAGE_PAL), 'w') as fp:
        fp.write('JASC-PAL\r\n0100\r\n16\r\n')
        for c in PAL:
            fp.write('%d %d %d\r\n' % c)

    tiles = Image.open(os.path.join(DST, 'tiles.png'))
    n_old = (tiles.size[0] // 8) * (tiles.size[1] // 8)
    new = Image.new('P', (128, tiles.size[1] + 8))
    new.putpalette(tiles.getpalette())
    new.paste(tiles, (0, 0))
    px = new.load()
    bottom, top = cage_pixels()
    first = n_old
    # 8 tiles: bottom TL TR BL BR, top TL TR BL BR
    for k, grid in enumerate((bottom, top)):
        for q in range(4):
            tid = first + k * 4 + q
            tx, ty = (tid % 16) * 8, (tid // 16) * 8
            ox, oy = (q % 2) * 8, (q // 2) * 8
            for y in range(8):
                for x in range(8):
                    px[tx + x, ty + y] = grid[oy + y][ox + x]
    new.save(os.path.join(DST, 'tiles.png'))

    meta_path = os.path.join(DST, 'metatiles.bin')
    attr_path = os.path.join(DST, 'metatile_attributes.bin')
    n_meta = os.path.getsize(meta_path) // 16
    entries = [640 + first + q | (CAGE_PAL << 12) for q in range(4)] + \
              [640 + first + 4 + q | (CAGE_PAL << 12) for q in range(4)]
    open(meta_path, 'ab').write(struct.pack('<8H', *entries))
    open(attr_path, 'ab').write(struct.pack('<I', 0))   # behavior 0, layer type NORMAL
    cage_meta = 640 + n_meta
    print('cage tiles at', 640 + first, 'cage metatile', cage_meta)
    return cage_meta


def build_layout(cage):
    src = os.path.join(ROOT, 'data/layouts/Mart')
    dst = os.path.join(ROOT, 'data/layouts/PokemonExchange')
    os.makedirs(dst, exist_ok=True)
    shutil.copy(os.path.join(src, 'border.bin'), os.path.join(dst, 'border.bin'))
    b = bytearray(open(os.path.join(src, 'map.bin'), 'rb').read())
    w, h = 11, 9
    FLOOR = 641

    def put(x, y, meta, solid):
        struct.pack_into('<H', b, (y * w + x) * 2, meta | ((1 if solid else 0) << 10) | (3 << 12))

    # remove the shelves on the right (cols 7, 8, 10, rows 3-6) -> floor
    for x in (7, 8, 10):
        for y in range(3, 7):
            put(x, y, FLOOR, False)
    # cages: a 2x2 stack (cols 7-8, rows 4-5) and a column on the wall (col 10, rows 4-5)
    for (x, y) in [(7, 4), (8, 4), (7, 5), (8, 5), (10, 4), (10, 5)]:
        put(x, y, cage, True)
    open(os.path.join(dst, 'map.bin'), 'wb').write(b)

    lp = os.path.join(ROOT, 'data/layouts/layouts.json')
    data = json.load(open(lp))
    if not any(l.get('id') == 'LAYOUT_POKEMON_EXCHANGE' for l in data['layouts']):
        data['layouts'].append({
            'id': 'LAYOUT_POKEMON_EXCHANGE',
            'name': 'PokemonExchange_Layout',
            'width': w, 'height': h, 'border_width': 2, 'border_height': 2,
            'primary_tileset': 'gTileset_Building',
            'secondary_tileset': 'gTileset_PokemonExchange',
            'border_filepath': 'data/layouts/PokemonExchange/border.bin',
            'blockdata_filepath': 'data/layouts/PokemonExchange/map.bin',
        })
        json.dump(data, open(lp, 'w'), indent=2, ensure_ascii=False)
        open(lp, 'a').write('\n')
    print('layout written')


def register_tileset():
    g = os.path.join(ROOT, 'src/data/tilesets/graphics.h')
    s = open(g).read()
    if 'gTilesetTiles_PokemonExchange' not in s:
        s += '\nconst u32 gTilesetTiles_PokemonExchange[] = INCBIN_U32("data/tilesets/secondary/pokemon_exchange/tiles.4bpp.lz");\n\n'
        s += 'const u16 gTilesetPalettes_PokemonExchange[][16] =\n{\n'
        for i in range(16):
            s += '\tINCBIN_U16("data/tilesets/secondary/pokemon_exchange/palettes/%02d.gbapal"),\n' % i
        s += '};\n'
        open(g, 'w').write(s)
    m = os.path.join(ROOT, 'src/data/tilesets/metatiles.h')
    s = open(m).read()
    if 'gMetatiles_PokemonExchange' not in s:
        s += '\nconst u16 gMetatiles_PokemonExchange[] = INCBIN_U16("data/tilesets/secondary/pokemon_exchange/metatiles.bin");\n'
        s += 'const u32 gMetatileAttributes_PokemonExchange[] = INCBIN_U32("data/tilesets/secondary/pokemon_exchange/metatile_attributes.bin");\n'
        open(m, 'w').write(s)
    hd = os.path.join(ROOT, 'src/data/tilesets/headers.h')
    s = open(hd).read()
    if 'gTileset_PokemonExchange' not in s:
        s += '''
const struct Tileset gTileset_PokemonExchange =
{
    .isCompressed = TRUE,
    .isSecondary = TRUE,
    .tiles = gTilesetTiles_PokemonExchange,
    .palettes = gTilesetPalettes_PokemonExchange,
    .metatiles = gMetatiles_PokemonExchange,
    .metatileAttributes = gMetatileAttributes_PokemonExchange,
    .callback = NULL,
};
'''
        open(hd, 'w').write(s)
    print('tileset registered')


def build_layout_large(cage):
    """Celadon flagship: the Mart widened by 6 columns, full of cages."""
    src = os.path.join(ROOT, 'data/layouts/Mart')
    dst = os.path.join(ROOT, 'data/layouts/PokemonExchangeLarge')
    os.makedirs(dst, exist_ok=True)
    shutil.copy(os.path.join(src, 'border.bin'), os.path.join(dst, 'border.bin'))
    b = open(os.path.join(src, 'map.bin'), 'rb').read()
    w0, h, extra = 11, 9, 6
    cells = [[struct.unpack_from('<H', b, (y * w0 + x) * 2)[0] for x in range(w0)] for y in range(h)]
    FLOOR = 641
    rows = []
    for y in range(h):
        filler = cells[y][6]                  # plain wall / floor / bottom edge column
        rows.append(cells[y][:7] + [filler] * extra + cells[y][7:])
    w = w0 + extra
    def put(x, y, meta, solid):
        rows[y][x] = meta | ((1 if solid else 0) << 10) | (3 << 12)
    # clear the old shelves (now at cols 13, 14, 16)
    for x in (13, 14, 16):
        for y in range(3, 7):
            put(x, y, FLOOR, False)
    # cage rows: three 2x2 stacks plus a wall column
    cages = [(8, 4), (9, 4), (8, 5), (9, 5),
             (11, 4), (12, 4), (11, 5), (12, 5),
             (14, 4), (15, 4), (14, 5), (15, 5),
             (16, 4), (16, 5)]
    # keep col 16 separate from 15: make 15 a walkway instead
    cages = [c for c in cages if c[0] != 15]
    for (x, y) in cages:
        put(x, y, cage, True)
    out = bytearray()
    for y in range(h):
        for x in range(w):
            out += struct.pack('<H', rows[y][x])
    open(os.path.join(dst, 'map.bin'), 'wb').write(out)
    lp = os.path.join(ROOT, 'data/layouts/layouts.json')
    data = json.load(open(lp))
    if not any(l.get('id') == 'LAYOUT_POKEMON_EXCHANGE_LARGE' for l in data['layouts']):
        data['layouts'].append({
            'id': 'LAYOUT_POKEMON_EXCHANGE_LARGE',
            'name': 'PokemonExchangeLarge_Layout',
            'width': w, 'height': h, 'border_width': 2, 'border_height': 2,
            'primary_tileset': 'gTileset_Building',
            'secondary_tileset': 'gTileset_PokemonExchange',
            'border_filepath': 'data/layouts/PokemonExchangeLarge/border.bin',
            'blockdata_filepath': 'data/layouts/PokemonExchangeLarge/map.bin',
        })
        json.dump(data, open(lp, 'w'), indent=2, ensure_ascii=False)
        open(lp, 'a').write('\n')
    print('large layout written, cages at', cages)


if __name__ == '__main__':
    cage = build_tileset()
    build_layout(cage)
    build_layout_large(cage)
    register_tileset()
