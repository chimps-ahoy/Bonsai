#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "../lib/types.h"
#include "../lib/tree.h"
#include "../lib/config.h"

static Display *dpy = NULL;
static int screen;
static Window root;
static Tree *t = NULL;
static int sw;
static int sh;

void new(XEvent *_e)
{
	static Side s = L;
	t->curr = addclient(addsplit(t->curr, V, 0.5), _e->xmap.window, (s=(s+1)%2), t->filter);
	if (!t->curr->parent)
		t->root = t->curr;
}

void recur(Node *n, int x, int y, unsigned int w, unsigned int h)
{
	if (!n) return;
	if (n->type == split && n->orient == H) {
		fprintf(stderr, "%d\n", __LINE__);
		recur(n->children[L], x, y, w, h*n->weight);
		recur(n->children[R], x, y+(h*n->weight), w, h*(1-n->weight));
		return;
	}
	if (n->type == split && n->orient == V) {
		fprintf(stderr, "%d\n", __LINE__);
		recur(n->children[L], x, y, w*n->weight, h);
		recur(n->children[R], x+(w*n->weight), y, w*(1-n->weight), h);
		return;
	}
	fprintf(stderr, "%d\n", __LINE__);
	XUnmapWindow(dpy, n->win);
	XMoveResizeWindow(dpy, n->win, x, y, w, h);
	XMapWindow(dpy, n->win);
}

void map(XEvent *_e)
{
	new(_e);
	recur(t->root, 0, 0, sw, sh);
	printtree(t->root, stderr);
	fprintf(stderr, "\n");
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
	root = RootWindow(dpy, screen);

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
		if (handler[e.type]) {
			handler[e.type](&e);
		} else fprintf(stderr, "event ignored %d", e.type);
	}

	return EXIT_SUCCESS;
}
