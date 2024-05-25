#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "tiles.h"
#include "config.h"

#define VIEW_INFO ((Args){.geo = {.x=gappx/2,.y=barpx+gappx/2,.w=sw-gappx,.h=sh-barpx-gappx,.filter=t->filter}})
#ifdef DEBUG
#define LOG(x,...) fprintf(stderr, x, __VA_ARGS__)
#else
#define LOG(x,...) do {} while(0)
#endif

static Tiling *t = NULL;
static int sw;
static int sh;

void draw(Region *n, Args a)
{
	/* todo! */
}

int main(void)
{
	return EXIT_FAILURE;
}
