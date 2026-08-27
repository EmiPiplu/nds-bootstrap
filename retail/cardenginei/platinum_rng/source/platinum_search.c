#include "platinum_search.h"

bool platinumSearchSpeciesAllowed(
	RngMethod method,
	u16 species
) {
	if (species == SPECIES_ANY) {
		return true;
	}

	switch (method) {
		case RNG_METHOD_STARTER:
			return
				species == SPECIES_TURTWIG ||
				species == SPECIES_CHIMCHAR ||
				species == SPECIES_PIPLUP;

		case RNG_METHOD_WILD:
			/*
			 * Proper area/encounter-table
			 * filtering comes later.
			 */
			return true;

		case RNG_METHOD_STATIONARY:
			/*
			 * Proper stationary encounter
			 * filtering comes later.
			 */
			return true;

		default:
			return false;
	}
}

SearchConfigResult platinumSearchValidate(
	const SearchConfig *config
) {
	if (!platinumSearchSpeciesAllowed(
		config->method,
		config->species
	)) {
		return SEARCH_CONFIG_INVALID_SPECIES_METHOD;
	}

	return SEARCH_CONFIG_OK;
}