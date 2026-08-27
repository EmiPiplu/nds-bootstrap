#include <nds/ndstypes.h>

#include "platinum_rng_api.h"
#include "platinum_hud.h"

#define PLATINUM_RNG_ADDR 0x021BFB14

#define PLATINUM_SHARED_MAGIC 9
#define PLATINUM_SHARED_SEED  10

#define PLATINUM_RNG_MAGIC 0x50474E52

static vu32 *const sharedAddr =
	(vu32 *)CARDENGINE_SHARED_ADDRESS_SDK1;

static void platinumPayloadEnter(void);

static void platinumPayloadLeave(void);

__attribute__((section(".header"), used))
const PlatinumRngApi platinumRngApi = {
	.magic   = PLATINUM_RNG_API_MAGIC,
	.version = PLATINUM_RNG_API_VERSION,
	.enter   = platinumPayloadEnter,
	.leave   = platinumPayloadLeave,
};

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

	platinumHudEnter(
		current,
		haveInitialSeed,
		initialSeed,
		advances
	);
}

static void platinumPayloadLeave(void) {
	platinumHudLeave();
}