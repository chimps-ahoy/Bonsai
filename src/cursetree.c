#include "../lib/types.h"
#include "../lib/stack.h"
#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>

#define other(x) (((x)+1)%2)
#define from(x) (((x)->parent->children[left] == (x)) ? left : right)
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#include <curses.h>
#ifdef CURSES
#define CTRL 0x1F
#define UNCTRL 0x60
#define SHIFT ~32
#define UNSHIFT ~SHIFT
#endif

/*within struct enums can be replaced with uint8_t to save
on memory but makes struct definition less readable; 
may also not be entirely portable?*/
typedef struct node {
	union {
		WINDOW *client; 
		struct /*split*/ {
			struct node *children[2];
			float weight;
			Orientation orient;
		};
	};
	struct node *parent;
	Type type;
	uint8_t tags;
	char c;
} Node;

typedef struct {
	Node *root;
	Node *curr;
	uint8_t filter;
} Tree;

#define DEBUG
#ifdef DEBUG
#define tosymbol(x) ((x) ? " - " : " | ")
#define shiftnum(x) (x == '!' ? 1 : (x == '@' ? 2 : (x == '#' ? 3 : (x == '$' ? 4 : 0))))
void _printtree(FILE *f, Node *n)
{
	if (n->type == split) {
		fprintf(f, " ( ");
		_printtree(f, n->children[left]);
		fprintf(f, "%s@%p ", tosymbol(n->orient), (void*)n);
		_printtree(f, n->children[right]);
		fprintf(f, " ) ");
		return;
	}
	fprintf(f, "%c", n->c);
}

void printtree(FILE *f, Tree *t)
{
	_printtree(f, t->root);
}
#endif

void _freetree(Node *t)
{
	if (!t) return;
	if (t->type == split) {
		_freetree(t->children[left]);
		_freetree(t->children[right]);
	} else {
		delwin(t->client);
	}
	free(t);
}

void freetree(Tree *t)
{
	_freetree(t->root);
	free(t);
}

uint8_t _updatetags_g(Node *n)
{
	if (!n) return 0;
	if (n->type == split)
		n->tags = _updatetags_g(n->children[left]) | _updatetags_g(n->children[right]);
	return n->tags;
}

void updatetags_r(Tree *t)
{
	(void)_updatetags_g(t->root);
}

void updatetags(Node *n)
{
	if (!n) return;
	n->tags = n->children[left]->tags | n->children[right]->tags;
	updatetags(n->parent);
	return;
}

Node *addsplit(Tree *t, Node *tosplit, Orientation o, float weight)
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
	new->children[left] = tosplit;
	new->children[right] = NULL;
	new->tags = tosplit->tags;
	if (!(new->parent)) t->root = new;
	return new;
}

Node *addclient(Node *currsplit, uint8_t filter, char c, Side child)
{
	if (currsplit->children[child]) 
		currsplit->children[other(child)] = currsplit->children[child];
	currsplit->children[child] = malloc(sizeof(Node));
	if (!(currsplit->children[child])) return NULL;
	currsplit->children[child]->parent = currsplit;
	currsplit->children[child]->type = client;
	currsplit->children[child]->c = c;
	currsplit->children[child]->tags = filter;
	currsplit->children[child]->client = NULL;
	updatetags(currsplit);
	return currsplit->children[child];	
}

Node *findnext(uint8_t filter, Node *n, const Direction d, Stack *breadcrumbs)
{	
	if (n->type == split && n->orient == d.o && n->children[other(d.s)]->tags & filter)
		return findnext(filter, n->children[other(d.s)], d, breadcrumbs);
	if (n->type == split && n->orient == d.o)
		return findnext(filter, n->children[d.s], d, breadcrumbs);
	Side crumb = pop(breadcrumbs);
	if (n->type == split && n->children[crumb]->tags & filter)
		return findnext(filter, n->children[crumb], d, breadcrumbs);
	if (n->type == split)
		return findnext(filter, n->children[other(crumb)], d, breadcrumbs);
	return n;
}

void flip(Node *n)
{
	if (n->type == split) {
		n->orient = other(n->orient);
		flip(n->children[left]);
		flip(n->children[right]);
	}
}

void killclient(Tree *t, Node *n)
{
	Node *parent = n->parent;
	if (!parent) return;

	Side s = other(from(n));
	free(n);
	n = parent->children[s];

	if (parent->parent) {
		s = from(parent);
		parent->parent->children[s] = n;
		n->parent = parent->parent;
		free(parent);
	} else {
		n->parent = NULL;
		t->root = n;
	}
	updatetags(n->parent);
	t->curr = findnext(t->filter, n, WEST, NULL);
}

Node *findparent(Node *n, uint8_t filter, Orientation o, Side s, Stack *breadcrumbs)
{
	Node *parent = n->parent;
	while (parent) {
		int nobacktrack = (s != FAKESIDE) ? parent->children[s] != n : 1; //TODO: this is kinda hacky...
		int tagmatch = (s != FAKESIDE) ? parent->children[s]->tags & filter : 1;
		if (parent->orient == o && nobacktrack && tagmatch)
			return parent;
		else if (parent->orient != o)
			push(breadcrumbs, from(n));
		n = parent;
		parent = parent->parent;
	}
	return parent;
}

void spawn(Tree *t, char c, const Direction d)
{
	(void)addsplit(t, t->curr, d.o, 0.5);
	t->curr = addclient(t->curr->parent, t->filter, c, d.s);
}

void shiftwidth(Tree *t, const Direction d)
{
	Node *parent = findparent(t->curr, t->filter, d.o, FAKESIDE, NULL);
	if (!parent && t->curr->parent)
		parent = findparent(t->curr->parent->children[other(from(t->curr))],
							t->filter, d.o, FAKESIDE, NULL);
	if (parent && d.s == right)
		parent->weight = MAX(0.1,MIN(parent->weight+0.1, 0.9));
	else if (parent)
		parent->weight = MIN(0.9,MAX(parent->weight-0.1, 0.1));
}

Node *findneighbor(Node *curr, unsigned char filter, const Direction d)
{
	Stack *breadcrumbs = initstack();
	Node *parent = findparent(curr, filter, d.o, d.s, breadcrumbs);
	if (parent)
		curr = findnext(filter, parent->children[d.s], d, breadcrumbs);
	freestack(breadcrumbs);
	return curr;
}

void movefocus(Tree *t, const Direction d)
{
	t->curr = findneighbor(t->curr, t->filter, d);
}

void moveclient(Tree *t, const Direction d)
{
	if (!t->curr->parent) return;
	if (t->curr->parent->orient != d.o) {
		Side s = from(t->curr); 
		if (s != d.s) {
			t->curr->parent->children[s] = t->curr->parent->children[other(s)];
			t->curr->parent->children[other(s)] = t->curr;
		}
		t->curr->parent->orient = d.o;
		return;
	}
	if (t->root->children[d.s] == t->curr) return;

	Node *neighbor = findneighbor(t->curr, t->filter, d);
	if (neighbor != t->curr && neighbor->parent == t->curr->parent) {
		Side s = from(neighbor);
		neighbor->parent->children[s] = t->curr;
		neighbor->parent->children[other(s)] = neighbor;
		return;
	}

	Side s = d.s;
	if (neighbor == t->curr) neighbor = t->root;
	else s = other(d.s);

	neighbor = addsplit(t, neighbor, d.o, 0.5);
	if (neighbor->children[s])
		neighbor->children[other(s)] = neighbor->children[s];
	neighbor->children[s] = t->curr;
	if (t->curr->parent->parent) {
		t->curr->parent->parent->children[from(t->curr->parent)] 
			= t->curr->parent->children[other(from(t->curr))];
		t->curr->parent->children[other(from(t->curr))]->parent
			= t->curr->parent->parent;
	} else {
		t->root = t->curr->parent->children[other(from(t->curr))];
		t->curr->parent->children[other(from(t->curr))]->parent = NULL;
	}
	free(t->curr->parent);
	t->curr->parent = neighbor;
	updatetags_r(t);
}

void drawclient(Tree *t, Node *n ,int lines, int cols, int y, int x)
{ 
	if (!n) return;
	for (int i = 0; i < 2; i++) {
		if (n->type == split && !(n->children[i]->tags & t->filter)) {
			drawclient(t, n->children[other(i)], lines, cols, y, x);
			return;
		}
	}
	if (n->type == split && n->orient == hori) {
		drawclient(t, n->children[left], lines*n->weight, cols, y, x);
		drawclient(t, n->children[right], lines*(1-n->weight), cols, 
										y+(lines*n->weight), x);
		return;
	}
	if (n->type == split) {
		drawclient(t, n->children[left], lines, cols*n->weight, y, x);
		drawclient(t, n->children[right], lines, cols*(1-n->weight),
											y,	x+(cols*n->weight));
		return;
	}
	if (n->client) {
		wclear(n->client);
		delwin(n->client);
	}
	n->client = newwin(lines, cols, y, x);
	if (n == t->curr)
		box(n->client, (int)'+', (int)'+');
	else
		box(n->client, (int)n->c, (int)n->c);
	wrefresh(n->client);
}

void drawclients(Tree *t, int y, int x, int yoff, int xoff)
{
	drawclient(t, t->root, y, x, yoff, xoff);
}

int main(void)
{ 
	initscr();
	cbreak();
	noecho();
	curs_set(0);

	Tree *t = malloc(sizeof(Tree)); //--------------------------------
	Node *n = malloc(sizeof(Node)); //This whole block will be within some 
	n->type = client;				//inittree function
	n->parent = NULL;
	t->filter = (1<<1)|(1<<2);
	t->root = n;
	t->curr = n;					
	n->c = 'a';
	n->tags = 1 << 1;				//----------------------------------

	WINDOW *menubar = newwin(4,80,0,0);

	int in = 0;
	while (in != 'Q') {
		switch (in) {
			case 'l':
			case 'k':
			case 'j':
			case 'h':
				movefocus(t, keymap[HASH(in)]);
				break;
			case 'l'&CTRL:
			case 'k'&CTRL:
			case 'j'&CTRL:
			case 'h'&CTRL:
				spawn(t, getch(), keymap[HASH(in|UNCTRL)]);
				break;
			case 'l'&SHIFT:
			case 'k'&SHIFT:
			case 'j'&SHIFT:
			case 'h'&SHIFT:
				moveclient(t, keymap[HASH(in|UNSHIFT)]);
				break;
			case 'r':
				flip(t->root);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
				t->filter = 1 << (in - '0');
				break;
			case '!':
			case '@':
			case '#':
			case '$':
				t->curr->tags ^= (1 << shiftnum(in));
				updatetags(t->curr->parent);
				break;
			case '<':
				shiftwidth(t, WEST);
				break;
			case '>':
				shiftwidth(t, EAST);
				break;
			case '.':
				shiftwidth(t, NORTH);
				break;
			case ',':
				shiftwidth(t, SOUTH);
				break;
			case 'q':
				killclient(t, t->curr);
				break;
		}
		drawclients(t, 24, 80, getmaxy(menubar), 0);
		box(menubar, 0, (int)'~');
		mvwprintw(menubar, 1, 1, "Bonsai-%s | filter: %.4b | current tags: %.4b"
				" | key pressed : %s |", VERSION, (t->filter>>1), (t->curr->tags>>1), keyname(in));
		wrefresh(menubar);
		refresh();
		printtree(stderr, t);
		fprintf(stderr, "\n");
		in = getch();
	}


	delwin(menubar);
	freetree(t);
	endwin();
	printf("sizeof keys: %ld\n", sizeof(keymap)/sizeof(keymap[0]));
	return EXIT_SUCCESS;
}
