# Pokémon Inc. dev tools

- `textkit.py` – rewrite dialogue in `.inc` files by label, and flag any line
  wider than vanilla's widest line (196 px) so text never overflows its box.
- `gbadrive.c` – tiny headless driver for mGBA's core. Feeds scripted button
  presses, saves/loads savestates and dumps screenshots. Used to smoke-test
  scripts (starter purchase, loan, debt collection) without playing by hand.
  Build against libmgba with the same `-D` defines libmgba was compiled with.
- `sheet.py` – tiles screenshots into a contact sheet.
- `maprender.py` – render any layout or a sheet of numbered metatiles to PNG.
- `exchange_building.py` – adds the brown "CASH" Pokémon Exchange building to
  a town: appends 16 metatiles to the town's tileset and stamps it on the map.
  `python3 exchange_building.py add <town_tileset_dir> <LAYOUT_ID> <x> <y>`
- `exchange_interior.py` – (re)builds the `pokemon_exchange` interior tileset
  (Mart + cage tiles) and `LAYOUT_POKEMON_EXCHANGE`.
- `find_spots.py` – list free 4x4 spots in a town for a new building.
- `add_exchange.py` – one command per town: building, door warp, interior map,
  caged Pokémon, customers, map group registration, stock and booth removal.
  Towns are configured in its TOWNS table.

## Trainers (v0.10)

All trainer changes are generated from vanilla data, so they can be reviewed
as Python tables and rerun after editing:

```
tools/pokemon_inc/build_trainers.sh
```

restores `src/data/trainers.h` and `src/data/trainer_parties.h` from the last
pre-v0.10 commit, then runs, in order:

- `smart_ai.py` – boss AI flags for every trainer.
- `gym_teams.py` – the eight gym leader teams (design doc: "Gym leader teams").
- `rocket_inc.py` – Rocket grunts by department and the four executives.
- `wealth_tiers.py` – regular trainers by prize-money tier (not idempotent on
  its own, which is why the runner starts from vanilla).
- `partylib.py` – shared helper that rewrites a trainer's party array and
  picks the right party struct.

`wild_gen23.py` sets the Gen 2/3 wild encounter slots (idempotent).
