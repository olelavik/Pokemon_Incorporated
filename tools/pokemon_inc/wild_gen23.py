"""Pokémon Incorporated: Gen 2/3 Pokémon in Kanto's wild tables.

    python3 tools/pokemon_inc/wild_gen23.py

Design doc: "Gen 2/3 Pokémon: where they live". Each change replaces one
encounter slot in a FireRed table (LeafGreen tables are left alone).

Slot chances, for reading the edits below:
  land     0-11: 20 20 10 10 10 10 5 5 4 4 1 1 (%)
  water    0-4:  60 30 5 4 1
  fishing  old rod 0-1: 70 30 | good rod 2-4: 60 20 20 | super rod 5-9: 40 40 15 4 1

Each edit: (table, slot, species, level) or (table, slot, species, min, max).
Rerunning is safe: the edits are absolute.
Feebas is not here: it only lives on six hidden tiles of Route 12
(src/wild_encounter.c, Feebas tiles).
"""
import json, os

R = os.path.join(os.path.dirname(__file__), '..', '..')

LAND, WATER, FISH = 'land_mons', 'water_mons', 'fishing_mons'

EDITS = {
    'Route1': [(LAND, 2, 'SENTRET', 3), (LAND, 6, 'SENTRET', 4), (LAND, 3, 'ZIGZAGOON', 3)],
    'Route22': [(LAND, 2, 'POOCHYENA', 4), (LAND, 7, 'POOCHYENA', 5)],
    'Route2': [(LAND, 4, 'WURMPLE', 3), (LAND, 5, 'HOOTHOOT', 3)],
    'ViridianForest': [(LAND, 4, 'LEDYBA', 4), (LAND, 5, 'SPINARAK', 4), (LAND, 7, 'SHROOMISH', 5)],
    'Route3': [(LAND, 4, 'TAILLOW', 6), (LAND, 6, 'SKITTY', 6), (LAND, 11, 'RALTS', 6)],
    'MtMoon1F': [(LAND, 3, 'NOSEPASS', 9), (LAND, 6, 'ARON', 8), (LAND, 10, 'SABLEYE', 9)],
    'MtMoonB2F': [(LAND, 3, 'NOSEPASS', 10), (LAND, 9, 'ARON', 10), (LAND, 10, 'SABLEYE', 11)],
    'Route24': [(LAND, 1, 'HOPPIP', 10), (LAND, 4, 'SUNKERN', 12), (LAND, 9, 'NATU', 10),
                (WATER, 1, 'MARILL', 10, 20)],
    'Route25': [(LAND, 1, 'HOPPIP', 10), (LAND, 4, 'SUNKERN', 12), (LAND, 9, 'NATU', 10),
                (WATER, 1, 'MARILL', 20, 25)],
    'Route5': [(LAND, 3, 'ELECTRIKE', 12), (LAND, 6, 'SNUBBULL', 14)],
    'Route6': [(LAND, 3, 'ELECTRIKE', 12), (LAND, 6, 'SNUBBULL', 14), (WATER, 1, 'LOTAD', 20, 25)],
    'Route9': [(LAND, 5, 'MAREEP', 14), (LAND, 6, 'MAKUHITA', 15)],
    'Route10': [(LAND, 4, 'MAREEP', 14), (LAND, 6, 'MAKUHITA', 15)],
    'RockTunnel1F': [(LAND, 2, 'WHISMUR', 16), (LAND, 6, 'WHISMUR', 17), (LAND, 7, 'ARON', 16),
                     (LAND, 10, 'MAWILE', 15)],
    'RockTunnelB1F': [(LAND, 2, 'WHISMUR', 17), (LAND, 7, 'ARON', 16), (LAND, 10, 'MAWILE', 15)],
    'Route11': [(LAND, 6, 'PLUSLE', 14), (LAND, 7, 'MINUN', 14)],
    'Route7': [(LAND, 7, 'HOUNDOUR', 19)],
    'Route8': [(LAND, 7, 'MURKROW', 18)],
    'DiglettsCaveB1F': [(LAND, 10, 'TRAPINCH', 18)],
    'Route12': [(LAND, 2, 'SEEDOT', 22), (LAND, 8, 'VOLBEAT', 25), (LAND, 10, 'ILLUMISE', 25),
                (FISH, 4, 'CORPHISH', 10, 20), (FISH, 6, 'BARBOACH', 20, 30)],
    'Route13': [(LAND, 2, 'SEEDOT', 22), (LAND, 8, 'VOLBEAT', 25), (LAND, 10, 'ILLUMISE', 25),
                (FISH, 4, 'CORPHISH', 10, 20), (FISH, 6, 'BARBOACH', 20, 30)],
    'Route14': [(LAND, 2, 'SEEDOT', 22)],
    'Route15': [(LAND, 2, 'SEEDOT', 22), (LAND, 8, 'VOLBEAT', 25), (LAND, 10, 'ILLUMISE', 25)],
    'Route16': [(LAND, 7, 'SWABLU', 22), (LAND, 10, 'ZANGOOSE', 20), (LAND, 11, 'SEVIPER', 22)],
    'Route17': [(LAND, 7, 'SWABLU', 26), (LAND, 10, 'ZANGOOSE', 25)],
    'Route18': [(LAND, 7, 'SWABLU', 26), (LAND, 10, 'SEVIPER', 25)],
    'SafariZoneCenter': [(LAND, 7, 'GIRAFARIG', 27), (LAND, 10, 'HERACROSS', 25), (WATER, 1, 'WOOPER', 20, 25)],
    'SafariZoneEast': [(LAND, 8, 'PHANPY', 24), (LAND, 10, 'HERACROSS', 25), (WATER, 1, 'WOOPER', 20, 25)],
    'SafariZoneNorth': [(LAND, 8, 'PHANPY', 24), (LAND, 10, 'TROPIUS', 26), (WATER, 1, 'WOOPER', 20, 25)],
    'SafariZoneWest': [(LAND, 8, 'GIRAFARIG', 25), (LAND, 10, 'TROPIUS', 26), (WATER, 1, 'WOOPER', 20, 25)],
    'PowerPlant': [(LAND, 4, 'ELECTRIKE', 24), (LAND, 8, 'MANECTRIC', 30), (LAND, 10, 'ELEKID', 25)],
    'SeafoamIslands1F': [(LAND, 3, 'SPHEAL', 24), (LAND, 4, 'SNORUNT', 24), (LAND, 10, 'SNEASEL', 28)],
    'SeafoamIslandsB1F': [(LAND, 3, 'SPHEAL', 26), (LAND, 4, 'SNORUNT', 26), (LAND, 10, 'SNEASEL', 30)],
    'SeafoamIslandsB2F': [(LAND, 4, 'SPHEAL', 26), (LAND, 5, 'SWINUB', 26)],
    'SeafoamIslandsB3F': [(LAND, 5, 'SPHEAL', 28), (LAND, 3, 'SNORUNT', 30)],
    'SeafoamIslandsB4F': [(LAND, 2, 'SPHEAL', 32), (LAND, 4, 'SWINUB', 32), (LAND, 10, 'SNEASEL', 34)],
    'CinnabarIsland': [(WATER, 2, 'WAILMER', 20, 30), (FISH, 5, 'CARVANHA', 15, 25)],
    'Route19': [(WATER, 2, 'WAILMER', 20, 30), (FISH, 6, 'CORSOLA', 15, 25)],
    'Route20': [(WATER, 2, 'WAILMER', 20, 30), (FISH, 6, 'CORSOLA', 15, 25)],
    'Route23': [(LAND, 4, 'GLIGAR', 34), (LAND, 10, 'ABSOL', 38), (LAND, 11, 'SKARMORY', 40)],
}

for floor in ('3F', '4F', '5F', '6F', '7F'):
    lv = {'3F': 15, '4F': 15, '5F': 15, '6F': 17, '7F': 18}[floor]
    EDITS['PokemonTower' + floor] = [(LAND, 2, 'SHUPPET', lv + 2), (LAND, 4, 'DUSKULL', lv),
                                     (LAND, 10, 'MISDREAVUS', lv + 2)]
for floor in ('1F', '2F', '3F', 'B1F'):
    EDITS['PokemonMansion' + floor] = [(LAND, 2, 'SLUGMA', 30), (LAND, 5, 'HOUNDOUR', 30),
                                       (LAND, 11, 'MAGBY', 30)]
for floor in ('1F', '2F', '3F'):
    EDITS['VictoryRoad' + floor] = [(LAND, 2, 'LAIRON', 40), (LAND, 7, 'SABLEYE', 40),
                                    (LAND, 8, 'MAWILE', 40), (LAND, 11, 'BAGON', 35)]


def main():
    path = os.path.join(R, 'src/data/wild_encounters.json')
    data = json.load(open(path))
    tables = {}
    for g in data['wild_encounter_groups']:
        for e in g['encounters']:
            if e['base_label'].endswith('_FireRed'):
                tables[e['base_label'][1:-len('_FireRed')]] = e
    count = 0
    for name, edits in EDITS.items():
        e = tables[name]
        for edit in edits:
            kind, slot, species = edit[:3]
            lo, hi = (edit[3], edit[3]) if len(edit) == 4 else edit[3:]
            e[kind]['mons'][slot] = {'min_level': lo, 'max_level': hi, 'species': 'SPECIES_' + species}
            count += 1
    with open(path, 'w') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write('\n')
    print(f'{count} slots set in {len(EDITS)} areas')


if __name__ == '__main__':
    main()
