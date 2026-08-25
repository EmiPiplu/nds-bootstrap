#ifndef PLATINUM_HUD_H
#define PLATINUM_HUD_H

#include <nds/ndstypes.h>
#include <stdbool.h>

#ifdef DLDI

static inline void platinumHudEnter(
	u32 currentRng,
	bool haveInitialSeed,
	u32 initialSeed,
	u32 advances
) {
	(void)currentRng;
	(void)haveInitialSeed;
	(void)initialSeed;
	(void)advances;
}

static inline void platinumHudLeave(void) {
}

#else

void platinumHudEnter(
	u32 currentRng,
	bool haveInitialSeed,
	u32 initialSeed,
	u32 advances
);

void platinumHudLeave(void);

#endif

#endif