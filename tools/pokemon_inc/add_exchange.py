"""Pokémon Incorporated: add a complete Pokémon Exchange to a town.

    python3 add_exchange.py TOWN  (towns are configured in TOWNS below)

Does everything for one town:
  1. stamps the brown CASH building onto the town map (exchange_building.py)
  2. adds the door warp in the town and creates <Town>_PokemonExchange
  3. puts the trader, caged Pokémon and customers inside
  4. registers the map (appended to its map group, so no map ids move)
  5. moves the town's stock from the Pokémon Center to the Exchange
  6. removes the trader booth from that town's Pokémon Center
Safe to rerun.
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(__file__))
import exchange_building

R = os.path.expanduser('~/pokefirered')

TOWNS = {
    'ViridianCity': dict(tileset='viridian_city', layout='LAYOUT_VIRIDIAN_CITY', x=34, y=23,
                         section='MAPSEC_VIRIDIAN_CITY', mart='ViridianCity_Mart',
                         center='MAP_VIRIDIAN_CITY_POKEMON_CENTER_1F', large=False,
                         caged=['PIKACHU', 'MEOWTH', 'PSYDUCK', 'JIGGLYPUFF', 'CLEFAIRY', 'PIDGEY'],
                         customers=[('GENTLEMAN', 5, 5, 'Customer')]),
    'PewterCity': dict(tileset='pewter_city', layout='LAYOUT_PEWTER_CITY', x=26, y=8,
                       section='MAPSEC_PEWTER_CITY', mart='PewterCity_Mart',
                       center='MAP_PEWTER_CITY_POKEMON_CENTER_1F', large=False,
                       caged=['CLEFAIRY', 'NIDORAN_M', 'NIDORAN_F', 'SPEAROW', 'JIGGLYPUFF', 'PIDGEY'],
                       customers=[('HIKER', 5, 5, 'CustomerHiker')]),
    'CeruleanCity': dict(tileset='cerulean_city', layout='LAYOUT_CERULEAN_CITY', x=23, y=21,
                         section='MAPSEC_CERULEAN_CITY', mart='CeruleanCity_Mart',
                         center='MAP_CERULEAN_CITY_POKEMON_CENTER_1F', large=False,
                         caged=['PSYDUCK', 'SLOWPOKE', 'SEEL', 'MEOWTH', 'MACHOP', 'VOLTORB'],
                         customers=[('LASS', 5, 5, 'CustomerLass')]),
    'CeladonCity': dict(tileset='celadon_city', layout='LAYOUT_CELADON_CITY', x=26, y=27,
                        section='MAPSEC_CELADON_CITY', mart='CeladonCity_DepartmentStore_1F',
                        center='MAP_CELADON_CITY_POKEMON_CENTER_1F', large=True,
                        caged=['PIKACHU', 'MEOWTH', 'CHANSEY', 'KABUTO', 'OMANYTE', 'CUBONE',
                               'DODUO', 'VOLTORB', 'WIGGLYTUFF', 'SLOWPOKE', 'MACHOP', 'NIDORINO'],
                        customers=[('BEAUTY', 6, 6, 'CustomerBeauty'), ('FAT_MAN', 10, 3, 'CustomerFatMan')]),
}

SMALL_CAGES = [(7, 4), (8, 4), (7, 5), (8, 5), (10, 4), (10, 5)]
LARGE_CAGES = [(8, 4), (9, 4), (8, 5), (9, 5), (11, 4), (12, 4), (11, 5), (12, 5),
               (14, 4), (14, 5), (16, 4), (16, 5)]


def snake_upper(name):
    out = ''
    for i, c in enumerate(name):
        if c.isupper() and i and not name[i - 1].isupper():
            out += '_'
        out += c.upper()
    return out


def cage_script(species):
    return 'PokemonExchange_EventScript_Caged' + ''.join(p.capitalize() for p in species.split('_'))


def obj(gfx, x, y, script, mv='MOVEMENT_TYPE_FACE_DOWN', elev=3, rx=0, ry=0):
    return {"type": "object", "graphics_id": "OBJ_EVENT_GFX_" + gfx, "x": x, "y": y, "elevation": elev,
            "movement_type": mv, "movement_range_x": rx, "movement_range_y": ry,
            "trainer_type": "TRAINER_TYPE_NONE", "trainer_sight_or_berry_tree_id": "0",
            "script": script, "flag": "0"}


def jdump(path, data):
    json.dump(data, open(path, 'w'), indent=2, ensure_ascii=False)
    open(path, 'a').write('\n')


def add(town):
    cfg = TOWNS[town]
    name = town + '_PokemonExchange'
    mid = 'MAP_' + snake_upper(town) + '_POKEMON_EXCHANGE'

    # 1. building
    exchange_building.add(cfg['tileset'], cfg['layout'], cfg['x'], cfg['y'])
    door = (cfg['x'] + 2, cfg['y'] + 3)

    # 2. warp in town (arrive on the middle of the mat: dest warp 1)
    tp = f'{R}/data/maps/{town}/map.json'
    t = json.load(open(tp))
    if not any(w['dest_map'] == mid for w in t['warp_events']):
        t['warp_events'].append({"x": door[0], "y": door[1], "elevation": 0, "dest_map": mid, "dest_warp_id": "1"})
    jdump(tp, t)
    back = str([i for i, w in enumerate(t['warp_events']) if w['dest_map'] == mid][0])

    # 3. interior
    cages = LARGE_CAGES if cfg['large'] else SMALL_CAGES
    objects = [obj('POKE_MANIAC', 2, 3, 'EventScript_PokemonTrader', 'MOVEMENT_TYPE_FACE_RIGHT', 0)]
    for (x, y), species in zip(cages, cfg['caged']):
        facing = 'MOVEMENT_TYPE_FACE_DOWN' if y == 4 else 'MOVEMENT_TYPE_FACE_UP'
        objects.append(obj(species, x, y, cage_script(species), facing))
    for gfx, x, y, script in cfg['customers']:
        objects.append(obj(gfx, x, y, 'PokemonExchange_EventScript_' + script, 'MOVEMENT_TYPE_WANDER_AROUND', 3, 1, 1))
    os.makedirs(f'{R}/data/maps/{name}', exist_ok=True)
    jdump(f'{R}/data/maps/{name}/map.json', {
        "id": mid, "name": name,
        "layout": 'LAYOUT_POKEMON_EXCHANGE_LARGE' if cfg['large'] else 'LAYOUT_POKEMON_EXCHANGE',
        "music": "MUS_POKE_CENTER", "region_map_section": cfg['section'], "requires_flash": False,
        "weather": "WEATHER_NONE", "map_type": "MAP_TYPE_INDOOR", "allow_cycling": False,
        "allow_escaping": False, "allow_running": False, "show_map_name": False, "floor_number": 0,
        "battle_scene": "MAP_BATTLE_SCENE_NORMAL", "connections": None,
        "object_events": objects,
        "warp_events": [{"x": x, "y": 7, "elevation": 3, "dest_map": 'MAP_' + snake_upper(town), "dest_warp_id": back}
                        for x in (3, 4, 5)],
        "coord_events": [], "bg_events": []})
    sp = f'{R}/data/maps/{name}/scripts.inc'
    if not os.path.exists(sp):
        open(sp, 'w').write(f'{name}_MapScripts::\n\t.byte 0\n')
    tx = f'{R}/data/maps/{name}/text.inc'
    if not os.path.exists(tx):
        open(tx, 'w').write('')

    # 4. map group + includes
    gp = f'{R}/data/maps/map_groups.json'
    g = json.load(open(gp))
    for grp, maps in g.items():
        if isinstance(maps, list) and cfg['mart'] in maps and name not in maps:
            maps.append(name)
    jdump(gp, g)
    ep = f'{R}/data/event_scripts.s'
    s = open(ep).read()
    if f'data/maps/{name}/scripts.inc' not in s:
        s = s.replace(f'\t.include "data/maps/{cfg["mart"]}/scripts.inc"\n',
                      f'\t.include "data/maps/{cfg["mart"]}/scripts.inc"\n\t.include "data/maps/{name}/scripts.inc"\n')
        s = s.replace(f'\t.include "data/maps/{cfg["mart"]}/text.inc"\n',
                      f'\t.include "data/maps/{cfg["mart"]}/text.inc"\n\t.include "data/maps/{name}/text.inc"\n')
        open(ep, 'w').write(s)
        assert f'data/maps/{name}/scripts.inc' in s, 'include anchor not found'

    # 5. stock
    stp = f'{R}/src/data/pokemon_trader_stock.h'
    s = open(stp).read()
    s = s.replace(f'SHOP({cfg["center"]},', f'SHOP({mid},')
    open(stp, 'w').write(s)

    # 6. remove the Center booth
    cname = ''.join(p.capitalize() for p in cfg['center'][4:].lower().split('_'))
    for d in os.listdir(f'{R}/data/maps'):
        mp = f'{R}/data/maps/{d}/map.json'
        if os.path.exists(mp):
            m = json.load(open(mp))
            if m.get('id') == cfg['center']:
                m['object_events'] = [o for o in m['object_events'] if o['script'] != 'EventScript_PokemonTrader']
                jdump(mp, m)
    print(f'{town}: exchange {mid}, door {door}, back warp {back}')
    return [cage_script(s) for s in cfg['caged']], [c[3] for c in cfg['customers']]


if __name__ == '__main__':
    for town in sys.argv[1:]:
        add(town)
