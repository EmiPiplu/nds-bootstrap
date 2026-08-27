#ifndef PLATINUM_RNG_API_H
#define PLATINUM_RNG_API_H

#include <nds/ndstypes.h>
#include "locations.h"

#define PLATINUM_RNG_API_MAGIC   0x474E5250
#define PLATINUM_RNG_API_VERSION 4

typedef enum {
	PLATINUM_PAYLOAD_NONE = 0,
	PLATINUM_PAYLOAD_STEP,
	PLATINUM_PAYLOAD_RESUME,
} PlatinumPayloadAction;

typedef struct {
	u32 magic;
	u32 version;

	void (*enter)(void);
	void (*leave)(void);

	PlatinumPayloadAction (*update)(u16 keys);
} PlatinumRngApi;

#define PLATINUM_RNG_API \
	((const PlatinumRngApi *)PLATINUM_RNG_LOCATION)

#endif