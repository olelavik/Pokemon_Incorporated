#!/bin/sh
# Pokémon Incorporated: regenerate every trainer change from vanilla data.
# Run from the repo root. Order matters: wealth tiers apply on top of the rest.
set -e
VANILLA=${VANILLA:-c05adddd9}   # last commit before v0.10 trainer changes
git checkout "$VANILLA" -- src/data/trainers.h src/data/trainer_parties.h
python3 tools/pokemon_inc/smart_ai.py
python3 tools/pokemon_inc/gym_teams.py
python3 tools/pokemon_inc/rocket_inc.py
python3 tools/pokemon_inc/wealth_tiers.py
