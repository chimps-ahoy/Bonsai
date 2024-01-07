#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "../lib/types.h"
#include "../lib/tree.h"
#include "../lib/config.h"

static Display *dpy = NULL;
static Window clients[1024] = {0};

void configure(XEvent *_e)
{
	XConfigureRequestEvent e = _e->xconfigurerequest;
	XWindowChanges changes;
	changes.x = e.x;
	changes.y = e.y;
	changes.width = e.width;
	changes.border_width = e.border_width;
	changes.sibling = e.above;
	changes.stack_mode = e.detail;
	XConfigureWindow(dpy, e.window, e.value_mask, &changes);
}

void map(XEvent *_e)
{
	XMapRequestEvent e = _e->xmaprequest;
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, e.window, &attrs);
	Window frame = XCreateSimpleWindow(
			dpy,
			DefaultRootWindow(dpy),
			attrs.x,
			attrs.y,
			attrs.width,
			attrs.height,
			border,
			border_colour,
			bg_colour);
	XSelectInput(dpy, frame,  SubstructureRedirectMask|SubstructureNotifyMask);
	XAddToSaveSet(dpy, e.window);
	XReparentWindow(dpy, e.window, frame, 0, 0);
	XMapWindow(dpy, frame);
	XMapWindow(dpy, e.window);
	clients[e.window] = frame;
}

void unmap(XEvent *_e)
{
	XUnmapEvent e = _e->xunmap;
	if (!clients[e.window]) return;
	Window frame = clients[e.window];
	XUnmapWindow(dpy, frame);
	XReparentWindow(dpy, e.window, DefaultRootWindow(dpy), 0, 0);
	XRemoveFromSaveSet(dpy, e.window);
	XDestroyWindow(dpy, frame);
	clients[e.window] = 0;
}

int otherwmdetected(Display *dpy, XErrorEvent *ee)
{
	fprintf(stdout, "another wm is run.");
	exit(1);
	return -1; /*UNREACHABLE*/
}

int xerror(Display *dpy, XErrorEvent *ee)
{
	fprintf(stdout, "some error. %d", ee->error_code);
	exit(1);
	return -1; /*UNREACHABLE*/
}

static void(*handler[LASTEvent])(XEvent *) = {
	[ConfigureRequest] = configure,
	[MapRequest] = map,
	/*[UnmapNotify] = unmap,*/
};

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stdout, "display didn't open.");
		return EXIT_FAILURE;
	}
	XSetErrorHandler(otherwmdetected);
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
	for (;;) {
		XEvent e;
		XNextEvent(dpy, &e);
		if (handler[e.type]) handler[e.type](&e);
		else fprintf(stdout, "event ignored %d", e.type);
	}
	return EXIT_SUCCESS;
}
