#include <nds/ndstypes.h>

#include "platinum_rng_api.h"
#include "platinum_hud.h"

static void platinumPayloadEnter(
	const PlatinumRngInfo *info
);

static void platinumPayloadLeave(void);

__attribute__((section(".header"), used))
const PlatinumRngApi platinumRngApi = {
	.magic   = PLATINUM_RNG_API_MAGIC,
	.version = PLATINUM_RNG_API_VERSION,
	.enter   = platinumPayloadEnter,
	.leave   = platinumPayloadLeave,
};

static void platinumPayloadEnter(const PlatinumRngInfo *info)
{
    (void)info;
}

static void platinumPayloadLeave(void)
{
}