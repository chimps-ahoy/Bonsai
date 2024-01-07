#include "../lib/tree.h"
#include "../lib/stack.h"
#include "../lib/util.h"
#define other(x) (((x)+1)%2)
#define from(x) (((x)->parent->children[L] == (x)) ? L : R)
/* TODO:
 * - expand for X11 structs
 * - 
 */

struct node {
	union {
		void *client;
		struct /*split*/ {
			struct node *children[2];
			float weight;
			Orientation orient;
		};
	};
	struct node *parent;
	Type type;
	uint8_t tags;
};

Tree *inittree()
{
	Tree *t = malloc(sizeof(Tree));
	Node *n = malloc(sizeof(Node));
	if (!t || !n) return NULL;
	n->type = client;
	n->parent = NULL;
	t->filter = (1<<1);
	t->root = n;
	t->curr = n;
	n->tags = t->filter;
	return t;
}

void freetree(Node *n)
{
	if (!n) return;
	if (n->type == split) {
		freetree(n->children[L]);
		freetree(n->children[R]);
	} else {
		/*delwin(t->client); --TODO: this for X11*/
	}
	free(n);
}

static uint8_t updatetags_g(Node *n)
{
	if (!n) return 0;
	if (n->type == split)
		n->tags = updatetags_g(n->children[L])
			    | updatetags_g(n->children[R]);
	return n->tags;
}

static void updatetags(Node *n)
{
	if (!n) return;
	n->tags = n->children[L]->tags | n->children[R]->tags;
	updatetags(n->parent);
	return;
}

Node *addsplit(Node *tosplit, Orientation o, float weight)
{
	Node *new = malloc(sizeof(Node));
	if (!new) return NULL;
	new->type = split;
	new->orient = o;
	new->weight = weight;
	new->parent = tosplit->parent;
	if (new->parent)
		new->parent->children[from(tosplit)] = new;
	tosplit->parent = new;
	new->children[L] = tosplit;
	new->children[R] = NULL;
	new->tags = tosplit->tags;
	/*if (!(new->parent)) t->root = new; --TODO: this check happens at call site*/
	return new;
}

Node *addclient(Node *currsplit, Side child, uint8_t tags)
{
	if (currsplit->children[child]) 
		currsplit->children[other(child)] = currsplit->children[child];
	currsplit->children[child] = malloc(sizeof(Node));
	if (!(currsplit->children[child])) return NULL;
	currsplit->children[child]->parent = currsplit;
	currsplit->children[child]->type = client;
	currsplit->children[child]->tags = tags;
	currsplit->children[child]->client = NULL;
	updatetags(currsplit);
	return currsplit->children[child];	
}

/* --FOR SPAWNING--
   t->curr = addclient(addsplit(t->curr, d.o, 0.5), t->filter, d.s);
NOTE: we just need addclient to handle null.
if we don't have enough memory to open another client, we just dont!
}*/

static Node *findnext(Node *n, const Direction d, uint8_t filter, Stack *breadcrumbs)
{	
	if (n->type == split && n->orient == d.o && n->children[other(d.s)]->tags & filter)
		return findnext(n->children[other(d.s)], d, filter, breadcrumbs);
	if (n->type == split && n->orient == d.o)
		return findnext(n->children[d.s], d, filter, breadcrumbs);
	Side crumb = pop(breadcrumbs);
	if (n->type == split && n->children[crumb]->tags & filter)
		return findnext(n->children[crumb], d, filter, breadcrumbs);
	if (n->type == split)
		return findnext(n->children[other(crumb)], d, filter, breadcrumbs);
	return n;
}

static Node *findparent(Node *n, uint8_t filter, const Direction d, Stack *breadcrumbs)
{
	Node *parent = n->parent;
	while (parent) {
		int nobacktrack = (d.s != FAKESIDE) ? parent->children[d.s] != n : 1; //TODO: this is kinda hacky...
		int tagmatch = (d.s != FAKESIDE) ? parent->children[d.s]->tags & filter : 1;
		if (parent->orient == d.o && nobacktrack && tagmatch)
			return parent;
		else if (parent->orient != d.o)
			push(breadcrumbs, from(n));
		n = parent;
		parent = parent->parent;
	}
	return parent;
}

Node *findneighbor(Node *curr, const Direction d, uint8_t filter)
{
	Stack *breadcrumbs = initstack();
	Node *parent = findparent(curr, filter, d, breadcrumbs);
	if (parent)
		curr = findnext(parent->children[d.s], d, filter, breadcrumbs);
	freestack(breadcrumbs);
	return curr;
}

void reflect(Node *n)
{
	if (n->type == split) {
		n->orient = other(n->orient);
		reflect(n->children[L]);
		reflect(n->children[R]);
	}
}

Node *orphan(Node *n)
{
	Node *parent = n->parent;
	if (!parent) return n; /*probably no*/

	n = parent->children[other(from(n))];

	if (parent->parent) {
		parent->parent->children[from(parent)] = n;
		n->parent = parent->parent;
	} else {
		n->parent = NULL;
	}
	free(parent);
	updatetags(n->parent);
	return n;
	/* This is how orphan() should be used in the main file
	 * ```
	 * Node *tmp = orphan(t->curr)
	 * free(t->curr);
	 * t->curr = tmp;
	 * if (!t->curr->parent) t->root = t->curr;
	 * t->curr = findfirst(t->curr, t->filter);
	 * ```
	 */
}


void shiftwidth(Node *n, const Direction d, uint8_t filter)
{
	Node *parent 
		= findparent(n, filter, (Direction){ d.o, FAKESIDE }, NULL);
	if (!parent && n->parent)
		parent = findparent(n->parent->children[other(from(n))],
							filter, (Direction){ d.o, FAKESIDE }, NULL);
	if (parent && d.s == R)
		parent->weight = MAX(0.1,MIN(parent->weight+0.1, 0.9));
	else if (parent)
		parent->weight = MIN(0.9,MAX(parent->weight-0.1, 0.1));
}

Node *move(Node *n, const Direction d, uint8_t filter)
{ 
	if (!n->parent) return n;
	if (n->parent->orient != d.o) {
		Side s = from(n); 
		if (s != d.s) {
			n->parent->children[s] = n->parent->children[other(s)];
			n->parent->children[other(s)] = n;
		}
		n->parent->orient = d.o;
		return n->parent->children[s];
	}

	Node *neighbor = findneighbor(n, d, filter);
	if (neighbor != n && neighbor->parent == n->parent) {
		Side s = from(neighbor);
		neighbor->parent->children[s] = n;
		neighbor->parent->children[other(s)] = neighbor;
		return neighbor;
	}

	Side s = d.s;
	if (neighbor == n) for(; neighbor->parent; neighbor = neighbor->parent); 
	else s = other(d.s);

	neighbor = addsplit(neighbor, d.o, 0.5);
	if (neighbor->children[s])
		neighbor->children[other(s)] = neighbor->children[s];
	neighbor->children[s] = n;
	Node *tmp = orphan(n);
	n->parent = neighbor;
	return tmp;
	/* Within the main file we will have to do the same check as for orphan()
	 *
	 * Node *tmp = move(...);
	 * t->curr = tmp;
	 * if (!t->curr->parent) t->root = t->curr;
	 * t->curr = findfirst(t->curr, t->filter);
	 * //in fact it is the same aside from freeing the current, so perhaps
	 * //there can be some DRYing
	 */
}
#undef other
#undef from
