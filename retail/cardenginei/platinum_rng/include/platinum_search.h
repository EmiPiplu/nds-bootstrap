#ifndef PLATINUM_SEARCH_H
#define PLATINUM_SEARCH_H

typedef enum {
	RNG_METHOD_STARTER = 0,
	RNG_METHOD_WILD,
	RNG_METHOD_STATIONARY,

	RNG_METHOD_COUNT
} RngMethod;

typedef struct {
	RngMethod method;
} SearchConfig;

#endif