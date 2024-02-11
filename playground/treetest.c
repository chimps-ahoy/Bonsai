#include "../lib/types.h"
#include "../lib/stack.h"
#include "../lib/tree.h"
#include "../lib/util.h"
#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>

#define CTRL 0x1F
#define UNCTRL 0x60
#define SHIFT ~32
#define UNSHIFT ~SHIFT
#define shiftnum(x) (((x)=='$') ? 4 : (((x)=='#') ? 3 : (((x)=='@') ? 2 : ((((x)=='!') ? 1 : 0)))))

static Tree *t; //kinda makes more sense to not even use the struct for this?
				//just have 3 statics?
				//makes the names nicer IMO

void debugpr(Region *r, Args a)
{
	fprintf(stderr, "%p - {(%d,%d)%dX%d}\n", r->win, a.geo.x, a.geo.y, a.geo.w, a.geo.h);
}

Args dummyt(Region *_, Args __)
{
	return __;
}

void draw(Region *r, Args a)
{
	if (r->win) {
		wclear(r->win);
		delwin(r->win);
	}
	r->win = newwin(a.geo.h, a.geo.w, a.geo.y, a.geo.x);
	if (!a.geo.w || !a.geo.h) return;
	if (r == t->curr) {
		box(r->win, (int)'+', (int)'+');
	} else {
		box(r->win, (int)'|', (int)'-');
	}
	wrefresh(r->win);
}

int main(void)
{ 
	initscr();
	cbreak();
	noecho();
	curs_set(0);

	t = malloc(sizeof(Tree)); 
	Region *r;
	t->screen = NULL;
	t->curr = NULL;
	t->filter = 1;

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
				t->curr = spawn(split(t->curr, keymap[HASH(in|UNCTRL)].o,
							        0.75), NULL, keymap[HASH(in|UNCTRL)].s, t->filter);
				if (!t->curr->parent) t->screen = t->curr;
				else if (!t->curr->parent->parent) t->screen = t->curr->parent;
				break;
			case 'l'&SHIFT:
			case 'k'&SHIFT:
			case 'j'&SHIFT:
			case 'h'&SHIFT:
			r = moveclient(t->curr, keymap[HASH(in|UNSHIFT)], t->filter);
				if (r && !r->parent) t->screen = r;
				break;
			case 'r':
				reflect(t->screen);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
				t->filter = 1 << (in - '0' - 1);
				t->curr = find(t->screen,0,t->filter);
				break;
			case '!':
			case '@':
			case '#':
			case '$':
				t->curr->tags ^= (1 << (shiftnum(in)-1));
				updatetags(t->curr->parent);
				t->curr = find(t->screen,0,t->filter);
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
				r = orphan(t->curr);
				free(t->curr);
				if (!r || !r->parent) t->screen = r;
				t->curr = find(r,0,t->filter);
				break;
		}
		uint8_t tags = (t->curr) ? t->curr->tags : 0;
		trickle(t->screen, draw, (Args){.geo={0,getmaxy(menubar),80,24,t->filter}}, partition);
		fprintf(stderr, "input: %s\n", keyname(in));
		trickle(t->screen, debugpr, (Args){.geo={.x=0,.y=0,.w=1920,.h=1080,.filter=t->filter}}, partition);
		fprintf(stderr, "\n\n");
		box(menubar, 0, (int)'~');
		mvwprintw(menubar, 1, 1, "Bonsai. | filter: %.4b | current tags: %.4b"
				" | key pressed : %s |", t->filter, tags, keyname(in));
		wrefresh(menubar);
		refresh();
		in = getch();
	}

	trickle(t->screen,freeregion,(Args){0},dummyt);
	delwin(menubar);
	delwin(stdscr);
	endwin();
	return EXIT_SUCCESS;
}
