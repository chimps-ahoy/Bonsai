#include "types.h"

#define FORALL_DIR(DO) \
	DO(XKB_KEY_h, WEST) \
	DO(XKB_KEY_j, SOUTH) \
	DO(XKB_KEY_k, NORTH) \
	DO(XKB_KEY_l, EAST)

static const unsigned int borderpx = 3;
static const unsigned long highlight = 0xff0000;
static const unsigned long border = 0x0000ff;

static const unsigned int gappx = 6;
static const unsigned int barpx = 12;

static const Direction defopen = EAST;
