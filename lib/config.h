#include "types.h"

#define FORALL_DIR(DO) \
	DO(XKB_KEY_h, WEST) \
	DO(XKB_KEY_j, SOUTH) \
	DO(XKB_KEY_k, NORTH) \
	DO(XKB_KEY_l, EAST)

static constexpr unsigned int borderpx = 3;
static constexpr unsigned long highlight = 0x33aa33;
static constexpr unsigned long border = 0x333366;

static constexpr unsigned int gappx = 3;
static constexpr unsigned int barpx = 0;

static constexpr Direction defopen = EAST;
