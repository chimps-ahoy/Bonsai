#ifndef TILES_H
#define TILES_H

#include <stdint.h>
#include <stdio.h>
#include <types.h>
#include <stack.h>

#define IN(x) (((x)->parent->subregion[L] == (x)) ? L : R)

/*
 * A region of the screen, represented by a node in a binary split tree
 */
typedef struct node {
	union {
		Window win;/*client*/
		struct /*split*/ {
			struct node *subregion[2];
			float fact;
			Orientation o;
		};
	};
	struct node *parent;
	Type type;
	uint8_t tags;
} Region;

/*
 * A binary split tree of windows.
 * contains a pointer to the root(screen),
 * the current focused node,
 * and a bit array for the tag filter of the current view
 */
typedef struct {
	Region *whole;
	Region *curr;
	uint8_t filter;
} Tiling;

/* Debug printing
 *
 */
void printtree(Region *, FILE *, Args a);

/* A wrapper for free() to allow it to be called with trickle() to free
 * an entire tree
 * 
 * TODO: this should probably be defined within the main file so we can
 * also free any Windows attached
 */
void freeregion(Region *, Args);

/* Updates the tags of the given split Region to be the union of its
 * childrens' tags, then travels to its parent and repeats until reaching the
 * root
 *
 * PARAMS: The region whose tags we want to update. Should be a split region.
 */
void updatetags(Region *);

/* Splits the given region in half with the given orientation and fact
 *
 * PARAMS: The region to split, the split's orientation and fact
 *
 * RETURNS: A pointer to the new split
 */
Region *split(Region *, Orientation, float);

/* Adds a client to the given region on the given side with the given tags
 *
 * PARAMS: The region which will contain the new client, the side to which
 * the client should belong, and the tags the client will have
 *
 * RETURNS: A pointer to the Region
 * NOTE: Must check the return value to see if it should become the screen(root)
 */
Region *spawn(Region *, Window, Side, uint8_t);

/* Reflects the given region about the line x=y
 *
 * PARAMS: The region to reflect
 */
void reflect(Region *);

/* Finds the subregion which contains the given window within the specified
 * region
 *
 * PARAMS: The region to search within, the window whose node we want to find
 *
 * RETURNS: A pointer to the region(leaf) which contains the given window
 */
Region *find(Region *, Window, uint8_t);

/* Orphans the given node, detacting it from its parent and causing its
 * sibling to superscede it
 *
 * PARAMS: The node to orphan
 * WARNING: This node is *not* freed, that must be done by the caller!
 *
 * RETURNS: A pointer to the node which takes the place of the killed node.
 * NOTE: Must check the return value to see if it should become the new root
 */
Region *orphan(Region *);

/* Shifts the width of the given node in the given direction according
 * to the view specified by the filter
 *
 * PARAMS: The Region we want to resize, the direction in which are are 
 * resizing, and the filter which specifies our view
 */
void shiftwidth(Region *, const Direction, uint8_t);

/* Finds the closest neighbor (geometrically as the windows/splits are
 * laid out on the screen) to the given Region in the given direction
 * according to the view specified by the filter
 *
 * PARAMS: The Region to start from, the direction we are going, and the 
 * current filter
 *
 * RETURNS: The closest neighbor found
 */
Region *findneighbor(Region *, const Direction, uint8_t);

/* Moves the given node in the given direction with the given filter
 *
 * PARAMS: The node we want to move, the direction to move it in, and the filter
 *
 * RETURNS: A pointer to the node which replaces the moved node
 * NOTE: Must check return value to see if node should become the new root
 */
Region *moveclient(Region *, const Direction, uint8_t);

/* Recursively applies function F downwards from n with Args a and transform
 * T.
 * Each split encountered applies T to a and recursively calls trickle to
 * each subregion.
 * Upon reaching a leaf node, F(n,a) is called. Note that T is not applied to
 * a on the final call
 *
 * PARAMS: A pointer to the starting node, the function to apply to the leaves,
 * the arguments to pass to the function, and the transformation function
 * to those arguments
 *
 * RETURNS: void
 */
void trickle(Region *n, void(*F)(Region *, Args), Args a, Args(*T)(Region *, Args));


/* Calculates the geometry of a Region given its parent's dimensions
 * as an Args(geo). Returns as an Args(geo)
 *
 * PARAMS: The node and its parent's dimension as an Args(geo)
 *
 * RETURNS: The dimensions of the given node as an Args(geo)
 */
Args partition(Region *, Args);
#endif
