@ Pokémon Incorporated: Oak's aide collects the starter loan after each badge.
@ Called from every gym leader's "defeated" script, right after the badge.

EventScript_OakAideDebtCollection::
	special OakDebt_Collect
	goto_if_eq VAR_0x8006, 0, EventScript_OakAideDebtCollection_End
	textcolor NPC_TEXT_COLOR_MALE
	playse SE_DOOR
	msgbox Text_OakAideBurstsIn
	showmoneybox 0, 0
	updatemoneybox
	buffernumberstring STR_VAR_1, VAR_0x8004
	buffernumberstring STR_VAR_2, VAR_0x8005
	buffernumberstring STR_VAR_3, VAR_0x8007
	switch VAR_0x8006
	case 1, EventScript_OakAideDebtCollection_InterestOnly
	case 2, EventScript_OakAideDebtCollection_Paid
	case 3, EventScript_OakAideDebtCollection_CouldntPay
	case 4, EventScript_OakAideDebtCollection_PaidOff
	end

EventScript_OakAideDebtCollection_InterestOnly::
	msgbox Text_OakAideInterestOnly
	goto EventScript_OakAideDebtCollection_Done
	end

EventScript_OakAideDebtCollection_Paid::
	playse SE_SHOP
	msgbox Text_OakAidePaid
	goto EventScript_OakAideDebtCollection_Done
	end

EventScript_OakAideDebtCollection_CouldntPay::
	playse SE_FAILURE
	msgbox Text_OakAideCouldntPay
	goto EventScript_OakAideDebtCollection_Done
	end

EventScript_OakAideDebtCollection_PaidOff::
	playfanfare MUS_LEVEL_UP
	msgbox Text_OakAidePaidOff
	waitfanfare
	goto EventScript_OakAideDebtCollection_Done
	end

EventScript_OakAideDebtCollection_Done::
	hidemoneybox
	call EventScript_RestorePrevTextColor
EventScript_OakAideDebtCollection_End::
	return

Text_OakAideBurstsIn::
	.string "Pardon the interruption!\n"
	.string "OAK FINANCIAL, debt collection!\p"
	.string "Congratulations on your BADGE.\n"
	.string "Badges mean income. Income means\l"
	.string "payments!$"

Text_OakAideInterestOnly::
	.string "I've added 14% interest.\n"
	.string "That's ¥{STR_VAR_1}.\p"
	.string "You now owe ¥{STR_VAR_3}.\p"
	.string "Payments start after your next\n"
	.string "BADGE. Start saving!$"

Text_OakAidePaid::
	.string "I've added ¥{STR_VAR_1} in interest\n"
	.string "and collected ¥{STR_VAR_2}.\p"
	.string "You still owe ¥{STR_VAR_3}.\n"
	.string "Pleasure doing business!$"

Text_OakAideCouldntPay::
	.string "Interest added: ¥{STR_VAR_1}.\p"
	.string "What? You can't pay in full?\n"
	.string "Then I'll take everything you have.\p"
	.string "…¥{STR_VAR_2}. Pathetic.\p"
	.string "Plus a ¥500 late fee.\n"
	.string "You now owe ¥{STR_VAR_3}.$"

Text_OakAidePaidOff::
	.string "I've added ¥{STR_VAR_1} in interest\n"
	.string "and collected ¥{STR_VAR_2}.\p"
	.string "…Your loan is paid off!\p"
	.string "PROF. OAK thanks you for your\n"
	.string "business. He'll miss you dearly.$"

@ ---- Pokémon Inc: gym challenge fee, paid before every attempt -----------
EventScript_GymChallengeFee::
	lock
	faceplayer
	setvar VAR_0x8004, 800
	special Economy_PrepareCost
	showmoneybox 0, 0
	msgbox Text_GymChallengeFee, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_GymChallengeFee_Declined
	special Economy_CanAfford
	goto_if_eq VAR_RESULT, FALSE, EventScript_GymChallengeFee_CantAfford
	special Economy_Pay
	updatemoneybox
	playse SE_SHOP
	msgbox Text_GymChallengeFeePaid
	hidemoneybox
	setvar VAR_RESULT, TRUE
	return

EventScript_GymChallengeFee_Declined::
	hidemoneybox
	msgbox Text_GymChallengeFeeDeclined
	setvar VAR_RESULT, FALSE
	return

EventScript_GymChallengeFee_CantAfford::
	hidemoneybox
	msgbox Text_GymChallengeFeeCantAfford
	setvar VAR_RESULT, FALSE
	return

@ ---- Pokémon Inc: news flash after every badge -----------------------------
EventScript_PriceIndexNews::
	special Economy_BufferPriceIndex
	textcolor NPC_TEXT_COLOR_NEUTRAL
	playse SE_PC_ON
	msgbox Text_PriceIndexNews
	call EventScript_RestorePrevTextColor
	return

Text_GymChallengeFee::
	.string "Before we battle, there's the\n"
	.string "challenge fee. ¥{STR_VAR_2}.\p"
	.string "The LEAGUE sets the price.\n"
	.string "Win or lose, no refunds. Pay?$"

Text_GymChallengeFeePaid::
	.string "Payment received.\n"
	.string "Now then, let's battle!$"

Text_GymChallengeFeeDeclined::
	.string "No fee, no battle.\n"
	.string "That's how the LEAGUE works.$"

Text_GymChallengeFeeCantAfford::
	.string "You can't even afford the fee?\p"
	.string "Come back when you have\n"
	.string "something to lose.$"

Text_PriceIndexNews::
	.string "KANTO NEWS FLASH!\p"
	.string "Prices rise again across KANTO!\n"
	.string "The KANTO PRICE INDEX is now\l"
	.string "{STR_VAR_2}%.\p"
	.string "Economists blame young TRAINERS\n"
	.string "with too many BADGES.$"

@ ---- Pokémon Inc: SAFFRON CITY entry pass, bought once at any gate --------
EventScript_SaffronCityPass::
	setvar VAR_0x8004, 3000
	special Economy_PrepareCost
	showmoneybox 0, 0
	msgbox Text_SaffronCityPass, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_SaffronCityPass_No
	special Economy_CanAfford
	goto_if_eq VAR_RESULT, FALSE, EventScript_SaffronCityPass_No
	special Economy_Pay
	updatemoneybox
	playse SE_SHOP
	waitse
	hidemoneybox
	setvar VAR_RESULT, TRUE
	return

EventScript_SaffronCityPass_No::
	hidemoneybox
	setvar VAR_RESULT, FALSE
	return

Text_SaffronCityPass::
	.string "SAFFRON CITY is a premium\n"
	.string "residential district.\p"
	.string "Visitors need a CITY PASS.\n"
	.string "One-time fee: ¥{STR_VAR_2}.\p"
	.string "It works at every gate.\n"
	.string "Buy a pass?$"

Text_CyclingRoadToll::
	.string "CYCLING ROAD is a toll road.\p"
	.string "That's ¥{STR_VAR_2} per ride.\n"
	.string "The ride down is free. Pay?$"

Text_CyclingRoadTollPaid::
	.string "Toll paid. Enjoy the ride!\n"
	.string "Helmets are sold separately.$"

Text_CyclingRoadTollCantAfford::
	.string "You can't afford the toll.\n"
	.string "The long way is free!$"

@ ---- Pokémon Inc: HMs are sold, not given. VAR_0x8004 = base price,
@ VAR_0x8005 = HM item. Returns VAR_RESULT TRUE if bought.
EventScript_SellHM::
	bufferitemname STR_VAR_3, VAR_0x8005
	special Economy_PrepareCost
	showmoneybox 0, 0
	msgbox Text_SellHM, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_SellHM_No
	special Economy_CanAfford
	goto_if_eq VAR_RESULT, FALSE, EventScript_SellHM_CantAfford
	special Economy_Pay
	updatemoneybox
	playse SE_SHOP
	waitse
	hidemoneybox
	setvar VAR_RESULT, TRUE
	return

EventScript_SellHM_CantAfford::
	hidemoneybox
	msgbox Text_SellHMCantAfford
	setvar VAR_RESULT, FALSE
	return

EventScript_SellHM_No::
	hidemoneybox
	msgbox Text_SellHMDeclined
	setvar VAR_RESULT, FALSE
	return

Text_SellHM::
	.string "Of course, the {STR_VAR_3} isn't\n"
	.string "free. It's ¥{STR_VAR_2}.\p"
	.string "Think of it as a license. Use it\n"
	.string "without teaching it to anyone.\p"
	.string "No POKéMON has to forget a thing.\n"
	.string "Deal?$"

Text_SellHMCantAfford::
	.string "You can't afford it?\n"
	.string "Then the way stays blocked.$"

Text_SellHMDeclined::
	.string "Suit yourself. Come back when\n"
	.string "you're ready to invest.$"
