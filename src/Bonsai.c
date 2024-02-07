#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
//#include "../lib/types.h"
//#include "../lib/tree.h"
//#include "../lib/config.h"

static Display *dpy = NULL;
static int screen;
static Window root;
static int sw;
static int sh;

void new(XEvent *_e)
{
}

void map(XEvent *_e)
{
	XUnmapWindow(dpy, _e->xmap.window);
	XMoveResizeWindow(dpy, _e->xmap.window, 20, 20, sw/2, sh/2);
	XMapWindow(dpy, _e->xmap.window);
}

void unmap(XEvent *_e)
{
}

void configure(XEvent *_e)
{
}

int otherwmdetected(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "another wm is run.");
	exit(1);
	return -1; /*UNREACHABLE*/
}

int xerror(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "some error. %d", ee->error_code);
	exit(1);
	return -1; /*UNREACHABLE*/
}

static void(*handler[LASTEvent])(XEvent *) = {
	//[CreateNotify] = new,
	[ConfigureRequest] = configure,
	[MapRequest] = map,
	[MapNotify] = map,
	[UnmapNotify] = unmap,
};

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "display didn't open.");
		return EXIT_FAILURE;
	}
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	fprintf(stderr, "%dX%d\n", sw, sh);
	root = RootWindow(dpy, screen);

	XSetErrorHandler(otherwmdetected);
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);

	for (;;) {
		XEvent e;
		XNextEvent(dpy, &e);
		if (handler[e.type]) {
			handler[e.type](&e);
		} else fprintf(stderr, "event ignored %d", e.type);
	}

	return EXIT_SUCCESS;
}
