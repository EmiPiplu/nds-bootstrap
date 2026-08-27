#ifndef PLATINUM_HUD_H
#define PLATINUM_HUD_H

#include <nds/ndstypes.h>
#include <stdbool.h>

void platinumHudEnter(void);
void platinumHudLeave(void);

void platinumHudDrawMethod(
	u8 selection
);

void platinumHudDrawSetup(
	u8 selection,
	const char *speciesName,
	const char *methodName
);

void platinumHudDrawTracking(
	u32 currentRng,
	bool haveInitialSeed,
	u32 initialSeed,
	u32 advances
);

void platinumHudDrawPokemon(
	u8 selection,
	const char *const *items,
	u8 count
);

#endif