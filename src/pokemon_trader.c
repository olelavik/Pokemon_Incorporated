// Pokémon Incorporated: the Pokémon trader.
//
// The trader in every Pokémon Center buys Pokémon from your party or your PC
// boxes. Prices come from a formula so all 386 species have a value:
//
//   base  = 80 * 2^((BST - 250) / 35)     value doubles every 35 base-stat points
//   price = base * (10 + level) / 20      Lv5 = 75%, Lv10 = 100%, Lv50 = 300%
//   legendary x3, shiny x5, eggs can't be sold, capped at MAX_MONEY
//
// So low-level catches are pocket change, while raising and evolving a
// Pokémon multiplies its value. That's the investing loop.
//
// Script interface (see data/scripts/pokemon_trader.inc):
//   Trader_PricePartyMon   VAR_0x8004 = party slot (from ChoosePartyMon)
//   Trader_SellPartyMon    VAR_0x8004 = party slot
//   Trader_ChooseBoxMon    opens a list of every boxed Pokémon (use waitstate)
//   Trader_SellBoxMon      sells the Pokémon picked by Trader_ChooseBoxMon
// Each sets VAR_RESULT to a TRADER_* result and buffers:
//   STR_VAR_2 = price you'll receive, STR_VAR_3 = species name
// (STR_VAR_1 is avoided on purpose: the money box overwrites it.)
#include "global.h"
#include "gflib.h"
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
#include "string_util.h"
#include "strings.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/species.h"

#define MAX_MONEY 999999

enum {
    TRADER_OK,
    TRADER_EGG,         // eggs aren't merchandise yet
    TRADER_LAST_MON,    // can't sell your last Pokémon
    TRADER_BAG_FULL,    // held item couldn't be returned to the bag
    TRADER_NO_BOX_MONS, // nothing in the PC
    TRADER_CANCEL,      // player backed out of the box list
};

static const u16 sLegendaries[] = {
    SPECIES_ARTICUNO, SPECIES_ZAPDOS, SPECIES_MOLTRES, SPECIES_MEWTWO, SPECIES_MEW,
    SPECIES_RAIKOU, SPECIES_ENTEI, SPECIES_SUICUNE, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI,
    SPECIES_REGIROCK, SPECIES_REGICE, SPECIES_REGISTEEL, SPECIES_LATIAS, SPECIES_LATIOS,
    SPECIES_KYOGRE, SPECIES_GROUDON, SPECIES_RAYQUAZA, SPECIES_JIRACHI, SPECIES_DEOXYS,
};

static bool32 IsLegendary(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLegendaries); i++)
    {
        if (sLegendaries[i] == species)
            return TRUE;
    }
    return FALSE;
}

// 80 * 2^((bst - 250) / 35), using whole doublings plus a linear blend for
// the remainder (1 + r/35 is within ~6% of 2^(r/35)). No floats on the GBA.
static u32 GetBaseValue(u16 species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    s32 bst = info->baseHP + info->baseAttack + info->baseDefense
            + info->baseSpeed + info->baseSpAttack + info->baseSpDefense;
    s32 steps = bst - 250;
    u32 value = 80;

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

static u32 GetTraderPrice(u16 species, u8 level, bool32 shiny)
{
    u32 price = GetBaseValue(species) * (10 + level) / 20;

    if (IsLegendary(species))
        price *= 3;
    if (shiny)
        price *= 5;
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

static u32 GetBoxMonPrice(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
    u8 level = GetLevelFromBoxMonExp(boxMon);
    bool32 shiny = IsShiny(GetBoxMonData(boxMon, MON_DATA_OT_ID), GetBoxMonData(boxMon, MON_DATA_PERSONALITY));

    return GetTraderPrice(species, level, shiny);
}

static void BufferOffer(struct BoxPokemon *boxMon)
{
    u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

    StringCopy(gStringVar3, gSpeciesNames[species]);
    ConvertIntToDecimalStringN(gStringVar2, GetPayout(GetBoxMonPrice(boxMon)), STR_CONV_MODE_LEFT_ALIGN, 6);
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
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    AddMoney(&gSaveBlock1Ptr->money, price);
}

// ---------------------------------------------------------------------------
// PC boxes: a scrollable list of every boxed Pokémon with its price
// ---------------------------------------------------------------------------

#define BOX_LIST_LABEL_LEN   (POKEMON_NAME_LENGTH + 12)
#define BOX_LIST_MAX_SHOWN   6
#define BOX_LIST_WIDTH       20
#define BOX_LIST_BASE_BLOCK  0x038

struct TraderBoxList
{
    struct ListMenuItem items[TOTAL_BOXES_COUNT * IN_BOX_COUNT];
    u8 labels[TOTAL_BOXES_COUNT * IN_BOX_COUNT][BOX_LIST_LABEL_LEN];
    u16 location[TOTAL_BOXES_COUNT * IN_BOX_COUNT]; // box * IN_BOX_COUNT + position
    u16 count;
    u8 windowId;
    u8 listTaskId;
    u8 arrowTaskId;
    u16 scrollOffset;
};

static EWRAM_DATA struct TraderBoxList *sBoxList = NULL;

static void Task_TraderBoxList(u8 taskId);

static const struct ScrollArrowsTemplate sBoxListArrows = {
    .firstArrowType = SCROLL_ARROW_UP,
    .firstX = 8 * (1 + BOX_LIST_WIDTH / 2) + 8,
    .firstY = 4,
    .secondArrowType = SCROLL_ARROW_DOWN,
    .secondX = 8 * (1 + BOX_LIST_WIDTH / 2) + 8,
    .secondY = 8 + 16 * BOX_LIST_MAX_SHOWN + 4,
    .fullyUpThreshold = 0,
    .fullyDownThreshold = 0,
    .tileTag = 2000,
    .palTag = 100,
    .palNum = 0,
};

static void BoxList_PrintPrice(u8 windowId, u32 itemIndex, u8 y)
{
    u8 buffer[12];
    u16 loc;
    s32 x;

    if (itemIndex >= sBoxList->count)
        return;
    loc = sBoxList->location[itemIndex];
    buffer[0] = CHAR_CURRENCY;
    ConvertIntToDecimalStringN(buffer + 1,
                               GetPayout(GetBoxMonPrice(GetBoxedMonPtr(loc / IN_BOX_COUNT, loc % IN_BOX_COUNT))),
                               STR_CONV_MODE_LEFT_ALIGN, 6);
    x = BOX_LIST_WIDTH * 8 - 4 - GetStringWidth(FONT_NORMAL, buffer, 0);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, buffer, x, y, TEXT_SKIP_DRAW, NULL);
}

static void BoxList_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
}

static void BoxList_Build(void)
{
    u32 box, pos;
    u16 n = 0;

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            struct BoxPokemon *boxMon = GetBoxedMonPtr(box, pos);
            u16 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
            u8 *label;

            if (species == SPECIES_NONE || GetBoxMonData(boxMon, MON_DATA_IS_EGG))
                continue;
            label = sBoxList->labels[n];
            label = StringCopy(label, gSpeciesNames[species]);
            *label++ = CHAR_SPACE;
            *label++ = CHAR_EXTRA_SYMBOL;
            *label++ = CHAR_LV_2;
            label = ConvertIntToDecimalStringN(label, GetLevelFromBoxMonExp(boxMon), STR_CONV_MODE_LEFT_ALIGN, 3);
            sBoxList->items[n].label = sBoxList->labels[n];
            sBoxList->items[n].index = n;
            sBoxList->location[n] = box * IN_BOX_COUNT + pos;
            n++;
        }
    }
    sBoxList->count = n;
}

void Trader_ChooseBoxMon(void)
{
    struct ListMenuTemplate template = {0};
    struct WindowTemplate window;
    u8 shown;

    sBoxList = AllocZeroed(sizeof(*sBoxList));
    BoxList_Build();
    if (sBoxList->count == 0)
    {
        FREE_AND_SET_NULL(sBoxList);
        gSpecialVar_Result = TRADER_NO_BOX_MONS;
        return; // no waitstate needed: the script checks VAR_RESULT first
    }

    LockPlayerFieldControls();
    shown = sBoxList->count < BOX_LIST_MAX_SHOWN ? sBoxList->count : BOX_LIST_MAX_SHOWN;
    window = SetWindowTemplateFields(0, 1, 1, BOX_LIST_WIDTH, shown * 2, 15, BOX_LIST_BASE_BLOCK);
    sBoxList->windowId = AddWindow(&window);
    SetStdWindowBorderStyle(sBoxList->windowId, FALSE);

    template.items = sBoxList->items;
    template.moveCursorFunc = BoxList_MoveCursor;
    template.itemPrintFunc = BoxList_PrintPrice;
    template.totalItems = sBoxList->count;
    template.maxShowed = shown;
    template.windowId = sBoxList->windowId;
    template.item_X = 8;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.lettersSpacing = 0;
    template.fontId = FONT_NORMAL;
    sBoxList->listTaskId = ListMenuInit(&template, 0, 0);

    sBoxList->arrowTaskId = TASK_NONE;
    if (sBoxList->count > BOX_LIST_MAX_SHOWN)
    {
        struct ScrollArrowsTemplate arrows = sBoxListArrows;
        arrows.fullyDownThreshold = sBoxList->count - BOX_LIST_MAX_SHOWN;
        sBoxList->arrowTaskId = AddScrollIndicatorArrowPair(&arrows, &sBoxList->scrollOffset);
    }
    PutWindowTilemap(sBoxList->windowId);
    CopyWindowToVram(sBoxList->windowId, COPYWIN_FULL);
    gSpecialVar_Result = TRADER_OK;
    CreateTask(Task_TraderBoxList, 8);
}

static void BoxList_Destroy(u8 taskId)
{
    if (sBoxList->arrowTaskId != TASK_NONE)
        RemoveScrollIndicatorArrowPair(sBoxList->arrowTaskId);
    DestroyListMenuTask(sBoxList->listTaskId, NULL, NULL);
    ClearStdWindowAndFrameToTransparent(sBoxList->windowId, TRUE);
    ClearWindowTilemap(sBoxList->windowId);
    CopyWindowToVram(sBoxList->windowId, COPYWIN_GFX);
    RemoveWindow(sBoxList->windowId);
    FREE_AND_SET_NULL(sBoxList);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_TraderBoxList(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(sBoxList->listTaskId);
    u16 cursorPos, itemsAbove;

    ListMenuGetScrollAndRow(sBoxList->listTaskId, &cursorPos, &itemsAbove);
    sBoxList->scrollOffset = cursorPos;

    if (input == LIST_NOTHING_CHOSEN)
        return;
    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL)
    {
        gSpecialVar_Result = TRADER_CANCEL;
    }
    else
    {
        u16 loc = sBoxList->location[input];
        gSpecialVar_0x8005 = loc / IN_BOX_COUNT;
        gSpecialVar_0x8006 = loc % IN_BOX_COUNT;
        BufferOffer(GetBoxedMonPtr(gSpecialVar_0x8005, gSpecialVar_0x8006));
        gSpecialVar_Result = TRADER_OK;
    }
    BoxList_Destroy(taskId);
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
    ZeroBoxMonAt(gSpecialVar_0x8005, gSpecialVar_0x8006);
    AddMoney(&gSaveBlock1Ptr->money, price);
    gSpecialVar_Result = TRADER_OK;
}
