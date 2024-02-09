#include <stdint.h>
#include <ncurses.h>
//#include <X11/Xlib.h>
#ifndef TYPES
#define TYPES
#ifdef CURSES_H
typedef WINDOW *Window;
#endif
#ifdef _X11_XLIB_H_
typedef Window Window;
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
#define HASH(x) ((x) - 'h') 
							/*I guess these are immediately obsolete because the
							way keypresses work will be handled differently in
							xorg, and we want hte ability to let users define
							their own keybindings which would mean editing this
							array and hash function which i think is too much 
							to ask of them.
							Obviously gonna handle keybindings similarly to DWM,
							but im just saying idk why i even put this in types.h*/
static const Direction keymap[] = {
	[HASH('h')] = WEST,
	[HASH('j')] = SOUTH,
	[HASH('k')] = NORTH,
	[HASH('l')] = EAST,
};
#endif
