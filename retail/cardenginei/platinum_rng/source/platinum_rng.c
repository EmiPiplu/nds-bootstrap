#include <nds/ndstypes.h>

#include "platinum_rng_api.h"

static void platinumPayloadEnter(vu32 *sharedAddr) {
	(void)sharedAddr;
}

static void platinumPayloadLeave(void) {
}

__attribute__((section(".header"), used))
const PlatinumRngApi platinumRngApi = {
	.magic   = PLATINUM_RNG_API_MAGIC,
	.version = PLATINUM_RNG_API_VERSION,
	.enter   = platinumPayloadEnter,
	.leave   = platinumPayloadLeave,
};