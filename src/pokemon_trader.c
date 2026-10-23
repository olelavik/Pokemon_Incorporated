// Pokémon Incorporated: the Pokémon trader.
//
// The trader in every Pokémon Center buys Pokémon from your party or your PC
// boxes. Prices come from a formula so all 386 species have a value:
//
//   base  = 50 * 2^((BST - 250) / 35)     value doubles every 35 base-stat points
//   price = base * (10 + level) / 20      Lv5 = 75%, Lv10 = 100%, Lv50 = 300%
//   x rarity  sqrt(255 / catch rate of the first evolution stage)
//   x shiny   20x, at least 100,000
//   x Kanto Price Index (src/economy.c), capped at MAX_MONEY
//   eggs can't be sold
//
// So low-level catches are pocket change, while raising and evolving a
// Pokémon multiplies its value. That's the investing loop.
//
// Script interface (see data/scripts/pokemon_trader.inc):
//   Trader_PricePartyMon   VAR_0x8004 = party slot (from ChoosePartyMon)
//   Trader_SellPartyMon    VAR_0x8004 = party slot
//   Trader_ChooseBoxMon    opens a list of every boxed Pokémon (use waitstate)
//   Trader_SellBoxMon      sells the Pokémon picked by Trader_ChooseBoxMon
//   Trader_ChooseBuyMon    opens this Center's stock list (use waitstate)
//   Trader_BuyMon          buys the Pokémon picked by Trader_ChooseBuyMon
//                          (VAR_0x8005 = species for showmonpic,
//                           VAR_0x8004 = 1 if it's a premium gym counter)
// Each sets VAR_RESULT to a TRADER_* result and buffers:
//   STR_VAR_2 = price you'll receive, STR_VAR_3 = species name
// (STR_VAR_1 is avoided on purpose: the money box overwrites it.)
#include "global.h"
#include "gflib.h"
#include "economy.h"
#include "data.h"
#include "event_data.h"
#include "item.h"
#include "list_menu.h"
#include "malloc.h"
#include "menu_indicators.h"
#include "sound.h"
#include "task.h"
#include "menu.h"
#include "money.h"
#include "new_menu_helpers.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "strings.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/vars.h"

#define MAX_MONEY 999999

enum {
    TRADER_OK,
    TRADER_EGG,         // eggs aren't merchandise yet
    TRADER_LAST_MON,    // can't sell your last Pokémon
    TRADER_BAG_FULL,    // held item couldn't be returned to the bag
    TRADER_NO_BOX_MONS, // nothing in the PC
    TRADER_CANCEL,      // player backed out of a list
    TRADER_NO_STOCK,    // this Center has nothing for sale
    TRADER_CANT_AFFORD, // not enough money
    TRADER_NO_ROOM,     // party and PC are both full
    TRADER_SENT_TO_PC,  // bought, but the party was full
};

#include "data/pokemon_trader_rarity.h"

#define VALUE_BASE        50      // a BST-250 Pokémon at Lv10, before rarity and inflation
#define SHINY_MULTIPLIER  20
#define SHINY_MINIMUM     100000  // any shiny is a jackpot

// 50 * 2^((bst - 250) / 35), using whole doublings plus a linear blend for
// the remainder (1 + r/35 is within ~6% of 2^(r/35)). No floats on the GBA.
static u32 GetBaseValue(u16 species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    s32 bst = info->baseHP + info->baseAttack + info->baseDefense
            + info->baseSpeed + info->baseSpAttack + info->baseSpDefense;
    s32 steps = bst - 250;
    u32 value = VALUE_BASE;

    if (steps >= 0)
    {
        value <<= steps / 35;
        value = value * (35 + steps % 35) / 35;
    }
    else
    {
        steps = -steps;
        value >>= steps / 35;
        value = value * 35 / (35 + steps % 35);
    }
    if (value == 0)
        value = 1;
    return value;
}

static u32 IntSqrt(u32 n)
{
    u32 r = 0, bit = 1u << 30;

    while (bit > n)
        bit >>= 2;
    while (bit != 0)
    {
        if (n >= r + bit)
        {
            n -= r + bit;
            r = (r >> 1) + bit;
        }
        else
        {
            r >>= 1;
        }
        bit >>= 2;
    }
    return r;
}

// Rarity factor in percent: sqrt(255 / catch rate of the first evolution
// stage). Rattata line x1.0, Eevee/Scyther/Dratini x2.4, Snorlax x3.2,
// legendaries (catch rate 3) x9.2.
static u32 GetRarityPercent(u16 species)
{
    u32 rate = sBaseFormCatchRate[species];

    if (rate == 0)
        rate = gSpeciesInfo[species].catchRate;
    if (rate == 0)
        rate = 255;
    return IntSqrt(255 * 10000 / rate);
}

// Full market value before the girl penalty and market saturation.
static u32 GetTraderPrice(u16 species, u8 level, bool32 shiny)
{
    u32 price = GetBaseValue(species) * (10 + level) / 20;

    price = price * GetRarityPercent(species) / 100;
    if (shiny)
    {
        price *= SHINY_MULTIPLIER;
        if (price < SHINY_MINIMUM)
            price = SHINY_MINIMUM;
    }
    if (price > MAX_MONEY)
        price = MAX_MONEY;     // keep the inflation multiply below from overflowing
    price = Economy_Inflate(price);
    if (price > MAX_MONEY)
        price = MAX_MONEY;
    if (price == 0)
        price = 1;
    return price;
}

// What the player actually receives: AddMoney cuts girls' income by 25%.
static u32 GetPayout(u32 price)
{
    if (gSaveBlock2Ptr->playerGender == FEMALE)
        price -= price / 4;
    return price;
}

static bool32 IsShiny(u32 otId, u32 personality)
{
    return GET_SHINY_VALUE(otId, personality) < SHINY_ODDS;
}

// ---------------------------------------------------------------------------
// Market saturation
//
// The trader remembers the last few species you sold him. Each sale of the
// same species knocks 25% off its next price (never below 10%). The market
// recovers one sale's worth every MARKET_RECOVERY_STEPS steps.
//
// Six saved vars each hold one species (low 9 bits) and its sale count
// (high 7 bits). A new species replaces the least saturated slot.
// ---------------------------------------------------------------------------
#define MARKET_SLOTS          6
#define MARKET_SPECIES_MASK   0x1FF
#define MARKET_COUNT_SHIFT    9
#define MARKET_MAX_COUNT      8
#define MARKET_RECOVERY_STEPS 1000

static u16 Market_GetSlot(u32 i)          { return VarGet(VAR_MARKET_SLOT_0 + i); }
static u16 Market_SlotSpecies(u16 slot)   { return slot & MARKET_SPECIES_MASK; }
static u16 Market_SlotCount(u16 slot)     { return slot >> MARKET_COUNT_SHIFT; }

static void Market_SetSlot(u32 i, u16 species, u16 count)
{
    VarSet(VAR_MARKET_SLOT_0 + i, count == 0 ? 0 : (species | (count << MARKET_COUNT_SHIFT)));
}

static u32 Market_GetSaturation(u16 species)
{
    u32 i;

    for (i = 0; i < MARKET_SLOTS; i++)
    {
        u16 slot = Market_GetSlot(i);
        if (Market_SlotCount(slot) != 0 && Market_SlotSpecies(slot) == species)
            return Market_SlotCount(slot);
    }
    return 0;
}

static void Market_RecordSale(u16 species)
{
    u32 i, target = 0;
    u16 lowest = 0xFFFF;

    for (i = 0; i < MARKET_SLOTS; i++)
    {
        u16 slot = Market_GetSlot(i);
        u16 count = Market_SlotCount(slot);

        if (count != 0 && Market_SlotSpecies(slot) == species)
        {
            if (count < MARKET_MAX_COUNT)
                Market_SetSlot(i, species, count + 1);
            return;
        }
        if (count < lowest)
        {
            lowest = count;
            target = i;
        }
    }
    Market_SetSlot(target, species, 1);
}

static u32 Market_ApplySaturation(u32 price, u16 species)
{
    u32 n = Market_GetSaturation(species);
    u32 floor = price / 10;

    while (n-- > 0)
        price -= price / 4;
    if (price < floor)
        price = floor;
    return price == 0 ? 1 : price;
}

// Called on every step from TryStartStepCountScript.
void Market_UpdateStepCounter(void)
{
    u16 steps = VarGet(VAR_MARKET_STEPS) + 1;
    u32 i;

    if (steps < MARKET_RECOVERY_STEPS)
    {
        VarSet(VAR_MARKET_STEPS, steps);
        return;
    }
    VarSet(VAR_MARKET_STEPS, 0);
    for (i = 0; i < MARKET_SLOTS; i++)
    {
        u16 slot = Market_GetSlot(i);
        if (Market_SlotCount(slot) != 0)
            Market_SetSlot(i, Market_SlotSpecies(slot), Market_SlotCount(slot) - 1);
    }
}

// What the trader pays for a Pokémon, before the girl penalty.
static u32 GetBoxMonPrice(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
    u8 level = GetLevelFromBoxMonExp(boxMon);
    bool32 shiny = IsShiny(GetBoxMonData(boxMon, MON_DATA_OT_ID), GetBoxMonData(boxMon, MON_DATA_PERSONALITY));

    return Market_ApplySaturation(GetTraderPrice(species, level, shiny), species);
}

static void BufferOffer(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

    StringCopy(gStringVar3, gSpeciesNames[species]);
    ConvertIntToDecimalStringN(gStringVar2, GetPayout(GetBoxMonPrice(boxMon)), STR_CONV_MODE_LEFT_ALIGN, 6);
    gSpecialVar_0x8007 = Market_GetSaturation(species) != 0; // script mentions the flooded market
}

// Returns the held item to the bag. FALSE if the bag is full.
static bool32 ReturnHeldItem(struct BoxPokemon *boxMon)
{
    u16 item = GetBoxMonData(boxMon, MON_DATA_HELD_ITEM);

    if (item == ITEM_NONE)
        return TRUE;
    return AddBagItem(item, 1);
}

// ---------------------------------------------------------------------------
// Party
// ---------------------------------------------------------------------------

static bool32 WouldLeaveNoUsableMon(u8 slot)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (i == slot)
            continue;
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            return FALSE;
    }
    return TRUE;
}

static u8 CheckPartyMonSellable(u8 slot)
{
    struct Pokemon *mon = &gPlayerParty[slot];

    if (GetMonData(mon, MON_DATA_IS_EGG))
        return TRADER_EGG;
    if (WouldLeaveNoUsableMon(slot))
        return TRADER_LAST_MON;
    return TRADER_OK;
}

void Trader_PricePartyMon(void)
{
    u8 slot = gSpecialVar_0x8004;

    gSpecialVar_Result = CheckPartyMonSellable(slot);
    if (gSpecialVar_Result == TRADER_OK)
        BufferOffer(&gPlayerParty[slot].box);
}

void Trader_SellPartyMon(void)
{
    u8 slot = gSpecialVar_0x8004;
    struct Pokemon *mon = &gPlayerParty[slot];
    u32 price;

    gSpecialVar_Result = CheckPartyMonSellable(slot);
    if (gSpecialVar_Result != TRADER_OK)
        return;
    if (!ReturnHeldItem(&mon->box))
    {
        gSpecialVar_Result = TRADER_BAG_FULL;
        return;
    }
    price = GetBoxMonPrice(&mon->box);
    BufferOffer(&mon->box);
    Market_RecordSale(GetMonData(mon, MON_DATA_SPECIES));
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    AddMoney(&gSaveBlock1Ptr->money, price);
}

// ---------------------------------------------------------------------------
// Scrollable list shared by the PC box sale and the shop: "NAME Lv.N  ¥price"
// ---------------------------------------------------------------------------

#define TRADER_LIST_CAPACITY   (TOTAL_BOXES_COUNT * IN_BOX_COUNT)
#define TRADER_LIST_LABEL_LEN  (POKEMON_NAME_LENGTH + 12)
#define TRADER_LIST_MAX_SHOWN  6
#define TRADER_LIST_WIDTH      20
#define TRADER_LIST_BASE_BLOCK 0x038

enum {
    TRADER_LIST_BOX,  // selling from the PC
    TRADER_LIST_SHOP, // buying from this town's stock
};

struct TraderList
{
    struct ListMenuItem items[TRADER_LIST_CAPACITY];
    u8 labels[TRADER_LIST_CAPACITY][TRADER_LIST_LABEL_LEN];
    u32 prices[TRADER_LIST_CAPACITY];
    u16 ids[TRADER_LIST_CAPACITY]; // box slot (box * IN_BOX_COUNT + pos) or stock index
    u16 count;
    u8 kind;
    u8 windowId;
    u8 listTaskId;
    u8 arrowTaskId;
    u16 scrollOffset;
};

static EWRAM_DATA struct TraderList *sList = NULL;

static void Task_TraderList(u8 taskId);

static const struct ScrollArrowsTemplate sListArrows = {
    .firstArrowType = SCROLL_ARROW_UP,
    .firstX = 8 + TRADER_LIST_WIDTH * 4,
    .firstY = 4,
    .secondArrowType = SCROLL_ARROW_DOWN,
    .secondX = 8 + TRADER_LIST_WIDTH * 4,
    .secondY = 8 + 16 * TRADER_LIST_MAX_SHOWN + 4,
    .fullyUpThreshold = 0,
    .fullyDownThreshold = 0,
    .tileTag = 2000,
    .palTag = 100,
    .palNum = 0,
};

static void List_Add(u16 species, u8 level, u32 price, u16 id)
{
    u16 n = sList->count;
    u8 *label = sList->labels[n];

    label = StringCopy(label, gSpeciesNames[species]);
    *label++ = CHAR_SPACE;
    *label++ = CHAR_EXTRA_SYMBOL;
    *label++ = CHAR_LV_2;
    ConvertIntToDecimalStringN(label, level, STR_CONV_MODE_LEFT_ALIGN, 3);
    sList->items[n].label = sList->labels[n];
    sList->items[n].index = n;
    sList->prices[n] = price;
    sList->ids[n] = id;
    sList->count++;
}

static void List_PrintPrice(u8 windowId, u32 itemIndex, u8 y)
{
    u8 buffer[12];
    s32 x;

    if (itemIndex >= sList->count)
        return;
    buffer[0] = CHAR_CURRENCY;
    ConvertIntToDecimalStringN(buffer + 1, sList->prices[itemIndex], STR_CONV_MODE_LEFT_ALIGN, 6);
    x = TRADER_LIST_WIDTH * 8 - 4 - GetStringWidth(FONT_NORMAL, buffer, 0);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, buffer, x, y, TEXT_SKIP_DRAW, NULL);
}

static void List_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
}

static void List_Open(void)
{
    struct ListMenuTemplate template = {0};
    struct WindowTemplate window;
    u8 shown = sList->count < TRADER_LIST_MAX_SHOWN ? sList->count : TRADER_LIST_MAX_SHOWN;

    LockPlayerFieldControls();
    window = SetWindowTemplateFields(0, 1, 1, TRADER_LIST_WIDTH, shown * 2, 15, TRADER_LIST_BASE_BLOCK);
    sList->windowId = AddWindow(&window);
    SetStdWindowBorderStyle(sList->windowId, FALSE);

    template.items = sList->items;
    template.moveCursorFunc = List_MoveCursor;
    template.itemPrintFunc = List_PrintPrice;
    template.totalItems = sList->count;
    template.maxShowed = shown;
    template.windowId = sList->windowId;
    template.item_X = 8;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.fontId = FONT_NORMAL;
    sList->listTaskId = ListMenuInit(&template, 0, 0);

    sList->arrowTaskId = TASK_NONE;
    if (sList->count > TRADER_LIST_MAX_SHOWN)
    {
        struct ScrollArrowsTemplate arrows = sListArrows;
        arrows.fullyDownThreshold = sList->count - TRADER_LIST_MAX_SHOWN;
        sList->arrowTaskId = AddScrollIndicatorArrowPair(&arrows, &sList->scrollOffset);
    }
    PutWindowTilemap(sList->windowId);
    CopyWindowToVram(sList->windowId, COPYWIN_FULL);
    CreateTask(Task_TraderList, 8);
}

static void List_Close(u8 taskId)
{
    if (sList->arrowTaskId != TASK_NONE)
        RemoveScrollIndicatorArrowPair(sList->arrowTaskId);
    DestroyListMenuTask(sList->listTaskId, NULL, NULL);
    ClearStdWindowAndFrameToTransparent(sList->windowId, TRUE);
    ClearWindowTilemap(sList->windowId);
    CopyWindowToVram(sList->windowId, COPYWIN_GFX);
    RemoveWindow(sList->windowId);
    FREE_AND_SET_NULL(sList);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void List_OnPicked(u16 id); // defined with the shop below

static void Task_TraderList(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(sList->listTaskId);
    u16 scroll, row;

    ListMenuGetScrollAndRow(sList->listTaskId, &scroll, &row);
    sList->scrollOffset = scroll;

    if (input == LIST_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL)
        gSpecialVar_Result = TRADER_CANCEL;
    else
        List_OnPicked(sList->ids[input]);
    List_Close(taskId);
}

// ---------------------------------------------------------------------------
// Selling from the PC boxes
// ---------------------------------------------------------------------------

void Trader_ChooseBoxMon(void)
{
    u32 box, pos;

    sList = AllocZeroed(sizeof(*sList));
    sList->kind = TRADER_LIST_BOX;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, pos);
            u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

            if (species == SPECIES_NONE || GetBoxMonData(boxMon, MON_DATA_IS_EGG))
                continue;
            List_Add(species, GetLevelFromBoxMonExp(boxMon), GetPayout(GetBoxMonPrice(boxMon)), box * IN_BOX_COUNT + pos);
        }
    }
    if (sList->count == 0)
    {
        FREE_AND_SET_NULL(sList);
        gSpecialVar_Result = TRADER_NO_BOX_MONS;
        return; // no list opened: the script skips its waitstate
    }
    gSpecialVar_Result = TRADER_OK;
    List_Open();
}

void Trader_SellBoxMon(void)
{
    struct BoxPokemon *boxMon = GetBoxedMonPtr(gSpecialVar_0x8005, gSpecialVar_0x8006);
    u32 price;

    if (GetBoxMonData(boxMon, MON_DATA_SPECIES) == SPECIES_NONE)
    {
        gSpecialVar_Result = TRADER_CANCEL;
        return;
    }
    if (!ReturnHeldItem(boxMon))
    {
        gSpecialVar_Result = TRADER_BAG_FULL;
        return;
    }
    price = GetBoxMonPrice(boxMon);
    BufferOffer(boxMon);
    Market_RecordSale(GetBoxMonData(boxMon, MON_DATA_SPECIES));
    ZeroBoxMonAt(gSpecialVar_0x8005, gSpecialVar_0x8006);
    AddMoney(&gSaveBlock1Ptr->money, price);
    gSpecialVar_Result = TRADER_OK;
}

// ---------------------------------------------------------------------------
// Buying: each Pokémon Center has its own stock (src/data/pokemon_trader_stock.h)
// ---------------------------------------------------------------------------

#define MARKUP_NORMAL  4 // buy price = 4x what he'd pay you back at that level
#define MARKUP_PREMIUM 8 // counters for the next gym cost double

struct TraderStock
{
    u16 species;
    u8 level;
    u8 minBadges:4; // appears once you have this many badges
    u8 premium:1;   // "most useful for the next gym" markup
};

struct TraderShop
{
    u16 map; // MAP_* of the Pokémon Center
    const struct TraderStock *stock;
    u16 count;
};

#include "data/pokemon_trader_stock.h"

static const struct TraderShop *GetCurrentShop(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sTraderShops); i++)
    {
        if (sTraderShops[i].map == map)
            return &sTraderShops[i];
    }
    return NULL;
}

static u32 GetBuyPrice(const struct TraderStock *stock)
{
    u32 price = GetTraderPrice(stock->species, stock->level, FALSE)
              * (stock->premium ? MARKUP_PREMIUM : MARKUP_NORMAL);

    return price > MAX_MONEY ? MAX_MONEY : price;
}


void Trader_ChooseBuyMon(void)
{
    const struct TraderShop *shop = GetCurrentShop();
    u32 i, badges = Economy_CountBadges();

    if (shop == NULL)
    {
        gSpecialVar_Result = TRADER_NO_STOCK;
        return;
    }
    sList = AllocZeroed(sizeof(*sList));
    sList->kind = TRADER_LIST_SHOP;
    for (i = 0; i < shop->count; i++)
    {
        const struct TraderStock *stock = &shop->stock[i];

        if (badges >= stock->minBadges)
            List_Add(stock->species, stock->level, GetBuyPrice(stock), i);
    }
    if (sList->count == 0)
    {
        FREE_AND_SET_NULL(sList);
        gSpecialVar_Result = TRADER_NO_STOCK;
        return;
    }
    gSpecialVar_Result = TRADER_OK;
    List_Open();
}

static void List_OnPicked(u16 id)
{
    gSpecialVar_Result = TRADER_OK;
    if (sList->kind == TRADER_LIST_BOX)
    {
        gSpecialVar_0x8005 = id / IN_BOX_COUNT;
        gSpecialVar_0x8006 = id % IN_BOX_COUNT;
        BufferOffer(GetBoxedMonPtr(gSpecialVar_0x8005, gSpecialVar_0x8006));
    }
    else
    {
        const struct TraderStock *stock = &GetCurrentShop()->stock[id];

        gSpecialVar_0x8004 = stock->premium;
        gSpecialVar_0x8005 = stock->species; // for showmonpic
        gSpecialVar_0x8006 = id;
        StringCopy(gStringVar3, gSpeciesNames[stock->species]);
        ConvertIntToDecimalStringN(gStringVar2, GetBuyPrice(stock), STR_CONV_MODE_LEFT_ALIGN, 6);
    }
}

void Trader_BuyMon(void)
{
    const struct TraderShop *shop = GetCurrentShop();
    const struct TraderStock *stock;
    u32 price;
    u8 given;

    if (shop == NULL || gSpecialVar_0x8006 >= shop->count)
    {
        gSpecialVar_Result = TRADER_CANCEL;
        return;
    }
    stock = &shop->stock[gSpecialVar_0x8006];
    price = GetBuyPrice(stock);
    if (GetMoney(&gSaveBlock1Ptr->money) < price)
    {
        gSpecialVar_Result = TRADER_CANT_AFFORD;
        return;
    }
    given = ScriptGiveMon(stock->species, stock->level, ITEM_NONE, 0, 0, 0);
    if (given == MON_CANT_GIVE)
    {
        gSpecialVar_Result = TRADER_NO_ROOM;
        return;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, price);
    gSpecialVar_Result = (given == MON_GIVEN_TO_PC) ? TRADER_SENT_TO_PC : TRADER_OK;
}
