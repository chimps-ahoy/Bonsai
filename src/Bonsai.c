#include <stdlib.h>
#include <stdio.h>

#include <xkbcommon/xkbcommon.h>
#include <wayland-server.h>
#include <swc.h>
#include <unistd.h>

#define WINTYPES
typedef struct swc_window * Window;
typedef struct swc_screen * Screen;

#include "types.h"
#include "tiles.h"
#include "config.h"
#include "util.h"

static struct wl_display *dpy = NULL;
static Tiling **screens = NULL;
static int scount = 0;
static int ssize = 0;
static int scurr = 0;

void draw(Region *n, Args a)
{
	LOG("tags: %B, filter: %B\n", n->tags, a.geo.filter);
	if (!(a.geo.filter & n->tags)) {
		LOG("tags don't match. hiding window.\n");
		swc_window_hide(n->win);
	} else {
		struct swc_rectangle geo = {
			.x = a.geo.x+gappx/2,
			.y = a.geo.y+gappx/2,
			.width = a.geo.w-gappx,
			.height = a.geo.h-gappx,
		};
		LOG("tags match. drawing window @ (%d,%d)%dx%d\n",
			geo.x+gappx/2,
			geo.y+gappx/2,
			geo.width-gappx,
			geo.height-gappx);
		swc_window_show(n->win);
		swc_window_set_geometry(n->win, &geo);
	}
}

void arrange(Tiling *t)
{
	Args geo = (Args){
		.geo = {
			.x = gappx/2,
			.y = barpx+gappx/2,
			.w = t->screen->usable_geometry.width-gappx,
			.h = t->screen->usable_geometry.height-gappx-barpx,
			.filter = t->filter,
		}
	};
	trickle(screens[scurr]->whole, draw, geo, partition);
}

struct swc_screen_handler *screenhandle = &(struct swc_screen_handler){ 0 };

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

struct swc_window_handler *winhandler = &(struct swc_window_handler){0};

void newwin(struct swc_window *s)
{
	Region *new = malloc(sizeof(Region));
	if (!new) {
		LOG("failed malloc new\n");
		exit(EXIT_FAILURE);
	}
	new->type = CLIENT;
	new->win = s;
	new->parent = NULL;
	new->tags = screens[scurr]->filter;

	screens[scurr]->curr = spawn(split(screens[scurr]->curr, spawnloc.o, 0.5),
			                     s, spawnloc.s ,screens[scurr]->filter);

	if (!screens[scurr]->curr->parent)
		screens[scurr]->whole = screens[scurr]->curr;
	else if (!screens[scurr]->curr->parent->parent)
		screens[scurr]->whole = screens[scurr]->curr->parent->parent;

	swc_window_set_handler(s, winhandler, new);
	swc_window_set_tiled(s);
	arrange(screens[scurr]);
}

void quit(void *data, uint32_t time, uint32_t value, uint32_t state)
{
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	LOG("quitting from keyboard input.\n"
		"time = %d,\n"
		"value = %d,\n"
		"state = %d,\n",
		time,
		value,
		state);
	wl_display_terminate(dpy);
}

void term(void *data, uint32_t time, uint32_t value, uint32_t state)
{
	char *term[] = {"st", NULL};
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	LOG("spawning %s\n", *term);
	if (fork() == 0) {
		execvp(*term, term);
		exit(EXIT_FAILURE);
	}
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
	/*setenv("WAYLAND_DISPLAY", sock, 1);*/

	if (!swc_initialize(dpy, wl_display_get_event_loop(dpy), man)) {
		LOG("failed to initialize swc\n");
		return EXIT_FAILURE;
	}

	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_CTRL, XKB_KEY_c,
			        quit, NULL);
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO, XKB_KEY_Return,
			        term, NULL);

	wl_display_run(dpy);

	return EXIT_SUCCESS;
}
