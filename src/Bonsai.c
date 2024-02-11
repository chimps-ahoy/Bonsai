#include <stdlib.h>
#include <stdio.h>

#include "../lib/types.h"
#include "../lib/tree.h"
#include "../lib/config.h"

#define VIEW_INFO ((Args){.geo = {.x=0,.y=0,.w=sw,.h=sh,.filter=m->filter}})

static Monitor *m = NULL;
static Display *dpy = NULL;
static int screen;
static Window root;
static int sw;
static int sh;

void draw(Region *n, Args a)
{
	if (a.geo.w && a.geo.h) {
		fprintf(stderr, "attempting to draw: %ld\n", n->win);
		XMoveResizeWindow(dpy, n->win, a.geo.x, a.geo.y, a.geo.w, a.geo.h);
		XMapWindow(dpy, n->win);
	}
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

static void create(XEvent *e)
{
	static Orientation o = H;
	static Side s = L;
	m->curr = spawn(split(m->curr, o^=1, 0.5), e->xmap.window, s, m->filter);
	if (!m->curr->parent) m->screen = m->curr;
	else if (!m->curr->parent->parent) m->screen = m->curr->parent;
	s^=o;
}

static void drawscreen(XEvent *e)
{
	trickle(m->screen, draw, VIEW_INFO, partition);
}

static void destroy(XEvent *e)
{
	Region *r = find(m->screen,e->xunmap.window,~0);
	fprintf(stderr, "r = %p\n", r);
	Region *r2 = orphan(r);
	free(r);
	if (r2 && !r2->parent) m->screen = r2;
	trickle(m->screen, draw, VIEW_INFO, partition);//TODO: i think this is causing the b ad?
}

static void(*handler[LASTEvent])(XEvent *) = {
	/*[ConfigureRequest] = configure,*/
	[CreateNotify] = create,
	[MapRequest] = drawscreen,
	[MapNotify] = drawscreen,
	[UnmapNotify] = drawscreen,
	[DestroyNotify] = destroy,
};

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "display didn't open.");
		return EXIT_FAILURE;
	}

	screen = DefaultScreen(dpy);
#ifdef DEBUG
	sw = 1920;
	sh = 1080;
#else
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
#endif
	root = RootWindow(dpy, screen);

	m = malloc(sizeof(Monitor));
	m->screen = m->curr = NULL;
	m->filter = 1;

	XSetErrorHandler(otherwmdetected);
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask
	                                         |SubstructureNotifyMask
	                                         |StructureNotifyMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);

	for (;;) {
		XEvent e;
		XNextEvent(dpy, &e);
		if (handler[e.type]) {
			fprintf(stderr, "\nevent %d\n", e.type);
			handler[e.type](&e);
		} else fprintf(stderr, "\nevent ignored %d\n", e.type);
#ifdef DEBUG
		printtree(m->screen, stderr, VIEW_INFO);
#endif
	}

	return EXIT_SUCCESS;
}
