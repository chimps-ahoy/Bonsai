#include <stdlib.h>
#include <stdio.h>

#include "../lib/types.h"
#include "../lib/tree.h"
#include "../lib/stack.h"
#include "../lib/util.h"

void printtree(Region *r, FILE *f, Args a)
{
	if (!r) {
		fprintf(f, "NULL");
		return;
	}
	if (r->type == SPLIT) {
		fprintf(f, "(");
		printtree(r->subregion[L], f, partition(r->subregion[L], a));
		fprintf(f, (r->orient == H) ? " - " : " | ");
		printtree(r->subregion[H], f, partition(r->subregion[L], a));
		fprintf(f, ")");
		return;
	}
	fprintf(f, "%ld={(%d,%d) %dx%d}", (long)(r->win), a.geo.x, a.geo.y,
			                                            a.geo.w, a.geo.h);
}

void freeregion(Region *r, Args _)
{
	free(r);
}

void updatetags(Region *r)
{
	if (!r) return;
	r->tags = r->subregion[L]->tags | r->subregion[R]->tags;
	updatetags(r->parent);
	return;
}

Region *split(Region *tosplit, Orientation o, float weight)
{
	if (!tosplit) return NULL;
	Region *new = malloc(sizeof(Region));
	if (!new) return NULL;
	new->type = SPLIT;
	new->orient = o;
	new->weight = weight;
	new->parent = tosplit->parent;
	if (new->parent)
		new->parent->subregion[IN(tosplit)] = new;
	tosplit->parent = new;
	new->subregion[L] = tosplit;
	new->subregion[R] = NULL;
	new->tags = tosplit->tags;
	return new;
}

Region *spawn(Region *currsplit, Window w, Side child, uint8_t tags)
{
	if (!currsplit) {
		Region *new = malloc(sizeof(Region));
		new->parent = NULL;
		new->type = CLIENT;
		new->tags = tags;
		new->win = w;
		return new;
	}
	if (currsplit->subregion[child]) 
		currsplit->subregion[NT(child)] = currsplit->subregion[child];
	currsplit->subregion[child] = malloc(sizeof(Region));
	if (!(currsplit->subregion[child])) return NULL;
	currsplit->subregion[child]->parent = currsplit;
	currsplit->subregion[child]->type = CLIENT;
	currsplit->subregion[child]->tags = tags;
	currsplit->subregion[child]->win = w;
	updatetags(currsplit);
	return currsplit->subregion[child];	
}

static Region *findnext(Region *r, const Direction d, uint8_t filter, Stack *breadcrumbs)
{	
	if (r->type == SPLIT && r->orient == d.o && r->subregion[NT(d.s)]->tags & filter)
		return findnext(r->subregion[NT(d.s)], d, filter, breadcrumbs);
	if (r->type == SPLIT && r->orient == d.o)
		return findnext(r->subregion[d.s], d, filter, breadcrumbs);
	Side crumb = pop(breadcrumbs);
	if (r->type == SPLIT && r->subregion[crumb]->tags & filter)
		return findnext(r->subregion[crumb], d, filter, breadcrumbs);
	if (r->type == SPLIT)
		return findnext(r->subregion[NT(crumb)], d, filter, breadcrumbs);
	return r;
}

static Region *findparent(Region *r, uint8_t filter, const Direction d, Stack *breadcrumbs)
{
	Region *parent = r->parent;
	while (parent) {
		int nobacktrack = (d.s != FAKESIDE) ? parent->subregion[d.s] != r : 1; //TODO: this is kinda hacky...
		int tagmatch = (d.s != FAKESIDE) ? parent->subregion[d.s]->tags & filter : 1;
		if (parent->orient == d.o && nobacktrack && tagmatch)
			return parent;
		else if (parent->orient != d.o)
			push(breadcrumbs, IN(r));
		r = parent;
		parent = parent->parent;
	}
	return parent;
}

Region *findneighbor(Region *curr, const Direction d, uint8_t filter)
{
	if (!curr) return NULL;
	Stack *breadcrumbs = initstack();
	Region *parent = findparent(curr, filter, d, breadcrumbs);
	if (parent)
		curr = findnext(parent->subregion[d.s], d, filter, breadcrumbs);
	freestack(breadcrumbs);
	return curr;
}

void reflect(Region *r)
{
	if (r->type == SPLIT) {
		r->orient = NT(r->orient);
		reflect(r->subregion[L]);
		reflect(r->subregion[R]);
	}
}

Region *find(Region *r,  Window w, uint8_t filter)
{
	if (!r) return NULL;
	if (r->type == SPLIT && r->tags & filter) {
		Region *tmp;
		if ((tmp = find(r->subregion[L], w, filter))) return tmp;
		if ((tmp = find(r->subregion[R], w, filter))) return tmp;
	}
	if (r->type == CLIENT && (!w || r->win == w) && r->tags & filter) return r;//hack to find first win
	return NULL;
}

Region *orphan(Region *r)
{
	if (!r) return NULL;
	Region *parent = r->parent;
	if (!parent) return NULL; /*probably no*/

	r = parent->subregion[NT(IN(r))];

	if (parent->parent) {
		parent->parent->subregion[IN(parent)] = r;
		r->parent = parent->parent;
		updatetags(r->parent);
	} else {
		r->parent = NULL;
	}
	free(parent);
	return r;
}

void shiftwidth(Region *r, const Direction d, uint8_t filter)
{
	if (!r) return;
	Region *parent = findparent(r, filter, (Direction){ d.o, FAKESIDE }, NULL);
	if (!parent && r->parent)
		parent = findparent(r->parent->subregion[NT(IN(r))],
							filter, (Direction){ d.o, FAKESIDE }, NULL);
	if (parent && d.s == R)
		parent->weight = MAX(0.1,MIN(parent->weight+0.1, 0.9));
	else if (parent)
		parent->weight = MIN(0.9,MAX(parent->weight-0.1, 0.1));
}

Region *moveclient(Region *r, const Direction d, uint8_t filter)
{ 
	if (!r) return NULL;
	if (!r->parent) return r;
	if (r->parent->orient != d.o) {
		Side s = IN(r); 
		if (s != d.s) {
			r->parent->subregion[s] = r->parent->subregion[NT(s)];
			r->parent->subregion[NT(s)] = r;
		}
		r->parent->orient = d.o;
		return r->parent;
	}

	Region *neighbor = findneighbor(r, d, filter);
	if (neighbor != r && neighbor->parent == r->parent) {
		Side s = IN(neighbor);
		neighbor->parent->subregion[s] = r;
		neighbor->parent->subregion[NT(s)] = neighbor;
		return neighbor->parent;
	}

	Side s = d.s;
	if (neighbor != r) s = NT(d.s);
	else for(; neighbor->parent; neighbor = neighbor->parent); 

	neighbor = split(neighbor, d.o, 0.5);
	if (neighbor->subregion[s])
		neighbor->subregion[NT(s)] = neighbor->subregion[s];
	neighbor->subregion[s] = r;
	Region *tmp = orphan(r);
	r->parent = neighbor;
	return (tmp->parent) ? neighbor : tmp;
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

Args partition(Region *r, Args a)
{
	Side fr = IN(r);
	uint8_t vis = !!(r->tags & a.geo.filter);
	r = r->parent;
	uint8_t fill = !(r->subregion[NT(fr)]->tags & a.geo.filter);
	Orientation o = r->orient;
	float fa = r->weight;
	return (Args){ .geo = {
	               .x = (a.geo.x + (fr & NT(fill) & NT(o)) * a.geo.w * (1 - fa)),
	               .y = (a.geo.y + (fr & NT(fill) & o)     * a.geo.h * (1 - fa)),
	               .w = vis * MAX((o|fill) * a.geo.w,
	                        a.geo.w * (2 * fa * fr - fa - fr + 1)),
	               .h = vis * MAX((NT(o)|fill) * a.geo.h,
	                        a.geo.h * (2 * fa * fr - fa - fr + 1)),
	               .filter = a.geo.filter
	               }
	             };
}
#undef NT
