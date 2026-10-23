IndigoPlateau_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, IndigoPlateau_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

IndigoPlateau_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_INDIGO_PLATEAU
	specialvar VAR_RESULT, IsNationalPokedexEnabled
	call_if_eq VAR_RESULT, TRUE, IndigoPlateau_PokemonCenter_1F_EventScript_CheckBlockDoor
	end

IndigoPlateau_PokemonCenter_1F_EventScript_CheckBlockDoor::
	goto_if_set FLAG_SYS_CAN_LINK_WITH_RS, EventScript_Return
	setobjectxyperm LOCALID_LEAGUE_DOOR_GUARD, 4, 2
	return

IndigoPlateau_PokemonCenter_1F_EventScript_DoorGuard::
	lock
	faceplayer
	specialvar VAR_RESULT, IsNationalPokedexEnabled
	goto_if_eq VAR_RESULT, TRUE, IndigoPlateau_PokemonCenter_1F_EventScript_CheckSeviiIslandComplete
	msgbox IndigoPlateau_PokemonCenter_1F_Text_FaceEliteFourGoodLuck
	release
	end

IndigoPlateau_PokemonCenter_1F_EventScript_CheckSeviiIslandComplete::
	goto_if_set FLAG_SYS_CAN_LINK_WITH_RS, IndigoPlateau_PokemonCenter_1F_EventScript_SeviiIslandComplete
	msgbox IndigoPlateau_PokemonCenter_1F_Text_LoreleiIsAbsentClosedForTimeBeing
	release
	end

IndigoPlateau_PokemonCenter_1F_EventScript_SeviiIslandComplete::
	msgbox IndigoPlateau_PokemonCenter_1F_Text_FaceEliteFourGoodLuck
	release
	end

IndigoPlateau_PokemonCenter_1F_EventScript_GymGuy::
	msgbox IndigoPlateau_PokemonCenter_1F_Text_GymGuyAdvice, MSGBOX_NPC
	end

IndigoPlateau_PokemonCenter_1F_EventScript_Clerk::
	goto_if_questlog EventScript_ReleaseEnd
	lock
	faceplayer
	message Text_MayIHelpYou
	waitmessage
	pokemart IndigoPlateau_PokemonCenter_1F_Items
	msgbox Text_PleaseComeAgain
	release
	end

	.align 2
IndigoPlateau_PokemonCenter_1F_Items::
	.2byte ITEM_ULTRA_BALL
	.2byte ITEM_GREAT_BALL
	.2byte ITEM_FULL_RESTORE
	.2byte ITEM_MAX_POTION
	.2byte ITEM_REVIVE
	.2byte ITEM_FULL_HEAL
	.2byte ITEM_MAX_REPEL
	.2byte ITEM_NONE
	release
	end

IndigoPlateau_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse
	release
	end

@ Pokémon Inc: the POKéMON LEAGUE entry fee, paid on every attempt.
IndigoPlateau_PokemonCenter_1F_EventScript_LeagueFee::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_LEAGUE_DOOR_GUARD, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	setvar VAR_0x8004, 1000
	special Economy_PrepareFlatCost
	showmoneybox 0, 0
	msgbox IndigoPlateau_PokemonCenter_1F_Text_LeagueFee, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, IndigoPlateau_PokemonCenter_1F_EventScript_LeagueFeeTurnBack
	special Economy_CanAfford
	goto_if_eq VAR_RESULT, FALSE, IndigoPlateau_PokemonCenter_1F_EventScript_LeagueFeeCantAfford
	special Economy_Pay
	updatemoneybox
	playse SE_SHOP
	msgbox IndigoPlateau_PokemonCenter_1F_Text_LeagueFeePaid
	hidemoneybox
	setvar VAR_TEMP_1, 1
	releaseall
	end

IndigoPlateau_PokemonCenter_1F_EventScript_LeagueFeeCantAfford::
	msgbox IndigoPlateau_PokemonCenter_1F_Text_LeagueFeeCantAfford
IndigoPlateau_PokemonCenter_1F_EventScript_LeagueFeeTurnBack::
	hidemoneybox
	closemessage
	applymovement LOCALID_PLAYER, IndigoPlateau_PokemonCenter_1F_Movement_StepBack
	waitmovement 0
	releaseall
	end

IndigoPlateau_PokemonCenter_1F_Movement_StepBack::
	walk_down
	step_end
