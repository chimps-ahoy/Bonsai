#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "../lib/types.h"
#include "../lib/tree.h"
#include "../lib/util.h"
#include "../lib/config.h"

static Tree *t = NULL;
static Display *dpy = NULL;
static int screen;
static Window root;
static int sw;
static int sh;

Args partition(Node *n, Args a)
{
	if (!n || !n->parent) return a;
    Side from = from(n);
    n = n->parent;
    return (Args){
		        .geo = {
                 .x = a.geo.x + from * a.geo.w * n->weight * !n->orient,
                 .y = a.geo.y + from * a.geo.h * n->weight * n->orient,
                 .w = MAX(a.geo.w*n->orient, //foo
						  a.geo.w*n->weight*!n->orient*!from 
						 +a.geo.w*(1-n->weight)*!n->orient*from),
                 .h = MAX(a.geo.h*!n->orient,
						  a.geo.h*n->weight*n->orient*!from 
						 +a.geo.h*(1-n->weight)*n->orient*from)
				}
            };
}

void draw(Node *n, Args a)
{
	XUnmapWindow(dpy, n->win);
	XMoveResizeWindow(dpy, n->win, a.geo.x, a.geo.y, a.geo.w, a.geo.h);
	XMapWindow(dpy, n->win);
}

void map(XEvent *_e)
{
	t->curr = addclient(addsplit(t->curr, H, 0.5), _e->xmap.window, R, t->filter);
	if (!t->curr->parent) t->root = t->curr;
	else if (!t->curr->parent->parent) t->root = t->curr->parent;
	trickle(t->root, draw, (Args){.geo={.w=sw,.h=sh}}, partition);
}

void unmap(XEvent *_e)
{
	Node *m = NULL;
	for (int i = 0; !m && i<4; i++)
		m = findneighbor(t->curr, (Direction){i&2,i&1}, t->filter);
	Node *n = orphan(find(t->root, _e->xunmap.window));
	free(t->curr);
	if (!n || !n->parent) t->root = n;
	t->curr = m;
	trickle(t->root, draw, (Args){.geo={.w=sw,.h=sh}}, partition);
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
	//[ConfigureRequest] = configure,
	[MapRequest] = map,
	[MapNotify] = map,
	[UnmapNotify] = unmap,
	[DestroyNotify] = unmap,
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
	t->root = t->curr = NULL;
	t->filter = ~0;

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
