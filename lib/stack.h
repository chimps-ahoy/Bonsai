#ifndef STACK_H
#define STACK_H

#include <types.h>

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
