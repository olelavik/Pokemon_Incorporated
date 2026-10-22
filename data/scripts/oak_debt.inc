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
