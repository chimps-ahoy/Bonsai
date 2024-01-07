#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "../lib/types.h"
#include "../lib/tree.h"
#include "../lib/config.h"

static Display *dpy = NULL;
static Tree *t = NULL;

void spawn(XEvent *_e)
{
}

void configure(XEvent *_e)
{
}

void map(XEvent *_e)
{
}

void unmap(XEvent *_e)
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
	[ConfigureRequest] = configure,
	[MapRequest] = map,
	[UnmapNotify] = unmap,
};

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "display didn't open.");
		return EXIT_FAILURE;
	}

	t = malloc(sizeof(Tree));
	if (!t) {
		fprintf(stderr, "not enough mem.");
		return EXIT_FAILURE;
	}
	t->root = NULL;
	t->curr = NULL;
	t->filter = (1<<1);


	XSetErrorHandler(otherwmdetected);
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);

	for (;;) {
		XEvent e;
		XNextEvent(dpy, &e);
		if (handler[e.type]) handler[e.type](&e);
		else fprintf(stderr, "event ignored %d", e.type);
	}

	return EXIT_SUCCESS;
}
