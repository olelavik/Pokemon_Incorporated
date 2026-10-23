"""Pokémon Incorporated economy model. All prices are badge-0 base prices;
the game multiplies them by the Kanto Price Index for the badges you hold."""
import json, math, os
INDEX = [100, 120, 150, 200, 260, 340, 440, 570, 750]          # % by badges held

BASE = {  # badge-0 prices (index applied in game)
    'POKE_BALL': 100, 'GREAT_BALL': 200, 'ULTRA_BALL': 500,
    'POTION': 100, 'SUPER_POTION': 250, 'HYPER_POTION': 600, 'MAX_POTION': 1200,
    'FULL_RESTORE': 1500, 'REVIVE': 500, 'FULL_HEAL': 300,
    'ANTIDOTE': 50, 'PARALYZE_HEAL': 60, 'AWAKENING': 80, 'BURN_HEAL': 80, 'ICE_HEAL': 80,
    'REPEL': 100, 'SUPER_REPEL': 200, 'MAX_REPEL': 300, 'ESCAPE_ROPE': 250, 'ETHER': 400,
    'CENTER_HEAL': 200, 'GYM_FEE': 500,
}
LEAGUE_FEE = 100_000        # flat, every attempt
LEAGUE_JACKPOT = 500_000    # flat, on becoming Champion

prize = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'prize.json')))
VANILLA = prize['seg']      # prize money per story stretch with vanilla wages

# a typical stretch: Center visits, balls, healing items by tier, gym fee
POTION_TIER = ['POTION', 'POTION', 'SUPER_POTION', 'SUPER_POTION', 'HYPER_POTION',
               'HYPER_POTION', 'MAX_POTION', 'MAX_POTION']
BALL_TIER = ['POKE_BALL', 'POKE_BALL', 'POKE_BALL', 'GREAT_BALL', 'GREAT_BALL',
             'ULTRA_BALL', 'ULTRA_BALL', 'ULTRA_BALL']
def basket(seg):              # seg 0..7, badges held during it = seg
    i = INDEX[seg] / 100
    cost = 10 * BASE['CENTER_HEAL'] + 10 * BASE[BALL_TIER[seg]] + 8 * BASE[POTION_TIER[seg]] \
         + 2 * BASE['REVIVE'] * (seg >= 3) + 3 * BASE['REPEL'] + BASE['GYM_FEE']
    return round(cost * i)

# How tight should each stretch feel? prize / cost of living
TARGET = [0.45, 0.65, 0.9, 1.0, 1.05, 1.1, 1.15, 1.1]

if __name__ == '__main__':
    wages = []
    print(f"{'stretch':10}{'index':>7}{'basket':>9}{'vanilla':>9}{'need x':>8}")
    for s in range(8):
        need = TARGET[s] * basket(s) / VANILLA[s]
        wages.append(need)
        print(f"{s+1:<10}{INDEX[s]:>6}%{basket(s):>9,}{VANILLA[s]:>9,}{need:>8.2f}")
    print('E4 vanilla prize', prize['league'])

def survival(seg):           # must-pay costs: heals, healing items, revives, repels, gym fee
    i = INDEX[seg] / 100
    cost = 10 * BASE['CENTER_HEAL'] + 8 * BASE[POTION_TIER[seg]] \
         + 2 * BASE['REVIVE'] * (seg >= 3) + 3 * BASE['REPEL'] + BASE['GYM_FEE']
    return round(cost * i)

WAGE = [100, 100, 100, 110, 130, 160, 200, 260, 330]   # % prize money by badges held

def report():
    print(f"\n{'str':4}{'idx':>6}{'wage':>6}{'survival':>10}{'prize':>9}{'girl':>8}{'ratio':>7}{'girl':>6}{'balls 10x':>10}")
    for s in range(8):
        p = round(VANILLA[s] * WAGE[s] / 100)
        g = p - p // 4
        print(f"{s+1:<4}{INDEX[s]:>5}%{WAGE[s]:>5}%{survival(s):>10,}{p:>9,}{g:>8,}{p/survival(s):>7.2f}{g/survival(s):>6.2f}"
              f"{round(10*BASE[BALL_TIER[s]]*INDEX[s]/100):>10,}")
    e4 = 0
    print('League: fee', LEAGUE_FEE, 'jackpot', LEAGUE_JACKPOT, '-> worth', LEAGUE_JACKPOT // LEAGUE_FEE, 'attempts')

report()

TRAINERS = [8, 13, 21, 33, 24, 44, 15, 28]
def survival(seg):
    i = INDEX[seg] / 100
    n = TRAINERS[seg]
    heals = max(4, round(n / 3))        # a Center visit every ~3 battles
    potions = round(n / 2)              # one healing item every other battle
    cost = heals * BASE['CENTER_HEAL'] + potions * BASE[POTION_TIER[seg]] \
         + round(n / 8) * BASE['REVIVE'] * (seg >= 3) + 3 * BASE['REPEL'] + BASE['GYM_FEE']
    return round(cost * i)

print('\n--- survival scaled by number of trainers ---')
report()

BASE['CENTER_HEAL'] = 300
BASE['GYM_FEE'] = 800
TARGET = [0.7, 0.9, 1.2, 1.2, 1.1, 1.15, 1.2, 1.0]
need = [TARGET[s] * survival(s) / VANILLA[s] * 100 for s in range(8)]
print('\nneeded wage % per stretch:', [round(x) for x in need])

WAGE = [85, 85, 85, 100, 140, 180, 215, 300, 330]
print('\n--- final ---')
report()
for s in range(8):
    pass
print('\nPrices at each badge count:')
for k in ['POKE_BALL','GREAT_BALL','ULTRA_BALL','POTION','SUPER_POTION','HYPER_POTION','FULL_RESTORE','REVIVE','CENTER_HEAL','GYM_FEE']:
    print(f"{k:14}", ' '.join(f"{round(BASE[k]*i/100):>6,}" for i in INDEX))
