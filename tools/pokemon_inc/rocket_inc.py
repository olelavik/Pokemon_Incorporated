"""Pokémon Incorporated: Team Rocket as a corporation (design doc: "Rocket Inc.").

    python3 tools/pokemon_inc/rocket_inc.py

Rewrites the Kanto Rocket grunts' teams by department and turns four of
them into executives (ROCKET EXEC class, a repurposed unused RS class).
Grunt team sizes and levels follow vanilla, +1 level ("working" wealth tier).
Sevii Rockets (grunts 42-51, admins) are left alone for now.
Safe to rerun: every team is written in full.
"""
import os, sys
sys.path.insert(0, os.path.dirname(__file__))
from partylib import set_party, set_field

R = os.path.join(os.path.dirname(__file__), '..', '..')
G = 'TRAINER_TEAM_ROCKET_GRUNT'

# Sales: Mt. Moon, Cerulean, Route 24. Cheap Pokémon, sold on.
SALES = {
    G: [('RATTATA', 14), ('POOCHYENA', 14)],
    G + '_2': [('MEOWTH', 12), ('ZUBAT', 12), ('RATTATA', 12)],
    G + '_3': [('EKANS', 12), ('POOCHYENA', 12)],
    G + '_5': [('MEOWTH', 18), ('MACHOP', 18)],
    G + '_6': [('EKANS', 16), ('MEOWTH', 16)],
}
# Casino & Logistics: Game Corner and the Rocket Hideout. Movers and guard dogs.
LOGISTICS = {
    G + '_7': [('RATICATE', 21), ('HOUNDOUR', 21)],
    G + '_8': [('DROWZEE', 22), ('MACHOP', 22)],
    G + '_9': [('RATICATE', 22), ('POOCHYENA', 22)],
    G + '_10': [('GRIMER', 21), ('KOFFING', 21), ('HOUNDOUR', 21)],
    G + '_11': [('RATTATA', 20), ('RATICATE', 20), ('POOCHYENA', 20), ('HOUNDOUR', 20)],
    G + '_12': [('GRIMER', 23), ('KOFFING', 23)],
    G + '_13': [('ZUBAT', 18), ('KOFFING', 18), ('GRIMER', 18), ('POOCHYENA', 18), ('RATICATE', 18)],
    G + '_14': [('MACHOP', 21), ('DROWZEE', 21), ('RATICATE', 21)],
    G + '_15': [('MACHOP', 22), ('MAKUHITA', 22)],
    G + '_16': [('SANDSHREW', 24), ('EKANS', 24), ('SANDSLASH', 24)],
    G + '_17': [('EKANS', 24), ('HOUNDOUR', 24), ('ARBOK', 24)],
}
# Security: Pokémon Tower.
SECURITY = {
    G + '_19': [('GOLBAT', 26), ('MURKROW', 26), ('POOCHYENA', 26)],
    G + '_20': [('MIGHTYENA', 27), ('KOFFING', 27)],
    G + '_22': [('DROWZEE', 27), ('KOFFING', 27)],
}
# Hostile takeover: Silph Co. Machines from R&D, dogs from Security.
TAKEOVER = {
    G + '_23': [('CUBONE', 30), ('ELECTRIKE', 30)],
    G + '_24': [('GOLBAT', 26), ('ZUBAT', 26), ('MAGNEMITE', 26), ('RATICATE', 26), ('VOLTORB', 26)],
    G + '_25': [('RATICATE', 29), ('HYPNO', 29), ('MIGHTYENA', 29)],
    G + '_26': [('MACHOKE', 30), ('HOUNDOUR', 30)],
    G + '_27': [('ARBOK', 29), ('ELECTRIKE', 29), ('CUBONE', 29)],
    G + '_28': [('SNEASEL', 33), ('ARBOK', 34)],
    G + '_29': [('MAGNETON', 33), ('HYPNO', 34)],
    G + '_30': [('MACHOKE', 30), ('MANECTRIC', 30)],
    G + '_31': [('GOLBAT', 29), ('MURKROW', 29), ('GOLBAT', 29)],
    G + '_32': [('RATICATE', 27), ('ARBOK', 27), ('KOFFING', 27), ('MIGHTYENA', 27)],
    G + '_33': [('CUBONE', 30), ('MAROWAK', 30)],
    G + '_34': [('SANDSLASH', 30), ('HOUNDOUR', 30)],
    G + '_35': [('RATICATE', 27), ('GOLBAT', 27), ('VOLTORB', 27), ('MAGNEMITE', 27)],
    G + '_36': [('WEEZING', 29), ('GOLBAT', 29), ('KOFFING', 29)],
    G + '_37': [('HYPNO', 29), ('GRIMER', 29), ('MACHOKE', 29)],
    G + '_38': [('GOLBAT', 29), ('SNEASEL', 29), ('HYPNO', 29)],
    G + '_39': [('HOUNDOOM', 33), ('MACHOKE', 34)],
    G + '_40': [('RATICATE', 33), ('ARBOK', 33), ('MIGHTYENA', 33)],
}


def m(species, lvl, moves, item=None, iv=120):
    return {'species': species, 'lvl': lvl, 'moves': moves, 'item': item, 'iv': iv}


# Executives: mini-bosses with smart movesets, one held item, better IVs.
EXECS = {
    G + '_4': ('VP SALES', [  # Mt. Moon B2F
        m('MEOWTH', 13, ['PAY_DAY', 'BITE', 'GROWL', 'SCRATCH']),
        m('ZUBAT', 13, ['LEECH_LIFE', 'SUPERSONIC', 'ASTONISH', 'BITE']),
        m('EKANS', 14, ['WRAP', 'POISON_STING', 'BITE', 'LEER'], 'ORAN_BERRY', 160),
    ]),
    G + '_18': ('LOGISTICS', [  # Rocket Hideout B4F, holds the Lift Key
        m('MACHOKE', 26, ['KARATE_CHOP', 'SEISMIC_TOSS', 'FORESIGHT', 'LEER']),
        m('WEEZING', 26, ['SLUDGE', 'SMOKESCREEN', 'SELF_DESTRUCT', 'TACKLE']),
        m('HOUNDOUR', 27, ['BITE', 'EMBER', 'ROAR', 'SMOG']),
        m('RATICATE', 28, ['HYPER_FANG', 'QUICK_ATTACK', 'SCARY_FACE', 'PURSUIT'], 'ORAN_BERRY', 160),
    ]),
    G + '_21': ('SECURITY', [  # Pokémon Tower 7F
        m('GOLBAT', 28, ['WING_ATTACK', 'BITE', 'CONFUSE_RAY', 'LEECH_LIFE']),
        m('MURKROW', 28, ['PURSUIT', 'WING_ATTACK', 'NIGHT_SHADE', 'HAZE']),
        m('ARBOK', 29, ['BITE', 'POISON_FANG', 'GLARE', 'WRAP']),
        m('MIGHTYENA', 30, ['BITE', 'TAKE_DOWN', 'ROAR', 'SCARY_FACE'], 'BLACK_GLASSES', 160),
    ]),
    G + '_41': ('CFO', [  # Silph Co. 11F
        m('MAGNETON', 38, ['THUNDERBOLT', 'SONIC_BOOM', 'THUNDER_WAVE', 'SUPERSONIC']),
        m('WEEZING', 38, ['SLUDGE_BOMB', 'THUNDERBOLT', 'SMOKESCREEN', 'EXPLOSION']),
        m('HOUNDOOM', 39, ['FLAMETHROWER', 'CRUNCH', 'ROAR', 'SMOG']),
        m('SNEASEL', 39, ['FAINT_ATTACK', 'ICE_PUNCH', 'QUICK_ATTACK', 'SCREECH']),
        m('PERSIAN', 40, ['PAY_DAY', 'SLASH', 'FAINT_ATTACK', 'SCREECH'], 'AMULET_COIN', 180),
    ]),
}


def main():
    tp = os.path.join(R, 'src/data/trainers.h')
    pp = os.path.join(R, 'src/data/trainer_parties.h')
    t, p = open(tp).read(), open(pp).read()
    for dept in (SALES, LOGISTICS, SECURITY, TAKEOVER):
        for trainer, mons in dept.items():
            t, p = set_party(t, p, trainer, mons, default_iv=40)
    for trainer, (name, mons) in EXECS.items():
        t, p = set_party(t, p, trainer, mons)
        t = set_field(t, trainer, 'trainerClass', 'TRAINER_CLASS_ROCKET_EXEC')
        t = set_field(t, trainer, 'trainerName', f'_("{name}")')
        t = set_field(t, trainer, 'items', '{ITEM_SUPER_POTION}')
    open(tp, 'w').write(t)
    open(pp, 'w').write(p)
    print(f'Rocket Inc.: {sum(map(len, (SALES, LOGISTICS, SECURITY, TAKEOVER)))} grunts, {len(EXECS)} executives')


if __name__ == '__main__':
    main()
