#ifndef TYPES
#define TYPES

#include <stdint.h>

#ifndef WINTYPES
typedef void *Window;
typedef void *Screen;
#endif

typedef enum : uint8_t { V = 0b00, H = 0b01, } Orientation; 
typedef enum : uint8_t { L = 0b00, R = 0b01, } Side;
#define FAKESIDE 69 //if we switch to bit arrays for directions, this could
					//be like 2 or something so we can just &2 to check

typedef enum : uint8_t { SPLIT = 0, CLIENT = 1} Type;

typedef struct { Orientation o; Side s; } Direction;

typedef union {
	struct {
		int x;
		int y;
		unsigned int w;
		unsigned int h;
		uint8_t filter;
	} geo;
} Args;

static const Direction NORTH = { H, L };
static const Direction SOUTH = { H, R };
static const Direction EAST = { V, R }; 
static const Direction WEST = { V, L };

/*TODO: bit arrays?*/
/*typedef enum : uint8_t {*/
/*	NORTH = H | L,//0b10 */
/*	EAST = V | R,//0b01 */
/*	SOUTH = H | R,//0b11 */
/*	WEST = V | L,//0b00 */
/*} Direction;*/

#endif
