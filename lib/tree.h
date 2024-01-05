#ifndef TREE_H
#define TREE_H
#include <stdint.h>
#include "types.h"
#include "stack.h"

/*
 * A node in the binary split tree of windows.
 */
typedef struct node Node;

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

/* Initializes a tree struct on the heap.
 *
 * RETURNS: a pointer to the new tree
 */
Tree *inittree();

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
Node *addclient(Node *, Side, uint8_t);

/* Recursively flips the orientation of all splits downwards from
 * the given node
 *
 * PARAMS: The node to start from
 */
void reflect(Node *);

/* Kills the given node (rehomes?)
 *
 * PARAMS: The node to kill
 * WARNING: This node is *not* freed, that must be done by the caller!
 *
 * RETURNS: A pointer to the node which takes the place of the killed node.
 * NOTE: Must check the return value to see if it should become the new root
 */
Node *kill(Node *);/* TODO: rename this?*/

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
Node *move(Node *, const Direction, uint8_t);
#endif
