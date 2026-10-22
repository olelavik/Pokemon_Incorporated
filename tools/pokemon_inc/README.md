# Pokémon Inc. dev tools

- `textkit.py` – rewrite dialogue in `.inc` files by label, and flag any line
  wider than vanilla's widest line (196 px) so text never overflows its box.
- `gbadrive.c` – tiny headless driver for mGBA's core. Feeds scripted button
  presses, saves/loads savestates and dumps screenshots. Used to smoke-test
  scripts (starter purchase, loan, debt collection) without playing by hand.
  Build against libmgba with the same `-D` defines libmgba was compiled with.
- `sheet.py` – tiles screenshots into a contact sheet.
