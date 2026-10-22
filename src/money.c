#include "global.h"
#include "gflib.h"
#include "event_data.h"
#include "menu.h"
#include "text_window.h"
#include "strings.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/vars.h"

#define MAX_MONEY 999999

EWRAM_DATA static u8 sMoneyBoxWindowId = 0;

u32 GetMoney(u32 *moneyPtr)
{
    return *moneyPtr ^ gSaveBlock2Ptr->encryptionKey;
}

void SetMoney(u32 *moneyPtr, u32 newValue)
{
    *moneyPtr = gSaveBlock2Ptr->encryptionKey ^ newValue;
}

bool8 IsEnoughMoney(u32 *moneyPtr, u32 cost)
{
    if (GetMoney(moneyPtr) >= cost)
        return TRUE;
    else
        return FALSE;
}

void AddMoney(u32 *moneyPtr, u32 toAdd)
{
    u32 toSet;

    // Dollar Bill Green: girls earn 25% less from every source of income
    if (moneyPtr == &gSaveBlock1Ptr->money && gSaveBlock2Ptr->playerGender == FEMALE)
        toAdd -= toAdd / 4;

    toSet = GetMoney(moneyPtr);

    // can't have more money than MAX
    if (toSet + toAdd > MAX_MONEY)
    {
        toSet = MAX_MONEY;
    }
    else
    {
        toSet += toAdd;
        // check overflow, can't have less money after you receive more
        if (toSet < GetMoney(moneyPtr))
            toSet = MAX_MONEY;
    }

    SetMoney(moneyPtr, toSet);
}

void RemoveMoney(u32 *moneyPtr, u32 toSub)
{
    u32 toSet = GetMoney(moneyPtr);

    // can't subtract more than you already have
    if (toSet < toSub)
        toSet = 0;
    else
        toSet -= toSub;

    SetMoney(moneyPtr, toSet);
}

bool8 IsEnoughForCostInVar0x8005(void)
{
    return IsEnoughMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8005);
}

void SubtractMoneyFromVar0x8005(void)
{
    RemoveMoney(&gSaveBlock1Ptr->money, gSpecialVar_0x8005);
}

void PrintMoneyAmountInMoneyBox(u8 windowId, int amount, u8 speed)
{
    u8 *txtPtr;
    s32 strLength;

    ConvertIntToDecimalStringN(gStringVar1, amount, STR_CONV_MODE_LEFT_ALIGN, 6);

    strLength = 6 - StringLength(gStringVar1);
    txtPtr = gStringVar4;

    while (strLength-- != 0)
        *(txtPtr++) = 0;

    StringExpandPlaceholders(txtPtr, gText_PokedollarVar1);
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, 64 - GetStringWidth(FONT_SMALL, gStringVar4, 0), 0xC, speed, NULL);
}

void PrintMoneyAmount(u8 windowId, u8 x, u8 y, int amount, u8 speed)
{
    u8 *txtPtr;
    s32 strLength;

    ConvertIntToDecimalStringN(gStringVar1, amount, STR_CONV_MODE_LEFT_ALIGN, 6);

    strLength = 6 - StringLength(gStringVar1);
    txtPtr = gStringVar4;

    while (strLength-- != 0)
        *(txtPtr++) = 0;

    StringExpandPlaceholders(txtPtr, gText_PokedollarVar1);
    AddTextPrinterParameterized(windowId, FONT_SMALL, gStringVar4, x, y, speed, NULL);
}

void PrintMoneyAmountInMoneyBoxWithBorder(u8 windowId, u16 tileStart, u8 paletteNum, int amount)
{
    DrawStdFrameWithCustomTileAndPalette(windowId, FALSE, tileStart, paletteNum);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_TrainerCardMoney, 0, 0, 0xFF, 0);
    PrintMoneyAmountInMoneyBox(windowId, amount, 0);
}

void ChangeAmountInMoneyBox(int amount)
{
    PrintMoneyAmountInMoneyBox(sMoneyBoxWindowId, amount, 0);
}

void DrawMoneyBox(int amount, u8 x, u8 y)
{
    struct WindowTemplate template;

    template = SetWindowTemplateFields(0, x + 1, y + 1, 8, 3, 15, 8);
    sMoneyBoxWindowId = AddWindow(&template);
    FillWindowPixelBuffer(sMoneyBoxWindowId, 0);
    PutWindowTilemap(sMoneyBoxWindowId);
    LoadStdWindowGfx(sMoneyBoxWindowId, 0x21D, BG_PLTT_ID(13));
    PrintMoneyAmountInMoneyBoxWithBorder(sMoneyBoxWindowId, 0x21D, 13, amount);
}

void HideMoneyBox(void)
{
    ClearStdWindowAndFrameToTransparent(sMoneyBoxWindowId, FALSE);
    CopyWindowToVram(sMoneyBoxWindowId, COPYWIN_GFX);
    RemoveWindow(sMoneyBoxWindowId);
}

// ---------------------------------------------------------------------------
// Pokémon Incorporated: Oak Financial debt collection.
// Called by EventScript_OakAideDebtCollection right after each Gym Badge.
//
// Each badge adds 14% interest. From the 2nd badge on, Oak's aide collects an
// installment sized so the loan is paid off by the 8th badge. If you can't
// pay, the aide takes everything you have and adds a late fee.
//
// Output (numbers, not strings: the money box overwrites gStringVar1 when it
// redraws, so the script formats these after updating the money box):
//   gSpecialVar_0x8004 = interest added
//   gSpecialVar_0x8005 = amount paid
//   gSpecialVar_0x8006 = DEBT_RESULT_*
//   gSpecialVar_0x8007 = debt remaining
// ---------------------------------------------------------------------------
#define DEBT_INTEREST_PERCENT 14
#define DEBT_LATE_FEE         500
#define DEBT_MAX              0xFFFF

enum {
    DEBT_RESULT_NONE,          // nothing owed, aide doesn't show up
    DEBT_RESULT_INTEREST_ONLY, // grace period: interest added, no payment
    DEBT_RESULT_PAID,          // installment paid
    DEBT_RESULT_COULDNT_PAY,   // took all your money + late fee
    DEBT_RESULT_PAID_OFF,      // loan fully repaid
};

static u32 CountBadges(void)
{
    u32 i, count = 0;

    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
    {
        if (FlagGet(i))
            count++;
    }
    return count;
}

void OakDebt_Collect(void)
{
    u32 debt = VarGet(VAR_OAK_DEBT);
    u32 badges = CountBadges();
    u32 interest, due, paid = 0, wallet, collectionsLeft;

    if (debt == 0)
    {
        gSpecialVar_0x8006 = DEBT_RESULT_NONE;
        return;
    }

    interest = debt * DEBT_INTEREST_PERCENT / 100;
    debt += interest;

    if (badges < 2)
    {
        gSpecialVar_0x8006 = DEBT_RESULT_INTEREST_ONLY;
    }
    else
    {
        // Badge 2 -> 7 payments left, ..., badge 8 -> pay everything.
        collectionsLeft = (badges >= NUM_BADGES) ? 1 : (NUM_BADGES + 1 - badges);
        due = (debt + collectionsLeft - 1) / collectionsLeft;
        wallet = GetMoney(&gSaveBlock1Ptr->money);

        if (wallet >= due)
        {
            paid = due;
            debt -= due;
            gSpecialVar_0x8006 = (debt == 0) ? DEBT_RESULT_PAID_OFF : DEBT_RESULT_PAID;
        }
        else
        {
            paid = wallet;
            debt = debt - paid + DEBT_LATE_FEE;
            gSpecialVar_0x8006 = DEBT_RESULT_COULDNT_PAY;
        }
        RemoveMoney(&gSaveBlock1Ptr->money, paid);
    }

    if (debt > DEBT_MAX)
        debt = DEBT_MAX;
    VarSet(VAR_OAK_DEBT, debt);

    // All three fit in 16 bits: interest and payments never exceed DEBT_MAX.
    gSpecialVar_0x8004 = interest;
    gSpecialVar_0x8005 = paid;
    gSpecialVar_0x8007 = debt;
}
