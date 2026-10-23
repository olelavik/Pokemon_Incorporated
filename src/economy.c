// Pokémon Incorporated: the Kanto Price Index.
//
// Every price in the game is a badge-0 base price multiplied by the price
// index for the number of badges you hold (100% -> 750% at 8 badges).
// Prize money from trainers ("wages") also rises, but more slowly
// (85% -> 330%). See tools/pokemon_inc/model/economy.py for the model these
// numbers come from.
//
// Script interface:
//   Economy_PrepareCost      VAR_0x8004 = base cost. Inflates it, buffers the
//                            price in STR_VAR_2 and remembers it.
//   Economy_PrepareFlatCost  VAR_0x8004 = cost / 100, no inflation (League fee).
//   Economy_CanAfford        VAR_RESULT = TRUE if you can pay the prepared cost
//   Economy_Pay              pays the prepared cost
//   Economy_GiveFlatMoney    VAR_0x8004 = amount / 100, gives it (League jackpot)
//   Economy_BufferPriceIndex STR_VAR_2 = current index in percent
#include "global.h"
#include "economy.h"
#include "event_data.h"
#include "money.h"
#include "string_util.h"
#include "constants/flags.h"

static const u16 sPriceIndex[NUM_BADGES + 1] = {100, 120, 150, 200, 260, 340, 440, 570, 750};
static const u16 sWageIndex[NUM_BADGES + 1]  = { 85,  85,  85, 100, 140, 180, 215, 300, 330};

static EWRAM_DATA u32 sPreparedCost = 0;

u32 Economy_CountBadges(void)
{
    u32 i, count = 0;

    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
    {
        if (FlagGet(i))
            count++;
    }
    return count;
}

u32 Economy_GetPriceIndex(void)
{
    return sPriceIndex[Economy_CountBadges()];
}

u32 Economy_Inflate(u32 basePrice)
{
    if (basePrice == 0)
        return 0;
    basePrice = basePrice * Economy_GetPriceIndex() / 100;
    return basePrice == 0 ? 1 : basePrice;
}

u32 Economy_ApplyWages(u32 prize)
{
    return prize * sWageIndex[Economy_CountBadges()] / 100;
}

static void BufferCost(u32 cost)
{
    ConvertIntToDecimalStringN(gStringVar2, cost, STR_CONV_MODE_LEFT_ALIGN, 7);
}

void Economy_PrepareCost(void)
{
    sPreparedCost = Economy_Inflate(gSpecialVar_0x8004);
    BufferCost(sPreparedCost);
}

void Economy_PrepareFlatCost(void)
{
    sPreparedCost = gSpecialVar_0x8004 * 100;
    BufferCost(sPreparedCost);
}

void Economy_CanAfford(void)
{
    gSpecialVar_Result = IsEnoughMoney(&gSaveBlock1Ptr->money, sPreparedCost);
}

void Economy_Pay(void)
{
    RemoveMoney(&gSaveBlock1Ptr->money, sPreparedCost);
}

void Economy_GiveFlatMoney(void)
{
    AddMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8004 * 100);
}

void Economy_BufferPriceIndex(void)
{
    ConvertIntToDecimalStringN(gStringVar2, Economy_GetPriceIndex(), STR_CONV_MODE_LEFT_ALIGN, 3);
}
