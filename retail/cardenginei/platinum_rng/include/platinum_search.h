#ifndef PLATINUM_SEARCH_H
#define PLATINUM_SEARCH_H

#include <nds/ndstypes.h>
#include <stdbool.h>

typedef enum {
	RNG_METHOD_STARTER = 0,
	RNG_METHOD_WILD,
	RNG_METHOD_STATIONARY,

	RNG_METHOD_COUNT
} RngMethod;

typedef enum {
	SPECIES_ANY      = 0,

	SPECIES_TURTWIG  = 387,
	SPECIES_CHIMCHAR = 390,
	SPECIES_PIPLUP   = 393,
	SPECIES_BUNEARY  = 427,
} PokemonSpecies;

typedef struct {
	RngMethod method;
	u16 species;
} SearchConfig;

typedef enum {
	SEARCH_CONFIG_OK = 0,
	SEARCH_CONFIG_INVALID_SPECIES_METHOD,
} SearchConfigResult;

bool platinumSearchSpeciesAllowed(
	RngMethod method,
	u16 species
);

SearchConfigResult platinumSearchValidate(
	const SearchConfig *config
);

#endif