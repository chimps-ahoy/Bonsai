#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "../lib/types.h"
#include "../lib/tree.h"

static Display *dpy = NULL;

void create(XCreateWindowEvent ee)
{
	return;
}

void destroy(XDestroyWindowEvent ee)
{
	return;
}

void configure(XConfigureRequestEvent e)
{
	XWindowChanges changes;
	changes.x = e.x;
	changes.y = e.y;
	changes.width = e.width;
	changes.border_width = e.border_width;
	changes.sibling = e.above;
	changes.stack_mode = e.detail;
	XConfigureWindow(dpy, e.window, e.value_mask, &changes);
}

void map(XMapRequestEvent e)
{
	unsigned int w = 3;
	unsigned long c = 0xff0000;
	unsigned long bc = 0x0000ff;
	XWindowAttributes attrs;
	XGetWindowAttributes(dpy, e.window, &attrs);
	Window frame = XCreateSimpleWindow(
			dpy,
			DefaultRootWindow(dpy),
			attrs.x,
			attrs.y,
			attrs.width,
			attrs.height,
			w,
			c,
			bc);
	XSelectInput(dpy, frame,  SubstructureRedirectMask|SubstructureNotifyMask);
	XAddToSaveSet(dpy, e.window);
	XReparentWindow(dpy, e.window, frame, 0, 0);
	XMapWindow(dpy, frame);
	XMapWindow(dpy, e.window);
}

int xerrorstart(Display *dpy, XErrorEvent *ee)
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

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "display didn't open.");
		return EXIT_FAILURE;
	}
	XSetErrorHandler(xerrorstart);
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
	for (;;) {
		XEvent e;
		XNextEvent(dpy, &e);
		switch (e.type) {
			case CreateNotify:
				create(e.xcreatewindow);
				break;
			case DestroyNotify:
				destroy(e.xdestroywindow);
				break;
			case ConfigureRequest:
				configure(e.xconfigurerequest);
				break;
			case MapRequest:
				map(e.xmaprequest);
				break;
			default:
				fprintf(stdout, "event ignored. %d", e.type);
				break;
		}
	}
	return EXIT_SUCCESS;
}
