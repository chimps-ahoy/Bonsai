#include "../lib/types.h"
#include "../lib/stack.h"
#include "../lib/tree.h"
#include <curses.h>
#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>

#define other(x) (((x)+1)%2)
#define from(x) (((x)->parent->children[L] == (x)) ? L : R)
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#include <curses.h>
#ifdef CURSES
#define CTRL 0x1F
#define UNCTRL 0x60
#define SHIFT ~32
#define UNSHIFT ~SHIFT
#endif

static Tree *t;

Args partition(Node *n, Args a)
{
	if (!n || !n->parent) return a;
    Side from = from(n);
    n = n->parent;
    return (Args){
                 a.geo.x + from * a.geo.w * n->weight * !n->orient,
                 a.geo.y + from * a.geo.h * n->weight * n->orient,
             MAX(a.geo.w*n->orient,a.geo.w*n->weight*!n->orient),
             MAX(a.geo.h*!n->orient,a.geo.h*n->weight*n->orient)
            };
}

void draw(Node *n, Args a)
{
	if (n->win) {
		wclear(n->win);
		delwin(n->win);
	}
	n->win = newwin(a.geo.h, a.geo.w, a.geo.y, a.geo.x);
	if (n == t->curr) {
		box(n->win, (int)'+', (int)'+');
	} else {
		box(n->win, (int)'|', (int)'-');
	}
	wrefresh(n->win);
}

int main(void)
{ 
	initscr();
	cbreak();
	noecho();
	curs_set(0);

	t = malloc(sizeof(Tree)); 
	Node *n;
	t->root = NULL;
	t->curr = NULL;
	t->filter = 1<<1;

	WINDOW *menubar = newwin(4,80,0,0);

	int in = 0;
	while (in != 'Q') {
		switch (in) {
			case 'l':
			case 'k':
			case 'j':
			case 'h':
				t->curr = findneighbor(t->curr, keymap[HASH(in)], t->filter);
				break;
			case 'l'&CTRL:
			case 'k'&CTRL:
			case 'j'&CTRL:
			case 'h'&CTRL:
				t->curr = addclient(addsplit(t->curr, keymap[HASH(in|UNCTRL)].o,
							        0.5), NULL, keymap[HASH(in|UNCTRL)].s, t->filter);
				if (!t->curr->parent) t->root = t->curr;
				else if (!t->curr->parent->parent) t->root = t->curr->parent;
				break;
			case 'l'&SHIFT:
			case 'k'&SHIFT:
			case 'j'&SHIFT:
			case 'h'&SHIFT:
				moveclient(t->curr, keymap[HASH(in|UNSHIFT)], t->filter);
				break;
			case 'r':
				reflect(t->root);
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
				/*t->curr->tags ^= (1 << shiftnum(in));
				updatetags(t->curr->parent);*/
				break;
			case '<':
				shiftwidth(t->curr, WEST, t->filter);
				break;
			case '>':
				shiftwidth(t->curr, EAST, t->filter);
				break;
			case '.':
				shiftwidth(t->curr, NORTH, t->filter);
				break;
			case ',':
				shiftwidth(t->curr, SOUTH, t->filter);
				break;
			case 'q':
				n = orphan(t->curr);
				free(t->curr);
				if (!n || !n->parent) t->root = n;
				t->curr = NULL;
				for (int i = 0; !t->curr && i<4; i++)
					t->curr = findneighbor(t->root, (Direction){i&2,i&1}, t->filter);
				break;
		}
		box(menubar, 0, (int)'~');
		uint8_t tags = (t->curr) ? t->curr->tags >> 1 : 0;
		mvwprintw(menubar, 1, 1, "Bonsai. | filter: %.4b | current tags: %.4b"
				" | key pressed : %s |", (t->filter>>1), tags, keyname(in));
		r_apply(t->root, draw, (Args){.geo={0,getmaxy(menubar),80,24}}, partition);
		wrefresh(menubar);
		refresh();
		in = getch();
	}
	delwin(menubar);
	printtree(t->root, stderr);
	fprintf(stderr, "\n");

	freetree(t->root);
	endwin();
	return EXIT_SUCCESS;
}
