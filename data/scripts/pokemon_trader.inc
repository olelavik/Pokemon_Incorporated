@ Pokémon Incorporated: the Pokémon trader in every Pokémon Center.
@ Buys Pokémon from your party or PC boxes. Prices come from
@ src/pokemon_trader.c. The money box is only ever shown once at a time:
@ every path back to the menu hides it first.

.set TRADER_OK,          0
.set TRADER_EGG,         1
.set TRADER_LAST_MON,    2
.set TRADER_BAG_FULL,    3
.set TRADER_NO_BOX_MONS, 4
.set TRADER_CANCEL,      5
.set TRADER_NO_STOCK,    6
.set TRADER_CANT_AFFORD, 7
.set TRADER_NO_ROOM,     8
.set TRADER_SENT_TO_PC,  9

EventScript_PokemonTrader::
	lock
	faceplayer
	msgbox Text_TraderWelcome
EventScript_PokemonTrader_Menu::
	showmoneybox 0, 0
	message Text_TraderWhatWillItBe
	waitmessage
	multichoice 19, 7, MULTICHOICE_TRADER_MAIN, FALSE
	hidemoneybox
	switch VAR_RESULT
	case 0, EventScript_PokemonTrader_Buy
	case 1, EventScript_PokemonTrader_SellMenu
	goto EventScript_PokemonTrader_Bye
	end

EventScript_PokemonTrader_SellMenu::
	showmoneybox 0, 0
	message Text_TraderSellFromWhere
	waitmessage
	multichoice 19, 7, MULTICHOICE_TRADER_SELL_FROM, FALSE
	hidemoneybox
	switch VAR_RESULT
	case 0, EventScript_PokemonTrader_Party
	case 1, EventScript_PokemonTrader_Box
	goto EventScript_PokemonTrader_Menu
	end

EventScript_PokemonTrader_Party::
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, EventScript_PokemonTrader_SellMenu
	special Trader_PricePartyMon
	goto_if_eq VAR_RESULT, TRADER_EGG, EventScript_PokemonTrader_Egg
	goto_if_eq VAR_RESULT, TRADER_LAST_MON, EventScript_PokemonTrader_LastMon
	showmoneybox 0, 0
	call EventScript_PokemonTrader_MakeOffer
	goto_if_eq VAR_RESULT, NO, EventScript_PokemonTrader_Declined
	special Trader_SellPartyMon
	goto_if_eq VAR_RESULT, TRADER_BAG_FULL, EventScript_PokemonTrader_BagFull
	goto EventScript_PokemonTrader_Sold
	end

EventScript_PokemonTrader_Box::
	special Trader_ChooseBoxMon
	goto_if_eq VAR_RESULT, TRADER_NO_BOX_MONS, EventScript_PokemonTrader_NoBoxMons
	waitstate
	goto_if_eq VAR_RESULT, TRADER_CANCEL, EventScript_PokemonTrader_SellMenu
	showmoneybox 0, 0
	call EventScript_PokemonTrader_MakeOffer
	goto_if_eq VAR_RESULT, NO, EventScript_PokemonTrader_Declined
	special Trader_SellBoxMon
	goto_if_eq VAR_RESULT, TRADER_BAG_FULL, EventScript_PokemonTrader_BagFull
	goto EventScript_PokemonTrader_Sold
	end

EventScript_PokemonTrader_MakeOffer::
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, EventScript_PokemonTrader_MakeOfferFemale
	msgbox Text_TraderOffer, MSGBOX_YESNO
	return

EventScript_PokemonTrader_MakeOfferFemale::
	msgbox Text_TraderOfferFemale, MSGBOX_YESNO
	return

EventScript_PokemonTrader_Sold::
	updatemoneybox
	playse SE_SHOP
	msgbox Text_TraderSold
	hidemoneybox
	goto EventScript_PokemonTrader_SellMenu
	end

EventScript_PokemonTrader_Declined::
	msgbox Text_TraderDeclined
	hidemoneybox
	goto EventScript_PokemonTrader_SellMenu
	end

EventScript_PokemonTrader_BagFull::
	msgbox Text_TraderBagFull
	hidemoneybox
	goto EventScript_PokemonTrader_SellMenu
	end

EventScript_PokemonTrader_Egg::
	msgbox Text_TraderEgg
	goto EventScript_PokemonTrader_SellMenu
	end

EventScript_PokemonTrader_LastMon::
	msgbox Text_TraderLastMon
	goto EventScript_PokemonTrader_SellMenu
	end

EventScript_PokemonTrader_NoBoxMons::
	msgbox Text_TraderNoBoxMons
	goto EventScript_PokemonTrader_SellMenu
	end

@ ---- Buying ---------------------------------------------------------------
EventScript_PokemonTrader_BuyAgain::
	message Text_TraderAnythingElse
	waitmessage
EventScript_PokemonTrader_Buy::
	special Trader_ChooseBuyMon
	goto_if_eq VAR_RESULT, TRADER_NO_STOCK, EventScript_PokemonTrader_NoStock
	waitstate
	goto_if_eq VAR_RESULT, TRADER_CANCEL, EventScript_PokemonTrader_Menu
	showmonpic VAR_0x8005, 10, 3
	showmoneybox 0, 0
	goto_if_eq VAR_0x8004, 1, EventScript_PokemonTrader_PitchPremium
	msgbox Text_TraderPitch, MSGBOX_YESNO
	goto EventScript_PokemonTrader_BuyDecide
	end

EventScript_PokemonTrader_PitchPremium::
	msgbox Text_TraderPitchPremium, MSGBOX_YESNO
EventScript_PokemonTrader_BuyDecide::
	hidemonpic
	goto_if_eq VAR_RESULT, NO, EventScript_PokemonTrader_BuyDeclined
	special Trader_BuyMon
	goto_if_eq VAR_RESULT, TRADER_CANT_AFFORD, EventScript_PokemonTrader_CantAfford
	goto_if_eq VAR_RESULT, TRADER_NO_ROOM, EventScript_PokemonTrader_NoRoom
	updatemoneybox
	playfanfare MUS_LEVEL_UP
	message Text_TraderBought
	waitmessage
	waitfanfare
	goto_if_eq VAR_RESULT, TRADER_SENT_TO_PC, EventScript_PokemonTrader_BoughtToPC
	msgbox Text_TraderEnjoy
	hidemoneybox
	goto EventScript_PokemonTrader_BuyAgain
	end

EventScript_PokemonTrader_BoughtToPC::
	msgbox Text_TraderSentToPC
	hidemoneybox
	goto EventScript_PokemonTrader_BuyAgain
	end

EventScript_PokemonTrader_BuyDeclined::
	msgbox Text_TraderBuyDeclined
	hidemoneybox
	goto EventScript_PokemonTrader_BuyAgain
	end

EventScript_PokemonTrader_CantAfford::
	playse SE_FAILURE
	msgbox Text_TraderCantAfford
	hidemoneybox
	goto EventScript_PokemonTrader_BuyAgain
	end

EventScript_PokemonTrader_NoRoom::
	msgbox Text_TraderNoRoom
	hidemoneybox
	goto EventScript_PokemonTrader_Menu
	end

EventScript_PokemonTrader_NoStock::
	msgbox Text_TraderNoStock
	goto EventScript_PokemonTrader_Menu
	end

EventScript_PokemonTrader_Bye::
	msgbox Text_TraderBye
	release
	end

Text_TraderWelcome::
	.string "Welcome to the KANTO POKéMON\n"
	.string "EXCHANGE!\p"
	.string "I buy POKéMON. I sell POKéMON.\n"
	.string "No questions asked.\p"
	.string "The higher the level, the higher\n"
	.string "the price. Evolved is even better!$"

Text_TraderSellFromWhere::
	.string "So, what are you selling?$"

Text_TraderOffer::
	.string "A {STR_VAR_3}, eh?\p"
	.string "I'll give you ¥{STR_VAR_2} for it.\n"
	.string "Take it or leave it.$"

Text_TraderOfferFemale::
	.string "A {STR_VAR_3}, eh?\p"
	.string "For a girl, that's ¥{STR_VAR_2}.\n"
	.string "Market rates, sweetie.\p"
	.string "Take it or leave it.$"

Text_TraderSold::
	.string "Pleasure doing business!\n"
	.string "It's going straight to auction.$"

Text_TraderDeclined::
	.string "Your loss.\n"
	.string "Prices only go down from here.$"

Text_TraderBagFull::
	.string "It's holding an item, and your\n"
	.string "BAG is full.\p"
	.string "Clear some space. I don't buy\n"
	.string "accessories.$"

Text_TraderEgg::
	.string "An EGG? I can't price an EGG.\n"
	.string "Come back when it hatches.$"

Text_TraderLastMon::
	.string "That's your last POKéMON!\p"
	.string "I'd buy it, but then who would\n"
	.string "earn your prize money?$"

Text_TraderNoBoxMons::
	.string "Your PC boxes are empty.\n"
	.string "No inventory, no deal.$"

Text_TraderBye::
	.string "Come back anytime!\n"
	.string "Cash is always king!$"

Text_TraderWhatWillItBe::
	.string "What'll it be?$"

Text_TraderPitch::
	.string "Ah, a {STR_VAR_3}!\n"
	.string "Fresh stock, barely used.\p"
	.string "Yours for ¥{STR_VAR_2}.\n"
	.string "Deal?$"

Text_TraderPitchPremium::
	.string "Ah, a {STR_VAR_3}! Hot item!\n"
	.string "Perfect for the next GYM.\p"
	.string "That's why it's ¥{STR_VAR_2}.\n"
	.string "Supply and demand, kid. Deal?$"

Text_TraderBought::
	.string "{PLAYER} bought the {STR_VAR_3}!$"

Text_TraderEnjoy::
	.string "Enjoy your purchase!\n"
	.string "No refunds, no returns.$"

Text_TraderSentToPC::
	.string "Your party's full, so I sent it\n"
	.string "to your PC. Delivery is free.\p"
	.string "This time.$"

Text_TraderBuyDeclined::
	.string "Window shopping, huh?\n"
	.string "Take your time. Time is money.$"

Text_TraderCantAfford::
	.string "…You can't afford that.\p"
	.string "Come back when you're less poor.$"

Text_TraderNoRoom::
	.string "Your party AND your PC are full?\n"
	.string "Sell me something first!$"

Text_TraderNoStock::
	.string "I'm all sold out here.\n"
	.string "Come back another time.$"

Text_TraderAnythingElse::
	.string "Anything else catch your eye?$"
