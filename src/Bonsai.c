#include <stdlib.h>
#include <stdio.h>

#include <types.h>
#include <tiles.h>
#include <config.h>

#define VIEW_INFO ((Args){.geo = {.x=0,.y=0,.w=sw,.h=sh,.filter=t->filter}})

static Tiling *t = NULL;
static Display *dpy = NULL;
static int whole;
static Window root;
static int sw;
static int sh;

void draw(Region *n, Args a)
{
	if (a.geo.w && a.geo.h) {
		fprintf(stderr, "\nattempting to draw: %ld\n", n->win);
		XMoveResizeWindow(dpy, n->win, a.geo.x, a.geo.y, a.geo.w, a.geo.h);
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
	[MapNotify] = drawscreen,
	/*[UnmapNotify] = destroy,*/
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
	XSynchronize(dpy, True);
	sw = 1920;
	sh = 1080;
#else
	sw = DisplayWidth(dpy, whole);
	sh = DisplayHeight(dpy, whole);
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
			fprintf(stderr, "\nevent %d\n", e.type);
			printtree(t->whole, stderr, VIEW_INFO);
			handler[e.type](&e);
		} else fprintf(stderr, "\nevent ignored %d\n", e.type);
#ifdef DEBUG
#endif
	}
	return EXIT_SUCCESS;
}
