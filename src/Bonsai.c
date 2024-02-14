#include <stdlib.h>
#include <stdio.h>

#include <types.h>
#include <tiles.h>
#include <config.h>

#define VIEW_INFO ((Args){.geo = {.x=gappx,.y=barpx+gappx,.w=sw-2*gappx,.h=sh-barpx-2*gappx,.filter=t->filter}})
#ifdef DEBUG
#define LOG(x,...) fprintf(stderr, x, __VA_ARGS__)
#else
#define LOG(x,...) do {} while(0)
#endif

static Tiling *t = NULL;
static Display *dpy = NULL;
static int whole;
static Window root;
static int sw;
static int sh;

void draw(Region *n, Args a)
{
	if (a.geo.w && a.geo.h) {
		LOG("\nattempting to draw: %ld at %d , %d with %d x %d\n", n->win,
		    a.geo.x+gappx, a.geo.y+gappx, a.geo.w-2*gappx, a.geo.h-2*gappx);
		XMoveResizeWindow(dpy, n->win, a.geo.x+gappx, a.geo.y+gappx, a.geo.w-2*gappx, a.geo.h-2*gappx);
		//XSetWindowBorderWidth(dpy, n->win, borderpx);
		XMapWindow(dpy, n->win);
	}
}

int wmdetect(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "another wm is run.");
	exit(1);
	return -1; /*UNREACHABLE*/
}

int xerror(Display *dpy, XErrorEvent *ee)
{
	fprintf(stderr, "\nError %d:\n\trequest: %d\n\tresouce: %ld", ee->error_code, 
			                                                      ee->request_code, 
																  ee->resourceid);
	exit(1);
	return -1; /*UNREACHABLE*/
}

static void create(XEvent *e)
{
	static Orientation o = H;
	static Side s = L;
	t->curr = spawn(split(t->curr, o^=1, 0.5), e->xcreatewindow.window, s, t->filter);
	if (!t->curr->parent) t->whole = t->curr;
	else if (!t->curr->parent->parent) t->whole = t->curr->parent;
	s^=o;
}

static void drawscreen(XEvent *e)
{
	trickle(t->whole, draw, VIEW_INFO, partition);
}

static void destroy(XEvent *e)
{
	Region *r = find(t->whole,e->xdestroywindow.window,~0);
	Region *r2 = orphan(r);
	free(r);
	if (!r2 || !r2->parent) t->whole = r2;
	t->curr = find(t->whole, 0, t->filter);
	trickle(t->whole, draw, VIEW_INFO, partition);
}

static void(*handler[LASTEvent])(XEvent *) = {
	/*[ConfigureRequest] = configure,*/
	[CreateNotify] = create,
	[MapRequest] = drawscreen,
	/*[MapNotify] = drawscreen,
	[UnmapNotify] = drawscreen,*/
	[DestroyNotify] = destroy,
};

int main(void)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "display didn't open.");
		return EXIT_FAILURE;
	}

	whole = DefaultScreen(dpy);
#ifdef DEBUG
	//XSynchronize(dpy, True);
	sw = 1920;
	sh = 1080;
	sw = DisplayWidth(dpy, whole);
	sh = DisplayHeight(dpy, whole);
#else
#endif
	root = RootWindow(dpy, whole);

	t = malloc(sizeof(Tiling));
	t->whole = t->curr = NULL;
	t->filter = 1;

	XSetErrorHandler(wmdetect);
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
			LOG("\nevent %d\n", e.type);
			printtree(t->whole, stderr, VIEW_INFO);
			handler[e.type](&e);
		} else LOG("\nevent ignored %d\n", e.type);
	}
	return EXIT_SUCCESS;
}
