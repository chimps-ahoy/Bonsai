#ifndef TREE_H
#define TREE_H
#include <stdint.h>
#include <stdio.h>
#include <curses.h>
#include "types.h"
#include "stack.h"
/*
 * A node in the binary split tree of windows.
 */
typedef WINDOW * Window;
typedef struct node {
	union {
		Window win;
		struct /*split*/ {
			struct node *children[2];
			float weight;
			Orientation orient;
		};
	};
	struct node *parent;
	Type type;
	uint8_t tags;
} Node;

/*
 * A binary split tree of windows.
 * contains a pointer  to the root,
 * the current focused node,
 * and a bit array for the tag filter of the current view
 */
typedef struct {
	Node *root;
	Node *curr;
	uint8_t filter;
} Tree;

void printtree(Node *, FILE *);

/* Recursively frees a tree downwards from the given node
 * 
 * PARAMS: The node to start freeing from
 */
void freetree(Node *);

/* Splits the given node in half with the given orientation and weight
 *
 * PARAMS: The node to split, the split's orientation and weight
 *
 * RETURNS: A pointer to the new split node
 */
Node *addsplit(Node *, Orientation, float);

/* Adds a client to the given node on the given side with the given tags
 *
 * PARAMS: The node which will parent the new node, the side to which
 * the new node should belong, and the tags the new node will have
 * TODO: add more params for x11 stuff
 *
 * RETURNS: A pointer to the new node
 * NOTE: Must check the return value to see if it should become the new root
 */
Node *addclient(Node *, Window, Side, uint8_t);

/* Recursively flips the orientation of all splits downwards from
 * the given node
 *
 * PARAMS: The node to start from
 */
void reflect(Node *);

/* Finds the node which contains the given window, looking downwards
 * from the given node
 *
 * PARAMS: The node to start from, the window whose node we want to find
 *
 * RETURNS: A pointer to the node which contains the given window
 */
Node *find(Node *, Window);

/* Orphans the given node, detacting it from its parent and causing its
 * sibling to superscede it
 *
 * PARAMS: The node to orphan
 * WARNING: This node is *not* freed, that must be done by the caller!
 *
 * RETURNS: A pointer to the node which takes the place of the killed node.
 * NOTE: Must check the return value to see if it should become the new root
 */
Node *orphan(Node *);

/* Shifts the width of the given node in the given direction according
 * to the view specified by the filter
 *
 * PARAMS: The node we want to resize, the direction in which are are 
 * resizing, and the filter which specifies our view
 */
void shiftwidth(Node *, const Direction, uint8_t);

/* Finds the closest neighbor (geometrically as the windows/splits are
 * laid out on the screen) to the given node in the given direction
 * according to the view specified by the filter
 *
 * PARAMS: The node to start from, the direction we are going, and the 
 * current filter
 *
 * RETURNS: The closest neighbor found
 */
Node *findneighbor(Node *, const Direction, uint8_t);

/* Moves the given node in the given direction with the given filter
 *
 * PARAMS: The node we want to move, the direction to move it in, and the filter
 *
 * RETURNS: A pointer to the node which replaces the moved node
 * NOTE: Must check return value to see if node should become the new root
 */
Node *moveclient(Node *, const Direction, uint8_t);

/* Recursively applies function F downwards from n with Args a and transform
 * T.
 * Each split encountered applies T to a and recursively calls r_apply to
 * the children.
 * Upon reaching a leaf node, F(n,a) is called. Note that T is not applied to
 * a on the final call
 *
 * PARAMS: A pointer to the starting node, the function to apply to the leaves,
 * the arguments to pass to the function, and the transformation function
 * to those arguments
 *
 * RETURNS: void
 */
void r_apply(Node *n, void(*F)(Node *, Args), Args a, Args(*T)(Node *, Args));
#endif
