#ifndef TYPES
#define TYPES
typedef enum { vert = 0, hori = 1, } Orientation;
typedef enum { left = 0, right = 1, } Side;
#define FAKESIDE 69
typedef enum { split = 0, client = 1} Type;
typedef struct { Orientation o; Side s; } Direction;
static const Direction NORTH = (const Direction){ hori, left };
static const Direction SOUTH = (const Direction){ hori, right };
static const Direction EAST = (const Direction){ vert, right };
static const Direction WEST = (const Direction){ vert, left };
#define HASH(x) ((x) - 'h') 
							/*I guess these are immediately obsolete because the
							way keypresses work will be handled differently in
							xorg, and we want hte ability to let users define
							their own keybindings which would mean editing this
							array and hash function which i think is too much 
							to ask of them.
							Obviously gonna handle keybindings similarly to DWM,
							but im just saying idk why i even put this in types.h*/
static const Direction keymap[5] = {
	[HASH('h')] = WEST,
	[HASH('j')] = SOUTH,
	[HASH('k')] = NORTH,
	[HASH('l')] = EAST
};
#endif
