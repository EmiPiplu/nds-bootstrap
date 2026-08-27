#include <nds/ndstypes.h>
#include <nds/input.h>

#include "platinum_rng_api.h"
#include "platinum_hud.h"
#include "platinum_search.h"

#define PLATINUM_RNG_ADDR 0x021BFB14

#define PLATINUM_SHARED_MAGIC 9
#define PLATINUM_SHARED_SEED  10

#define PLATINUM_RNG_MAGIC 0x50474E52

typedef struct {
	u16 held;
	u16 pressed;
	u16 released;
} PlatinumInput;

typedef enum {
	PLATINUM_SCREEN_SETUP,
	PLATINUM_SCREEN_POKEMON,
	PLATINUM_SCREEN_METHOD,
	PLATINUM_SCREEN_TRACKING,
} PlatinumScreen;

static u8 methodSelection = 0;

static SearchConfig searchConfig = {
	.method = RNG_METHOD_STARTER,
	.species = SPECIES_ANY,
};

static const PokemonSpecies pokemonOptions[] = {
	SPECIES_ANY,
	SPECIES_TURTWIG,
	SPECIES_CHIMCHAR,
	SPECIES_PIPLUP,
	SPECIES_BUNEARY,
};

static const char *platinumSpeciesName(
	u16 species
) {
	switch (species) {
		case SPECIES_ANY:
			return "ANY";

		case SPECIES_TURTWIG:
			return "TURTWIG";

		case SPECIES_CHIMCHAR:
			return "CHIMCHAR";

		case SPECIES_PIPLUP:
			return "PIPLUP";

		case SPECIES_BUNEARY:
			return "BUNEARY";

		default:
			return "UNKNOWN";
	}
}

#define POKEMON_OPTION_COUNT \
	(sizeof(pokemonOptions) / sizeof(pokemonOptions[0]))

static u8 pokemonSelection = 0;

static PlatinumScreen currentScreen =
	PLATINUM_SCREEN_SETUP;

static u8 setupSelection = 0;

static PlatinumPayloadAction platinumPayloadUpdate(u16 keys);

static u16 previousKeys = 0;

static vu32 *const sharedAddr =
	(vu32 *)CARDENGINE_SHARED_ADDRESS_SDK1;

static void platinumPayloadEnter(void);
static void platinumPayloadLeave(void);
static void platinumPayloadDrawTracking(void);

static void platinumUpdateSetup(
	const PlatinumInput *input
);

static void platinumUpdateTracking(
	const PlatinumInput *input
);

static void platinumUpdateMethod(
	const PlatinumInput *input
);

__attribute__((section(".header"), used))
const PlatinumRngApi platinumRngApi = {
	.magic   = PLATINUM_RNG_API_MAGIC,
	.version = PLATINUM_RNG_API_VERSION,
	.enter   = platinumPayloadEnter,
	.leave   = platinumPayloadLeave,
	.update  = platinumPayloadUpdate,
};

static const char *platinumMethodName(
	RngMethod method
) {
	switch (method) {
		case RNG_METHOD_STARTER:
			return "STARTER";

		case RNG_METHOD_WILD:
			return "WILD";

		case RNG_METHOD_STATIONARY:
			return "STATIONARY";

		default:
			return "UNKNOWN";
	}
}

static void platinumPayloadDrawSetup(void)
{
	platinumHudDrawSetup(
		setupSelection,
		platinumSpeciesName(
			searchConfig.species
		),
		platinumMethodName(
			searchConfig.method
		)
	);
}

static void platinumMovePokemonSelection(
	int direction
) {
	do {
		if (direction < 0) {
			if (pokemonSelection == 0) {
				pokemonSelection =
					POKEMON_OPTION_COUNT - 1;
			} else {
				pokemonSelection--;
			}
		} else {
			pokemonSelection =
				(pokemonSelection + 1) %
				POKEMON_OPTION_COUNT;
		}
	} while (
		!platinumSearchSpeciesAllowed(
			searchConfig.method,
			pokemonOptions[
				pokemonSelection
			]
		)
	);
}

static void platinumPayloadDrawPokemon(void)
{
	const char *items[POKEMON_OPTION_COUNT];

	u8 count = 0;
	u8 visibleSelection = 0;

	for (u8 i = 0;
	     i < POKEMON_OPTION_COUNT;
	     i++) {

		if (!platinumSearchSpeciesAllowed(
			searchConfig.method,
			pokemonOptions[i]
		)) {
			continue;
		}

		if (i == pokemonSelection) {
			visibleSelection = count;
		}

		items[count++] =
			platinumSpeciesName(
				pokemonOptions[i]
			);
	}

	platinumHudDrawPokemon(
		visibleSelection,
		items,
		count
	);
}

static u32 platinumRngDistance(
	u32 state,
	u32 target
) {
	u32 curMult = 0x41C64E6D;
	u32 curPlus = 0x6073;
	u32 distance = 0;

	for (u32 bit = 1;
	     bit != 0;
	     bit <<= 1) {

		if ((state & bit) !=
		    (target & bit)) {

			state =
				state * curMult +
				curPlus;

			distance |= bit;
		}

		curPlus *= curMult + 1;
		curMult *= curMult;
	}

	return distance;
}

static void platinumPayloadEnter(void)
{
	previousKeys = 0;

	platinumHudEnter();

	switch (currentScreen) {
		case PLATINUM_SCREEN_SETUP:
			platinumPayloadDrawSetup();
			break;

		case PLATINUM_SCREEN_METHOD:
			platinumHudDrawMethod(
				methodSelection
			);
			break;

		case PLATINUM_SCREEN_TRACKING:
			platinumPayloadDrawTracking();
			break;
		case PLATINUM_SCREEN_POKEMON:
			platinumPayloadDrawPokemon();
			break;
	}
}

static void platinumPayloadDrawTracking(void)
{
	u32 current =
		*(vu32 *)PLATINUM_RNG_ADDR;

	bool haveInitialSeed =
		sharedAddr[PLATINUM_SHARED_MAGIC] ==
			PLATINUM_RNG_MAGIC;

	u32 initialSeed = 0;
	u32 advances = 0;

	if (haveInitialSeed) {
		initialSeed =
			sharedAddr[PLATINUM_SHARED_SEED];

		advances =
			platinumRngDistance(
				initialSeed,
				current
			);
	}

	platinumHudDrawTracking(
		current,
		haveInitialSeed,
		initialSeed,
		advances
	);
}

static void platinumPayloadLeave(void) {
	platinumHudLeave();
}

static void platinumUpdateSetup(
	const PlatinumInput *input
) {
	if (input->pressed & KEY_UP) {
		if (setupSelection == 0) {
			setupSelection = 3;
		} else {
			setupSelection--;
		}

		platinumPayloadDrawSetup();
	}

	if (input->pressed & KEY_DOWN) {
		setupSelection =
			(setupSelection + 1) % 4;

		platinumPayloadDrawSetup();
	}

	if (
		(input->pressed & KEY_A) &&
		setupSelection == 1
	) {
		methodSelection =
			(u8)searchConfig.method;

		currentScreen =
			PLATINUM_SCREEN_METHOD;

		platinumHudDrawMethod(
			methodSelection
		);

		return;
	}

	if (
		(input->pressed & KEY_A) &&
		setupSelection == 3
	) {
		if (
			platinumSearchValidate(
				&searchConfig
			) != SEARCH_CONFIG_OK
		) {
			return;
		}

		currentScreen =
			PLATINUM_SCREEN_TRACKING;

		platinumPayloadDrawTracking();
	}
}

static void platinumUpdateTracking(
	const PlatinumInput *input
) {
	if (input->pressed & KEY_B) {
		currentScreen =
			PLATINUM_SCREEN_SETUP;

		platinumPayloadDrawSetup();
	}
}

static void platinumUpdateMethod(
	const PlatinumInput *input
) {
	if (input->pressed & KEY_UP) {
		if (methodSelection == 0) {
			methodSelection =
				RNG_METHOD_COUNT - 1;
		} else {
			methodSelection--;
		}

		platinumHudDrawMethod(
			methodSelection
		);
	}

	if (input->pressed & KEY_DOWN) {
		methodSelection =
			(methodSelection + 1) %
			RNG_METHOD_COUNT;

		platinumHudDrawMethod(
			methodSelection
		);
	}

	if (input->pressed & KEY_A) {
		searchConfig.method =
			(RngMethod)methodSelection;

		currentScreen =
			PLATINUM_SCREEN_SETUP;

		platinumPayloadDrawSetup();
		return;
	}

	if (input->pressed & KEY_B) {
		currentScreen =
			PLATINUM_SCREEN_SETUP;

		platinumPayloadDrawSetup();
	}
}

static void platinumUpdatePokemon(
	const PlatinumInput *input
) {
	if (input->pressed & KEY_UP) {
		platinumMovePokemonSelection(-1);
		platinumPayloadDrawPokemon();
	}

	if (input->pressed & KEY_DOWN) {
		platinumMovePokemonSelection(1);
		platinumPayloadDrawPokemon();
	}

	if (input->pressed & KEY_A) {
		searchConfig.species =
			pokemonOptions[
				pokemonSelection
			];

		currentScreen =
			PLATINUM_SCREEN_SETUP;

		platinumPayloadDrawSetup();
		return;
	}

	if (input->pressed & KEY_B) {
		currentScreen =
			PLATINUM_SCREEN_SETUP;

		platinumPayloadDrawSetup();
	}
}

static PlatinumPayloadAction platinumPayloadUpdate(u16 keys)
{
	PlatinumInput input = {
		.held = keys,
		.pressed = keys & ~previousKeys,
		.released = previousKeys & ~keys,
	};

	previousKeys = keys;

	/*
	 * Global debugger controls.
	 *
	 * These work regardless of which menu screen
	 * we're currently looking at.
	 */
	if (input.released & KEY_START) {
		return PLATINUM_PAYLOAD_RESUME;
	}

	if (input.released & KEY_R) {
		return PLATINUM_PAYLOAD_STEP;
	}

	/*
	 * Everything else belongs to the active screen.
	 */
	switch (currentScreen) {
		case PLATINUM_SCREEN_SETUP:
			platinumUpdateSetup(&input);
			break;

		case PLATINUM_SCREEN_METHOD:
			platinumUpdateMethod(&input);
			break;

		case PLATINUM_SCREEN_TRACKING:
			platinumUpdateTracking(&input);
			break;
		case PLATINUM_SCREEN_POKEMON:
			platinumUpdatePokemon(&input);
			break;
	}

	return PLATINUM_PAYLOAD_NONE;
}