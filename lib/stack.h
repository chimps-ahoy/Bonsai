#include "types.h"
#include <stdlib.h>
#ifndef STACK_H
#define STACK_H
typedef struct stack Stack;

/*
 * allocates a stack on the heap and returns a pointer to it 
 */
Stack *initstack(void);

/*
 * recursively frees the stack
 */
void freestack(Stack *);

/*
 * pushes the value onto the stack
 */
Stack *push(Stack *, Side);

/*
 * pops the value
 */
Side pop(Stack *);

#endif
