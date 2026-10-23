"""Pokémon Incorporated: gym leader teams (design doc: "Gym leader teams").

    python3 tools/pokemon_inc/gym_teams.py

Rewrites the eight sParty_Leader* arrays in src/data/trainer_parties.h and
switches their trainers to ITEM_CUSTOM_MOVES. Safe to rerun.

IVs: 150 for the team (about 18/31), 200 for the ace (about 24/31).
Strong, not perfect. The ace's level is the level cap for that gym.
"""
import os, re

R = os.path.join(os.path.dirname(__file__), '..', '..')
IV, ACE_IV = 150, 200

# (species, level, item or None, [moves])
TEAMS = {
    'LeaderBrock': [
        ('GEODUDE', 12, None, ['TACKLE', 'DEFENSE_CURL', 'ROCK_THROW', 'MUD_SPORT']),
        ('SANDSHREW', 12, None, ['SCRATCH', 'DEFENSE_CURL', 'SAND_ATTACK', 'POISON_STING']),
        ('NOSEPASS', 13, None, ['TACKLE', 'HARDEN', 'BLOCK', 'ROCK_THROW']),
        ('ONIX', 15, 'ORAN_BERRY', ['ROCK_TOMB', 'BIND', 'SCREECH', 'HARDEN']),
    ],
    'LeaderMisty': [
        ('GOLDEEN', 18, None, ['HORN_ATTACK', 'SUPERSONIC', 'WATER_PULSE', 'TAIL_WHIP']),
        ('MARILL', 19, None, ['WATER_GUN', 'DEFENSE_CURL', 'ROLLOUT', 'TAIL_WHIP']),
        ('STARYU', 19, None, ['WATER_PULSE', 'RAPID_SPIN', 'HARDEN', 'SWIFT']),
        ('STARMIE', 22, 'SITRUS_BERRY', ['WATER_PULSE', 'SWIFT', 'RECOVER', 'RAPID_SPIN']),
    ],
    'LeaderLtSurge': [
        ('VOLTORB', 23, None, ['SPARK', 'SONIC_BOOM', 'SCREECH', 'SELF_DESTRUCT']),
        ('MAGNEMITE', 23, None, ['THUNDER_SHOCK', 'SONIC_BOOM', 'THUNDER_WAVE', 'SUPERSONIC']),
        ('ELECTRIKE', 24, None, ['SPARK', 'QUICK_ATTACK', 'HOWL', 'LEER']),
        ('PIKACHU', 24, None, ['SHOCK_WAVE', 'QUICK_ATTACK', 'DOUBLE_TEAM', 'SLAM']),
        ('RAICHU', 26, 'MAGNET', ['SHOCK_WAVE', 'BRICK_BREAK', 'QUICK_ATTACK', 'THUNDER_WAVE']),
    ],
    'LeaderErika': [
        ('TANGELA', 28, None, ['MEGA_DRAIN', 'BIND', 'STUN_SPORE', 'INGRAIN']),
        ('JUMPLUFF', 28, None, ['LEECH_SEED', 'BULLET_SEED', 'COTTON_SPORE', 'SYNTHESIS']),
        ('BRELOOM', 29, None, ['MACH_PUNCH', 'HEADBUTT', 'MEGA_DRAIN', 'LEECH_SEED']),
        ('VICTREEBEL', 30, None, ['RAZOR_LEAF', 'ACID', 'WRAP', 'STUN_SPORE']),
        ('VILEPLUME', 31, 'MIRACLE_SEED', ['GIGA_DRAIN', 'ACID', 'SLEEP_POWDER', 'MOONLIGHT']),
    ],
    'LeaderKoga': [
        ('QWILFISH', 38, None, ['SPIKES', 'PIN_MISSILE', 'WATER_PULSE', 'MINIMIZE']),
        ('ARIADOS', 38, None, ['TOXIC', 'NIGHT_SHADE', 'SPIDER_WEB', 'PIN_MISSILE']),
        ('MUK', 40, None, ['SLUDGE_BOMB', 'FIRE_PUNCH', 'MINIMIZE', 'DISABLE']),
        ('CROBAT', 41, None, ['WING_ATTACK', 'BITE', 'CONFUSE_RAY', 'MEAN_LOOK']),
        ('WEEZING', 43, 'LEFTOVERS', ['SLUDGE_BOMB', 'THUNDERBOLT', 'SMOKESCREEN', 'TOXIC']),
    ],
    'LeaderSabrina': [
        ('MR_MIME', 39, None, ['PSYCHIC', 'REFLECT', 'LIGHT_SCREEN', 'ENCORE']),
        ('GIRAFARIG', 40, None, ['PSYBEAM', 'CRUNCH', 'STOMP', 'AGILITY']),
        ('VENOMOTH', 40, None, ['PSYBEAM', 'SILVER_WIND', 'SLEEP_POWDER', 'SUPERSONIC']),
        ('ESPEON', 42, None, ['PSYCHIC', 'SHADOW_BALL', 'QUICK_ATTACK', 'MORNING_SUN']),
        ('ALAKAZAM', 44, 'TWISTED_SPOON', ['PSYCHIC', 'SHADOW_BALL', 'RECOVER', 'REFLECT']),
    ],
    'LeaderBlaine': [
        ('MAGCARGO', 44, None, ['FLAMETHROWER', 'ROCK_SLIDE', 'AMNESIA', 'HARDEN']),
        ('NINETALES', 45, None, ['FLAMETHROWER', 'WILL_O_WISP', 'CONFUSE_RAY', 'QUICK_ATTACK']),
        ('HOUNDOOM', 45, None, ['FLAMETHROWER', 'CRUNCH', 'SMOG', 'ROAR']),
        ('RAPIDASH', 45, None, ['FIRE_BLAST', 'STOMP', 'BOUNCE', 'AGILITY']),
        ('ARCANINE', 48, 'CHARCOAL', ['FLAMETHROWER', 'EXTREME_SPEED', 'IRON_TAIL', 'BITE']),
    ],
    'LeaderGiovanni': [
        ('DUGTRIO', 46, None, ['EARTHQUAKE', 'ROCK_SLIDE', 'SLASH', 'SAND_ATTACK']),
        ('STEELIX', 46, None, ['IRON_TAIL', 'DIG', 'ROCK_SLIDE', 'SCREECH']),
        ('DONPHAN', 47, None, ['EARTHQUAKE', 'ROLLOUT', 'TAKE_DOWN', 'DEFENSE_CURL']),
        ('NIDOQUEEN', 47, None, ['EARTHQUAKE', 'BODY_SLAM', 'ICE_BEAM', 'CRUNCH']),
        ('NIDOKING', 48, None, ['EARTHQUAKE', 'MEGAHORN', 'THUNDERBOLT', 'ICE_BEAM']),
        ('RHYDON', 50, 'SOFT_SAND', ['EARTHQUAKE', 'ROCK_SLIDE', 'TAKE_DOWN', 'SCARY_FACE']),
    ],
}


def party_c(name, team):
    out = [f'static const struct TrainerMonItemCustomMoves sParty_{name}[] = {{']
    for i, (sp, lvl, item, moves) in enumerate(team):
        iv = ACE_IV if i == len(team) - 1 else IV
        out += ['    {',
                f'        .iv = {iv},',
                f'        .lvl = {lvl},',
                f'        .species = SPECIES_{sp},',
                f'        .heldItem = ITEM_{item or "NONE"},',
                '        .moves = {' + ', '.join('MOVE_' + m for m in moves) + '},',
                '    },']
    out.append('};')
    return '\n'.join(out)


def main():
    pp = os.path.join(R, 'src/data/trainer_parties.h')
    s = open(pp).read()
    for name, team in TEAMS.items():
        pat = re.compile(r'static const struct TrainerMon\w+ sParty_' + name + r'\[\] = \{.*?\n\};', re.S)
        s, n = pat.subn(party_c(name, team), s, count=1)
        assert n == 1, name
    open(pp, 'w').write(s)

    tp = os.path.join(R, 'src/data/trainers.h')
    t = open(tp).read()
    for name in TEAMS:
        t, n = re.subn(r'\.party = \w+\(sParty_' + name + r'\)', f'.party = ITEM_CUSTOM_MOVES(sParty_{name})', t)
        assert n == 1, name
    open(tp, 'w').write(t)
    print('gym teams written')


if __name__ == '__main__':
    main()
