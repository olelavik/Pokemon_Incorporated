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
