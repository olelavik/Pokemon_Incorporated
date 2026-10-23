"""Pokémon Incorporated: every trainer gets the boss AI (design doc: "Battle overhaul").

    python3 tools/pokemon_inc/smart_ai.py

Vanilla gave CHECK_VIABILITY and TRY_TO_FAINT to 68 bosses; everyone else only
avoided useless moves. Safe to rerun.
"""
import os
R = os.path.join(os.path.dirname(__file__), '..', '..')
p = os.path.join(R, 'src/data/trainers.h')
s = open(p).read()
s = s.replace('.aiFlags = AI_SCRIPT_CHECK_BAD_MOVE,',
              '.aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,')
open(p, 'w').write(s)
print('smart AI for all trainers')
