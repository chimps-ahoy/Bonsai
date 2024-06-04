/* INCLUDES */
#include <stdlib.h>
#include <stdio.h>

#include <spawn.h>
#include <swc.h>
#include <unistd.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>

/* TYPES */
#define WINTYPES
typedef struct swc_window * Window;

#include "types.h"
#include "tiles.h"
#include "config.h"
#include "util.h"

typedef struct {
	struct swc_screen *screen;
	Region *whole;
	Region *curr;
	uint8_t filter;
} Tiling;

/* STATE INFO */
static struct wl_display *dpy = NULL;
static Tiling **screens = NULL;
static int scount = 0;
static int ssize = 0;
static int scurr = 0;

/* CONFIG INFO */
static Direction diropen = defopen;
#define BIND(key,dir) [key] = dir,
static const Direction dirmap[] = {
	FORALL_DIR(BIND)
};
#undef BIND

inline void dbtree(void)
{
	LOG("\n");
	printtree(screens[scurr]->whole, stderr, (Args) {
			.geo = {
				.x = OFFSET,
				.y = barpx+OFFSET,
				.w = screens[scurr]->screen->usable_geometry.width-2*OFFSET,
				.h = screens[scurr]->screen->usable_geometry.height-barpx-2*OFFSET,
				.filter = screens[scurr]->filter,
			}
		});
	LOG("\n");
}

void setborder(Region *r, Args _)
{
    #ifdef DEBUG
	int border = rand();
    #else
	int border = border;
    #endif
	if (screens[scurr]->curr == r) 
		swc_window_set_border(contents(r), highlight, borderpx);
	else
		swc_window_set_border(contents(r), border, borderpx);
}

void draw(Region *r, Args a)
{
	if (!visible(r, a.geo.filter)) {
		swc_window_hide(contents(r));
	} else {
		struct swc_rectangle geo = {
			.x = a.geo.x+OFFSET,
			.y = a.geo.y+OFFSET,
			.width = a.geo.w-2*OFFSET,
			.height = a.geo.h-2*OFFSET,
		};
		LOG("tags match. drawing window @ (%d,%d)%dx%d\r",
			geo.x,
			geo.y,
			geo.width,
			geo.height);
		setborder(r, (Args){0});
		swc_window_show(contents(r));
		swc_window_set_geometry(contents(r), &geo);
	}
}

inline void drawscreen(Tiling *t)
{
	Args geo = (Args){
		.geo = {
			.x = OFFSET,
			.y = barpx+OFFSET,
			.w = t->screen->usable_geometry.width-2*OFFSET,
			.h = t->screen->usable_geometry.height-barpx-2*OFFSET,
			.filter = t->filter,
		}
	};
	trickle(t->whole, draw, geo, partition);
}

void delscreen(void *data)
{
	int i = 0;
	do {
		if (screens[i]->screen == (struct swc_screen *)data) {
			trickle(screens[i]->whole, freeregion, (Args){0}, id);
			screens[i] = NULL;
			break;
		}
	} while (++i < scount);
	for (; i < scount; i++)
		screens[i-1] = screens[i];
}

void redrawscreen(void *data)
{
	for (int i = 0; i < scount; i++) {
		if (screens[i]->screen == (struct swc_screen *)data) {
			drawscreen(screens[i]);
			return;
		}
	}
}

struct swc_screen_handler *screenhandle = &(struct swc_screen_handler){
	.destroy = delscreen,
	.usable_geometry_changed = redrawscreen,
};

void newscreen(struct swc_screen *s)
{
	Tiling *new = malloc(sizeof(Tiling));
	if (!new) {
		LOG("failed screen malloc\n");
		exit(EXIT_FAILURE);
	}
	new->whole = NULL;
	new->curr = NULL;
	new->filter = ~0;

	if (scount == ssize) {
		if (!(screens = realloc(screens, ++ssize))) {
			LOG("failed screen realloc\n");
			exit(EXIT_FAILURE);
		}
	}

	screens[scount++] = new;
	new->screen = s;
	swc_screen_set_handler(s, screenhandle, new);
}

void quit(void *data, uint32_t time, uint32_t value, uint32_t state)
{
	if (!data) {
		LOG("quitting from keyboard input.\n"
				"time = %d,\n"
				"value = %d,\n"
				"state = %d,\n",
				time,
				value,
				state);
	} else {
		LOG("exiting autonomously.\n"
				"time = %d,\n"
				"value = %d,\n"
				"state = %d,\n",
				time,
				value,
				state);
	}
	wl_display_terminate(dpy);
	exit((long)data);
}

static inline void delwin(Region *todestroy, int screen)
{
	if (todestroy) {
		Region *r = orphan(todestroy);
		LOG("found orphan\n");
		if (!r) {
			quit((void *)EXIT_FAILURE, 0, 0, 0);
		}
		if (isorphan(r)) {
			LOG("root replaced\n");
			screens[screen]->whole = r;
		}
		else for (int i = 0; i < 3 && (!contents(r) || r == todestroy); i++) {
			LOG("searching for neighbor, %b\n", i);
			r = findneighbor(todestroy, (Direction){(i&2)>>1,(i&1)}, screens[screen]->filter);
		}
		if (!contents(r)) {
			LOG("only neighbor was split. last resort search\n");
			r = find(screens[screen]->whole, NULL, screens[screen]->filter);
		}
		screens[screen]->curr = r;
		swc_window_hide(contents(todestroy));
		LOG("window hidden\n");
		free(todestroy);
		LOG("region destroyed\n");
		drawscreen(screens[screen]);
	}
}

void delcurr(void *_, uint32_t time, uint32_t value, uint32_t state)
{
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	delwin(screens[scurr]->curr, scurr);
}

void windel(void *win)
{
	for (int i = 0; i < scount; i++) {
		Region *todestroy = find(screens[i]->whole, (Window)win, ~0);
		if (todestroy) {
			delwin(todestroy, i);
			return;
		} 
	}
	/*if the window is not found in any region, it is transient or floating
	 * or whatever. some weird edge case. this code almost never runs*/
	swc_window_hide(win);
}

void focus(void *win)
{
	if (win) {
		Region *focused = find(screens[scurr]->whole, win, screens[scurr]->filter);
		if (focused) {
			screens[scurr]->curr = focused;
			swc_window_focus(win);
		}
		trickle(screens[scurr]->whole, setborder, (Args){0}, id);
	}
}

void movefocus(void *_, uint32_t time, uint32_t value, uint32_t state)
{
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	Region *next;
	if ((next = findneighbor(screens[scurr]->curr,
					         dirmap[value], screens[scurr]->filter))) {
		screens[scurr]->curr = next;
	}
	focus(contents(screens[scurr]->curr));
}

struct swc_window_handler *winhandler = &(struct swc_window_handler){
	.destroy = windel,
	.entered = focus
};

void newwin(struct swc_window *s)
{
	screens[scurr]->curr = split(screens[scurr]->curr, diropen, 0.5, s,
			                     screens[scurr]->filter);
	zoomout(&(screens[scurr]->whole), screens[scurr]->curr);

	swc_window_set_handler(s, winhandler, s);
	swc_window_set_tiled(s);
	swc_window_focus(s);
	dbtree();
	drawscreen(screens[scurr]);
}

void spawn(void *data, uint32_t time, uint32_t value, uint32_t state)
{
	diropen = dirmap[value];
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	LOG("spawning %s\n", *(char **)data);
	extern char **environ;
	int _;
	posix_spawnp(&_, *(char **)data, NULL, NULL, data, environ);
}

static struct swc_manager *man = &(struct swc_manager){
	.new_screen = newscreen,
	.new_window = newwin,
};

int main(void)
{
	if (!(dpy = wl_display_create())) {
		LOG("failed to open display\n");
		return EXIT_FAILURE;
	}
	LOG("dpy = %p\n", (void*)dpy);

	const char *sock = wl_display_add_socket_auto(dpy);
	if (!sock) {
		LOG("failed to add socket. dpy = %p\n", (void*)dpy);
		return EXIT_FAILURE;
	}
	LOG("sock = %s\n", sock);
	setenv("WAYLAND_DISPLAY", sock, 1);

	if (!swc_initialize(dpy, wl_display_get_event_loop(dpy), man)) {
		LOG("failed to initialize swc\n");
		return EXIT_FAILURE;
	}

	static const char *term[] = {"st", NULL};

    #define BINDTERM(key,_) { \
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO | SWC_MOD_SHIFT, key, \
			        spawn, term); \
    }
    #define MOVEFOCUS(key,_) { \
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO, key, \
			        movefocus, NULL); \
	}
	FORALL_DIR(BINDTERM);
	FORALL_DIR(MOVEFOCUS);
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO, XKB_KEY_q,
			        delcurr, NULL);
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO | SWC_MOD_SHIFT, XKB_KEY_q,
			        quit, NULL);

	wl_display_run(dpy);

	return EXIT_SUCCESS;
}
