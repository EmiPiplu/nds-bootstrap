#ifndef PLATINUM_RNG_API_H
#define PLATINUM_RNG_API_H

#include <nds/ndstypes.h>
#include "locations.h"

#define PLATINUM_RNG_API_MAGIC   0x474E5250
#define PLATINUM_RNG_API_VERSION 2

#define PLATINUM_RNG_INFO_HAVE_SEED BIT(0)

typedef struct {
	u32 currentRng;
	u32 initialSeed;
	u32 advances;
	u32 flags;
} PlatinumRngInfo;

typedef struct {
	u32 magic;
	u32 version;

	void (*enter)(const PlatinumRngInfo *info);
	void (*leave)(void);
} PlatinumRngApi;

#define PLATINUM_RNG_API \
	((const PlatinumRngApi *)PLATINUM_RNG_LOCATION)

#endif