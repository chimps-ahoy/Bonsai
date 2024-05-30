#include <stdlib.h>
#include <stdio.h>

#include <swc.h>
#include <unistd.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>

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

static Direction diropen = defopen;
#define BIND(key,dir) [key] = dir,
static const Direction dirmap[] = {
	FORALL_DIR(BIND)
};
#undef BIND

void setborder(Region *r, Args _)
{
	if (screens[scurr]->curr == r) 
		swc_window_set_border(r->win, highlight, borderpx);
	else
		swc_window_set_border(r->win, border, borderpx);
}

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
		setborder(n, (Args){0});
		swc_window_show(n->win);
		swc_window_set_geometry(n->win, &geo);
	}
}

void drawscreen(Tiling *t)
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
	trickle(t->whole, draw, geo, partition);
}

void delscreen(void *data)
{
	int i = 0;
	do {
		if (screens[i]->screen == (Screen)data) {
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
		if (screens[i]->screen == (Screen)data) {
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

void delwin(void *win)
{
	for (int i = 0; i < scount; i++) {
		Region *todestroy = find(screens[i]->whole, (Window)win, ~0);
		if (todestroy) {
			Region *r = orphan(todestroy);
			if (!r)
				quit((void *)EXIT_FAILURE, 0, 0, 0);
			if (!r->parent)
				screens[i]->whole = r;
			screens[i]->curr = r;
			free(todestroy);
			drawscreen(screens[i]);
			return;
		}
	}
}

void delcurr(void *win, uint32_t time, uint32_t value, uint32_t state)
{
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	delwin(win);
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
	focus(screens[scurr]->curr->win);
}


struct swc_window_handler *winhandler = &(struct swc_window_handler){
	.destroy = delwin,
	.entered = focus
};

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

	screens[scurr]->curr = spawn(split(screens[scurr]->curr, diropen.o, 0.5),
			                     s, diropen.s ,screens[scurr]->filter);

	if (!screens[scurr]->curr->parent)
		screens[scurr]->whole = screens[scurr]->curr;
	else if (!screens[scurr]->curr->parent->parent)
		screens[scurr]->whole = screens[scurr]->curr->parent;

	swc_window_set_handler(s, winhandler, s);
	swc_window_set_tiled(s);
	swc_window_focus(s);
	printtree(screens[scurr]->whole, stderr, (Args) {
			.geo = {
				.x = gappx/2,
				.y = barpx+gappx/2,
				.w = screens[scurr]->screen->usable_geometry.width-gappx,
				.h = screens[scurr]->screen->usable_geometry.height-gappx-barpx,
				.filter = screens[scurr]->filter,
			}
		});
	LOG("\n");
	drawscreen(screens[scurr]);
}

void openwin(void *data, uint32_t time, uint32_t value, uint32_t state)
{
	diropen = dirmap[value];
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	LOG("spawning %s\n", *(char **)data);
	if (fork() == 0) {
		execvp(*(char **)data, data);
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
	setenv("WAYLAND_DISPLAY", sock, 1);

	if (!swc_initialize(dpy, wl_display_get_event_loop(dpy), man)) {
		LOG("failed to initialize swc\n");
		return EXIT_FAILURE;
	}

	static const char *term[] = {"weston-terminal", NULL};
	static const char *disc[] = {"discord", NULL};

    #define BINDTERM(key,_) { \
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO | SWC_MOD_SHIFT, key, \
			        openwin, term); \
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
