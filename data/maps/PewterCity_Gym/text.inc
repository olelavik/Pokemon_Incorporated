PewterCity_Gym_Text_BrockIntro::
    .string "So, you're here. I'm BROCK.\n"
    .string "I'm PEWTER's GYM LEADER.\p"
    .string "I have nine brothers and sisters\n"
    .string "to feed.\p"
    .string "So I battle for money. Lots of it.\p"
    .string "My POKéMON are all ROCK type.\n"
    .string "Rock-solid investments!\p"
    .string "Fuhaha! You're challenging me,\n"
    .string "knowing you'll lose your money?\p"
    .string "Fine, then!\n"
    .string "Show me your wallet!{PLAY_BGM}{MUS_ENCOUNTER_GYM_LEADER}$"

@ NOTE: This defeat text actually causes a buffer overflow. It's too long for the gDisplayedStringBattle
@ buffer that it's put into, and it stomps all over the gBattleTextBuffs after, as well as the otherwise
@ unused array after that, sFlickerArray. Perhaps that's the reason why said array exists.
PewterCity_Gym_Text_BrockDefeat::
    .string "I took you for granted, and so\n"
    .string "I lost.\p"
    .string "As proof of your victory, I confer\n"
    .string "on you this…the official POKéMON\l"
    .string "LEAGUE BOULDERBADGE.\p"
    .string "{FONT_NORMAL}{PLAYER} received the BOULDERBADGE\n"
    .string "from BROCK!{PAUSE_MUSIC}{PLAY_BGM}{MUS_OBTAIN_BADGE}{PAUSE 0xFE}{PAUSE 0x56}{RESUME_MUSIC}\p"
    .string "{FONT_MALE}Just having the BOULDERBADGE makes\n"
    .string "your POKéMON more powerful.\p"
    .string "It also enables the use of the\n"
    .string "move FLASH outside of battle.\p"
    .string "Of course, a POKéMON must know the\n"
    .string "move FLASH to use it.$"

PewterCity_Gym_Text_TakeThisWithYou::
    .string "Wait!\n"
    .string "The LEAGUE makes me hand this out.$"

PewterCity_Gym_Text_ReceivedTM39FromBrock::
    .string "{PLAYER} received TM39\n"
    .string "from BROCK.$"

PewterCity_Gym_Text_ExplainTM39::
    .string "A TM teaches a POKéMON a move.\n"
    .string "It only works once.\p"
    .string "TM39 contains ROCK TOMB.\n"
    .string "It hurls boulders at the foe.\p"
    .string "A POKéMON with a good move sells\n"
    .string "for more. Remember that!$"

PewterCity_Gym_Text_BrockPostBattle::
    .string "Losing money stings.\n"
    .string "But I'll make it back from the\l"
    .string "next challenger.\p"
    .string "You have a talent for this,\n"
    .string "kid. A money talent.\p"
    .string "The CERULEAN GYM pays better.\n"
    .string "Go test yourself there.$"

PewterCity_Gym_Text_DontHaveRoomForThis::
    .string "You don't have room for this.$"

PewterCity_Gym_Text_LiamIntro::
    .string "Stop right there, kid!\p"
    .string "You're ten thousand dollars\n"
    .string "away from facing BROCK!$"

PewterCity_Gym_Text_LiamDefeat::
    .string "Darn!\p"
    .string "That was my rent money!$"

PewterCity_Gym_Text_LiamPostBattle::
    .string "You're pretty good.\n"
    .string "…But BROCK's richer!$"

PewterCity_Gym_Text_LetMeTakeYouToTheTop::
    .string "Hiya!\n"
    .string "Do you want to get rich?\p"
    .string "Do you dare to dream of the\n"
    .string "CHAMPION's prize money?\p"
    .string "I'm no TRAINER, but I sell advice.\n"
    .string "Let me take you to the top!$"

PewterCity_Gym_Text_LetsGetHappening::
    .string "All right!\n"
    .string "Let's get happening!$"

PewterCity_Gym_Text_TryDifferentPartyOrders::
    .string "The first POKéMON out in a match is\n"
    .string "at the left of the POKéMON LIST.\p"
    .string "By changing the order of POKéMON,\n"
    .string "you may gain an advantage.\p"
    .string "Try different orders to suit your\n"
    .string "opponent's party.$"

PewterCity_Gym_Text_ItsFreeLetsGetHappening::
    .string "The first tip is free!\n"
    .string "The rest are on a subscription.$"

PewterCity_Gym_Text_YoureChampMaterial::
    .string "Just as I thought!\n"
    .string "You're POKéMON tycoon material!$"

PewterCity_Gym_Text_GymStatue::
    .string "PEWTER POKéMON GYM\n"
    .string "LEADER: BROCK\p"
    .string "WINNING TRAINERS:\n"
    .string "{RIVAL}$"

PewterCity_Gym_Text_GymStatuePlayerWon::
    .string "PEWTER POKéMON GYM\n"
    .string "LEADER: BROCK\p"
    .string "WINNING TRAINERS:\n"
    .string "{RIVAL}, {PLAYER}$"

