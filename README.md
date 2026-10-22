# Pokémon Incorporated

*Gotta Buy 'Em All.*

A capitalist satire of Pokémon FireRed. Pokémon are private commodities to be
bought and sold, you start the game in debt to Professor Oak, and everything
costs money: Pokémon Centers, gyms, forests, even Bill's PC.

Built on the [pret/pokefirered](https://github.com/pret/pokefirered) decompilation.
The original decomp README is in [README_DECOMP.md](README_DECOMP.md).

## Status: proof of concept

| Feature | Status |
| --- | --- |
| Oak's intro: Pokémon are products | Done |
| Boy (easy) / Girl (hard) select | Done |
| Girls earn 25% less from all income | Done (`src/money.c`, `AddMoney`) |
| Start with ₽0 and a Bicycle (resells for ₽4,000) | Done (`src/new_game.c`) |
| Pokémon Centers cost ₽300 | Done (`data/scripts/pkmn_center_nurse.inc`) |
| Expensive balls and healing items | Done (`src/data/items.json`) |
| Oak rewritten as a greedy salesman | Done |
| Starter prices, Gary mocks you, Oak's 14% loan | Done (`PalletTown_ProfessorOaksLab`) |
| Oak's aide collects debt after every badge | Done (`OakDebt_Collect` in `src/money.c`, `data/scripts/oak_debt.inc`) |
| Greedy NPCs, Pallet Town to Pewter City | Done |
| Viridian old man: landowner lecture, no tutorial | Done |
| Selling Pokémon, Pokémon shops | Planned |
| Entry fees, toll booths | Planned |
| Bill's PC free (ads) / premium | Planned |
| Mom bailout when broke | Planned |
| Greedy NPCs past Pewter | Planned |

## Playing

Apply the `.bps` patch to your own **Pokémon FireRed (US) v1.0** ROM
(`sha1: 41cb23d8dccc8ebd7c649cd8fbb58eeace6e2fdc`) using
[Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/) or Floating IPS.
No ROMs are distributed here.

## Building

Follow [INSTALL.md](INSTALL.md) from the decomp, then run `make`. The output is
`pokefirered.gba`.

To make a patch, build the unmodified decomp once to get a clean base ROM, then:

```
flips --create --bps vanilla.gba pokefirered.gba PokemonIncorporated.bps
```

## Credits

- Concept and design: olelavik
- Code: written with Claude (Anthropic)
- Decompilation: [pret](https://pret.github.io/)
