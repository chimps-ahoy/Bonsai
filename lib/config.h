#include "types.h"

#define FORALL_DIR(DO) \
	DO(XKB_KEY_h, WEST) \
	DO(XKB_KEY_j, SOUTH) \
	DO(XKB_KEY_k, NORTH) \
	DO(XKB_KEY_l, EAST)

#define FORALL_TAGS(DO) \
	DO(XKB_KEY_1, TAG(1))\
	DO(XKB_KEY_2, TAG(2))\
	DO(XKB_KEY_3, TAG(3))\
	DO(XKB_KEY_4, TAG(4))\
	DO(XKB_KEY_5, TAG(5))\
	DO(XKB_KEY_6, TAG(6))\
	DO(XKB_KEY_7, TAG(7))\
	DO(XKB_KEY_8, TAG(8))


static constexpr unsigned int borderpx = 3;
static constexpr unsigned long highlight = 0x33aa33;
static constexpr unsigned long border = 0x333366;

static constexpr unsigned int gappx = 3;
static constexpr unsigned int barpx = 0;

static constexpr Direction defopen = EAST;
