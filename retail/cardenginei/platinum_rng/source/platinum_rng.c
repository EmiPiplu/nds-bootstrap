#include <nds/ndstypes.h>
#include <nds/input.h>

#include "platinum_rng_api.h"
#include "platinum_hud.h"

#define PLATINUM_RNG_ADDR 0x021BFB14

#define PLATINUM_SHARED_MAGIC 9
#define PLATINUM_SHARED_SEED  10

#define PLATINUM_RNG_MAGIC 0x50474E52

typedef enum {
	PLATINUM_SCREEN_SETUP,
	PLATINUM_SCREEN_TRACKING,
} PlatinumScreen;

static PlatinumScreen currentScreen =
	PLATINUM_SCREEN_SETUP;

static u8 setupSelection = 0;

static PlatinumPayloadAction platinumPayloadUpdate(u16 keys);

static u16 previousKeys = 0;

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
	.update  = platinumPayloadUpdate,
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
	previousKeys = 0;

	platinumHudEnter();

	if (currentScreen == PLATINUM_SCREEN_SETUP) {
		platinumHudDrawSetup(
			setupSelection
		);
	} else {
		platinumPayloadDrawTracking();
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

static PlatinumPayloadAction platinumPayloadUpdate(u16 keys)
{
	u16 released =
		previousKeys & ~keys;

	previousKeys = keys;

	if (released & KEY_START) {
		return PLATINUM_PAYLOAD_RESUME;
	}

	if (released & KEY_R) {
		return PLATINUM_PAYLOAD_STEP;
	}

	if (currentScreen == PLATINUM_SCREEN_SETUP) {
		if (pressed & KEY_UP) {
			if (setupSelection == 0) {
				setupSelection = 3;
			} else {
				setupSelection--;
			}

			platinumHudDrawSetup(
				setupSelection
			);
		}

		if (pressed & KEY_DOWN) {
			setupSelection =
				(setupSelection + 1) % 4;

			platinumHudDrawSetup(
				setupSelection
			);
		}

		if (
			(pressed & KEY_A) &&
			setupSelection == 3
		) {
			currentScreen =
				PLATINUM_SCREEN_TRACKING;

			platinumPayloadDrawTracking();
		}
	} else {
		if (pressed & KEY_B) {
			currentScreen =
				PLATINUM_SCREEN_SETUP;

			platinumHudDrawSetup(
				setupSelection
			);
		}
	}

	return PLATINUM_PAYLOAD_NONE;
}