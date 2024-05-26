#ifndef TYPES
#define TYPES

#include <stdint.h>

#ifndef WINTYPE
typedef long Window;
#endif

typedef enum : uint8_t { V = 0, H = 1, } Orientation; /* TODO: bit arrays? */
typedef enum : uint8_t { L = 0, R = 1, } Side;
#define FAKESIDE 69 //if we switch to bit arrays for directions, this could
					//be like 2 or something so we can just &2 to check

typedef enum : uint8_t { SPLIT = 0, CLIENT = 1} Type;

typedef struct { Orientation o; Side s; } Direction;

typedef union {
	struct {
		int x;
		int y;
		int w;
		int h;
		uint8_t filter;
	} geo;
} Args;

static const Direction NORTH = { H, L };
static const Direction SOUTH = { H, R };
static const Direction EAST = { V, R };
static const Direction WEST = { V, L };
#endif
