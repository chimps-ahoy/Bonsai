#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "tiles.h"
#include "stack.h"
#include "util.h"

typedef struct node {
	Type type;
	struct node *container;
	uint8_t tags;
	union {
		Window win;/*client*/
		struct /*split*/ {
			struct node *subregion[2];
			float fact;
			Orientation o;
		};
	};
} Region;

void printtree(Region *r, FILE *f, Args a)
{
#ifdef DEBUG
	if (!r) {
		fprintf(f, "NULL");
		return;
	}
	if (r->type == SPLIT) {
		fprintf(f, "(");
		printtree(r->subregion[L], f, partition(r->subregion[L], a));
		fprintf(f, (r->o == H) ? " - " : " | ");
		printtree(r->subregion[R], f, partition(r->subregion[L], a));
		fprintf(f, ")");
		return;
	}
	fprintf(f, "%ld={(%d,%d) %dx%d}", (long)(r->win), a.geo.x, a.geo.y,
			                                            a.geo.w, a.geo.h);
#endif
}

inline Window contents(Region *r)
{
	if (r->type == CLIENT) return r->win;
	return NULL;
}

inline bool isorphan(Region *r)
{
	return !(bool)(r->container);
}

inline void zoomout(Region **whole, Region *r)
{
	if (!r->container)
		*whole = r;
	else if (!r->container->container)
		*whole = r->container;
}

inline bool visible(Region *r, uint8_t filter)
{
	return r->tags & filter;
}

inline void freeregion(Region *r, Args _)
{
	free(r);
}

inline void propegatetags(Region *r)
{
	while ((r = r->container)) 
		r->tags = r->subregion[L]->tags | r->subregion[R]->tags;
}


static Region *reparent(Region *tosplit, Orientation o, float fact)
{
	if (!tosplit) return NULL;
	Region *new = malloc(sizeof(Region));
	if (!new) return NULL;
	new->type = SPLIT;
	new->o = o;
	new->fact = fact;
	new->container = tosplit->container;
	if (new->container)
		new->container->subregion[IN(tosplit)] = new;
	tosplit->container = new;
	new->subregion[L] = tosplit;
	new->subregion[R] = NULL;
	new->tags = tosplit->tags;
	return new;
}

static Region *insert(Region *currsplit, Window w, Side child, uint8_t tags)
{
	if (!currsplit) {
		Region *new = malloc(sizeof(Region));
		new->container = NULL;
		new->type = CLIENT;
		new->tags = tags;
		new->win = w;
		return new;
	}
	if (currsplit->subregion[child]) 
		currsplit->subregion[NT(child)] = currsplit->subregion[child];
	currsplit->subregion[child] = malloc(sizeof(Region));
	if (!(currsplit->subregion[child])) return NULL;
	currsplit->subregion[child]->container = currsplit;
	currsplit->subregion[child]->type = CLIENT;
	currsplit->subregion[child]->tags = tags;
	currsplit->subregion[child]->win = w;
	currsplit->tags =
		currsplit->subregion[L]->tags | currsplit->subregion[R]->tags;
	propegatetags(currsplit);
	return currsplit->subregion[child];	
}

Region *split(Region *tosplit, Direction to, float fact, Window w, uint8_t tags)
{
	return insert(reparent(tosplit, to&ORIENMASK, fact), w, to&SIDEMASK, tags);
}

static Region *findnext(Region *r, const Direction d, uint8_t filter, Stack *breadcrumbs)
{	
	if (r->type == SPLIT && !((r->o ^ d)&ORIENMASK) && r->subregion[NT(d&SIDEMASK)]->tags & filter)
		return findnext(r->subregion[NT(d&SIDEMASK)], d, filter, breadcrumbs);
	if (r->type == SPLIT && (r->o ^ d)&ORIENMASK)
		return findnext(r->subregion[d&SIDEMASK], d, filter, breadcrumbs);
	Side crumb = pop(breadcrumbs);
	if (r->type == SPLIT && r->subregion[crumb]->tags & filter)
		return findnext(r->subregion[crumb], d, filter, breadcrumbs);
	if (r->type == SPLIT)
		return findnext(r->subregion[NT(crumb)], d, filter, breadcrumbs);
	return r;
}

static Region *findparent(Region *r, uint8_t filter, const Direction d, Stack *breadcrumbs)
{
	Region *container = r->container;
	while (container) {
		bool nobacktrack = (d&NOSIDE) || container->subregion[d&SIDEMASK] != r;
		bool visible = (d&NOSIDE) || container->subregion[d&SIDEMASK]->tags & filter;
		if (!((container->o ^ d)&ORIENMASK) && nobacktrack && visible)
			return container;
		else if ((container->o ^ d)&ORIENMASK)
			push(breadcrumbs, IN(r));
		r = container;
		container = container->container;
	}
	return container;
}

Region *findneighbor(Region *curr, const Direction d, uint8_t filter)
{
	if (!curr) return NULL;
	Stack *breadcrumbs = initstack();
	Region *container = findparent(curr, filter, d, breadcrumbs);
	if (container)
		curr = findnext(container->subregion[d&SIDEMASK], d, filter, breadcrumbs);
	freestack(breadcrumbs);
	return curr;
}

void reflect(Region *r)
{
	if (r->type == SPLIT) {
		r->o ^= ORIENMASK; 
		reflect(r->subregion[L]);
		reflect(r->subregion[R]);
	}
}

Region *find(Region *r, Window w, uint8_t filter)
{
	if (!r) return NULL;
	if (r->type == SPLIT && r->tags & filter) {
		Region *tmp;
		if     ((tmp = find(r->subregion[L], w, filter))) return tmp;
		return ((tmp = find(r->subregion[R], w, filter)));
	}
	if (r->type == CLIENT && (!w || r->win == w) && r->tags & filter) return r;//hack to find first win
	return NULL;
}

Region *orphan(Region *r)
{
	if (!r) return NULL;
	Region *container = r->container;
	if (!container) return NULL; /*probably no*/

	r = container->subregion[NT(IN(r))];

	if (container->container) {
		container->container->subregion[IN(container)] = r;
		r->container = container->container;
		propegatetags(r);
	} else {
		r->container = NULL;
	}
	free(container);
	return r;
}

void shiftwidth(Region *r, const Direction d, uint8_t filter)
{
	if (!r) return;
	Region *container = findparent(r, filter, d|NOSIDE, NULL);
	if (!container && r->container)
		container = findparent(r->container->subregion[NT(IN(r))], filter, d|NOSIDE, NULL);
	if (container && (d&SIDEMASK) == R)
		container->fact = MAX(0.1,MIN(container->fact+0.1, 0.9));
	else if (container)
		container->fact = MIN(0.9,MAX(container->fact-0.1, 0.1));
}

Region *moveclient(Region *r, const Direction d, uint8_t filter)
{ 
	if (!r) return NULL;
	if (!r->container) return r;
	if ((r->container->o ^ d)&ORIENMASK) {
		Side s = IN(r); 
		if (s != (d&SIDEMASK)) {
			r->container->subregion[s] = r->container->subregion[NT(s)];
			r->container->subregion[NT(s)] = r;
		}
		r->container->o = d&ORIENMASK;
		return r->container;
	}

	Region *neighbor = findneighbor(r, d, filter);
	if (neighbor != r && neighbor->container == r->container) {
		Side s = IN(neighbor);
		neighbor->container->subregion[s] = r;
		neighbor->container->subregion[NT(s)] = neighbor;
		return neighbor->container;
	}

	Side s = d&SIDEMASK;
	if (neighbor != r) s = NT(d&SIDEMASK);
	else for(; neighbor->container; neighbor = neighbor->container); 

	neighbor = reparent(neighbor, d&ORIENMASK, 0.5);
	if (neighbor->subregion[s])
		neighbor->subregion[NT(s)] = neighbor->subregion[s];
	neighbor->subregion[s] = r;
	Region *tmp = orphan(r);
	r->container = neighbor;
	return (tmp->container) ? neighbor : tmp;
}

void trickle(Region *r, void(*F)(Region *, Args), Args a, Args(*T)(Region *, Args))
{
	if (!r) return;
	if (r->type == SPLIT) {
		trickle(r->subregion[L],F,T(r->subregion[L],a),T);
		trickle(r->subregion[R],F,T(r->subregion[R],a),T);
		return;
	}
	F(r,a);
}

inline Args partition(Region *r, Args a)
{
	Side fr = IN(r);
	bool vis = r->tags & a.geo.filter;
	r = r->container;
	bool fill = !(r->subregion[NT(fr)]->tags & a.geo.filter);
	Orientation o = (r->o)>>1;
	float fa = r->fact;
	return (Args){
		.geo = {
			.x = (a.geo.x + (fr & NT(fill) & NT(o)) * a.geo.w * (1 - fa)),
			.y = (a.geo.y + (fr & NT(fill) & o)     * a.geo.h * (1 - fa)),
			.w = vis * MAX(a.geo.w * (o|fill),
			               a.geo.w * (2 * fa * fr - fa - fr + 1)),
			.h = vis * MAX(a.geo.h * (NT(o)|fill),
			               a.geo.h * (2 * fa * fr - fa - fr + 1)),
			.filter = a.geo.filter
		}
	};
}

inline Args id(Region *r, Args a)
{
	return a;
}
