#ifndef PLATINUM_RNG_API_H
#define PLATINUM_RNG_API_H

#include <nds/ndstypes.h>
#include "locations.h"

#define PLATINUM_RNG_API_MAGIC   0x474E5250
#define PLATINUM_RNG_API_VERSION 1

/*
 * 0x474E5250 appears in RAM as:
 *
 * 50 52 4E 47
 * P  R  N  G
 */
typedef struct {
	u32 magic;
	u32 version;

	void (*enter)(vu32 *sharedAddr);
	void (*leave)(void);
} PlatinumRngApi;

#define PLATINUM_RNG_API \
	((const PlatinumRngApi *)PLATINUM_RNG_LOCATION)

#endif