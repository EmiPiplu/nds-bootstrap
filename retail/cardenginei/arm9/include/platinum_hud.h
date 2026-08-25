#ifndef PLATINUM_HUD_H
#define PLATINUM_HUD_H

#include <nds/ndstypes.h>
#include <stdbool.h>

void platinumHudEnter(
	u32 currentRng,
	bool haveInitialSeed,
	u32 initialSeed,
	u32 advances
);

void platinumHudLeave(void);

#endif