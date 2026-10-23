#ifndef GUARD_ECONOMY_H
#define GUARD_ECONOMY_H

// Pokémon Incorporated: the Kanto Price Index (src/economy.c)
u32 Economy_CountBadges(void);
u32 Economy_GetPriceIndex(void);
u32 Economy_Inflate(u32 basePrice);
u32 Economy_ApplyWages(u32 prize);

#endif // GUARD_ECONOMY_H
