#ifndef PLATINUM_HUD_H
#define PLATINUM_HUD_H

#include <nds/ndstypes.h>
#include <stdbool.h>

void platinumHudEnter(void);
void platinumHudLeave(void);

void platinumHudDrawSetup(u8 selection);

void platinumHudDrawTracking(
	u32 currentRng,
	bool haveInitialSeed,
	u32 initialSeed,
	u32 advances
);

#endif