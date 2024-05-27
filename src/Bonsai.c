#include <stdlib.h>
#include <stdio.h>
#include <xkbcommon/xkbcommon.h>
#include <wayland-server.h>
#include <swc.h>

#include <unistd.h>

#define WINTYPE
typedef struct swc_window *Window;

#include "types.h"
#include "tiles.h"
#include "config.h"
#include "util.h"

#define VIEW_INFO ((Args){.geo = {.x=gappx/2,.y=barpx+gappx/2,.w=sw-gappx,.h=sh-barpx-gappx,.filter=t->filter}})

static struct wl_display *dpy = NULL;

void draw(Region *n, Args a)
{
	/* todo! */
}

void newscreen(struct swc_screen *s)
{
	/* todo! */
}

void newwin(struct swc_window *s)
{
	/* todo! */
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
	wl_display_destroy(dpy);
}

void term(void *data, uint32_t time, uint32_t value, uint32_t state)
{
	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;
	if (!fork()) {
		char *term[] = {"weston-smoke", NULL};
		LOG("executing %s , %p\n", *term, (void*)term);
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

	if (!swc_initialize(dpy, NULL, man)) {
		LOG("failed to initialize swc\n");
		return EXIT_FAILURE;
	}

	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO, XKB_KEY_q,
			        quit, NULL);
	swc_add_binding(SWC_BINDING_KEY, SWC_MOD_LOGO, XKB_KEY_Return,
			        term, NULL);

	wl_display_run(dpy);
	wl_display_destroy(dpy);

	return EXIT_SUCCESS;
}
