#ifndef PLATINUM_RNG_API_H
#define PLATINUM_RNG_API_H

#include <nds/ndstypes.h>
#include "locations.h"

#define PLATINUM_RNG_API_MAGIC   0x474E5250
#define PLATINUM_RNG_API_VERSION 3

typedef struct {
	u32 magic;
	u32 version;

	void (*enter)(void);
	void (*leave)(void);
} PlatinumRngApi;

#define PLATINUM_RNG_API \
	((const PlatinumRngApi *)PLATINUM_RNG_LOCATION)

#endif