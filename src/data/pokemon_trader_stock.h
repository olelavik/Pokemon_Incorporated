// Pokémon Incorporated: what the Pokémon trader sells in each Pokémon Center.
//
// Levels sit a few above the local wild Pokémon but below the next gym
// leader's strongest Pokémon: buying saves you grinding, it doesn't skip the gym.
// PREMIUM marks counters for the next gym, sold at double markup.
// minBadges hides an entry until you have that many badges (Celadon grows
// into the big late-game market this way).
//
// Fields: { species, level, minBadges, premium }
#define STOCK(species, level)               { SPECIES_##species, level, 0, FALSE }
#define PREMIUM(species, level)             { SPECIES_##species, level, 0, TRUE }
#define STOCK_AT(badges, species, level)    { SPECIES_##species, level, badges, FALSE }
#define PREMIUM_AT(badges, species, level)  { SPECIES_##species, level, badges, TRUE }

// Next gym: Brock (Rock). Wild Pokémon here are Lv. 2-5.
static const struct TraderStock sStock_Viridian[] = {
    STOCK(PIDGEY, 6),
    STOCK(RATTATA, 6),
    STOCK(SPEAROW, 6),
    PREMIUM(MANKEY, 8),
    PREMIUM(ODDISH, 7),
};

// Next gym: Misty (Water). Brock's ace is Lv. 14.
static const struct TraderStock sStock_Pewter[] = {
    STOCK(MAGIKARP, 5),
    STOCK(SPEAROW, 10),
    STOCK(JIGGLYPUFF, 10),
    STOCK(NIDORAN_F, 10),
    PREMIUM(ODDISH, 11),
    PREMIUM(PIKACHU, 11),
};

// Mt. Moon's exit. Next gym: Misty (Water).
static const struct TraderStock sStock_Route4[] = {
    STOCK(ZUBAT, 11),
    STOCK(EKANS, 12),
    STOCK(CLEFAIRY, 12),
    PREMIUM(PARAS, 12),
};

// Next gym: Lt. Surge (Electric). Misty's ace is Lv. 21.
static const struct TraderStock sStock_Cerulean[] = {
    STOCK(MAGIKARP, 10),
    STOCK(ABRA, 12),
    STOCK(MEOWTH, 15),
    STOCK(BELLSPROUT, 15),
    PREMIUM(SANDSHREW, 16),
    PREMIUM(DIGLETT, 16),
};

// Next gym: Erika (Grass). Surge's ace is Lv. 24.
static const struct TraderStock sStock_Vermilion[] = {
    STOCK(SPEAROW, 18),
    STOCK(KRABBY, 18),
    STOCK(DROWZEE, 18),
    PREMIUM(GROWLITHE, 20),
    PREMIUM(PONYTA, 20),
};

static const struct TraderStock sStock_Route10[] = {
    STOCK(EKANS, 18),
    STOCK(SPEAROW, 18),
    STOCK(VOLTORB, 18),
    STOCK(MACHOP, 18),
};

static const struct TraderStock sStock_Lavender[] = {
    STOCK(PSYDUCK, 22),
    STOCK(GASTLY, 22),
    STOCK(CUBONE, 22), // don't ask where it came from
    STOCK(GROWLITHE, 22),
};

// The big market. Grows with every badge.
static const struct TraderStock sStock_Celadon[] = {
    STOCK(MAGIKARP, 15),
    STOCK(ABRA, 20),
    STOCK(MEOWTH, 25),
    STOCK(VENONAT, 25),
    STOCK(EEVEE, 25),
    STOCK(VULPIX, 25),
    STOCK(PONYTA, 25),
    STOCK(GROWLITHE, 25),
    PREMIUM(DROWZEE, 26),      // vs Koga
    PREMIUM(KADABRA, 28),      // vs Koga
    STOCK_AT(4, DRATINI, 25),
    STOCK_AT(4, TAUROS, 30),
    STOCK_AT(4, KANGASKHAN, 30),
    STOCK_AT(4, HITMONLEE, 30),
    STOCK_AT(4, HITMONCHAN, 30),
    PREMIUM_AT(4, SCYTHER, 30), // vs Sabrina
    PREMIUM_AT(4, PINSIR, 30),  // vs Sabrina
    STOCK_AT(5, JOLTEON, 35),
    STOCK_AT(5, FLAREON, 35),
    PREMIUM_AT(5, VAPOREON, 35), // vs Blaine
    PREMIUM_AT(5, LAPRAS, 35),   // vs Blaine
    STOCK_AT(6, ALAKAZAM, 38),
    STOCK_AT(6, ARCANINE, 38),
    STOCK_AT(6, MACHAMP, 40),
    STOCK_AT(6, GENGAR, 40),
    PREMIUM_AT(6, GYARADOS, 38), // vs Giovanni
    STOCK_AT(7, DRAGONAIR, 40),
    STOCK_AT(7, AERODACTYL, 42),
    STOCK_AT(7, SNORLAX, 42),
    STOCK_AT(8, DRAGONITE, 55),
};

// Next gym: Sabrina (Psychic). Koga's ace is Lv. 43.
static const struct TraderStock sStock_Fuchsia[] = {
    STOCK(VENONAT, 30),
    STOCK(NIDORINA, 30),
    STOCK(NIDORINO, 30),
    STOCK(RHYHORN, 30),
    PREMIUM(SCYTHER, 32),
};

// Next gym: Blaine (Fire). Sabrina's ace is Lv. 43.
static const struct TraderStock sStock_Saffron[] = {
    STOCK(PIDGEOTTO, 32),
    STOCK(DROWZEE, 32),
    STOCK(PSYDUCK, 32),
    PREMIUM(KRABBY, 32),
    PREMIUM(SEEL, 34),
};

// Next gym: Giovanni (Ground). Blaine's ace is Lv. 47.
static const struct TraderStock sStock_Cinnabar[] = {
    STOCK(PONYTA, 38),
    STOCK(GROWLITHE, 38),
    STOCK(MAGMAR, 38),
    PREMIUM(STARYU, 38),
    PREMIUM(TANGELA, 38),
};

// Last stop before the Elite Four: desperation pricing.
static const struct TraderStock sStock_IndigoPlateau[] = {
    PREMIUM(GYARADOS, 45),
    PREMIUM(LAPRAS, 45),
    PREMIUM(ARCANINE, 45),
    PREMIUM(ALAKAZAM, 45),
    PREMIUM(MACHAMP, 45),
    PREMIUM(DRAGONAIR, 45),
};

// Sevii Islands, all one chain.
static const struct TraderStock sStock_Sevii[] = {
    STOCK(SLOWPOKE, 35),
    STOCK(PSYDUCK, 35),
    STOCK(TEDDIURSA, 35),
    STOCK(PHANPY, 35),
    STOCK(HOUNDOUR, 35),
    STOCK(SNEASEL, 35),
    PREMIUM(LARVITAR, 30),
};

#define SHOP(mapConst, stockArray) { mapConst, stockArray, ARRAY_COUNT(stockArray) }

static const struct TraderShop sTraderShops[] = {
    SHOP(MAP_VIRIDIAN_CITY_POKEMON_EXCHANGE, sStock_Viridian),
    SHOP(MAP_PEWTER_CITY_POKEMON_EXCHANGE, sStock_Pewter),
    SHOP(MAP_ROUTE4_POKEMON_CENTER_1F, sStock_Route4),
    SHOP(MAP_CERULEAN_CITY_POKEMON_EXCHANGE, sStock_Cerulean),
    SHOP(MAP_VERMILION_CITY_POKEMON_CENTER_1F, sStock_Vermilion),
    SHOP(MAP_ROUTE10_POKEMON_CENTER_1F, sStock_Route10),
    SHOP(MAP_LAVENDER_TOWN_POKEMON_CENTER_1F, sStock_Lavender),
    SHOP(MAP_CELADON_CITY_POKEMON_EXCHANGE, sStock_Celadon),
    SHOP(MAP_FUCHSIA_CITY_POKEMON_CENTER_1F, sStock_Fuchsia),
    SHOP(MAP_SAFFRON_CITY_POKEMON_CENTER_1F, sStock_Saffron),
    SHOP(MAP_CINNABAR_ISLAND_POKEMON_CENTER_1F, sStock_Cinnabar),
    SHOP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_1F, sStock_IndigoPlateau),
    SHOP(MAP_ONE_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
    SHOP(MAP_TWO_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
    SHOP(MAP_THREE_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
    SHOP(MAP_FOUR_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
    SHOP(MAP_FIVE_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
    SHOP(MAP_SIX_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
    SHOP(MAP_SEVEN_ISLAND_POKEMON_CENTER_1F, sStock_Sevii),
};
