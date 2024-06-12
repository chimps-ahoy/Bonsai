#include <stdlib.h>
#include <swc.h>

#include "types.h"

static constexpr unsigned int cfg_mod = SWC_MOD_LOGO;

static constexpr unsigned int cfg_fcsmod = cfg_mod;
static constexpr unsigned int cfg_mnmod  = cfg_mod | SWC_MOD_ALT;
static constexpr unsigned int cfg_szmod  = cfg_mod | SWC_MOD_CTRL;
static constexpr unsigned int cfg_movmod = cfg_mod | SWC_MOD_SHIFT;

static constexpr unsigned int cfg_filtogmod = cfg_mod;
static constexpr unsigned int cfg_filmovmod = cfg_mod | SWC_MOD_SHIFT;
static constexpr unsigned int cfg_tagtogmod = cfg_mod | SWC_MOD_SHIFT | SWC_MOD_CTRL;
static constexpr unsigned int cfg_tagmovmod = cfg_mod | SWC_MOD_SHIFT | SWC_MOD_CTRL;

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

static constexpr unsigned int  cfg_borderpx  = 3;
static constexpr unsigned long cfg_highlight = 0x33aa33;
static constexpr unsigned long cfg_border    = 0x333366;
static constexpr unsigned int  cfg_gappx     = 3;
static constexpr unsigned int  cfg_barpx     = 0;
#define OFFSET (cfg_borderpx+cfg_gappx)

static constexpr Direction cfg_defopen = EAST;

static const char *const cfg_menu[] = {"st", NULL};
