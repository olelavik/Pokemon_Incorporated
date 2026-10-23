"""Pokémon Incorporated: trainer teams follow wealth (design doc: "Wealth tiers").

    tools/pokemon_inc/build_trainers.sh   (regenerates all trainer changes)

NOT idempotent: levels go up every run, so don't run it on its own;
build_trainers.sh starts from vanilla data and runs every step in order.

Only trainers used in the game (data/scripts/trainers.inc and map scripts) are touched; leaders, Elite Four,
champion, rivals, Rocket, bosses and executives are handled elsewhere.

Tier by the class's prize factor (src/battle_main.c, gTrainerMoneyTable):
  broke    1-5    levels as vanilla; about 1 in 3 Pokémon swapped for a
                  Gen 2/3 Pokémon of the same niche
  working  6-12   +1 level; a lone Pokémon gets a partner from badge 2 on
  well off 13-25  +2 levels; filled up to 3-4 Pokémon from Exchange stock;
                  a berry on the lead
  the 1%   50     +3 levels; up to 4 Pokémon with premium Exchange stock;
                  everyone holds an item
Parties with custom movesets keep their species (the moves are tuned for
them); they only get the level, item and size rules where safe.
"""
import os, re, sys
sys.path.insert(0, os.path.dirname(__file__))
from partylib import set_party

R = os.path.join(os.path.dirname(__file__), '..', '..')

SKIP_CLASSES = {'LEADER', 'ELITE_FOUR', 'CHAMPION', 'RIVAL_EARLY', 'RIVAL_LATE', 'TEAM_ROCKET',
                'BOSS', 'ROCKET_EXEC', 'AQUA_ADMIN', 'PKMN_PROF', 'PLAYER'}

# Same niche, similar power: what a poor trainer might have caught instead.
SWAP = {
    'RATTATA': 'ZIGZAGOON', 'RATICATE': 'LINOONE', 'PIDGEY': 'TAILLOW', 'SPEAROW': 'TAILLOW',
    'CATERPIE': 'WURMPLE', 'WEEDLE': 'WURMPLE', 'METAPOD': 'SILCOON', 'KAKUNA': 'CASCOON',
    'ODDISH': 'SUNKERN', 'BELLSPROUT': 'HOPPIP', 'GEODUDE': 'NOSEPASS', 'JIGGLYPUFF': 'SKITTY',
    'MEOWTH': 'SNUBBULL', 'SANDSHREW': 'PHANPY', 'MANKEY': 'MAKUHITA', 'MACHOP': 'MAKUHITA',
    'PSYDUCK': 'MARILL', 'POLIWAG': 'LOTAD', 'POLIWHIRL': 'LOMBRE', 'GOLDEEN': 'BARBOACH',
    'SEAKING': 'WHISCASH', 'TENTACOOL': 'WAILMER', 'SHELLDER': 'CORSOLA', 'STARYU': 'CORSOLA',
    'KRABBY': 'CORPHISH', 'KINGLER': 'CRAWDAUNT', 'PIKACHU': 'PLUSLE', 'VOLTORB': 'ELECTRIKE',
    'GASTLY': 'SHUPPET', 'HAUNTER': 'BANETTE', 'VENONAT': 'SPINARAK', 'PARAS': 'SHROOMISH',
    'DODUO': 'SWABLU', 'SEEL': 'SPHEAL', 'DEWGONG': 'SEALEO', 'ARBOK': 'SEVIPER',
    'EKANS': 'POOCHYENA', 'GROWLITHE': 'HOUNDOUR',
}

# What money buys at the Exchange, by level band.
WELL_OFF_STOCK = [
    (0, ['EEVEE', 'PONYTA', 'VULPIX', 'GROWLITHE', 'PIKACHU', 'SKITTY']),
    (15, ['EEVEE', 'PONYTA', 'GROWLITHE', 'VULPIX', 'MAREEP', 'SNUBBULL']),
    (30, ['PERSIAN', 'RAPIDASH', 'NINETALES', 'GRANBULL', 'FLAAFFY', 'DELCATTY']),
    (40, ['ARCANINE', 'JOLTEON', 'FLAREON', 'VAPOREON', 'AMPHAROS', 'RAPIDASH']),
]
PREMIUM_STOCK = [
    (0, ['DRATINI', 'EEVEE', 'PONYTA']),
    (20, ['DRATINI', 'SCYTHER', 'EEVEE']),
    (35, ['DRAGONAIR', 'SCYTHER', 'LAPRAS', 'ESPEON']),
    (45, ['DRAGONAIR', 'LAPRAS', 'ARCANINE', 'SKARMORY']),
]
ITEMS_1PCT = ['SITRUS_BERRY', 'QUICK_CLAW', 'SILK_SCARF', 'LUM_BERRY']


def pick(stock, lvl, n):
    band = [s for lo, s in stock if lvl >= lo][-1]
    return band[n % len(band)]


def tier(factor):
    if factor <= 5:
        return 'broke'
    if factor <= 12:
        return 'working'
    if factor <= 25:
        return 'well_off'
    return 'one_pct'


def parse_party(parties_src, name):
    arr = re.search(r'struct (\w+) ' + name + r'\[\] = \{(.*?)\n\};', parties_src, re.S)
    mons = []
    for block in re.findall(r'\{(.*?)\n    \}', arr.group(2), re.S):
        mon = {'iv': int(re.search(r'\.iv = (\d+)', block).group(1)),
               'lvl': int(re.search(r'\.lvl = (\d+)', block).group(1)),
               'species': re.search(r'\.species = SPECIES_(\w+)', block).group(1)}
        item = re.search(r'\.heldItem = ITEM_(\w+)', block)
        mon['item'] = item.group(1) if item and item.group(1) != 'NONE' else None
        moves = re.search(r'\.moves = \{([^}]*)\}', block)
        mon['moves'] = [x.strip()[5:] for x in moves.group(1).split(',')] if moves else None
        mons.append(mon)
    return mons


def main():
    tp = os.path.join(R, 'src/data/trainers.h')
    pp = os.path.join(R, 'src/data/trainer_parties.h')
    t, p = open(tp).read(), open(pp).read()
    money = dict((c, int(v)) for c, v in
                 re.findall(r'\{TRAINER_CLASS_(\w+), (\d+)\}', open(os.path.join(R, 'src/battle_main.c')).read()))
    used = set(re.findall(r'\b(TRAINER_\w+)', open(os.path.join(R, 'data/scripts/trainers.inc')).read()))
    for root, _, files in os.walk(os.path.join(R, 'data/maps')):
        for f in files:
            if f == 'scripts.inc':
                used |= set(re.findall(r'\b(TRAINER_\w+)', open(os.path.join(root, f)).read()))

    counts = {}
    for i, m in enumerate(re.finditer(r'\[(TRAINER_\w+)\] = \{\s*\.trainerClass = TRAINER_CLASS_(\w+),.*?'
                                      r'\.party = \w+\((\w+)\)', t, re.S)):
        trainer, cls, party = m.groups()
        if trainer not in used or cls in SKIP_CLASSES or cls not in money:
            continue
        tr = tier(money[cls])
        mons = parse_party(p, party)
        custom = mons[0]['moves'] is not None
        top = max(x['lvl'] for x in mons)
        n = i

        if tr == 'broke':
            if not custom:
                for j, mon in enumerate(mons):
                    if (n + j) % 3 == 0 and mon['species'] in SWAP:
                        mon['species'] = SWAP[mon['species']]
        elif tr == 'working':
            for mon in mons:
                mon['lvl'] += 1
            if len(mons) == 1 and top >= 15 and not custom:
                partner = SWAP.get(mons[0]['species'], mons[0]['species'])
                mons.insert(0, {'species': partner, 'lvl': mons[0]['lvl'] - 2, 'iv': mons[0]['iv'],
                                'item': None, 'moves': None})
        elif tr == 'well_off':
            for mon in mons:
                mon['lvl'] += 2
            if not custom:
                want = 3 if top < 30 else 4
                k = 0
                while len(mons) < want:
                    mons.append({'species': pick(WELL_OFF_STOCK, top + 2, n + k), 'lvl': top + 1,
                                 'iv': mons[0]['iv'], 'item': None, 'moves': None})
                    k += 1
            mons[0]['item'] = mons[0]['item'] or ('SITRUS_BERRY' if top >= 30 else 'ORAN_BERRY')
        else:  # one_pct
            for mon in mons:
                mon['lvl'] += 3
            if not custom:
                k = 0
                while len(mons) < 4:
                    mons.append({'species': pick(PREMIUM_STOCK, top + 3, n + k), 'lvl': top + 3,
                                 'iv': 200, 'item': None, 'moves': None})
                    k += 1
            for j, mon in enumerate(mons):
                mon['item'] = mon['item'] or ITEMS_1PCT[j % len(ITEMS_1PCT)]
        for mon in mons:
            mon['lvl'] = min(mon['lvl'], 100)
        t, p = set_party(t, p, trainer, mons)
        counts[tr] = counts.get(tr, 0) + 1

    open(tp, 'w').write(t)
    open(pp, 'w').write(p)
    print('wealth tiers applied:', counts)


if __name__ == '__main__':
    main()
