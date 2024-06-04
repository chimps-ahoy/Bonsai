#ifndef TYPES
#define TYPES

#include <stdint.h>

#ifndef WINTYPES
/*default typedef for window types so we can remain independent of implementation*/
typedef void *Window;
#endif

typedef enum : uint8_t { L = 0b00, R = 0b01, } Side;
typedef enum : uint8_t { V = 0b00, H = 0b10, } Orientation; 
typedef enum : uint8_t {
	NORTH = H | L,//0b10
	EAST = V | R,//0b01
	SOUTH = H | R,//0b11
	WEST = V | L,//0b00
} Direction;
#define SIDEMASK 1
#define ORIENMASK 2
#define NOSIDE 4

typedef enum : uint8_t { SPLIT = 0, CLIENT = 1} Type;

typedef union {
	struct {
		int x;
		int y;
		unsigned int w;
		unsigned int h;
		uint8_t filter;
	} geo;
} Args;
#endif
